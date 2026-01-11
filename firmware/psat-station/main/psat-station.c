#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "sm.h"
#include "timers.h"

void fsm_start() {
    xTaskCreate(psatFSM_start, "fsm_task", 4096, NULL, 10, NULL);
}
void app_main(void) {
    timer_init();
    fsm_start();

    timer_start(timer_timerId_1s);
    timer_start(timer_timerId_5s);
    timer_start(timer_timerId_10s);
    timer_startOnce(timer_timerId_mechanical, 65000);

    vTaskDelay(pdMS_TO_TICKS(60000));
    timer_stop(timer_timerId_1s);
    timer_stop(timer_timerId_5s);
    timer_stop(timer_timerId_10s);
}
