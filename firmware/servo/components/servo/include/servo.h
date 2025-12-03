#ifndef included_servo_h
#define included_servo_h
#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
typedef uint16_t angle_t;

// Struct to organise the servo configuration & state
typedef struct {
    ledc_timer_config_t pwm_timer;
    ledc_channel_config_t pwm_channel;
    angle_t angle;
} servo_t;

// Sets the angle of the servo
void servo_set_angle(servo_t *servo_state, uint16_t angle);

// Returns the angle the servo was last set to because the servo does not store the current servo angle
uint16_t servo_get_angle(servo_t *servo_state);

// Receives the angle and returns the duty steps required for the corresponding angle.
// The duty steps are the amount of steps the signal is kept at HIGH.
uint32_t get_duty(uint16_t angle);

// Configure the servo peripheral and sets the initial angle to 0
servo_t servo_setup(void);

#endif