#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PSAT_FSM_STATE_LIST    \
    X(psatFSM_state_prelaunch) \
    X(psatFSM_state_ascent)    \
    X(psatFSM_state_deployed)  \
    X(psatFSM_state_descent)   \
    X(psatFSM_state_landing)   \
    X(psatFSM_state_recovery)  \
    X(psatFSM_state_lowpwr)    \
    X(psatFSM_state_error)

#define PSAT_FSM_EVENT_TYPE_LIST             \
    X(psatFSM_eventType_startPrelaunch)      \
    X(psatFSM_eventType_prelaunchComplete)   \
    X(psatFSM_eventType_deploymentConfirmed) \
    X(psatFSM_eventType_timer1s)             \
    X(psatFSM_eventType_timer5s)             \
    X(psatFSM_eventType_timer10s)            \
    X(psatFSM_eventType_cameraOn)            \
    X(psatFSM_eventType_cameraStop)          \
    X(psatFSM_eventType_unfoldMechanism)     \
    X(psatFSM_eventType_landingConfirmed)    \
    X(psatFSM_eventType_audioOn)             \
    X(psatFSM_eventType_audioOff)            \
    X(psatFSM_eventType_audioBeep)           \
    X(psatFSM_eventType_loraCommand)         \
    X(psatFSM_eventType_error)

#define X(name) name,
typedef enum { PSAT_FSM_STATE_LIST } psatFSM_state_e;
typedef enum { PSAT_FSM_EVENT_TYPE_LIST } psatFSM_eventType_e;
#undef X

typedef struct {
    int global;
    psatFSM_eventType_e type;
} psatFSM_event_t;

typedef struct {
    float latitude;
    float longitude;
    float speedKnots;
    float speedKph;
    float courseDeg;
    float hdop;
    float altitude;
    float geoidalSep;

    int32_t day;
    int32_t month;
    int32_t year;

    int32_t hours;
    int32_t minutes;
    int32_t seconds;

    int32_t fixQuality;
    int32_t satellitesTracked;
    int32_t satsInView;

    bool positionValid;  // lat, long
    bool navValid;       // knots, kph & course
    bool fixInfoValid;   // fix quality, sats tracked
    bool altitudeValid;  // altitude, geoidal
} gps_data_t;

// Functions to print out enums:
static inline const char* psatFSM_stateToString(psatFSM_state_e state) {
    switch (state) {
#define X(name) \
    case name:  \
        return #name;
        PSAT_FSM_STATE_LIST
#undef X
        default:
            return "Invalid State";
    }
}

static inline const char* psatFSM_eventTypeToString(psatFSM_eventType_e type) {
    switch (type) {
#define X(name) \
    case name:  \
        return #name;
        PSAT_FSM_EVENT_TYPE_LIST
#undef X
        default:
            return "Invalid State";
    }
}
