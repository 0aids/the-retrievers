#pragma once

typedef enum {
    psatFSM_state_prelaunch,
    psatFSM_state_ascent,
    psatFSM_state_deployed,
    psatFSM_state_descent,
    psatFSM_state_landing,
    psatFSM_state_recovery,
    psatFSM_state_lowpwr,
    psatFSM_state_error,
} psatFSM_state_e;

typedef enum {
    psatFSM_eventType_startPrelaunch,
    psatFSM_eventType_prelaunchComplete,
    psatFSM_eventType_deploymentConfirmed,
    psatFSM_eventType_timer1s,
    psatFSM_eventType_timer5s,
    psatFSM_eventType_timer10s,
    psatFSM_eventType_cameraOn,
    psatFSM_eventType_cameraStop,
    psatFSM_eventType_unfoldMechanism,
    psatFSM_eventType_landingConfirmed,
    psatFSM_eventType_audioOn,
    psatFSM_eventType_audioOff,
    psatFSM_eventType_audioBeep,
    psatFSM_eventType_loraCommand,
    psatFSM_eventType_error,
} psatFSM_eventType_e;

typedef struct {
    int global;
    psatFSM_eventType_e type;
} psatFSM_event_t;