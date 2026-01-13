#pragma once

typedef struct {
    int gpsUartNum;
    int gpsTxPin;
    int gpsRxPin;

    int landingButtonPin;
    int prelaunchButtonPin;
    int ldrPin;
} config_pinConfig_t;

const config_pinConfig_t* config_getPinConfig(void);
