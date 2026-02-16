#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "sm.h"
#include <helpers.h>

void app_main(void)
{
    helpers_printHeapDetails();
    psatFSM_start();
}
