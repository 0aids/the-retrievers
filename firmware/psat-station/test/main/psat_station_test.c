#include "esp_task_wdt.h"
#include "unity.h"

void app_main(void) {
    esp_task_wdt_deinit();
    unity_run_menu();
}