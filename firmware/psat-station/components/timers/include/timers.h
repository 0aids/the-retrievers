#pragma once

#include <stdint.h>

#include "state.h"

typedef enum {
    timer_timerId_1s,
    timer_timerId_5s,
    timer_timerId_10s,
    timer_timerId_mechanical,
    timer_timerId__COUNT  // since this one is last item in the enum it will be
                          // equal to number of timers we got
} timer_id_e;

typedef struct {
    uint32_t period_ms;
    psatFSM_eventType_e event;
    const char* name;
} timer_config_t;

void timer_init(void);
void timer_start(timer_id_e id);
void timer_stop(timer_id_e id);
void timer_startOnce(timer_id_e id, uint32_t delay_ms);
