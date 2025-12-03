#include <stdio.h>
#include "driver/uart.h"
#include "esp_err.h"
#include "global_radio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_config.h"
#define X(name) case grReq_##name: return "grReq_" #name;
const char* grRequestToString(e_grRequest req) {
    switch (req) {
        grReq_XMacro
        default: return "Unknown_grRequest";
    }
}
#undef X

void app_main(void)
{
    gr_RadioInit();

    while (1) {
        printf("Sending hello!\r\n");
        uart_write_bytes(d_gr_uartPort, "hello", 5);
        if (
            uart_wait_tx_done(d_gr_uartPort, 500/portTICK_PERIOD_MS) == 
            ESP_ERR_TIMEOUT
        )
        {
            printf("Failed to send!\r\n");
        }
        else printf("Sent successfully! \r\n");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
