#pragma once

#include "gps_data.h"
#include "shared_state.h"

void gps_init(void);
void gps_deinit(void);
void gps_startTask(void);
void gps_killTask(void);