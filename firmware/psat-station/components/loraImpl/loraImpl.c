#include "loraImpl.h"
#include "driver/spi_master.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "loraHal.h"
#include "loraRegs.h"
#include "loraImpl_cfgToRegs.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include "portmacro.h"

// Forward declarations
static void IRAM_ATTR lora_dio0IsrHandler(void*);
static void           _loraImpl_rxDoneTask(void*);
static void           _loraImpl_setupDefaultConfig();
static void           _loraImpl_printGlobalState();
static void           _loraImpl_printIrqState();

#define _loraImpl_runCallback_df(func)                               \
    if (func)                                                        \
    {                                                                \
        func();                                                      \
    }

// #undef ESP_LOGI
// #define ESP_LOGI(...)

// static const char* const __FUNCTION__ = "loraImpl";

loraImpl_globalState_t   loraImpl_globalState_g = {0};

TaskHandle_t             rxDoneTaskHandle     = {0};
static SemaphoreHandle_t rxDoneIsrSemaphore   = NULL;
static SemaphoreHandle_t rxProcessedSemaphore = NULL;

static struct
{
    atomic_bool rxDone;
    atomic_bool txDone;
    atomic_bool rxTimeout;
    atomic_bool txTimeout;
    atomic_bool rxError;
} loraImpl_irqState = {};

static uint8_t        rxBuffer[256] = {0};
static uint16_t       rxBufferSize  = 0;

static void IRAM_ATTR lora_dio0IsrHandler(void*)
{
    xSemaphoreGive(rxDoneIsrSemaphore);
}

static void _loraImpl_rxDoneTask(void*)
{
    while (1)
    {
        if (rxDoneIsrSemaphore == NULL)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        if (xSemaphoreTake(rxDoneIsrSemaphore, 10000) == pdTRUE)
        {
            ESP_LOGI(__FUNCTION__, "rxDoneTask triggered by ISR");
            // Check payload received correctly, if not set rxTimeout or rxError
            uint8_t flags;
            loraHal_readReg(loraReg_addrIrqFlags_d, &flags);
            uint8_t crcErr = flags & loraBit_irqFlags_crcErrMsk_d;
            uint8_t validHeader =
                flags & loraBit_irqFlags_validHeadMsk_d;
            uint8_t rxDone = flags & loraBit_irqFlags_rxDoneMsk_d;
            uint8_t rxTimeout =
                flags & loraBit_irqFlags_rxTimeoutMsk_d;
            uint8_t relevantFlags =
                crcErr | validHeader | rxDone | rxTimeout;
            ESP_LOGI(__FUNCTION__, "irq flags: %#x", relevantFlags);
            if (crcErr || !(validHeader && rxDone))
            {
                ESP_LOGE(__FUNCTION__, "Rx Error");
                // Rx Error
                atomic_store(&loraImpl_irqState.rxError, true);
                loraHal_writeReg(loraReg_addrIrqFlags_d,
                                 loraBit_irqFlags_clearAll_d, NULL);
                continue;
            }
            else if (rxTimeout)
            {
                ESP_LOGW(__FUNCTION__, "Rx Timeout");
                // Rx timeout
                atomic_store(&loraImpl_irqState.rxTimeout, true);
                loraHal_writeReg(loraReg_addrIrqFlags_d,
                                 loraBit_irqFlags_clearAll_d, NULL);
                continue;
            }
            loraHal_writeReg(loraReg_addrIrqFlags_d,
                             loraBit_irqFlags_clearAll_d, NULL);
            ESP_LOGI(__FUNCTION__, "Packet received successfully");
            // Read from the rxDone buffer.
            loraHal_readReg(loraReg_addrRxNbBytes_d,
                            (uint8_t*)&rxBufferSize);
            ESP_LOGI(__FUNCTION__, "Received packet of size: %d",
                     rxBufferSize);
            uint8_t currAddr;
            loraHal_readReg(loraReg_addrFifoRxCurAddr_d, &currAddr);
            loraHal_writeReg(loraReg_addrFifoAddrPtr_d, currAddr,
                             NULL);
            loraHal_readRegContinuous(loraReg_addrFifo_d, rxBuffer,
                                      rxBufferSize);
            atomic_store(&loraImpl_irqState.rxDone, true);
#ifdef psat_debugMode_d
            printf("hex: ");
            for (size_t i = 0; i < rxBufferSize; i++)
            {
                printf("%02x ", rxBuffer[i]);
            }
            printf("\r\n");
#endif
            xSemaphoreGive(rxProcessedSemaphore);
        }
    }
}

