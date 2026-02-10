#include "sm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "on_entry_handlers.h"
#include "on_exit_handlers.h"
#include "shared_state.h"
#include "states.h"
#include "state_handlers.h"

#define QUEUE_SIZE 32
static const char*       TAG = "PSAT_FSM";

static TaskHandle_t      xHandleSM_s  = NULL;
static QueueHandle_t     eventQueue_s = NULL;
static SemaphoreHandle_t stateMutex_s = NULL;

void psatFSM_postEvent(const psatFSM_event_t* event)
{
    if (!eventQueue_s || !event)
        return;

    if (xQueueSend(eventQueue_s, event, 0) != pdPASS)
    {
        ESP_LOGW(TAG,
                 "the event queue is full or something went wrong "
                 "event dropped");
    }
}

void psatFSM_postEventISR(const psatFSM_event_t* event)
{
    if (!eventQueue_s || !event)
        return;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t res = xQueueSendFromISR(eventQueue_s, event,
                                       &xHigherPriorityTaskWoken);
    if (res != pdPASS) {} // TODO: implement error handling for this
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void psatFSM_setCurrentState(psatFSM_state_e newState)
{
    if (stateMutex_s &&
        xSemaphoreTake(stateMutex_s, portMAX_DELAY) == pdTRUE)
    {
        psat_globalState.currentFSMState = newState;
        xSemaphoreGive(stateMutex_s);
    }
}

psatFSM_state_e psatFSM_getCurrentState()
{
    psatFSM_state_e state = psatFSM_state_error;

    if (stateMutex_s &&
        xSemaphoreTake(stateMutex_s, portMAX_DELAY) == pdTRUE)
    {
        state = psat_globalState.currentFSMState;
        xSemaphoreGive(stateMutex_s);
    }

    return state;
}

void psatFSM_mainLoop(void* arg)
{
    psatFSM_event_t currentEvent = {0};
    psatFSM_state_e nextState    = psat_globalState.currentFSMState;

    while (1)
    {
        if (!xQueueReceive(eventQueue_s, &currentEvent,
                           portMAX_DELAY))
            continue; // if no event received do nothing

        if (currentEvent.type == psatFSM_eventType_error)
        {
            ESP_LOGI("FSM", "Recieved Error Event :(");
            psatFsm_state_t* errorState =
                psatFSM_getState(psatFSM_state_error);

            errorState->onStateEntry();
            nextState = errorState->stateHandler(&currentEvent);
            errorState->onStateExit();

            psatFSM_setCurrentState(nextState);

            taskYIELD();
            continue;
        }

        if (currentEvent.global)
        {
            ESP_LOGI("FSM", "Received Global Event: %s",
                     psatFSM_eventTypeToString(currentEvent.type));
            // for events that are state agnostic, so will be handled or run
            // regardless of what state we are in
            globalEventHandler(&currentEvent);
        }

        ESP_LOGI("FSM", "Received Event: %s",
                 psatFSM_eventTypeToString(currentEvent.type));

        psatFSM_state_e  currentState = psatFSM_getCurrentState();
        psatFsm_state_t* currentStateDefinition =
            psatFSM_getState(currentState);
        if (currentStateDefinition->stateHandler)
        {
            nextState =
                currentStateDefinition->stateHandler(&currentEvent);
        }
        else
        {
            ESP_LOGE(TAG, "No handler for state %d", currentState);
            nextState = psatFSM_state_error;
        }

        if (nextState != currentState)
        {
            ESP_LOGI("FSM",
                     "Transitioning from state: %s to new state: %s",
                     psatFSM_stateToString(
                         psat_globalState.currentFSMState),
                     psatFSM_stateToString(nextState));

            // run exit function for current state
            currentStateDefinition->onStateExit();

            // change the state to the next state
            psatFSM_setCurrentState(nextState);

            // call the entry function for the new state
            psatFsm_state_t* nextStateDefinition =
                psatFSM_getState(nextState);
            nextStateDefinition->onStateEntry();
        }

        taskYIELD();
    }
}

void psatFSM_start()
{
    eventQueue_s = xQueueCreate(QUEUE_SIZE, sizeof(psatFSM_event_t));
    stateMutex_s = xSemaphoreCreateMutex();

    if (!eventQueue_s || !stateMutex_s)
    {
        ESP_LOGE(TAG, "Failed to create queue or mutex");
        return;
    }

    psatFSM_event_t startupEvent = {
        .type = psatFSM_eventType_startPrelaunch, .global = false};
    psatFSM_postEvent(&startupEvent);

    psatFSM_mainLoop(NULL);
}

void psatFSM_startAsTask()
{
    xTaskCreate(psatFSM_start, "fsm_task", 4096, NULL, 10,
                &xHandleSM_s);
}

void psatFSM_killTask()
{
    if (xHandleSM_s != NULL)
    {
        vTaskDelete(xHandleSM_s);
        xHandleSM_s = NULL;
    }
    if (eventQueue_s != NULL)
    {
        vQueueDelete(eventQueue_s);
        eventQueue_s = NULL;
    }
    if (stateMutex_s != NULL)
    {
        vSemaphoreDelete(stateMutex_s);
        stateMutex_s = NULL;
    }
}

void psatFSM_stateOverride(psatFSM_state_e newState)
{
    ESP_LOGI(TAG, "Overriding current state to %i", newState);
    psatFSM_setCurrentState(newState);

    psatFsm_state_t* newStateDefinition = psatFSM_getState(newState);
    newStateDefinition->onStateEntry();
}

void psatFSM_stateNext()
{
    ESP_LOGI(TAG, "Moving to the next state");

    psatFSM_state_e  currentState = psatFSM_getCurrentState();
    psatFsm_state_t* currentStateDefinition =
        psatFSM_getState(currentState);
    currentStateDefinition->onStateExit();

    psatFSM_state_e nextState =
        currentStateDefinition->defaultNextState;
    psatFSM_setCurrentState(nextState);

    psatFsm_state_t* nextStateDefinition =
        psatFSM_getState(nextState);
    nextStateDefinition->onStateEntry();
}

void psatFSM_stateFastForward(psatFSM_state_e target)
{
    ESP_LOGI(TAG, "Fast forwarding current state to %i", target);
    psatFSM_state_e currentState = psatFSM_getCurrentState();
    if (target < currentState)
    {
        return;
    }

    while (currentState != target)
    {
        psatFSM_stateNext();
        currentState = psatFSM_getCurrentState();
    }
}