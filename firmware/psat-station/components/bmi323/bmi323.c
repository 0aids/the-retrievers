#include "bmi323.h"
#include "pin_config.h"
#include "shared_state.h"
#include "errors.h"
#include "I2C.h"
#include <string.h>
#include "driver/i2c_master.h"
//#include <stdint.h>



uint8_t bmi323_accConfig[3] = {BMI323_ACCELEROMETER_CONFIG_ADDRESS, BMI323_ACCELEROMETER_CONFIG1, BMI323_ACCELEROMETER_CONFIG2};

uint8_t bmi323_gyroConfig[3] = {BMI323_GYRO_CONFIG_ADDRESS, BMI323_GYRO_CONFIG1, BMI323_GYRO_CONFIG2};

uint8_t bmi323_fifoConfig[3] = {BMI323_FIFO_CONFIG_ADDRESS, 0, 0};

//reg 0x0C saturation flags for checking when acceleration is too high


static i2c_device_config_t bmi323_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = BMI323_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

static i2c_master_dev_handle_t bmi323_handle;

void bmi323_init() {
    if(i2c_master_bus_add_device(i2c_bus1_handle, &bmi323_Config, &bmi323_handle) != ESP_OK) {
        //psatErr_postError(psatErr_bmi323_i2cBusAddition_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_fifoConfig, sizeof(bmi323_fifoConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        //psatErr_postError(psatErr_bmi323_FIFOConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_accConfig, sizeof(bmi323_accConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        //psatErr_postError(psatErr_bmi323_AccConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_gyroConfig, sizeof(bmi323_gyroConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        //psatErr_postError(psatErr_bmi323_GyroConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    //could add alternate configs for a sleep/low power mode to allow quicker switching

    //check err_reg 0x01 to check chip is operational
    uint8_t bmi323_readBuffer[2] = {1,1};
    uint8_t err_reg_address = BMI323_ERRREG_ADDRESS;
    i2c_master_transmit_receive(bmi323_handle, &err_reg_address, sizeof(err_reg_address), bmi323_readBuffer, sizeof(bmi323_readBuffer), I2c_WAIT_TIME_MS);

    if(bmi323_readBuffer[0]!=0 || bmi323_readBuffer[1] != 0) {
        //psatErr_postError(psatErr_bmi323_ChipError, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

}


void bmi323_deinit() {
    if(i2c_master_bus_rm_device(bmi323_handle) != ESP_OK) {
    //psatErr_postError(psatErr_bmi323_i2cBusRemoval_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
    return;
    }
}

