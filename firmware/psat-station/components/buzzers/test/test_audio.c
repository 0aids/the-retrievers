#include "../include/buzzer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "pin_config.h"
#include "unity.h"

#define LOW 0
#define HIGH 1

static const char* TAG = "Buzzers-Test";

TEST_CASE("Buzzer GPIO pin goes HIGH on beep", "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;
    const int duration_ms = 2500;

    buzzer_init();
    gpio_set_direction(
        pin, GPIO_MODE_INPUT_OUTPUT);  // overwrite default which is OUTPUT only

    int before, during, after;
    before = gpio_get_level(pin);
    buzzer_beep(duration_ms);

    during = gpio_get_level(pin);
    vTaskDelay(pdMS_TO_TICKS(duration_ms +
                             100));  // give it about 100ms extra to go LOW

    after = gpio_get_level(pin);
    ESP_LOGI(TAG, "GPIO LEVELS: %i, %i, %i", before, during, after);

    TEST_ASSERT_EQUAL(LOW, before);
    TEST_ASSERT_EQUAL(HIGH, during);
    TEST_ASSERT_EQUAL(LOW, after);
}
