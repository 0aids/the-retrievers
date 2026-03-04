#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "sm.h"
#include <helpers.h>
#include "bmp280.h"
#include "I2C.h"
#include "timers.h"
#include "gps_driver.h"
#include "bmi323.h"
#include "highg.h"
#include "sensor-processing.h"

void app_main(void)
{
    //helpers_printHeapDetails();
    //psatFSM_startAsTask();

    I2C_init_bus1();
    I2C_init_bus2();

    vTaskDelay(200);

    bmp280_init();
    bmi323_init();
    highg_init();

    //while(true) {
        updateSensorData();
        printf("\npressure: %lf", sensorData.barometric_altitude);
        printf("\n\nACC X: %lf\nACC Y: %lf\nACC Z: %lf\nGYRO X: %lf\nGYRO Y: %lf\nGYRO Z: %lf\n", sensorData.accX, sensorData.accY, sensorData.accZ, sensorData.orientationX, sensorData.orientationY, sensorData.orientationZ);
        vTaskDelay(10);
    //}

    
}
