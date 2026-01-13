#include "gps_driver.h"

#include <stdio.h>

#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"

#define TIMEOUT 50
#define GPS_TASK_PRIO 5
#define GPS_LINE_MAX 512
#define GPS_UART_BAUD 9600
#define UART_READ_CHUNK 128
#define GPS_TASK_STACK 4096
#define GPS_RX_BUFFER_SIZE 2048

static TaskHandle_t gpsTask_s = NULL;

static void gps_task(void* arg) {
    char gpsBuffer[GPS_LINE_MAX];
    int gpsBufferPosition = 0;

    while (1) {
        uint8_t data[UART_READ_CHUNK];
        int len = uart_read_bytes(config_getPinConfig()->gpsUartNum, data,
                                  sizeof(data), TIMEOUT / portTICK_PERIOD_MS);

        if (len == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // go through each byte we just read and add it to the buffer
        for (int i = 0; i < len; i++) {
            char c = (char)data[i];
            if (c == '\r') continue;

            if (gpsBufferPosition < GPS_LINE_MAX - 1)
                gpsBuffer[gpsBufferPosition++] = c;
            else {
                ESP_LOGW("GPS", "The line overflowed so discarded");
                while (i < len && data[i] != '\n') i++;
                gpsBufferPosition = 0;
                continue;
            }

            if (c == '\n') {
                // when we reach new line, we know a sentence is
                // complete
                gpsBuffer[gpsBufferPosition] = '\0';

                if (gpsBufferPosition > 0 &&
                    gpsBuffer[gpsBufferPosition - 1] == '\n') {
                    gpsBuffer[gpsBufferPosition - 1] = '\0';
                }

                gps_processLine(gpsBuffer);

                gpsBufferPosition = 0;  // reset buffer pos to start
            }
        }
    }
}

void gps_init() {
    ESP_LOGI("GPS", "Initialising GPS state");
    gps_stateInit();

    ESP_LOGI("GPS", "Install GPS uart driver");
    uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_port_t gpsUartNum = config_getPinConfig()->gpsUartNum;
    ESP_ERROR_CHECK(uart_param_config(gpsUartNum, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(gpsUartNum, config_getPinConfig()->gpsTxPin,
                                 config_getPinConfig()->gpsRxPin,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(
        uart_driver_install(gpsUartNum, GPS_RX_BUFFER_SIZE, 0, 0, NULL, 0));
}

void gps_startTask() {
    ESP_LOGI("GPS", "Starting GPS Task");
    xTaskCreate(gps_task, "gps_task", GPS_TASK_STACK, NULL, GPS_TASK_PRIO,
                &gpsTask_s);
}

void gps_killTask() {
    ESP_LOGI("GPS", "Killing GPS Task");
    if (gpsTask_s) {
        uart_flush(config_getPinConfig()->gpsUartNum);
        vTaskDelete(gpsTask_s);
        gpsTask_s = NULL;
    }
}