#include "camera_control.h"
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "pin_config.h"
#include <stdio.h>

static TaskHandle_t      xHandleCamera_s  = NULL;

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
}

void camera_task(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    char* tag = "Camera";

    if (data == NULL) {
        ESP_LOGE(tag, "Failed to allocate memory!");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(tag, "UART started.");

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(CAMERA_UART_NUM, data, (BUF_SIZE - 1), pdMS_TO_TICKS(100));
        
        if (len) {
            data[len] = 0;
            ESP_LOGI(tag, "%s", data);
            // Write data back to the console.
        }
    }
}

void camera_init(void){
    ESP_LOGI("Control", "Initialising");
    CAMERA_MESSAGE("++INIT++");
}

void camera_take_pics(void){
    ESP_LOGI("Control", "Apogee reached");
    CAMERA_MESSAGE("++APOGEE++");
}

void log_data(const char* data){
    int len = strlen(data);
    char message[len + 15];

    len = snprintf(message, sizeof(message), "++LOG++\n%s\n\n", data);

    ESP_LOGI("Control", "Logging data");
    uart_write_bytes(CAMERA_UART_NUM, message, len);
}

void camera_deinit(void){
    ESP_LOGI("Control", "Deinitialise");
    CAMERA_MESSAGE("++DEINIT++");
}

void camera_startTask(void){
    xTaskCreate(camera_task, "camera_task", STACK_SIZE,
                NULL, 10, &xHandleCamera_s);
}

void camera_stopTask(void){
    vTaskDelete(xHandleCamera_s);
}

void camera_control_deinit(void){
    uart_driver_delete(CAMERA_UART_NUM);
}

// preflight -> called during prelaunch
void camera_preflightTest(void){
    camera_control_init();
    camera_startTask();
    vTaskDelay(pdMS_TO_TICKS(6000));
    camera_init();
    vTaskDelay(pdMS_TO_TICKS(2000));
    camera_take_pics();
    vTaskDelay(pdMS_TO_TICKS(14000));
    const char* data = "Test: Lorem ipsum dolor sit amet";
    log_data(data);
    vTaskDelay(pdMS_TO_TICKS(2000));
    camera_deinit();
    vTaskDelay(pdMS_TO_TICKS(2000));
    camera_stopTask();
    camera_control_deinit();
}
