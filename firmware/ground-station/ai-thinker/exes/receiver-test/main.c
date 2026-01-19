#include <stdio.h>
#include <string.h>
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

void dummyCallback() {}
char recvBuffer[255] = {};
void testRxDoneCallback(uint8_t* payload, uint16_t len, int16_t rssi,
                        int8_t snr)
{
    uint16_t actualLength = (len > 254) ? 254 : len;
    memcpy(recvBuffer, payload, actualLength);
    recvBuffer[actualLength] = 0;
    printf("Received: %s\r\n", recvBuffer);
    printf("In hex:");
    for (uint8_t i = 0; i < actualLength; i++)
    {
        printf(" %02x", (uint16_t) payload[i]);
    }
    printf("\r\n");
    printf("size: %" PRIu16, len);
    printf(" rssi: %" PRId16, rssi);
    printf("  snr: %" PRId8 "\r\n", snr);
}

void main(void)
{
    board_init();
    printf("Initializing...\r\n");
    loraImpl_setCallbacks(dummyCallback, testRxDoneCallback,
                          dummyCallback, dummyCallback,
                          dummyCallback);
    loraImpl_init();
    delay_ms(100);
    loraImpl_setRx(0);
    printf("Initialization complete!\r\n");
    uint16_t i = 0;
    while (true)
    {
        if (i++ % 1000 == 0)
            printf("Looping!\r\n");
        delay_ms(1);
        loraImpl_irqProcess();
    }
}
