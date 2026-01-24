#include <stdint.h>
#include "driver/i2c_master.h"

#include "driver/i2c_slave.h"
extern i2c_master_bus_handle_t BusHandle;

uint8_t ADXL345_PowerTransmit[2] = {0x2D, 0b00001000};

uint8_t ADXL345_FIFOTransmit[2] = {0x38, 0b00000000};

uint8_t ADXL345_DataRegister = 0x32;


i2c_device_config_t ADXL345_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address = 0x1D,
    .scl_speed_hz = 100000,
};

i2c_master_dev_handle_t ADXL345_Handle;

void ADXL345_Setup() {


 ESP_ERROR_CHECK(i2c_master_bus_add_device(BusHandle, &ADXL345_Config, &ADXL345_Handle));
    printf("ADXL345 added\n");

    printf("transmit\n");
    ESP_ERROR_CHECK(i2c_master_transmit(ADXL345_Handle, ADXL345_PowerTransmit, 2, -1));
    printf("Power Transmit\n");

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_transmit(ADXL345_Handle, ADXL345_FIFOTransmit, 2, 100));

    printf("FIFO Transmit\n");

}





void ADXL345_GetData(uint16_t* Xacc, uint16_t* Yacc, uint16_t* Zacc){
    uint8_t i2cout[6] = {0};
    printf("Data Call\n");
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_transmit_receive(ADXL345_Handle, &ADXL345_DataRegister, 1, i2cout, 6, 100));
    printf("Data Rcieved\n");
    
    *Xacc = (i2cout[0] << 8) + i2cout[1];
    *Yacc = (i2cout[2] << 8) + i2cout[3];
    *Zacc = (i2cout[4] << 8) + i2cout[5];

}

