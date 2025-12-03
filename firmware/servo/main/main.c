//Using LEDC peripheral to control the servo motor

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "servo.h"

#include "driver/gpio.h"

#define button           (25)


// For testing
void app_main(void) { 
    gpio_reset_pin(button);

    gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);
    gpio_set_direction(button, GPIO_MODE_INPUT);

    // Sets up the servo
    servo_t servo_state = servo_setup();
    // Loops to check if deployed
    while (1){
        int status = gpio_get_level(button);
        if (status){
            servo_set_angle(&servo_state, 0);
            printf("Angle at 0\n");
        }
        else{

            servo_set_angle(&servo_state , 60);
            printf("Angle at 60\n");
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}