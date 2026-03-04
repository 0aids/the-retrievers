#include "../include/camera_control.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "pin_config.h"
#include "unity.h"

#define LOW  0
#define HIGH 1
#define REACTION_TIME                                                \
    50 // how much delay to wait before checking pins for update

static const char* TAG = "Camera-Test";

TEST_CASE("Camera init/deinit test", "[camera]")
{
    camera_control_init();
    //xTaskCreate(camera_)
    CAMERA_MESSAGE("++INIT++");
    
    CAMERA_MESSAGE("++DEINIT++");
}
