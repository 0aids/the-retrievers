#include "state_handlers.h"

#include <stdio.h>

#include "buttons.h"
#include "buzzer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_driver.h"
#include "pin_config.h"
#include "ldr_task.h"
#include "servo.h"
#include "timers.h"

static servo_data_t servo;

void                globalEventHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Global-Event";

    switch (event->type)
    {
        case psatFSM_eventType_unfoldMechanism:
            servo_moveTo(&servo, 120, 60);
            return;

        case psatFSM_eventType_audioBeep: buzzer_beep(2500); return;

        default: return;
    }
}

// this state purely kick-starts the FSM, and immediately move into prelaunch
psatFSM_state_e
psatFSM_startStateHandler(const psatFSM_event_t* event)
{
    return psatFSM_state_prelaunch;
}

psatFSM_state_e
psatFSM_prelaunchStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Prelaunch";

    switch (event->type)
    {
        case psatFSM_eventType_prelaunchComplete:
            servo_init(&servo, CFG_SERVO_PIN_d);
            return psatFSM_state_ascent;
        default: return psatFSM_state_prelaunch;
    }
}

psatFSM_state_e
psatFSM_ascentStateHandler(const psatFSM_event_t* event)
{
    static const char* TAG = "PSAT_FSM-Ascent";

    switch (event->type)
    {
        case psatFSM_eventType_timer10s:
            ESP_LOGI(TAG, "send ping with lora");
            return psatFSM_state_ascent;

        case psatFSM_eventType_deploymentPending:
            return psatFSM_state_deployPending;

        default: return psatFSM_state_ascent;
    }
}

psatFSM_state_e
psatFSM_deployPendingStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Deployment-Pending";

    switch (event->type)
    {
        case psatFSM_eventType_deploymentConfirmed:
            return psatFSM_state_deployed;

        case psatFSM_eventType_deploymentTimeout:
            return psatFSM_state_ascent;

        default: return psatFSM_state_deployPending;
    }
}

psatFSM_state_e
psatFSM_deployedStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Deployed";

    switch (event->type)
    {
        case psatFSM_eventType_timer5s: return psatFSM_state_descent;
        default: return psatFSM_state_deployed;
    }
}

psatFSM_state_e
psatFSM_descentStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Descent";

    switch (event->type)
    {
        case psatFSM_eventType_timer5s:
            gps_data_t snapshot;
            gps_stateGetSnapshot(&snapshot);
            gps_logGpsSnapshot(&snapshot);
            return psatFSM_state_descent;
        case psatFSM_eventType_landingConfirmed:
            return psatFSM_state_landing;
        default: return psatFSM_state_descent;
    }
}

psatFSM_state_e
psatFSM_landingStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Landing";

    switch (event->type)
    {
        case psatFSM_eventType_timer5s: return psatFSM_state_recovery;
        default: return psatFSM_state_landing;
    }
}

psatFSM_state_e
psatFSM_recoveryStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Recovery";

    switch (event->type)
    {
        case psatFSM_eventType_audioBeep:
            buzzer_beep(2500);
            return psatFSM_state_recovery;
        default: return psatFSM_state_recovery;
    }
}

psatFSM_state_e
psatFSM_lowPowerStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-LowPower";

    switch (event->type)
    {
        default: return psatFSM_state_lowPower;
    }
}

psatFSM_state_e
psatFSM_errorStateHandler(const psatFSM_event_t* event)
{
    // static const char* TAG = "PSAT_FSM-Error";

    switch (event->type)
    {
        default: return psatFSM_state_error;
    }
}
