#ifndef included_servo_h
#define included_servo_h
#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
typedef uint16_t angle_t;

typedef struct {
    ledc_timer_config_t pwm_timer;
    ledc_channel_config_t pwm_channel;
    angle_t angle;
} servo_t;

void servo_set_angle(servo_t *servo_state, uint16_t angle);

uint16_t servo_get_angle(servo_t *servo_state);

static uint32_t get_duty(uint16_t angle);

servo_t servo_setup(void);

#endif