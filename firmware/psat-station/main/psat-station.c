#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "sm.h"

void app_main(void) {
    // psatFSM_start();
    psatFSM_startAsTask();  // running it in task so i can put other code in
                            // this file, however it wont be a task in prod
}
