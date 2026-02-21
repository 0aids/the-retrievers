#include "bmi323.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "pin_config.h"
#include "shared_state.h"
#include "errors.h"
#include "I2C.h"
#include <string.h>
#include "driver/i2c_master.h"

//11011110 10101111 write to CMD for soft reset
//check status register and chip is if not responsive refer to data sheet quick start guide

uint8_t bmi323_accConfig[3] = {BMI323_ACCELEROMETER_CONFIG_ADDRESS, BMI323_ACCELEROMETER_CONFIG1, BMI323_ACCELEROMETER_CONFIG2};

uint8_t bmi323_gyroConfig[3] = {BMI323_GYRO_CONFIG_ADDRESS, BMI323_GYRO_CONFIG1, BMI323_GYRO_CONFIG2};

uint8_t bmi323_fifoConfig[3] = {BMI323_FIFO_CONFIG_ADDRESS, 0, 0};

uint8_t bmi323_resetCommand[3] = {0x7E, 0b11011110, 0b10101111};

bmi323_data_t bmi323_data = {0};

static i2c_device_config_t bmi323_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = BMI323_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

static i2c_master_dev_handle_t bmi323_handle;

void bmi323_init() {
    if(i2c_master_bus_add_device(i2c_bus1_handle, &bmi323_Config, &bmi323_handle) != ESP_OK) {
        psatErr_postError(psatErr_bmi323_i2cBusAddition_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_fifoConfig, sizeof(bmi323_fifoConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        psatErr_postError(psatErr_bmi323_FIFOConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_accConfig, sizeof(bmi323_accConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        psatErr_postError(psatErr_bmi323_AccConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(bmi323_handle, bmi323_gyroConfig, sizeof(bmi323_gyroConfig), I2c_WAIT_TIME_MS) != ESP_OK){
        psatErr_postError(psatErr_bmi323_GyroConfig_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    //add self calibration for gyro?

    //could add alternate configs for a sleep/low power mode to allow quicker switching

    //check err_reg 0x01 to check chip is operational
    uint8_t bmi323_readBuffer[2] = {1,1};
    uint8_t err_reg_address = BMI323_ERRREG_ADDRESS;
    i2c_master_transmit_receive(bmi323_handle, &err_reg_address, sizeof(err_reg_address), bmi323_readBuffer, sizeof(bmi323_readBuffer), I2c_WAIT_TIME_MS);

    if(bmi323_readBuffer[0]!=0 || bmi323_readBuffer[1] != 0) {
        psatErr_postError(psatErr_bmi323_ChipError, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

}

void reset() {
    if(i2c_master_transmit(bmi323_handle, bmi323_resetCommand, sizeof(bmi323_resetCommand), I2c_WAIT_TIME_MS) != ESP_OK) {
        psatErr_postError(psatErr_bmi323_reset_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }
}


void bmi323_deinit() {
    if(i2c_master_bus_rm_device(bmi323_handle) != ESP_OK) {
    psatErr_postError(psatErr_bmi323_i2cBusRemoval_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
    return;
    }
}


void bmi323_getData() {
    uint8_t dataAddress = 0x03;
    uint8_t readBuffer[12] = {0};
    int16_t rawData[6] = {0};

    if(i2c_master_transmit_receive(bmi323_handle, &dataAddress, sizeof(dataAddress), readBuffer, sizeof(readBuffer), I2c_WAIT_TIME_MS) != ESP_OK) {
        psatErr_postError(psatErr_bmi323_DataRead_failed, psatFSM_component_bmi323, psatFSM_getCurrentState());
        return;
    }

    bmi323_data.time = esp_timer_get_time();

    for(int i = 0; i < 6; i ++) {
        memcpy(readBuffer + (2 * i), rawData + i, 2);
    }

    //32767 (2^15 - 1)
    //~2048 is 1 g
    //gyro is set to 500 degree/s max
    //accelerometer is set to 16g max

    bmi323_data.accX = ((double)rawData[0] /2048) * 9.81;
    bmi323_data.accY = ((double)rawData[1] /2048) * 9.81;
    bmi323_data.accZ = ((double)rawData[2] /2048) * 9.81;

    bmi323_data.gyroX = ((double)rawData[3] / 32767) * 500;
    bmi323_data.gyroY = ((double)rawData[4] / 32767) * 500;
    bmi323_data.gyroZ = ((double)rawData[5] / 32767) * 500;    


}

