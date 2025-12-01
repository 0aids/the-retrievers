#ifndef packets_h_INCLUDED
#define packets_h_INCLUDED
// Cross platform  
#include <stdint.h>
#include <stdbool.h>

#define d_defaultPacketBufferSize 256

typedef enum __attribute__((packed)) {
    EMPTY,
    PING,
    PONG,
    ACK,
    GPS_REQ,
    GPS_DATA,
    BUZ_REQ, // Uses ACK for acknowledgement.
    FOLD_REQ, // ^
    STATE_DATA, // This fits
} e_PacketType_t;

typedef enum __attribute__((packed)) {
    PRE_LAUNCH,
    POST_LAUNCH,
    POST_DEPLOY,
    POST_LAND,
} e_PSAT_States_t;


// This will be sent as a raw buffer, up to m_dataSize bytes.
// 
typedef struct {
    e_PacketType_t type; // 8 bits
    uint8_t data[d_defaultPacketBufferSize]; // OWNING, for receiving
    uint16_t m_dataSize; // This is not part of the actual packet, this is just for sending.
    bool wellFormed;
} packet_t;

const char* PacketTypeToString(e_PacketType_t type);

const char* PSATStateToString(e_PSAT_States_t state);

packet_t CreatePacket(e_PacketType_t type, const uint8_t * const buffer, const uint16_t bufferSize);

// The size is the total size received, not just the data.
packet_t ParsePacket(uint8_t *payload, uint16_t size);

void printPacketStats(packet_t *packet);

#define d_pingResponse() \
    { \
    packet_t packet = CreatePacket(PONG, NULL, 0); \
    Radio.Send((uint8_t*) &packet, 1); \
    printf("Sent PONG response!\r\n"); \
    delay_ms(100); \
    }

#define d_ackResponse() \
    { \
    packet_t packet = CreatePacket(ACK, NULL, 0) ; \
    Radio.Send((uint8_t*) &packet, 1); \
    printf("Sent ACK response!\r\n"); \
    delay_ms(100); \
    } 

#endif // packets_h_INCLUDED
