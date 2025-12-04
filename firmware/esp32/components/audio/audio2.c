// this file is purely for fun, ill probably remove it later

#include "audio2.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define BUZZER_PASSIVE_PIN 32

#define LEDC_TIMER LEDC_TIMER_1
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_1
#define LEDC_DUTY 4000
#define LEDC_FREQ_MAX 2000

static const uint32_t notes[7] = {208, 233, 277, 233, 349, 349, 311};
static const uint32_t durations[7] = {265, 265, 265, 265, 531, 531, 531};

void audio2_init(void) {
    ledc_timer_config_t timer_conf = {.speed_mode = LEDC_MODE,
                                      .timer_num = LEDC_TIMER,
                                      .duty_resolution = LEDC_TIMER_13_BIT,
                                      .freq_hz = 1000,
                                      .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ch_conf = {.gpio_num = BUZZER_PASSIVE_PIN,
                                     .speed_mode = LEDC_MODE,
                                     .channel = LEDC_CHANNEL,
                                     .timer_sel = LEDC_TIMER,
                                     .duty = 0,
                                     .hpoint = 0};
    ledc_channel_config(&ch_conf);
}

void passive_buzzer_play_song(void) {
    for (int i = 0; i < 7; i++) {
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, notes[i]);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(durations[i]));
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void audio2_beep_task(void* arg) {
    uint32_t duration_ms = (uint32_t)(uintptr_t)arg;
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, 1400);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelete(NULL);
}

void audio2_beep(uint32_t duration_ms) {
    BaseType_t res = xTaskCreate(audio2_beep_task, "beep", 2048,
                                 (void*)(uintptr_t)duration_ms, 1, NULL);
}