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

// Defining our stuff here.

// /*!
//  * \brief Function to be executed on Radio Tx Done event
//  */
// void lora_onTxDone( void );

// /*!
//  * \brief Function to be executed on Radio Rx Done event
//  */
// void lora_onRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr
// );

// /*!
//  * \brief Function executed on Radio Tx Timeout event
//  */
// void lora_onTxTimeout( void );

// /*!
//  * \brief Function executed on Radio Rx Timeout event
//  */
// void lora_onRxTimeout( void );

// /*!
//  * \brief Function executed on Radio Rx Error event
//  */
// void lora_onRxError( void );

// clang-format off
// clang-format on

#endif /* __LORA_CONFIG_H */
