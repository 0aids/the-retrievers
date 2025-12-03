/*
Using LEDC peripheral to control the servo motor
*/


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "servo.h"

#include "driver/gpio.h"

#define interrupt_pin           (25)
bool deploy = false;
uint16_t interrupt_count = 0;

#define SERVO_GPIO_PIN          (13)             // GPIO Pin the servo signal is connected to
#define PWM_TIMER              LEDC_TIMER_0
#define PWM_MODE               LEDC_LOW_SPEED_MODE
#define PWM_CHANNEL            LEDC_CHANNEL_0
#define PWM_DUTY_RES           LEDC_TIMER_14_BIT // 14-bit resolution (0 to 16383 steps)
#define PWM_FREQUENCY          (50)              // servo standard frequency = 50Hz
#define PWM_INTR_TYPE          LEDC_INTR_DISABLE
#define PWM_CLK                LEDC_AUTO_CLK

// 50Hz -> 20ms timer period
// 14 bit timer resolution -> 16383 duty steps
// 20ms/16383 steps = 1.221 us/step

// (500 us pulse)/(1.221 us/step) = 410 duty steps
#define SERVO_DUTY_MIN          (410)           // Corresponds to 500 us pulse (approx. 0 degrees)
// (2500 us pulse)/(1.221 us/step) = 2048 duty steps
#define SERVO_DUTY_MAX          (2048)          // Corresponds to 2500 us pulse (approx. 180 degrees)
#define SERVO_ANGLE_MIN         (0)             // The minimum angle of the servo
#define SERVO_ANGLE_MAX         (180)           // The maximum angle of the servo

servo_t servo_setup(void) {

    servo_t servo_state = {
        .pwm_timer.speed_mode       = PWM_MODE,
        .pwm_timer.timer_num        = PWM_TIMER,
        .pwm_timer.duty_resolution  = PWM_DUTY_RES,
        .pwm_timer.freq_hz          = PWM_FREQUENCY,
        .pwm_timer.clk_cfg          = PWM_CLK,
        .pwm_channel.speed_mode     = PWM_MODE,
        .pwm_channel.channel        = PWM_CHANNEL,
        .pwm_channel.timer_sel      = PWM_TIMER,
        .pwm_channel.intr_type      = PWM_INTR_TYPE,
        .pwm_channel.gpio_num       = SERVO_GPIO_PIN,
        .pwm_channel.duty           = SERVO_DUTY_MIN, 
        .pwm_channel.hpoint         = 0,
        .angle                      = 0 // Set initial angle to 0 degrees
    };
    ESP_ERROR_CHECK(ledc_timer_config(&servo_state.pwm_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&servo_state.pwm_channel));

    return servo_state;
}

uint32_t get_duty(uint16_t angle) {
    // Ensure the angle value is kept within range
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    

    return (uint32_t)((angle) * 
    (SERVO_DUTY_MAX - SERVO_DUTY_MIN) / 
    (SERVO_ANGLE_MAX ) + 
    SERVO_DUTY_MIN);
}

void servo_set_angle(servo_t *servo_state, uint16_t angle) {
    // Ensure the angle value is kept within range
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    // Map the angle (0-180) to the required duty steps (410-2048)
    uint32_t duty_steps = get_duty(angle);

    // Set the new duty cycle & sets the new duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty_steps));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_MODE, PWM_CHANNEL));

    // Update the current angle
    servo_state->angle = angle;
}

uint16_t servo_get_angle(servo_t *servo_state) {
    return servo_state->angle;
}

void loop(servo_t *servo_state){
    // Changes the angle if the PSAT is deployed. Remains at 0 otherwise.
    if (deploy){
        servo_set_angle(servo_state , 60);
    }
    else{
        servo_set_angle(servo_state, 0);
    }
    // For testing
    deploy = false;
    vTaskDelay(100/portTICK_PERIOD_MS);
    
}

// For testing
static void IRAM_ATTR gpio_isr_handler(void *arg){
    deploy = true;
    gpio_isr_handler_add(interrupt_pin, gpio_isr_handler, NULL);
    gpio_intr_enable(interrupt_pin);
}


void app_main(void) {
    // For testing
    gpio_reset_pin(interrupt_pin);
    gpio_set_direction(interrupt_pin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(interrupt_pin, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(interrupt_pin, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(interrupt_pin, gpio_isr_handler, NULL);

    gpio_intr_enable(interrupt_pin);

    // Sets up the servo
    servo_t servo_state = servo_setup();
    // Loops to check if deployed
    while (1){
        loop(&servo_state);
    }
}