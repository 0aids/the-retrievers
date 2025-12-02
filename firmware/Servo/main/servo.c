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

#define interrupt_pin           (35)
int button_state = 0;
uint16_t interrupt_count = 0;

// --- Servo Configuration Constants ---
#define SERVO_GPIO_PIN          (13)            // GPIO Pin the servo signal line is connected to
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT // 14-bit resolution (0 to 16383 steps)
#define LEDC_FREQUENCY          (50)            // Servo standard frequency is 50 Hz (20ms period)

// --- Duty Cycle Limits for Standard Servo (1ms to 2ms pulse) ---
// Total period (20ms) / Max Duty (16383) = 1.221 us per step
// 500 us / 1.221 us/step =  duty steps
// 2500 us / 1.221 us/step =  duty steps
#define SERVO_DUTY_MIN          (410)          // Corresponds to 500 us pulse (approx. 0 degrees)
#define SERVO_DUTY_MAX          (2048)          // Corresponds to 2500 us pulse (approx. 180 degrees)
#define SERVO_ANGLE_MIN         (0)             // The minimum angle the servo should rotate to
#define SERVO_ANGLE_MAX         (180)           // The maximum angle the servo should rotate to

/**
 * @brief 1. Sets up the LEDC Timer and Channel for servo control.
 */
servo_t servo_setup(void) {

    servo_t servo_state = {
        .pwm_timer.speed_mode       = LEDC_MODE,
        .pwm_timer.timer_num        = LEDC_TIMER,
        .pwm_timer.duty_resolution  = LEDC_DUTY_RES,
        .pwm_timer.freq_hz          = LEDC_FREQUENCY,
        .pwm_timer.clk_cfg          = LEDC_AUTO_CLK,
        .pwm_channel.speed_mode     = LEDC_MODE,
        .pwm_channel.channel        = LEDC_CHANNEL,
        .pwm_channel.timer_sel      = LEDC_TIMER,
        .pwm_channel.intr_type      = LEDC_INTR_DISABLE,
        .pwm_channel.gpio_num       = SERVO_GPIO_PIN,
        .pwm_channel.duty           = SERVO_DUTY_MIN, // Set initial angle to 0 degrees
        .pwm_channel.hpoint         = 0,
        .angle         = 0
    };
    ESP_ERROR_CHECK(ledc_timer_config(&servo_state.pwm_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&servo_state.pwm_channel));

    printf("LEDC Servo Control Setup Complete. Pin: GPIO%d, Freq: %d Hz\n", SERVO_GPIO_PIN, LEDC_FREQUENCY);
    return servo_state;
}

/**
 * @brief Maps a value from one range to another.
 * @param angle The value to map.
 * @return The mapped value (e.g., duty cycle).
 */
static uint32_t get_duty(uint16_t angle) {
    // Ensure the input value is clamped within the input range
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    
    // Linear interpolation
    return (uint32_t)((angle - SERVO_ANGLE_MIN) * 
    (SERVO_DUTY_MAX - SERVO_DUTY_MIN) / 
    (SERVO_ANGLE_MAX - SERVO_ANGLE_MIN) + 
    SERVO_DUTY_MIN);
}

/**
 * @brief 2. Moves the servo motor to a specific angle (0 to 180 degrees).
 * @param angle The desired angle in degrees (clamped to 0 to SERVO_ANGLE_MAX).
 */
void servo_set_angle(servo_t *servo_state, uint16_t angle) {
    // Clamp the input angle to the defined range
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    // Map the angle (0-180) to the required duty cycle (1638-3277)
    uint32_t duty_value = get_duty(angle);

    // Set the new duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty_value));

    // Apply the duty cycle change immediately
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

    // Update the static current angle variable
    servo_state->angle = angle;
    printf("Moving servo to %d degrees (Duty: %lu)\n", angle, duty_value);
}

/**
 * @brief 3. Gets the last commanded angle of the servo.
 * @return The current angle in degrees.
 */
uint16_t servo_get_angle(servo_t *servo_state) {
    // In ESP-IDF, the LEDC only stores the duty cycle value.
    // The most practical way to retrieve the "current angle" is to return 
    // the last commanded angle value we saved in servo_state->angle.

    return servo_state->angle;
}

void loop(servo_t *servo_state){
    if (button_state == 0){
        servo_set_angle(servo_state , 0);
    }
    else if (button_state == 1){
        servo_set_angle(servo_state, 60);
    }
    button_state = 0;
    vTaskDelay(100/portTICK_PERIOD_MS);
}

static void IRAM_ATTR gpio_isr_handler(void *arg){
    button_state = 1;
    gpio_isr_handler_add(interrupt_pin, gpio_isr_handler, NULL);
    gpio_intr_enable(interrupt_pin);
}

void app_main(void) {
    gpio_reset_pin(interrupt_pin);
    gpio_set_direction(interrupt_pin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(interrupt_pin, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(interrupt_pin, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(interrupt_pin, gpio_isr_handler, NULL);

    gpio_intr_enable(interrupt_pin);

    // 1. Initialize the LEDC Timer and Channel
    servo_t servo_state = servo_setup();
    while (1){
        loop(&servo_state);
    }
}