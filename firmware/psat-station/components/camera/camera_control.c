#include "camera_control.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "pin_config.h"
#include <stdio.h>

static TaskHandle_t      xHandleCamera_s  = NULL;
#define TAG             "Cam-controller"

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

    char* tag = "Camera module echo";

    if (data == NULL) {
        ESP_LOGE(tag, "Failed to allocate memory!");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(tag, "Echo started");

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(CAMERA_UART_NUM, data, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            ESP_LOGI(tag, "%s", data);
            // Write data back to the console.
        }
    }
}

void camera_init(void){
    ESP_LOGI(TAG, "Initialising");
    CAMERA_MESSAGE("++INIT++");
}

void camera_take_pics(void){
    ESP_LOGI(TAG, "Taking 10 pictures");
    CAMERA_MESSAGE("++TAKE++");
}

void log_data(const char* data){
    int len = strlen(data);
    char message[len + 15];

    len = snprintf(message, sizeof(message), "++LOG++\n%s\n\n", data);

    ESP_LOGI(TAG, "Logging data");
    uart_write_bytes(CAMERA_UART_NUM, message, len);
}

void camera_deinit(void){
    ESP_LOGI(TAG, "Deinitialise");
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

void camera_control_test(void *const pvParameters){
    bool *pass_ptr = (bool *)pvParameters;
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    char* tag = "Test";

    if (data == NULL) {
        ESP_LOGE(tag, "Failed to allocate memory!");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(tag, "Start");
    bool init = false, pic = false, log = false, deinit = false;

    while (!*pass_ptr) {
        // Read data from the UART
        int len = uart_read_bytes(CAMERA_UART_NUM, data, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            ESP_LOGI("Camera module echo", "%s", data);
            // Write data back to the console.
            // Write data back to the UART
            if (strstr((const char*)data, "successful init"))    
            {
                init = true; 
                ESP_LOGI(tag, "INIT success");
            }
            else if (strstr((const char*)data, "Saved pic"))            
            {
                pic = true; 
                ESP_LOGI(tag, "PIC success");
            }
            else if (strstr((const char*)data, "Logged data"))         
            { 
                log = true; 
                ESP_LOGI(tag, "LOG success");
            }
            else if (strstr((const char*)data, "Successfully deinit"))  
            {
                deinit = true; 
                ESP_LOGI(tag, "DEINIT success");
            }
        }
        if (init && pic && log && deinit){
            *pass_ptr = true;
        }
    }
    free(data);
    vTaskDelete(NULL);
}

// preflight -> called during prelaunch
bool camera_preflightTest(void){
    bool pass = false;
    TaskHandle_t xHandleTestCamera_s = NULL;    
    camera_control_init();
    xTaskCreate(camera_control_test, "camera control test", STACK_SIZE,
                &pass, 9, &xHandleTestCamera_s);
    
    camera_init();
    vTaskDelay(pdMS_TO_TICKS(1000));

        CAMERA_MESSAGE("++TEST++");
    ESP_LOGI(TAG, "Taking pic");
    vTaskDelay(pdMS_TO_TICKS(1000));

    log_data("Test: Data logged");
    vTaskDelay(pdMS_TO_TICKS(1000));

    camera_deinit();
    vTaskDelay(pdMS_TO_TICKS(200));

    camera_control_deinit();
    vTaskDelete(xHandleTestCamera_s);
    if (pass == true) ESP_LOGI("Test", "Successful cam test");
    return pass;
}
