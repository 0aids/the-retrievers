#ifndef SM_TYPES_H
#define SM_TYPES_H

typedef enum {
    STATE_PRELAUNCH,
    STATE_ASCENT,
    STATE_DEPLOYED,
    STATE_DESCENT,
    STATE_LANDING,
    STATE_RECOVERY,
    STATE_LOWPWR,
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
    EVENT_UNFOLD_MECHANISM,
    EVENT_LANDING_CONFIRMED,
    EVENT_AUDIO_ON,
    EVENT_AUDIO_OFF,
    EVENT_AUDIO_BEEP,
    EVENT_LORA_COMMAND,
    EVENT_ERROR,
} fsm_event_type_t;

typedef union {
    int num;
    char *str;
    void* ptr;
} fsm_event_data_t;

typedef struct {
    int global;
    fsm_event_type_t type;
    fsm_event_data_t data;
} fsm_event_t;

#endif