// The underlying lora implementation for sending.
void loraImpl_send(uint8_t* payload, uint16_t payloadSize)
{
    ESP_LOGI(__FUNCTION__, "Sending packet - size: %" PRIu16,
             payloadSize);
    // Change to stdby mode.
    uint8_t opMode;
    loraHal_readReg(loraReg_addrOpMode_d, &opMode);
    loraBit_set(opMode, loraBit_opMode_modeMsk_d,
                loraBit_opMode_mode_stdby_d);
    loraHal_writeReg(loraReg_addrOpMode_d, opMode, NULL);

    // Setting the fifo to point to the tx base.
    uint8_t txBase;
    loraHal_readReg(loraReg_addrFifoTxBaseAddr_d, &txBase);
    loraHal_writeReg(loraReg_addrFifoAddrPtr_d, txBase, NULL);
    // Set the number of bytes to be sent.
    loraHal_writeReg(loraReg_addrPayloadLen_d, (uint8_t)payloadSize,
                     NULL);

    // Then write the data to be sent.
    loraHal_writeRegContinuous(loraReg_addrFifo_d, payload,
                               (uint8_t)payloadSize);

    // Then set the op mode to be TX
    loraBit_set(opMode, loraBit_opMode_modeMsk_d,
                loraBit_opMode_mode_tx_d);
    loraHal_writeReg(loraReg_addrOpMode_d, opMode, NULL);
    atomic_store(&loraImpl_irqState.txDone, true);
}

void loraImpl_setCallbacks(void (*onTxDone)(void),
                           void (*onRxDone)(uint8_t* payload,
                                            uint16_t size,
                                            int16_t rssi, int8_t snr),
                           void (*onTxTimeout)(void),
                           void (*onRxTimeout)(void),
                           void (*onRxError)(void))
{
    loraImpl_globalState_g.onTxDone    = onTxDone;
    loraImpl_globalState_g.onRxDone    = onRxDone;
    loraImpl_globalState_g.onTxTimeout = onTxTimeout;
    loraImpl_globalState_g.onRxTimeout = onRxTimeout;
    loraImpl_globalState_g.onRxError   = onRxError;
}

