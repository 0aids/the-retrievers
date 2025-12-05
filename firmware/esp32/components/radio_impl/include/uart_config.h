#ifndef uart_config_h_INCLUDED
#define uart_config_h_INCLUDED

#include <string.h>
#include "packets/packets.h"
#include <stdio.h>
#include <stdint.h>

// Packet style
// byte 1  -> Enum of request
// byte 2+ -> data bytes if relevant
// Who cares about error checking

// =================================================
// ============= Enum for Request Type =============
// =================================================

#define grReq_XMacro \
    X(RadioSend) \
    X(RadioCheckRecv) \
    X(RadioSetRx) \
    X(RadioSetIdle) \
    X(RadioGetStatus) \
    X(RadioGetRSSI) \
    X(RadioGetTimeOnAir) \
    X(_RadioRequestRxPacket) \
    X(Invalid) \
// _RadioRequestRxPacket is for retrieving the radio data.

#define X(name) grReq_##name,
typedef enum __attribute__((packed)) {
    grReq_XMacro
} e_grRequest;
#undef X

static inline e_grRequest grReq_StringToEnum(const char *str)
{
    #define X(name) if (strcmp(#name, str) == 0) return grReq_##name;
    grReq_XMacro
    #undef X
    return grReq_Invalid;
}

static inline const char* grRequestToString(e_grRequest req)
{
    switch (req) {
        #define X(name) case grReq_##name: return "grReq_" #name;
        grReq_XMacro
        #undef X
        default: return "Unknown_grRequest";
    }
}

static inline void grReq_printAllTypes(void)
{
    printf("All types:\r\n");
    #define X(name) printf(#name "\r\n");
    grReq_XMacro
    #undef X
}

// =====================================================
// ============= Inter-Board communication =============
// =====================================================
// Board to board communication over uart. packets have a small
// 1 byte extra header which is just the instructions to do
// with the data.

// It needs to own the data you absolute monkey otherwise you cannot submit it!!!
typedef struct {
    e_grRequest requestType;
    uint8_t dataBuffer[d_defaultPacketBufferSize];
    uint16_t m_dataBufferSize;
} espToLoraPacket_t;

static inline espToLoraPacket_t EspToLoraPacket_Create(
    e_grRequest requestType,
    uint8_t *dataBuffer,
    uint16_t dataBufferSize
)
{
    espToLoraPacket_t ret = {
        .requestType = requestType,
        .m_dataBufferSize = dataBufferSize
    };
    memcpy(ret.dataBuffer, dataBuffer, dataBufferSize);
    return ret;
}

// Converts an array of bytes (including that data bytes) into a
// espToLoraPacket_t.
static inline espToLoraPacket_t EspToLoraPacket_CreateFromBuffer(uint8_t* const buffer, uint16_t bufferSize)
{
    if (bufferSize == 0 || buffer == NULL) {
        return EspToLoraPacket_Create(grReq_Invalid, NULL, 0);
    }
    else if (bufferSize == 1 &&  buffer != NULL) {
        return EspToLoraPacket_Create(buffer[0], NULL, 0);
    }
    else {
        return EspToLoraPacket_Create(buffer[0], &buffer[1], bufferSize - 1);
    }
}

static inline void EspToLoraPacket_PrintPacketStats(const espToLoraPacket_t * const packet) {
    printf("Request type: %s\r\n", grRequestToString(packet->requestType));
    printf("Data Buffer size: %d\r\n", packet->m_dataBufferSize);
}

// AHHH this is the fucking 1000000th packet format I've had to write!!!!
typedef struct {
    int16_t rssi;
    int8_t snr;
    uint8_t data[d_defaultPacketBufferSize];
    uint16_t m_dataLength; // The m_ prefix indicates that this data is not sent / can be inferred.
} gr_RxDonePacket_t;

static inline void gr_RxDonePacket_PrintPacketStats(const gr_RxDonePacket_t * const packet) {
    printf("Rssi: %d\r\n", packet->rssi);
    printf("Snr: %d\r\n", packet->snr);
    printf("Data length: %d\r\n", packet->m_dataLength);
}

// This is a bitmask of the irqs that might need to be handled.
// These are sent back.
typedef enum __attribute__((packed)) {
    gr_irqs_None = 0,
    gr_irqs_RxDone = 1 << 0,
    gr_irqs_TxDone = 1 << 1,
    gr_irqs_RxTimeout = 1 << 2,
    gr_irqs_TxTimeout = 1 << 3,
    gr_irqs_RxError = 1 << 4,
} gr_irqsToHandle_t;

#define gr_irqs_PrintAssociatedTypes(irqs)\
{\
    if (irqs == gr_irqs_None) {\
        verbPrintf("gr_irqs_None\r\n");\
        return;\
    }\
    if (irqs & gr_irqs_RxDone)\
    {\
        verbPrintf("gr_irqs_RxDone\r\n");\
    }\
    if (irqs & gr_irqs_TxDone)\
    {\
        verbPrintf("gr_irqs_TxDone\r\n");\
    }\
    if (irqs & gr_irqs_RxTimeout)\
    {\
        verbPrintf("gr_irqs_RxTimeout\r\n");\
    }\
    if (irqs & gr_irqs_TxTimeout)\
    {\
        verbPrintf("gr_irqs_TxTimeout\r\n");\
    }\
    if (irqs & gr_irqs_RxError)\
    {\
        verbPrintf("gr_irqs_RxError\r\n");\
    }\
}

// =======================================
// ============= Definitions =============
// =======================================

#ifdef ESP_PLATFORM
#include "driver/uart.h"
// Setup radio on a specified uart port, defined through macro
// If manually specified wil use those instead.
#ifndef d_gr_radioRxPort

#define d_gr_uartPort UART_NUM_2
#define d_gr_radioTxPort 17
#define d_gr_radioRxPort 16
#endif

// Really fucking slow baud rate because otherwise the lora will not receive most bits.
#define d_gr_uartBaudRate 2400
#define d_gr_uartDataBits UART_DATA_8_BITS
#define d_gr_uartParityMode UART_PARITY_DISABLE
#define d_gr_uartStopBits UART_STOP_BITS_1
#define d_gr_uartFlowCtrlMode UART_HW_FLOWCTRL_DISABLE
#define d_gr_uartFlowCtrlThresh 122

#elifdef d_aiThinkerPlatform

#include "tremo_uart.h"
#define d_gr_uartPort UART1
#define d_gr_radioTxPort GPIO_PIN_4
#define d_gr_radioRxPort GPIO_PIN_5
#define d_gr_radioGpioPort GPIOA

#define d_gr_uartBaudRate UART_BAUDRATE_2400
#define d_gr_uartDataBits UART_DATA_WIDTH_8
#define d_gr_uartParityMode UART_PARITY_NO
#define d_gr_uartStopBits UART_STOP_BITS_1
// #define d_gr_uartFlowCtrlMode UART_HW_FLOWCTRL_CTS_RTS
// #define d_gr_uartFlowCtrlThresh 122

#else

#error "d_aiThinkerPlatform or ESP_PLATFORM is _NOT_ defined!!!"

#endif

#endif // uart_config_h_INCLUDED
