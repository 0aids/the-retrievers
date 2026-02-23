#ifndef packets_h_INCLUDED
#define packets_h_INCLUDED
// Cross platform
#include "helpers.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <shared_state.h>
#include <shared_lora.h>

// Consider using malloc rather than using massive structs
#define loraFsm_defaultPacketSize_d 255

#define LORA_FSM_PACKET_TYPE_LIST                                    \
    X(loraFsm_packetType_empty)                                      \
    X(loraFsm_packetType_test)                                       \
    X(loraFsm_packetType_ack)                                        \
                                                                     \
    X(loraFsm_packetType_ping)                                       \
    X(loraFsm_packetType_pong)                                       \
                                                                     \
    X(loraFsm_packetType_gpsData)                                    \
    X(loraFsm_packetType_stateData)                                  \
    X(loraFsm_packetType_sensorData)                                 \
    X(loraFsm_packetType_preflightData)                              \
                                                                     \
    X(loraFsm_packetType_preflightReq)                               \
    X(loraFsm_packetType_preflightDataReq)                           \
    X(loraFsm_packetType_prelaunchCompleteReq)                       \
                                                                     \
    X(loraFsm_packetType_buzzShortReq)                               \
    X(loraFsm_packetType_buzzLongReq)                                \
                                                                     \
    X(loraFsm_packetType_fastForwardReq)                             \
    X(loraFsm_packetType_stateOverrideReq)                           \
                                                                     \
    X(loraFsm_packetType_enableComponentReq)                         \
    X(loraFsm_packetType_disableComponentReq)                        \
                                                                     \
    X(loraFsm_packetType_dataDumpReq)                                \
                                                                     \
    X(loraFsm_packetType__COUNT)

#define X(name) name,
typedef enum __attribute__((packed))
{
    LORA_FSM_PACKET_TYPE_LIST
} loraFsm_packetType_e;
#undef X

static inline const char*
loraFsm_packetTypeToString(loraFsm_packetType_e packetType)
{
    switch (packetType)
    {
#define X(packetType)                                                \
    case packetType: return #packetType; break;
        LORA_FSM_PACKET_TYPE_LIST
#undef X
        default: return "loraFsm_packetType_invalid";
    }
}

// This will be sent as a raw buffer, up to m_dataSize bytes.
typedef struct __attribute__((packed))
{
    loraFsm_packetType_e type;
    // clang-format off
    uint8_t data[ /*unknown length, entire struct is allcoated as helpers_malloced_t*/];
    // clang-format on
} loraFsm_packet_t;

typedef struct
{
    // treat ma.buffer as a "(loraFsm_Packet_t*)ma.buffer" to access members.
    helpers_malloced_t packetStorage;
    loraFsm_packet_t*  packetInterpreter;
    bool               wellFormed;
} loraFsm_packetWrapper_t;

// Create a packet.
loraFsm_packetWrapper_t
loraFsm_packetCreate(loraFsm_packetType_e type,
                     const uint8_t        dataBuffer_nma[],
                     const uint16_t       dataBufferLength);

// The size of the parsed packet is the entire size of the received buffer.
loraFsm_packetWrapper_t
     loraFsm_packetParse(const uint8_t payload_nma[], uint16_t size);

void loraFsm_packetSend(loraFsm_packetWrapper_t* packet);

bool loraFsm_packetFree(loraFsm_packetWrapper_t* packet);

#endif // packets_h_INCLUDED