static void _loraImpl_setupDefaultConfig()
{
    // Sleep and change into lora mode
    uint8_t opMode;
    loraHal_readReg(loraReg_addrOpMode_d, &opMode);
    loraBit_set(opMode, loraBit_opMode_modeMsk_d,
                loraBit_opMode_mode_sleep_d);
    loraHal_writeReg(loraReg_addrOpMode_d, opMode, NULL);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    loraBit_set(opMode,
                (loraBit_opMode_longRangeModeMsk_d |
                 loraBit_opMode_freqMsk_d),
                (loraBit_opMode_longRangeMode_lora_d |
                 loraBit_opMode_freq_high_d));
    loraHal_writeReg(loraReg_addrOpMode_d, opMode, NULL);

    // Write mdmcfg1
    uint8_t mdmcfg1 = 0;
    loraBit_set(mdmcfg1,
                (loraBit_mdmCfg1_bwMsk_d | loraBit_mdmCfg1_crMsk_d |
                 loraBit_mdmCfg1_implicitHeadMsk_d),
                (loraHalCfg_mdmCfg1_implicitHead_d |
                 loraHalCfg_mdmCfg1_bw_d | loraHalCfg_mdmCfg1_cr_d));

    loraHal_writeReg(loraReg_addrMdmCfg1_d, mdmcfg1, NULL);
    uint8_t readMdmcfg1;
    loraHal_readReg(loraReg_addrMdmCfg1_d, &readMdmcfg1);
    ESP_LOGI(__FUNCTION__, "mdmcfg1 - Expected: %#x, received: %#x",
             mdmcfg1, readMdmcfg1);
    if (mdmcfg1 != readMdmcfg1)
    {
        ESP_LOGE(__FUNCTION__,
                 "mdmcfg1 - Expected: %#x, received: %#x", mdmcfg1,
                 readMdmcfg1);
    }

    uint8_t mdmcfg2 = 0;
    loraBit_set(
        mdmcfg2,
        (loraBit_mdmCfg2_sfMsk_d | loraBit_mdmCfg2_rxCrcMsk_d |
         loraBit_mdmCfg2_txContMsk_d |
         loraBit_mdmCfg2_symbTimeMsbMsk_d),
        (loraHalCfg_mdmCfg2_rxCrc_d | loraHalCfg_mdmCfg2_sf_d |
         loraHalCfg_mdmCfg2_symbTimeoutMsb_d |
         loraBit_mdmCfg2_txCont_off_d));
    loraHal_writeReg(loraReg_addrMdmCfg2_d, mdmcfg2, NULL);
    uint8_t readMdmcfg2;
    loraHal_readReg(loraReg_addrMdmCfg2_d, &readMdmcfg2);
    ESP_LOGI(__FUNCTION__, "mdmcfg2 - Expected: %#x, received: %#x",
             mdmcfg2, readMdmcfg2);
    if (mdmcfg2 != readMdmcfg2)
    {
        ESP_LOGE(__FUNCTION__,
                 "mdmcfg2 - Expected: %#x, received: %#x", mdmcfg2,
                 readMdmcfg2);
    }

    uint8_t mdmcfg3 = 0;
    loraBit_set(mdmcfg3,
                (loraBit_mdmCfg3_agcAutoMsk_d |
                 loraBit_mdmCfg3_lowDataOptMsk_d),
                (loraBit_mdmCfg3_agcAuto_on_d |
                 loraHalCfg_mdmCfg3_lowDataOpt_d));
    loraHal_writeReg(loraReg_addrMdmCfg3_d, mdmcfg3, NULL);
    uint8_t readMdmcfg3;
    loraHal_readReg(loraReg_addrMdmCfg3_d, &readMdmcfg3);
    ESP_LOGI(__FUNCTION__, "mdmcfg3 - Expected: %#x, received: %#x",
             mdmcfg3, readMdmcfg3);
    if (mdmcfg3 != readMdmcfg3)
    {
        ESP_LOGE(__FUNCTION__,
                 "mdmcfg3 - Expected: %#x, received: %#x", mdmcfg3,
                 readMdmcfg3);
    }

    uint8_t lna = 0;
    loraBit_set(lna,
                (loraBit_lna_gainMsk_d | loraBit_lna_boostHfMsk_d),
                (loraBit_lna_boostHf_on_d | loraBit_lna_gain_g1_d));
    loraHal_writeReg(loraReg_addrLna_d, lna, NULL);

    uint8_t pacfg = 0;
    loraBit_set(pacfg,
                (loraBit_paCfg_maxPowerMsk_d |
                 loraBit_paCfg_paSelectMsk_d |
                 loraBit_paCfg_outputPowerMsk_d),
                (loraHalCfg_paCfg_paSelect_d |
                 loraHalCfg_paCfg_outputPower_d));
    loraHal_writeReg(loraReg_addrPaCfg_d, pacfg, NULL);
    uint8_t readPacfg3;
    loraHal_readReg(loraReg_addrPaCfg_d, &readPacfg3);
    ESP_LOGI(__FUNCTION__, "pacfg - Expected: %#x, received: %#x",
             pacfg, readMdmcfg3);
    if (pacfg != readPacfg3)
    {
        ESP_LOGE(__FUNCTION__, "pacfg - Expected: %#x, received: %#x",
                 pacfg, readPacfg3);
    }

    uint8_t frequency[3] = {0};
    frequency[0]         = (loraHalCfg_frf_val_d >> 16) & (0xff);
    frequency[1]         = (loraHalCfg_frf_val_d >> 8) & (0xff);
    frequency[2]         = (loraHalCfg_frf_val_d >> 0) & (0xff);
    loraHal_writeRegContinuous(loraReg_addrFrfMsb_d, frequency, 3);

    uint8_t preambleLength[2] = {0};
    preambleLength[0]         = loraHalCfg_preamble_val_d >> 8;
    preambleLength[1]         = loraHalCfg_preamble_val_d & 0xff;
    loraHal_writeRegContinuous(loraReg_addrPreambleMsb_d,
                               (uint8_t*)&preambleLength, 2);

    uint8_t symbolTimeout = loraHalCfg_symbTimeout_lsb_d;
    loraHal_writeReg(loraReg_addrSymbTimeLsb_d, symbolTimeout, NULL);
}

