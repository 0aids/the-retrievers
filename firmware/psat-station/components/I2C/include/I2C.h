#include "driver/i2c_master.h"
#include <pin_config.h>

void I2C_init(i2c_master_bus_handle_t* BusHandle, gpio_num_t SDA,
              gpio_num_t SCL);

void I2C_dinit(i2c_master_bus_handle_t* BusHandle);