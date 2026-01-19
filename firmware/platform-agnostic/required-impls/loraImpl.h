#ifndef shared_loraImpl_h_INCLUDED
#define shared_loraImpl_h_INCLUDED
#include <stdint.h>
#include <stdbool.h>

// TODO This should be lora_interPacketTimeout, not loraImpl.
// TODO: Figure out how low this can go.
// TODO: Figure out how to do error checking platform-agnostically.
#define loraImpl_interPacketTimeout_d 500 //ms

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
void loraImpl_irqProcess(void);

// Non blocking, when the IRQ process is run it will either run
// the rx timeout callback,
// rxdone callback
// or do nothing because timeout hasn't been reached and no packet has been received
void loraImpl_setRx(uint32_t milliseconds);

void loraImpl_setIdle();

// Holds the actual temporary packet data.
typedef struct
{
    // This will always point to the
    void (*onTxDone)(void);
    // This is maximum 60 byte packet (well actually 255 is the max but bandwidth
    //                                 problems at long range so 60 to be safe)
    void (*onRxDone)(uint8_t* payload, uint16_t size, int16_t rssi,
                     int8_t snr);
    void (*onTxTimeout)(void);
    void (*onRxTimeout)(void);
    void (*onRxError)(void);

    uint8_t dataBuffer[loraImpl_numBufferBytes_d];
    // Number of bytes that the dataBuffer currently contains
    uint16_t dataCurrentContentSize;
    int16_t  rssi;
    int8_t   snr;
    bool preInitComplete;
    bool postInitComplete;
} loraImpl_globalState_t;

extern loraImpl_globalState_t loraImpl_globalState_g;

#endif // shared_loraImpl_h_INCLUDED
