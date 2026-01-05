#ifndef SM_HANDLERS_H
#define SM_HANDLERS_H

#include "sm_types.h"

fsm_state_t state_prelaunch(const fsm_event_t* e);
fsm_state_t state_ascent(const fsm_event_t* e);
fsm_state_t state_deployed(const fsm_event_t* e);
fsm_state_t state_descent(const fsm_event_t* e);
fsm_state_t state_landing(const fsm_event_t* e);
fsm_state_t state_recovery(const fsm_event_t* e);
fsm_state_t state_error(const fsm_event_t* e);

#endif