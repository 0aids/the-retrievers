#pragma once

#include "shared_state.h"

void psatFSM_registerComponent(
    psatFSM_component_e componentId, psatFSM_componentType_e type,
    void (*init)(void), void (*deinit)(void), void (*recover)(void),
    void (*start)(void), void (*stop)(void), bool (*preflight)(void));

void psatFSM_markComponentEnabled(psatFSM_component_e id, bool enable);

void psatFSM_enableComponent(psatFSM_component_e id);
void psatFSM_disableComponent(psatFSM_component_e id);

void psatFSM_startComponentTask(psatFSM_component_e id);
void psatFSM_stopComponentTask(psatFSM_component_e id);

void psatFSM_initAll();

psatFSM_component_t*
psatFSM_getComponent(psatFSM_component_e componentId);
void psatFSM_getComponentConfig(psatFSM_componentConfig_t* out);

uint16_t psatFSM_preflightTest(bool useCache);
