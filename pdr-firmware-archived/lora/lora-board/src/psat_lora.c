#include "psat_lora.h"

#include <stdint.h>

#include "global_radio.h"
#include "gps_state.h"

// Only non-cross platform thing.
#ifndef ESP_PLATFORM

#include "delay.h"
#include "timer.h"

#else  // on esp:

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo.h"
#include "sm.h"

#define DelayMs(timeInMs) vTaskDelay((timeInMs) / portTICK_PERIOD_MS)
#define TimerTime_t int64_t
#define TimerGetCurrentTime() (esp_timer_get_time() / 1000)

#endif
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "packets/packets.h"
#define RX_TIMEOUT_VALUE 2500
#define RX_CONTINUOUS 0

#define d_psatRadioStatesXMacro   \
    X(psatRadioStates_Idle)       \
    X(psatRadioStates_ExecuteCMD) \
    X(psatRadioStates_Beacon)     \
    X(psatRadioStates_TxRoutine)

#define X(name) name,

typedef enum { d_psatRadioStatesXMacro } e_psatRadioState;

#undef X

#define X(name) \
    case name:  \
        return #name;
const char* PsatRadioStateToString(e_psatRadioState state) {
    switch (state) {
        d_psatRadioStatesXMacro default : return "UNKNOWN_RADIO_STATE";
    }
}
#undef X

#define d_psatRadioProcessingStateXMacro \
    X(psatRadioProcessingState_Idle)     \
    X(psatRadioProcessingState_Tx)       \
    X(psatRadioProcessingState_TxDone)   \
    X(psatRadioProcessingState_Rx)       \
    X(psatRadioProcessingState_RxDone)

#define X(name) name,

typedef enum { d_psatRadioProcessingStateXMacro } e_psatRadioProcessingState;

#undef X

#define X(name) \
    case name:  \
        return #name;
const char* PsatRadioProcessingStateToString(e_psatRadioProcessingState state) {
    switch (state) {
        d_psatRadioProcessingStateXMacro default : return "UNKNOWN_RADIO_STATE";
    }
}
#undef X

void PrintGpsState(const gps_state_t* gps) {
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
    printf("  Fix Quality:     %ld\r\n", gps->fix_quality);
    printf("  Sats Tracked:    %ld\r\n", gps->satellites_tracked);
    printf("  Sats in View:    %ld\r\n", gps->sats_in_view);

    printf("\r\nTimestamp:\r\n");
    // BUG: Buggy printf implementation means multiple format specifiers causes
    // a crash! printf("  Date:            %02d-%02d-%04d\n", gps->day,
    // gps->month, gps->year); printf("  Time:            %02d:%02d:%02d\n",
    // gps->hours, gps->minutes, gps->seconds);
    printf("  Date:            %02ld", gps->day);
    printf("-%02ld", gps->month);
    printf("-%04ld\r\n", gps->year);
    printf("  Time:            %02ld:", gps->hours);
    printf("%02ld:", gps->minutes);
    printf("%02ld\r\n", gps->seconds);

    printf("\r\nValidity Flags:\r\n");
    printf("  Position Valid:  %s\r\n", gps->position_valid ? "true" : "false");
    printf("  Nav Valid:       %s\r\n", gps->nav_valid ? "true" : "false");
    printf("  Fix Info Valid:  %s\r\n", gps->fix_info_valid ? "true" : "false");
    printf("  Altitude Valid:  %s\r\n", gps->altitude_valid ? "true" : "false");

    printf("===================\r\n");
}

static e_psatRadioState g_psatRadioState = psatRadioStates_Idle;
static e_psatRadioState g_lastPsatRadioState = psatRadioStates_Idle;
static e_psatRadioProcessingState g_psatRadioProcessingState =
    psatRadioProcessingState_Idle;
static packet_t g_recvPacket;
static packet_t g_sendPacket;

