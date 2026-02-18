#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"

const uart_port_t uart_num = UART_NUM_2;
// Setup UART buffered IO with event queue
const int uart_buffer_size = (1024 * 2);

void camera_control_init(void);
void call_camera(void);
void camera_control_deinit(void);
