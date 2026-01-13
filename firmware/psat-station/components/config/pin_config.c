#include "pin_config.h"

static const config_pinConfig_t board_cfg = {.gpsUartNum = 1,
                                             .gpsTxPin = 27,
                                             .gpsRxPin = 25,
                                             .landingButtonPin = 19,
                                             .prelaunchButtonPin = 21,
                                             .ldrPin = 22};

const config_pinConfig_t* config_getPinConfig(void) { return &board_cfg; }
