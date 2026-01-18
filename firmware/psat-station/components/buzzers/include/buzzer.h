#pragma once
#include <stdint.h>

void buzzer_init(void);
void buzzer_turnOn(void);
void buzzer_turnOff(void);
void buzzer_beep(uint32_t durationMs);
