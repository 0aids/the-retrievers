#ifdef d_psatStandaloneMode
#include <stdio.h>
#include "delay.h"
#include "tremo_cm4.h"
#include "tremo_delay.h"
#include "tremo_regs.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"

// Why does timer.h need to be here???
#include "timer.h"
#include "rtc-board.h"
#include "tremo_lpuart.h"
#include "psat_to_esp_uart_config.h"
#endif

void uartLogInit()
{
#ifdef d_psatStandaloneMode
    // uart0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = UART_BAUDRATE_115200;
    uart_config.fifo_mode = ENABLE;
    uart_init(CONFIG_DEBUG_UART, &uart_config);
    uart_cmd(CONFIG_DEBUG_UART, ENABLE);
#endif
}

void uartToEsp32Init()
{
#ifdef d_psatStandaloneMode
    // uart1
    // IO4 = uart1 TX
    // IO5 = uart1 RX

    // func_num of 1: UART1_RXD and UART1_TXD
    gpio_set_iomux(GPIOA, d_gr_radioRxPort, 1);
    gpio_set_iomux(GPIOA, d_gr_radioTxPort, 1);

    uart_config_t uart_config;
    // Defaults to RxTx mode.
    uart_config_init(&uart_config);

    uart_config.baudrate = d_gr_uartBaudRate;
    uart_config.data_width = d_gr_uartDataBits;
    uart_config.parity = d_gr_uartParityMode;
    uart_config.stop_bits = d_gr_uartStopBits;

    uart_config.fifo_mode = ENABLE; // Enable inputs and outputs
    uart_init(d_gr_uartPort, &uart_config);
    uart_cmd(d_gr_uartPort, ENABLE);
#endif
}

void initBoard()
{
#ifdef d_psatStandaloneMode
    rcc_enable_oscillator(RCC_OSC_XO32K, true);

    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART1, true);
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
    
    uartLogInit();
    uartToEsp32Init();

    RtcInit();
#endif
}

#define d_uartInputBufferSize 256
static char g_uartInputBuffer[d_uartInputBufferSize] = {0};

void appMain()
{
#ifdef d_psatStandaloneMode
    // Test: Poll the uart input and print it to the screen.
    uint16_t iter = 0;
    printf("Starting loop!\r\n");
    while (1) {
        DelayMs(10);
        if (iter++ % 50 == 0) {
            printf("Still Looping\r\n");
        }
        if (uart_get_flag_status(d_gr_uartPort, UART_FLAG_RX_FIFO_EMPTY) == RESET)
        {
            printf("Received something in uart!\r\n");
            // Clear the buffer
            uint16_t i = 0;
            while(uart_get_flag_status(d_gr_uartPort, UART_FLAG_RX_FIFO_EMPTY) == RESET &&
                  i < d_uartInputBufferSize - 1)
            {
                g_uartInputBuffer[i++] = uart_receive_data(d_gr_uartPort);
                printf("Received char: %#X\r\n", g_uartInputBuffer[i-1]);
                DelayMs(10);
            }
            // End the string.
            g_uartInputBuffer[i] = 0;
            printf("%s\r\n", g_uartInputBuffer);
        }
    }
#endif
}
