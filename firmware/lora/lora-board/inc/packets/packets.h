#ifndef packets_h_INCLUDED
#define packets_h_INCLUDED
// Cross platform
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// esp only
#include "sm_types.h"

#define d_defaultPacketBufferSize 256

typedef enum __attribute__((packed)) {
    EMPTY,
    PING,
    PONG,
    ACK,
    GPS_REQ,
    GPS_DATA,
    BUZ_REQ,     // Uses ACK for acknowledgement.
    FOLD_REQ,    // ^
    STATE_DATA,  // This fits
    TEST_TYPE,
} e_PacketType_t;

typedef enum __attribute__((packed)) {
    PRE_LAUNCH,
    POST_LAUNCH,
    POST_DEPLOY,
    POST_LAND,
} e_PSAT_States_t;

typedef struct {
    fsm_state_t state;
    bool gps_fix;
    uint16_t servo_angle;
} PSAT_State_Data_t;

// This will be sent as a raw buffer, up to m_dataSize bytes.
//
typedef struct {
    e_PacketType_t type;                      // 8 bits
    uint8_t data[d_defaultPacketBufferSize];  // OWNING, for receiving
    uint16_t m_dataSize;  // This is not part of the actual packet, this is just
                          // for sending.
    bool wellFormed;
} packet_t;

#define d_pingResponse()                               \
    {                                                  \
        packet_t packet = CreatePacket(PONG, NULL, 0); \
        delay_ms(500);                                 \
        Radio.Send((uint8_t*)&packet, 1);              \
        printf("Sent PONG response!\r\n");             \
        delay_ms(100);                                 \
    }

#define d_ackResponse()                               \
    {                                                 \
        packet_t packet = CreatePacket(ACK, NULL, 0); \
        Radio.Send((uint8_t*)&packet, 1);             \
        printf("Sent ACK response!\r\n");             \
        delay_ms(100);                                \
    }

// Ideally the data should be appended to the end of a buffer that already has
// the first byte free. In that case you should just set buffer[0] =
// packageHeaderpacket(...);
static inline packet_t CreatePacket(e_PacketType_t type,
                                    const uint8_t* const dataBuffer,
                                    const uint16_t dataBufferLength) {
    packet_t packet = {
        .type = type,
        .data = {0},
        .m_dataSize = 0,
        .wellFormed = false,
    };
    if (d_defaultPacketBufferSize < dataBufferLength) return packet;

    packet.wellFormed = true;
    // If length is 0 or a nullptr is passed
    if (dataBufferLength == 0 || dataBuffer == NULL) {
        return packet;
    }
    memcpy(packet.data, dataBuffer, dataBufferLength);
    packet.m_dataSize = dataBufferLength;

    return packet;
}

// The size of the parsed packet is the entire size of the received buffer.
static inline packet_t ParsePacket(uint8_t* payload, uint16_t size) {
    packet_t pack = {
        .type = payload[0],
        .data = {0},
        .m_dataSize = size - 1,
    };
    if (pack.m_dataSize <= 0) return pack;

    memcpy(pack.data, payload + 1, size - 1);
    return pack;
}

static inline const char* PacketTypeToString(e_PacketType_t type) {
    switch (type) {
        case PING:
            return "PING";
        case PONG:
            return "PONG";
        case ACK:
            return "ACK";
        case GPS_REQ:
            return "GPS_REQ";
        case GPS_DATA:
            return "GPS_SEND";
        case BUZ_REQ:
            return "BUZ_REQ";
        case FOLD_REQ:
            return "FOLD_REQ";
        case STATE_DATA:
            return "STATE_SEND";
        default:
            return "UNKNOWN_PACKET_TYPE";
    }
}

static inline const char* PSATStateToString(e_PSAT_States_t state) {
    switch (state) {
        case PRE_LAUNCH:
            return "PRE_LAUNCH";
        case POST_LAUNCH:
            return "POST_LAUNCH";
        case POST_DEPLOY:
            return "POST_DEPLOY";
        case POST_LAND:
            return "POST_LAND";
        default:
            return "UNKNOWN_PSAT_STATE";
    }
}

static inline void printPacketStats(packet_t* packet) {
    printf("Received: %s\r\n", PacketTypeToString(packet->type));
    // printf("Time on air: %u ms\r\n", gr_RadioGetTimeOnAir(packet));
    // printf("Signal strength: %d dbm\r\n", gr_RadioGetRSSI());
    printf("Total packet size including header: %d\r\n",
           packet->m_dataSize + 1);
}

#endif  // packets_h_INCLUDED
