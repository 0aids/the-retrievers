#include "camera_control.h"


void camera_control_init(void) {
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(CONTROL_UART_NUM, BUF_SIZE, BUF_SIZE, 10, &uart_queue, 0));
    
    
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(CONTROL_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CONTROL_UART_NUM, TX_IO, RX_IO, -1, -1));
}

void camera_init(void) {
    char* message = "INIT";
    uart_write_bytes(CONTROL_UART_NUM, (const char*)message, strlen(message));
}

void take_pics(void){
    char* message = "APOGEEE";
    uart_write_bytes(CONTROL_UART_NUM, (const char*)message, strlen(message));
}

void camera_deinit(void) {
    char* message = "DEINIT";
    uart_write_bytes(CONTROL_UART_NUM, (const char*)message, strlen(message));
}

void camera_control_deinit(void){
    uart_driver_delete(CONTROL_UART_NUM);
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