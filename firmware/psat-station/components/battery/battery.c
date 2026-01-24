#include "include/battery.h"   
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_err.h"
#include "pin_config.h"
#include "freertos/task.h"


extern esp_err_t adc_cali_delete_scheme_line_fitting(adc_cali_handle_t handle);

// Global Variables

battery_handlers_t battery_adcHandlers_g;
battery_state_t battery_state;

// Static function definitions
// This function sets the calibration scheme to be used when the raw adc data is converted to voltage

static bool adcCalibrationInit(adc_cali_handle_t *outHandle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // Sets the calibration scheme to curve fitting and configures it
    if (!calibrated) {
        ESP_LOGI(battery_tag_c, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = BATTERY_ADC_UNIT_d,
            .chan = BATTERY_ADC_CHANNEL_d,
            .atten = BATTERY_ADC_ATTEN_d,
            .bitwidth = BATTERY_ADC_BITWIDTH_d,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    // Sets the calibration scheme to line fitting and configures it
    if (!calibrated) {
        ESP_LOGI(battery_tag_c, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = BATTERY_ADC_UNIT_d,
            .atten = BATTERY_ADC_ATTEN_d,
            .bitwidth = BATTERY_ADC_BITWIDTH_d,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    // Checks for errors5
    *outHandle = handle;
    if (ret == ESP_OK) {
        ESP_LOGD(battery_tag_c, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(battery_tag_c, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(battery_tag_c, "Invalid arg or no memory");
    }

    return calibrated;
}

// Global function definitions

void battery_setup(void){
    // Initialises the ADC unit and disables ultra low power mode
    adc_oneshot_unit_init_cfg_t initConfigBattery = {
        .unit_id = BATTERY_ADC_UNIT_d,
        .ulp_mode = BATTERY_ULP_MODE_d,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&initConfigBattery,  &battery_adcHandlers_g.adcHandle));
    battery_state.unit = initConfigBattery.unit_id;
    battery_state.ulpMode = initConfigBattery.ulp_mode;

    // Configures the bitwidth (resolution of the adc signal) and attenuation (what the max voltage is)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = BATTERY_ADC_BITWIDTH_d,
        .atten = BATTERY_ADC_ATTEN_d,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(battery_adcHandlers_g.adcHandle, BATTERY_ADC_CHANNEL_d, &config));
    battery_state.bitwidth = config.bitwidth,
    battery_state.atten = config.atten;

    // Sets the calibration scheme for calculating voltage based off of the esp32 settings
    adcCalibrationInit(&battery_adcHandlers_g.adcCaliChan);
}

// Returns the voltage(mV) measured on the pin
int battery_getVoltage(void){
    int adcRaw;
    int voltage;

    ESP_ERROR_CHECK(adc_oneshot_read(battery_adcHandlers_g.adcHandle, BATTERY_ADC_CHANNEL_d, &adcRaw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(battery_adcHandlers_g.adcCaliChan, adcRaw, &voltage));

    return voltage;
}

void battery_queryState(char* stateString)
{

    char*       emptyBuffer = NULL;
    FILE*       memStream;
    size_t      size       = 0;

    memStream = open_memstream(&emptyBuffer, &size);
    if (memStream == NULL)
    {
        return;
    }

    // This writes all the information we need to the stream
    gpio_dump_io_configuration(memStream, BATTERY_PIN_MASK_d);
    // This flushes the unwritten data
    // and updates the variables 'buffer' and 'size'
    fclose(memStream);

    size_t trueSize =
        (BATTERY_STATE_CONFIG_BUFFER_SIZE_d - 1 < size) ?
        BATTERY_STATE_CONFIG_BUFFER_SIZE_d - 1 :
        size;
    // Copies the data in temp buffer to the buffer we will return, while making sure the string ends
    memcpy(stateString, emptyBuffer, trueSize);
    stateString[trueSize] = '\0';
}

void battery_deinit(void){
    ESP_ERROR_CHECK(adc_oneshot_del_unit(battery_adcHandlers_g.adcHandle));
    ESP_LOGD(battery_tag_c, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(battery_adcHandlers_g.adcCaliChan));
}

void battery_preflightTest(battery_preflightTest_t* test)
{
    
    // Gets the state before anything happens
    battery_queryState(test->stateBefore.stateString);
    // Gets the state after the ldr is setup
    battery_setup();
    battery_queryState(test->stateMiddle.stateString);
    // Gets the voltage is accurate
    test->sampleData  = battery_getVoltage();

    // Gets the state after the ldr deinits
    battery_deinit();
    battery_queryState(test->stateAfter.stateString);
}

void battery_callocTestState(battery_preflightTest_t* test)
{
    test->stateBefore.stateString = calloc(BATTERY_STATE_CONFIG_BUFFER_SIZE_d, 1);
    test->stateMiddle.stateString = calloc(BATTERY_STATE_CONFIG_BUFFER_SIZE_d, 1);
    test->stateAfter.stateString = calloc(BATTERY_STATE_CONFIG_BUFFER_SIZE_d, 1);
}

void battery_freeTestState(battery_preflightTest_t* test)
{
    free(test->stateBefore.stateString);
    free(test->stateMiddle.stateString);
    free(test->stateAfter.stateString);
}