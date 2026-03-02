#include "loraFsm.h"
#include <stdint.h>
#include "components.h"
#include <stdio.h>
#include <string.h>
#include <shared_lora.h>
#include <shared_state.h>
#include <helpers.h>

#include "components.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "loraImpl.h"
#include "portmacro.h"
#include "gps_data.h"
#include "sm.h"

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
static bool                  _rxProcessed = false;
static struct
{
    helpers_malloced_t mp;
    uint32_t           currentlyUsedSize;
} rxBuffer                             = {0};
static uint64_t lastSuccessfulPing_sec = 0;
static uint64_t timeSinceBeacon_sec    = 0;

static void     _loraFsm_onRxError()
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

#define LAST_SEQ_CACHE_SIZE 16
static uint16_t lastSeqCache_s[LAST_SEQ_CACHE_SIZE];
static int      lastSeqIdx_s = 0;

static bool     seenSeq(uint16_t seq)
{
    for (int i = 0; i < LAST_SEQ_CACHE_SIZE; i++)
    {
        if (lastSeqCache_s[i] == seq)
            return true;
    }
    return false;
}

static void recordSeq(uint16_t seq)
{
    lastSeqCache_s[lastSeqIdx_s++] = seq;
    if (lastSeqIdx_s >= LAST_SEQ_CACHE_SIZE)
        lastSeqIdx_s = 0;
}

// The payload will be freed after this is run, so memcpy everything.
static void _loraFsm_onRxDone(uint8_t* payload, uint16_t payloadSize,
                              int16_t rssi, int8_t snr)
{
    if (!helpers_smartAlloc(&rxBuffer.mp, payloadSize))
    {
        ESP_LOGE(__FUNCTION__,
                 "Failed to allocate buffer for rxDone!");
        return;
    }
    memcpy(rxBuffer.mp.buffer, payload, payloadSize);
    rxBuffer.currentlyUsedSize = payloadSize;
    ESP_LOGI(__FUNCTION__, "rx done! payload size: %" PRIu16,
             payloadSize);
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
    // Wait 2s until if we get a response.
    lora_setRx(0);
    uint64_t startTime_sec = esp_timer_get_time() / 1000000;
    ESP_LOGI(__FUNCTION__,
             "Sent ping, waiting for pong, start time: %" PRIu64,
             startTime_sec);
    uint16_t i = 0;
    while (_rxProcessed != true &&
           startTime_sec + 2 > (esp_timer_get_time() / 1000000))
    {
        if (i++ % 1000 == 0)
            ESP_LOGI(__FUNCTION__,
                     "Still waiting, current time is: %" PRIu64,
                     (esp_timer_get_time() / 1000000));
        lora_irqProcess();
        taskYIELD();
    }
    if (_rxProcessed)
    {
        _rxProcessed = false;

        loraFsm_packetWrapper_t packet = loraFsm_packetParse(
            rxBuffer.mp.buffer, rxBuffer.currentlyUsedSize);

        if (!packet.wellFormed)
        {
            ESP_LOGE(__FUNCTION__, "not well formed packet");
            return false;
        }

        if (packet.packetInterpreter->type == loraFsm_packetType_pong)
        {
            ESP_LOGI(__FUNCTION__, "Valid pong received");
            loraFsm_packetFree(&packet);
            return true;
        }

        ESP_LOGW(__FUNCTION__, "Received packet, but not pong: %s",
                 loraFsm_packetTypeToString(
                     packet.packetInterpreter->type));

        loraFsm_packetFree(&packet);
        return true;
    }
    ESP_LOGW(__FUNCTION__, "Unsuccessful ping pong.");
    return false;
}

static void _loraFsm_broadcast_sendGPS()
{
    gps_data_t gpsData = {0};
    gps_stateGetSnapshot(&gpsData);

    loraFsm_packetWrapper_t gpsStatePacket =
        loraFsm_packetCreate(loraFsm_packetType_gpsData,
                             (uint8_t*)&gpsData, sizeof(gpsData));

    loraFsm_packetSend(&gpsStatePacket);
    loraFsm_packetFree(&gpsStatePacket);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // maybe we only send this one in the actual launch day
    gps_psatTelemetryPacket_t gpsData2;
    gps_telemtryPacketGetSnapshot(&gpsData2);

    loraFsm_packetWrapper_t gpsStatePacket2 =
        loraFsm_packetCreate(loraFsm_packetType_telemetryData,
                             (uint8_t*)&gpsData2, sizeof(gpsData2));

    loraFsm_packetSend(&gpsStatePacket2);
    loraFsm_packetFree(&gpsStatePacket2);
}

