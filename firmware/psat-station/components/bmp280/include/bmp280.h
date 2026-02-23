#pragma once
#include <stdint.h>
#include "driver/i2c_master.h"
#include "shared_state.h"
#include <pin_config.h>
#include <string.h>
#include "shared_state.h"

extern bmp280_data_t bmp280_data;


void           bmp280_init();
void           bmp280_deinit();
void           bmp280_reset();
void           bmp280_getCalibration();
bool           bmp280_isHealthy();


double         bmp280_getData();    
//int32_t        bmp280_getTemperature(); // in degrees C

bool bmp280_preflightTest();
bmp280_status_t          bmp280_queryStatus();