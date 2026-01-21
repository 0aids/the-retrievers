#include "packets.h"

loraFsm_packet_t loraFsm_createPacket(loraFsm_packetType_e type,
                                    const uint8_t* const dataBuffer,
                                    const uint16_t dataBufferLength) {
    loraFsm_packet_t packet = {
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
loraFsm_packet_t loraFsm_packetParse(uint8_t* payload, uint16_t size) {
    loraFsm_packet_t pack = {
        .type = payload[0],
        .data = {0},
        .m_dataSize = size - 1,
    };
    if (pack.m_dataSize <= 0) return pack;

    memcpy(pack.data, payload + 1, size - 1);
    return pack;
}


const char* loraFsm_packetTypeToStr(loraFsm_packetType_e type) {
    switch (type) {
        case loraFsm_packetType_ping:
            return "PING";
        case loraFsm_packetType_pong:
            return "PONG";
        case loraFsm_packetType_ack:
            return "ACK";
        case loraFsm_packetType_gpsData:
            return "GPS_SEND";
        case loraFsm_packetType_buzReq:
            return "BUZ_REQ";
        case loraFsm_packetType_stateData:
            return "STATE_SEND";
        default:
            return "UNKNOWN_PACKET_TYPE";
    }
}

void loraFSM_printPacketStats(loraFsm_packet_t* packet) {
    printf("Received: %s\r\n", loraFsm_packetTypeToStr(packet->type));
    // printf("Time on air: %u ms\r\n", gr_RadioGetTimeOnAir(packet));
    // printf("Signal strength: %d dbm\r\n", gr_RadioGetRSSI());
    printf("Total packet size including header: %d\r\n",
           packet->m_dataSize + 1);
}
