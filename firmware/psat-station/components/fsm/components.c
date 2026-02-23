// TODO: each component needs init and deinit
// TODO: Modify to account for start tasks and kill tasks
// TODO: component will know if it is a task based on or not, and if it is, it will stoptask, deinit, init and start task
// TODO: if its not then just deinit, init
// TODO: also if recovery function is defined we just run that instead

#include "esp_log.h"
#include "shared_state.h"
#include <stdbool.h>

#include "components.h"

static const char*         TAG = "PSAT_COMPONENT";

static psatFSM_component_t componentTable[psatFSM_component__COUNT] =
    {0};

void psatFSM_registerComponent(
    psatFSM_component_e componentId, psatFSM_componentType_e type,
    void (*init)(void), void (*deinit)(void), void (*recover)(void),
    void (*start)(void), void (*stop)(void), bool (*preflight)(void))
{
    if (componentId < 0 || componentId >= psatFSM_component__COUNT)
    {
        ESP_LOGW(TAG, "Invalid component Id Provided (%i)",
                 componentId);
        return;
    }

    psatFSM_component_t component = {
        .init    = init,
        .deinit  = deinit,
        .status  = psatFSM_componentStatus_disabled,
        .type    = type,
        .recover = recover,
        .recoveryContext =
            {
                              .last_recovery_timestamp = 0,
                              .retry_count             = 0,
                              },
        .start = start,
        .stop  = stop
    };

    componentTable[componentId] = component;

    ESP_LOGI(TAG, "%s Component Registered",
             psatFSM_componentToString(componentId));
}

void psatFSM_initComponent(psatFSM_component_e componentId)
{
    if (componentId < 0 || componentId >= psatFSM_component__COUNT)
    {
        ESP_LOGW(TAG, "Invalid component Id Provided (%i)",
                 componentId);
        return;
    }

    psatFSM_component_t* component = &componentTable[componentId];

    if (component->status == psatFSM_componentStatus_unRegistered)
    {
        ESP_LOGW(TAG, "Component %s is unregistered",
                 psatFSM_componentToString(componentId));
        return;
    }

    component->init();
    component->status = psatFSM_componentStatus_enabled;

    ESP_LOGI(TAG,
             "%s component has been initialised and is now enabled",
             psatFSM_componentToString(componentId));
}

void psatFSM_deinitComponent(psatFSM_component_e componentId)
{
    if (componentId < 0 || componentId >= psatFSM_component__COUNT)
    {
        ESP_LOGW(TAG, "Invalid component Id Provided (%i)",
                 componentId);
        return;
    }

    psatFSM_component_t* component = &componentTable[componentId];

    if (component->status == psatFSM_componentStatus_unRegistered)
    {
        ESP_LOGW(TAG, "Component %s is unregistered",
                 psatFSM_componentToString(componentId));
        return;
    }

    component->deinit();
    component->status = psatFSM_componentStatus_disabled;

    ESP_LOGI(
        TAG,
        "%s component has been deinitialised and is now disabled",
        psatFSM_componentToString(componentId));
}

void psatFSM_initAll()
{
    for (int componentId = 0; componentId < psatFSM_component__COUNT;
         componentId++)
    {
        psatFSM_initComponent(componentId);
    }
}

psatFSM_component_t*
psatFSM_getComponent(psatFSM_component_e componentId)
{
    if (componentId < 0 || componentId >= psatFSM_component__COUNT)
    {
        ESP_LOGW(TAG, "Invalid component Id Provided (%i)",
                 componentId);
        return NULL;
    }

    return &componentTable[componentId];
}

void psatFSM_enableComponent(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (!component ||
        component->status == psatFSM_componentStatus_enabled)
        return;

    psatFSM_initComponent(id);

    if (component->type == psatFSM_componentType_task &&
        component->start)
        component->start();
}

void psatFSM_disableComponent(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (!component ||
        component->status == psatFSM_componentStatus_disabled)
        return;

    if (component->type == psatFSM_componentType_task &&
        component->stop)
        component->stop();

    psatFSM_deinitComponent(id);
}

uint16_t psatFSM_preflightTest()
{

    uint16_t            preflightTest_output = 0;

    psatFSM_component_t component;

    for (int componentId = psatFSM_component__COUNT - 1;
         componentId >= 0; componentId--)
    {
        component = componentTable[componentId];
        preflightTest_output += component.preflight();
        preflightTest_output <<= 1;
    }

    return preflightTest_output;
}