static gps_state_t g_fakeGpsData = {
    .latitude = -36.8485,  // Auckland-ish
    .longitude = 174.7633,
    .speed_knots = 12.5,
    .speed_kph = 23.1,
    .course_deg = 87.0,  // Heading east
    .hdop = 0.9,
    .altitude = 150.0,  // meters
    .geoidal_sep = -34.2,

    .day = 5,
    .month = 12,
    .year = 2025,
    .hours = 14,
    .minutes = 37,
    .seconds = 22,

    .fix_quality = 1,  // Standard GPS fix
    .satellites_tracked = 10,
    .sats_in_view = 14,

    .position_valid = true,
    .nav_valid = true,
    .fix_info_valid = true,
    .altitude_valid = true,
};

void PsatOnTxDone(void) {
    gr_RadioSetIdle();
    g_psatRadioProcessingState = psatRadioProcessingState_TxDone;
    printf("Sent!\r\n");
}

void PsatOnRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr) {
    // This will need to be changed later to forward all data via i2c, but for
    // now we will just parse it here.
    g_recvPacket = ParsePacket(payload, size + 1);
    printPacketStats(&g_recvPacket);
    g_psatRadioProcessingState = psatRadioProcessingState_RxDone;
}

void PsatOnTxTimeout(void) {
    printf("PSAT tx timeout\r\n");
    // RadioSleep( );
}

void PsatOnRxTimeout(void) {
    printf("PSAT rx timeout\r\n");
    // RadioSleep( );
}

void PsatOnRxError(void) {
    printf("PSAT rx ERROR\r\n");
    // RadioSleep( );
}

TimerTime_t g_TxRoutineTimer = 0;
volatile uint8_t g_TxRoutineCounter = 0;
volatile uint8_t g_numPingTries = 0;

// Define it in compilation time to override.
// Otherwise it defaults to 9999 in which it will basically won't go to the
// beacon mode.
#ifndef d_disconnectRetryThreshold
#define d_disconnectRetryThreshold 9999
#endif
static gps_state_t s_curGpsState;

void PsatRadioInit() {
    gr_RadioSetTxDoneCallback(PsatOnTxDone);
    gr_RadioSetRxDoneCallback(PsatOnRxDone);
    gr_RadioSetTxTimeoutCallback(PsatOnTxTimeout);
    gr_RadioSetRxTimeoutCallback(PsatOnRxTimeout);
    gr_RadioSetRxErrorCallback(PsatOnRxError);
    gr_RadioInit();
    printf("Starting up\r\n");
    g_TxRoutineTimer = TimerGetCurrentTime();
}

#define d_TxRoutineIntervalMilliseconds 5000

