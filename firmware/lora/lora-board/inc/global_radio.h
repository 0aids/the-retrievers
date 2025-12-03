#ifndef global_radio_h_INCLUDED
#define global_radio_h_INCLUDED

/*
 * A collection of functions that has it's own state separate from the rest of the program.
 * The implementations will differ depending on whether it's an implementation for LoRa by itself,
 * or an ESP32 -> via I2C -> LoRa, but the underlying operation is the same,
 * Essentially it is abstracted away such that these operations will be used in the same way by the PSAT
 * and the GND ST.
 * */


#include <stdint.h>
#include "packets/packets.h"

#define d_radioStatesXMacro \
    X(gr_RadioStates_Init) \
    X(gr_RadioStates_Idle) \
    X(gr_RadioStates_Rx) \
    X(gr_RadioStates_RxTimeout) \
    X(gr_RadioStates_RxError) \
    X(gr_RadioStates_Tx) \
    X(gr_RadioStates_TxTimeout) \
    X(gr_RadioStates_Unknown) \

#define X(name) name,

typedef enum {
    d_radioStatesXMacro
} e_radioState;

#undef X


const char* const gr_RadioStatesToString(e_radioState state);

void gr_RadioSetTxDoneCallback(void (*callback)(void));

void gr_RadioSetRxDoneCallback(void (*callback)(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr));

void gr_RadioSetTxTimeoutCallback(void (*callback)(void));

void gr_RadioSetRxTimeoutCallback(void (*callback)(void));

void gr_RadioSetRxErrorCallback(void (*callback)(void));

/*
 * Send a buffer over the radio. This is done asynchronously.
 */
void gr_RadioSend(uint8_t *buffer, uint16_t bufferSize);

/*
 * Does a check on the Radio to see if it has received anything, and will
 * run any of the callbacks setup. The radio will only see stuff when Rx is enabled.
 * */
void gr_RadioCheckRecv(); 

// Sets the Radio to RX mode for timeout ms.
// If the radio is made to TX during the timeout it will
// immediately leave Rx mode and move to Tx mode. Is non-blocking
void gr_RadioSetRx(uint32_t timeout);

/*
 * Essentially turn on power saving mode.
 * */
void gr_RadioSetIdle();

void gr_RadioInit();

e_radioState gr_RadioGetStatus();

int16_t gr_RadioGetRSSI();

uint32_t gr_RadioGetTimeOnAir(packet_t *packet);


#endif // global_radio_h_INCLUDED
