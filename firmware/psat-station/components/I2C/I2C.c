#include "I2C.h"
#include "driver/i2c_types.h"
#include "pin_config.h"
#include "shared_state.h"

i2c_master_bus_handle_t i2c_bus1_handle;
i2c_master_bus_handle_t i2c_bus2_handle;


void I2C_init_bus1()
{

    i2c_master_bus_config_t MasterConfig = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .i2c_port                     = I2C_AUTO,
        .scl_io_num                   = I2C_SCL_1,
        .sda_io_num                   = I2C_SDA_1,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,

    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&MasterConfig, &i2c_bus1_handle));
}

void I2C_dinit_bus1()
{
    i2c_del_master_bus(i2c_bus1_handle);
}



void I2C_init_bus2()
{

    i2c_master_bus_config_t MasterConfig = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .i2c_port                     = I2C_AUTO,
        .scl_io_num                   = I2C_SCL_2,
        .sda_io_num                   = I2C_SDA_2,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,

    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&MasterConfig, &i2c_bus2_handle));
}

void I2C_dinit_bus2()
{
    i2c_del_master_bus(i2c_bus2_handle);
}

