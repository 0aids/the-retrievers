#include "sm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "state_handlers.h"

#define QUEUE_SIZE 32
static const char* TAG = "PSAT_FSM";

static TaskHandle_t xHandleSM_s = NULL;
static QueueHandle_t eventQueue_s = NULL;
static volatile psatFSM_state_e currentState_s = psatFSM_state_prelaunch;

void psatFSM_postEvent(const psatFSM_event_t* event) {
    if (!eventQueue_s || !event) return;
    xQueueSend(eventQueue_s, event, 0);
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
    // TODO: Implement a mutex lock for this
    currentState_s = newState;
}

psatFSM_state_e psatFSM_getCurrentState() {
    // TODO: Implement a mutex lock for this
    return currentState_s;
}

void psatFSM_mainLoop(void* arg) {
    psatFSM_event_t currentEvent = {0};
    psatFSM_state_e nextState = currentState_s;

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

        switch (currentState_s) {
            case psatFSM_state_prelaunch:
                nextState = psatFSM_prelaunchStateHandler(&currentEvent);
                break;
            case psatFSM_state_ascent:
                nextState = psatFSM_ascentStateHandler(&currentEvent);
                break;
            case psatFSM_state_deployed:
                nextState = psatFSM_deployedStateHandler(&currentEvent);
                break;
            case psatFSM_state_descent:
                nextState = psatFSM_descentStateHandler(&currentEvent);
                break;
            case psatFSM_state_landing:
                nextState = psatFSM_landingStateHandler(&currentEvent);
                break;
            case psatFSM_state_recovery:
                nextState = psatFSM_recoveryStateHandler(&currentEvent);
                break;
            case psatFSM_state_lowpwr:
                nextState = psatFSM_lowpwrStateHandler(&currentEvent);
                break;
            case psatFSM_state_error:
                nextState = psatFSM_errorStateHandler(&currentEvent);
                break;
        }

        if (nextState != currentState_s) {
            ESP_LOGI("FSM", "Transitioning from state: %s to new state: %s",
                     psatFSM_stateToString(currentState_s),
                     psatFSM_stateToString(nextState));
            currentState_s = nextState;
        }
    }
}

void psatFSM_start() {
    eventQueue_s = xQueueCreate(QUEUE_SIZE, sizeof(psatFSM_event_t));

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
}