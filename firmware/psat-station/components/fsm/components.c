#include "esp_log.h"
#include "shared_state.h"
#include <stdbool.h>

#include "components.h"

static const char*               TAG = "PSAT_COMPONENT";

static psatFSM_componentConfig_t psatConfig_s;
static psatFSM_component_t componentTable[psatFSM_component__COUNT] =
    {0};

static inline void setFlag(uint16_t* mask, psatFSM_component_e id,
                           bool enable)
{
    if (enable)
        *mask |= (1U << id);
    else
        *mask &= ~(1U << id);
}

static inline bool getFlag(uint16_t* mask, psatFSM_component_e id)
{
    return *mask & (1 << id);
}

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
    setFlag(&psatConfig_s.enabled, componentId,
            true); // enabled by default

    ESP_LOGI(TAG, "%s Component Registered",
             psatFSM_componentToString(componentId));
}

void psatFSM_markComponentEnabled(psatFSM_component_e id, bool enable)
{
    setFlag(&psatConfig_s.enabled, id, enable);
    ESP_LOGI(TAG, "%s Component Enabled: %i",
             psatFSM_componentToString(id), enable);
}

void psatFSM_enableComponent(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (component == NULL)
        return;

    if (getFlag(&psatConfig_s.init, id) == true)
        return; // component already inited

    componentTable[id].init();
    setFlag(&psatConfig_s.init, id, true);

    ESP_LOGI(TAG,
             "%s component has been initialised and is now enabled",
             psatFSM_componentToString(id));
}

void psatFSM_disableComponent(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (component == NULL)
        return;

    if (getFlag(&psatConfig_s.task, id))
        psatFSM_stopComponentTask(id);

    if (!getFlag(&psatConfig_s.init, id))
        return; // component was never inited

    componentTable[id].deinit();
    setFlag(&psatConfig_s.init, id, false);

    ESP_LOGI(
        TAG,
        "%s component has been deinitialised and is now disabled",
        psatFSM_componentToString(id));
}

void psatFSM_startComponentTask(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (component == NULL)
        return;

    if (!getFlag(&psatConfig_s.init, id))
        return;

    if (!component->start)
        return;

    component->start();
    setFlag(&psatConfig_s.task, id, true);

    ESP_LOGI(TAG, "%s component task has been started",
             psatFSM_componentToString(id));
}

void psatFSM_stopComponentTask(psatFSM_component_e id)
{
    psatFSM_component_t* component = psatFSM_getComponent(id);
    if (component == NULL)
        return;

    if (!getFlag(&psatConfig_s.task, id))
        return;

    if (component->stop)
        component->stop();

    setFlag(&psatConfig_s.task, id, false);

    ESP_LOGI(TAG, "%s component task has been stopped",
             psatFSM_componentToString(id));
}

void psatFSM_initAll()
{
    for (int componentId = 0; componentId < psatFSM_component__COUNT;
         componentId++)
    {

        if (getFlag(&psatConfig_s.enabled, componentId))
        {
            psatFSM_enableComponent(componentId);
        }
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

void psatFSM_getComponentConfig(psatFSM_componentConfig_t* out)
{
    // TODO: Add semaphores
    memcpy(out, &psatConfig_s, sizeof(psatConfig_s));
}

uint16_t psatFSM_preflightTest(bool useCache)
{
    static uint16_t lastResult      = 0;
    static bool     hasCachedResult = false;

    if (useCache && hasCachedResult)
    {
        ESP_LOGI(TAG, "Returning cached preflight result");
        return lastResult;
    }

    uint16_t             preflightTest_output = 0;
    psatFSM_component_t* component;

    for (int componentId = 0; componentId < psatFSM_component__COUNT;
         componentId++)
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
        setFlag(&preflightTest_output, componentId, preflightResult);

        ESP_LOGI(TAG, "Preflight Result for %s: %i",
                 psatFSM_componentToString(componentId),
                 preflightResult);
    }

    lastResult      = preflightTest_output;
    hasCachedResult = true;

    return preflightTest_output;
}