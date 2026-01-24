#include "ldr.h"
#include <stdlib.h>
#include <string.h>
#include "pin_config.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "freertos/task.h"

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
extern esp_err_t adc_cali_create_scheme_line_fitting(adc_cali_line_fitting_config_t *,  adc_cali_handle_t *);
extern esp_err_t adc_cali_delete_scheme_line_fitting(adc_cali_handle_t *);
#endif
const static char* ldr_tag_c = "LDR";

//
// Global Variables
//

ldr_handlers_t ldr_adcHandlers_g = {};

//
// Static Function Definitions
//

// This function sets the calibration scheme to be used when the raw adc data is converted to voltage
static bool adcCalibrationInit(adc_cali_handle_t* outHandle)
{
    adc_cali_handle_t handle     = NULL;
    esp_err_t         ret        = ESP_FAIL;
    bool              calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    // Sets the calibtration scheme to curve fitting and configures it
    if (!calibrated)
    {
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id  = LDR_ADC_UNIT_d,
            .chan     = LDR_ADC_CHANNEL_d,
            .atten    = LDR_ADC_ATTEN_d,
            .bitwidth = LDR_ADC_BITWIDTH_d,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config,
                                                   &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    // Sets the calibtration scheme to line fitting and configures it
    if (!calibrated)
    {

        adc_cali_line_fitting_config_t cali_config = {
            .unit_id  = LDR_ADC_UNIT_d,
            .atten    = LDR_ADC_ATTEN_d,
            .bitwidth = LDR_ADC_BITWIDTH_d,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config,
                                                  &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    // Checks for errors
    *outHandle = handle;

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
        .unit_id  = LDR_ADC_UNIT_d,
        .ulp_mode = LDR_ULP_MODE_d,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&initConfigLDR,
                                         &ldr_adcHandlers_g.adcOneshotHandle));

    //-------------ADC1 Config---------------//
    // Attenuation determines the range or maximum voltage that we can measure
    // The attenuation will be 12db, aka from 0v-3.1v
    //
    // Bitwidth determines the resolution of the ADC
    // The max amount of bits the esp32 supports will be allocated
    adc_oneshot_chan_cfg_t config = {
        .atten    = LDR_ADC_ATTEN_d,
        .bitwidth = LDR_ADC_BITWIDTH_d,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        ldr_adcHandlers_g.adcOneshotHandle, LDR_ADC_CHANNEL_d, &config));

    //-------------ADC1 Calibration Init---------------//
    // sets the calibration handle to NULL.
    // It will be changed when the adcCalibrationInit function is called
    // Then calls the adcCalibrationInit function
    ldr_adcHandlers_g.adcCaliChanHandle = NULL;
    adcCalibrationInit(&ldr_adcHandlers_g.adcCaliChanHandle);
}

int ldr_getVoltage(void)
{
    int adcRaw;
    int voltage;

    // Reads the raw adc value and then calibrates it to get the voltage
    ESP_ERROR_CHECK(adc_oneshot_read(ldr_adcHandlers_g.adcOneshotHandle,
                                     LDR_ADC_CHANNEL_d, &adcRaw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
        ldr_adcHandlers_g.adcCaliChanHandle, adcRaw, &voltage));

    return voltage;
}


void ldr_queryState(char* stateString)
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
    gpio_dump_io_configuration(memStream, LDR_PIN_MASK_d);
    // This flushes the unwritten data
    // and updates the variables 'buffer' and 'size'
    fclose(memStream);

    size_t trueSize =
        (LDR_STATE_CONFIG_BUFFER_SIZE_d - 1 < size) ?
        LDR_STATE_CONFIG_BUFFER_SIZE_d - 1 :
        size;
    // Copies the data in temp buffer to the buffer we will return, while making sure the string ends
    memcpy(stateString, emptyBuffer, trueSize);
    stateString[trueSize] = '\0';
}

void ldr_deinit(void)
{
    //Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(ldr_adcHandlers_g.adcOneshotHandle));

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(
        ldr_adcHandlers_g.adcCaliChanHandle));
#endif
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(
        ldr_adcHandlers_g.adcCaliChanHandle));
#endif
}

void ldr_preflightTest(ldr_preflightTest_t* test)
{
    
    // Gets the state before anything happens
    ldr_queryState(test->stateBefore.stateString);
    // Gets the state after the ldr is setup
    ldr_setup();
    ldr_queryState(test->stateMiddle.stateString);
    // Gets the voltage is accurate
    test->sampleData  = ldr_getVoltage();

    // Gets the state after the ldr deinits
    ldr_deinit();
    ldr_queryState(test->stateAfter.stateString);
}

void ldr_callocTestState(ldr_preflightTest_t* test)
{
    test->stateBefore.stateString = calloc(LDR_STATE_CONFIG_BUFFER_SIZE_d, 1);
    test->stateMiddle.stateString = calloc(LDR_STATE_CONFIG_BUFFER_SIZE_d, 1);
    test->stateAfter.stateString = calloc(LDR_STATE_CONFIG_BUFFER_SIZE_d, 1);
}

void ldr_freeTestState(ldr_preflightTest_t* test)
{
    free(test->stateBefore.stateString);
    free(test->stateMiddle.stateString);
    free(test->stateAfter.stateString);
}