#include "deployment.h"

#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sm.h"

#define DEPLOYMENT_PIN 22

static volatile uint32_t last_press_tick = 0;

static void IRAM_ATTR deployment_isr(void* arg) {
    uint32_t now = xTaskGetTickCountFromISR();
    if (now - last_press_tick < 5) return;
    last_press_tick = now;

    static fsm_event_t evt = {.type = EVENT_DEPLOYMENT_CONFIRMED,
                              .data.ptr = NULL};
    fsm_post_event_from_isr(&evt);
}

void deployment_init_isr(void) {
    gpio_config_t cfg = {.pin_bit_mask = 1ULL << DEPLOYMENT_PIN,
                         .mode = GPIO_MODE_INPUT,
                         .pull_up_en = GPIO_PULLUP_ENABLE,
                         .intr_type = GPIO_INTR_NEGEDGE};
    gpio_config(&cfg);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(DEPLOYMENT_PIN, deployment_isr, NULL);
}

void stop_deployment_button(void) {
    gpio_intr_disable(DEPLOYMENT_PIN);
    gpio_isr_handler_remove(DEPLOYMENT_PIN);
    gpio_reset_pin(DEPLOYMENT_PIN);
}