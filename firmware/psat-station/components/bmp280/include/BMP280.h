#include <stdint.h>
#include "driver/i2c_master.h"
#include "shared_state.h"
#include <pin_config.h>
#include <string.h>

void           bmp280_init(i2c_master_bus_handle_t* BusHandle);
void           bmp280_deinit();
void           bmp280_reset();
void           bmp280_getCalibration();

double         bmp280_getPressure();    // in Pa
int32_t        bmp280_getTemperature(); // in degrees C

psatErr_code_e bmp280_checkErr();
bmp280_preflightStatus_t bmp280_preflightTest();
bmp280_status_t          bmp280_queryStatus();