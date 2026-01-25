#include "main.h"



void I2C_init(i2c_master_bus_handle_t* BusHandle, gpio_num_t SDA, gpio_num_t SCL) {

    i2c_master_bus_config_t MasterConfig = {
    .clk_source = I2C_CLK_SRC_DEFAULT, 
    .i2c_port = I2C_AUTO,
    .scl_io_num = SCL,
    .sda_io_num = SDA,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,

    };


    ESP_ERROR_CHECK(i2c_new_master_bus(&MasterConfig, BusHandle));
    printf("I2C bus created\n");

}

void I2C_dinit(i2c_master_bus_handle_t* BusHandle) {
    i2c_del_master_bus(*BusHandle);
}