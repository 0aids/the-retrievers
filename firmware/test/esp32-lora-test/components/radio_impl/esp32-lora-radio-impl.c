// ================= global_radio.h implementation!!! ================ 

// Implementations for global_radio for the psat stack -
// The Esp32 connects to the lora over uart. The esp32 compiles against this
// file for implementations of radio.

#include "global_radio.h"
#include "uart_config.h"
#include <stdint.h>
#include "driver/uart.h"
#include "esp_err.h"

void gr_RadioSetTxDoneCallback(void (*callback)(void))
{
}

void gr_RadioSetRxDoneCallback(void (*callback)(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr))
{
}

void gr_RadioSetTxTimeoutCallback(void (*callback)(void))
{
}

void gr_RadioSetRxTimeoutCallback(void (*callback)(void))
{
}

void gr_RadioSetRxErrorCallback(void (*callback)(void))
{
}

/*
 * Send a buffer over the radio. This is done asynchronously?.
 */
void gr_RadioSend(uint8_t *buffer, uint16_t bufferSize)
{
}

/*
 * Does a check on the Radio to see if it has received anything, and will
 * run any of the callbacks setup. The radio will only see stuff when Rx is enabled.
 * */
void gr_RadioCheckRecv()
{
}

// Sets the Radio to RX mode for timeout ms.
// If the radio is made to TX during the timeout it will
// immediately leave Rx mode and move to Tx mode. Is non-blocking
void gr_RadioSetRx(uint32_t timeout)
{
}

/*
 * Essentially turn on power saving mode.
 * */
void gr_RadioSetIdle()
{
}


static QueueHandle_t s_uartQueue;
static uart_config_t uart_config = {
    .baud_rate = d_gr_uartBaudRate,
    .data_bits = d_gr_uartDataBits,
    .parity = d_gr_uartParityMode,
    .stop_bits = d_gr_uartStopBits,
    .flow_ctrl = d_gr_uartFlowCtrlMode,
    .rx_flow_ctrl_thresh = d_gr_uartFlowCtrlThresh,
};

void gr_RadioInit()
{
    // Setup uart with the lora.
    ESP_ERROR_CHECK(
        uart_driver_install(
            d_gr_uartPort, 
            d_defaultPacketBufferSize, 
            d_defaultPacketBufferSize, 
            10, 
            &s_uartQueue, 
            0 // itrpt flags
        )
    );

    ESP_ERROR_CHECK(
        uart_param_config(d_gr_uartPort, &uart_config)
    );

    ESP_ERROR_CHECK(
        uart_set_pin(d_gr_uartPort, d_gr_radioTxPort, d_gr_radioRxPort, 
        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)
    );
}

e_radioState gr_RadioGetStatus()
{
    return gr_RadioStates_Idle;
}

// A little bit slow, but will ask for returned data.
int16_t gr_RadioGetRSSI()
{
    return 1;
}

uint32_t gr_RadioGetTimeOnAir(packet_t *packet)
{
    return 1;
}