void loraImpl_init(void)
{
    esp_err_t err;
    uint32_t  stackSize = 1 << 15;
    err                 = loraHal_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,
                 "loraHal_init failed with error code: %d", err);
        return;
    }
    _loraImpl_setupDefaultConfig();
    err = loraHal_registerRxDoneIsr(lora_dio0IsrHandler);
    if (err != ESP_OK)
    {
        ESP_LOGE(
            __FUNCTION__,
            "loraHal_registerRxDoneIsr failed with error code: %d",
            err);
        loraHal_deinit();
        return;
    }
    rxDoneIsrSemaphore   = xSemaphoreCreateBinary();
    rxProcessedSemaphore = xSemaphoreCreateBinary();
    xTaskCreate(_loraImpl_rxDoneTask, "rxDoneTask", stackSize, NULL,
                configMAX_PRIORITIES / 2, &rxDoneTaskHandle);
}

void loraImpl_deinit(void)
{
    esp_err_t err;
    err = loraHal_deregisterRxDoneIsr();
    if (err != ESP_OK)
    {
        ESP_LOGE(
            __FUNCTION__,
            "loraHal_deregisterRxDoneIsr failed with error code: %d",
            err);
    }
    vTaskDelete(rxDoneTaskHandle);
    err = loraHal_deinit();
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,
                 "loraHal_deinit failed with error code: %d", err);
    }
}

static void _loraImpl_printGlobalState()
{
    ESP_LOGI(__FUNCTION__, "Global State:");
    ESP_LOGI(__FUNCTION__, "  onTxDone: %p",
             loraImpl_globalState_g.onTxDone);
    ESP_LOGI(__FUNCTION__, "  onRxDone: %p",
             loraImpl_globalState_g.onRxDone);
    ESP_LOGI(__FUNCTION__, "  onTxTimeout: %p",
             loraImpl_globalState_g.onTxTimeout);
    ESP_LOGI(__FUNCTION__, "  onRxTimeout: %p",
             loraImpl_globalState_g.onRxTimeout);
    ESP_LOGI(__FUNCTION__, "  onRxError: %p",
             loraImpl_globalState_g.onRxError);
}

static void _loraImpl_printIrqState()
{
    ESP_LOGI(__FUNCTION__, "IRQ State:");
    ESP_LOGI(__FUNCTION__, "  rxDone: %d",
             atomic_load(&loraImpl_irqState.rxDone));
    ESP_LOGI(__FUNCTION__, "  txDone: %d",
             atomic_load(&loraImpl_irqState.txDone));
    ESP_LOGI(__FUNCTION__, "  rxTimeout: %d",
             atomic_load(&loraImpl_irqState.rxTimeout));
    ESP_LOGI(__FUNCTION__, "  txTimeout: %d",
             atomic_load(&loraImpl_irqState.txTimeout));
    ESP_LOGI(__FUNCTION__, "  rxError: %d",
             atomic_load(&loraImpl_irqState.rxError));
}

void loraImpl_queryState(void)
{
    _loraImpl_printGlobalState();
    _loraImpl_printIrqState();
    // For now we just dump all.
    for (size_t reg = 0x00; reg < loraReg_addrEnd_d; reg++)
    {
        uint8_t val;
        loraHal_readReg(reg, &val);
        ESP_LOGI(__FUNCTION__, "Register: %#x, Value: %#x", reg, val);
    }
}

