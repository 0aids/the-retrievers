#ifndef AUDIO_H
#define AUDIO_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void audio_init(void);
void audio_on(void);
void audio_off(void);
void audio_beep(uint32_t duration_ms);

#endif