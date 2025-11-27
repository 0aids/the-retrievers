#include "ground_lora.h"
#include <string.h>
#include <stdio.h>
#include "global_lora.h"
#include "radio.h"
#include "sx126x-board.h"
#include "tremo_delay.h"

#define RX_TIMEOUT 0 //ms

static uint8_t Buffer[BUFFER_SIZE];
static uint8_t BufferSize = BUFFER_SIZE;
static RadioEvents_t RadioEvents;


void GroundOnTxDone( void )
{
    // Radio.Sleep( );
}

void GroundOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    printf("Received: ");
    for (uint8_t i = 0; i < BufferSize; i++) {
        printf("%c", Buffer[i]);
    }
    printf("\r\n");
    printf("Time on air: %ld ms\r\n", Radio.TimeOnAir(MODEM_LORA, size));
    printf("Signal strength: %d dbm\r\n", Radio.Rssi(MODEM_LORA));
}

void GroundOnTxTimeout( void )
{
    // Radio.Sleep( );
}

void GroundOnRxTimeout( void )
{
    printf("OnRxTimeout\r\n");
    // Radio.Sleep( );
}

void GroundOnRxError( void )
{
    // Radio.Sleep( );
}

void GroundRadioInit(void) {
    RadioEvents.TxDone = GroundOnTxDone;
    RadioEvents.RxDone = GroundOnRxDone;
    RadioEvents.TxTimeout = GroundOnTxTimeout;
    RadioEvents.RxTimeout = GroundOnRxTimeout;
    RadioEvents.RxError = GroundOnRxError;
    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    Radio.Rx(RX_TIMEOUT);
    // SX126xAntSwOn();
}


void GroundRadioMain() {
    // For now, set the PSAT to be a beacon broadcasting some random ass text.
    printf("Waiting for payload...\r\n");
    Radio.IrqProcess();
}
