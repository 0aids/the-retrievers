#include "ldr_task.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ldr.h"
#include "sm.h"

#define LDR_SAMPLE_PERIOD_MS     25
#define LDR_CONFIRMATION_TIME_MS 150
#define LDR_MAX_PENDING_TIME_MS  2500
#define LDR_LIGHT_THRESHOLD      500

static const char*  TAG       = "LDR_ADC_TASK";
static TaskHandle_t ldrTask_s = NULL;

void                ldr_adcValidationTask(void* arg)
{
    int adcValue           = 0;
    int timeAboveThreshold = 0;
    int totalTime          = 0;

    ldr_setup();

    while (1)
    {
        if (psatFSM_getCurrentState() != psatFSM_state_deployPending)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        ESP_LOGI(
            TAG,
            "Checking the adc to see if we should actually deploy");

        timeAboveThreshold = 0;
        totalTime          = 0;

        while (psatFSM_getCurrentState() ==
               psatFSM_state_deployPending)
        {
            adcValue = ldr_getVoltage();

            ESP_LOGD(TAG, "LDR value: %d", adcValue);

            if (adcValue >= LDR_LIGHT_THRESHOLD)
            {
                timeAboveThreshold += LDR_SAMPLE_PERIOD_MS;
            }
            else
            {
                timeAboveThreshold = 0;
            }

            if (timeAboveThreshold >= LDR_CONFIRMATION_TIME_MS)
            {
                ESP_LOGI(TAG, "LDR confirmation achieved");

                psatFSM_event_t evt = {
                    .type   = psatFSM_eventType_deploymentConfirmed,
                    .global = false};
                psatFSM_postEvent(&evt);
                break;
            }

            totalTime += LDR_SAMPLE_PERIOD_MS;

            if (totalTime >= LDR_MAX_PENDING_TIME_MS)
            {
                ESP_LOGW(TAG, "LDR analog confirmation timed out");

                psatFSM_event_t evt = {
                    .type   = psatFSM_eventType_deploymentTimeout,
                    .global = false};
                psatFSM_postEvent(&evt);
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(LDR_SAMPLE_PERIOD_MS));
        }
    }
}

void ldr_startTask()
{
    ESP_LOGI(TAG, "Starting LDR Task");
    xTaskCreate(ldr_adcValidationTask, "ldr_adc_task", 4096, NULL, 5,
                &ldrTask_s);
}

void ldr_killTask()
{
    ESP_LOGI(TAG, "Killing LDR Task");
    if (ldrTask_s)
    {
        vTaskDelete(ldrTask_s);
        ldrTask_s = NULL;
    }
}