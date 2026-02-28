#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "sm.h"
#include <helpers.h>
#include "camera_control.h"

#define STACK_SIZE  (3072)

void uart_command(void *pvParameters){
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI("Control", "Initialising");
    UART_MESSAGE("INIT");
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI("Control", "Apogee reached");
    UART_MESSAGE("APOGEE");
    vTaskDelay(pdMS_TO_TICKS(12000));

    ESP_LOGI("Control", "Logging data");
    UART_MESSAGE("LOG");
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI("Control", "Attempting to deinitialise");
    UART_MESSAGE("DEIN");
    
    vTaskDelete(NULL);
}
void app_main(void)
{
    // helpers_printHeapDetails();
    // psatFSM_start();
    camera_control_init();
    xTaskCreate(uart_read, "uart_read", STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(uart_command, "uart_command", STACK_SIZE, NULL, 10, NULL);
}