static void _loraFsm_broadcast_sendTelemetryData()
{
    gps_psatTelemetryPacket_t telemetryData = {0};
    gps_telemtryPacketGetSnapshot(&telemetryData);

    loraFsm_packetWrapper_t telemetryDataPacket =
        loraFsm_packetCreate(loraFsm_packetType_telemetryData,
                             (uint8_t*)&telemetryData,
                             sizeof(telemetryData));

    loraFsm_packetSend(&telemetryDataPacket);
    loraFsm_packetFree(&telemetryDataPacket);
}

static void _loraFsm_broadcast_sendState()
{
    psatGlobal_state_t psatState = {
        .currentFSMState = psatFSM_getCurrentState(),
        .prevFSMState    = psatFSM_getPreviousState()};

    loraFsm_packetWrapper_t psatStatePacket =
        loraFsm_packetCreate(loraFsm_packetType_stateData,
                             (uint8_t*)&psatState, sizeof(psatState));

    loraFsm_packetSend(&psatStatePacket);
    loraFsm_packetFree(&psatStatePacket);
}

static void _loraFsm_broadcast_sendComponents()
{
    psatFSM_componentConfig_t psatConfig;
    psatFSM_getComponentConfig(&psatConfig);

    loraFsm_packetWrapper_t psatConfigPacket = loraFsm_packetCreate(
        loraFsm_packetType_componentData, (uint8_t*)&psatConfig,
        sizeof(psatConfig));

    loraFsm_packetSend(&psatConfigPacket);
    loraFsm_packetFree(&psatConfigPacket);
}

static void _loraFsm_broadcast_sendSensors() {}

static void _loraFsm_broadcast()
{
    ESP_LOGI(__FUNCTION__, "Broadcasting state information!");

    _loraFsm_broadcast_sendState();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _loraFsm_broadcast_sendGPS();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _loraFsm_broadcast_sendTelemetryData();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _loraFsm_broadcast_sendComponents();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    _loraFsm_broadcast_sendSensors();
}

