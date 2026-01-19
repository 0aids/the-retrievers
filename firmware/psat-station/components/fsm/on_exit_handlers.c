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

void psatFSM_startExitHandler() {}  // do nothing

void psatFSM_prelaunchExitHandler() { button_disable(button_id_prelaunch); }

void psatFSM_ascentExitHandler() {}
void psatFSM_deployPendingExitHandler() {}

void psatFSM_deployedExitHandler() {}
void psatFSM_descentExitHandler() {
    timer_stop(timer_timerId_1s);
    button_disable(button_id_landing);
}

void psatFSM_landingExitHandler() {}
void psatFSM_recoveryExitHandler() {}
void psatFSM_lowPowerExitHandler() {}
void psatFSM_errorExitHandler() {}