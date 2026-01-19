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
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce nulla urna, laoreet dapibus pellentesque in, porta eget mauris. Sed massa sapien, eleifend a pretium vitae, interdum vitae erat. Maecenas sit amet laoreet magna. Donec ac nibh in ligula cursus lobortis ut a velit. Sed scelerisque, orci pulvinar porttitor eleifend, quam ante aliquam elit, sed pulvinar lectus orci eu ligula. Integer egestas molestie magna, eu interdum tortor aliquet vitae. Suspendisse sagittis, nibh sit amet tempor congue, orci velit mattis sem, non pulvinar ex felis eget elit. Aliquam fringilla tortor lacus, in fermentum felis convallis id. Nullam dolor mauris, mollis at ultrices tincidunt, commodo sed nibh.In congue lectus sit amet urna consequat laoreet. Sed vulputate efficitur velit at aliquet. Donec id purus vehicula, dapibus justo ac, malesuada nunc. Suspendisse id felis et neque feugiat tincidunt ac ac neque. Phasellus venenatis non urna ut ornare. Vivamus at orci urna. Ut euismod facilisis augue a volutpat. Ut facilisis elit et scelerisque mollis. Nunc consequat, erat sed condimentum facilisis, tellus lorem tincidunt risus, eget semper nunc leo id risus.Maecenas condimentum bibendum odio. Maecenas auctor varius nibh ac ullamcorper. Nam convallis, magna in consequat faucibus, elit lorem sodales ipsum, vitae feugiat lacus nulla id arcu. Donec eget finibus libero. Duis vestibulum nec nibh eu semper. In et velit mauris. Proin hendrerit posuere nisl, ac dictum sapien eleifend eget. In hac habitasse platea dictumst. Vivamus eu luctus elit. Mauris in bibendum erat, varius aliquet leo. Interdum et malesuada fames ac ante ipsum primis in faucibus. Quisque consectetur dapibus eros, eu tincidunt nisi fermentum non. Nam velit purus, sollicitudin quis purus non, dignissim imperdiet metus. Sed rutrum, nulla sit amet dignissim suscipit, lectus augue gravida felis, nec gravida enim nunc malesuada velit.Etiam at malesuada justo. Vestibulum tincidunt blandit nulla, et ullamcorper sem cursus sed. Sed nec massa id velit dictum feugiat. Aliquam non. ";

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
