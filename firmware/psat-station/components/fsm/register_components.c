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

void test()
{
    printf("hey there my name is test and i was called\n");
}

void psatFSM_registerAllComponents()
{
    psatFSM_registerComponent(
        psatFSM_component_gps, psatFSM_componentType_task, gps_init,
        gps_deinit, NULL, gps_startTask, gps_killTask);
    psatFSM_registerComponent(psatFSM_component_timers,
                              psatFSM_componentType_multiple,
                              timer_init, NULL, NULL, NULL, NULL);
    psatFSM_registerComponent(psatFSM_component_buzzers,
                              psatFSM_componentType_normal,
                              buzzer_init, NULL, NULL, NULL, NULL);
    psatFSM_registerComponent(psatFSM_component_buttons,
                              psatFSM_componentType_multiple,
                              button_init, NULL, NULL, NULL, NULL);
}