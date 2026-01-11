#pragma once

#include "shared_state.h"

void psatFSM_setCurrentState(psatFSM_state_e newState);
void psatFSM_postEvent(const psatFSM_event_t* event);
void psatFSM_postEventISR(const psatFSM_event_t* event);
void psatFSM_mainLoop(void* arg);
void psatFSM_start();
void psatFSM_startAsTask();
void psatFSM_killTask();