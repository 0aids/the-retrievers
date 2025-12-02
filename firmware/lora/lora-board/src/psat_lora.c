#include "psat_lora.h"
#include "global_radio.h"
// Only non-cross platform thing.
#include "delay.h"
#include "timer.h"
#include <stdint.h>
#include <string.h>
#include "packets/packets.h"
#include <stdio.h>
#define RX_TIMEOUT_VALUE                            2500
#define RX_CONTINUOUS 0

#define d_psatRadioStatesXMacro \
    X(psatRadioStates_Idle) \
    X(psatRadioStates_ExecuteCMD) \
    X(psatRadioStates_Beacon) \
    X(psatRadioStates_TxRoutine) \

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

#define d_psatRadioProcessingStateXMacro \
    X(psatRadioProcessingState_Idle) \
    X(psatRadioProcessingState_Tx) \
    X(psatRadioProcessingState_TxDone) \
    X(psatRadioProcessingState_Rx) \
    X(psatRadioProcessingState_RxDone) \

#define X(name) name,

typedef enum  {
    d_psatRadioProcessingStateXMacro
} e_psatRadioProcessingState;

#undef X


#define X(name) case name: return #name;
const char* PsatRadioProcessingStateToString(e_psatRadioProcessingState state) {
    switch (state) {
        d_psatRadioProcessingStateXMacro
        default:                return "UNKNOWN_RADIO_STATE";
    }
}
#undef X

static e_psatRadioState g_psatRadioState = psatRadioStates_Idle;
static e_psatRadioState g_lastPsatRadioState = psatRadioStates_Idle;
static e_psatRadioProcessingState g_psatRadioProcessingState = psatRadioProcessingState_Idle;
static packet_t g_recvPacket;
static packet_t g_sendPacket;

void PsatOnTxDone( void )
{
    gr_RadioSetIdle( );
    g_psatRadioProcessingState = psatRadioProcessingState_TxDone;
}

void PsatOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // This will need to be changed later to forward all data via i2c, but for now
    // we will just parse it here.
    g_recvPacket = ParsePacket(payload, size + 1);
    printPacketStats(&g_recvPacket);
    g_psatRadioProcessingState = psatRadioProcessingState_RxDone;
}

