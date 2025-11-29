#include "sm.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "state_handlers.h"

static const char* TAG = "FSM";

static QueueHandle_t fsm_event_queue = NULL;
static fsm_state_t current_state = STATE_PRELAUNCH;

void fsm_post_event(const fsm_event_t* event) {
    if (!fsm_event_queue || !event) return;
    xQueueSend(fsm_event_queue, event, 0);
}

void fsm_post_event_from_isr(const fsm_event_t* event) {
    if (!fsm_event_queue || !event) return;
    xQueueSendFromISR(fsm_event_queue, event, NULL);
}

void fsm_task(void* arg) {
    fsm_event_t event;

    fsm_event_t startup;
    startup.type = EVENT_START_PRELAUNCH;

    fsm_post_event(&startup);

    while (1) {
        if (!xQueueReceive(fsm_event_queue, &event, portMAX_DELAY)) continue;

        fsm_state_t new_state = current_state;

        switch (current_state) {
            case STATE_PRELAUNCH:
                new_state = state_prelaunch(&event);
                break;
            case STATE_ASCENT:
                new_state = state_ascent(&event);
                break;
            case STATE_DEPLOYED:
                new_state = state_deployed(&event);
                break;
            case STATE_DECENT:
                new_state = state_descent(&event);
                break;
            case STATE_LANDING:
                new_state = state_landing(&event);
                break;
            case STATE_RECOVERY:
                new_state = state_recovery(&event);
                break;
            case STATE_ERROR:
                new_state = state_error(&event);
                break;
        }

        if (new_state != current_state) {
            ESP_LOGI(TAG, "moving to a new state from %d to %d", current_state,
                     new_state);
            current_state = new_state;
        }
    }
}

void fsm_start() {
    fsm_event_queue = xQueueCreate(32, sizeof(fsm_event_t));
    xTaskCreate(fsm_task, "fsm_task", 4096, NULL, 10, NULL);
}