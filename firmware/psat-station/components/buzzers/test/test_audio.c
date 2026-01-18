#include "../include/buzzer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "pin_config.h"
#include "unity.h"

#define LOW 0
#define HIGH 1
#define REACTION_TIME \
    50  // how much delay to wait before checking pins for update

static const char* TAG = "Buzzers-Test";

TEST_CASE("Buzzer init sets GPIO LOW", "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;

    buzzer_init();
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);

    TEST_ASSERT_EQUAL(LOW, gpio_get_level(pin));
}

TEST_CASE("Buzzer beep with zero duration does nothing", "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;

    buzzer_init();
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);

    gpio_set_level(pin,
                   LOW);  // test 1 will have ensured this, but just in case
    buzzer_beep(0);

    vTaskDelay(pdMS_TO_TICKS(REACTION_TIME));
    TEST_ASSERT_EQUAL(LOW, gpio_get_level(pin));
}

TEST_CASE("Buzzer GPIO pin goes LOW HIGH LOW on beep", "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;
    const int duration_ms = 2500;

    buzzer_init();
    gpio_set_direction(
        pin, GPIO_MODE_INPUT_OUTPUT);  // overwrite default which is OUTPUT only

    int before, during, after;
    before = gpio_get_level(pin);

    buzzer_beep(duration_ms);
    during = gpio_get_level(pin);

    vTaskDelay(pdMS_TO_TICKS(
        duration_ms + REACTION_TIME));  // give it about 100ms extra to go LOW
    after = gpio_get_level(pin);

    TEST_ASSERT_EQUAL(LOW, before);
    TEST_ASSERT_EQUAL(HIGH, during);
    TEST_ASSERT_EQUAL(LOW, after);
}

TEST_CASE("buzzer turn off stops active beep", "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;

    buzzer_init();
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);

    buzzer_beep(500);
    vTaskDelay(pdMS_TO_TICKS(50));
    buzzer_turnOff();  // should cut the 500ms beep short

    vTaskDelay(pdMS_TO_TICKS(REACTION_TIME));
    TEST_ASSERT_EQUAL(LOW, gpio_get_level(pin));
}

TEST_CASE("Buzzer beep mid another beep uses most recent beep (extending)",
          "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;

    buzzer_init();
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);

    buzzer_beep(500);  // original beep is 500ms but we will extend it
    vTaskDelay(pdMS_TO_TICKS(250));

    buzzer_beep(2500);
    vTaskDelay(pdMS_TO_TICKS(REACTION_TIME));
    TEST_ASSERT_EQUAL(HIGH, gpio_get_level(pin));

    vTaskDelay(pdMS_TO_TICKS(2500));
    TEST_ASSERT_EQUAL(LOW, gpio_get_level(pin));
}

TEST_CASE("Buzzer beep mid another beep uses most recent beep (reducing)",
          "[audio]") {
    const int pin = config_getPinConfig()->buzzerPin;

    buzzer_init();
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);

    buzzer_beep(2500);  // original beep is 500ms but we will this time lower it
    vTaskDelay(pdMS_TO_TICKS(250));

    buzzer_beep(500);
    vTaskDelay(pdMS_TO_TICKS(500));

    vTaskDelay(pdMS_TO_TICKS(REACTION_TIME));
    TEST_ASSERT_EQUAL(LOW, gpio_get_level(pin));
}