#include <stdio.h>
#include "freertos/FreeRTOS.h"

#include "commands.h"

void app_main(void) {
    const int wait_duration = 5000;
    vTaskDelay(pdMS_TO_TICKS(wait_duration));
    printf("Helloo");
}
