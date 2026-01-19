#pragma once

#include "shared_state.h"

psatFSM_state_e psatFSM_getCurrentState();
void psatFSM_setCurrentState(psatFSM_state_e newState);
void psatFSM_postEvent(const psatFSM_event_t* event);
void psatFSM_postEventISR(const psatFSM_event_t* event);
void psatFSM_mainLoop(void* arg);
void psatFSM_start();
void psatFSM_startAsTask();
void psatFSM_killTask();
void psatFSM_stateOverride(psatFSM_state_e newState);
void psatFSM_stateNext();
void psatFSM_stateFastForward(psatFSM_state_e target);