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

// static const uint32_t notes[7] = {208, 233, 277, 233, 349, 349, 311};
// static const uint32_t durations[7] = {265, 265, 265, 265, 531, 531, 531};

#define SONG_LENGTH 257

static const uint32_t notes[SONG_LENGTH] = {
    554, 622, 622, 698, 831, 740, 698, 622, 554, 622, 0,   415, 415, 0,   277,
    277, 277, 277, 311, 0,   261, 233, 208, 0,   233, 233, 261, 277, 208, 415,
    415, 311, 0,   233, 233, 261, 277, 233, 277, 311, 0,   261, 233, 233, 208,
    0,   233, 233, 261, 277, 208, 208, 311, 311, 311, 349, 311, 277, 311, 349,
    277, 311, 311, 311, 349, 311, 208, 0,   233, 261, 277, 208, 0,   311, 349,
    311, 466, 466, 415, 415, 698, 698, 622, 466, 466, 415, 415, 622, 622, 554,
    523, 466, 554, 554, 554, 554, 554, 622, 523, 466, 415, 415, 415, 622, 554,
    466, 466, 415, 415, 698, 698, 622, 466, 466, 415, 415, 831, 523, 554, 523,
    466, 554, 554, 554, 554, 554, 622, 523, 466, 415, 0,   415, 622, 554, 0,
    0,   277, 277, 277, 277, 311, 0,   261, 233, 208, 0,   233, 233, 261, 277,
    208, 415, 415, 311, 0,   233, 233, 261, 277, 233, 277, 311, 0,   261, 233,
    233, 208, 0,   233, 233, 261, 277, 208, 208, 311, 311, 311, 349, 311, 277,
    311, 349, 277, 311, 311, 311, 349, 311, 208, 0,   233, 261, 277, 208, 0,
    311, 349, 311, 466, 466, 415, 415, 698, 698, 622, 466, 466, 415, 415, 622,
    622, 554, 523, 466, 554, 554, 554, 554, 554, 622, 523, 466, 415, 415, 415,
    622, 554, 466, 466, 415, 415, 698, 698, 622, 466, 466, 415, 415, 831, 523,
    554, 523, 466, 554, 554, 554, 554, 554, 622, 523, 466, 415, 0,   415, 622,
    554, 0};

static const uint32_t durations[SONG_LENGTH] = {
    600, 1000, 600, 600, 100, 100,  100, 100, 600, 1000, 400,  200, 1000,
    400, 200,  200, 200, 200, 400,  200, 200, 200, 1000, 200,  200, 200,
    200, 600,  200, 400, 200, 1000, 200, 200, 200, 200,  200,  200, 200,
    400, 200,  200, 200, 200, 600,  200, 200, 200, 200,  400,  200, 200,
    200, 200,  200, 200, 800, 1000, 200, 200, 200, 200,  200,  200, 200,
    400, 400,  400, 200, 200, 200,  600, 200, 200, 200,  600,  100, 100,
    100, 100,  300, 300, 600, 100,  100, 100, 100, 300,  300,  300, 100,
    200, 100,  100, 100, 100, 300,  300, 300, 100, 200,  200,  200, 400,
    800, 100,  100, 100, 100, 300,  300, 600, 100, 100,  100,  100, 300,
    300, 300,  100, 200, 100, 100,  100, 100, 300, 300,  300,  100, 200,
    200, 200,  400, 800, 400, 400,  200, 200, 200, 200,  400,  200, 200,
    200, 1000, 200, 200, 200, 200,  600, 200, 400, 200,  1000, 200, 200,
    200, 200,  200, 200, 200, 400,  200, 200, 200, 200,  600,  200, 200,
    200, 200,  400, 200, 200, 200,  200, 200, 200, 800,  1000, 200, 200,
    200, 200,  200, 200, 200, 400,  400, 400, 200, 200,  200,  600, 200,
    200, 200,  600, 100, 100, 100,  100, 300, 300, 600,  100,  100, 100,
    100, 300,  300, 300, 100, 200,  100, 100, 100, 100,  300,  300, 300,
    100, 200,  200, 200, 400, 800,  100, 100, 100, 100,  300,  300, 600,
    100, 100,  100, 100, 300, 300,  300, 100, 200, 100,  100,  100, 100,
    300, 300,  300, 100, 200, 200,  200, 400, 800, 400};

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
    for (int i = 0; i < SONG_LENGTH; i++) {
        uint32_t f = notes[i];
        uint32_t d = durations[i];

        if (f == 0) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(pdMS_TO_TICKS(d));
        } else {
            ledc_set_freq(LEDC_MODE, LEDC_TIMER, f);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(pdMS_TO_TICKS(d));

            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    vTaskDelete(NULL);
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
    xTaskCreate(audio2_beep_task, "beep2", 2048, (void*)(uintptr_t)duration_ms,
                1, NULL);
}

void audio2_song_task() { passive_buzzer_play_song(); }

void audio2_song() {
    xTaskCreate(audio2_song_task, "play song", 2048, NULL, 1, NULL);
}