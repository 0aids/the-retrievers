#include "sm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "on_entry_handlers.h"
#include "on_exit_handlers.h"
#include "shared_state.h"
#include "state_handlers.h"

#define QUEUE_SIZE 32
static const char* TAG = "PSAT_FSM";

static TaskHandle_t xHandleSM_s = NULL;
static QueueHandle_t eventQueue_s = NULL;
static SemaphoreHandle_t stateMutex_s = NULL;

static psatFSM_state_t stateTable[] = {
    [psatFSM_state_prelaunch] = {.state = psatFSM_state_prelaunch,
                                 .defaultNextState = psatFSM_state_ascent,
                                 .onStateEntry = psatFSM_prelaunchEntryHandler,
                                 .onStateExit = psatFSM_prelaunchExitHandler,
                                 .stateHandler = psatFSM_prelaunchStateHandler},
    [psatFSM_state_ascent] = {.state = psatFSM_state_ascent,
                              .defaultNextState = psatFSM_state_deployPending,
                              .onStateEntry = psatFSM_ascentEntryHandler,
                              .onStateExit = psatFSM_ascentExitHandler,
                              .stateHandler = psatFSM_ascentStateHandler},
    [psatFSM_state_deployPending] =
        {.state = psatFSM_state_deployPending,
         .defaultNextState = psatFSM_state_deployed,
         .onStateEntry = psatFSM_deployPendingEntryHandler,
         .onStateExit = psatFSM_deployPendingExitHandler,
         .stateHandler = psatFSM_deployPendingStateHandler},
    [psatFSM_state_deployed] = {.state = psatFSM_state_deployed,
                                .defaultNextState = psatFSM_state_descent,
                                .onStateEntry = psatFSM_deployedEntryHandler,
                                .onStateExit = psatFSM_deployedExitHandler,
                                .stateHandler = psatFSM_deployedStateHandler},
    [psatFSM_state_descent] = {.state = psatFSM_state_descent,
                               .defaultNextState = psatFSM_state_landing,
                               .onStateEntry = psatFSM_descentEntryHandler,
                               .onStateExit = psatFSM_descentExitHandler,
                               .stateHandler = psatFSM_descentStateHandler},
    [psatFSM_state_landing] = {.state = psatFSM_state_landing,
                               .defaultNextState = psatFSM_state_recovery,
                               .onStateEntry = psatFSM_landingEntryHandler,
                               .onStateExit = psatFSM_landingExitHandler,
                               .stateHandler = psatFSM_landingStateHandler},
    [psatFSM_state_recovery] = {.state = psatFSM_state_recovery,
                                .defaultNextState = psatFSM_state_lowPower,
                                .onStateEntry = psatFSM_recoveryEntryHandler,
                                .onStateExit = psatFSM_recoveryExitHandler,
                                .stateHandler = psatFSM_recoveryStateHandler},
    [psatFSM_state_lowPower] = {.state = psatFSM_state_lowPower,
                                .defaultNextState = psatFSM_state_error,
                                .onStateEntry = psatFSM_lowPowerEntryHandler,
                                .onStateExit = psatFSM_lowPowerExitHandler,
                                .stateHandler = psatFSM_lowPowerStateHandler},
    [psatFSM_state_error] = {.state = psatFSM_state_error,
                             .defaultNextState = psatFSM_state_error,
                             .onStateEntry = psatFSM_errorEntryHandler,
                             .onStateExit = psatFSM_errorExitHandler,
                             .stateHandler = psatFSM_errorStateHandler},
};

void psatFSM_postEvent(const psatFSM_event_t* event) {
    if (!eventQueue_s || !event) return;

    if (xQueueSend(eventQueue_s, event, 0) != pdPASS) {
        ESP_LOGW(
            TAG,
            "the event queue is full or something went wrong event dropped");
    }
}

