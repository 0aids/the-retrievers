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
            UART_MESSAGE(CAM_UART_NUM, "Camera capture failed\n");
            return;
        }

        char filename[32];
        sprintf(filename, MOUNT_POINT"/img_%d.jpg", i);

        FILE *file = fopen(filename, "wb");
        if (file == NULL) {
            UART_MESSAGE(CAM_UART_NUM, "Failed to open file for writing\n");
            return;
        }
        fwrite(fb->buf, 1, fb->len, file);
        fclose(file);
        esp_camera_fb_return(fb);
        UART_MESSAGE(CAM_UART_NUM, "Saved pic\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void uart_task(void *pvParameters){
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    if (data == NULL) {
        UART_MESSAGE(CAM_UART_NUM, "Failed to allocate memory.\n");
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
            if(strstr((const char*)data, "INIT")){
                err = init_camera();
                if(err != ESP_OK) UART_MESSAGE(CAM_UART_NUM, "Cam init fails\n");
                else UART_MESSAGE(CAM_UART_NUM, "Cam init success\n");

                err = init_sd_card();
                if (err != ESP_OK) {
                    if (err == ESP_FAIL) UART_MESSAGE(CAM_UART_NUM, "Failed to mount filesystem.\n");
                    else UART_MESSAGE(CAM_UART_NUM, "Failed to initialize the card.\n");
                }
                else UART_MESSAGE(CAM_UART_NUM, "Filesystem mounted.\n");
            }
            
            else if(strstr((const char*)data, "APOGEE")){
                take_pics();
                UART_MESSAGE(CAM_UART_NUM, "Pictures taken");
            }
            else if(strstr((const char*)data, "DEIN")){
                esp_camera_deinit();
                sdmmc_host_deinit();
                free(data);
                listen = false;
            }
        }
    }
    UART_MESSAGE(CAM_UART_NUM, "Deinitialised");
    uart_driver_delete(CAM_UART_NUM);
    vTaskDelete(NULL);
}

void app_main(void){
    init_uart();
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}

