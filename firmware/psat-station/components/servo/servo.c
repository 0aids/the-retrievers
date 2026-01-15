#include "servo.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVO_FREQ_HZ 50
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_MODE LEDC_LOW_SPEED_MODE
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_RESOLUTION LEDC_TIMER_14_BIT
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2500
#define SERVO_PERIOD_US 20000
#define SERVO_UPDATE_MS 20

static uint32_t angle_toDuty(uint16_t angle) {
    if (angle > 180) angle = 180;

    uint32_t maxDuty = (1 << SERVO_RESOLUTION) - 1;
    uint32_t pulseUs =
        SERVO_MIN_US + (angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180;

    return (pulseUs * maxDuty) / SERVO_PERIOD_US;
}

void servo_init(servo_data_t* servo, gpio_num_t gpio) {
    servo->gpio = gpio;
    servo->angle = 0;
    servo->target = 0;

    ledc_timer_config_t timer = {.speed_mode = SERVO_MODE,
                                 .timer_num = SERVO_TIMER,
                                 .duty_resolution = SERVO_RESOLUTION,
                                 .freq_hz = SERVO_FREQ_HZ,
                                 .clk_cfg = LEDC_AUTO_CLK};

    ledc_channel_config_t channel = {.speed_mode = SERVO_MODE,
                                     .channel = SERVO_CHANNEL,
                                     .timer_sel = SERVO_TIMER,
                                     .gpio_num = gpio,
                                     .duty = 0,
                                     .hpoint = 0};

    ledc_timer_config(&timer);
    ledc_channel_config(&channel);

    servo_setAngle(servo, 0);
}

void servo_setAngle(servo_data_t* servo, uint16_t angle) {
    if (angle > 180) angle = 180;

    uint32_t duty = angle_toDuty(angle);

    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);

    servo->angle = angle;
    servo->target = angle;
}

void servo_moveTo(servo_data_t* servo, uint16_t target, uint16_t speedDps) {
    if (target > 180) target = 180;
    if (speedDps == 0) speedDps = 1;

    servo->target = target;

    int step = (speedDps * SERVO_UPDATE_MS) / 1000;
    if (step < 1) step = 1;

    int current = servo->angle;
    if (current > target) step = -step;

    while (current != target) {
        current += step;

        if ((step > 0 && current > target) || (step < 0 && current < target)) {
            current = target;
        }

        servo_setAngle(servo, current);
        vTaskDelay(pdMS_TO_TICKS(SERVO_UPDATE_MS));
    }
}

uint16_t servo_getAngle(servo_data_t* servo) { return servo->angle; }
