#include "ground_lora.h"
#include <string.h>
#include <stdio.h>
#include "global_radio.h"
#include "packets/packets.h"
#include "global_lora.h"
#include "radio.h"
#include "tremo_delay.h"

#define RX_TIMEOUT 0 //ms

static packet_t g_packet;

void GroundOnTxDone( void )
{
    printf("tx done\r\n");
    gr_RadioSetRx(RX_TIMEOUT);
    // gr_RadioSleep( );
}

void GroundOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // gr_RadioSleep( );
    g_packet = ParsePacket(payload, size);
    printPacketStats(&g_packet);
    delay_ms(2000);
    switch (g_packet.type) {
        case PING:
            d_pingResponse();
        default:
            break;

    }
}

void GroundOnTxTimeout( void )
{
    printf("tx timeout\r\n");
    // gr_RadioSleep( );
}

void GroundOnRxTimeout( void )
{
    printf("OnRxTimeout\r\n");
    // gr_RadioSleep( );
}

void GroundOnRxError( void )
{
    printf("RX Error\r\n");
    // gr_RadioSleep( );
}

void GroundRadioInit(void) {
    gr_RadioSetTxDoneCallback(GroundOnTxDone);
    gr_RadioSetRxDoneCallback(GroundOnRxDone);
    gr_RadioSetTxTimeoutCallback(GroundOnTxTimeout);
    gr_RadioSetRxTimeoutCallback(GroundOnRxTimeout);
    gr_RadioSetRxErrorCallback(GroundOnRxError);

    gr_RadioInit();
    gr_RadioSetRx(0);
}


void GroundRadioMain() {
    // For now, set the PSAT to be a beacon broadcasting some random ass text.
    printf("Waiting for payload...\r\n");
    e_radioState radioState = gr_RadioGetStatus();
    printf("State: %s\r\n", gr_RadioStatesToString(radioState));
    gr_RadioCheckRecv();
    printf("\r\n");
}
