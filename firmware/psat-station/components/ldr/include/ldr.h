#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include <stdbool.h>
#include "shared.h"


const static char* ldr_tag_c = "LDR";

//Global structs

typedef struct
{
    adc_oneshot_unit_handle_t adcOneshotHandle;
    adc_cali_handle_t         adcCaliChanHandle;
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
// In millivolts
int                 ldr_getVoltage(void);
ldr_state_t         ldr_queryState(void);
void                ldr_deinit(void);
ldr_preflightTest_t ldr_preflightTest(void);
psatErrStates_e check_err(void);