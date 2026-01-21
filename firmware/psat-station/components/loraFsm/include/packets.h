#ifndef packets_h_INCLUDED
#define packets_h_INCLUDED
// Cross platform
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <shared_state.h>

// Consider using malloc rather than using massive structs
#define loraFsm_defaultPacketSize_d 255

typedef enum __attribute__((packed)) {
    loraFsm_packetType_empty,
    loraFsm_packetType_ping,
    loraFsm_packetType_pong,
    loraFsm_packetType_ack,
    loraFsm_packetType_gpsData,
    loraFsm_packetType_buzReq,     // Uses ACK for acknowledgement.
    loraFsm_packetType_stateData,  // This fits
    loraFsm_packetType_fastForwardReq,
    loraFsm_packetType_stateOverrideReq,  
    loraFsm_packetType_stateDumpReq,  
    loraFsm_packetType_test,
} loraFsm_packetType_e;

// This will be sent as a raw buffer, up to m_dataSize bytes.
typedef struct __attribute__((packed)){
    loraFsm_packetType_e type;                      // 8 bits
    uint8_t data[loraFsm_defaultPacketSize_d];  // OWNING, for receiving
    uint16_t m_dataSize;  // This is not part of the actual packet, this is just
                          // for sending.
    bool wellFormed;
} loraFsm_packet_t;

// Create a packet. 
loraFsm_packet_t loraFsm_createPacket(loraFsm_packetType_e type,
                                    const uint8_t* const dataBuffer,
                                    const uint16_t dataBufferLength);

// The size of the parsed packet is the entire size of the received buffer.
loraFsm_packet_t loraFsm_packetParse(uint8_t* payload, uint16_t size);

const char* loraFsm_packetTypeToStr(loraFsm_packetType_e type);

void loraFsm_printPacketStats(loraFsm_packet_t* packet); 

#endif  // packets_h_INCLUDED

