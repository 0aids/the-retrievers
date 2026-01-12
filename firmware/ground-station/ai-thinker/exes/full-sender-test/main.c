#include <stdio.h>
#include <string.h>
#include <shared_lora.h>
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

char recvBuffer[2048] = {};

void txDoneCallback()
{
    printf("txdoneCallback\r\n");
}

void rxDoneCallback(uint8_t* payload, uint16_t len, int16_t rssi,
                    int8_t snr)
{
    uint16_t actualSize = (len > 2047) ? 2047 : len;
    memcpy(recvBuffer, payload, actualSize);
    recvBuffer[actualSize] = 0;
    printf("Received: %s\r\n", recvBuffer);
    printf("size: %" PRIu16, len);
    printf(" rssi: %" PRId16, rssi);
    printf("  snr: %" PRId8 "\r\n", snr);
}

void txTimeoutCallback()
{
    printf("txTimeoutCallback\r\n");
}

void rxTimeoutCallback()
{
    printf("rxTimeoutCallback\r\n");
}

void rxErrorCallback()
{
    printf("rxErrorCallback\r\n");
}

uint8_t payload[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut "
    "enim ad minim veniam, quis nostrud exercitation ullamco laboris "
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor "
    "in reprehenderit in voluptate velit esse cillum dolore eu "
    "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
    "proident, sunt in culpa qui officia deserunt mollit anim id est "
    "laborum.";

void main(void)
{
    board_init();
    printf("Initializing...\r\n");
    lora_init();
    lora_setCallbacks(txDoneCallback, rxDoneCallback,
                      txTimeoutCallback, rxTimeoutCallback,
                      rxErrorCallback);
    delay_ms(100);
    printf("Initialization complete!\r\n");
    while (true)
    {
        printf("Sending payload!\r\n");
        lora_send(payload, sizeof(payload));
        delay_ms(10000);
        lora_irqProcess();
    }
}
