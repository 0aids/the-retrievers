#include "highg.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "pin_config.h"
#include "shared_state.h"
#include "errors.h"
#include "I2C.h"
#include <string.h>
#include "driver/i2c_master.h"
#include "sm.h"
#include <stdint.h>



#define HIGHG_ADDRESS  0b0011000
#define HIGHG_POWER_ADDRESS 0x20
#define HIGHG_POWER_CONFIG 0b00101111
#define HIGHG_DATA_ADDRESS 0x29

static uint8_t highg_powerConfig[2] = {HIGHG_POWER_ADDRESS, HIGHG_POWER_CONFIG};

highGAcc_data_t highg_data = {0};

static i2c_device_config_t highg_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = BMI323_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

static i2c_master_dev_handle_t highg_handle;


void highg_init() {
    
    if(i2c_master_bus_add_device(i2c_bus2_handle, &highg_Config, &highg_handle) != ESP_OK) {
        psatErr_postError(psatErr_highg_i2cBusAddition_failed, psatFSM_component_highg, psatFSM_getCurrentState());
        return;
    }

    if(i2c_master_transmit(highg_handle, highg_powerConfig, sizeof(highg_powerConfig), I2c_WAIT_TIME_MS) != ESP_OK) {
        psatErr_postError(psatErr_highg_i2cBusAddition_failed, psatFSM_component_highg, psatFSM_getCurrentState());
        return;
    }

}

void highg_deinit() {

    if(i2c_master_bus_rm_device(highg_handle) != ESP_OK) {
        psatErr_postError(psatErr_highg_i2cBusAddition_failed, psatFSM_component_highg, psatFSM_getCurrentState());
        return;
    }

}

void highg_getData() {

    uint8_t readBuffer[6] = {0};
    uint8_t readAddress = HIGHG_DATA_ADDRESS;
    int16_t rawData[3] = {0};


    if(i2c_master_transmit_receive(highg_handle,&readAddress, 1, readBuffer, sizeof(readBuffer), I2c_WAIT_TIME_MS) != ESP_OK) {
        psatErr_postError(psatErr_highg_i2cBusAddition_failed, psatFSM_component_highg, psatFSM_getCurrentState());
        return;
    }

    highg_data.time = esp_timer_get_time();

    for(int i = 0; i < 3; i ++) {
        memcpy(readBuffer + (2 * i), rawData + i, 2);
    }

    //32767 = 100g 

    highg_data.accX = ((double)rawData[0] /327.67) * 9.81;
    highg_data.accY = ((double)rawData[1] /327.67) * 9.81;
    highg_data.accZ = ((double)rawData[2] /327.67) * 9.81;

}