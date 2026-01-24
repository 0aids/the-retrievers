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

// clang-format off
#define allError_xmacro \ 
    X(ldrErr_none) \
    X(ldrErr_calibrationInitErr) \
    X(ldrErr_adcInitErr) \
    X(ldrErr_adcConfig) \
    X(ldrErr_oneshotRead) \
    X(ldrErr_voltageCali) \
    X(ldrErr_OpenMemStream) \
    X(ldrErr_adcDel) \
    X(ldrErr_caliDel) \
// clang-format on


typedef enum {
#define X(name) name,
    allError_xmacro
#undef X
} psatErrStates_e;


void printErrorType(psatErrStates_e err)
{
    extern void printf(const char* fmt, ...);

    switch (err) {
        #define X(errType) \
            case errType: \
                printf("Error type: %s\n", #errType);  \
                break; \
        allError_xmacro
        #undef X
    }
}

#endif
