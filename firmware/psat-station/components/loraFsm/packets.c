#include "packets.h"
#include "helpers.h"
#include <esp_log.h>

inline static bool
_loraFsm_copyData(loraFsm_packetWrapper_t* packetWrapped,
                  const uint8_t* const     dataBuffer_nma,
                  const uint16_t           dataBufferLength)
{
    if (!helpers_smartAlloc(&packetWrapped->packetStorage,
                            dataBufferLength + 1))
    {
        return false;
    }
    memcpy(packetWrapped->packetStorage.buffer + 1, dataBuffer_nma,
           dataBufferLength);
    return true;
}

loraFsm_packetWrapper_t
loraFsm_packetCreate(loraFsm_packetType_e type,
                     const uint8_t* const dataBuffer_nma,
                     const uint16_t       dataBufferLength)
{
    loraFsm_packetWrapper_t packetWrapped = {0};
    packetWrapped.wellFormed              = false;
    if (!_loraFsm_copyData(&packetWrapped, dataBuffer_nma,
                           dataBufferLength))
    {
        ESP_LOGE(__FUNCTION__,
                 "Failed to allocate and copy data to packet!");
        return packetWrapped;
    }
    packetWrapped.packetInterpreter =
        (loraFsm_packet_t*)packetWrapped.packetStorage.buffer;
    packetWrapped.packetStorage.buffer[0] = type;
    packetWrapped.wellFormed              = true;
    return packetWrapped;
}

// The size of the parsed packet is the entire size of the received buffer.
loraFsm_packetWrapper_t
loraFsm_packetParse(const uint8_t* const payload_nma, uint16_t size)
{
    loraFsm_packetWrapper_t packetWrapped = {0};
    packetWrapped.wellFormed              = false;
    if (!_loraFsm_copyData(&packetWrapped, payload_nma + 1, size - 1))
    {
        ESP_LOGE(__FUNCTION__,
                 "Failed to allocate and copy data to packet!");
        return packetWrapped;
    }
    packetWrapped.packetStorage.buffer[0] = payload_nma[0];
    packetWrapped.packetInterpreter =
        (loraFsm_packet_t*)packetWrapped.packetStorage.buffer;
    packetWrapped.wellFormed = true;
    return packetWrapped;
}

void loraFsm_packetSend(loraFsm_packetWrapper_t* packet)
{
    if (!packet->wellFormed)
    {
        ESP_LOGE(__FUNCTION__,
                 "Unable to send packet! Packet is not well formed!");
        return;
    }
    lora_send(packet->packetStorage.buffer,
              packet->packetStorage.bufferSize);
}

bool loraFsm_packetFree(loraFsm_packetWrapper_t* packet)
{
    if (!packet->wellFormed)
    {
        ESP_LOGE(__FUNCTION__,
                 "Unable free packet! Packet is not well formed!");
        return false;
    }
    packet->packetInterpreter = NULL;
    return helpers_free(&packet->packetStorage);
}

const char* loraFsm_packetTypeToStr(loraFsm_packetType_e type)
{
    switch (type)
    {
        case loraFsm_packetType_ping: return "PING";
        case loraFsm_packetType_pong: return "PONG";
        case loraFsm_packetType_ack: return "ACK";
        case loraFsm_packetType_gpsData: return "GPS_SEND";
        case loraFsm_packetType_buzReq: return "BUZ_REQ";
        case loraFsm_packetType_stateData: return "STATE_SEND";
        default: return "UNKNOWN_PACKET_TYPE";
    }
}
