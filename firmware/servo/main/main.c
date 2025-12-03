//Using LEDC peripheral to control the servo motor

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "servo.h"

#include "driver/gpio.h"

#define interrupt_pin           (25)

void loop(servo_t *servo_state, bool deploy){
    // Changes the angle if the PSAT is deployed. Remains at 0 otherwise.
    if (deploy){
        servo_set_angle(servo_state , 60);
    }
    else{
        servo_set_angle(servo_state, 0);
    }
}

// For testing
static void IRAM_ATTR gpio_isr_handler(void *arg){
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
    bool deploy = false;
    while (1){
        loop(&servo_state, deploy);
    }
}