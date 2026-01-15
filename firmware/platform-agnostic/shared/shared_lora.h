#ifndef shared_lora_h_INCLUDED
#define shared_lora_h_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "loraImpl.h"

// --------------------------------------------------------------------------
// Lora Protocol Layer
// --------------------------------------------------------------------------

#define lora_numDataBytes_d ((uint8_t)32)

// Payloads can now be up to 1mb in size, but realistically we'll only allocate
// up to ~2kb and check for bounds
// NOTE: the asr6601CB that the ai-thinker uses has 16kb sram.
// The esp32-s3-wroom-1u-n16r8 has 8mb of ram, 16mb flash.
#define lora_numBufferBytes_d ((uint16_t)(2048))

enum lora_RXStates_e
{
    lora_RXStates_WaitingForHeader,
    lora_RXStates_ProcessingHeader,
    lora_RXStates_WaitingForDataOrFooter,
    lora_RXStates_ProcessingData,
    lora_RXStates_ProcessFooter,
    lora_RXStates_RxTimeout,
    lora_RXStates_RxError,
    lora_RXStates_RxDone,
};

enum lora_RXErrorTypes_e
{
    lora_RXErrorTypes_none = 0,
    lora_RXErrorTypes_failedToAppend,
    lora_RXErrorTypes_spuriousError, // If the hal encounters an error.
    lora_RXErrorTypes_headerPacketNumberIsNot1,
    lora_RXErrorTypes_headerPacketSizeMismatch,
    lora_RXErrorTypes_dataPacketSizeMismatch,
    lora_RXErrorTypes_dataPacketNumPacketsMismatch,
    lora_RXErrorTypes_dataPacketNotConsecutive,
    lora_RXErrorTypes_footerPacketSizeMismatch,
    lora_RXErrorTypes_footerPacketNumPacketsMismatch,
    lora_RXErrorTypes_footerPacketIsNotLast,
    lora_RXErrorTypes_footerPacketIsNotConsecutive,
};

// This is for the collection of packets (frames)
typedef struct
{
    // When a frame is sent
    void (*onTXDone)(void);
    // When a frame is received
    void (*onRXDone)(uint8_t* payload, uint16_t size, int16_t rssi,
                     int8_t snr);
    // When a frame timeouts/fails to send
    void (*onTXTimeout)(void);
    // When a frame fails to be received due to inter-packet delay being too long
    // or just no receiving any more packets unexpectedly.
    void (*onRXTimeout)(void);
    // When a frame fails to be received due to some drastic error.
    void (*onRXError)(void);

    // Internal metrics for keeping track of packets during construction of the frame.
    uint16_t                      currentTotalPacketCount;
    uint16_t                      currentPacketNumber;
    volatile enum lora_RXStates_e RXState;

    // Same as what's given by the implementation.
    volatile int16_t backendRSSI;
    volatile int8_t  backendSNR;

    // Stuff for ensuring callbacks are called
    // These are only set to true via callbacks to the implementation
    // and false whenever we start trying to recv or send.
    volatile bool backendRXDone;
    volatile bool backendTXDone;
    volatile bool backendRXError;
    volatile bool backendRXTimeout;
    volatile bool backendTXTimeout;

    // Is true when the databuffer is fully formed
    bool     dataReceived;
    uint16_t dataCurrentContentSize;
    uint8_t  dataBuffer[lora_numBufferBytes_d];
    enum lora_RXErrorTypes_e errorType;
} lora_globalState_t;

extern lora_globalState_t lora_globalState_g;

#pragma pack(push, 1)
typedef struct
{
    uint8_t  preamble;
    uint8_t  numDataBytes; // Number of data bytes.
    uint16_t packetNumber; // starting from 1
    uint16_t numPackets;   // including this one, and the footer
    uint8_t  data[lora_numDataBytes_d];
} lora_headerPacket_t;

typedef struct
{
    uint8_t  preamble;
    uint8_t  numDataBytes;
    uint16_t packetNumber;
    uint16_t numPackets;
    uint8_t  data[lora_numDataBytes_d];
} lora_dataPacket_t;

typedef struct
{
    uint8_t  preamble;
    uint16_t packetNumber;
    uint16_t numPackets;
    uint32_t crc;
} lora_footerPacket_t;
#pragma pack(pop)

// Function Prototypes
void                lora_init(void);
void                lora_deinit(void);

lora_headerPacket_t lora_createHeaderPacket(uint8_t* payload,
                                            uint16_t payloadSize);

lora_dataPacket_t   lora_createDataPacket(uint8_t* payload,
                                          uint16_t payloadIndex,
                                          uint16_t payloadSize);

lora_footerPacket_t lora_createFooterPacket(uint8_t* payload,
                                            uint16_t payloadSize);

void                lora_IRQProcess(void);
void                lora_queryState(void);

// Receiving is always done via the Rx Done callback.
void lora_setCallbacks(void (*onTxDone)(void),
                       void (*onRxDone)(uint8_t* payload,
                                        uint16_t size, int16_t rssi,
                                        int8_t snr),
                       void (*onTxTimeout)(void),
                       void (*onRxTimeout)(void),
                       void (*onRxError)(void));

// Blocking, prevents receiving until everything is sent.
void lora_send(uint8_t* payload, const uint16_t payloadSize);

// Internal callbacks exposed for the Implementation layer if needed
void _lora_backendRXDoneCallback(uint8_t* payload, uint16_t size,
                                 int16_t rssi, int8_t snr);
void _lora_backendTXDoneCallback(void);
void _lora_backendRXTimeoutCallback(void);
void _lora_backendTXTimeoutCallback(void);
void _lora_backendRXErrorCallback(void);

// These 3 functions confirm that the packets are in the correct format,
// and if so, will do their respective actions

// Set data received = true.
bool _lora_processFooterPacket(uint8_t* payload, uint16_t size);

// Append to data buffer as well as the above
bool _lora_processDataPacket(uint8_t* payload, uint16_t size);

// Reset and setup total packet counts, packet number, data buffer and
// data Received
// Will also start appending to data buffer.
bool _lora_processHeaderPacket(uint8_t* payload, uint16_t size);

#endif // shared_lora_h_INCLUDED
