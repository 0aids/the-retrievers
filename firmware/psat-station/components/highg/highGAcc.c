#include "highGAcc.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "pin_config.h"
#include "shared_state.h"
#include "errors.h"
#include "I2C.h"
#include <string.h>
#include "driver/i2c_master.h"
#include <stdint.h>



#define HIGHG_ACC_ADDRESS  0b0011000
#define HIGHG_ACC_POWER_ADDRESS 0x20
#define HIGHG_ACC_POWER_CONFIG 0b00101111
#define HIGHG_ACC_DATA_ADDRESS 0x29

static uint8_t highGAcc_powerConfig[2] = {HIGHG_ACC_POWER_ADDRESS, HIGHG_ACC_POWER_CONFIG};

highGAcc_data_t highg_data = {0};

static i2c_device_config_t highGAcc_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = BMI323_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

static i2c_master_dev_handle_t highGAcc_handle;


void highg_init() {
    
    if(i2c_master_bus_add_device(i2c_bus2_handle, &highGAcc_Config, &highGAcc_handle) != ESP_OK) {

        return;
    }

    if(i2c_master_transmit(highGAcc_handle, highGAcc_powerConfig, sizeof(highGAcc_powerConfig), I2c_WAIT_TIME_MS) != ESP_OK) {

        return;
    }

}

void highg_deinit() {

    //reset/powercycle if possible

    if(i2c_master_bus_rm_device(highGAcc_handle) != ESP_OK) {

        return;
    }

}

void highg_reset() {

    //datasheet and implement
}

void highg_getData() {

    uint8_t readBuffer[6] = {0};
    uint8_t readAddress = HIGHG_ACC_DATA_ADDRESS;
    int16_t rawData[3] = {0};


    if(i2c_master_transmit_receive(highGAcc_handle,&readAddress, 1, readBuffer, sizeof(readBuffer), I2c_WAIT_TIME_MS) != ESP_OK) {

        return;
    }

    highg_data.time = esp_timer_get_time();

    for(int i = 0; i < 3; i ++) {
        memcpy(readBuffer + (2 * i), rawData + i, 2);
    }


}