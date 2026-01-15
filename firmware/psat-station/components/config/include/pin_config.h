#pragma once

typedef struct {
    int gpsUartNum;
    int gpsTxPin;
    int gpsRxPin;
    // ill add more config later
} config_pinConfig_t;

const config_pinConfig_t* config_getPinConfig(void);
