#include "shared_lora.h"
#include "string.h"
#include "utilsImpl.h"

// Brief:
// The implementation of an n-byte (max 1mb) lora packeting system
// Which serves as an abstraction over the main lora HAL implementation.
// Has similar API to the ai-thinker library, but allows for much larger
// payloads to be sent.

// The loraImpl libraries are platform specific, but should satisfy the
// requirements as stated in the header file.

// Plans for a CRC are available, but are not implemented as of 11/01.
// Written by: aids

// Define the global state
lora_globalState_t lora_globalState_g = {
    .dataReceived           = false,
    .dataBuffer             = {},
    .dataCurrentContentSize = 0,
    .backendRssi            = 0,
    .backendSnr             = 0,
    .rxState                = lora_rxStates_waitingForHeader,
    .backendRxDone          = false,
    .backendTxDone          = false,
    .backendRxError         = false,
    .backendRxTimeout       = false,
    .backendTxTimeout       = false,
    .errorType              = lora_rxErrorTypes_none,
};

static bool _lora_waitUntilTxDone(void)
{
    while (!lora_globalState_g.backendTxDone)
    {
        loraImpl_irqProcess();
        utils_sleepMs(1);
        // spinloop
        if (lora_globalState_g.backendTxTimeout)
        {
            return false;
        }
    }
    lora_globalState_g.backendTxDone = false;
    return true;
}

void lora_init()
{
    loraImpl_init();
    loraImpl_setRx(0);
    loraImpl_setCallbacks(
        _lora_backendTxDoneCallback, _lora_backendRxDoneCallback,
        _lora_backendTxTimeoutCallback,
        _lora_backendRxTimeoutCallback, _lora_backendRxErrorCallback);
}

void lora_setRx(uint16_t ms)
{
    loraImpl_setRx(ms);
}

void lora_deinit()
{
    loraImpl_deinit();
    memset(&lora_globalState_g, 0, sizeof(lora_globalState_g));
    lora_globalState_g.rxState = lora_rxStates_waitingForHeader;
}

static uint16_t _lora_calculateNumPackets(uint16_t payloadSize)
{
    uint16_t numPackets = 2;
    if (payloadSize > 0)
    {
        // Funny formula for correct values (execpt when payloadSize = 0)
        numPackets = (payloadSize - 1) / lora_numDataBytes_d + 2;
    }
    return numPackets;
}

lora_headerPacket_t lora_createHeaderPacket(uint8_t* payload,
                                            uint16_t payloadSize)
{
    uint16_t numPackets   = _lora_calculateNumPackets(payloadSize);
    uint8_t  numDataBytes = (lora_numDataBytes_d < payloadSize) ?
         lora_numDataBytes_d :
         payloadSize;
    lora_headerPacket_t header = {
        .preamble     = loraImpl_headerPacketPreamble_d,
        .numDataBytes = numDataBytes,
        .packetNumber = 1,
        .numPackets   = numPackets,
        .data         = {},
    };
    memcpy(&header.data, payload, numDataBytes);
    return header;
}

lora_dataPacket_t lora_createDataPacket(uint8_t* payload,
                                        uint16_t payloadIndex,
                                        uint16_t payloadSize)
{
    uint16_t numPackets = _lora_calculateNumPackets(payloadSize);
    uint8_t  numDataBytes =
        (lora_numDataBytes_d < payloadSize - payloadIndex) ?
         lora_numDataBytes_d :
         payloadSize - payloadIndex;
    // starts from 0, so index of 32 represents 33rd value, and thus 2nd packet.
    // index of 31 represents 1st packet, and that should cause an error/not be possible.
    uint16_t packetNumber = (payloadIndex) / lora_numDataBytes_d + 1;
    lora_dataPacket_t dataPacket = {
        .preamble     = loraImpl_dataPacketPreamble_d,
        .numDataBytes = numDataBytes,
        .packetNumber = packetNumber,
        .numPackets   = numPackets,
        .data         = {},
    };
    memcpy(&dataPacket.data, payload + payloadIndex, numDataBytes);
    return dataPacket;
}

lora_footerPacket_t lora_createFooterPacket(uint8_t* payload,
                                            uint16_t payloadSize)
{
    uint16_t numPackets = _lora_calculateNumPackets(payloadSize);
    // For now we won't compute the crc.
    lora_footerPacket_t footer = {
        .preamble = loraImpl_footerPacketPreamble_d,
        // Again we are the last packet.
        .packetNumber = numPackets,
        .numPackets   = numPackets,
        .crc          = 0,
    };
    return footer;
}

void lora_setCallbacks(void (*onTxDone)(void),
                       void (*onRxDone)(uint8_t* payload,
                                        uint16_t size, int16_t rssi,
                                        int8_t snr),
                       void (*onTxTimeout)(void),
                       void (*onRxTimeout)(void),
                       void (*onRxError)(void))
{
    lora_globalState_g.onRxError   = onRxError;
    lora_globalState_g.onTxDone    = onTxDone;
    lora_globalState_g.onRxDone    = onRxDone;
    lora_globalState_g.onRxTimeout = onRxTimeout;
    lora_globalState_g.onTxTimeout = onTxTimeout;
}

