#ifndef shared_lora_h_INCLUDED
#define shared_lora_h_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "loraImpl.h"

// --------------------------------------------------------------------------
// Lora Protocol Layer
// --------------------------------------------------------------------------

#define lora_numDataBytes_d ((uint8_t)96)

// Payloads can now be up to 1mb in size, but realistically we'll only allocate
// up to ~2kb and check for bounds
// NOTE: the asr6601CB that the ai-thinker uses has 16kb sram.
// The esp32-s3-wroom-1u-n16r8 has 8mb of ram, 16mb flash.
#define lora_numBufferBytes_d ((uint16_t)(3072))

typedef enum
{
    lora_rxStates_waitingForHeader,
    lora_rxStates_processingHeader,
    lora_rxStates_waitingForDataOrFooter,
    lora_rxStates_processingData,
    lora_rxStates_processFooter,
    lora_rxStates_rxTimeout,
    lora_rxStates_rxError,
    lora_rxStates_rxDone,
} lora_rxStates_e;

typedef enum
{
    lora_rxErrorTypes_none = 0,
    lora_rxErrorTypes_failedToAppend,
    lora_rxErrorTypes_spuriousError, // If the hal encounters an error.
    lora_rxErrorTypes_firstPacketWasNotHeader,
    lora_rxErrorTypes_headerPacketNumberIsNot1,
    lora_rxErrorTypes_headerPacketSizeMismatch,
    lora_rxErrorTypes_dataPacketSizeMismatch,
    lora_rxErrorTypes_dataPacketNumPacketsMismatch,
    lora_rxErrorTypes_dataPacketNotConsecutive,
    lora_rxErrorTypes_footerPacketSizeMismatch,
    lora_rxErrorTypes_footerPacketNumPacketsMismatch,
    lora_rxErrorTypes_footerPacketIsNotLast,
    lora_rxErrorTypes_footerPacketIsNotConsecutive,
    lora_rxErrorTypes_expectedDataOrFooterGotUnknown,
} lora_rxErrorTypes_e;

// This is for the collection of packets (frames)
typedef struct
{
    // When a frame is sent
    void (*onTxDone)(void);
    // When a frame is received
    void (*onRxDone)(uint8_t* payload, uint16_t size, int16_t rssi,
                     int8_t snr);
    // When a frame timeouts/fails to send
    void (*onTxTimeout)(void);
    // When a frame fails to be received due to inter-packet delay being too long
    // or just no receiving any more packets unexpectedly.
    void (*onRxTimeout)(void);
    // When a frame fails to be received due to some drastic error.
    void (*onRxError)(void);

    // Internal metrics for keeping track of packets during construction of the frame.
    uint16_t                      currentTotalPacketCount;
    uint16_t                      currentPacketNumber;
    volatile lora_rxStates_e      rxState;

    // Same as what's given by the implementation.
    volatile int16_t backendRssi;
    volatile int8_t  backendSnr;

    // Stuff for ensuring callbacks are called
    // These are only set to true via callbacks to the implementation
    // and false whenever we start trying to recv or send.
    volatile bool backendRxDone;
    volatile bool backendTxDone;
    volatile bool backendRxError;
    volatile bool backendRxTimeout;
    volatile bool backendTxTimeout;

    // Is true when the databuffer is fully formed
    bool     dataReceived;
    uint16_t dataCurrentContentSize;
    // This needs to be allocated some way or another. 
    uint8_t  *dataBuffer;
    lora_rxErrorTypes_e errorType;
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

void                lora_irqProcess(void);
void                lora_queryState(void);

void lora_setRx(uint16_t ms);

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
void _lora_backendRxDoneCallback(uint8_t* payload, uint16_t size,
                                 int16_t rssi, int8_t snr);
void _lora_backendTxDoneCallback(void);
void _lora_backendRxTimeoutCallback(void);
void _lora_backendTxTimeoutCallback(void);
void _lora_backendRxErrorCallback(void);

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
