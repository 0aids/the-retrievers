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
    TEST_ASSERT_EQUAL_INT8(fsm_get_current_state(), STATE_DEPLOYED);

    fsm_kill();
}
