#include "pin_config.h"

static const config_pinConfig_t board_cfg = {.gpsUartNum = UART_NUM_1,
                                             .gpsTxPin = 27,
                                             .gpsRxPin = 25,
                                             .landingButtonPin = 22,
                                             .prelaunchButtonPin = 21,
                                             .light1Pin = 19,
                                             .light2Channel = ADC2_CHANNEL_0,
                                             .servoPin = 13,
                                             .buzzerPin = 23};

const config_pinConfig_t* config_getPinConfig(void) { return &board_cfg; }
