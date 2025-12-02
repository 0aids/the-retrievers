#include "global_radio.h"
#include "global_lora.h"

#include "radio.h"
#include "packets/packets.h"
#include <stdint.h>
// ONLY USED FOR THE LORA STANDALONE

static RadioEvents_t RadioEvents;
#define X(name) case name: return #name;
const char* const gr_RadioStatesToString(e_radioState state) {
    switch (state) {
        d_radioStatesXMacro
        default:
            return "Unknown radio state";
    }
}

#undef X

void gr_RadioSetTxDoneCallback(void (*callback)(void))
{
    RadioEvents.TxDone = callback;
}

void gr_RadioSetRxDoneCallback(void (*callback)(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr))
{
    RadioEvents.RxDone = callback;
}

void gr_RadioSetTxTimeoutCallback(void (*callback)(void))
{
    RadioEvents.TxTimeout = callback;
}

void gr_RadioSetRxTimeoutCallback(void (*callback)(void))
{
    RadioEvents.RxTimeout = callback;
}

void gr_RadioSetRxErrorCallback(void (*callback)(void))
{
    RadioEvents.RxError = callback;
}

void gr_RadioInit() {
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}

void gr_RadioSend(uint8_t *buffer, uint16_t bufferSize) {
    Radio.Send(buffer, bufferSize);
}

void gr_RadioCheckRecv()
{
    Radio.IrqProcess();
}


void gr_RadioSetRx(uint32_t timeout)
{
    Radio.Rx(timeout);
}

void gr_RadioSetIdle()
{
    Radio.Sleep();
}

e_radioState gr_RadioGetStatus()
{
    RadioState_t radioState = Radio.GetStatus();
    switch (radioState) {
        case RF_IDLE: return gr_RadioStates_Idle;   //!< The radio is idle
        case RF_RX_RUNNING: return gr_RadioStates_Rx; //!< The radio is in reception state
        case RF_TX_RUNNING: return gr_RadioStates_Tx; //!< The radio is in transmission state
        default: return gr_RadioStates_Unknown;
    }
}

int16_t gr_RadioGetRSSI()
{
    return Radio.Rssi(MODEM_LORA);
}

uint32_t gr_RadioGetTimeOnAir(packet_t *packet)
{
    return Radio.TimeOnAir(MODEM_LORA, packet->m_dataSize + 1);
}
