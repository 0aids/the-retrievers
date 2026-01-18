#ifndef loraHal_h_INCLUDED
#define loraHal_h_INCLUDED

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the underlying hardware for the LoRa module.
 */
esp_err_t loraHal_init(void);

/**
 * @brief De-initializes the underlying hardware for the LoRa module.
 */
esp_err_t loraHal_deinit(void);

/**
 * @brief Writes a single byte to a LoRa register.
 *
 * @param reg The address of the register to write to.
 * @param val The value to write.
 *
 * @return previous value
 */
uint8_t loraHal_writeReg(uint8_t reg, uint8_t val);

/**
 * @brief Reads a single byte from a LoRa register.
 *
 * @param reg The address of the register to read from.
 * @return uint8_t The value of the register.
 */
uint8_t loraHal_readReg(uint8_t reg);

/**
 * @brief Writes a buffer of data to consecutive LoRa registers.
 *
 * @param reg The starting address of the register to write to.
 * @param buffer Pointer to the data to write.
 * @param numBytes Number of bytes to write.
 */
void loraHal_writeRegContinuous(uint8_t reg, const uint8_t* buffer, uint8_t numBytes);

/**
 * @brief Reads a buffer of data from consecutive LoRa registers.
 *
 * @param reg The starting address of the register to read from.
 * @param buffer Pointer to the destination buffer.
 * @param numBytes Number of bytes to read.
 */
void loraHal_readRegContinuous(uint8_t reg, uint8_t* buffer, uint8_t numBytes);

void loraHal_registerRxDoneIsr(void(*isr)(void*));

void loraHal_deregisterRxDoneIsr();

#endif // loraHal_h_INCLUDED
