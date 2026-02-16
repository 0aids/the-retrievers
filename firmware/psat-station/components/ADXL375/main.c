#include "main.h"

uint8_t ADXL375_PowerTransmit[2] = {ADXL375_POWER_TRANSMIT_ADDRESS,
                                    ADXL375_POWER_TRANSMIT_DATA};

uint8_t ADXL375_FIFOTransmit[2] = {ADXL375_FIFO_TRANSMIT_ADDRESS,
                                   ADXL375_FIFO_TRANSMIT_DATA};

uint8_t ADXL375_DataRegister = ADXL375_DATA_REGISTER;

i2c_device_config_t ADXL375_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = ADXL375_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

i2c_master_dev_handle_t ADXL375_Handle;

void ADXL375_init(i2c_master_bus_handle_t* BusHandle)
{

    ESP_ERROR_CHECK(i2c_master_bus_add_device(
        *BusHandle, &ADXL375_Config, &ADXL375_Handle));
    printf("ADXL375 added\n");

    ESP_ERROR_CHECK(i2c_master_transmit(
        ADXL375_Handle, ADXL375_PowerTransmit, 2, -1));

    ESP_ERROR_CHECK(i2c_master_transmit(ADXL375_Handle,
                                        ADXL375_FIFOTransmit, 2, -1));
}

void ADXL375_dinit()
{
    i2c_master_bus_rm_device(ADXL375_Handle);
}

void ADXL375_getData(uint16_t* Xacc, uint16_t* Yacc, uint16_t* Zacc)
{
    uint8_t i2cout[6] = {0};
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        ADXL375_Handle, &ADXL375_DataRegister, 1, i2cout, 6, -1));

    *Xacc = (i2cout[0] << 8) + i2cout[1];
    *Yacc = (i2cout[2] << 8) + i2cout[3];
    *Zacc = (i2cout[4] << 8) + i2cout[5];
}
