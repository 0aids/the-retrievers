#include "BMP280.h"
#include "pin_config.h"
#include "shared_state.h"

bool                   I2C_initalised        = false;
bool                   powerConfigured       = false;
bool                   measurementConfigured = false;
bool                   calibrated           = false;

static psatErrStates_e BMP280_err = noError;

BMP280_preflightStatus_t BMP280_preflightStatus = {0};

static void BMP280_compensatePressureAndTemperature(uint32_t ADC_T, uint32_t ADC_P, int32_t* temperature, double* pressure);

i2c_device_config_t    BMP280_Config = {
       .dev_addr_length = I2C_ADDR_BIT_7,
       .device_address  = BMP280_ADDRESS,
       .scl_speed_hz    = I2C_FREQUENCY,
};

i2c_master_dev_handle_t BMP280_handle;

uint8_t                 BMP280_ctrlmeas[2] = {BMP280_CTRLMEAS_ADDRESS,
                                              BMP280_CTRLMEAS_DATA};

uint8_t  BMP280_configReg[2] = {BMP280_CONFIGREG_ADDRESS,
                                BMP280_CONFIGREG_DATA};

uint8_t  BMP280_resetData[2] = {BMP280_RESET_ADDRESS,
                                BMP280_RESET_DATA};

uint8_t  BMP280_RawData[6];

uint8_t  BMP280_CalibrationData[24];

uint16_t dig_T1 = 0;
int16_t  dig_T2 = 0;
int16_t  dig_T3 = 0;
uint16_t dig_P1 = 0;
int16_t  dig_P2 = 0;
int16_t  dig_P3 = 0;
int16_t  dig_P4 = 0;
int16_t  dig_P5 = 0;
int16_t  dig_P6 = 0;
int16_t  dig_P7 = 0;
int16_t  dig_P8 = 0;
int16_t  dig_P9 = 0;

psatErrStates_e BMP280_checkErr() {
    psatErrStates_e temp = BMP280_err;
    BMP280_err = noError;
    return temp;
}

BMP280_status_t BMP280_queryStatus() {
    BMP280_status_t status;

    status.calibrated = calibrated;
    status.I2C_initalised = I2C_initalised;
    status.measurementConfigured = measurementConfigured;
    status.calibrated = calibrated;

    return status;
}

BMP280_preflightStatus_t BMP280_preflightTest() {
    i2c_master_bus_handle_t TestBusHandle;
    BMP280_init(&TestBusHandle);
    BMP280_getData(&BMP280_preflightStatus.temperature, &BMP280_preflightStatus.pressure);
    BMP280_deinit();
    return BMP280_preflightStatus;
}

