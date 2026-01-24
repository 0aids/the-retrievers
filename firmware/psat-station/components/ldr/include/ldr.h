#pragma once
#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include <stdbool.h>




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
void                ldr_queryState(char* stateString);
void                ldr_deinit(void);

void ldr_preflightTest(ldr_preflightTest_t* test);
// This function allocates dynamic memory, dont forget to free it
void ldr_callocTestState(ldr_preflightTest_t* test);
// Frees the dynamic memory that was allocated in the test input struct
void ldr_freeTestState(ldr_preflightTest_t* test);