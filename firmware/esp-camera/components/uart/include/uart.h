#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"


#define CAM_UART_NUM        UART_NUM_0
#define UART_TX             (1)
#define UART_RX             (3)
#define BUF_SIZE            (1024*2)
#define STACK_SIZE          (3072)
#define UART_BAUD_RATE      (115200)

void init_uart(void);