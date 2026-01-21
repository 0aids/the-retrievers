#pragma once

#include <stdint.h>

#include "driver/gpio.h"

typedef struct
{
    gpio_num_t gpio;
    uint16_t   angle;
    uint16_t   target;
} servo_data_t;

void     servo_init(servo_data_t* servo, gpio_num_t gpio);
void     servo_setAngle(servo_data_t* servo, uint16_t angle);
void     servo_moveTo(servo_data_t* servo, uint16_t target,
                      uint16_t speedDps);
uint16_t servo_getAngle(servo_data_t* servo);
