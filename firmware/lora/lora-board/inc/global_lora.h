#ifndef global_lora_h_INCLUDED
#define global_lora_h_INCLUDED

// AS923 standard
#include "radio.h"
#include <stdint.h>
#define RF_FREQUENCY                                923000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define BUFFER_SIZE                                 6 // Define the payload size here

typedef void (*DefRadioCallback_t) (void);
typedef void (*DefRxCallback_t) (uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

#endif // global_lora_h_INCLUDED
