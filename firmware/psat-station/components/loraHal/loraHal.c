#include "loraHal.h"
#include <driver/spi_common.h>
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"
#include "loraHal_boardCfg.h"
#include "loraRegs.h"
#include "portmacro.h"
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <string.h>

// Note: This handle is not initialized because loraHal_init is a placeholder.
static spi_device_handle_t    loraSpiHandle = NULL;
static const char* const      TAG           = "loraHal";

SemaphoreHandle_t spiSemaphore = NULL;

static const spi_bus_config_t loraSpiBusConfig = {
    .mosi_io_num =
        loraHalBoardCfg_spiMosiGpio_d, ///< GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
    .miso_io_num =
        loraHalBoardCfg_spiMisoGpio_d, ///< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.
    .sclk_io_num =
        loraHalBoardCfg_spiSclkGpio_d, ///< GPIO pin for SPI Clock signal, or -1 if not used.
    .data2_io_num =
        -1, ///< GPIO pin for spi data2 signal in quad/octal mode, or -1 if not used.
    .data3_io_num =
        -1, ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    .data4_io_num =
        -1, ///< GPIO pin for spi data4 signal in octal mode, or -1 if not used.
    .data5_io_num =
        -1, ///< GPIO pin for spi data5 signal in octal mode, or -1 if not used.
    .data6_io_num =
        -1, ///< GPIO pin for spi data6 signal in octal mode, or -1 if not used.
    .data7_io_num =
        -1, ///< GPIO pin for spi data7 signal in octal mode, or -1 if not used.
    .data_io_default_level =
        0, // Doesn't matter that much for our case.
    ///< Maximum transfer size, in bytes. Defaults to 4092 if 0 when DMA enabled, or to `SOC_SPI_MAXIMUM_BUFFER_SIZE` if DMA is disabled.
    // Default
    .max_transfer_sz = 0,
    ///< Abilities of bus to be checked by the driver. Or-ed value of ``SPICOMMON_BUSFLAG_*`` flags.
    .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_DUAL,
    .isr_cpu_id =
        ESP_INTR_CPU_AFFINITY_AUTO, ///< Select cpu core to register SPI ISR.
    .intr_flags =
        0, /**< Interrupt flag for the bus to set the priority, and IRAM attribute, see
                           *  ``esp_intr_alloc.h``. Note that the EDGE, INTRDISABLED attribute are ignored
                           *  by the driver. Note that if ESP_INTR_FLAG_IRAM is set, ALL the callbacks of
                           *  the driver, and their callee functions, should be put in the IRAM.
                           */
};

static const spi_device_interface_config_t loraSpiDeviceConfig = {
    .command_bits     = 1,
    .address_bits     = 7,
    .dummy_bits       = 0,
    .mode             = 0, // cpol = 0, cpha = 0
    .clock_source     = SPI_CLK_SRC_DEFAULT,
    .duty_cycle_pos   = 0,
    .cs_ena_pretrans  = 0,
    .cs_ena_posttrans = 0,
    .clock_speed_hz   = loraHalBoardCfg_spiClockSpeedHz_d,
    .input_delay_ns   = 0, // Not dealing with high-speed signals
    .sample_point = SPI_SAMPLING_POINT_PHASE_0, // Only available one
    .spics_io_num = loraHalBoardCfg_spiCsGpio_d,
    .flags        = 0,    // No relevant flags (not that I could see)
    .queue_size   = 10,   // Arbitrary, won't be using anyways.
    .pre_cb       = NULL, // Don't need callbacks.
    .post_cb      = NULL,
};

