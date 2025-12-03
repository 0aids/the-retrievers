#ifndef uart_config_h_INCLUDED
#define uart_config_h_INCLUDED

// Packet style
// byte 1 -> Enum of request
// byte 2 and 3 -> Number of data bytes if relevant
// byte 4 and 4+ -> data bytes if relevant
// Who cares about error checking

#define grReq_XMacro \
    X(RadioSend) \
    X(RadioCheckRecv) \
    X(RadioSetRx) \
    X(RadioSetIdle) \
    X(RadioGetStatus) \
    X(RadioGetRSSI) \
    X(RadioGetTimeOnAir) \


#define X(name) grReq_##name,
typedef enum __attribute__((packed)) {
    grReq_XMacro
} e_grRequest;
#undef X


#ifdef ESP_PLATFORM
#include "driver/uart.h"
// Setup radio on a specified uart port, defined through macro
// If manually specified wil use those instead.
#ifndef d_gr_radioRxPort

#define d_gr_uartPort UART_NUM_2
#define d_gr_radioTxPort 17
#define d_gr_radioRxPort 16
#endif

#define d_gr_uartBaudRate 115200
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

#define d_gr_uartBaudRate UART_BAUDRATE_115200
#define d_gr_uartDataBits UART_DATA_WIDTH_8
#define d_gr_uartParityMode UART_PARITY_NO
#define d_gr_uartStopBits UART_STOP_BITS_1
// #define d_gr_uartFlowCtrlMode UART_HW_FLOWCTRL_CTS_RTS
// #define d_gr_uartFlowCtrlThresh 122

#else

#error "d_aiThinkerPlatform or ESP_PLATFORM is _NOT_ defined!!!"

#endif


#endif // uart_config_h_INCLUDED
