#pragma once
#include "shared_state.h"

extern bmi323_data_t bmi323_data;

void bmi323_init();
void bmi323_deinit();
void bmi323_getData();
void bmi323_reset();