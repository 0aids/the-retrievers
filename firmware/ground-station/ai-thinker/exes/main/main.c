#include <stdio.h>
#include <string.h>
#include <shared_lora.h>
#include "printWrapper.h"
#include "loraImpl.h"
#include "tremo_cm4.h"
#include "tremo_delay.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include <inttypes.h>

// Why does timer.h need to be here???
#include "timer.h"
#include "rtc-board.h"

#define endPacket() \
	for (uint8_t i = 0; i < 4; i++) uart_send_data(CONFIG_DEBUG_UART, 0xaa)
#define printb(...) printw(__VA_ARGS__); \
	endPacket();

// Returns 0 if nothing is in the buffer.
// Otherwise it will block until receiving anything, and return the number of bytes received.
// It will also stop recording at bufferSize to not overflow.
uint16_t receiveUart(uint8_t* buffer, uint16_t bufferSize)
{
    if (uart_get_flag_status(CONFIG_DEBUG_UART,
                             UART_FLAG_RX_FIFO_EMPTY) == SET)
    {
        return 0;
    }
    uint16_t i = 0;
    while (uart_get_flag_status(CONFIG_DEBUG_UART,
                                UART_FLAG_RX_FIFO_EMPTY) != SET &&
           i != bufferSize - 1)
    {
        buffer[i++] = uart_receive_data(CONFIG_DEBUG_UART);
    }
    return i;
}

void uart_log_init(void)
{
    // uart0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate  = UART_BAUDRATE_115200;
    uart_config.fifo_mode = ENABLE;
    uart_init(CONFIG_DEBUG_UART, &uart_config);
    uart_cmd(CONFIG_DEBUG_UART, ENABLE);
}

void board_init()
{
    rcc_enable_oscillator(RCC_OSC_XO32K, true);

    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    delay_ms(100);
    pwr_xo32k_lpm_cmd(true);

    uart_log_init();

    RtcInit();
}

uint8_t txrxBuffer[2048] = {};

void txDoneCallback()
{
    printb("txDoneCallback\r\n");
    loraImpl_setRx(0);
}

void rxDoneCallback(uint8_t* payload, uint16_t len, int16_t rssi,
                    int8_t snr)
{
    uint16_t actualSize = (len > 2047) ? 2047 : len;
    memcpy(txrxBuffer, payload, actualSize);
    printb("rxDoneCallback\r\n");
    delay_ms(1);
    for (uint16_t i = 0; i < actualSize; i++)
    {
        uart_send_data(CONFIG_DEBUG_UART, txrxBuffer[i]);
    }
    endPacket();
    loraImpl_setRx(0);
}

void txTimeoutCallback()
{
    printb("txTimeoutCallback\r\n");
    loraImpl_setRx(0);
}

void rxTimeoutCallback()
{
    printb("rxTimeoutCallback\r\n");
    loraImpl_setRx(0);
}

void rxErrorCallback()
{
    printb("rxErrorCallback\r\n");
    loraImpl_setRx(0);
}
// For debug prints here, set first byte to 0xff, and then append the rest.
// Allocate 4kb array for sending and receiving.

void main(void)
{
    loraImpl_setCallbacks(txDoneCallback, rxDoneCallback,
                      txTimeoutCallback, rxTimeoutCallback,
                      rxErrorCallback);
    board_init();
    printb("Initializing the lora\r\n");
    loraImpl_init();
    delay_ms(100);
    printb("Lora initialized!\r\n");
    loraImpl_setRx(0);
    uint16_t size;
    while (true)
    {
        if ((size = receiveUart(txrxBuffer, sizeof(txrxBuffer))))
        {
            loraImpl_send(txrxBuffer, size);
            loraImpl_setRx(0);
        }
        loraImpl_irqProcess();
    }
}
