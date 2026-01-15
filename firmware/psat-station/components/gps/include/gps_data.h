#pragma once

#include "minmea.h"
#include "shared_state.h"

void gps_stateInit(void);
void gps_logGpsSnapshot(gps_data_t* gps);
void gps_stateGetSnapshot(gps_data_t* out);
void gps_processLine(const char* gpsBuffer_c);
void gps_stateCompleteOverwrite(const gps_data_t* src);