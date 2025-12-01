#include "psat_lora.h"
#include "global_radio.h"
#include "tremo_delay.h"
#include <stdint.h>
#include <string.h>
#include "packets/packets.h"
#include <stdio.h>
#define RX_TIMEOUT_VALUE                            2500
#define RX_CONTINUOUS 0

#define d_psatRadioStatesXMacro \
    X(psatRadioStates_ToSend) \
    X(psatRadioStates_Sending) \
    X(psatRadioStates_SendDone) \
    X(psatRadioStates_ToReceive) \
    X(psatRadioStates_Receiving) \
    X(psatRadioStates_ReceiveDone) \
    X(psatRadioStates_ReceiveTimedOut) \
    X(psatRadioStates_Idle) \

#define X(name) name,

typedef enum  {
    d_psatRadioStatesXMacro
} e_psatRadioState;

#undef X


#define X(name) case name: return #name;
const char* PsatRadioStateToString(e_psatRadioState state) {
    switch (state) {
        d_psatRadioStatesXMacro
        default:                return "UNKNOWN_RADIO_STATE";
    }
}
#undef X

static e_psatRadioState g_psatRadioState = psatRadioStates_Idle;
/*!
 * Radio events function pointer
 */
static packet_t g_recvPacket;
static packet_t g_sendPacket;

// Listens for pings and requests.
// If a ping is requested, then send a pong back.
// If a req is received, return an ack
void PsatRadioRX() {
    switch (g_recvPacket.type) {
        case PING:
            {
                d_pingResponse();
            }
            break;
        case GPS_REQ:
        case BUZ_REQ:
        case FOLD_REQ:
            {
                d_ackResponse();
            }
            break;
        default:
            break;
    }
}


void PsatOnTxDone( void )
{
    gr_RadioSetIdle( );
    g_psatRadioState = psatRadioStates_SendDone;
}

void PsatOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // This will need to be changed later to forward all data via i2c, but for now
    // we will just parse it here.
    g_recvPacket = ParsePacket(payload, size + 1);
    printPacketStats(&g_recvPacket);
    PsatRadioRX();
    g_psatRadioState = psatRadioStates_ReceiveDone;
}

void PsatOnTxTimeout( void )
{
    // RadioSleep( );
}

void PsatOnRxTimeout( void )
{
    printf("PSAT rx timeout\r\n");
    // RadioSleep( );
}

void PsatOnRxError( void )
{
    printf("PSAT rx ERROR\r\n");
    // RadioSleep( );
}

void PsatRadioInit(void) {
    gr_RadioSetTxDoneCallback(PsatOnTxDone);
    gr_RadioSetRxDoneCallback(PsatOnRxDone);
    gr_RadioSetTxTimeoutCallback(PsatOnTxTimeout);
    gr_RadioSetRxTimeoutCallback(PsatOnRxTimeout);
    gr_RadioSetRxErrorCallback(PsatOnRxError);
    gr_RadioInit();
}

volatile uint16_t recvTries = 0;
#define recvTrialThreshold 5

void PsatRadioMain() {
    printf("Before psat state: %s\r\n", PsatRadioStateToString(g_psatRadioState));
    switch (g_psatRadioState) {
        case psatRadioStates_ToSend:
            g_psatRadioState = psatRadioStates_Sending;
            g_sendPacket = CreatePacket(PING, NULL, 0);
            gr_RadioSend((uint8_t*)&g_sendPacket, 1);

        case psatRadioStates_Sending:
            break;

        case psatRadioStates_SendDone:
            g_psatRadioState = psatRadioStates_ToReceive;
            break;

        case psatRadioStates_ToReceive:
            g_psatRadioState = psatRadioStates_Receiving;
            gr_RadioSetRx(RX_TIMEOUT_VALUE);
            break;

        case psatRadioStates_Receiving:
            gr_RadioSetRx(RX_TIMEOUT_VALUE);
            recvTries++;
            if (recvTries >= 5) {
                g_psatRadioState = psatRadioStates_ReceiveTimedOut;
                recvTries = 0;
            }
            break;

        case psatRadioStates_ReceiveDone:
            recvTries = 0;
        case psatRadioStates_Idle:
            g_psatRadioState = psatRadioStates_ToSend;
            break;

        case psatRadioStates_ReceiveTimedOut:
            printf("Receive Timed out!!!\r\n");
            g_psatRadioState = psatRadioStates_ToSend;
            break;
    }

    printf("Current psat state: %s\r\n", PsatRadioStateToString(g_psatRadioState));
    gr_RadioCheckRecv();
    printf(
        "===========================================================\r\n"
        "===========================================================\r\n"
    );
}
