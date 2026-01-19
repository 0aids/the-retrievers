#include <stdio.h>

#include "buttons.h"
#include "buzzer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_driver.h"
#include "ldr_task.h"
#include "servo.h"
#include "state_handlers.h"
#include "timers.h"

void psatFSM_prelaunchEntryHandler() {
    gps_init();

    timer_init();
    timer_start(timer_timerId_10s);

    ldr_startTask();

    button_init();
    button_enable(button_id_prelaunch);
    button_enable(button_id_landing);
    button_enable(button_id_ldr);

    buzzer_init();
}
void psatFSM_ascentEntryHandler() {
    printf("Entering state psatFSM_ascentEntryHandler\n");
}
void psatFSM_deployPendingEntryHandler() {
    printf("Entering state psatFSM_deployPendingEntryHandler\n");
}
void psatFSM_deployedEntryHandler() {
    printf("Entering state psatFSM_deployedEntryHandler\n");
}
void psatFSM_descentEntryHandler() {
    printf("Entering state psatFSM_descentEntryHandler\n");
}
void psatFSM_landingEntryHandler() {
    printf("Entering state psatFSM_landingEntryHandler\n");
}
void psatFSM_recoveryEntryHandler() {
    printf("Entering state psatFSM_recoveryEntryHandler\n");
}
void psatFSM_lowPowerEntryHandler() {
    printf("Entering state psatFSM_lowPowerEntryHandler\n");
}
void psatFSM_errorEntryHandler() {
    printf("Entering state psatFSM_errorEntryHandler\n");
}