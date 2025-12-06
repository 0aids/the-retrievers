#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "global_radio.h"
#include "helpers.h"
#include "packets/packets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "psat_lora.h"
#include "uart_config.h"
#include "gps_state.h"

static gps_state_t g_fakeGpsData = {
    .latitude           = -36.8485,     // Auckland-ish
    .longitude          = 174.7633,
    .speed_knots        = 12.5,
    .speed_kph          = 23.1,
    .course_deg         = 87.0,         // Heading east
    .hdop               = 0.9,
    .altitude           = 150.0,        // meters
    .geoidal_sep        = -34.2,

    .day                = 5,
    .month              = 12,
    .year               = 2025,
    .hours              = 14,
    .minutes            = 37,
    .seconds            = 22,

    .fix_quality        = 1,            // Standard GPS fix
    .satellites_tracked = 10,
    .sats_in_view       = 14,

    .position_valid     = true,
    .nav_valid          = true,
    .fix_info_valid     = true,
    .altitude_valid     = true,
};
static packet_t s_packetSend;

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


static uart_config_t uart0_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};

// A basic function to setup communication with computer over serial
static void setupUsbSerialUart() {
    ESP_ERROR_CHECK(
        uart_driver_install(
            UART_NUM_0, 
            d_defaultPacketBufferSize, 
            0, 
            0, 
            NULL,
            0 // itrpt flags
        )
    );

    ESP_ERROR_CHECK(
        uart_param_config(UART_NUM_0, &uart0_config)
    );

    ESP_ERROR_CHECK(
        uart_set_pin(UART_NUM_0, 1, 3, 
        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)
    );
}

void blockingGetInput(uart_port_t uartPort, char* buffer, uint16_t bufferSize)
{
    char charIn = 0;
    uint16_t bufferInd = 0;

    while (charIn != '\r')
    {
        int len = uart_read_bytes(
            UART_NUM_0, 
            &charIn, 
            sizeof(charIn), 
            100 / portTICK_PERIOD_MS
        );

        if (len == 1 && charIn != '\r' && bufferInd< bufferSize) {
            buffer[bufferInd++] = charIn;
            printf("%c", charIn);
            fflush(stdout);
        }
        else if (len == 1 && bufferInd > 0) {
            uint16_t bufferLength = bufferInd;
            buffer[bufferInd] = '\0';
            bufferInd = 0;
            printf("\r\nFull line: %s\r\n", buffer);
            return;
        }
    }
}

static char inputBuffer[d_defaultPacketBufferSize];
static uint16_t inputBufferInd = 0;

void app_main(void)
{
    gr_RadioInit();
    setupUsbSerialUart();

    while (1) {
        blockingGetInput(UART_NUM_0, inputBuffer, d_defaultPacketBufferSize);
        uart_flush_input(d_gr_uartPort);
        if (strcmp("fsm", inputBuffer) == 0) {
            printf("running fsm!\r\n");
            logging_verbose = false;
            uart_driver_delete(d_gr_uartPort);
            PsatRadioInit();
            while (1) {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                PsatRadioMain();
            }
        }
        e_grRequest requestType = grReq_StringToEnum((char*)inputBuffer);
        if (requestType == grReq_Invalid)
        {
            printf("Invalid Request!!\r\n");
            grReq_printAllTypes();
            continue;
        }

        switch (requestType) {
            case grReq_RadioSend:
                // As a test we'll send the fake gps data.
                printf("Sending fake gps data!\r\n");
                s_packetSend = CreatePacket(
                    GPS_DATA, 
                  (uint8_t*)&g_fakeGpsData, 
              sizeof(g_fakeGpsData)
                );
                PrintGpsState((gps_state_t*)s_packetSend.data);
                printf("Inter-board packet-size: %u\r\n", s_packetSend.m_dataSize + 1);
                gr_RadioSend((uint8_t*)&s_packetSend, s_packetSend.m_dataSize + 1);
                break;
            case grReq_RadioCheckRecv:
                gr_RadioCheckRecv();
                break;

            case grReq_RadioSetRx:
                // Using a test timeout of 5000ms
                gr_RadioSetRx(5000);
                break;

            case grReq_RadioSetIdle:
                gr_RadioSetIdle();
                break;

            case grReq_RadioGetStatus:
                e_radioState state = gr_RadioGetStatus();
                printf("Radio state given: %s\r\n", gr_RadioStatesToString(state));
                break;

            case grReq_RadioGetRSSI:
                int16_t rssi = gr_RadioGetRSSI();
                printf("Rssi given: %d\r\n", rssi);
                break;
                
            case grReq_RadioGetTimeOnAir:
                printf("Time on air is not implemented!\r\n");
                break;

            // case grReq__RadioRequestRxPacket:
            //     printf("Retrieving the packet in the Lora's Rx!!!\r\n");
            //     break;

            default:
                printf("Invalid or unimplemented (_RadioRequestRxPacket must be run via checkrecv.)!\r\n");
                break;
        }
        // Convert this into a basic command scheme
        // printf("Sending that line!\r\n");
        // blockingTransmitBuffer(d_gr_uartPort, inputBuffer, bufferLength);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
