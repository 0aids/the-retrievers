#include "psat_lora.h"
#include "tremo_delay.h"
#include <string.h>
#include <stdio.h>
#define RX_TIMEOUT_VALUE                            1800

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT
}States_t;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

volatile States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

uint32_t ChipId[2] = {0};

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;


void PsatOnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
}

void PsatOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void PsatOnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void PsatOnRxTimeout( void )
{
    printf("OnRxTimeout\r\n");
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void PsatOnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}

void PsatRadioInit(void) {
    RadioEvents.TxDone = PsatOnTxDone;
    RadioEvents.RxDone = PsatOnRxDone;
    RadioEvents.TxTimeout = PsatOnTxTimeout;
    RadioEvents.RxTimeout = PsatOnRxTimeout;
    RadioEvents.RxError = PsatOnRxError;
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
}

void PsatRadioMain() {
    // For now, set the PSAT to be a beacon broadcasting some random ass text.
    Buffer[0] = 'B';
    Buffer[1] = 'A';
    Buffer[2] = 'L';
    Buffer[3] = 'L';
    Buffer[4] = 'S';

    Radio.Send(Buffer, BufferSize);
    delay_ms(10);

    printf("Sent BALLS\r\n");
}
