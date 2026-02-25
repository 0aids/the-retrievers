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


void take_pics(){
    for (int i = 1; i <= 5; i++) {

        char *message;

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            message = "Camera capture failed\n";
            uart_write_bytes(CAM_UART_NUM, message, strlen(message));
            return;
        }

        char filename[32];
        sprintf(filename, MOUNT_POINT"/img_%d.jpg", i);

        FILE *file = fopen(filename, "wb");
        if (file == NULL) {
            message = "Failed to open file for writing\n";
            uart_write_bytes(CAM_UART_NUM, message, strlen(message));
            return;
        }
        fwrite(fb->buf, 1, fb->len, file);
        fclose(file);
        esp_camera_fb_return(fb);
        message = "Saved pic\n";
        uart_write_bytes(CAM_UART_NUM, message, strlen(message));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void uart_task(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        char *message = "Failed to allocate memory.\n";
        uart_write_bytes(CAM_UART_NUM, message, strlen(message));
        vTaskDelete(NULL);
        return;
    }

    esp_err_t err;

    bool listen;
    listen = true;

    while (listen) {
        // Read data from the UART
        int len = uart_read_bytes(CAM_UART_NUM, data, (BUF_SIZE - 1), 200 / portTICK_PERIOD_MS);
        
        if (len) {
            data[len] = 0;
            // Write data back to the UART
            uart_write_bytes(CAM_UART_NUM, (const char *) data, len);
            if(strstr((const char*)data, "INIT")){
                char* message;

                err = init_camera();
                if(ESP_OK != err){
                    message = "Cam init fails\n";
                    uart_write_bytes(CAM_UART_NUM, message, strlen(message));
                }
                message = "Cam init success\n";
                uart_write_bytes(CAM_UART_NUM, message, strlen(message));

                err = init_sd_card();
                if (err != ESP_OK) {
                    if (err == ESP_FAIL) {
                        message = "Failed to mount filesystem.\n";
                        uart_write_bytes(CAM_UART_NUM, message, strlen(message));
                    } else {
                        message = "Failed to initialize the card.\n";
                        uart_write_bytes(CAM_UART_NUM, message, strlen(message));
                    }
                }
                else{
                    message = "Filesystem mounted.\n";
                    uart_write_bytes(CAM_UART_NUM, message, strlen(message));
                }
            }
            
            else if(strstr((const char*)data, "APOGEE")){
                take_pics();
            }
            else if(strstr((const char*)data, "DEIN")){
                esp_camera_deinit();
                sdmmc_host_deinit();
                free(data);
                vTaskDelete(NULL);
                listen = false;
            }
        }
    }
    uart_write_bytes(CAM_UART_NUM, "Deinitialised", strlen("Deinitialised"));
    uart_driver_delete(CAM_UART_NUM);
}

void app_main(void){
    init_uart();
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}

