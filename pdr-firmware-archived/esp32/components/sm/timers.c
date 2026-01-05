#include "timers.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sm.h"

static esp_timer_handle_t timer_1s = NULL;
static esp_timer_handle_t timer_5s = NULL;
static esp_timer_handle_t timer_10s = NULL;
static esp_timer_handle_t mechanical_timer = NULL;

static void timer_1s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_1S};
    fsm_post_event_from_isr(&event);
}
static void timer_5s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_5S};
    fsm_post_event_from_isr(&event);
}
static void timer_10s_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_TIMER_10S};
    fsm_post_event_from_isr(&event);
}
static void timer_mechanical_callback(void* arg) {
    fsm_event_t event = {.type = EVENT_FOLDING_TIMER_REACHED};
    fsm_post_event_from_isr(&event);
}

void timers_start_1s(void) { esp_timer_start_periodic(timer_1s, 1000000); }
void timers_stop_1s(void) { esp_timer_stop(timer_1s); }

void timers_start_5s(void) { esp_timer_start_periodic(timer_5s, 5000000); }
void timers_stop_5s(void) { esp_timer_stop(timer_5s); }

void timers_start_10s(void) { esp_timer_start_periodic(timer_10s, 10000000); }
void timers_stop_10s(void) { esp_timer_stop(timer_10s); }

void timers_start_mechanical(uint32_t delay_seconds) {
    esp_timer_start_once(mechanical_timer, delay_seconds * 1000000ULL);
}

static const esp_timer_create_args_t cb1 = {.callback = timer_1s_callback,
                                            .name = "t1s"};
static const esp_timer_create_args_t cb5 = {.callback = timer_5s_callback,
                                            .name = "t5s"};
static const esp_timer_create_args_t cb10 = {.callback = timer_10s_callback,
                                             .name = "t10s"};
static const esp_timer_create_args_t cbm = {
    .callback = timer_mechanical_callback, .name = "deploy_mechanical_timer"};

void timers_init() {
    esp_timer_create(&cb1, &timer_1s);
    esp_timer_create(&cb5, &timer_5s);
    esp_timer_create(&cb10, &timer_10s);
    esp_timer_create(&cbm, &mechanical_timer);
}