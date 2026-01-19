#include "ldr.h"



//
// Global Variables
//

ldr_handlers_t ldr_adcHandlers_g = {};

//
// Static Function Definitions
//

// This function sets the calibration scheme to be used when the raw adc data is converted to voltage
static bool adcCalibrationInit(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *outHandle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // Sets the calibtration scheme to curve fitting and configures it
    if (!calibrated) {
        ESP_LOGI(ldr_tag_c, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    // Sets the calibtration scheme to line fitting and configures it
    if (!calibrated) {
        ESP_LOGI(ldr_tag_c, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    // Checks for errors
    *outHandle = handle;
    if (ret == ESP_OK) {
        ESP_LOGD(ldr_tag_c, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(ldr_tag_c, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(ldr_tag_c, "Invalid arg or no memory");
    }

    return calibrated;
}


//
// Global Function Definitions
//

void ldr_setup(void)
{
    //-------------ADC1 Init---------------//
    // sets the handle and initConfig1 initialises ADC1 and disables ULP mode
    adc_oneshot_unit_init_cfg_t initConfigLDR = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&initConfigLDR, &ldr_adcHandlers_g.adc1));

    //-------------ADC1 Config---------------//
    // Attenuation determines the range or maximum voltage that we can measure
    // The attenuation will be 12db, aka from 0v-3.1v
    //
    // Bitwidth determines the resolution of the ADC
    // The max amount of bits the esp32 supports will be allocated
    adc_oneshot_chan_cfg_t config = {
        .atten = ldr_adcAtten_d,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ldr_adcHandlers_g.adc1, ldr_adc1Chan0_d, &config));

    //-------------ADC1 Calibration Init---------------//
    // sets the calibration handle to NULL.
    // It will be changed when the adcCalibrationInit function is called
    // Then calls the adcCalibrationInit function
    ldr_adcHandlers_g.adc1CaliChan0 = NULL;
    adcCalibrationInit(ADC_UNIT_1, ldr_adc1Chan0_d, ldr_adcAtten_d, &ldr_adcHandlers_g.adc1CaliChan0);
}

int ldr_getVoltage(void)
{
    int adcRaw;
    int voltage;

    // Reads the raw adc value and then calibrates it to get the voltage
    ESP_ERROR_CHECK(adc_oneshot_read(ldr_adcHandlers_g.adc1, ldr_adc1Chan0_d, &adcRaw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(ldr_adcHandlers_g.adc1CaliChan0, adcRaw, &voltage));

    return voltage;
}

// limits the size of the buffer to 1024 bytes and removes all the data in the buffer
#define ldr_stateConfigurationBufferSize_d 1024
char ldr_stateConfigurationBuffer_g[ldr_stateConfigurationBufferSize_d] = {0};


ldr_state_t ldr_queryState(void) {

    ldr_state_t emptyState = { 0 };
    FILE *memStream;
    char *tempBuffer = NULL;
    size_t size = 0;

    memStream = open_memstream(&tempBuffer, &size);
    if (memStream == NULL) {
        perror("open_memstream failed");
        return emptyState;
    }

    // This writes all the information we need to the stream
    gpio_dump_io_configuration(memStream, ldr_pinMask_d);
    // This flushes the unwritten data 
    // and updates the variables 'buffer' and 'size'
    fclose(memStream);

    size_t trueSize = (ldr_stateConfigurationBufferSize_d - 1 < size) ? ldr_stateConfigurationBufferSize_d - 1 : size;
    // Copies the data in temp buffer to the buffer we will return, while making sure the string ends
    memcpy(ldr_stateConfigurationBuffer_g, tempBuffer, trueSize);
    ldr_stateConfigurationBuffer_g[trueSize] = '\0';

    // Remember to free the dynamic variable
    free(tempBuffer);

    ldr_state_t state = {
        .stateString = ldr_stateConfigurationBuffer_g
    };
    return state;
}

void ldr_deinit(void){
    //Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(ldr_adcHandlers_g.adc1));
    ESP_LOGD(ldr_tag_c, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(ldr_adcHandlers_g.adc1CaliChan0));
}

ldr_preflightTest_t ldr_preflightTest(void) {
    ldr_preflightTest_t test = {};
    
    test.stateBefore = ldr_queryState();
    ldr_setup();
    test.stateMiddle = ldr_queryState();
    test.sampleData = ldr_getVoltage();
    ldr_deinit();
    test.stateAfter = ldr_queryState();

    return test;
}