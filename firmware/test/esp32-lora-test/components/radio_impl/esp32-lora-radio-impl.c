// ================= global_radio.h implementation!!! ================ 
// Implementations for global_radio for the psat stack -
// The Esp32 connects to the lora over uart. The esp32 compiles against this
// file for implementations of radio.

#include "global_radio.h"
#include "helpers.h"
#include "uart_config.h"
#include <stdint.h>
#include "driver/uart.h"
#include "esp_err.h"

static espToLoraPacket_t s_espToLoraPacket;

static void gr_RadioDefaultCallback(void) {
    printf("Default callback\r\n");
}
static void gr_RadioDefaultRxDoneCallback(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr) {
    printf("Default RxDone callback\r\n");
}

static void (*gr_RadioTxDoneCallback)(void) = gr_RadioDefaultCallback;
static void (*gr_RadioRxDoneCallback)(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr) = gr_RadioDefaultRxDoneCallback;
static void (*gr_RadioTxTimeoutCallback)(void) = gr_RadioDefaultCallback;
static void (*gr_RadioRxTimeoutCallback)(void) = gr_RadioDefaultCallback;
static void (*gr_RadioRxErrorCallback)(void) = gr_RadioDefaultCallback;

void gr_RadioSetTxDoneCallback(void (*callback)(void))
{
    gr_RadioTxDoneCallback = callback;
}

void gr_RadioSetRxDoneCallback(void (*callback)(uint8_t* data, uint16_t dataLength, int16_t rssi, int8_t snr))
{
    gr_RadioRxDoneCallback = callback;
}

void gr_RadioSetTxTimeoutCallback(void (*callback)(void))
{
    gr_RadioTxTimeoutCallback = callback;
}

void gr_RadioSetRxTimeoutCallback(void (*callback)(void))
{
    gr_RadioRxTimeoutCallback = callback;
}

void gr_RadioSetRxErrorCallback(void (*callback)(void))
{
    gr_RadioRxErrorCallback = callback;
}

/*
 * Send a buffer over the radio. This is done asynchronously?.
 */
void gr_RadioSend(uint8_t *buffer, uint16_t bufferSize)
{
    s_espToLoraPacket = 
        EspToLoraPacket_Create(
           grReq_RadioSend, 
           buffer, 
           bufferSize
        );

    blockingTransmitBuffer(
        d_gr_uartPort, 
    (uint8_t*)&s_espToLoraPacket, 
    s_espToLoraPacket.m_dataBufferSize + 1
    );
    // The lora will send an ack to acknowledge it's received the entire payload
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, s_espToLoraPacket.m_dataBufferSize + 1);
}

// Checks uart to see if anything is there. If so, we will run the relevant callbacks
void gr_RadioCheckRecv()
{
    const static e_grRequest reqType = grReq_RadioCheckRecv;
    blockingTransmitBuffer(
        d_gr_uartPort,
        &reqType,
        sizeof(reqType)
    );
    gr_irqsToHandle_t irqsToHandle = gr_irqs_None;
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(reqType));
    int len = uart_read_bytes(d_gr_uartPort, &irqsToHandle, sizeof(irqsToHandle), 100 / portTICK_PERIOD_MS); 
    if (len == sizeof(irqsToHandle)) {
        printf("Received: %0#1b\r\n", irqsToHandle);
    } else {
        printf("Invalid response received! len = %d\r\n", len);
    }

    if (irqsToHandle & gr_irqs_TxDone) {
        gr_RadioTxDoneCallback();
    }
    if (irqsToHandle & gr_irqs_RxDone) {
        // TODO: Collect the received packet
        // This means we send another request to retreive the packet.
        const static e_grRequest rxDoneGetPacket = grReq__RadioRequestRxPacket;
        blockingTransmitBuffer(d_gr_uartPort, &rxDoneGetPacket, sizeof(rxDoneGetPacket));
        // waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(rxDoneGetPacket));
        // uart_flush(d_gr_uartPort);
        // Get packet
        // We don't know the actual size of the packet
        // yolo it, can't be assed to add error checking
        waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(rxDoneGetPacket));
        int len = uart_read_bytes(d_gr_uartPort, &s_rxDonePacket, sizeof(s_rxDonePacket), 500 / portTICK_PERIOD_MS); 
        if (len <= 0) {
            printf("Failed to retrieve any data from rxdone???\r\n");
            irqsToHandle |= gr_irqs_RxError;
            goto goto_RxDoneError;
        }
        verbPrintf("Received %d bytes! \r\n", len);
        s_rxDonePacket.m_dataLength = len - sizeof(s_rxDonePacket.rssi) - sizeof(s_rxDonePacket.snr);

        for (uint16_t i = 0; i < len; i++) {
            verbPrintf("Received char %u: ", i);
            verbPrintf("%#X,   ", ((uint8_t*)&s_rxDonePacket)[i]);
            verbPrintf("%c\r\n", ((uint8_t*)&s_rxDonePacket)[i]);
        }
        

        gr_RadioRxDoneCallback(
            s_rxDonePacket.data,
            s_rxDonePacket.m_dataLength,
            s_rxDonePacket.rssi,
            s_rxDonePacket.snr
        );
    }
    goto_RxDoneError:
    if (irqsToHandle & gr_irqs_TxTimeout) {
        gr_RadioTxTimeoutCallback();
    }
    if (irqsToHandle & gr_irqs_RxTimeout) {
        gr_RadioRxTimeoutCallback();
    }
    if (irqsToHandle & gr_irqs_RxError) {
        gr_RadioRxErrorCallback();
    }
}