void psatFSM_postEventISR(const psatFSM_event_t* event) {
    if (!eventQueue_s || !event) return;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t res =
        xQueueSendFromISR(eventQueue_s, event, &xHigherPriorityTaskWoken);
    if (res != pdPASS) {
    }  // TODO: implement error handling for this
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void psatFSM_setCurrentState(psatFSM_state_e newState) {
    if (stateMutex_s && xSemaphoreTake(stateMutex_s, portMAX_DELAY) == pdTRUE) {
        psat_globalState.currentFSMState = newState;
        xSemaphoreGive(stateMutex_s);
    }
}

psatFSM_state_e psatFSM_getCurrentState() {
    psatFSM_state_e state = psatFSM_state_error;

    if (stateMutex_s && xSemaphoreTake(stateMutex_s, portMAX_DELAY) == pdTRUE) {
        state = psat_globalState.currentFSMState;
        xSemaphoreGive(stateMutex_s);
    }

    return state;
}

void psatFSM_mainLoop(void* arg) {
    psatFSM_event_t currentEvent = {0};
    psatFSM_state_e nextState = psat_globalState.currentFSMState;

    while (1) {
        if (!xQueueReceive(eventQueue_s, &currentEvent, portMAX_DELAY))
            continue;  // if no event received do nothing

        if (currentEvent.global) {
            ESP_LOGI("FSM", "Received Global Event: %s",
                     psatFSM_eventTypeToString(currentEvent.type));
            // for events that are state agnostic, so will be handled or run
            // regardless of what state we are in
            globalEventHandler(&currentEvent);
        }

        ESP_LOGI("FSM", "Received Event: %s",
                 psatFSM_eventTypeToString(currentEvent.type));

        psatFSM_state_e currentState;

        xSemaphoreTake(stateMutex_s, portMAX_DELAY);
        currentState = psat_globalState.currentFSMState;
        xSemaphoreGive(stateMutex_s);

        // TODO: do a check for state count

        psatFSM_state_t currentStateDefinition = stateTable[currentState];
        if (currentStateDefinition.stateHandler) {
            nextState = currentStateDefinition.stateHandler(&currentEvent);
        } else {
            ESP_LOGE(TAG, "No handler for state %d", currentState);
            nextState = psatFSM_state_error;
        }

        if (nextState != currentState) {
            ESP_LOGI("FSM", "Transitioning from state: %s to new state: %s",
                     psatFSM_stateToString(psat_globalState.currentFSMState),
                     psatFSM_stateToString(nextState));

            // run exit function for current state
            currentStateDefinition.onStateExit();

            // change the state to the next state
            xSemaphoreTake(stateMutex_s, portMAX_DELAY);
            psat_globalState.currentFSMState = nextState;
            xSemaphoreGive(stateMutex_s);

            // call the entry function for the new state
            psatFSM_state_t nextStateDefinition = stateTable[nextState];
            nextStateDefinition.onStateEntry();
        }

        taskYIELD();
    }
}

void psatFSM_start() {
    eventQueue_s = xQueueCreate(QUEUE_SIZE, sizeof(psatFSM_event_t));
    stateMutex_s = xSemaphoreCreateMutex();

    if (!eventQueue_s || !stateMutex_s) {
        ESP_LOGE(TAG, "Failed to create queue or mutex");
        return;
    }

    psatFSM_event_t startupEvent = {.type = psatFSM_eventType_startPrelaunch,
                                    .global = false};
    psatFSM_postEvent(&startupEvent);

    psatFSM_mainLoop(NULL);
}

void psatFSM_startAsTask() {
    xTaskCreate(psatFSM_start, "fsm_task", 4096, NULL, 10, &xHandleSM_s);
}

void psatFSM_killTask() {
    if (xHandleSM_s != NULL) {
        vTaskDelete(xHandleSM_s);
        xHandleSM_s = NULL;
    }
    if (eventQueue_s != NULL) {
        vQueueDelete(eventQueue_s);
        eventQueue_s = NULL;
    }
    if (stateMutex_s != NULL) {
        vSemaphoreDelete(stateMutex_s);
        stateMutex_s = NULL;
    }
}