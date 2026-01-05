#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include "gps_state.h"

void gps_init(void);
void gps_start(void);
void gps_kill(void);
void log_gps_data(gps_state_t* gps);

#endif