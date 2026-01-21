#include "../include/servo.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "pin_config.h"
#include "unity.h"

TEST_CASE("Servo init sets angle and target angles to 0", "[servo]")
{
    servo_data_t servo;
    servo_init(&servo, CFG_SERVO_PIN_d);

    servo_moveTo(&servo, 0, 60);
    TEST_ASSERT_EQUAL_UINT16(0, servo.angle);
    TEST_ASSERT_EQUAL_UINT16(0, servo.target);
}

TEST_CASE("Servo angles over 180 deg are set to 180 deg", "[servo]")
{
    servo_data_t servo;
    servo_init(&servo, CFG_SERVO_PIN_d);

    servo_setAngle(&servo, 6767);
    TEST_ASSERT_EQUAL_UINT16(180, servo_getAngle(&servo));
}

TEST_CASE("Servo move to sets target angle", "[servo]")
{
    servo_data_t servo;
    servo_init(&servo, CFG_SERVO_PIN_d);

    servo_moveTo(&servo, 120, 60);
    TEST_ASSERT_EQUAL_UINT16(120, servo.target);
}

TEST_CASE("Servo move to eventually reaches target angle", "[servo]")
{
    servo_data_t servo;
    servo_init(&servo, CFG_SERVO_PIN_d);

    servo_moveTo(&servo, 120, 60);

    TEST_ASSERT_EQUAL_UINT16(120, servo_getAngle(&servo));
}

TEST_CASE("Servo moveTo with zero speed is forced to 1", "[servo]")
{
    servo_data_t servo;
    servo_init(&servo, CFG_SERVO_PIN_d);

    servo_moveTo(&servo, 90, 0);
    TEST_ASSERT_EQUAL_UINT16(90, servo_getAngle(&servo));
}
