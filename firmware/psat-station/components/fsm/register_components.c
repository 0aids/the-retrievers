#include "buttons.h"
#include "servo.h"
#include "ldr_task.h"
#include "ldr.h"
#include "buzzer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_driver.h"
#include "ldr_task.h"
#include "servo.h"
#include "shared_state.h"
#include "state_handlers.h"
#include "timers.h"
#include "loraFsm.h"
#include "components.h"
#include "bmp280.h"
#include "highg.h"

#include "register_components.h"

void psatFSM_registerAllComponents()
{
    psatFSM_registerComponent(
        psatFSM_component_gps, psatFSM_componentType_task, gps_init,
        gps_deinit, NULL, gps_startTask, gps_killTask,gps_preflightTest);

    psatFSM_registerComponent(
        psatFSM_component_timers, psatFSM_componentType_multiple,
        timer_init, timer_deinit, NULL, NULL, NULL, timer_preflightTest);

    psatFSM_registerComponent(
        psatFSM_component_buzzers, psatFSM_componentType_normal,
        buzzer_init, buzzer_deinit, NULL, NULL, NULL, buzzer_preflightTest);

    psatFSM_registerComponent(
        psatFSM_component_buttons, psatFSM_componentType_multiple,
        button_init, button_deinit, NULL, NULL, NULL, NULL);

    psatFSM_registerComponent(
        psatFSM_component_servo, psatFSM_componentType_normal,
        servo_init, servo_deinit, NULL, NULL, NULL, NULL);

    psatFSM_registerComponent(
        psatFSM_component_ldr, psatFSM_componentType_normal,
        ldr_setup, ldr_deinit, NULL, ldr_startTask, ldr_killTask, ldr_preflightTest);

    // psatFSM_registerComponent(psatFSM_component_bmp280, psatFSM_componentType_normal, 
        // bmp280_init, bmp280_deinit, NULL, NULL, NULL, bmp280_preflightTest);

    // psatFSM_registerComponent(psatFSM_component_highg, psatFSM_componentType_normal, 
    //     highg_init, highg_deinit, NULL, NULL, NULL, NULL);

    // TODO: whats left: PRESSURE, CAMERA, ACCELEROMETER, BATTERY
}