void     BMP280_init(i2c_master_bus_handle_t* BusHandle)
{

    if (i2c_master_bus_add_device(*BusHandle, &BMP280_Config,
                                  &BMP280_handle) != ESP_OK)
    {
        BMP280_err = BMP280_i2cBusAddition_failed;
        return;
    }

    I2C_initalised = true;

    if (i2c_master_transmit(BMP280_handle, BMP280_ctrlmeas,
                            sizeof(BMP280_ctrlmeas),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        BMP280_err = BMP280_powerConfig_failed;
        return;
    }

    powerConfigured = true;

    if (i2c_master_transmit(BMP280_handle, BMP280_configReg,
                            sizeof(BMP280_configReg),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        BMP280_err = BMP280_measurementConfig_failed;
        return;
    }

    measurementConfigured = true;

    BMP280_getCalibration();
}

void BMP280_reset()
{

    if (i2c_master_transmit(BMP280_handle, BMP280_resetData,
                            sizeof(BMP280_resetData),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        BMP280_err = BMP280_reset_failed;
        return;
    }
}

void BMP280_deinit()
{
    
    BMP280_reset();

    if (i2c_master_bus_rm_device(BMP280_handle) != ESP_OK)
    {
        BMP280_err = BMP280_i2cBusRemoval_failed;
        return;
    }

}

void BMP280_getCalibration()
{

    uint8_t BMP280_CalibrationDataAddress =
        BMP280_CALIBRATION_DATA_ADDRESS;

    if (i2c_master_transmit_receive(
            BMP280_handle, &BMP280_CalibrationDataAddress,
            sizeof(BMP280_CalibrationDataAddress),
            BMP280_CalibrationData, sizeof(BMP280_CalibrationData),
            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        BMP280_err = BMP280_calibration_failed;
        return;
    }

    memcpy(&dig_T1, BMP280_CalibrationData, 2);
    memcpy(&dig_T2, BMP280_CalibrationData + 2, 2);
    memcpy(&dig_T3, BMP280_CalibrationData + 4, 2);
    memcpy(&dig_P1, BMP280_CalibrationData + 6, 2);
    memcpy(&dig_P2, BMP280_CalibrationData + 8, 2);
    memcpy(&dig_P3, BMP280_CalibrationData + 10, 2);
    memcpy(&dig_P4, BMP280_CalibrationData + 12, 2);
    memcpy(&dig_P5, BMP280_CalibrationData + 14, 2);
    memcpy(&dig_P6, BMP280_CalibrationData + 16, 2);
    memcpy(&dig_P7, BMP280_CalibrationData + 18, 2);
    memcpy(&dig_P8, BMP280_CalibrationData + 20, 2);
    memcpy(&dig_P9, BMP280_CalibrationData + 22, 2);
    calibrated = true;
}

static void BMP280_compensatePressureAndTemperature(uint32_t ADC_T,
                                             uint32_t ADC_P,
                                             int32_t* temperature,
                                             double*  pressure)
{
    //All magic numbers are magic from the datasheet and will never change so I will not make a #define for them
    // - zac
    double  var1;
    double  var2;
    double  var3;
    double  pressure_min = 30000.0;
    double  pressure_max = 110000.0;

    int32_t var1T;
    int32_t var2T;
    int32_t temperature_min = -4000;
    int32_t temperature_max = 8500;
    int32_t t_fine;

    int32_t ADC_Tint;

    memcpy(&ADC_Tint, &ADC_T, 3);

    var1T    = (int32_t)((ADC_Tint / 8) - ((int32_t)dig_T1 * 2));
    var1T    = (var1T * ((int32_t)dig_T2)) / 2048;
    var2T    = (int32_t)((ADC_Tint / 16) - ((int32_t)dig_T1));
    var2T    = (((var2T * var2T) / 4096) * ((int32_t)dig_T3)) / 16384;
    t_fine   = var1T + var2T;
    ADC_Tint = (t_fine * 5 + 128) / 256;

    if (ADC_Tint < temperature_min)
    {
        ADC_Tint = temperature_min;
    }
    else if (ADC_Tint > temperature_max)
    {
        ADC_Tint = temperature_max;
    }

    *temperature = ADC_Tint;

    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var3 = ((double)dig_P3) * var1 * var1 / 524288.0;
    var1 = (var3 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);

    // Avoid exception caused by division by zero
    if (var1 > (0.0))
    {
        ADC_P = 1048576.0 - (double)ADC_P;
        ADC_P = (ADC_P - (var2 / 4096.0)) * 6250.0 / var1;
        var1  = ((double)dig_P9) * ADC_P * ADC_P / 2147483648.0;
        var2  = ADC_P * ((double)dig_P8) / 32768.0;
        ADC_P = ADC_P + (var1 + var2 + ((double)dig_P7)) / 16.0;

        if (ADC_P < pressure_min)
        {
            ADC_P = pressure_min;
        }
        else if (ADC_P > pressure_max)
        {
            ADC_P = pressure_max;
        }
    }
    else // Invalid case
    {
        ADC_P = pressure_min;
    }

    *pressure = ADC_P;
}

void BMP280_getData(int32_t* Temperature, double* Pressure)
{

    uint8_t BMP280_ReadAddress = BMP280_READ_ADDRESS;

    if (i2c_master_transmit_receive(
            BMP280_handle, &BMP280_ReadAddress,
            sizeof(BMP280_ReadAddress), BMP280_RawData,
            sizeof(BMP280_RawData), I2c_WAIT_TIME_MS) != ESP_OK)
    {
        BMP280_err = BMP280_dataRead_failed;
        return;
    }

    uint32_t ADC_P = ((uint32_t)(BMP280_RawData[0]) << 12) |
        ((uint32_t)(BMP280_RawData[1]) << 4) |
        ((uint32_t)(BMP280_RawData[2]) >> 4);

    uint32_t ADC_T = ((uint32_t)(BMP280_RawData[3]) << 12) |
        ((uint32_t)(BMP280_RawData[4]) << 4) |
        ((uint32_t)(BMP280_RawData[5]) >> 4);

    BMP280_compensatePressureAndTemperature(ADC_T, ADC_P, Temperature,
                                            Pressure);
}
