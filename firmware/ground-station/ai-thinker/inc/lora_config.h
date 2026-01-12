#ifndef __LORA_CONFIG_H
#define __LORA_CONFIG_H

#include "radio.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "tremo_gpio.h"

#define CONFIG_LORA_RFSW_CTRL_GPIOX GPIOD
#define CONFIG_LORA_RFSW_CTRL_PIN GPIO_PIN_11

#define CONFIG_LORA_RFSW_VDD_GPIOX GPIOA
#define CONFIG_LORA_RFSW_VDD_PIN GPIO_PIN_10

#ifdef __cplusplus
}

#endif

void aiLora_init();

void aiLora_onTxDone( void );

void aiLora_onRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

void aiLora_onTxTimeout( void );

void aiLora_onRxTimeout( void );

void aiLora_onRxError( void );

// Defining our stuff here.

#endif /* __LORA_CONFIG_H */
