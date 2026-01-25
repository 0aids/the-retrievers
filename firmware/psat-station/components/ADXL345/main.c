#include "main.h"


uint8_t ADXL345_PowerTransmit[2] = {ADXL345_POWER_TRANSMIT_ADDRESS, ADXL345_POWER_TRANSMIT_DATA};

uint8_t ADXL345_FIFOTransmit[2] = {ADXL345_FIFO_TRANSMIT_ADDRESS, ADXL345_FIFO_TRANSMIT_DATA};

uint8_t ADXL345_DataRegister = ADXL345_DATA_REGISTER;



i2c_device_config_t ADXL345_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address = ADXL345_ADDRESS,
    .scl_speed_hz = I2C_FREQUENCY,
};

i2c_master_dev_handle_t ADXL345_Handle;

void ADXL345_init(i2c_master_bus_handle_t* BusHandle) {


 ESP_ERROR_CHECK(i2c_master_bus_add_device(*BusHandle, &ADXL345_Config, &ADXL345_Handle));
    printf("ADXL345 added\n");

    printf("transmit\n");
    ESP_ERROR_CHECK(i2c_master_transmit(ADXL345_Handle, ADXL345_PowerTransmit, 2, -1));
    printf("Power Transmit\n");

    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_transmit(ADXL345_Handle, ADXL345_FIFOTransmit, 2, 100));

    printf("FIFO Transmit\n");

}

void ADXL375_dinit() {
    i2c_master_bus_rm_device(ADXL345_Handle);
}



void ADXL345_getData(uint16_t* Xacc, uint16_t* Yacc, uint16_t* Zacc){
    uint8_t i2cout[6] = {0};
    printf("Data Call\n");
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_transmit_receive(ADXL345_Handle, &ADXL345_DataRegister, 1, i2cout, 6, 100));
    printf("Data Rcieved\n");
    
    *Xacc = (i2cout[0] << 8) + i2cout[1];
    *Yacc = (i2cout[2] << 8) + i2cout[3];
    *Zacc = (i2cout[4] << 8) + i2cout[5];

}

