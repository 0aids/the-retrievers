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
    X(psatFSM_state_permanentError)                                  \
                                                                     \
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
                                                                     \
    X(psatFSM_eventType__COUNT)

#define PSAT_FSM_COMPONENTS_LIST                                     \
    X(psatFSM_component_ldr)                                         \
    X(psatFSM_component_gps)                                         \
    X(psatFSM_component_buzzers)                                     \
    X(psatFSM_component_buttons)                                     \
    X(psatFSM_component_battery)                                     \
    X(psatFSM_component_camera)                                      \
    X(psatFSM_component_servo)                                       \
    X(psatFSM_component_timers)                                      \
    X(psatFSM_component_bmp280)                                      \
    X(psatFSM_component_bmi323)                                      \
    X(psatFSM_component_highg)                                       \
    X(psatFSM_component__COUNT)

#define PSAT_FSM_COMPONENTS_TYPES_LIST                               \
    X(psatFSM_componentType_task)                                    \
    X(psatFSM_componentType_normal)                                  \
    X(psatFSM_componentType_multiple)

// TODO: suffixes for the error codes, currently ive just made them all _failed
#define PSAT_ERR_CODE_LIST                                           \
    X(psatErr_none)                                                  \
                                                                     \
    X(psatErr_ldr_calibrationInit_failed)                            \
    X(psatErr_ldr_adcInit_failed)                                    \
    X(psatErr_ldr_adcConfig_failed)                                  \
    X(psatErr_ldr_readRawValue_failed)                               \
    X(psatErr_ldr_voltage_failed)                                    \
    X(psatErr_ldr_openMemStr_failed)                                 \
    X(psatErr_ldr_adcDelUnit_failed)                                 \
    X(psatErr_ldr_caliDeleteScheme_failed)                           \
                                                                     \
    X(psatErr_bmp280_i2cBusAddition_failed)                          \
    X(psatErr_bmp280_powerConfig_failed)                             \
    X(psatErr_bmp280_measurementConfig_failed)                       \
    X(psatErr_bmp280_calibration_failed)                             \
    X(psatErr_bmp280_i2cBusRemoval_failed)                           \
    X(psatErr_bmp280_dataRead_failed)                                \
    X(psatErr_bmp280_reset_failed)                                   \
                                                                     \
    X(psatErr_gps_uartConfig_failed)                                 \
    X(psatErr_gps_uartPinSet_failed)                                 \
    X(psatErr_gps_uartDriverInstall_failed)                          \
    X(psatErr_gps_uartDriverUninstall_failed)                        \
    X(psatErr_gps_startTask_failed)                                  \
                                                                     \
    X(psatErr_buzzer_gpioConfig_failed)                              \
    X(psatErr_buzzer_gpioInitLevel_failed)                           \
    X(psatErr_buzzer_gpioDeinitLevel_failed)                         \
    X(psatErr_buzzer_gpioReset_failed)                               \
    X(psatErr_buzzer_turnOn_failed)                                  \
    X(psatErr_buzzer_turnOff_failed)                                 \
    X(psatErr_timer_init_failed)                                     \
                                                                     \
    X(psatErr_bmi323_i2cBusAddition_failed)                          \
    X(psatErr_bmi323_FIFOConfig_failed)                              \
    X(psatErr_bmi323_AccConfig_failed)                               \
    X(psatErr_bmi323_GyroConfig_failed)                              \
    X(psatErr_bmi323_ChipError)                                      \
    X(psatErr_bmi323_i2cBusRemoval_failed)                           \
    X(psatErr_bmi323_DataRead_failed)                                \
    X(psatErr_bmi323_reset_failed)                                   \
                                                                     \
    X(psatErr_highg_i2cBusAddition_failed)                           \
    X(psatErr_highg_AccConfig_failed)                                \
    X(psatErr_highg_i2cBusRemoval_failed)                            \
    X(psatErr_highg_DataRead_failed)                                 \
    X(psatErr_highg_reset_failed)                                    \
                                                                     \
    X(psatErr__COUNT)

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
    PSAT_FSM_COMPONENTS_LIST
} psatFSM_component_e;
typedef enum
{
    PSAT_FSM_COMPONENTS_TYPES_LIST
} psatFSM_componentType_e;
typedef enum
{
    PSAT_ERR_CODE_LIST
} psatErr_code_e;
#undef X

