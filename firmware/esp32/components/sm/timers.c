#include "timers.h"
#include "esp_timer.h"
#include "sm.h"

static esp_timer_handle_t timer_1s;
static esp_timer_handle_t timer_5s;
static esp_timer_handle_t timer_10s;

static void timer_1s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_1S};
    fsm_post_event(&event);
}
void timers_start_1s(void) { esp_timer_start_periodic(timer_1s, 1000000); }
void timers_stop_1s(void) { esp_timer_stop(timer_1s); }

static void timer_5s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_5S};
    fsm_post_event(&event);
}
void timers_start_5s(void) { esp_timer_start_periodic(timer_5s, 5000000); }
void timers_stop_5s(void) { esp_timer_stop(timer_5s); }

static void timer_10s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_10S};
    fsm_post_event(&event);
}
void timers_start_10s(void) { esp_timer_start_periodic(timer_10s, 10000000); }
void timers_stop_10s(void) { esp_timer_stop(timer_10s); }

const esp_timer_create_args_t cb1 = {.callback = timer_1s_callback,
                                     .name = "t1s"};
const esp_timer_create_args_t cb5 = {.callback = timer_5s_callback,
                                     .name = "t5s"};
const esp_timer_create_args_t cb10 = {.callback = timer_10s_callback,
                                      .name = "t10s"};

void timers_init() {
    esp_timer_create(&cb1, &timer_1s);
    esp_timer_create(&cb5, &timer_5s);
    esp_timer_create(&cb10, &timer_10s);
}