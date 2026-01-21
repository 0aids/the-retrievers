#ifndef LDR_H
#define LDR_H

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

#define ldr_adcAtten_d  ADC_ATTEN_DB_12
#define ldr_adc1Chan0_d ADC_CHANNEL_0 // This is GPIO4
#define ldr_pinMask_d   (1ULL << 4)   // pin mask for GPIO 4

const static char* ldr_tag_c = "LDR";

//Global structs

typedef struct
{
    adc_oneshot_unit_handle_t adc1;
    adc_cali_handle_t         adc1CaliChan0;
} ldr_handlers_t;

extern ldr_handlers_t ldr_adcHandlers_g;

typedef struct
{
    char* stateString;
} ldr_state_t;

typedef struct
{
    ldr_state_t stateBefore;
    ldr_state_t stateMiddle;
    int         sampleData;
    ldr_state_t stateAfter;
} ldr_preflightTest_t;

void                ldr_setup(void);
int                 ldr_getVoltage(void);
ldr_state_t         ldr_queryState(void);
void                ldr_deinit(void);
ldr_preflightTest_t ldr_preflightTest(void);

#endif // LDR_H
