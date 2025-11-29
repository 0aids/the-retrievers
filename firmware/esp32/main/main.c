#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "sm.h"
#include "timers.h"

static const char* TAG = "MAIN";

void app_main(void) {
    timers_init();

    fsm_start();
    ESP_LOGI(TAG, "PSAT has started");

    vTaskDelay(pdMS_TO_TICKS(30000));

    ESP_LOGI(TAG, "Triggering fake landing event");
    fsm_event_t landing_event = {.type = EVENT_LANDING_CONFIRMED};
    fsm_post_event(&landing_event);

    vTaskDelay(pdMS_TO_TICKS(15000));
    ESP_LOGI(TAG, "Triggering sound event");
    fsm_event_t audio_event = {.type = EVENT_AUDIO_BEEP};
    fsm_post_event(&audio_event);

    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI(TAG, "Triggering sound event 2");
    fsm_event_t audio_event2 = {.type = EVENT_AUDIO_FUN};
    fsm_post_event(&audio_event2);
}
