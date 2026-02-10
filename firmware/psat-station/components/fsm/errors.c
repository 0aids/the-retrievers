#include "errors.h"
#include "states.h"
#include "esp_log.h"
#include "components.h"
#include "math.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#define ERROR_IGNORE_THRESHOLD 20000000 // 20 seconds

static int       errorCounter = 0; // used for error id

psatErr_error_t  errorBuffer[NUMBER_ERRORS_STORED];
circularBuffer_t errorCircularBuffer = {
    .head  = 0,
    .count = 0,
};

void psatErr_bufferInsert(psatErr_error_t error)
{
    errorBuffer[errorCircularBuffer.head] = error;
    errorCircularBuffer.head =
        (errorCircularBuffer.head + 1) % NUMBER_ERRORS_STORED;

    if (errorCircularBuffer.count < NUMBER_ERRORS_STORED)
    {
        errorCircularBuffer.count++;
    }
}

int psatErr_bufferCopyOrdered(
    psatErr_error_t errorOrderedBuffer[NUMBER_ERRORS_STORED])
{
    int index =
        (errorCircularBuffer.head - 1 + NUMBER_ERRORS_STORED) %
        NUMBER_ERRORS_STORED;
    for (int i = 0; i < errorCircularBuffer.count; i++)
    {
        errorOrderedBuffer[i] = errorBuffer[index];
        index =
            (index - 1 + NUMBER_ERRORS_STORED) % NUMBER_ERRORS_STORED;
    }

    return errorCircularBuffer.count;
}

psatErr_error_t* psatErr_getMostRecentError()
{
    int index =
        (errorCircularBuffer.head - 1 + NUMBER_ERRORS_STORED) %
        NUMBER_ERRORS_STORED;

    return &errorBuffer[index];
}

psatErr_error_t* psatErr_getErrorById(int id)
{
    for (int i = 0; i < NUMBER_ERRORS_STORED; i++)
    {
        if (errorBuffer[i].id == id)
            return &errorBuffer[i];
    }
    return NULL;
}

void psatErr_postError(psatErr_code_e      errorCode,
                       psatFSM_component_e originComponent,
                       psatFSM_state_e     originState)
{
    psatErr_error_t error = {
        .id              = errorCounter++,
        .code            = errorCode,
        .originComponent = originComponent,
        .originState     = originState,
        .timestamp       = esp_timer_get_time(),
    };

    psatErr_bufferInsert(error);

    psatFSM_event_t event = {.global = true,
                             .type   = psatFSM_eventType_error,
                             .arg    = error.id};
    psatFSM_postEvent(&event);
}

bool psatErr_attemptRecovery(psatFSM_component_e componentId,
                             psatErr_error_t     error)
{
    static const char*   TAG = "PSAT_FSM-Error";

    psatFSM_component_t* component =
        psatFSM_getComponent(componentId);
    if (!component)
        return false;

    int64_t lastErrTimestamp =
        component->recoveryContext.last_recovery_timestamp;

    int64_t currentTime = esp_timer_get_time();
    component->recoveryContext.last_recovery_timestamp = currentTime;

    if ((currentTime - lastErrTimestamp) >= ERROR_IGNORE_THRESHOLD)
    {
        component->recoveryContext.retry_count = 0;
    }

    if (error.id > 0)
    {
        psatErr_error_t* lastErr = psatErr_getErrorById(error.id - 1);
        if (lastErr != NULL && error.code != lastErr->code)
        {
            component->recoveryContext.retry_count = 0;
        }
    }

    int      currentRetry = component->recoveryContext.retry_count++;

    uint32_t backoffDelay = pow(3, currentRetry) *
        1000; // so its exponential longer each time

    ESP_LOGW(TAG, "Attempting to recover %s (retry %d)",
             psatFSM_componentToString(componentId), currentRetry);

    if (currentRetry >= 3)
        return false;
    else if (currentRetry == 2)
    {
        psatFsm_state_t* originState =
            psatFSM_getState(error.originState);

        if (originState)
        {

            vTaskDelay(pdMS_TO_TICKS(backoffDelay));
            originState->onStateExit();
            vTaskDelay(pdMS_TO_TICKS(backoffDelay));
            originState->onStateEntry();
        }
    }

    if (component->recover)
    {
        component->recover();
        return true;
    }

    vTaskDelay(pdMS_TO_TICKS(backoffDelay));

    if (component->type == psatFSM_componentType_task &&
        component->stop)
        component->stop();

    if (component->deinit)
        component->deinit();

    vTaskDelay(pdMS_TO_TICKS(backoffDelay));

    if (component->init)
        component->init();

    if (component->type == psatFSM_componentType_task &&
        component->start)
        component->start();

    return true;
}
// how this works rn:
// if retry = 0 (default so first time)
//  run recover, deinit and init (+ stop/start if applicable)
// if retry = 1 (second time)
//  run recover, deinit and init (+ stop/start if applicable)
// if retry = 2 (third time)
//  run state on exit, state on entry, recover, deinit and init (+ stop/start if applicable)
// if retry = 3 (forth time)
//  transition to permanent error state
//  in this state we can simply disable a component
//  and then put back into whatever state we want