#include "tremo_delay.h"
#include <loraCfg.h>
#include <loraImpl.h>
#include <radio.h>
#include <tremo_gpio.h>
#include <tremo_pwr.h>
#include <tremo_rcc.h>
#include <timer.h>
#include <rtc-board.h>
#include <stddef.h>

static void dummyCallback() {}
static void dummyRxDone(uint8_t*, uint16_t, int16_t, int8_t) {}

// C23 {} 0's everything out. Obviously we use C23, not like anything will break.
// Global State
loraImpl_globalState_t loraImpl_globalState_g = {
    .onTxDone               = dummyCallback,
    .onRxDone               = dummyRxDone,
    .onRxError              = dummyCallback,
    .onRxTimeout            = dummyCallback,
    .onTxTimeout            = dummyCallback,
    .dataBuffer             = {},
    .dataCurrentContentSize = {},
    .rssi                   = {},
    .snr                    = {},
};

static RadioEvents_t radioEvents = {
    .TxDone               = dummyCallback,
    .RxDone               = dummyRxDone,
    .RxError              = dummyCallback,
    .RxTimeout            = dummyCallback,
    .TxTimeout            = dummyCallback,
};

// The underlying lora implementation for sending.
void loraImpl_send(uint8_t* payload, uint16_t payloadSize)
{
    Radio.Send(payload, payloadSize);
    while (Radio.GetStatus() == RF_TX_RUNNING)
        delay_ms(1);
}

void loraImpl_setCallbacks(void (*onTxDone)(void),
                           void (*onRxDone)(uint8_t* payload,
                                            uint16_t size,
                                            int16_t rssi, int8_t snr),
                           void (*onTxTimeout)(void),
                           void (*onRxTimeout)(void),
                           void (*onRxError)(void))
{
    loraImpl_globalState_g.onTxDone    = onTxDone;
    loraImpl_globalState_g.onRxDone    = onRxDone;
    loraImpl_globalState_g.onTxTimeout = onTxTimeout;
    loraImpl_globalState_g.onRxTimeout = onRxTimeout;
    loraImpl_globalState_g.onRxError   = onRxError;
    radioEvents.TxDone                 = onTxDone;
    radioEvents.RxDone                 = onRxDone;
    radioEvents.TxTimeout              = onTxTimeout;
    radioEvents.RxTimeout              = onRxTimeout;
    radioEvents.RxError                = onRxError;
}

// init
void loraImpl_init(void)
{
// #define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//                                                               //  1: 250 kHz,
//                                                               //  2: 500 kHz,
//                                                               //  3: Reserved]
// #define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
// #define LORA_CODINGRATE                             1         // [1: 4/5,
//                                                               //  2: 4/6,
//                                                               //  3: 4/7,
//                                                               //  4: 4/8]
// #define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
// #define LORA_SYMBOL_TIMEOUT                         0         // Symbols
// #define LORA_FIX_LENGTH_PAYLOAD_ON                  false
// #define LORA_IQ_INVERSION_ON                        false
// #define TX_OUTPUT_POWER                             14        // dBm
// #define RF_FREQUENCY                                915000000 // Hz
//     // luckily this takes a pointer and all updates will be noticed, so loraImpl_setCallbacks
//     // can be done later.
//     // No fdev because that's for modem_fsk (an older version of lora)
//     Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
//                                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
//                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
//                                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

//     Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
//                                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
//                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
//                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    Radio.Init(&radioEvents);
    Radio.SetChannel(loraCfg_frequency_d);
    Radio.SetTxConfig(
        MODEM_LORA, loraCfg_txOutputPower_d, 0, loraCfg_bandwidth_d,
        loraCfg_spreadingFactor_d, loraCfg_codingRate_d,
        loraCfg_preambleLength_d, loraCfg_fixLengthPayloadOn_d,
        loraCfg_crcOn_d, loraCfg_freqHop_d, loraCfg_hopPeriod_d,
        loraCfg_iqInversionOn_d, loraCfg_txTimeout_ms_d);

    Radio.SetRxConfig(
        MODEM_LORA, loraCfg_bandwidth_d, loraCfg_spreadingFactor_d,
        loraCfg_codingRate_d, 0, loraCfg_preambleLength_d,
        loraCfg_symbolTimeout_d, loraCfg_fixLengthPayloadOn_d, 0,
        loraCfg_crcOn_d, loraCfg_freqHop_d, 0,
        loraCfg_iqInversionOn_d, true);
}

// Deinit
void loraImpl_deinit(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, false);
    // I don't think you can reset the txconfig or rxconfig.
}

// Returns the global state
void loraImpl_queryState(void)
{
    // noop
}

// Process non-blocking peripherals
// Also checks the time since last RX to allow for non-blocking SetRX
// Please run this once per main loop.
void loraImpl_irqProcess(void)
{
    Radio.IrqProcess();
}

// Non blocking, when the IRQ process is run it will either run
// the rx timeout callback,
// rxdone callback
// or do nothing because timeout hasn't been reached and no packet has been received
void loraImpl_setRx(uint32_t milliseconds)
{
    Radio.Rx(milliseconds);
}

void loraImpl_setIdle()
{
    Radio.Sleep();
}
