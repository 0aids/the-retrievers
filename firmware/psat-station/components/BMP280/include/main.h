#include <stdint.h>
#include "driver/i2c_master.h"
#include <pin_config.h>


void BMP280_init();
void BMP280_dinit();
void BMP280_reset();
void BMP280_getCalibration();
void BMP280_compensatePressureAndTemperature();
void BMP280_getData();