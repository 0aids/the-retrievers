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
    psatFSM_registerComponent(psatFSM_component_gps, gps_init, NULL,
                              test);
    psatFSM_registerComponent(psatFSM_component_timers, timer_init,
                              NULL, NULL);
    psatFSM_registerComponent(psatFSM_component_buzzers, buzzer_init,
                              NULL, NULL);
    psatFSM_registerComponent(psatFSM_component_buttons, button_init,
                              NULL, NULL);
}