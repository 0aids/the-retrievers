#pragma once

#include "driver/uart.h"

#define CFG_GPS_UART_NUM_d         UART_NUM_1
#define CFG_GPS_TX_PIN_d           27
#define CFG_GPS_RX_PIN_d           25
#define CFG_LANDING_BUTTON_PIN_d   22
#define CFG_PRELAUNCH_BUTTON_PIN_d 21
#define CFG_LIGHT1_PIN_d           19
#define CFG_BUZZER_PIN_d           23
#define CFG_SERVO_PIN_d            13

#define CFG_GPS_TX_PIN_d           37
#define CFG_GPS_RX_PIN_d           38 // We don't care about this. 
#define CFG_LANDING_BUTTON_PIN_d   39
#define CFG_PRELAUNCH_BUTTON_PIN_d 40
#define CFG_LIGHT1_PIN_d           2
#define CFG_BUZZER_PIN_d           21
<<<<<<< Updated upstream
#define CFG_SERVO_PIN_d            8
=======
#define CFG_SERVO_PIN_d            8

#define LDR_ADC_UNIT_d             ADC_UNIT_1
#define LDR_ADC_CHANNEL_d          ADC_CHANNEL_1
#define LDR_ULP_MODE_d             ADC_ULP_MODE_DISABLE
#define LDR_ADC_ATTEN_d            ADC_ATTEN_DB_12
#define LDR_ADC_BITWIDTH_d         ADC_BITWIDTH_DEFAULT
#define LDR_PIN_MASK_d             (1ULL << 18)
#define LDR_STATE_CONFIG_BUFFER_SIZE_d (1024)

>>>>>>> Stashed changes
