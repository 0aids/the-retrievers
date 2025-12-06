#include "psat_lora.h"
#include "freertos/FreeRTOS.h"

void psat_radio_init() {
    PsatRadioInit();
}

void psat_radio_task() {
    while (1) {
        PsatRadioMain();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void psat_radio_task_start() {
    xTaskCreate(psat_radio_task, "radio_task", 4096, NULL, 10, NULL);
}