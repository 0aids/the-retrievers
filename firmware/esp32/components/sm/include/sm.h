#ifndef SM_H
#define SM_H

#include "sm_types.h"

void fsm_start(void);
fsm_state_t fsm_get_current_state();
void fsm_post_event(const fsm_event_t* event);
void fsm_post_event_from_isr(const fsm_event_t* evt);

#endif