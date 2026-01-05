#ifndef AUDIO2_H
#define AUDIO2_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void audio2_init(void);
void passive_buzzer_play_song(void);
void audio2_beep(uint32_t duration_ms);
void audio2_song();

#endif