void PsatRadioMain() {
    g_lastPsatRadioState = g_psatRadioState;
    switch (g_psatRadioState) {
        case psatRadioStates_Idle:
            if (gr_RadioGetStatus() != gr_RadioStates_Rx) {
                gr_RadioSetRx(0);
            } else
                gr_RadioCheckRecv();

            if (g_psatRadioProcessingState == psatRadioProcessingState_RxDone) {
                g_psatRadioState = psatRadioStates_ExecuteCMD;
                printf("Received packet, state transition to %s\r\n",
                       PsatRadioStateToString(g_psatRadioState));
                break;
            }
            // If the time since the last routine is larger the
            // d_TxRoutineIntervalMilliseconds This method causes no negative
            // numbers, as we are using unsigned ints.
            if (TimerGetCurrentTime() >
                d_TxRoutineIntervalMilliseconds + g_TxRoutineTimer) {
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
                    gr_RadioSend((uint8_t*)&g_sendPacket, 1);
                    break;

                case BUZ_REQ:
#ifdef ESP_PLATFORM
                    fsm_event_t audio_event = {.type = EVENT_AUDIO_BEEP};
                    fsm_post_event(&audio_event);
                    printf("Sending audio event\r\n");
#endif

                    printf("Received audio req, sending ack!\r\n");
                    g_sendPacket = CreatePacket(ACK, NULL, 0);
                    gr_RadioSend((uint8_t*)&g_sendPacket, 1);
                    break;
                case FOLD_REQ:
                    // TODO: Send relevant data if applicable (GPS data request)
                    printf("Received req, sending ack!\r\n");
                    g_sendPacket = CreatePacket(ACK, NULL, 0);
                    gr_RadioSend((uint8_t*)&g_sendPacket, 1);
                    break;
                case GPS_REQ:
                    printf("Sending GPS data!\r\n");
                    gps_get_snapshot(&s_curGpsState);

                    g_sendPacket =
                        CreatePacket(GPS_DATA, (uint8_t*)&s_curGpsState,
                                     sizeof(gps_state_t));
                    gr_RadioSend((uint8_t*)&g_sendPacket,
                                 g_sendPacket.m_dataSize + 1);
                    DelayMs(200);
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
            // Just do the normal beacon stuff. Alternate between beaconing GPS
            // data, State data, or pings to home station.

            // Send GPS data
            if (g_TxRoutineCounter % 3 == 0) {
                printf("Sending GPS data\r\n");
                gps_get_snapshot(&s_curGpsState);
                PrintGpsState(&s_curGpsState);
                g_sendPacket = CreatePacket(GPS_DATA, (uint8_t*)&s_curGpsState,
                                            sizeof(gps_state_t));

                gr_RadioSend((uint8_t*)&g_sendPacket,
                             g_sendPacket.m_dataSize + 1);
                DelayMs(100);
            }
            // Send State data
            else if (g_TxRoutineCounter % 3 == 1) {
#ifdef ESP_PLATFORM
                printf("Sending State data\r\n");
                g_sendPacket = CreatePacket(STATE_DATA, NULL, 0);
                gr_RadioSend((uint8_t*)&g_sendPacket, 1);
                gps_get_snapshot(&s_curGpsState);

                servo_t servo_state = fsm_get_current_servo_state();
                uint16_t angle = servo_get_angle(&servo_state);

                PSAT_State_Data_t data = {
                    .gps_fix = s_curGpsState.fix_info_valid,
                    .servo_angle = angle,
                    .state = fsm_get_current_state()};

                
                printf("Current State: %s\n",
                       fsm_state_to_string(data.state));
                printf("GPS Fix Valid: %d\n", data.gps_fix);
                printf("Servo Angle: %d\n", data.servo_angle);

                g_sendPacket = CreatePacket(STATE_DATA, (uint8_t*)&data,
                                            sizeof(PSAT_State_Data_t));
                gr_RadioSend((uint8_t*)&g_sendPacket,
                             g_sendPacket.m_dataSize + 1);
#endif
                DelayMs(50);
            }
            // Send Ping request and wait for response for 2s
#if defined(d_runPsatConnectionTests) || defined(ESP_PLATFORM)
            else if (g_TxRoutineCounter % 3 == 2) {
                printf("Sending ping\r\n");
                g_sendPacket = CreatePacket(PING, NULL, 0);
                gr_RadioSend((uint8_t*)&g_sendPacket, 1);
                DelayMs(50);

                gr_RadioSetRx(2000);
                DelayMs(2000);
                gr_RadioCheckRecv();
                if (g_psatRadioProcessingState ==
                    psatRadioProcessingState_RxDone) {
                    g_psatRadioProcessingState = psatRadioProcessingState_Idle;
                    g_numPingTries = 0;
                } else {
                    g_numPingTries++;
                    // if (g_numPingTries > d_disconnectRetryThreshold) {
                    if (false) {
                        // Assume we have lost connection
                        g_psatRadioState = psatRadioStates_Beacon;
                        printf(
                            "Lost connection with GS - No pong received "
                            ";_;\r\n");
                    }
                }
            }
#endif

            // If we were in beacon mode we return to beacon mode
            if (g_lastPsatRadioState != psatRadioStates_Beacon) {
                g_psatRadioState = psatRadioStates_Idle;
            } else {
                g_psatRadioState = psatRadioStates_Beacon;
            }

            break;
    }
}
