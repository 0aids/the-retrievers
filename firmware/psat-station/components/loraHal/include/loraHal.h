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
esp_err_t loraHal_writeReg(uint8_t reg, uint8_t val, uint8_t* readVal);

/**
 * @brief Reads a single byte from a LoRa register.
 *
 * @param reg The address of the register to read from.
 * @param readVal Pointer to store the value of the register.
 * @return esp_err_t ESP_OK if successful, an error code otherwise.
 */
esp_err_t loraHal_readReg(uint8_t reg, uint8_t* readVal);

/**
 * @brief Writes a buffer of data to consecutive LoRa registers.
 *
 * @param reg The starting address of the register to write to.
 * @param buffer Pointer to the data to write.
 * @param numBytes Number of bytes to write.
 * @return esp_err_t ESP_OK if successful, an error code otherwise.
 */
esp_err_t loraHal_writeRegContinuous(uint8_t reg, const uint8_t* buffer, uint8_t numBytes);

/**
 * @brief Reads a buffer of data from consecutive LoRa registers.
 *
 * @param reg The starting address of the register to read from.
 * @param buffer Pointer to the destination buffer.
 * @param numBytes Number of bytes to read.
 * @return esp_err_t ESP_OK if successful, an error code otherwise.
 */
esp_err_t loraHal_readRegContinuous(uint8_t reg, uint8_t* buffer, uint8_t numBytes);

esp_err_t loraHal_registerRxDoneIsr(void(*isr)(void*));

esp_err_t loraHal_deregisterRxDoneIsr();

#endif // loraHal_h_INCLUDED
