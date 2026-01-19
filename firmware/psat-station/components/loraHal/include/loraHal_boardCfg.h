#ifndef loraHal_board_config_h_INCLUDED
#define loraHal_board_config_h_INCLUDED

// SPI definitions for the LoRa module
#define loraHalBoardCfg_spiMisoGpio_d 7
#define loraHalBoardCfg_spiMosiGpio_d 6
#define loraHalBoardCfg_spiSclkGpio_d 3
#define loraHalBoardCfg_spiCsGpio_d  4
#define loraHalBoardCfg_spiHost_d     SPI3_HOST

// LoRa chip GPIO definitions
#define loraHalBoardCfg_dio0Gpio_d    GPIO_NUM_16
#define loraHalBoardCfg_enableGpio_d  GPIO_NUM_15
#define loraHalBoardCfg_resetGpio_d   -1 // -1 if not used

// SPI Clock Speed
#define loraHalBoardCfg_spiClockSpeedHz_d (400000)

#endif // loraHal_board_config_h_INCLUDED
