#ifndef uart_config_h_INCLUDED
#define uart_config_h_INCLUDED

#include <string.h>
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
    X(Invalid) \


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

typedef struct {
    e_grRequest requestType;
    uint8_t * dataBuffer;
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
        .dataBuffer = dataBuffer,
        .m_dataBufferSize = dataBufferSize
    };
    return ret;
}

// Converts an array of bytes (including that data bytes) into a
// espToLoraPacket_t. DOES NOT OWN IT'S DATA BUFFER!!!!!!!
static inline espToLoraPacket_t EspToLoraPacket_CreateFromBuffer(uint8_t* const buffer, uint16_t bufferSize)
{
    if (bufferSize == 0 || buffer == NULL) {
        return EspToLoraPacket_Create(grReq_Invalid, NULL, 0);
    }
    else if (bufferSize == 1 || buffer != NULL) {
        return EspToLoraPacket_Create(buffer[0], NULL, 0);
    }
    else {
        return EspToLoraPacket_Create(buffer[0], buffer + 1, bufferSize - 1);
    }
}

static inline void EspToLoraPacket_PrintPacketStats(const espToLoraPacket_t *packet) {
    printf("Request type: %s\r\n", grRequestToString(packet->requestType));
    printf("Data Buffer size: %d\r\n", packet->m_dataBufferSize);
}

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
