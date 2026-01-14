#include "state_handlers.h"

#include <stdio.h>

#include "buttons.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_driver.h"
#include "ldr_task.h"
#include "timers.h"

void globalEventHandler(const psatFSM_event_t* event) {}

psatFSM_state_e psatFSM_prelaunchStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Prelaunch";

    switch (event->type) {
        case psatFSM_eventType_startPrelaunch:
            gps_init();

            timer_init();
            timer_start(timer_timerId_10s);

            ldr_startTask();

            button_init();
            button_enable(button_id_prelaunch);
            button_enable(button_id_landing);
            button_enable(button_id_ldr);

            // buzzer_init()
            return psatFSM_state_prelaunch;

        case psatFSM_eventType_prelaunchComplete:
            button_disable(button_id_prelaunch);
            return psatFSM_state_ascent;

        default:
            return psatFSM_state_prelaunch;
    }
}

psatFSM_state_e psatFSM_ascentStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Ascent";

    switch (event->type) {
        case psatFSM_eventType_timer10s:
            ESP_LOGI(TAG, "send ping with lora");
            return psatFSM_state_ascent;

        case psatFSM_eventType_deploymentPending:
            return psatFSM_state_deployPending;

        default:
            return psatFSM_state_ascent;
    }
}

psatFSM_state_e psatFSM_deployPendingStateHandler(
    const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Deployment-Pending";

    switch (event->type) {
        case psatFSM_eventType_deploymentConfirmed:
            timer_stop(timer_timerId_10s);
            timer_start(timer_timerId_1s);
            timer_start(timer_timerId_5s);
            timer_startOnce(timer_timerId_mechanical, 60000);

            gps_startTask();

            ldr_killTask();
            button_disable(button_id_ldr);

            return psatFSM_state_deployed;

        case psatFSM_eventType_deploymentTimeout:
            return psatFSM_state_ascent;

        default:
            return psatFSM_state_deployPending;
    }
}

psatFSM_state_e psatFSM_deployedStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Deployed";

    switch (event->type) {
        case psatFSM_eventType_timer5s:
            return psatFSM_state_descent;
        default:
            return psatFSM_state_deployed;
    }
}

psatFSM_state_e psatFSM_descentStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Descent";

    switch (event->type) {
        case psatFSM_eventType_timer5s:
            gps_data_t snapshot;
            gps_stateGetSnapshot(&snapshot);
            gps_logGpsSnapshot(&snapshot);
            return psatFSM_state_descent;
        case psatFSM_eventType_landingConfirmed:
            timer_stop(timer_timerId_1s);
            button_disable(button_id_landing);
            return psatFSM_state_landing;
        default:
            return psatFSM_state_descent;
    }
}

psatFSM_state_e psatFSM_landingStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Landing";

    switch (event->type) {
        default:
            return psatFSM_state_landing;
    }
}

psatFSM_state_e psatFSM_recoveryStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Recovery";

    switch (event->type) {
        default:
            return psatFSM_state_recovery;
    }
}

psatFSM_state_e psatFSM_lowPowerStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-LowPower";

    switch (event->type) {
        default:
            return psatFSM_state_lowPower;
    }
}

psatFSM_state_e psatFSM_errorStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Error";

    switch (event->type) {
        default:
            return psatFSM_state_error;
    }
}
