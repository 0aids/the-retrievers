#include "uart.h"

void uart_init(void){
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
}

void uart_task(void *arg){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    bool listen;
    listen = true;

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CAM_UART_NUM, data, (BUF_SIZE - 1), 500 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            // Write data back to the UART
            uart_write_bytes(CAM_UART_NUM, (const char *) data, len);
            if(strstr((const char*)data, "DEINIT")){
                free(data);
                listen = false;
                vTaskDelete(NULL);
            }
        }
    }
}