#ifndef d_psatStandaloneMode
#include <stdio.h>
#include "delay.h"
#include "tremo_cm4.h"
#include "tremo_delay.h"
#include "tremo_regs.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "global_radio.h"

// Why does timer.h need to be here???
#include "timer.h"
#include "rtc-board.h"
#include "tremo_lpuart.h"
#include "psat_to_esp_uart_config.h"
#include "gps_state.h"
#endif

static gr_irqsToHandle_t s_irqsToHandle;
static packet_t g_recvPacket;
static packet_t g_sendPacket;
static uint32_t s_pleaseAlignMe32bit;
static gps_state_t s_gpsState;

void PrintGpsState(const gps_state_t *gps)
{
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
    printf("  Fix Quality:     %d\r\n", gps->fix_quality);
    printf("  Sats Tracked:    %d\r\n", gps->satellites_tracked);
    printf("  Sats in View:    %d\r\n", gps->sats_in_view);

    printf("\r\nTimestamp:\r\n");
    // BUG: Buggy printf implementation means multiple format specifiers causes a crash!
    // printf("  Date:            %02d-%02d-%04d\n", gps->day, gps->month, gps->year);
    // printf("  Time:            %02d:%02d:%02d\n", gps->hours, gps->minutes, gps->seconds);
    printf("  Date:            %02d", gps->day);
    printf("-%02d", gps->month);
    printf("-%04d\r\n", gps->year);
    printf("  Time:            %02d:", gps->hours);
    printf("%02d:", gps->minutes);
    printf("%02d\r\n", gps->seconds);

    printf("\r\nValidity Flags:\r\n");
    printf("  Position Valid:  %s\r\n", gps->position_valid ? "true" : "false");
    printf("  Nav Valid:       %s\r\n", gps->nav_valid ? "true" : "false");
    printf("  Fix Info Valid:  %s\r\n", gps->fix_info_valid ? "true" : "false");
    printf("  Altitude Valid:  %s\r\n", gps->altitude_valid ? "true" : "false");

    printf("===================\r\n");
}


void uartLogInit()
{
#ifndef d_psatStandaloneMode
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
#ifndef d_psatStandaloneMode
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
    uart_config.flow_control = UART_FLOW_CONTROL_DISABLED;

    uart_config.fifo_mode = ENABLE;
    uart_init(d_gr_uartPort, &uart_config);
    uart_cmd(d_gr_uartPort, ENABLE);
#endif
}

void initBoard()
{
#ifndef d_psatStandaloneMode
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

void ForwarderOnTxDone( void )
{
    gr_RadioSetIdle( );
    s_irqsToHandle |= gr_irqs_TxDone;
    printf("Sent!\r\n");
}

void ForwarderOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    // Store it here
    g_recvPacket = ParsePacket(payload, size + 1);
    printPacketStats(&g_recvPacket);
    s_irqsToHandle |= gr_irqs_RxDone;
}

void ForwarderOnTxTimeout( void )
{
    s_irqsToHandle |= gr_irqs_TxTimeout;
    printf("PSAT tx timeout\r\n");
    // RadioSleep( );
}

void ForwarderOnRxTimeout( void )
{
    s_irqsToHandle |= gr_irqs_RxTimeout;
    printf("PSAT rx timeout\r\n");
    // RadioSleep( );
}

void ForwarderOnRxError( void )
{
    s_irqsToHandle |= gr_irqs_RxError;
    printf("PSAT rx ERROR\r\n");
    // RadioSleep( );
}


#define d_uartInputBufferSize 256
static uint8_t g_uartInputBuffer[d_uartInputBufferSize] = {0};
static espToLoraPacket_t s_espToLoraPacket;

