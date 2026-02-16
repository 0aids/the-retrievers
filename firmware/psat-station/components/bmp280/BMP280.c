#include "bmp280.h"
#include "pin_config.h"
#include "shared_state.h"

static bmp280_status_t   bmp280_status = {.I2C_initalised  = false,
                                          .powerConfigured = false,
                                          .measurementConfigured =
                                              false,
                                          .calibrated = false};
static psatErr_code_e    bmp280_err    = psatErr_none;

bmp280_preflightStatus_t bmp280_preflightStatus = {0};

static void              bmp280_compensatePressureAndTemperature(
                 uint32_t ADC_T, uint32_t ADC_P, int32_t* temperature,
                 double* pressure);

i2c_device_config_t bmp280_Config = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address  = BMP280_ADDRESS,
    .scl_speed_hz    = I2C_FREQUENCY,
};

i2c_master_dev_handle_t bmp280_handle;

uint8_t                 bmp280_ctrlmeas[2] = {BMP280_CTRLMEAS_ADDRESS,
                                              BMP280_CTRLMEAS_DATA};

uint8_t        bmp280_configReg[2] = {BMP280_CONFIGREG_ADDRESS,
                                      BMP280_CONFIGREG_DATA};

uint8_t        bmp280_resetData[2] = {BMP280_RESET_ADDRESS,
                                      BMP280_RESET_DATA};

uint8_t        bmp280_RawData[6];

uint8_t        bmp280_CalibrationData[24];

uint16_t       dig_T1 = 0;
int16_t        dig_T2 = 0;
int16_t        dig_T3 = 0;
uint16_t       dig_P1 = 0;
int16_t        dig_P2 = 0;
int16_t        dig_P3 = 0;
int16_t        dig_P4 = 0;
int16_t        dig_P5 = 0;
int16_t        dig_P6 = 0;
int16_t        dig_P7 = 0;
int16_t        dig_P8 = 0;
int16_t        dig_P9 = 0;

psatErr_code_e bmp280_checkErr()
{
    psatErr_code_e temp = bmp280_err;
    bmp280_err          = psatErr_none;
    return temp;
}

bmp280_status_t bmp280_queryStatus()
{
    return bmp280_status;
}

bmp280_preflightStatus_t bmp280_preflightTest()
{
    i2c_master_bus_handle_t TestBusHandle;
    bmp280_init(&TestBusHandle);
    bmp280_preflightStatus.temperature = bmp280_getTemperature();
    bmp280_preflightStatus.pressure    = bmp280_getPressure();
    bmp280_deinit();
    return bmp280_preflightStatus;
}

void bmp280_init(i2c_master_bus_handle_t* BusHandle)
{

    if (i2c_master_bus_add_device(*BusHandle, &bmp280_Config,
                                  &bmp280_handle) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_i2cBusAddition_failed;
        return;
    }

    bmp280_status.I2C_initalised = true;

    if (i2c_master_transmit(bmp280_handle, bmp280_ctrlmeas,
                            sizeof(bmp280_ctrlmeas),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_powerConfig_failed;
        return;
    }

    bmp280_status.powerConfigured = true;

    if (i2c_master_transmit(bmp280_handle, bmp280_configReg,
                            sizeof(bmp280_configReg),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_measurementConfig_failed;
        return;
    }

    bmp280_status.measurementConfigured = true;

    bmp280_getCalibration();
}

void bmp280_reset()
{

    if (i2c_master_transmit(bmp280_handle, bmp280_resetData,
                            sizeof(bmp280_resetData),
                            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_reset_failed;
        return;
    }
}

void bmp280_deinit()
{

    bmp280_reset();

    if (i2c_master_bus_rm_device(bmp280_handle) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_i2cBusRemoval_failed;
        return;
    }
}

void bmp280_getCalibration()
{

    uint8_t bmp280_CalibrationDataAddress =
        BMP280_CALIBRATION_DATA_ADDRESS;

    if (i2c_master_transmit_receive(
            bmp280_handle, &bmp280_CalibrationDataAddress,
            sizeof(bmp280_CalibrationDataAddress),
            bmp280_CalibrationData, sizeof(bmp280_CalibrationData),
            I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_calibration_failed;
        return;
    }

    memcpy(&dig_T1, bmp280_CalibrationData, 2);
    memcpy(&dig_T2, bmp280_CalibrationData + 2, 2);
    memcpy(&dig_T3, bmp280_CalibrationData + 4, 2);
    memcpy(&dig_P1, bmp280_CalibrationData + 6, 2);
    memcpy(&dig_P2, bmp280_CalibrationData + 8, 2);
    memcpy(&dig_P3, bmp280_CalibrationData + 10, 2);
    memcpy(&dig_P4, bmp280_CalibrationData + 12, 2);
    memcpy(&dig_P5, bmp280_CalibrationData + 14, 2);
    memcpy(&dig_P6, bmp280_CalibrationData + 16, 2);
    memcpy(&dig_P7, bmp280_CalibrationData + 18, 2);
    memcpy(&dig_P8, bmp280_CalibrationData + 20, 2);
    memcpy(&dig_P9, bmp280_CalibrationData + 22, 2);
    bmp280_status.calibrated = true;
}

static void bmp280_compensatePressureAndTemperature(
    uint32_t ADC_T, uint32_t ADC_P, int32_t* temperature,
    double* pressure)
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

int32_t bmp280_getTemperature()
{
    double  Pressure    = 0;
    int32_t Temperature = 0;

    uint8_t bmp280_ReadAddress = BMP280_READ_ADDRESS;

    if (i2c_master_transmit_receive(
            bmp280_handle, &bmp280_ReadAddress,
            sizeof(bmp280_ReadAddress), bmp280_RawData,
            sizeof(bmp280_RawData), I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_dataRead_failed;
        return Temperature;
    }

    uint32_t ADC_P = ((uint32_t)(bmp280_RawData[0]) << 12) |
        ((uint32_t)(bmp280_RawData[1]) << 4) |
        ((uint32_t)(bmp280_RawData[2]) >> 4);

    uint32_t ADC_T = ((uint32_t)(bmp280_RawData[3]) << 12) |
        ((uint32_t)(bmp280_RawData[4]) << 4) |
        ((uint32_t)(bmp280_RawData[5]) >> 4);

    bmp280_compensatePressureAndTemperature(ADC_T, ADC_P,
                                            &Temperature, &Pressure);
    return Temperature;
}

double bmp280_getPressure()
{
    int32_t Temperature = 0;
    double  Pressure    = 0;

    uint8_t bmp280_ReadAddress = BMP280_READ_ADDRESS;

    if (i2c_master_transmit_receive(
            bmp280_handle, &bmp280_ReadAddress,
            sizeof(bmp280_ReadAddress), bmp280_RawData,
            sizeof(bmp280_RawData), I2c_WAIT_TIME_MS) != ESP_OK)
    {
        bmp280_err = psatErr_bmp280_dataRead_failed;
        return Pressure;
    }

    uint32_t ADC_P = ((uint32_t)(bmp280_RawData[0]) << 12) |
        ((uint32_t)(bmp280_RawData[1]) << 4) |
        ((uint32_t)(bmp280_RawData[2]) >> 4);

    uint32_t ADC_T = ((uint32_t)(bmp280_RawData[3]) << 12) |
        ((uint32_t)(bmp280_RawData[4]) << 4) |
        ((uint32_t)(bmp280_RawData[5]) >> 4);

    bmp280_compensatePressureAndTemperature(ADC_T, ADC_P,
                                            &Temperature, &Pressure);

    return Pressure;
}