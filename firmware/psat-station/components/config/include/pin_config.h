#pragma once

#include "driver/adc.h"
#include "driver/uart.h"

typedef struct {
    uart_port_t gpsUartNum;
    int gpsTxPin;
    int gpsRxPin;

    int landingButtonPin;
    int prelaunchButtonPin;

    int light1Pin;                 // based on the transistor circuit
    adc2_channel_t light2Channel;  // will be used for redundancy with the ADC

    int buzzerPin;

    int servoPin;
} config_pinConfig_t;

const config_pinConfig_t* config_getPinConfig(void);
