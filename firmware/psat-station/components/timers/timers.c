#include "timers.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "sm.h"

#define TIMER_MS_TO_US(ms) ((uint64_t)(ms) * 1000ULL)

static timer_id_e timer_ids[timer_timerId__COUNT];
static const char* TAG = "TIMER";

static const timer_config_t timer_config_c[timer_timerId__COUNT] = {
    [timer_timerId_1s] = {.period_ms = 1000,
                          .event = psatFSM_eventType_timer1s,
                          .name = "t1s",
                          .global = false},
    [timer_timerId_5s] = {.period_ms = 5000,
                          .event = psatFSM_eventType_timer5s,
                          .name = "t5s",
                          .global = false},
    [timer_timerId_10s] = {.period_ms = 10000,
                           .event = psatFSM_eventType_timer10s,
                           .name = "t10s",
                           .global = false},
    [timer_timerId_mechanical] = {.period_ms =
                                      0,  // this one is a one time use timer
                                  .event = psatFSM_eventType_unfoldMechanism,
                                  .name = "mechanical",
                                  .global = true},
};
static esp_timer_handle_t timers_s[timer_timerId__COUNT];

static void timer_callback(void* arg) {
    timer_id_e id = *(timer_id_e*)arg;
    psatFSM_event_t event = {.type = timer_config_c[id].event,
                             .global = timer_config_c[id].global};
    psatFSM_postEvent(&event);
}

void timer_init(void) {
    for (int i = 0; i < timer_timerId__COUNT; i++) {
        timer_ids[i] = (timer_id_e)i;
        esp_timer_create_args_t args = {.callback = timer_callback,
                                        .arg = &timer_ids[i],
                                        .name = timer_config_c[i].name};
        ESP_ERROR_CHECK(esp_timer_create(&args, &timers_s[i]));
    }
}

void timer_start(timer_id_e id) {
    if (id < 0 || id >= timer_timerId__COUNT) {
        ESP_LOGW(TAG, "Invalid timer id provided");
        return;
    }

    if (timer_config_c[id].period_ms == 0) {
        ESP_LOGE(TAG,
                 "Timer %d is only supposed to be used with timer_startOnce()",
                 id);
        return;
    }

    esp_timer_stop(timers_s[id]);

    esp_err_t err = esp_timer_start_periodic(
        timers_s[id], TIMER_MS_TO_US(timer_config_c[id].period_ms));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start timer %d (error: %s)", id,
                 esp_err_to_name(err));
    }
}

void timer_startOnce(timer_id_e id, uint32_t delay_ms) {
    if (id < 0 || id >= timer_timerId__COUNT) {
        ESP_LOGW(TAG, "Invalid timer id provided");
        return;
    }

    esp_timer_stop(timers_s[id]);

    esp_err_t err =
        esp_timer_start_once(timers_s[id], TIMER_MS_TO_US(delay_ms));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start once timer %d (%s)", id,
                 esp_err_to_name(err));
    }
}

void timer_stop(timer_id_e id) {
    if (id < 0 || id >= timer_timerId__COUNT) {
        ESP_LOGW(TAG, "Invalid timer id provided");
        return;
    }

    esp_err_t err = esp_timer_stop(timers_s[id]);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to stop timer %d (the error: %s)", id,
                 esp_err_to_name(err));
    }
}