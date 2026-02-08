#pragma once

#include "driver/uart.h"

#define CFG_GPS_UART_NUM_d         UART_NUM_1
#define CFG_GPS_TX_PIN_d           48 // 37 // CANNOT USE! Part of heap.
#define CFG_GPS_RX_PIN_d           42
#define CFG_LANDING_BUTTON_PIN_d   39
#define CFG_PRELAUNCH_BUTTON_PIN_d 21
#define CFG_LIGHT1_PIN_d           41
#define CFG_BUZZER_PIN_d           40
#define CFG_SERVO_PIN_d            8

#define LDR_ADC_UNIT_d                 ADC_UNIT_1
#define LDR_ADC_CHANNEL_d              ADC_CHANNEL_1
#define LDR_ULP_MODE_d                 ADC_ULP_MODE_DISABLE
#define LDR_ADC_ATTEN_d                ADC_ATTEN_DB_12
#define LDR_ADC_BITWIDTH_d             ADC_BITWIDTH_DEFAULT
#define LDR_PIN_MASK_d                 (1ULL << 2)
#define LDR_STATE_CONFIG_BUFFER_SIZE_d (1024)

#define I2C_AUTO         -1
#define I2C_SDA_1        21
#define I2C_SCL_1        22
#define I2C_SDA_2        6
#define I2C_SCL_2        7
#define I2C_FREQUENCY    100000
#define I2c_WAIT_TIME_MS 2000

#define BMP280_CALIBRATION_DATA_ADDRESS 0x88
#define BMP280_READ_ADDRESS             0xF7
#define BMP280_CTRLMEAS_ADDRESS         0xF4
#define BMP280_CTRLMEAS_DATA            0b10110111
#define BMP280_CONFIGREG_ADDRESS        0xF5
#define BMP280_CONFIGREG_DATA           0b00000100
#define BMP280_ADDRESS                  0x76
#define BMP280_RESET_ADDRESS            0xE0
#define BMP280_RESET_DATA               0xB6

#define ADXL375_POWER_TRANSMIT_ADDRESS 0x2D
#define ADXL375_POWER_TRANSMIT_DATA    0b00001000
#define ADXL375_FIFO_TRANSMIT_ADDRESS  0x38
#define ADXL375_FIFO_TRANSMIT_DATA     0b00000000
#define ADXL375_DATA_REGISTER          0x32
#define ADXL375_ADDRESS                0x53

#define ADXL345_POWER_TRANSMIT_ADDRESS 0x2D
#define ADXL345_POWER_TRANSMIT_DATA    0b00001000
#define ADXL345_FIFO_TRANSMIT_ADDRESS  0x38
#define ADXL345_FIFO_TRANSMIT_DATA     0b00000000
#define ADXL345_DATA_REGISTER          0x32
#define ADXL345_ADDRESS                0x1D
