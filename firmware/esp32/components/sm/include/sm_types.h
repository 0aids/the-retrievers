#ifndef SM_TYPES_H
#define SM_TYPES_H

#include "gps_state.h"

typedef enum {
    STATE_PRELAUNCH,
    STATE_ASCENT,
    STATE_DEPLOYED,
    STATE_DECENT,
    STATE_LANDING,
    STATE_RECOVERY,
    STATE_ERROR,
} fsm_state_t;

typedef enum {
    EVENT_START_PRELAUNCH,
    EVENT_PRELAUNCH_COMPLETE,
    EVENT_DEPLOYMENT_CONFIRMED,
    EVENT_TIMER_1S,
    EVENT_TIMER_5S,
    EVENT_TIMER_10S,
    EVENT_CAMERA_ON,
    EVENT_CAMERA_STOP,
    EVENT_FOLDING_TIMER_REACHED,
    EVENT_LANDING_CONFIRMED,
    EVENT_AUDIO_ON,
    EVENT_AUDIO_OFF,
    EVENT_AUDIO_BEEP,
    EVENT_AUDIO_FUN,
    EVENT_LORA_COMMAND,
    EVENT_ERROR,
} fsm_event_type_t;

typedef union {
    int number;
    char str[67];  // idk man this placeholder, maybe like the lora command eg
    // smth like "AUDIO_ON"
    void* ptr;  // catchall type shit
} fsm_event_data_t;

typedef struct {
    fsm_event_type_t type;
    fsm_event_data_t data;  // if we wanna put some data along with the event,
                            // eg gps or the specific lora command
} fsm_event_t;

#endif