void loraImpl_irqProcess(void)
{
    // Do not touch any flags relating to rx done, these include:
    // rxDone, rxTimeout, validHeader, crcErr
    uint8_t relevantFlags = loraBit_irqFlags_clearAll_d &
        ~(loraBit_irqFlags_rxDoneMsk_d |
          loraBit_irqFlags_rxTimeoutMsk_d |
          loraBit_irqFlags_validHeadMsk_d |
          loraBit_irqFlags_crcErrMsk_d);
    uint8_t flags;
    loraHal_writeReg(loraReg_addrIrqFlags_d, relevantFlags, &flags);
#ifdef psat_debugMode_d
    if (flags)
        ESP_LOGI(__FUNCTION__, "flags - Value: %#x", flags);
#endif
    if (flags & loraBit_irqFlags_txDoneMsk_d)
    {
        ESP_LOGI(__FUNCTION__, "Processing TX Done IRQ");
        _loraImpl_runCallback_df(loraImpl_globalState_g.onTxDone);
    }

    if (atomic_load(&loraImpl_irqState.rxDone))
    {
        ESP_LOGI(__FUNCTION__, "Processing RX Done IRQ");
        // Only set rxDone if we successfully read from the buffer.
        if (loraImpl_globalState_g.onRxDone &&
            xSemaphoreTake(rxProcessedSemaphore, portMAX_DELAY))
        {
            uint8_t rssi_val;
            loraHal_readReg(loraReg_addrRssi_d, &rssi_val);
            int16_t rssi = -157 + rssi_val;

            // Funky stuff because the the implicit cast to uint8_t might do some sus stuff.
            uint8_t unproccessedSnr;
            loraHal_readReg(loraReg_addrPktSnr_d, &unproccessedSnr);
            int8_t processedSnr = 0;
            memcpy(&processedSnr, &unproccessedSnr, 1);
            processedSnr /= 4;

            loraImpl_globalState_g.onRxDone(rxBuffer, rxBufferSize,
                                            rssi, processedSnr);
            atomic_store(&loraImpl_irqState.rxDone, false);
        }
        else if (!loraImpl_globalState_g.onRxDone)
        {
            atomic_store(&loraImpl_irqState.rxDone, false);
        }
        else
        {
            ESP_LOGI(__FUNCTION__,
                     "Unable to process RX Done irq, other task is "
                     "still writing!");
        }
    }
    if (atomic_load(&loraImpl_irqState.rxError))
    {
        ESP_LOGI(__FUNCTION__, "Processing RX Error IRQ");
        _loraImpl_runCallback_df(loraImpl_globalState_g.onRxError);
        atomic_store(&loraImpl_irqState.rxError, false);
    }

    if (atomic_load(&loraImpl_irqState.rxTimeout))
    {
        ESP_LOGI(__FUNCTION__, "Processing RX Timeout IRQ");
        _loraImpl_runCallback_df(loraImpl_globalState_g.onRxTimeout);
        atomic_store(&loraImpl_irqState.rxTimeout, false);
    }

    if (atomic_load(&loraImpl_irqState.txTimeout))
    {
        ESP_LOGI(__FUNCTION__, "Processing TX Timeout IRQ");
        _loraImpl_runCallback_df(loraImpl_globalState_g.onTxTimeout);
        atomic_store(&loraImpl_irqState.txTimeout, false);
    }
}

void loraImpl_setRx(uint32_t milliseconds)
{
    // ESP_LOGI(__FUNCTION__, "Setting Rx mode, timeout: %lu",
    //          milliseconds);
    uint8_t curOpMode;
    loraHal_readReg(loraReg_addrOpMode_d, &curOpMode);
    loraBit_set(curOpMode, loraBit_opMode_modeMsk_d,
                loraBit_opMode_mode_rxc_d);
    loraHal_writeReg(loraReg_addrOpMode_d, curOpMode, NULL);
    // Also set the rxTimeout duration
    // Truncate it to fit.
    milliseconds                     = (uint16_t)milliseconds;
    uint8_t mdmcfg2_and_rxtimeout[2] = {0};
    loraHal_readRegContinuous(loraReg_addrMdmCfg2_d,
                              mdmcfg2_and_rxtimeout,
                              sizeof(mdmcfg2_and_rxtimeout));
    loraBit_set(mdmcfg2_and_rxtimeout[0],
                loraBit_mdmCfg2_symbTimeMsbMsk_d,
                ((milliseconds >> 8) & 0b11));
    loraBit_set(mdmcfg2_and_rxtimeout[1],
                loraBit_addrSymbTimeLsbMsk_d, (milliseconds & 0xff));
    loraHal_writeRegContinuous(loraReg_addrMdmCfg2_d,
                               mdmcfg2_and_rxtimeout,
                               sizeof(mdmcfg2_and_rxtimeout));
}

void loraImpl_setIdle()
{
    ESP_LOGI(__FUNCTION__, "Setting Idle mode");
    uint8_t curOpMode;
    loraHal_readReg(loraReg_addrOpMode_d, &curOpMode);
    loraBit_set(curOpMode, loraBit_opMode_modeMsk_d,
                loraBit_opMode_mode_stdby_d);
    loraHal_writeReg(loraReg_addrOpMode_d, curOpMode, NULL);
}