// Sets the Radio to RX mode for timeout ms.
// If the radio is made to TX during the timeout it will
// immediately leave Rx mode and move to Tx mode. Is non-blocking
void gr_RadioSetRx(uint32_t timeout)
{
    const static e_grRequest reqType = grReq_RadioSetRx;
    s_espToLoraPacket = EspToLoraPacket_Create(reqType, (uint8_t*)&timeout, sizeof(timeout));
    blockingTransmitBuffer(
        d_gr_uartPort,
        (uint8_t*)&s_espToLoraPacket,
        s_espToLoraPacket.m_dataBufferSize + 1
    );
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, s_espToLoraPacket.m_dataBufferSize + 1);
}

/*
 * Essentially turn on power saving mode.
 * */
void gr_RadioSetIdle()
{
    const static e_grRequest reqType = grReq_RadioSetIdle;
    blockingTransmitBuffer(
        d_gr_uartPort,
        (uint8_t*)&reqType,
        sizeof(reqType)
    );
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(reqType));
}

static QueueHandle_t s_uartQueue;
static uart_config_t uart_config = {
    .baud_rate = d_gr_uartBaudRate,
    .data_bits = d_gr_uartDataBits,
    .parity = d_gr_uartParityMode,
    .stop_bits = d_gr_uartStopBits,
    .flow_ctrl = d_gr_uartFlowCtrlMode,
    .source_clk = UART_SCLK_DEFAULT,
};

void gr_RadioInit()
{
    // Setup uart with the lora.
    ESP_ERROR_CHECK(
        uart_driver_install(
            d_gr_uartPort, 
            d_defaultPacketBufferSize, 
            d_defaultPacketBufferSize, 
            0, 
            NULL, 
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
    const static e_grRequest reqType = grReq_RadioGetStatus;
    blockingTransmitBuffer(
        d_gr_uartPort,
        (uint8_t*)&reqType,
        sizeof(reqType)
    );
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(reqType));

    e_radioState status = gr_RadioStates_Unknown;
    int len = uart_read_bytes(d_gr_uartPort, &status, sizeof(status), 100 / portTICK_PERIOD_MS); 
    if (len == sizeof(status)) {
        printf("Received: %s\r\n", gr_RadioStatesToString(status));
    } else {
        printf("Invalid response received! len = %d\r\n", len);
    }
    return status;
}

// A little bit slow, but will ask for returned data.
int16_t gr_RadioGetRSSI()
{
    const static e_grRequest reqType = grReq_RadioGetRSSI;
    blockingTransmitBuffer(
        d_gr_uartPort,
        (uint8_t*)&reqType,
        sizeof(reqType)
    );
    waitForAck(d_gr_uartPort, d_uartAckTimeout_ms, sizeof(reqType));

    int16_t rssi = gr_RadioStates_Unknown;
    int len = uart_read_bytes(d_gr_uartPort, &rssi, sizeof(rssi), 100 / portTICK_PERIOD_MS); 
    if (len == sizeof(rssi)) {
        printf("Received rssi: %ddbm\r\n", rssi);
    } else {
        printf("Invalid response received! len = %d\r\n", len);
    }
    return rssi;
}

uint32_t gr_RadioGetTimeOnAir(packet_t *packet)
{
    printf("Unimplemented!\r\n");
    return 1;
}


