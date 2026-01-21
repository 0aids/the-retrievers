#include "loraFsm.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <shared_lora.h>
#include <shared_state.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"

#define DelayMs(timeInMs)     vTaskDelay((timeInMs) / portTICK_PERIOD_MS)
#define TimerTime_t           int64_t
#define TimerGetCurrentTime() (esp_timer_get_time() / 1000)

#include "packets.h"
#define loraFsm_radioStates_xmacro                                   \
    X(loraFsm_radioStates_idle)                                      \
    X(loraFsm_radioStates_executeCmd)                                \
    X(loraFsm_radioStates_beacon)                                    \
    X(loraFsm_radioStates_txRoutine)

typedef enum
{
#define X(name) name,
    loraFsm_radioStates_xmacro
#undef X
} loraFsm_radioStates_e;

const char* loraFsm_radioStates_toString(loraFsm_radioStates_e state)
{
#define X(name)                                                      \
    case name: return #name;
    switch (state)
    {
        loraFsm_radioStates_xmacro default
            : return "UNKNOWN_RADIO_STATE";
    }
#undef X
}

static void _loraFsm_onRxError();
static void _loraFsm_onRxTimeout();
static void _loraFsm_onTxTimeout();
static void _loraFsm_onRxDone(uint8_t* payload, uint16_t payloadSize,
                              int16_t rssi, int8_t snr);
static void _loraFsm_onTxDone();

static bool _loraFsm_attemptPing();
static void _loraFsm_broadcast();

static void _loraFsm_runStateIdle();
static void _loraFsm_runStateCmd();
static void _loraFsm_runStateBeacon();
static void _loraFsm_runStateTxRoutine();

static loraFsm_radioStates_e _loraFsm_currentState_s;
static bool                  _rxProcessed           = false;
static uint8_t               rxBuffer[16384]        = {0};
static uint16_t              rxBufferSize           = 0;
static uint64_t              lastSuccessfulPing_sec = 0;
static uint64_t timeSinceBeacon_sec = 0;

static void                  _loraFsm_onRxError()
{
    ESP_LOGI(__FUNCTION__, "onRxError");
}
static void _loraFsm_onRxTimeout()
{
    ESP_LOGI(__FUNCTION__, "onRxTimeout");
}
static void _loraFsm_onTxTimeout()
{
    ESP_LOGI(__FUNCTION__, "onTxTimeout");
}
static void _loraFsm_onRxDone(uint8_t* payload, uint16_t payloadSize,
                              int16_t rssi, int8_t snr)
{
    // Wait for something to do.
    memcpy(rxBuffer, payload, payloadSize);
    ESP_LOGI(__FUNCTION__, "onRxDone");
    rxBufferSize = payloadSize;
    _rxProcessed = true;
}
static void _loraFsm_onTxDone()
{
    ESP_LOGI(__FUNCTION__, "onTxDone");
}

static bool _loraFsm_attemptPing()
{
    loraFsm_packetType_e ping = loraFsm_packetType_ping;
    lora_send((uint8_t*)&ping, sizeof(ping));
    ESP_LOGI(__FUNCTION__, "Sent ping, waiting for pong");
    // Wait 2s until if we get a response.
    lora_setRx(0);
    uint64_t startTime_sec = esp_timer_get_time() / 1000000;
    while (_rxProcessed != true || startTime_sec + 2 > (esp_timer_get_time() / 1000000))
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    if (_rxProcessed)
    {
        ESP_LOGI(__FUNCTION__, "Received pong, successful ping pong");
        _rxProcessed = false;
        return true;
    }
    ESP_LOGW(__FUNCTION__, "Unsuccessful ping pong.");
    return false;
}

static void _loraFsm_broadcast()
{
    // Get state and then transmit it.
    // TODO: create function that returns more than just our current PSAT state.
    psatFSM_state_e psatState = psatFSM_getCurrentState();
    gps_data_t      gpsData   = {0};
    // Might not fill out the data.
    gps_stateGetSnapshot(&gpsData);
    // Transmit the state data
    loraFsm_packet_t psatStatePacket = loraFsm_createPacket(
        loraFsm_packetType_stateData, (uint8_t*)&psatState, sizeof(psatState));

    lora_send((uint8_t*)&psatStatePacket, sizeof(psatState) + 1);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    loraFsm_packet_t gpsStatePacket = loraFsm_createPacket(
        loraFsm_packetType_stateData, (uint8_t*)&gpsData, sizeof(gpsData));

    lora_send((uint8_t*)&gpsStatePacket, sizeof(gpsData) + 1);
}

