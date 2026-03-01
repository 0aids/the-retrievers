#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "pin_config.h"


#define UART_BAUD_RATE          (115200)
#define BUF_SIZE                (1024*2)

#define CAMERA_MESSAGE(message)      (uart_write_bytes(CAMERA_UART_NUM, message, sizeof(message)))
#define LOG_DATA(data)             

#define STACK_SIZE  (3072)

void camera_control_init(void);
void camera_read(void *pvParameters);
void test(void *pvParameters);
