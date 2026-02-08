#pragma once

#include "shared_state.h"

void globalEventHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_startStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_prelaunchStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_ascentStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_deployPendingStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_deployedStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_descentStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_landingStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_recoveryStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_lowPowerStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_errorStateHandler(const psatFSM_event_t* event);
psatFSM_state_e
psatFSM_permanentErrorStateHandler(const psatFSM_event_t* event);