static void _loraHal_spi_init(void)
{
    ESP_ERROR_CHECK(spi_bus_initialize(loraHalBoardCfg_spiHost_d,
                                       &loraSpiBusConfig,
                                       SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(loraHalBoardCfg_spiHost_d,
                                       &loraSpiDeviceConfig,
                                       &loraSpiHandle));
}

static void _loraHal_spi_deinit()
{
    ESP_ERROR_CHECK(spi_bus_remove_device(loraSpiHandle));
    ESP_ERROR_CHECK(spi_bus_free(loraHalBoardCfg_spiHost_d));
}

struct
{
    bool initComplete;
    bool isOn;
} loraHal_en_state_s = {false, false};

static void _loraHal_en_init(void)
{
    ESP_ERROR_CHECK(gpio_set_direction(loraHalBoardCfg_enableGpio_d,
                                       GPIO_MODE_OUTPUT));
    // Off for now
    ESP_ERROR_CHECK(gpio_set_level(loraHalBoardCfg_enableGpio_d, 0));

    loraHal_en_state_s.initComplete = true;
    loraHal_en_state_s.isOn         = false;
}

static void _loraHal_en_on(void)
{
    if (!loraHal_en_state_s.initComplete)
    {
        ESP_LOGE(TAG,
                 "Attempted to enable lora without initialising "
                 "enable pin!");
        return;
    }
    ESP_ERROR_CHECK(gpio_set_level(loraHalBoardCfg_enableGpio_d, 1));
    loraHal_en_state_s.isOn = true;
}

static void _loraHal_en_off(void)
{
    if (!loraHal_en_state_s.initComplete)
    {
        ESP_LOGE(TAG,
                 "Attempted to enable lora without initialising "
                 "enable pin!");
        return;
    }
    ESP_ERROR_CHECK(gpio_set_level(loraHalBoardCfg_enableGpio_d, 1));
    loraHal_en_state_s.isOn = false;
}

static void _loraHal_en_deinit(void)
{
    ESP_ERROR_CHECK(gpio_set_level(loraHalBoardCfg_enableGpio_d, 0));
    ESP_ERROR_CHECK(gpio_reset_pin(loraHalBoardCfg_enableGpio_d));
    loraHal_en_state_s.initComplete = false;
    loraHal_en_state_s.isOn         = false;
}

static void(*_loraHal_rxDoneIsr)(void*) = NULL;
static void _loraHal_rxDoneIsr_init(void)
{
    // Init the GPIO
    ESP_ERROR_CHECK(gpio_set_direction(loraHalBoardCfg_dio0Gpio_d, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(loraHalBoardCfg_dio0Gpio_d, GPIO_PULLDOWN_ENABLE));
    ESP_ERROR_CHECK(gpio_set_intr_type(loraHalBoardCfg_dio0Gpio_d, GPIO_INTR_POSEDGE));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
}
static void _loraHal_rxDoneIsr_deinit(void)
{
    if (_loraHal_rxDoneIsr)
    {
        // Deregister the isr
        loraHal_deregisterRxDoneIsr();
    }
    // Deinit the correspond GPIO
    gpio_uninstall_isr_service();
    ESP_ERROR_CHECK(gpio_reset_pin(loraHalBoardCfg_dio0Gpio_d));
}

void loraHal_registerRxDoneIsr(void(*isr)(void*))
{
    _loraHal_rxDoneIsr = isr;
    ESP_ERROR_CHECK(gpio_isr_handler_add(loraHalBoardCfg_dio0Gpio_d, isr, NULL));
}

void loraHal_deregisterRxDoneIsr() 
{
    _loraHal_rxDoneIsr = NULL;
    ESP_ERROR_CHECK(gpio_isr_handler_remove(loraHalBoardCfg_dio0Gpio_d));
}

esp_err_t loraHal_init(void)
{
    // Initialise SPI
    _loraHal_spi_init();
    // Initialise enable gpio.
    _loraHal_en_init();
    _loraHal_en_on();
    _loraHal_rxDoneIsr_init();
    // Initialise GPIO for rxDone itr? (probably not, we'll just poll IRQ for now)
    spiSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(spiSemaphore);
    return ESP_OK;
}

esp_err_t loraHal_deinit(void)
{
    xSemaphoreTake(spiSemaphore, portMAX_DELAY);
    _loraHal_rxDoneIsr_deinit();
    _loraHal_en_off();
    // Takes 10ms to reset the lora configuration
    vTaskDelay(portTICK_PERIOD_MS / 10);
    _loraHal_en_deinit();
    _loraHal_spi_deinit();
    return ESP_OK;
}

uint8_t loraHal_writeReg(uint8_t reg, uint8_t val)
{
    if (!loraSpiHandle)
    {
        ESP_LOGE(TAG,
                 "Failed to write register, lora not initialised!");
        return 0;
    }
    spi_transaction_t trans = {
        .cmd     = loraReg_cmdWrite_d,
        .addr    = reg,
        .tx_data = {val},
        .length  = 8,
        .flags   = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
    };
    xSemaphoreTake(spiSemaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(spi_device_transmit(loraSpiHandle, &trans));
    xSemaphoreGive(spiSemaphore);
    return trans.rx_data[0];
}

uint8_t loraHal_readReg(uint8_t reg)
{
    if (!loraSpiHandle)
    {
        ESP_LOGE(TAG,
                 "Failed to read register, lora not initialised!");
        return 0;
    }
    spi_transaction_t trans = {
        .cmd     = loraReg_cmdRead_d,
        .addr    = reg,
        .tx_data = {},
        .rx_data = {},
        .length  = 8,
        .flags   = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
    };
    xSemaphoreTake(spiSemaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(spi_device_transmit(loraSpiHandle, &trans));
    xSemaphoreGive(spiSemaphore);
    return trans.rx_data[0];
}

void loraHal_writeRegContinuous(uint8_t reg, const uint8_t* buffer,
                                uint8_t numBytes)
{
    if (!loraSpiHandle)
    {
        ESP_LOGE(TAG,
                 "Failed to write register, lora not initialised!");
        return;
    }
    spi_transaction_t trans = {
        .cmd       = loraReg_cmdWrite_d,
        .addr      = reg,
        .tx_buffer = buffer,
        .length    = numBytes * 8,
        .flags     = 0,
    };
    xSemaphoreTake(spiSemaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(spi_device_transmit(loraSpiHandle, &trans));
    xSemaphoreGive(spiSemaphore);
}

void loraHal_readRegContinuous(uint8_t reg, uint8_t* buffer,
                               uint8_t numBytes)
{
    if (!loraSpiHandle)
    {
        ESP_LOGE(TAG,
                 "Failed to read register, lora not initialised!");
        return;
    }
    spi_transaction_t trans = {
        .cmd       = loraReg_cmdRead_d,
        .addr      = reg,
        .rx_buffer = buffer,
        .length    = numBytes * 8,
        .flags     = 0,
    };
    xSemaphoreTake(spiSemaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(spi_device_transmit(loraSpiHandle, &trans));
    xSemaphoreGive(spiSemaphore);
}