void PsatOnTxTimeout( void )
{
    printf("PSAT tx timeout\r\n");
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

TimerTime_t g_TxRoutineTimer = 0;
volatile uint8_t g_TxRoutineCounter = 0;
volatile uint8_t g_numPingTries = 0;

// Define it in compilation time to override.
// Otherwise it defaults to 9999 in which it will basically won't go to the beacon mode.
#ifndef d_disconnectRetryThreshold
#define d_disconnectRetryThreshold 9999
#endif

void PsatRadioInit(void) {
    gr_RadioSetTxDoneCallback(PsatOnTxDone);
    gr_RadioSetRxDoneCallback(PsatOnRxDone);
    gr_RadioSetTxTimeoutCallback(PsatOnTxTimeout);
    gr_RadioSetRxTimeoutCallback(PsatOnRxTimeout);
    gr_RadioSetRxErrorCallback(PsatOnRxError);
    gr_RadioInit();
    printf("Starting up\r\n");
    g_TxRoutineTimer = TimerGetCurrentTime();
}

static inline void BlockingRadioSend(uint8_t* buffer, uint16_t bufferSize) {
    printf("Sending packet!\r\n");
    gr_RadioSend(buffer, bufferSize);
    // while (gr_RadioGetStatus() != gr_RadioStates_Idle)
    // {
        // Sleep until we finished transmitting.
    DelayMs(100);
    gr_RadioSetRx(0);
    // }
}

#define d_TxRoutineIntervalMilliseconds 5000

void PsatRadioMain() {
    g_lastPsatRadioState = g_psatRadioState;
    switch (g_psatRadioState) {
        case psatRadioStates_Idle:
            if (gr_RadioGetStatus() != gr_RadioStates_Rx)
            {
                gr_RadioSetRx(0);
            }
            else
                gr_RadioCheckRecv();
            if (g_psatRadioProcessingState == psatRadioProcessingState_RxDone)
            {
                g_psatRadioState = psatRadioStates_ExecuteCMD;
                printf("Received packet, state transition to %s\r\n",  PsatRadioStateToString(g_psatRadioState));
                break;
            }
            // If the time since the last routine is larger the d_TxRoutineIntervalMilliseconds
            // This method causes no negative numbers, as we are using unsigned ints.
            if (TimerGetCurrentTime() > d_TxRoutineIntervalMilliseconds + g_TxRoutineTimer)
            {
                printf("Changing to TxRoutine!\r\n");
                g_psatRadioState = psatRadioStates_TxRoutine;
                g_TxRoutineTimer = TimerGetCurrentTime();
            }
            break;

        case psatRadioStates_Beacon:
            // Attempt if we can ping stuff back
            break;
        case psatRadioStates_ExecuteCMD:
            // We received a command, so check the recv packet
            switch (g_recvPacket.type) {
                case PING:
                    printf("Received ping, sending pong!\r\n");
                    g_sendPacket = CreatePacket(PONG, NULL, 0);
                    BlockingRadioSend((uint8_t*)&g_sendPacket, 1);
                    break;
                        
                case BUZ_REQ:
                case GPS_REQ:
                case FOLD_REQ:
                    // TODO: Send relevant data if applicable (GPS data request)
                    printf("Received req, sending ack!\r\n");
                    g_sendPacket = CreatePacket(ACK, NULL, 0);
                    BlockingRadioSend((uint8_t*)&g_sendPacket, 1);
                    break;

                default:
                    printf("Received unknown or malformed request packet\r\n");
                    break;
            }
            g_psatRadioProcessingState = psatRadioProcessingState_Idle;
            g_psatRadioState = psatRadioStates_Idle;
            g_recvPacket.type = EMPTY;
            break;

        case psatRadioStates_TxRoutine:
            g_TxRoutineCounter++;
            // Just do the normal beacon stuff. Alternate between beaconing GPS data,
            // State data, or pings to home station.

            // Send GPS data
            if (g_TxRoutineCounter % 3 == 0)
            {
                printf("Sending GPS data\r\n");
                g_sendPacket = CreatePacket(GPS_DATA, NULL, 0);
                BlockingRadioSend((uint8_t*)&g_sendPacket, 1);
            }
            // Send State data
            else if (g_TxRoutineCounter % 3 == 1)
            {
                printf("Sending State data\r\n");
                g_sendPacket = CreatePacket(STATE_DATA, NULL, 0);
                BlockingRadioSend((uint8_t*)&g_sendPacket, 1);
            }

            // Send Ping request and wait for response for 2s
#ifdef d_runPsatConnectionTests
            else if (g_TxRoutineCounter % 3 == 2)
            {
                printf("Sending ping\r\n");
                g_sendPacket = CreatePacket(PING, NULL, 0);
                BlockingRadioSend((uint8_t*)&g_sendPacket, 1);
                gr_RadioSetRx(2000);
                DelayMs(2000);
                gr_RadioCheckRecv();
                if (g_psatRadioProcessingState == psatRadioProcessingState_RxDone)
                {
                    g_psatRadioProcessingState = psatRadioProcessingState_Idle;
                    g_numPingTries = 0;
                }
                else {
                    g_numPingTries++;
                    if (g_numPingTries > d_disconnectRetryThreshold) {
                        // Assume we have lost connection
                        g_psatRadioState = psatRadioStates_Beacon;
                        printf("Lost connection with GS - No pong received ;_;\r\n");
                    }
                }
            }
#endif

            // If we were in beacon mode we return to beacon mode
            if (g_lastPsatRadioState != psatRadioStates_Beacon)
            {
                g_psatRadioState = psatRadioStates_Idle;
            }
            else 
            {
                g_psatRadioState = psatRadioStates_Beacon;
            }

            break;
    }
}
