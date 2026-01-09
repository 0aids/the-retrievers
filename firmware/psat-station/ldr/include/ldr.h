#include <stdio.h>
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"

//setup adc pins
void ldr_setup(void);

//gets voltage value of the ldr
int ldr_get(int pin);