void lora_send(uint8_t* payload, const uint16_t payloadSize)
{
    // Caution? Hopefully this won't be called when receiving a packet,
    // as when receiving a packet we should wait until we receive everything.
    // It shouldn't be able to be called when the irq process is running.

    // Reset all the TX states.
    lora_globalState_g.backendTxTimeout = false;
    lora_globalState_g.backendTxDone    = false;

    // Send the header packet
    lora_headerPacket_t header =
        lora_createHeaderPacket(payload, payloadSize);
    loraImpl_send((uint8_t*)&header, sizeof(header));

    if (!_lora_waitUntilTxDone())
    {
        // If we fail to TX due to timing out.
        if (lora_globalState_g.backendTxTimeout)
            lora_globalState_g.onTxTimeout();
        return;
    }

    // Send the data packet
    for (uint16_t i = lora_numDataBytes_d; i < payloadSize;
         i += lora_numDataBytes_d)
    {
        lora_dataPacket_t dataPacket =
            lora_createDataPacket(payload, i, payloadSize);
        loraImpl_send((uint8_t*)&dataPacket, sizeof(dataPacket));
        if (!_lora_waitUntilTxDone())
        {
            // If we fail to TX due to timing out.
            if (lora_globalState_g.backendTxTimeout)
                lora_globalState_g.onTxTimeout();
            return;
        }
    }

    // Send the footer packet
    lora_footerPacket_t footer =
        lora_createFooterPacket(payload, payloadSize);
    loraImpl_send((uint8_t*)&footer, sizeof(lora_footerPacket_t));
    if (!_lora_waitUntilTxDone())
    {
        // If we fail to TX due to timing out.
        if (lora_globalState_g.backendTxTimeout)
            lora_globalState_g.onTxTimeout();
        return;
    }

    if (lora_globalState_g.backendTxDone)
        lora_globalState_g.onTxDone();

    // Reset all the TX states.
    lora_globalState_g.backendTxTimeout = false;
    lora_globalState_g.backendTxDone    = false;
    return;
}

void lora_queryState(void)
{
    // TODO: For unit testing
    // IDK i'll think about it later.
}

static bool _lora_appendData(uint8_t* payload, uint16_t size)
{
    if (lora_globalState_g.dataCurrentContentSize + size >
        lora_numBufferBytes_d)
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_failedToAppend;
        return false;
    }

    uint8_t* head = lora_globalState_g.dataBuffer +
        lora_globalState_g.dataCurrentContentSize;
    memcpy(head, payload, size);
    lora_globalState_g.dataCurrentContentSize += size;
    return true;
}

// Reset and setup total packet counts, packet number, data buffer and
// data Received
// Will also start appending to data buffer.
bool _lora_processHeaderPacket(uint8_t* payload, uint16_t size)
{
    // Check that the values are possible
    if (size != sizeof(lora_headerPacket_t))
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_headerPacketSizeMismatch;
        return false;
    }

    lora_headerPacket_t header;
    memcpy(&header, payload, sizeof(lora_headerPacket_t));
    // Check that the values correspond to what's expected next (a data packet or a footer packet)
    if (header.packetNumber != 1) // first packet so 1
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_headerPacketNumberIsNot1;
        return false;
    }

    // Set it's values as the truth

    lora_globalState_g.currentPacketNumber     = 1;
    lora_globalState_g.currentTotalPacketCount = header.numPackets;
    lora_globalState_g.dataCurrentContentSize  = 0;

    // Append
    if (!_lora_appendData(header.data, header.numDataBytes))
    {
        lora_globalState_g.rxState = lora_rxStates_rxError;
        return false;
    }
    return true;
}

// Append to data buffer as well as the above
bool _lora_processDataPacket(uint8_t* payload, uint16_t size)
{
    // Check that the values are possible
    if (size != sizeof(lora_dataPacket_t))
    {
        lora_globalState_g.errorType = lora_rxErrorTypes_dataPacketSizeMismatch;
        return false;
    }

    lora_dataPacket_t data;
    memcpy(&data, payload, sizeof(lora_dataPacket_t));
    // Check that the values correspond to what's expected next
    if (data.numPackets != lora_globalState_g.currentTotalPacketCount)
    {
        lora_globalState_g.errorType = lora_rxErrorTypes_dataPacketNumPacketsMismatch;
        return false;
    }

    // Not the next packet
    if (data.packetNumber !=
        lora_globalState_g.currentPacketNumber + 1)
    {
        lora_globalState_g.errorType = lora_rxErrorTypes_dataPacketNotConsecutive;
        return false;
    }

    // Append
    if (!_lora_appendData(data.data, data.numDataBytes))
    {
        lora_globalState_g.rxState = lora_rxStates_rxError;
        return false;
    }
    lora_globalState_g.currentPacketNumber += 1;
    return true;
}

