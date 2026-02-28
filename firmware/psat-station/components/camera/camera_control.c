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
    ESP_ERROR_CHECK(uart_driver_install(CAMERA_UART_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(CAMERA_UART_NUM, &control_uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CAMERA_UART_NUM, CAMERA_TX_IO, CAMERA_RX_IO, -1, -1));
    xTaskCreate(uart_read, "uart_read", STACK_SIZE, NULL, 10, NULL);
}

void uart_read(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    char* tag = "Camera";

    if (data == NULL) {
        ESP_LOGE(tag, "Failed to allocate memory!");
        vTaskDelete(NULL);
        return;
    }

    bool listen;
    listen = true;

    ESP_LOGI(tag, "LISTENING");

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CAMERA_UART_NUM, data, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            ESP_LOGI(tag, "%s", data);
            // Write data back to the UART
            if(strstr((const char*)data, "Successfully deinitialised")){
                free(data);
                listen = false;
            }
        }
    }
    uart_driver_delete(CAMERA_UART_NUM);
    vTaskDelete(NULL);
}

void test(void *pvParameters){
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI("Control", "Initialising");
    UART_MESSAGE("++INIT++");
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI("Control", "Apogee reached");
    UART_MESSAGE("++APOGEE++");
    vTaskDelay(pdMS_TO_TICKS(12000));

    ESP_LOGI("Control", "Logging data");
    UART_MESSAGE("++LOG++\nPressure: 50 pascal\n\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI("Control", "Deinitialise");
    UART_MESSAGE("++DEINIT++");
    
    vTaskDelete(NULL);
}
