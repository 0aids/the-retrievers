#include "sm_types.h"

#include <stdio.h>

static const char* STATE_NAMES[STATE__COUNT] = {
    [STATE_PRELAUNCH] = "PRELAUNCH", [STATE_ASCENT] = "ASCENT",
    [STATE_DEPLOYED] = "DEPLOYED",   [STATE_DECENT] = "DECENT",
    [STATE_LANDING] = "LANDING",     [STATE_RECOVERY] = "RECOVERY",
    [STATE_ERROR] = "ERROR"};

const char* fsm_state_to_string(fsm_state_t state) {
    if (state < 0 || state >= STATE__COUNT) return "UNKNOWN STATE";
    return STATE_NAMES[state];
}

void fsm_print_transition(fsm_state_t old_state, fsm_state_t new_state) {
    printf("\nTRANSITIONING STATE FROM %s TO %s\n\n",
           fsm_state_to_string(old_state), fsm_state_to_string(new_state));
}

static const char* EVENT_NAMES[EVENT__COUNT] = {
    [EVENT_START_PRELAUNCH] = "START_PRELAUNCH",
    [EVENT_PRELAUNCH_COMPLETE] = "PRELAUNCH_COMPLETE",
    [EVENT_DEPLOYMENT_CONFIRMED] = "DEPLOYMENT_CONFIRMED",
    [EVENT_TIMER_1S] = "TIMER_1S",
    [EVENT_TIMER_5S] = "TIMER_5S",
    [EVENT_TIMER_10S] = "TIMER_10S",
    [EVENT_CAMERA_ON] = "CAMERA_ON",
    [EVENT_CAMERA_STOP] = "CAMERA_STOP",
    [EVENT_FOLDING_TIMER_REACHED] = "FOLDING_TIMER_REACHED",
    [EVENT_LANDING_CONFIRMED] = "LANDING_CONFIRMED",
    [EVENT_AUDIO_ON] = "AUDIO_ON",
    [EVENT_AUDIO_OFF] = "AUDIO_OFF",
    [EVENT_AUDIO_BEEP] = "AUDIO_BEEP",
    [EVENT_AUDIO_FUN] = "AUDIO_FUN",
    [EVENT_LORA_COMMAND] = "LORA_COMMAND",
    [EVENT_ERROR] = "ERROR"};

const char* fsm_event_to_string(fsm_event_type_t event) {
    if (event < 0 || event >= EVENT__COUNT) return "UNKOWN EVENT";
    return EVENT_NAMES[event];
}
