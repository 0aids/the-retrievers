#pragma once
#include <stdint.h>
#include <stdbool.h>

void buzzer_init(void);
void buzzer_deinit(void);
void buzzer_turnOn(void);
void buzzer_turnOff(void);
void buzzer_beep(uint32_t durationMs);

bool buzzer_preflightTest();
