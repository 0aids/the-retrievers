#include "servo.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"
#include <string.h>
#include "freertos/semphr.h"

#define SERVO_FREQ_HZ    50
#define SERVO_TIMER      LEDC_TIMER_0
#define SERVO_MODE       LEDC_LOW_SPEED_MODE
#define SERVO_CHANNEL    LEDC_CHANNEL_0
#define SERVO_RESOLUTION LEDC_TIMER_14_BIT
#define SERVO_MIN_US     500
#define SERVO_MAX_US     2500
#define SERVO_PERIOD_US  20000
#define SERVO_UPDATE_MS  20

static servo_data_t      servoData_s;
static SemaphoreHandle_t servoStateMutex_s;

static inline void       lockTake()
{
    if (servoStateMutex_s)
        xSemaphoreTake(servoStateMutex_s, portMAX_DELAY);
}

static inline void lockGive()
{
    if (servoStateMutex_s)
        xSemaphoreGive(servoStateMutex_s);
}

void servo_stateGetSnapshot(servo_data_t* out)
{
    if (!out || !servoStateMutex_s)
        return;

    lockTake();
    memcpy(out, &servoData_s, sizeof(servoData_s));
    lockGive();
}

static uint32_t angletoDuty(uint16_t angle)
{
    if (angle > 180)
        angle = 180;

    uint32_t maxDuty = (1 << SERVO_RESOLUTION) - 1;
    uint32_t pulseUs =
        SERVO_MIN_US + (angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180;

    return (pulseUs * maxDuty) / SERVO_PERIOD_US;
}

void servo_init()
{
    memset(&servoData_s, 0, sizeof(servoData_s));

    if (!servoStateMutex_s)
    {
        servoStateMutex_s = xSemaphoreCreateMutex();
    }

    lockTake();
    servoData_s.angle  = 0;
    servoData_s.target = 0;
    lockGive();

    ledc_timer_config_t   timer = {.speed_mode = SERVO_MODE,
                                   .timer_num  = SERVO_TIMER,
                                   .duty_resolution = SERVO_RESOLUTION,
                                   .freq_hz = SERVO_FREQ_HZ,
                                   .clk_cfg = LEDC_AUTO_CLK};

    ledc_channel_config_t channel = {.speed_mode = SERVO_MODE,
                                     .channel    = SERVO_CHANNEL,
                                     .timer_sel  = SERVO_TIMER,
                                     .gpio_num   = CFG_SERVO_PIN_d,
                                     .duty       = 0,
                                     .hpoint     = 0};

    ledc_timer_config(&timer);
    ledc_channel_config(&channel);

    servo_setAngle(0);
}

void servo_deinit()
{
    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, 0);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);

    ledc_stop(SERVO_MODE, SERVO_CHANNEL, 0);
    gpio_reset_pin(CFG_SERVO_PIN_d);

    if (servoStateMutex_s)
    {
        vSemaphoreDelete(servoStateMutex_s);
        servoStateMutex_s = NULL;
    }
}

void servo_setAngle(uint16_t angle)
{
    if (angle > 180)
        angle = 180;

    uint32_t duty = angletoDuty(angle);

    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);

    lockTake();
    servoData_s.angle  = angle;
    servoData_s.target = angle;
    lockGive();
}

void servo_moveTo(uint16_t target, uint16_t speedDps)
{
    if (target > 180)
        target = 180;
    if (speedDps == 0)
        speedDps = 1;

    int step = (speedDps * SERVO_UPDATE_MS) / 1000;
    if (step < 1)
        step = 1;

    lockTake();
    servoData_s.target = target;
    int current        = servoData_s.angle;
    lockGive();
    if (current > target)
        step = -step;

    while (current != target)
    {
        current += step;

        if ((step > 0 && current > target) ||
            (step < 0 && current < target))
        {
            current = target;
        }

        servo_setAngle(current);
        vTaskDelay(pdMS_TO_TICKS(SERVO_UPDATE_MS));
    }
}

uint16_t servo_getAngle()
{
    lockTake();
    uint16_t angle = servoData_s.angle;
    lockGive();
    return angle;
}