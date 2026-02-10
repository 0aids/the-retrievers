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
#include "loraFsm.h"
#include "components.h"
#include "register_components.h"

void loraFSM_startAsTask()
{
    xTaskCreate((void*)loraFsm_start, "lora_task", 4096, NULL, 4,
                NULL);
}

void psatFSM_prelaunchEntryHandler()
{
    gpio_install_isr_service(0);

    psatFSM_registerAllComponents();
    psatFSM_initAll();

    loraFsm_init();
    loraFSM_startAsTask();

    timer_start(timer_timerId_10s);
    button_enable(button_id_prelaunch);
}

void psatFSM_ascentEntryHandler()
{
    ldr_startTask();
    button_enable(button_id_ldr);
}

void psatFSM_deployPendingEntryHandler() {}

void psatFSM_deployedEntryHandler()
{
    ldr_killTask();
    button_disable(button_id_ldr);
    timer_stop(timer_timerId_10s);

    timer_start(timer_timerId_1s);
    timer_start(timer_timerId_5s);
    timer_startOnce(timer_timerId_mechanical, 10000);
}

void psatFSM_descentEntryHandler()
{
    gps_startTask();
    button_enable(button_id_landing);
}

void psatFSM_landingEntryHandler() {}
void psatFSM_recoveryEntryHandler() {}
void psatFSM_lowPowerEntryHandler() {}
void psatFSM_errorEntryHandler() {}
void psatFSM_permanentErrorEntryHandler() {}
