#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "sm.h"
#include <helpers.h>
#include "camera_control.h"

#define STACK_SIZE  (3072)

void echo_forward(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        ESP_LOGE("UART", "Failed to allocate memory!");
        vTaskDelete(NULL);
        return;
    }

    bool listen;
    listen = true;

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CONSOLE_UART_NUM, data, (BUF_SIZE - 1), 200 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            // Write data back to the UART
            uart_write_bytes(CONTROL_UART_NUM, (const char *) data, len);
            if(strstr((const char*)data, "STOP")){
                free(data);
                vTaskDelete(NULL);
                listen = false;
            }
        }
    }
}

void echo_backward(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        ESP_LOGE("UART", "Failed to allocate memory for back");
        vTaskDelete(NULL);
        return;
    }

    bool listen;
    listen = true;

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CONTROL_UART_NUM, data, (BUF_SIZE - 1), 500 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            // Write data back to the UART
            uart_write_bytes(CONSOLE_UART_NUM, (const char *) data, len);
            if(strstr((const char*)data, "STOP")){
                free(data);
                vTaskDelete(NULL);
                listen = false;
            }
        }
    }
}

void app_main(void)
{
    // helpers_printHeapDetails();
    // psatFSM_start();
    camera_control_init();
    camera_console_init();
    xTaskCreate(echo_forward, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(echo_backward, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}

