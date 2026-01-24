#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include <stdbool.h>


const static char* battery_tag_c = "Battery";

//Global struct variables
typedef struct
{
    adc_oneshot_unit_handle_t adcHandle;
    adc_cali_handle_t         adcCaliChan;
} battery_handlers_t;

extern battery_handlers_t battery_adcHandlers_g;

typedef struct
{
    char*          stateString;
    adc_unit_t     unit;
    adc_ulp_mode_t ulpMode;
    adc_atten_t    atten;
    adc_bitwidth_t bitwidth;
} battery_state_t;

extern battery_state_t battery_state_g;

typedef struct
{
    battery_state_t stateBefore;
    battery_state_t stateMiddle;
    int             sampleData;
    battery_state_t stateAfter;
} battery_preflightTest_t;

void        battery_setup(void);
int         battery_getVoltage(void);
void        battery_queryState(char* stateString);
void        battery_deinit(void);
// This function allocates dynamic memory, dont forget to free it after.
void        battery_callocTestState(battery_preflightTest_t* test);
void        battery_preflightTest(battery_preflightTest_t* test);
// Call this to free the dynamic memory
void        battery_freeTestState(battery_preflightTest_t* test);