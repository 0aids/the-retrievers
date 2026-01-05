#ifndef SM_H
#define SM_H

#include "sm_types.h"
#include "servo.h"

void fsm_start(void);
void fsm_kill();
void fsm_set_current_state(fsm_state_t new_state);
void fsm_post_event(const fsm_event_t* event);
void fsm_post_event_from_isr(const fsm_event_t* evt);
fsm_state_t fsm_get_current_state();
servo_t fsm_get_current_servo_state();

#endif