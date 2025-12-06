#include "ground_lora.h"
#include <string.h>
#include "gps_state.h"
#include <stdio.h>
#include "delay.h"
#include "global_radio.h"
#include "packets/packets.h"
#include "global_lora.h"
#include "radio.h"
#include "tremo_cm4.h"
#include "tremo_delay.h"
#include "timer.h"
#include "tremo_uart.h"

static gps_state_t g_gpsBuffer;

void PrintGpsState(const gps_state_t *gps)
{
    if (!gps) {
        printf("GPS State: NULL pointer\r\n");
        return;
    }

    printf("=== GPS STATE ===\n\r");

    printf("Position:\n\r");
    printf("  Latitude:        %.6f\r\n", gps->latitude);
    printf("  Longitude:       %.6f\r\n", gps->longitude);
    printf("  Altitude:        %.2f m\r\n", gps->altitude);
    printf("  Geoidal Sep:     %.2f m\r\n", gps->geoidal_sep);

    printf("\r\nSpeed & Course:\r\n");
    printf("  Speed (knots):   %.2f\r\n", gps->speed_knots);
    printf("  Speed (kph):     %.2f\r\n", gps->speed_kph);
    printf("  Course:          %.2f deg\r\n", gps->course_deg);

    printf("\r\nFix / Satellite Info:\r\n");

    printf("  HDOP:            %.2f\r\n", gps->hdop);
    printf("  Fix Quality:     %d\r\n", gps->fix_quality);
    printf("  Sats Tracked:    %d\r\n", gps->satellites_tracked);
    printf("  Sats in View:    %d\r\n", gps->sats_in_view);

    printf("\r\nTimestamp:\r\n");
    // BUG: Buggy printf implementation means multiple format specifiers causes a crash!
    // printf("  Date:            %02d-%02d-%04d\n", gps->day, gps->month, gps->year);
    // printf("  Time:            %02d:%02d:%02d\n", gps->hours, gps->minutes, gps->seconds);
    printf("  Date:            %02d", gps->day);
    printf("-%02d", gps->month);
    printf("-%04d\r\n", gps->year);
    printf("  Time:            %02d:", gps->hours);
    printf("%02d:", gps->minutes);
    printf("%02d\r\n", gps->seconds);

    printf("\r\nValidity Flags:\r\n");
    printf("  Position Valid:  %s\r\n", gps->position_valid ? "true" : "false");
    printf("  Nav Valid:       %s\r\n", gps->nav_valid ? "true" : "false");
    printf("  Fix Info Valid:  %s\r\n", gps->fix_info_valid ? "true" : "false");
    printf("  Altitude Valid:  %s\r\n", gps->altitude_valid ? "true" : "false");

    printf("===================\r\n");
}


#define RX_TIMEOUT 0 //ms

static packet_t g_packetRecv = {EMPTY};
static packet_t g_packetSend = {EMPTY};
static PSAT_State_Data_t g_packetStateData = {0};

TimerTime_t g_timer = 0;

void GroundOnTxDone( void )
{
    printf("tx done\r\n");
    gr_RadioSetRx(RX_TIMEOUT);
    // gr_RadioSleep( );
}

void PrintState(PSAT_State_Data_t *g_packetStateData);
void GroundOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // gr_RadioSleep( );
    g_packetRecv = ParsePacket(payload, size);
    printPacketStats(&g_packetRecv);
    delay_ms(50);
    switch (g_packetRecv.type) {
        case PING:
            d_pingResponse();
            break;
        case GPS_DATA:
            printf("Success, received GPS_DATA!\r\n");
            printf("Payload size: %u\r\n", size);
            PrintGpsState((gps_state_t*)&g_packetRecv.data);
            break;
        case STATE_DATA:
            printf("Success, received STATE_DATA!\r\n");
            printf("Payload size: %u\r\n", size);
            memcpy(&g_packetStateData, &g_packetRecv.data, g_packetRecv.m_dataSize);
            PrintState(&g_packetStateData);
        default:
            break;
    }
}

void PrintState(PSAT_State_Data_t *g_packetStateData) {
    printf("Current State: %s", fsm_state_to_string(g_packetStateData->state));
    printf("GPS Fix Valid: %d", g_packetStateData->gps_fix);
    printf("Servo Angle: %d", g_packetStateData->servo_angle);
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
    // As a test we'll ping on startup and then listen for the beacon details later afterwards.
    printf("Starting up\r\n");


    printf("Initial communication test complete\r\n");
    printf("Falling back to receive only mode (Pingpong only)\r\n");
    gr_RadioSetRx(0);
}

#define d_uartInputBufferSize 256
char g_uartInputBuffer[d_uartInputBufferSize] = {0};

#define d_possibleInstXMacro \
    X(gpsReq) \
    X(foldReq) \
    X(buzReq) \
    X(ping) \

