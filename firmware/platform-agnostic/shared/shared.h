#ifndef commands_h_INCLUDED
#define commands_h_INCLUDED

// Shared commands

typedef char lora_uint8_t;
// Shared commands
typedef enum {
    lora_packetTypes_ping,
    lora_packetTypes_pong,

    lora_packetTypes_statusReq,
    lora_packetTypes_statusData,

    lora_packetTypes_gpsReq,
    lora_packetTypes_gpsData,

    lora_packetTypes_buzReq,
    lora_packetTypes_buzAck,

    // For preflight checks and onthefly debugging
    lora_packetTypes_testReq,
    lora_packetTypes_testData,
} lora_packetTypes_e;


typedef enum {
    ldrErr_none,
    ldrErr_eFuseNotBurnt,
    ldrErr_caliInit,
    ldrErr_adcInit,
    ldrErr_adcConfig,
    ldrErr_oneshotRead,
    ldrErr_voltageCali,
    ldrErr_OpenMemStream,
    ldrErr_adcDel,
    ldrErr_caliDel
} psatErrStates_e;

#endif
