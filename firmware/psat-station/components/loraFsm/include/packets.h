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

typedef enum __attribute__((packed))
{
    loraFsm_packetType_empty,
    loraFsm_packetType_ping,
    loraFsm_packetType_pong,
    loraFsm_packetType_ack,
    loraFsm_packetType_gpsData,
    loraFsm_packetType_buzReq,    // Uses ACK for acknowledgement.
    loraFsm_packetType_stateData, // This fits
    loraFsm_packetType_fastForwardReq,
    loraFsm_packetType_stateOverrideReq,
    loraFsm_packetType_stateDumpReq,
    loraFsm_packetType_test,
    loraFsm_packetType_prelaunch,
    loraFsm_packetType_landing,
} loraFsm_packetType_e;

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
    helpers_malloced_t    packetStorage;
    loraFsm_packet_t* packetInterpreter;
    bool wellFormed;
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

const char* loraFsm_packetTypeToStr(loraFsm_packetType_e type);

#endif // packets_h_INCLUDED

