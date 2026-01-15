#include "timers.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sm.h"

#define TIMER_MS_TO_US(ms) ((uint64_t)(ms) * 1000ULL)
// TODO: might use this if none of the timers are under 1s
#define TIMER_S_TO_US(s) ((uint64_t)(s) * 1000000ULL)

static const timer_config_t timer_config_c[timer_timerId__COUNT] = {
    [timer_timerId_1s] = {.period_ms = 1000,
                          .event = psatFSM_eventType_timer1s,
                          .name = "t1s"},
    [timer_timerId_5s] = {.period_ms = 5000,
                          .event = psatFSM_eventType_timer5s,
                          .name = "t5s"},
    [timer_timerId_10s] = {.period_ms = 10000,
                           .event = psatFSM_eventType_timer10s,
                           .name = "t10s"},
    [timer_timerId_mechanical] = {.period_ms =
                                      0,  // this one is a one time use timer
                                  .event = psatFSM_eventType_unfoldMechanism,
                                  .name = "mechanical"},
};
static esp_timer_handle_t timers_s[timer_timerId__COUNT];

static void timer_callback(void* arg) {
    timer_id_e id = (timer_id_e)arg;
    psatFSM_event_t event = {.type = timer_config_c[id].event, .global = false};
    psatFSM_postEventISR(&event);
}

void timer_init(void) {
    for (int i = 0; i < timer_timerId__COUNT; i++) {
        esp_timer_create_args_t args = {.callback = timer_callback,
                                        .arg = (void*)i,
                                        .name = timer_config_c[i].name};

        esp_timer_create(&args, &timers_s[i]);
    }
}

// TODO: bounds checking on timers like < TIMER_COUNT and delay != 0 or smth

void timer_start(timer_id_e id) {
    esp_timer_start_periodic(timers_s[id],
                             TIMER_MS_TO_US(timer_config_c[id].period_ms));
}

void timer_startOnce(timer_id_e id, uint32_t delay_ms) {
    esp_timer_start_once(timers_s[id], TIMER_MS_TO_US(delay_ms));
}

void timer_stop(timer_id_e id) { esp_timer_stop(timers_s[id]); }