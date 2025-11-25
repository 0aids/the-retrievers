#include <stdio.h>
#include <string.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps.h"
#include "minmea.h"

#define GPS_UART_NUM UART_NUM_1
#define GPS_TX_PIN 27
#define GPS_RX_PIN 25
#define GPS_UART_BAUD 9600
#define GPS_BUFFER_SIZE 512
#define TIMEOUT 50  // how long we wait for uart data in ms

static char gps_buffer[GPS_BUFFER_SIZE];
static int gps_pos = 0;

void gps_task(void* arg) {
    uint8_t data[64];

    while (1) {
        int len = uart_read_bytes(GPS_UART_NUM, data, sizeof(data),
                                  TIMEOUT / portTICK_PERIOD_MS);

        for (int i = 0; i < len; i++) {
            char c = data[i];
            if (c == '\r') continue;

            if (gps_pos < GPS_BUFFER_SIZE - 1) gps_buffer[gps_pos++] = c;

            if (c == '\n') {
                gps_buffer[gps_pos] = '\0';
                gps_pos = 0;  // restart curr buffer pos

                log_nmea(gps_buffer);
            }
        }
    }
}

void app_main(void) {
    uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(GPS_UART_NUM, &uart_config);

    uart_set_pin(GPS_UART_NUM, GPS_TX_PIN, GPS_RX_PIN, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    const int rx_buffer = 2048;
    const int tx_buffer = 0;  // cause we arent sending anything to gps
    uart_driver_install(GPS_UART_NUM, rx_buffer, tx_buffer, 0, NULL, 0);

    const int stack_size = 4096;
    const int priority =
        5;  // gps only is at 1 time per second so no need give priority
    xTaskCreate(gps_task, "gps_task", stack_size, NULL, priority, NULL);
}