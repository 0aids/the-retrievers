#include "ldr.h"
#include "freertos/idf_additions.h"


handlers_t ldr_adcHandlers_g = {};


static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    // Sets the calibtration scheme to curve fitting and configures it
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is Curve Fitting");
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

    // Checks for errors
    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(void)
{
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(ldr_adcHandlers_g.adc1_cali_chan0));
}

void ldr_setup(void)
{   
    //-------------ADC1 Init---------------//
    // sets the handle and init_config1 initialises ADC1 and disables ULP mode 
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &ldr_adcHandlers_g.adc1));

    //-------------ADC1 Config---------------//
    // Attenuation determines the range or maximum voltage that we can measure
    // The attenuation will be 12db, aka from 0v-3.1v
    //
    // Bitwidth determines the resolution of the ADC
    // The max amount of bits the esp32 supports will be allocated 
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ldr_adcHandlers_g.adc1, ADC1_CHAN0, &config));

    //-------------ADC1 Calibration Init---------------//
    // sets the calibration handle to NULL. 
    // It will be changed when the calibration_init function is called 
    // Then calls the calibration_init function
    ldr_adcHandlers_g.adc1_cali_chan0 = NULL;
    bool adc_chan0_calibrated = adc_calibration_init(ADC_UNIT_1, ADC1_CHAN0, ADC_ATTEN, &ldr_adcHandlers_g.adc1_cali_chan0);
}

int ldr_get_voltage(void)
{
    ESP_ERROR_CHECK(adc_oneshot_read(ldr_adcHandlers_g.adc1, ADC1_CHAN0, &adc_raw[0][0]));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN0, adc_raw[0][0]);
    //
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(ldr_adcHandlers_g.adc1_cali_chan0, adc_raw[0][0], &voltage[0][0]));
    ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN0, voltage[0][0]);

    return voltage[0][0];
}

#define ldr_stateConfigurationBufferSize_d 1024
char ldr_stateConfigurationBuffer_g[ldr_stateConfigurationBufferSize_d] = {0};


ldr_state ldr_queryState(void) {

    ldr_state emptyState = {};
    FILE *mem_stream;
    char *temp_buffer = NULL;
    size_t size = 0;
    mem_stream = open_memstream(&temp_buffer, &size);
    if (mem_stream == NULL) {
        perror("open_memstream failed");
        return emptyState;
    }

    // This writes all the information we need to the stream
    gpio_dump_io_configuration(mem_stream, SOC_GPIO_VALID_GPIO_MASK);
    // This flushes the unwritten data and updates the variables 'buffer' and 'size'
    fclose(mem_stream);

    size_t true_size = (ldr_stateConfigurationBufferSize_d - 1 < size) ? ldr_stateConfigurationBufferSize_d - 1 : size; 

    memcpy(ldr_stateConfigurationBuffer_g, temp_buffer, true_size);
    ldr_stateConfigurationBuffer_g[true_size] = '\0';

    free(temp_buffer);

    ldr_state state = {
        ldr_stateConfigurationBuffer_g
    };
    return state;
}

void ldr_deinit(void){
    //Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(ldr_adcHandlers_g.adc1));
    adc_calibration_deinit();
}


void app_main(void){
    printf("hello wrold!\r\n");
    // while (1){
    //     ESP_LOGI(TAG, "Hello world");
    //     ldr_setup();
    //     ldr_state state1 = ldr_queryState();
    //     ESP_LOGI(TAG, "%s",state1.stateString);

    //     ESP_LOGI(TAG, "voltage: %dmV", ldr_get_voltage());        
    //     ldr_state state2 = ldr_queryState();
    //     ESP_LOGI(TAG, "%s",state2.stateString);

    //     ldr_deinit();
    //     ldr_state state3 = ldr_queryState();
    //     ESP_LOGI(TAG, "%s",state3.stateString);
    // }
    // vTaskDelay(1000);
}