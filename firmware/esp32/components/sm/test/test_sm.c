#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "sm.h"
#include "timers.h"
#include "unity.h"

TEST_CASE("transition from ascent to deployed", "[sm]") {
    timers_init();
    fsm_start();

    ESP_LOGI("TESTING", "PRESS THE BUTTON NOW");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI("TESTING", "Current State %d", fsm_get_current_state());

    fsm_kill();
    TEST_ASSERT_EQUAL_INT8(fsm_get_current_state(), STATE_DEPLOYED);
}

TEST_CASE("transition from descent to landing", "[sm]") {
    timers_init();
    fsm_start();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    fsm_set_current_state(STATE_DECENT);
    ESP_LOGI("TESTING", "Current State %d", fsm_get_current_state());

    ESP_LOGI("TESTING", "Triggering fake landing event");
    fsm_event_t landing_event = {.type = EVENT_LANDING_CONFIRMED};
    fsm_post_event(&landing_event);
    ESP_LOGI("TESTING", "Current State %d", fsm_get_current_state());

    fsm_kill();
    TEST_ASSERT_EQUAL_INT8(fsm_get_current_state(), STATE_LANDING);
}
