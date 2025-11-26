#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gps_driver.h"
#include "gps_state.h"

static const char* TAG = "MAIN";
static const char* GPS_TAG = "GPS";

void main_task(void* arg) {
    gps_state_t gps;

    while (1) {
        gps_get_snapshot(&gps);

        if (gps.fix_info_valid) {
            // if valid -> lora send the data... bla bla or smth like that

            ESP_LOGI(GPS_TAG, "position data valid: %d", gps.position_valid);
            ESP_LOGI(GPS_TAG, "coords: (%f, %f)", gps.latitude, gps.longitude);

            ESP_LOGI(GPS_TAG, "navigation data valid: %d", gps.nav_valid);
            ESP_LOGI(GPS_TAG, "speed: %.2f kph", gps.speed_kph);
            ESP_LOGI(GPS_TAG, "course: %.2f", gps.course_deg);

            ESP_LOGI(GPS_TAG, "Date: %d-%d-%d", gps.day, gps.month, gps.year);
            ESP_LOGI(GPS_TAG, "Time: %d:%d:%d", gps.hours, gps.minutes,
                     gps.seconds);

            ESP_LOGI(GPS_TAG, "fix valid: %d", gps.fix_info_valid);
            ESP_LOGI(GPS_TAG, "Fix quality: %d", gps.fix_quality);
            ESP_LOGI(GPS_TAG, "sats tracked: %d", gps.satellites_tracked);
            ESP_LOGI(GPS_TAG, "HDOP: %f", gps.hdop);  // idk what this even is

            ESP_LOGI(GPS_TAG, "altitude valkid: %d", gps.altitude_valid);
            ESP_LOGI(GPS_TAG, "Alltitude: %f", gps.altitude);

            ESP_LOGI(GPS_TAG, "sats in view: %d", gps.sats_in_view);
            ESP_LOGI(GPS_TAG, "end\n\n");

        } else {
            ESP_LOGI(TAG, "FIX NOT VALID - NOT CONNECTED");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    gps_init();
    gps_start();

    xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "PSAT has started");
}
