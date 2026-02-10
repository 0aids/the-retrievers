#pragma once

#include "shared_state.h"

void psatFSM_registerComponent(psatFSM_component_e componentId,
                               void (*init)(void),
                               void (*deinit)(void),
                               void (*recover)(void));
void psatFSM_initComponent(psatFSM_component_e componentId);
void psatFSM_deinitComponent(psatFSM_component_e componentId);
void psatFSM_initAll();
psatFSM_component_t*
psatFSM_getComponent(psatFSM_component_e componentId);