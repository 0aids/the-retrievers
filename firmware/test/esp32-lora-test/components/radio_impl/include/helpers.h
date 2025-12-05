#ifndef helpers_h_INCLUDED
#define helpers_h_INCLUDED
#include "driver/uart.h"
#include <stdio.h>
#define d_uartTxTimeout_ms 500
#define d_uartAckTimeout_ms 100
// Blocking transmit buffer over uart
void blockingTransmitBuffer(uart_port_t uartPort, const uint8_t* const buffer, uint16_t bufferLength);

// Tries to get the ack from the uart
// Checks by comparing it to the number received (which is the number of bytes the
//                                                lora received)
bool waitForAck(uart_port_t uartPort, uint16_t timeout_ms, uint8_t numBytesSent);

extern bool logging_verbose;
#define verbPrintf(...) \
    if (logging_verbose) { \
        printf(__VA_ARGS__); \
    }

#endif // helpers_h_INCLUDED