#define X(name) possibleInst_##name,
typedef enum {
    d_possibleInstXMacro
} e_possibleInst;

#undef X

#define X(name) #name,

const char * const g_possibleInst[] = {
    d_possibleInstXMacro
};

#undef X

uint8_t g_groundRadioMainIters = 0;
void GroundRadioMain() {
    g_groundRadioMainIters++;
    e_radioState radioState = gr_RadioGetStatus();
    if (g_groundRadioMainIters % 50 == 0)
        printf("State: %s\r\n", gr_RadioStatesToString(radioState));
    gr_RadioCheckRecv();
    // Check for any instructions from uart

    if (uart_get_flag_status(CONFIG_DEBUG_UART, UART_FLAG_RX_FIFO_EMPTY) == RESET)
    {
        // Clear the buffer
        uint16_t i = 0;
        while(uart_get_flag_status(CONFIG_DEBUG_UART, UART_FLAG_RX_FIFO_EMPTY) == RESET &&
              i < d_uartInputBufferSize - 1)
        {
            g_uartInputBuffer[i++] = uart_receive_data(CONFIG_DEBUG_UART);
            // DelayMs(10);
        }
        g_uartInputBuffer[i] = 0;
        printf("Received uart input: \"%s\"\r\n", g_uartInputBuffer);

        if (strcmp(g_uartInputBuffer, g_possibleInst[possibleInst_buzReq]) == 0) 
        {
            printf("Requesting buzzer!\r\n");
            g_packetSend = CreatePacket(BUZ_REQ, NULL, 0);
            gr_RadioSend((uint8_t*)&g_packetSend, 1);
            // This is required otherwise we will get a tx timeout.
            DelayMs(100) ;

            printf("Waiting 2s for response!\r\n");
            g_timer = TimerGetCurrentTime();
            while (TimerGetCurrentTime() < g_timer + 2000)
            {
                gr_RadioSetRx(0);
                gr_RadioCheckRecv();
                DelayMs(10);
                if (g_packetRecv.type == ACK) {
                    printf("Success, received ack!\r\n");
                    goto buz_req_success;
                }
            }
            printf("Fail! ACK not received \r\n");
        buz_req_success:
            ;
        }
        else if (strcmp(g_uartInputBuffer, g_possibleInst[possibleInst_foldReq]) == 0) 
        {
            printf("Requesting fold!\r\n");
            g_packetSend = CreatePacket(FOLD_REQ, NULL, 0);
            gr_RadioSend((uint8_t*)&g_packetSend, 1);
            // This is required otherwise we will get a tx timeout.
            DelayMs(100) ;

            printf("Waiting 2s for response!\r\n");
            g_timer = TimerGetCurrentTime();
            while (TimerGetCurrentTime() < g_timer + 2000)
            {
                gr_RadioSetRx(0);
                gr_RadioCheckRecv();
                DelayMs(10);
                if (g_packetRecv.type == ACK) {
                    printf("Success, received ACK!\r\n");
                    goto fold_req_success;
                }
            }
            printf("Fail! ACK not received \r\n");
        fold_req_success:
            ;
        }
        else if (strcmp(g_uartInputBuffer, g_possibleInst[possibleInst_ping]) == 0) 
        {
            printf("Pinging!\r\n");
            g_packetSend = CreatePacket(PING, NULL, 0);
            gr_RadioSend((uint8_t*)&g_packetSend, 1);
            // This is required otherwise we will get a tx timeout.
            DelayMs(100) ;

            printf("Waiting 2s for response!\r\n");
            g_timer = TimerGetCurrentTime();
            while (TimerGetCurrentTime() < g_timer + 2000)
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
            ;
        }
        else if (strcmp(g_uartInputBuffer, g_possibleInst[possibleInst_gpsReq]) == 0) 
        {
            printf("Requesting GPS!\r\n");
            g_packetRecv.type = EMPTY;
            g_packetSend = CreatePacket(GPS_REQ, NULL, 0);
            gr_RadioSend((uint8_t*)&g_packetSend, g_packetSend.m_dataSize + 1);
            // This is required otherwise we will get a tx timeout.
            DelayMs(100);

            printf("Waiting 2s for response!\r\n");
            g_timer = TimerGetCurrentTime();
            while (TimerGetCurrentTime() < g_timer + 2000)
            {
                gr_RadioSetRx(0);
                gr_RadioCheckRecv();
                DelayMs(10);
                if (g_packetRecv.type == GPS_DATA) {
                    printf("Success, received GPS_DATA!\r\n");
                    memcpy(&g_gpsBuffer, g_packetRecv.data, g_packetRecv.m_dataSize);
                    PrintGpsState(&g_gpsBuffer);
                    goto gps_data_success;
                }
            }
            printf("FAIL, no gps data was received\r\n");
        gps_data_success:
            // And then we need to ensure we also get the data.
            ;
        }
    }
}
