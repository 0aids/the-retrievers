#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "pin_config.h"


#define UART_BAUD_RATE          (115200)
#define BUF_SIZE                (1024*2)

#define UART_MESSAGE(message)      (uart_write_bytes(CAMERA_UART_NUM, message, strlen(message)))
#define LOG_DATA(data) do { \
    char buf[256]; \
    int len = snprintf(buf, sizeof(buf), "++LOG++\n\n%s\n\n", data); \
    if (len > 0) { \
        uart_write_bytes(CONTROL_UART_NUM, buf, len); \
    } \
} while(0)

#define STACK_SIZE  (3072)

void camera_control_init(void);
void uart_read(void *pvParameters);
void test(void *pvParameters);