static void _loraFsm_runStateIdle()
{
    static uint64_t lastTxRoutineTime = 0;
    lora_setRx(0);
    if (_rxProcessed)
    {
        ESP_LOGI(__FUNCTION__,
                 "Rx Received! Setting state to execute cmd");
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
    if (currentTime >
        lastSuccessfulPing_sec + loraFsm_connTimeoutThreshold_s_d)
    {
        if (_loraFsm_attemptPing())
        {
            lastSuccessfulPing_sec = currentTime;
            return;
        }
        // We failed and thus we become a beacon.
        timeSinceBeacon_sec     = esp_timer_get_time() / 1000000;
        _loraFsm_currentState_s = loraFsm_radioStates_beacon;
        return;
    }
}
static void _loraFsm_runStateCmd()
{
    ESP_LOGE(__FUNCTION__, "Received command, parsing!");
    // Figure out what the command is.
    loraFsm_packetWrapper_t packet = loraFsm_packetParse(
        rxBuffer.mp.buffer, rxBuffer.currentlyUsedSize);
    if (!packet.wellFormed)
    {
        ESP_LOGE(__FUNCTION__,
                 "Unable to run state cmd, packet parsing failed!");
        _loraFsm_currentState_s = loraFsm_radioStates_idle;
        return;
    }

    if (rxBuffer.currentlyUsedSize < 3)
    {
        ESP_LOGE(__FUNCTION__, "Packet missing sequence number");
        return;
    }

    uint16_t seq = (uint16_t)(packet.packetInterpreter->data)[0] |
        ((uint16_t)(packet.packetInterpreter->data)[1] << 8);

    ESP_LOGI(
        __FUNCTION__,
        "Recieved Request: %s, Sequence Number: %hu, sending ack",
        loraFsm_packetTypeToString(packet.packetInterpreter->type),
        seq);

    loraFsm_packetWrapper_t ackPacket = loraFsm_packetCreate(
        loraFsm_packetType_ack, (uint8_t*)&seq, sizeof(seq));
    vTaskDelay(50 / portTICK_PERIOD_MS);
    loraFsm_packetSend(&ackPacket);
    loraFsm_packetFree(&ackPacket);

    if (seenSeq(seq))
    { // we already recieved this command and proceesed it, so we wont run it again
        loraFsm_packetFree(&packet);
        _loraFsm_currentState_s = loraFsm_radioStates_idle;
        return;
    }

    recordSeq(seq);

    switch (packet.packetInterpreter->type)
    {
        // Buzzer Requests
        case loraFsm_packetType_buzzLongReq:
        {
            psatFSM_event_t event = {
                .global = true,
                .type   = psatFSM_eventType_audioBeep,
                .arg    = true,
            };
            psatFSM_postEvent(&event);
            break;
        }
        case loraFsm_packetType_buzzShortReq:
        {
            psatFSM_event_t event = {
                .global = true,
                .type   = psatFSM_eventType_audioBeep,
                .arg    = false,
            };
            psatFSM_postEvent(&event);
            break;
        }

        // Prelaunch Stuff
        case loraFsm_packetType_prelaunchCompleteReq:
        {
            psatFSM_event_t event = {
                .global = false,
                .type   = psatFSM_eventType_prelaunchComplete,
            };

            psatFSM_postEvent(&event);
            break;
        }
        case loraFsm_packetType_preflightReq:
        {
            uint16_t prelaunchOutput = psatFSM_preflightTest(false);
            ESP_LOGI("PREFLIGHT", "PREFLIGHT TEST RESULTS: %hx",
                     prelaunchOutput);

            loraFsm_packetWrapper_t preflightStatePacket =
                loraFsm_packetCreate(loraFsm_packetType_preflightData,
                                     (uint8_t*)&prelaunchOutput,
                                     sizeof(prelaunchOutput));

            loraFsm_packetSend(&preflightStatePacket);
            loraFsm_packetFree(&preflightStatePacket);
            break;
        }
        case loraFsm_packetType_preflightDataReq:
        {
            uint16_t prelaunchOutput = psatFSM_preflightTest(true);

            loraFsm_packetWrapper_t preflightStatePacket =
                loraFsm_packetCreate(loraFsm_packetType_preflightData,
                                     (uint8_t*)&prelaunchOutput,
                                     sizeof(prelaunchOutput));

            loraFsm_packetSend(&preflightStatePacket);
            loraFsm_packetFree(&preflightStatePacket);
            break;
        }

        // state overriding and forwarding
        case loraFsm_packetType_fastForwardReq:
        {
            psatFSM_state_e targetState = (psatFSM_state_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_stateFastForward(targetState);
            break;
        }
        case loraFsm_packetType_stateOverrideReq:
        {
            psatFSM_state_e targetState = (psatFSM_state_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_stateOverride(targetState);
            break;
        }

        // component stuff
        case loraFsm_packetType_enableComponentReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_enableComponent(targetComponent);
            break;
        }
        case loraFsm_packetType_disableComponentReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_disableComponent(targetComponent);
            break;
        }

        case loraFsm_packetType_startComponentTaskReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_startComponentTask(targetComponent);
            break;
        }
        case loraFsm_packetType_stopComponentTaskReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_stopComponentTask(targetComponent);
            break;
        }

        case loraFsm_packetType_markComponentEnabledReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_markComponentEnabled(targetComponent, true);
            break;
        }

        case loraFsm_packetType_markComponentDisabledReq:
        {
            psatFSM_component_e targetComponent =
                (psatFSM_component_e) *
                (packet.packetInterpreter->data + 4);
            psatFSM_markComponentEnabled(targetComponent, false);
            break;
        }

        default: ESP_LOGE(__FUNCTION__, "Invalid request!"); break;
    }

    loraFsm_packetFree(&packet);
    _loraFsm_currentState_s = loraFsm_radioStates_idle;
}

static void _loraFsm_runStateBeacon()
{
    // Using time since last beacon
    uint64_t currentTime_sec = esp_timer_get_time() / 1000000;
    if (timeSinceBeacon_sec + loraFsm_beaconRoutineInterval_s_d <
        currentTime_sec)
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
    lora_setRx(0);
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

void loraFsm_queryState() {}

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
