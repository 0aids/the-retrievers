#include "packets/packets.h"
#include "global_radio.h"
#include <string.h>
#include "stddef.h"
#include "stdbool.h"
#include "stdio.h"


// Ideally the data should be appended to the end of a buffer that already has the first
// byte free. In that case you should just set buffer[0] = packageHeaderpacket(...);
packet_t CreatePacket(e_PacketType_t type, const uint8_t * const dataBuffer, const uint16_t dataBufferLength)
{
    packet_t packet = {
        .type = type,
        .data = {0},
        .m_dataSize = 0,
        .wellFormed = false,
    };
    if (d_defaultPacketBufferSize < dataBufferLength)
        return packet;

    packet.wellFormed = true;
    // If length is 0 or a nullptr is passed
    if (dataBufferLength == 0 || dataBuffer == NULL)
    {
        return packet;
    }
    memcpy(packet.data, dataBuffer, dataBufferLength);
    packet.m_dataSize = dataBufferLength;

    return packet;
}

packet_t ParsePacket(uint8_t *payload, uint16_t size) {
    packet_t pack = {
        .type = payload[0],
        .data = {0},
        .m_dataSize = size - 1,
    };
    if (pack.m_dataSize <= 0) return pack;

    memcpy(pack.data, payload + 1, size - 1);
    return pack;
}

const char* PacketTypeToString(e_PacketType_t type) {
    switch (type) {
        case PING:        return "PING";
        case PONG:        return "PONG";
        case ACK:         return "ACK";
        case GPS_REQ:     return "GPS_REQ";
        case GPS_DATA:    return "GPS_SEND";
        case BUZ_REQ:     return "BUZ_REQ";
        case FOLD_REQ:    return "FOLD_REQ";
        case STATE_DATA:  return "STATE_SEND";
        default:          return "UNKNOWN_PACKET_TYPE";
    }
}

const char* PSATStateToString(e_PSAT_States_t state) {
    switch (state) {
        case PRE_LAUNCH:  return "PRE_LAUNCH";
        case POST_LAUNCH: return "POST_LAUNCH";
        case POST_DEPLOY: return "POST_DEPLOY";
        case POST_LAND:   return "POST_LAND";
        default:          return "UNKNOWN_PSAT_STATE";
    }
}

void printPacketStats(packet_t *packet)
{
    printf("Received: %s\r\n", PacketTypeToString(packet->type));
    printf("Time on air: %u ms\r\n", gr_RadioGetTimeOnAir(packet));
    printf("Signal strength: %d dbm\r\n", gr_RadioGetRSSI());
    printf("Total packet size including header: %d\r\n", packet->m_dataSize + 1);
}
