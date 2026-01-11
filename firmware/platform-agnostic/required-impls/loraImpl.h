#ifndef shared_loraImpl_h_INCLUDED
#define shared_loraImpl_h_INCLUDED
#include <stdint.h>

// Shared stuff for lora, just a whole bunch of defines.
// clang-format off
#define loraImpl_frequency_d 915000000                 // Hz
#define loraImpl_TXOutputPower_d 14                    // dBm
                                                   //
#define loraImpl_bandwidth_d 0                         // [0: 125 kHz,
                                                   //  1: 250 kHz,
                                                   //  2: 500 kHz,
                                                   //  3: Reserved]
                                                   //
#define loraImpl_spreadingFactor_d 7                   // [SF7..SF12]
                                                   //
#define loraImpl_codingRate_d 1                        // [1: 4/5,
                                                   //  2: 4/6,
                                                   //  3: 4/7,
                                                   //  4: 4/8]
                                                   //
#define loraImpl_preambleLength_d 8                    // Same for Tx and Rx
#define loraImpl_symbolTimeout_d 0                     // Symbols
#define loraImpl_fixLengthPayloadOn_d false
#define loraImpl_IQInversionOn_d false

#define lora_headerPacketPreamble_d 0xff // 0b11111111
#define lora_dataPacketPreamble_d 0xaa   // 0b10101010
#define lora_footerPacketPreamble_d 0xcc // 0b11001100

// TODO: Figure out how low this can go.
#define lora_interPacketTimeout_d 500 //ms

// clang-format on

#define loraImpl_numBufferBytes_d ((uint8_t)255)

// --------------------------------------------------------------------------
// Hardware Abstraction Layer (Implemented per-device)
// --------------------------------------------------------------------------

// The underlying lora implementation for sending.
void loraImpl_send(uint8_t* payload, uint16_t payloadSize);

void loraImpl_setCallbacks(void (*onTxDone)(void),
                           void (*onRxDone)(uint8_t* payload,
                                            uint16_t size,
                                            int16_t rssi, int8_t snr),
                           void (*onTxTimeout)(void),
                           void (*onRxTimeout)(void),
                           void (*onRxError)(void));

// init
void loraImpl_init(void);

// Deinit
void loraImpl_deinit(void);

// Returns the global state
void loraImpl_queryState(void);

// Process non-blocking peripherals
// Also checks the time since last RX to allow for non-blocking SetRX
// Please run this once per main loop.
void loraImpl_IRQProcess(void);

// Non blocking, when the IRQ process is run it will either run
// the rx timeout callback,
// rxdone callback
// or do nothing because timeout hasn't been reached and no packet has been received
void loraImpl_SetRX(uint32_t milliseconds);

void loraImpl_SetIdle();

// Holds the actual temporary packet data.
typedef struct
{
    // This will always point to the
    void (*onTXDone)(void);
    // This is maximum 60 byte packet (well actually 255 is the max but bandwidth
    //                                 problems at long range so 60 to be safe)
    void (*onRXDone)(uint8_t* payload, uint16_t size, int16_t rssi,
                     int8_t snr);
    void (*onTXTimeout)(void);
    void (*onRXTimeout)(void);
    void (*onRXError)(void);

    uint8_t dataBuffer[loraImpl_numBufferBytes_d];
    // Number of bytes that the dataBuffer currently contains
    uint16_t dataCurrentContentSize;
    int16_t  rssi;
    int8_t   snr;
} loraImpl_globalState_t;

extern loraImpl_globalState_t loraImpl_globalState_g;

#endif // shared_loraImpl_h_INCLUDED
