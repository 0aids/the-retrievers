#include "ready.h"

#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_PIN 22

bool deployment_is_ready() { return !gpio_get_level(BUTTON_PIN); }

void deployement_init() {
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
}