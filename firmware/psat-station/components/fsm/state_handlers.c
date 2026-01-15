#include "state_handlers.h"
#include <stdio.h>

void globalEventHandler(const psatFSM_event_t* event) {}

psatFSM_state_e psatFSM_prelaunchStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Prelaunch";

    switch (event->type) {
        case psatFSM_eventType_timer1s:
            printf("1s\n");
            return psatFSM_state_prelaunch;
        case psatFSM_eventType_timer5s:
            printf("5s\n");
            return psatFSM_state_prelaunch;
        case psatFSM_eventType_timer10s:
            printf("10s\n");
            return psatFSM_state_prelaunch;
        case psatFSM_eventType_unfoldMechanism:
            printf("unfolding\n");
            return psatFSM_state_prelaunch;

        default:
            return psatFSM_state_prelaunch;
    }
}

psatFSM_state_e psatFSM_ascentStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Ascent";

    switch (event->type) {
        default:
            return psatFSM_state_ascent;
    }
}

psatFSM_state_e psatFSM_deployedStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Deployed";

    switch (event->type) {
        default:
            return psatFSM_state_deployed;
    }
}

psatFSM_state_e psatFSM_descentStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Descent";

    switch (event->type) {
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

psatFSM_state_e psatFSM_lowpwrStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-LowPower";

    switch (event->type) {
        default:
            return psatFSM_state_lowpwr;
    }
}

psatFSM_state_e psatFSM_errorStateHandler(const psatFSM_event_t* event) {
    static const char* TAG = "PSAT_FSM-Error";

    switch (event->type) {
        default:
            return psatFSM_state_error;
    }
}
