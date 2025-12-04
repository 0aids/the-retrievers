#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "global_radio.h"
#include "packets/packets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "portmacro.h"
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
        e_grRequest requestType = grReq_StringToEnum((char*)inputBuffer);
        if (requestType == grReq_Invalid)
        {
            printf("Invalid Request!!\r\n");
            grReq_printAllTypes();
        }

        switch (requestType) {
            case grReq_RadioSend:
                // As a test we'll send the fake gps data.
                printf("Sending fake gps data!\r\n");
                packet_t packet = CreatePacket(
                    GPS_DATA, 
                  (uint8_t*)&g_fakeGpsData, 
              sizeof(g_fakeGpsData)
                );
                printf("Inter-board packet-size: %u\r\n", packet.m_dataSize + 1);
                gr_RadioSend((uint8_t*)&packet, packet.m_dataSize + 1);
                break;
            case grReq_RadioCheckRecv:
            case grReq_RadioSetRx:
            case grReq_RadioSetIdle:
            case grReq_RadioGetStatus:
            case grReq_RadioGetRSSI:
            case grReq_RadioGetTimeOnAir:
            default:
                printf("Unimplemented or invalid!\r\n");
        }
        // Convert this into a basic command scheme
        // printf("Sending that line!\r\n");
        // blockingTransmitBuffer(d_gr_uartPort, inputBuffer, bufferLength);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
