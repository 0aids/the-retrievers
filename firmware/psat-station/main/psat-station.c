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

void app_main(void)
{
    //helpers_printHeapDetails();
    //psatFSM_start();

    bool a = gps_preflightTest();

    printf("preflight done\n");

    printf("\n\n%d\n\n", a);




}
