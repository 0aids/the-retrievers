#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"


#define CONTROL_UART_NUM        (2)
#define CONTROL_TX_IO                   (17)
#define CONTROL_RX_IO                   (16)

#define CONSOLE_UART_NUM        (0)
#define CONSOLE_TX_IO                   (1)
#define CONSOLE_RX_IO                   (3)

#define UART_BAUD_RATE          (115200)

// Setup UART buffered IO with event queue
#define BUF_SIZE                (1024*2)

void camera_control_init(void);
void camera_console_init(void);
void camera_listen(void);
