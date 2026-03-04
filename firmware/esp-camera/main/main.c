#include "camera.h"
#include "driver/uart.h"
#include "esp_camera.h"
#include "esp_err.h"
#include "sd_card.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "uart.h"


void system_init(void){
    esp_err_t err ;

    err = init_camera();
    if(err != ESP_OK) {
        UART_MESSAGE("Cam init fails");
        return;
    }
    UART_MESSAGE("Cam init success");

    err = init_sd_card();
    if (err != ESP_OK) {
        if (err == ESP_FAIL) UART_MESSAGE("Failed to mount filesystem.");
        else UART_MESSAGE("Failed to initialize the card.");
        return;
    }
    UART_MESSAGE("Filesystem mounted.");
    UART_MESSAGE("Camera and sd card successful init");
}

void uart_task(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        UART_MESSAGE("Failed to allocate memory.");
        vTaskDelete(NULL);
        return;
    }

    bool listen;
    listen = true;

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CAM_UART_NUM, data, (BUF_SIZE - 1), 50 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            // Write data back to the UART
            if (strstr((const char*)data, "++INIT++"))          system_init();
            else if (strstr((const char*)data, "++APOGEE++"))   take_pics();
            else if (strstr((const char*)data, "++LOG++"))      log_data((const char*)data);
            else if (strstr((const char*)data, "++TEST++"))     test_pic();
            else if (strstr((const char*)data, "++DEINIT++")){
                deinit_sd_card();
                esp_camera_deinit();
                sdmmc_host_deinit();
                free(data);
                UART_MESSAGE("Successfully deinitialised");
                vTaskDelay(pdMS_TO_TICKS(500));
                listen = false;
            }
        }
    }
    uart_driver_delete(CAM_UART_NUM);
    vTaskDelete(NULL);
}

void app_main(void){
    init_uart();
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}