// Set data received = true.
bool _lora_processFooterPacket(uint8_t* payload, uint16_t size)
{
    // Check that the values are possible
    if (size != sizeof(lora_footerPacket_t))
        return false;

    lora_footerPacket_t footer;
    memcpy(&footer, payload, sizeof(lora_footerPacket_t));

    // Check that the values correspond to what was expected (footer packet)
    if (footer.numPackets !=
        lora_globalState_g.currentTotalPacketCount)
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_footerPacketNumPacketsMismatch;
        return false;
    }
    if (footer.packetNumber != footer.numPackets)
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_footerPacketIsNotLast;
        return false;
    }

    if (footer.packetNumber !=
        lora_globalState_g.currentPacketNumber + 1)
    {
        lora_globalState_g.errorType =
            lora_rxErrorTypes_footerPacketIsNotConsecutive;
        return false;
    }

    lora_globalState_g.currentPacketNumber += 1;
    // Do crc check (unimplemented)
    return true;
}

// A large process which activates the receiving.
// This means that when we receive something, we will try to continue receiving
// until it finishes, timeout, or error occurs.
void lora_irqProcess(void)
{
    // Will also activate RxTimeout if need be.
    do
    {
        loraImpl_irqProcess();
        // Process the RX errors and timeouts
        switch (lora_globalState_g.rxState)
        {
            case lora_rxStates_rxError:
                // Whoever has their callback can inspect the state
                // to figure out what went wrong.
                lora_globalState_g.onRxError();
                // Don't forget to reset the states!
                goto lora_irqProcess_resetRxStates_gt;

            case lora_rxStates_rxTimeout:
                lora_globalState_g.onRxTimeout();
                goto lora_irqProcess_resetRxStates_gt;

            case lora_rxStates_rxDone:
                lora_globalState_g.onRxDone(
                    lora_globalState_g.dataBuffer,
                    lora_globalState_g.dataCurrentContentSize,
                    lora_globalState_g.backendRssi,
                    lora_globalState_g.backendSnr);
                goto lora_irqProcess_resetRxStates_gt;
            default: break;
        }
        utils_sleepMs(1);
    } while (lora_globalState_g.rxState !=
             lora_rxStates_waitingForHeader);
    return;

lora_irqProcess_resetRxStates_gt:
    lora_globalState_g.rxState       = lora_rxStates_waitingForHeader;
    lora_globalState_g.backendRxDone = false;
    lora_globalState_g.backendRxError   = false;
    lora_globalState_g.backendRxTimeout = false;
    lora_globalState_g.errorType        = lora_rxErrorTypes_none;
    return;
}

// Has its own FSM to figure out how to generate the new types of packets.
// And when to call its own callbacks.
// DOES NOT CALL CALLBACKS!!!!
void _lora_backendRxDoneCallback(uint8_t* payload, uint16_t size,
                                 int16_t rssi, int8_t snr)
{
    // Obvious error checking
    if (size < 1)
        goto _lora_backendRxDoneCallback_error_gt;
    loraImpl_globalState_g.rssi    = rssi;
    loraImpl_globalState_g.snr     = snr;
    lora_globalState_g.backendRssi = rssi;
    lora_globalState_g.backendSnr  = snr;

    switch (lora_globalState_g.rxState)
    {
        case lora_rxStates_waitingForHeader:
            if (payload[0] == loraImpl_headerPacketPreamble_d)
            {
                if (!_lora_processHeaderPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                lora_globalState_g.rxState =
                    lora_rxStates_waitingForDataOrFooter;
                // non-blocking??? Might be a source of error.
                loraImpl_setRx(loraImpl_interPacketTimeout_d);
                break;
            }
            else
                goto _lora_backendRxDoneCallback_error_gt;

        case lora_rxStates_waitingForDataOrFooter:
            // Figure out if footer or header
            // Set state to respective state.
            if (payload[0] == loraImpl_footerPacketPreamble_d)
            {
                // Process the footer packet
                if (!_lora_processFooterPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                lora_globalState_g.rxState = lora_rxStates_rxDone;

                loraImpl_setIdle();
                return;
            }
            else if (payload[0] == loraImpl_dataPacketPreamble_d)
            {
                if (!_lora_processDataPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                // Continue receiving.
                loraImpl_setRx(loraImpl_interPacketTimeout_d);
            }
            else
                goto _lora_backendRxDoneCallback_error_gt;

        default: break;
    }
    return;

_lora_backendRxDoneCallback_error_gt:
    lora_globalState_g.rxState = lora_rxStates_rxError;
    return;
}

void _lora_backendTxDoneCallback(void)
{
    lora_globalState_g.backendTxDone = true;
}

void _lora_backendRxTimeoutCallback(void)
{
    lora_globalState_g.rxState          = lora_rxStates_rxTimeout;
    lora_globalState_g.backendRxTimeout = true;
}

void _lora_backendTxTimeoutCallback(void)
{
    lora_globalState_g.backendTxTimeout = true;
}

void _lora_backendRxErrorCallback(void)
{
    lora_globalState_g.errorType = lora_rxErrorTypes_spuriousError;
    lora_globalState_g.backendRxError = true;
}
