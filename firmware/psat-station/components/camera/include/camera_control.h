#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"


#define CONTROL_UART_NUM        (2)
#define CONTROL_TX_IO           (17)
#define CONTROL_RX_IO           (16)

#define UART_BAUD_RATE          (115200)
#define BUF_SIZE                (1024*2)

#define UART_MESSAGE(message)      (uart_write_bytes(CONTROL_UART_NUM, message, strlen(message)))


void camera_control_init(void);
void uart_read(void *pvParameters);
