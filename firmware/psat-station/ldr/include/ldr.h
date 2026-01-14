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
# include "driver/gpio.h"
#include <driver/gpio.h>
#include "soc/io_mux_reg.h"

#define ADC_ATTEN           ADC_ATTEN_DB_12
#define ADC1_CHAN0          ADC_CHANNEL_1       //This is GPIO1
const static char *TAG = "LDR";

static int adc_raw[2][10];
static int voltage[2][10];

typedef struct {
    adc_oneshot_unit_handle_t adc1;
    adc_cali_handle_t adc1_cali_chan0;
} handlers_t;
extern handlers_t ldr_adcHandlers_g;

typedef struct {
    char *stateString;
} ldr_state;

//setup adc pins
void ldr_setup(void);

//gets voltage(mV) value of the ldr
int ldr_getVoltage(void);

// gets the unit state, gpio state, and channel state
ldr_state ldr_queryState(void);

// deinitialises the ldr
void ldr_deinit(void);