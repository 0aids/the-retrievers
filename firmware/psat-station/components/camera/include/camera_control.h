#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"


#define CONTROL_UART_NUM        UART_NUM_2
#define TX_IO                   17
#define RX_IO                   16

// Setup UART buffered IO with event queue
#define BUF_SIZE                (1024*2)

void camera_control_init(void);
void camera_init(void);
void take_pics(void);
void camera_deinit(void);
void camera_control_deinit(void);
void camera_listen(void);