// For component statusses
typedef struct
{
    // prelaunch:
    uint16_t enabled;

    // During mission:
    uint16_t init;
    uint16_t task;
    uint16_t error;
} psatFSM_componentConfig_t;

// STATE MACHINE EVENT
typedef struct
{
    int                 global;
    psatFSM_eventType_e type;
    int                 arg; // for errors
} psatFSM_event_t;

// GLOBAL STATE
typedef struct
{
    psatFSM_state_e currentFSMState;
    psatFSM_state_e prevFSMState;
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
} psatFSM_state_t;

// Component Definition
typedef struct
{
    int     retry_count;
    int64_t last_recovery_timestamp;
} psatFSM_componentRecoveryContext_t;

typedef struct
{
    void (*init)(void);
    void (*deinit)(void);

    // for task based components eg gps:
    void (*start)(void);
    void (*stop)(void);

    void (*recover)(void);

    bool (*preflight)(void);

    psatFSM_componentType_e            type;
    psatFSM_componentRecoveryContext_t recoveryContext;

} psatFSM_component_t;

// Error Defintion
typedef struct
{
    int                 id;
    psatErr_code_e      code;
    int64_t             timestamp;
    psatFSM_component_e originComponent;
    psatFSM_state_e     originState;
    // maybe i could add a severerity here
} psatErr_error_t;

// BMP280 (PRESSURE SENSOR) STATE AND PREFLIGHT
typedef struct
{
    bool I2C_initalised;
    bool powerConfigured;
    bool measurementConfigured;
    bool calibrated;

} bmp280_status_t;

typedef struct
{
    float   temperature;
    int     pressure;

    double  altitude;

    int64_t time;

} bmp280_data_t;

// BMI323 IMU (LOW-G ACCELEROMETER & GRYOSCOPE) DATA

typedef struct
{
    //acc in m/s/s and gyro in degree/s
    double accX;
    double accY;
    double accZ;

    double accMag;

    double gyroX;
    double gyroY;
    double gyroZ;

    //time is in uS since esp power on using esp_timer_get_time()
    int64_t time;

} bmi323_data_t;

// HIGH G ACCELEROMETER DATA
typedef struct
{
    //acc in m/s/s
    double accX;
    double accY;
    double accZ;

    double accMag;

    //time is in uS since esp power on using esp_timer_get_time()
    int64_t time;

} highg_data_t;

//SENSOR DATA STRUCT

typedef struct
{

    float temperature;
    int pressure;

    double  barometric_altitude;
    double  altitude;

    double  accX;
    double  accY;
    double  accZ;

    double orintationX; //Yaw
    double orintationY; //Pitch
    double orintationZ; //Roll

    double velX;
    double velY;
    double velZ;

    double roll;
    double pitch;

    double gyroX;
    double gyroY;
    double gyroZ;

    double batterVoltage;
    int ldrVoltage;

    int64_t velTime;
    int64_t oriTime;
    int64_t barometerTime;

    //adjusted gps based accelerometer and barometer?

} sensorData_t;

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

    int     linesRecieved;
} gps_data_t;

typedef struct {
   uint8_t data[14]; 
} gps_psatTelemetryPacket_t;

// FUNCTIONS TO CONVERT ENUM TO A STRING
static inline const char* psatFSM_stateToString(psatFSM_state_e state)
{
    switch (state)
    {
#define X(name)                                                      \
    case name: return #name;
        PSAT_FSM_STATE_LIST
#undef X
        default: return "psatFSM_state_invalid";
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
        default: return "psatFSM_eventType_invalid";
    }
}

static inline const char*
psatFSM_componentToString(psatFSM_component_e type)
{
    switch (type)
    {
#define X(name)                                                      \
    case name: return #name;
        PSAT_FSM_COMPONENTS_LIST
#undef X
        default: return "psatFSM_component_invalid";
    }
}

static inline const char* psatErr_codeToString(psatErr_code_e err)
{
    switch (err)
    {
#define X(errType)                                                   \
    case errType: return #errType; break;
        PSAT_ERR_CODE_LIST
#undef X
        default: return "psatErr_invalid";
    }
}
