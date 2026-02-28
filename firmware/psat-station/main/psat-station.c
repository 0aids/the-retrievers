#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "sm.h"
#include <helpers.h>
#include "camera_control.h"

void app_main(void)
{
    // helpers_printHeapDetails();
    // psatFSM_start();
    camera_control_init();
    xTaskCreate(test, "uart_command", STACK_SIZE, NULL, 10, NULL);
}

