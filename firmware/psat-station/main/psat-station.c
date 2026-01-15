#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "sm.h"
#include "ldr.h"

void app_main(void) {
    // psatFSM_start();
    // psatFSM_startAsTask();  // running it in task so i can put other code in
    // this file, however it wont be a task in prod
    ESP_LOGI(TAG, "Hello world");
    ldr_setup();
    ldr_state state1 = ldr_queryState();
    ESP_LOGI(TAG, "%s",state1.stateString);

    ESP_LOGI(TAG, "voltage: %dmV", ldr_getVoltage());        
    vTaskDelay(1000);

    ESP_LOGI(TAG, "voltage: %dmV", ldr_getVoltage());        
    vTaskDelay(1000);

    ESP_LOGI(TAG, "voltage: %dmV", ldr_getVoltage());        
    vTaskDelay(1000);

    ldr_deinit();
    ldr_state state3 = ldr_queryState();
    ESP_LOGI(TAG, "%s",state3.stateString);
}
