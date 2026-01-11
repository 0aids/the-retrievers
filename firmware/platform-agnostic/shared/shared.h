#ifndef commands_h_INCLUDED
#define commands_h_INCLUDED

// Shared commands

typedef struct {
} GpsData_t;

typedef struct {
} TestData_t;

typedef struct {
} StatusData_t;

typedef enum {
} psat_mainState_e;

typedef char lora_uint8_t;
// Shared commands
typedef enum {
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
} lora_PacketTypes_e;

#endif
