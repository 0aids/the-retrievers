#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_parser.h"
#include "gps_state.h"

#define GPS_UART_NUM UART_NUM_1
#define GPS_UART_BAUD 9600

#define GPS_TASK_PRIORITY \
    5  // gps only is at 1 time per second so no need give priority
#define GPS_TASK_STACK_SIZE 4096

#define GPS_TX_PIN 27
#define GPS_RX_PIN 25
#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 0  // cause we arent sending anything to gps

#define GPS_BUFFER_SIZE 512  // aint no way we hitting the max
#define TIMEOUT 50           // how long we wait for uart data in ms

// buffer and buffer position
static char gps_buffer[GPS_BUFFER_SIZE];
static int gps_buffer_pos = 0;

void gps_task(void* arg) {
    uint8_t data[64];

    while (1) {
        int len = uart_read_bytes(GPS_UART_NUM, data, sizeof(data),
                                  TIMEOUT / portTICK_PERIOD_MS);

        // go through each byte we just read and add it to the buffer
        for (int i = 0; i < len; i++) {
            char c = data[i];
            if (c == '\r') continue;

            if (gps_buffer_pos < GPS_BUFFER_SIZE - 1)
                gps_buffer[gps_buffer_pos++] = c;

            if (c == '\n') {  // when we reach new line, we know a sentence is
                              // complete
                gps_buffer[gps_buffer_pos] = '\0';
                gps_buffer_pos = 0;  // reset buffer pos to start

                gps_process_nmea(gps_buffer);
            }
        }
    }
}

void gps_init() {
    uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(GPS_UART_NUM, &uart_config);

    uart_set_pin(GPS_UART_NUM, GPS_TX_PIN, GPS_RX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    uart_driver_install(GPS_UART_NUM, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 0, NULL,
                        0);

    gps_state_init();
}

void gps_start() {
    xTaskCreate(gps_task, "gps_task", GPS_TASK_STACK_SIZE, NULL,
                GPS_TASK_PRIORITY, NULL);
}