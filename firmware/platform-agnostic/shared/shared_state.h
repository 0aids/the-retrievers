#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PSAT_FSM_STATE_LIST                                          \
    X(psatFSM_state_start)                                           \
    X(psatFSM_state_prelaunch)                                       \
    X(psatFSM_state_ascent)                                          \
    X(psatFSM_state_deployPending)                                   \
    X(psatFSM_state_deployed)                                        \
    X(psatFSM_state_descent)                                         \
    X(psatFSM_state_landing)                                         \
    X(psatFSM_state_recovery)                                        \
    X(psatFSM_state_lowPower)                                        \
    X(psatFSM_state_error)                                           \
    X(psatFSM_state__COUNT)

#define PSAT_FSM_EVENT_TYPE_LIST                                     \
    X(psatFSM_eventType_startPrelaunch)                              \
    X(psatFSM_eventType_prelaunchComplete)                           \
    X(psatFSM_eventType_deploymentPending)                           \
    X(psatFSM_eventType_deploymentTimeout)                           \
    X(psatFSM_eventType_deploymentConfirmed)                         \
    X(psatFSM_eventType_timer1s)                                     \
    X(psatFSM_eventType_timer5s)                                     \
    X(psatFSM_eventType_timer10s)                                    \
    X(psatFSM_eventType_cameraOn)                                    \
    X(psatFSM_eventType_cameraStop)                                  \
    X(psatFSM_eventType_unfoldMechanism)                             \
    X(psatFSM_eventType_landingConfirmed)                            \
    X(psatFSM_eventType_audioOn)                                     \
    X(psatFSM_eventType_audioOff)                                    \
    X(psatFSM_eventType_audioBeep)                                   \
    X(psatFSM_eventType_loraCommand)                                 \
    X(psatFSM_eventType_error)                                       \
    X(psatFSM_eventType__COUNT)

#define PSAT_ERR_TYPE_LIST                                           \
    X(noError)                                                       \
    X(ldrErr_calibrationInitErr)                                     \
    X(ldrErr_adcInitErr)                                             \
    X(ldrErr_adcConfigErr)                                           \
    X(ldrErr_readRawValueErr)                                        \
    X(ldrErr_VoltageErr)                                             \
    X(ldrErr_OpenMemStr)                                             \
    X(ldrErr_adcDelUnitErr)                                          \
    X(ldrErr_caliDeleteSchemeErr)                                    \
    X(BMP280_i2cBusAddition_failed)                                  \
    X(BMP280_powerConfig_failed)                                     \
    X(BMP280_measurementConfig_failed)                               \
    X(BMP280_calibration_failed)                                     \
    X(BMP280_i2cBusRemoval_failed)                                   \
    X(BMP280_dataRead_failed)                                        \
    X(BMP280_reset_failed)

// DEFINE ENUMS FOR EACH LIST
#define X(name) name,
typedef enum
{
    PSAT_FSM_STATE_LIST
} psatFSM_state_e;
typedef enum
{
    PSAT_FSM_EVENT_TYPE_LIST
} psatFSM_eventType_e;
typedef enum
{
    PSAT_ERR_TYPE_LIST
} psatErr_state_e;
#undef X

// STATE MACHINE EVENT
typedef struct
{
    int                 global;
    psatFSM_eventType_e type;
} psatFSM_event_t;

// GLOBAL STATE
typedef struct
{
    psatFSM_state_e currentFSMState;
} psatGlobal_state_t;

extern psatGlobal_state_t psat_globalState;

// STATE DEFINITION
typedef psatFSM_state_e (*psatFSM_stateHandler_t)(
    const psatFSM_event_t* event);
typedef struct
{
    psatFSM_state_e state;
    psatFSM_state_e defaultNextState;
    void (*onStateEntry)(void);
    psatFSM_stateHandler_t stateHandler;
    void (*onStateExit)(void);
} psatFsm_state_t;

// BMP280 PRESSURE SENSOR STATE AND PREFLIGHT
typedef struct
{
    bool I2C_initalised;
    bool powerConfigured;
    bool measurementConfigured;
    bool calibrated;

} BMP280_status_t;

typedef struct
{
    int32_t temperature;
    double  pressure;
} BMP280_preflightStatus_t;

// GPS DATA
typedef struct
{
    float   latitude;
    float   longitude;
    float   speedKnots;
    float   speedKph;
    float   courseDeg;
    float   hdop;
    float   altitude;
    float   geoidalSep;

    int32_t day;
    int32_t month;
    int32_t year;

    int32_t hours;
    int32_t minutes;
    int32_t seconds;

    int32_t fixQuality;
    int32_t satellitesTracked;
    int32_t satsInView;

    bool    positionValid; // lat, long
    bool    navValid;      // knots, kph & course
    bool    fixInfoValid;  // fix quality, sats tracked
    bool    altitudeValid; // altitude, geoidal
} gps_data_t;

// FUNCTIONS TO CONVERT ENUM TO A STRING
static inline const char* psatFSM_stateToString(psatFSM_state_e state)
{
    switch (state)
    {
#define X(name)                                                      \
    case name: return #name;
        PSAT_FSM_STATE_LIST
#undef X
        default: return "Invalid State";
    }
}

static inline const char*
psatFSM_eventTypeToString(psatFSM_eventType_e type)
{
    switch (type)
    {
#define X(name)                                                      \
    case name: return #name;
        PSAT_FSM_EVENT_TYPE_LIST
#undef X
        default: return "Invalid State";
    }
}

static inline const char* psatErr_stateToString(psatErr_state_e err)
{
    switch (err)
    {
#define X(errType)                                                   \
    case errType: return #errType; break;
        PSAT_ERR_TYPE_LIST
#undef X
    }
}