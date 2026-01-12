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
    .backendRSSI            = 0,
    .backendSNR             = 0,
    .RXState                = lora_RXStates_WaitingForHeader,
    .backendRXDone          = false,
    .backendTXDone          = false,
    .backendRXError         = false,
    .backendRXTimeout       = false,
    .backendTXTimeout       = false,
    .errorType              = lora_RXErrorTypes_none,
};

static bool _lora_waitUntilTXDone(void)
{
    while (!lora_globalState_g.backendTXDone)
    {
        loraImpl_IRQProcess();
        utils_sleepms(1);
        // spinloop
        if (lora_globalState_g.backendTXTimeout)
        {
            return false;
        }
    }
    lora_globalState_g.backendTXDone = false;
    return true;
}

void lora_init()
{
    loraImpl_init();
    loraImpl_setRX(0);
    loraImpl_setCallbacks(
        _lora_backendTXDoneCallback, _lora_backendRXDoneCallback,
        _lora_backendTXTimeoutCallback,
        _lora_backendRXTimeoutCallback, _lora_backendRXErrorCallback);
}

void lora_setRX(uint16_t ms)
{
    loraImpl_setRX(ms);
}

void lora_deinit()
{
    loraImpl_deinit();
    memset(&lora_globalState_g, 0, sizeof(lora_globalState_g));
    lora_globalState_g.RXState = lora_RXStates_WaitingForHeader;
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
        .preamble     = lora_headerPacketPreamble_d,
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
        .preamble     = lora_dataPacketPreamble_d,
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
        .preamble = lora_footerPacketPreamble_d,
        // Again we are the last packet.
        .packetNumber = numPackets,
        .numPackets   = numPackets,
        .crc          = 0,
    };
    return footer;
}

void lora_setCallbacks(void (*onTXDone)(void),
                       void (*onRXDone)(uint8_t* payload,
                                        uint16_t size, int16_t rssi,
                                        int8_t snr),
                       void (*onTXTimeout)(void),
                       void (*onRXTimeout)(void),
                       void (*onRXError)(void))
{
    lora_globalState_g.onRXError   = onRXError;
    lora_globalState_g.onTXDone    = onTXDone;
    lora_globalState_g.onRXDone    = onRXDone;
    lora_globalState_g.onRXTimeout = onRXTimeout;
    lora_globalState_g.onTXTimeout = onTXTimeout;
}

void lora_send(uint8_t* payload, const uint16_t payloadSize)
{
    // Caution? Hopefully this won't be called when receiving a packet,
    // as when receiving a packet we should wait until we receive everything.
    // It shouldn't be able to be called when the irq process is running.

    // Reset all the TX states.
    lora_globalState_g.backendTXTimeout = false;
    lora_globalState_g.backendTXDone    = false;

    // Send the header packet
    lora_headerPacket_t header =
        lora_createHeaderPacket(payload, payloadSize);
    loraImpl_send((uint8_t*)&header, sizeof(header));

    if (!_lora_waitUntilTXDone())
    {
        // If we fail to TX due to timing out.
        if (lora_globalState_g.backendTXTimeout)
            lora_globalState_g.onTXTimeout();
        return;
    }

    // Send the data packet
    for (uint16_t i = lora_numDataBytes_d; i < payloadSize;
         i += lora_numDataBytes_d)
    {
        lora_dataPacket_t dataPacket =
            lora_createDataPacket(payload, i, payloadSize);
        loraImpl_send((uint8_t*)&dataPacket, sizeof(dataPacket));
        if (!_lora_waitUntilTXDone())
        {
            // If we fail to TX due to timing out.
            if (lora_globalState_g.backendTXTimeout)
                lora_globalState_g.onTXTimeout();
            return;
        }
    }

    // Send the footer packet
    lora_footerPacket_t footer =
        lora_createFooterPacket(payload, payloadSize);
    loraImpl_send((uint8_t*)&footer, sizeof(lora_footerPacket_t));
    if (!_lora_waitUntilTXDone())
    {
        // If we fail to TX due to timing out.
        if (lora_globalState_g.backendTXTimeout)
            lora_globalState_g.onTXTimeout();
        return;
    }

    if (lora_globalState_g.backendTXDone)
        lora_globalState_g.onTXDone();

    // Reset all the TX states.
    lora_globalState_g.backendTXTimeout = false;
    lora_globalState_g.backendTXDone    = false;
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
            lora_RXErrorTypes_failedToAppend;
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
            lora_RXErrorTypes_headerPacketSizeMismatch;
        return false;
    }

    lora_headerPacket_t header;
    memcpy(&header, payload, sizeof(lora_headerPacket_t));
    // Check that the values correspond to what's expected next (a data packet or a footer packet)
    if (header.packetNumber != 1) // first packet so 1
    {
        lora_globalState_g.errorType =
            lora_RXErrorTypes_headerPacketNumberIsNot1;
        return false;
    }

    // Set it's values as the truth

    lora_globalState_g.currentPacketNumber     = 1;
    lora_globalState_g.currentTotalPacketCount = header.numPackets;
    lora_globalState_g.dataCurrentContentSize  = 0;

    // Append
    if (!_lora_appendData(header.data, header.numDataBytes))
    {
        lora_globalState_g.RXState = lora_RXStates_RxError;
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
        lora_globalState_g.errorType = lora_RXErrorTypes_dataPacketSizeMismatch;
        return false;
    }

    lora_dataPacket_t data;
    memcpy(&data, payload, sizeof(lora_dataPacket_t));
    // Check that the values correspond to what's expected next
    if (data.numPackets != lora_globalState_g.currentTotalPacketCount)
    {
        lora_globalState_g.errorType = lora_RXErrorTypes_dataPacketNumPacketsMismatch;
        return false;
    }

    // Not the next packet
    if (data.packetNumber !=
        lora_globalState_g.currentPacketNumber + 1)
    {
        lora_globalState_g.errorType = lora_RXErrorTypes_dataPacketNotConsecutive;
        return false;
    }

    // Append
    if (!_lora_appendData(data.data, data.numDataBytes))
    {
        lora_globalState_g.RXState = lora_RXStates_RxError;
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
            lora_RXErrorTypes_footerPacketNumPacketsMismatch;
        return false;
    }
    if (footer.packetNumber != footer.numPackets)
    {
        lora_globalState_g.errorType =
            lora_RXErrorTypes_footerPacketIsNotLast;
        return false;
    }

    if (footer.packetNumber !=
        lora_globalState_g.currentPacketNumber + 1)
    {
        lora_globalState_g.errorType =
            lora_RXErrorTypes_footerPacketIsNotConsecutive;
        return false;
    }

    lora_globalState_g.currentPacketNumber += 1;
    // Do crc check (unimplemented)
    return true;
}

