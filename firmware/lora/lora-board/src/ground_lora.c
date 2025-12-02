#include "ground_lora.h"
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "global_radio.h"
#include "packets/packets.h"
#include "global_lora.h"
#include "radio.h"
#include "tremo_delay.h"
#include "timer.h"

#define RX_TIMEOUT 0 //ms

static packet_t g_packetRecv = {EMPTY};
static packet_t g_packetSend = {EMPTY};
TimerTime_t g_timer = 0;

void GroundOnTxDone( void )
{
    printf("tx done\r\n");
    gr_RadioSetRx(RX_TIMEOUT);
    // gr_RadioSleep( );
}

void GroundOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // gr_RadioSleep( );
    g_packetRecv = ParsePacket(payload, size);
    printPacketStats(&g_packetRecv);
    delay_ms(500);
    switch (g_packetRecv.type) {
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

static inline void BlockingRadioSend(uint8_t* buffer, uint16_t bufferSize) {
    printf("Sending packet!\r\n");
    gr_RadioSend(buffer, bufferSize);
    // while (gr_RadioGetStatus() != gr_RadioStates_Idle)
    // {
        // Sleep until we finished transmitting.
    DelayMs(100);
    // }
}

void GroundRadioInit(void) {
    gr_RadioSetTxDoneCallback(GroundOnTxDone);
    gr_RadioSetRxDoneCallback(GroundOnRxDone);
    gr_RadioSetTxTimeoutCallback(GroundOnTxTimeout);
    gr_RadioSetRxTimeoutCallback(GroundOnRxTimeout);
    gr_RadioSetRxErrorCallback(GroundOnRxError);

    gr_RadioInit();
    // As a test we'll ping on startup and then listen for the beacon details later afterwards.
    printf("Starting up\r\n");

    printf("Pinging!\r\n");
    g_packetSend = CreatePacket(PING, NULL, 0);
    BlockingRadioSend((uint8_t*)&g_packetSend, 1);

    printf("Waiting 2s for response!\r\n");
    g_timer = TimerGetCurrentTime();
    while (TimerGetCurrentTime() < g_timer + 5000)
    {
        gr_RadioSetRx(0);
        gr_RadioCheckRecv();
        DelayMs(10);
        if (g_packetRecv.type == PONG) {
            printf("success! Received pong!\r\n");
            goto pong_success;
        }
    }
    printf("Fail! Pong not received \r\n");

pong_success:
    printf("Initial communication test complete\r\n");
    printf("Falling back to receive only mode (Pingpong only)\r\n");
    gr_RadioSetRx(0);
}

uint8_t g_groundRadioMainIters = 0;
void GroundRadioMain() {
    // For now, set the PSAT to be a beacon broadcasting some random ass text.
    g_groundRadioMainIters++;
    e_radioState radioState = gr_RadioGetStatus();
    if (g_groundRadioMainIters % 20)
        printf("State: %s\r\n", gr_RadioStatesToString(radioState));
    gr_RadioCheckRecv();
}
