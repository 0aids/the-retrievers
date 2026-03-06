#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

//#define CONSOLE    1

#if defined(CONSOLE)
#define CAM_UART_NUM        UART_NUM_0
#define UART_TX             (1)
#define UART_RX             (3)
#else
#define CAM_UART_NUM        UART_NUM_1
#define UART_TX             (13)
#define UART_RX             (12)
#endif
#define BUF_SIZE            (1024*2)
#define STACK_SIZE          (3072)
#define UART_BAUD_RATE      (115200)

#define UART_MESSAGE(message)      (uart_write_bytes(CAM_UART_NUM, message, strlen(message)))

void init_psat_uart(void);