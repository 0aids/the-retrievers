#include "helpers.h"
#include "driver/uart.h"
#include "uart_config.h"

void blockingTransmitBuffer(uart_port_t uartPort, const uint8_t* const buffer, uint16_t bufferLength)
{
    uart_write_bytes(uartPort, buffer,bufferLength);
    if (
        uart_wait_tx_done(uartPort, d_uartTxTimeout_ms/portTICK_PERIOD_MS) == 
        ESP_ERR_TIMEOUT
    )
    {
        printf("Failed to send!\r\n");
    }
    else printf("Sent successfully! \r\n");
}

bool waitForAck(uart_port_t uartPort, uint16_t timeout_ms, uint8_t numBytesSent)
{
    uint8_t buf = 0;
    int len = uart_read_bytes(uartPort, &buf, sizeof(buf), timeout_ms / portTICK_PERIOD_MS) ;
    if (len == 0) {
        printf("No ack received! \r\n");
        return false;
    }
    printf("Ack Received!\r\n");
    return (buf == numBytesSent);
}
