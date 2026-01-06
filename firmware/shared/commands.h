#ifndef commands_h_INCLUDED
#define commands_h_INCLUDED

#define lora_defaultDataSize_d 64
typedef char lora_uint8_t;
// Shared commands
enum lora_PacketTypes_e {
    lora_PacketTypes_Ping,
    lora_PacketTypes_Pong,

    lora_PacketTypes_StatusReq,
    lora_PacketTypes_StatusData,

    lora_PacketTypes_GpsReq,
    lora_PacketTypes_GpsData,

    lora_PacketTypes_BuzReq,
    lora_PacketTypes_BuzAck,

    // For preflight checks and onthefly debugging
    lora_PacketTypes_TestReq,
    lora_PacketTypes_TestData,
};

typedef struct {
} GpsData_t;

typedef struct {
} TestData_t;

typedef struct {
} StatusData_t;

typedef struct {
    lora_PacketTypes_e type;
    lora_uint8_t dataLength;
    lora_uint8_t data[lora_defaultDataSize_d];
} lora_Packet_t;

#endif
