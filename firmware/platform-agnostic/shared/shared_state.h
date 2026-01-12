#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    psatFsm_state_prelaunch,
    psatFsm_state_ascent,
    psatFsm_state_deployed,
    psatFsm_state_descent,
    psatFsm_state_landing,
    psatFsm_state_recovery,
    psatFsm_state_lowpwr,
    psatFsm_state_error,
} psatFsm_state_e;

typedef enum {
    psatFsm_eventType_startPrelaunch,
    psatFsm_eventType_prelaunchComplete,
    psatFsm_eventType_deploymentConfirmed,
    psatFsm_eventType_timer1s,
    psatFsm_eventType_timer5s,
    psatFsm_eventType_timer10s,
    psatFsm_eventType_cameraOn,
    psatFsm_eventType_cameraStop,
    psatFsm_eventType_unfoldMechanism,
    psatFsm_eventType_landingConfirmed,
    psatFsm_eventType_audioOn,
    psatFsm_eventType_audioOff,
    psatFsm_eventType_audioBeep,
    psatFsm_eventType_loraCommand,
    psatFsm_eventType_error,
} psatFsm_eventType_e;

typedef struct {
    int global;
    psatFsm_eventType_e type;
} psatFsm_event_t;

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
