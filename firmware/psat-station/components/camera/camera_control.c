#include "camera_control.h"


void camera_control_init(void) {
    uart_config_t control_uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    int intr_alloc_flags = 0;
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_driver_install(CONTROL_UART_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CONTROL_UART_NUM, &control_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONTROL_UART_NUM, CONTROL_TX_IO, CONTROL_RX_IO, -1, -1));
}

void camera_console_init(void) {
    uart_config_t console_uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    int intr_alloc_flags = 0;
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_driver_install(CONSOLE_UART_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CONSOLE_UART_NUM, &console_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONSOLE_UART_NUM, CONSOLE_TX_IO, CONSOLE_RX_IO, -1, -1));
}

void camera_listen(void){

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        ESP_LOGE("UART Echo", "Failed to allocate memory!\n");
        return;
    }

    bool listen = true;
    while(listen){
        int len = uart_read_bytes(CONTROL_UART_NUM, data, BUF_SIZE-1, pdMS_TO_TICKS(50));

        if (len > 0){
            data[len] = '\0';
            char* echo = (char *) data;
            printf("return %s", echo);
            listen = false;
        }
    }
    free(data);
}