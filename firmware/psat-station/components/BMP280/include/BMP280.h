#include <stdint.h>
#include "driver/i2c_master.h"
#include "shared_state.h"
#include <pin_config.h>


void BMP280_init(i2c_master_bus_handle_t* BusHandle);
void BMP280_deinit();
void BMP280_reset();
void BMP280_getCalibration();
void BMP280_getData(int32_t* Temperature, double* Pressure);
psatErrStates_e BMP280_checkErr();

BMP280_preflightStatus_t BMP280_preflightTest();
BMP280_status_t BMP280_queryStatus();