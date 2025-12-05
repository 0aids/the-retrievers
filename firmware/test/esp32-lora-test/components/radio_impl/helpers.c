#include "helpers.h"
#include "driver/uart.h"
#include "freertos/idf_additions.h"
#include "uart_config.h"

void blockingTransmitBuffer(uart_port_t uartPort, const uint8_t* const buffer, uint16_t bufferLength)
{
    uart_write_bytes(uartPort, buffer,bufferLength);
    if (
        uart_wait_tx_done(uartPort, d_uartTxTimeout_ms/portTICK_PERIOD_MS) == 
        ESP_ERR_TIMEOUT
    )
    {
        verbPrintf("Failed to send!\r\n");
    }
    else verbPrintf("Sent successfully! \r\n");
}

bool waitForAck(uart_port_t uartPort, uint16_t timeout_ms, uint8_t numBytesSent)
{
    uint8_t buf = 0;
    vTaskDelay(1 / portTICK_PERIOD_MS);
    int len = uart_read_bytes(uartPort, &buf, sizeof(buf), timeout_ms / portTICK_PERIOD_MS) ;
    if (len == 0) {
        verbPrintf("No ack received! \r\n");
        return false;
    }
    verbPrintf("Ack Received! Correct: %s\r\n", (buf == numBytesSent) ? "true" : "false");
    verbPrintf("Sent Size: %u      ", numBytesSent);
    verbPrintf("Ack value: %u\r\n", buf);
    return (buf == numBytesSent);
}

bool logging_verbose = true;
