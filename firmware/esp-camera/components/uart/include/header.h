#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

const uart_port_t uart_num = UART_NUM_0;
// Setup UART buffered IO with event queue
const int uart_buffer_size = (1024 * 2);

void camera_uart_init(void);
void call_camera(void);
void camera_uart_deinit(void);

#define PORT_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)