#include "driver/i2c_master.h"
#include "shared_state.h"
#include <pin_config.h>
extern i2c_master_bus_handle_t i2c_bus1_handle;
extern i2c_master_bus_handle_t i2c_bus2_handle;

void I2C_init_bus1();

void I2C_dinit_bus1();

void I2C_init_bus2();

void I2C_dinit_bus2();