void appMain()
{
#ifndef d_psatStandaloneMode
    // TODO: Figure out how to get this done without polling IE using interrupts.
    uint16_t iter = 0;
    printf("Starting loop!\r\n");
    gr_RadioInit();
    gr_RadioSetRxDoneCallback(ForwarderOnRxDone);
    while (1) {
        // DelayMs(10);
        // if (iter++ % 50 == 0) {
        //     printf("Still Looping\r\n");
        // }
        DelayMs(1);
        if (uart_get_flag_status(d_gr_uartPort, UART_FLAG_RX_FIFO_EMPTY) == RESET)
        {
            printf("Received something in uart!\r\n");
            // Clear the buffer
            uint8_t i = 0;
            while(uart_get_flag_status(d_gr_uartPort, UART_FLAG_RX_FIFO_EMPTY) == RESET &&
                  i < d_uartInputBufferSize - 1)
            {
                g_uartInputBuffer[i++] = uart_receive_data(d_gr_uartPort);
                printf("Received char %u: %#X\r\n", i-1,  g_uartInputBuffer[i-1]);
                DelayMs(4);
            }
            uint8_t bufferSize = i;
            printf("Inter-board buffer size: %u\r\n", bufferSize);
            s_espToLoraPacket = EspToLoraPacket_CreateFromBuffer(g_uartInputBuffer, bufferSize);
            EspToLoraPacket_PrintPacketStats(&s_espToLoraPacket);

            // Send the ack (The amount of bytes received)
            uart_send_data(d_gr_uartPort, bufferSize);

            switch (s_espToLoraPacket.requestType) {
                case grReq_RadioSend:
                    memcpy(&s_gpsState, &(s_espToLoraPacket.dataBuffer[1]) , sizeof(gps_state_t));
                    PrintGpsState(&s_gpsState);
                    gr_RadioSend(s_espToLoraPacket.dataBuffer, s_espToLoraPacket.m_dataBufferSize);
                    // Forward the data using the standalone.
                    break;
                case grReq_RadioCheckRecv:
                    // Sends the irqs that got triggered.
                    s_irqsToHandle = gr_irqs_None;
                    gr_RadioCheckRecv();
                    // And then we send the activated ones back.
                    uart_send_data(d_gr_uartPort, s_irqsToHandle);
                    break;

                case grReq_RadioSetRx:
                    // Check the timeout's size
                    if (s_espToLoraPacket.m_dataBufferSize != 4) {
                        printf("Error: Didn't receive correct payload size!");
                        break;
                    }
                    // Using static variable to align it.
                    memcpy(&s_pleaseAlignMe32bit, s_espToLoraPacket.dataBuffer, sizeof(uint32_t));
                    printf("Setting Rx for timeout: %ums\r\n", s_pleaseAlignMe32bit);
                    gr_RadioSetRx(s_pleaseAlignMe32bit);
                    break;

                case grReq_RadioSetIdle:
                    printf("Setting idle!\r\n");
                    gr_RadioSetIdle();
                    break;

                case grReq_RadioGetStatus:
                    printf("Returning Status!\r\n");
                    uart_send_data(d_gr_uartPort, gr_RadioGetStatus());
                    break;

                case grReq_RadioGetRSSI:
                    printf("Returning RSSI!\r\n");
                    int16_t rssi = gr_RadioGetRSSI();
                    uint8_t low = rssi & 0xFF;
                    uint8_t high = (rssi >> 8) & 0xFF;
                    // Send the data using funky conversions
                    // Convert the int16_t into uint8_t[2], and then send them byte by byte
                    uart_send_data(d_gr_uartPort, low);
                    uart_send_data(d_gr_uartPort, high);
                    break;

                case grReq_RadioGetTimeOnAir:
                    // uint32_t timeOnAir =  gr_RadioGetTimeOnAir();
                    // uart_send_data(d_gr_uartPort, ((uint8_t*)&rssi)[0]);
                    // uart_send_data(d_gr_uartPort, ((uint8_t*)&rssi)[1]);
                    // NOTE: I have no clue how to implement this rn, so i'll make it send 0
                    uart_send_data(d_gr_uartPort, 0);
                    uart_send_data(d_gr_uartPort, 0);
                    uart_send_data(d_gr_uartPort, 0);
                    uart_send_data(d_gr_uartPort, 0);
                    printf("Time on air is not implemented! Sent 0ms!\r\n");
                    break;

                case grReq__RadioRequestRxPacket:
                    printf("Sending stored Rx Packet!!!\r\n");
                    if (g_recvPacket.type == EMPTY) {
                        printf("Error: There is no packet waiting to be received???\r\n");
                        break;
                    }
                    // TODO: Continue later.
                    break;

                default:
                    printf("Unimplemented or invalid!\r\n");
                    break;
            }
        }
    }
#endif
}
