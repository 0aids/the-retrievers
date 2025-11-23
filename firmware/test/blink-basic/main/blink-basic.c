#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include BLINK_GPIO 23

void app_main(void)
{
    uint8_t ledState = 0;
    while (1) {
        ESP_LOGI("example", "Hello world"); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, ledState);
    }
}
