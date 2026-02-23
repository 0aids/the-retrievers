#include "camera.h"
#include "driver/uart.h"
#include "sd_card.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"


#define CAM_UART_NUM        UART_NUM_0
#define UART_TX             (1)
#define UART_RX             (3)
#define BUF_SIZE            (1024*2)
#define STACK_SIZE          (3072)
#define UART_BAUD_RATE      (115200)

void uart_task(void *arg){
    // Install UART driver using an event queue here
    
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    int intr_alloc_flags = 0;
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_driver_install(CAM_UART_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CAM_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CAM_UART_NUM, UART_TX, UART_RX, -1, -1));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(CAM_UART_NUM, data, (BUF_SIZE - 1), 500 / portTICK_PERIOD_MS);
        
        if (len) {
            // Write data back to the UART
            uart_write_bytes(CAM_UART_NUM, (const char *) data, len);
        }
    }
}

void app_main(void){
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}