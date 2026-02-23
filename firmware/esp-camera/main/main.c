#include "camera.h"
#include "driver/uart.h"
#include "sd_card.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "uart.h"

void app_main(void){
    uart_init();
    xTaskCreate(uart_task, "uart_echo_task", STACK_SIZE, NULL, 10, NULL);
}