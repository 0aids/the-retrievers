#pragma once

#include <stdint.h>

#include "driver/gpio.h"

typedef struct
{
    uint16_t   angle;
    uint16_t   target;
} servo_data_t;

void     servo_init();
void     servo_deinit();
void     servo_setAngle(uint16_t angle);
void     servo_stateGetSnapshot(servo_data_t* out);
void     servo_moveTo(uint16_t target, uint16_t speedDps);

uint16_t servo_getAngle();