// A large process which activates the receiving.
// This means that when we receive something, we will try to continue receiving
// until it finishes, timeout, or error occurs.
void lora_IRQProcess(void)
{
    // Will also activate RxTimeout if need be.
    do
    {
        loraImpl_IRQProcess();
        // Process the RX errors and timeouts
        switch (lora_globalState_g.RXState)
        {
            case lora_RXStates_RxError:
                // Whoever has their callback can inspect the state
                // to figure out what went wrong.
                lora_globalState_g.onRXError();
                // Don't forget to reset the states!
                goto lora_IRQProcess_resetRXStates_gt;

            case lora_RXStates_RxTimeout:
                lora_globalState_g.onRXTimeout();
                goto lora_IRQProcess_resetRXStates_gt;

            case lora_RXStates_RxDone:
                lora_globalState_g.onRXDone(
                    lora_globalState_g.dataBuffer,
                    lora_globalState_g.dataCurrentContentSize,
                    lora_globalState_g.backendRSSI,
                    lora_globalState_g.backendSNR);
                goto lora_IRQProcess_resetRXStates_gt;
            default: break;
        }
        utils_sleepms(1);
    } while (lora_globalState_g.RXState !=
             lora_RXStates_WaitingForHeader);
    return;

lora_IRQProcess_resetRXStates_gt:
    lora_globalState_g.RXState       = lora_RXStates_WaitingForHeader;
    lora_globalState_g.backendRXDone = false;
    lora_globalState_g.backendRXError   = false;
    lora_globalState_g.backendRXTimeout = false;
    lora_globalState_g.errorType        = lora_RXErrorTypes_none;
    return;
}

// Has its own FSM to figure out how to generate the new types of packets.
// And when to call its own callbacks.
// DOES NOT CALL CALLBACKS!!!!
void _lora_backendRXDoneCallback(uint8_t* payload, uint16_t size,
                                 int16_t rssi, int8_t snr)
{
    // Obvious error checking
    if (size < 1)
        goto _lora_backendRxDoneCallback_error_gt;
    loraImpl_globalState_g.rssi    = rssi;
    loraImpl_globalState_g.snr     = snr;
    lora_globalState_g.backendRSSI = rssi;
    lora_globalState_g.backendSNR  = snr;

    switch (lora_globalState_g.RXState)
    {
        case lora_RXStates_WaitingForHeader:
            if (payload[0] == lora_headerPacketPreamble_d)
            {
                if (!_lora_processHeaderPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                lora_globalState_g.RXState =
                    lora_RXStates_WaitingForDataOrFooter;
                // non-blocking??? Might be a source of error.
                loraImpl_setRX(lora_interPacketTimeout_d);
                break;
            }
            else
                goto _lora_backendRxDoneCallback_error_gt;

        case lora_RXStates_WaitingForDataOrFooter:
            // Figure out if footer or header
            // Set state to respective state.
            if (payload[0] == lora_footerPacketPreamble_d)
            {
                // Process the footer packet
                if (!_lora_processFooterPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                lora_globalState_g.RXState = lora_RXStates_RxDone;

                loraImpl_setIdle();
                return;
            }
            else if (payload[0] == lora_dataPacketPreamble_d)
            {
                if (!_lora_processDataPacket(payload, size))
                    goto _lora_backendRxDoneCallback_error_gt;

                // Continue receiving.
                loraImpl_setRX(lora_interPacketTimeout_d);
            }
            else
                goto _lora_backendRxDoneCallback_error_gt;

        default: break;
    }
    return;

_lora_backendRxDoneCallback_error_gt:
    lora_globalState_g.RXState = lora_RXStates_RxError;
    return;
}

void _lora_backendTXDoneCallback(void)
{
    lora_globalState_g.backendTXDone = true;
}

void _lora_backendRXTimeoutCallback(void)
{
    lora_globalState_g.RXState          = lora_RXStates_RxTimeout;
    lora_globalState_g.backendRXTimeout = true;
}

void _lora_backendTXTimeoutCallback(void)
{
    lora_globalState_g.backendTXTimeout = true;
}

void _lora_backendRXErrorCallback(void)
{
    lora_globalState_g.errorType = lora_RXErrorTypes_spuriousError;
    lora_globalState_g.backendRXError = true;
}
