#include "audio.h"

#define BUZZER_PIN 23

void audio_init(void) {
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_PIN, 0);
}

void audio_on(void) { gpio_set_level(BUZZER_PIN, 1); }

void audio_off(void) { gpio_set_level(BUZZER_PIN, 0); }

void audio_beep_old(uint32_t duration_ms) {
    audio_on();
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    audio_off();
}

static void audio_beep_task(void* arg) {
    uint32_t duration_ms = (uint32_t)(uintptr_t)arg;
    audio_on();
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    audio_off();
    vTaskDelete(NULL);
}

void audio_beep(uint32_t duration_ms) {
    BaseType_t res = xTaskCreate(audio_beep_task, "beep", 2048,
                                 (void*)(uintptr_t)duration_ms, 1, NULL);
    if (res != pdPASS) {
        audio_beep_old(duration_ms);  // if it fails just use old function
    }
}