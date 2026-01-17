
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include <stdbool.h> 
#include "freertos/idf_additions.h"

//Constants
#define battery_adcAtten_d                      ADC_ATTEN_DB_12
#define battery_adc1Chan0_d                     ADC_CHANNEL_0       // This is GPIO4
#define battery_pinMask_d                       (1ULL << 4)         // pin mask for GPIO 4
#define battery_stateConfigBufferSize_d         (1024)

const static char* battery_tag_c = "Battery";

//Global struct variables
typedef struct {
    adc_oneshot_unit_handle_t adcHandle;
    adc_cali_handle_t adcCaliChan0;
} battery_handlers_t;

extern battery_handlers_t battery_adcHandlers_g;

typedef struct {
    char* stateString;
    adc_unit_t unit;
    adc_ulp_mode_t ulpMode;
    adc_atten_t atten;
    adc_bitwidth_t bitwidth; 
} battery_state_t;

extern battery_state_t battery_state_g;

typedef struct {
    battery_state_t stateBefore;
    battery_state_t stateMiddle;
    int sampleData;
    battery_state_t stateAfter;
} battery_preflightTest_t;


void battery_setup(void);
int battery_getVoltage(void);
battery_state_t battery_queryState(void);
void battery_deinit(void);
battery_preflightTest_t battery_preflightTest(void);