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
        .start     = start,
        .stop      = stop,
        .preflight = preflight
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

    uint16_t             preflightTest_output = 0;

    psatFSM_component_t* component;

    for (int componentId = psatFSM_component__COUNT - 1;
         componentId >= 0; componentId--)
    {
        component = psatFSM_getComponent(componentId);
        if (component == NULL)
        {
            continue;
        }

        if (!(component->preflight))
        {
            ESP_LOGW(
                TAG,
                "Component %s, does not have a preflight function",
                psatFSM_componentToString(componentId));
            continue;
        }

        ESP_LOGI(TAG, "Running Preflight Test For: %s",
                 psatFSM_componentToString(componentId));

        bool preflightResult = component->preflight();
        preflightTest_output += preflightResult;
        preflightTest_output <<= 1;

        ESP_LOGI(TAG, "Preflight Result for %s: %i",
                 psatFSM_componentToString(componentId),
                 preflightResult);
    }

    return preflightTest_output;
}