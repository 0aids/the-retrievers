#include "buttons.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo.h"
#include "state_handlers.h"
#include "timers.h"
#include "loraFsm.h"
#include "components.h"
#include "register_components.h"

void loraFSM_startAsTask()
{
    xTaskCreatePinnedToCore((void*)loraFsm_start, "lora_task", 4096,
                            NULL, 8, NULL, 0);
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
    psatFSM_getComponent(psatFSM_component_ldr)->start();
}

void psatFSM_deployPendingEntryHandler() {}

void psatFSM_deployedEntryHandler()
{
    psatFSM_getComponent(psatFSM_component_ldr)->stop();

    timer_stop(timer_timerId_10s);

    timer_start(timer_timerId_1s);
    timer_start(timer_timerId_5s);
    timer_startOnce(timer_timerId_mechanical, 10000);
}

void psatFSM_descentEntryHandler()
{
    psatFSM_getComponent(psatFSM_component_gps)->start();
    button_enable(button_id_landing);
}

void psatFSM_landingEntryHandler() {}
void psatFSM_recoveryEntryHandler() {}
void psatFSM_lowPowerEntryHandler() {}
void psatFSM_errorEntryHandler()
{
    timer_pauseAllActive();
}
void psatFSM_permanentErrorEntryHandler() {}
