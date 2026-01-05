#include "mock_buttons.h"

#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sm.h"

#define READY_PIN 21
// io5 -> 16
#define LANDING_PIN 19

static volatile uint32_t last_press_landing_tick = 0;
static volatile uint32_t last_press_prelaunch_tick = 0;

static void IRAM_ATTR landing_isr(void* arg) {
    uint32_t now = xTaskGetTickCountFromISR();
    if (now - last_press_landing_tick < 5) return;
    last_press_landing_tick = now;

    static fsm_event_t evt = {
        .type = EVENT_LANDING_CONFIRMED,
    };
    fsm_post_event_from_isr(&evt);
}

void landing_init_isr(void) {
    gpio_config_t cfg = {.pin_bit_mask = 1ULL << LANDING_PIN,
                         .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_ENABLE,
                         .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&cfg);
    gpio_isr_handler_add(LANDING_PIN, landing_isr, NULL);
}

void stop_landing_button(void) {
    gpio_intr_disable(LANDING_PIN);
    gpio_isr_handler_remove(LANDING_PIN);
    gpio_reset_pin(LANDING_PIN);
}

static void IRAM_ATTR prelaunch_isr(void* arg) {
    uint32_t now = xTaskGetTickCountFromISR();
    if (now - last_press_prelaunch_tick < 5) return;
    last_press_prelaunch_tick = now;

    static fsm_event_t evt = {.type = EVENT_PRELAUNCH_COMPLETE};
    fsm_post_event_from_isr(&evt);
}

void prelaunch_init_isr(void) {
    gpio_config_t cfg = {.pin_bit_mask = 1ULL << READY_PIN,
                         .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_ENABLE,
                         .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&cfg);
    gpio_isr_handler_add(READY_PIN, prelaunch_isr, NULL);
}

void stop_prelaunch_button(void) {
    gpio_intr_disable(READY_PIN);
    gpio_isr_handler_remove(READY_PIN);
    gpio_reset_pin(READY_PIN);
}