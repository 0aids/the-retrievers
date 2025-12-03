#ifndef SM_TIMERS_H
#define SM_TIMERS_H

#include <stdint.h>

void timers_start_1s(void);
void timers_stop_1s(void);
void timers_start_5s(void);
void timers_stop_5s(void);
void timers_start_10s(void);
void timers_stop_10s(void);
void timers_start_mechanical(uint32_t delay_seconds);

void timers_init(void);

#endif