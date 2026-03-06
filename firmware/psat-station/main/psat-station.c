#include <stdio.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "ldr.h"
#include "sm.h"
#include <helpers.h>
#include "bmp280.h"
#include "I2C.h"
#include "timers.h"
#include "gps_driver.h"
#include "bmi323.h"
#include "highg.h"
//#include "sensor-processing.h"
#include "camera_control.h"

void app_main(void)
{
    helpers_printHeapDetails();
    psatFSM_startAsTask();
}