static void _loraFsm_runStateIdle()
{
    static uint64_t lastTxRoutineTime = 0;
    lora_setRx(0);
    if (_rxProcessed)
    {
        _rxProcessed            = false;
        _loraFsm_currentState_s = loraFsm_radioStates_executeCmd;
        return;
    }
    uint64_t currentTime = esp_timer_get_time() / 1000000;
    if (currentTime >
        lastTxRoutineTime + loraFsm_txRoutineInterval_s_d)
    {
        lastTxRoutineTime       = currentTime;
        _loraFsm_currentState_s = loraFsm_radioStates_txRoutine;
        return;
    }
    if (currentTime > lastSuccessfulPing_sec + loraFsm_connTimeoutThreshold_s_d)
    {
        if (_loraFsm_attemptPing())
        {
            lastSuccessfulPing_sec = currentTime;
            return;
        }
        // We failed and thus we become a beacon.
        timeSinceBeacon_sec = esp_timer_get_time() / 1000000;
        _loraFsm_currentState_s = loraFsm_radioStates_beacon;
        return;
    }
}
static void _loraFsm_runStateCmd()
{
    // Figure out what the command is.
    loraFsm_packet_t packet =
        loraFsm_packetParse(rxBuffer, rxBufferSize);

    switch (packet.type)
    {
        case loraFsm_packetType_buzReq:
        {
            ESP_LOGE(__FUNCTION__, "Buzzer Request Received!");
            psatFSM_event_t event = {
                .global = false,
                .type   = psatFSM_eventType_audioBeep,
            };
            psatFSM_postEvent(event);
            break;
        }

        case loraFsm_packetType_stateDumpReq:
            ESP_LOGE(__FUNCTION__, "Dump request received! noop");
            break;

        case loraFsm_packetType_fastForwardReq:
            ESP_LOGE(__FUNCTION__,
                     "Fast forward request received! noop");
            break;

        case loraFsm_packetType_stateOverrideReq:
            ESP_LOGE(__FUNCTION__, "Overriding state!");
            psatFSM_stateOverride(*(psatFsm_state_t*)(packet.data));
            break;

        default: ESP_LOGE(__FUNCTION__, "Invalid request!"); break;
    }

    _loraFsm_currentState_s = loraFsm_radioStates_idle;
}

static void _loraFsm_runStateBeacon() {
    // Using time since last beacon
    uint64_t currentTime_sec = esp_timer_get_time() / 1000000;
    if (timeSinceBeacon_sec + loraFsm_beaconRoutineInterval_s_d < currentTime_sec)
    {
        timeSinceBeacon_sec = currentTime_sec;
        _loraFsm_broadcast();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        // Attempt to ping or if we received anything.
        if (_loraFsm_attemptPing() || _rxProcessed)
        {
            // Successfuly means we go back to normal states.
            _loraFsm_currentState_s = loraFsm_radioStates_idle;
            return;
        }
    }
}

static void _loraFsm_runStateTxRoutine()
{
    _loraFsm_broadcast();
    _loraFsm_currentState_s = loraFsm_radioStates_idle;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    // Send a ping request.
    if (_loraFsm_attemptPing()) 
        lastSuccessfulPing_sec = esp_timer_get_time() / 1000000;
}

void loraFsm_init()
{
    lora_init();
    lora_setCallbacks(_loraFsm_onTxDone, _loraFsm_onRxDone,
                      _loraFsm_onTxTimeout, _loraFsm_onRxTimeout,
                      _loraFsm_onRxError);
}

void loraFsm_queryState() {
}

void loraFsm_start()
{
    while (1)
    {
        lora_irqProcess();
        switch (_loraFsm_currentState_s)
        {
            case loraFsm_radioStates_idle:
                _loraFsm_runStateIdle();
                break;
            case loraFsm_radioStates_beacon:
                _loraFsm_runStateBeacon();
                break;
            case loraFsm_radioStates_txRoutine:
                _loraFsm_runStateTxRoutine();
                break;
            case loraFsm_radioStates_executeCmd:
                _loraFsm_runStateCmd();
                break;

            default:
                ESP_LOGE(__FUNCTION__, "Not a valid state!");
                break;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void loraFsm_deinit()
{
    lora_deinit();
}
