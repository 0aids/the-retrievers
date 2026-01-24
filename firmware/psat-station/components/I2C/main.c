#define AUTO -1
#define SDA 21
#define SCL 22
#include "driver/i2c_master.h"




i2c_master_bus_config_t MasterConfig = {
    .clk_source = I2C_CLK_SRC_DEFAULT, 
    .i2c_port = AUTO,
    .scl_io_num = SCL,
    .sda_io_num = SDA,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,

    };

i2c_master_bus_handle_t BusHandle;

void I2C_Setup () {

    ESP_ERROR_CHECK(i2c_new_master_bus(&MasterConfig, &BusHandle));
    printf("I2C bus created\n");

}