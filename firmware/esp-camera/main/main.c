#include "camera.h"
#include "driver/uart.h"
#include "sd_card.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "uart.h"

void uart_task(void *pvParameters){
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
            if(strstr((const char*)data, "INIT")){
                init_camera();
                init_sd_card();
            }
            else if(strstr((const char*)data, "APOGEE")){

            }
            else if(strstr((const char*)data, "DEINIT")){
                free(data);
                listen = false;
                vTaskDelete(NULL);
            }
        }
    }
}

void app_main(void){
    init_uart();
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}