#include "gps_driver.h"

#include <stdio.h>

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_config.h"
#include "shared_state.h"
#include "errors.h"

#define TIMEOUT            50
#define GPS_TASK_PRIO      5
#define GPS_LINE_MAX       512
#define GPS_UART_BAUD      115200 // for breadboard gps chip it is 9600
#define UART_READ_CHUNK    128
#define GPS_TASK_STACK     4096
#define GPS_RX_BUFFER_SIZE 2048

static TaskHandle_t gpsTask_s = NULL;
bool                runTask   = true;

static void         gps_task(void* arg)
{
    char gpsBuffer[GPS_LINE_MAX];
    int  gpsBufferPosition = 0;

    while (runTask)
    {
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            printf("\nTASK NOTIFICATION KILLED ME\n");
            break;
        }

        uint8_t data[UART_READ_CHUNK];
        int     len =
            uart_read_bytes(CFG_GPS_UART_NUM_d, data, sizeof(data),
                            TIMEOUT / portTICK_PERIOD_MS);

        if (len == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // go through each byte we just read and add it to the buffer
        for (int i = 0; i < len; i++)
        {
            char c = (char)data[i];
            if (c == '\r')
                continue;

            if (gpsBufferPosition < GPS_LINE_MAX - 1)
                gpsBuffer[gpsBufferPosition++] = c;
            else
            {
                ESP_LOGW("GPS", "The line overflowed so discarded");
                while (i < len && data[i] != '\n')
                    i++;
                gpsBufferPosition = 0;
                continue;
            }

            if (c == '\n')
            {
                // when we reach new line, we know a sentence is
                // complete
                gpsBuffer[gpsBufferPosition] = '\0';

                if (gpsBufferPosition > 0 &&
                    gpsBuffer[gpsBufferPosition - 1] == '\n')
                {
                    gpsBuffer[gpsBufferPosition - 1] = '\0';
                }

                gps_processLine(gpsBuffer);
                ESP_LOGD("GPS", "%s", gpsBuffer);

                gpsBufferPosition = 0; // reset buffer pos to start
            }
        }
    }
    vTaskDelete(NULL);
}

void gps_init()
{
    ESP_LOGI("GPS", "Initialising GPS state");
    gps_stateInit();

    ESP_LOGI("GPS", "Install GPS uart driver");
    uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD,
                                 .data_bits = UART_DATA_8_BITS,
                                 .parity    = UART_PARITY_DISABLE,
                                 .stop_bits = UART_STOP_BITS_1,
                                 .flow_ctrl =
                                     UART_HW_FLOWCTRL_DISABLE};

    
    if(uart_param_config(CFG_GPS_UART_NUM_d, &uart_config) != ESP_OK) {
        psatErr_postError(psatErr_gps_uartConfig_failed ,psatFSM_component_gps , psatFSM_getCurrentState());
        return;
    }
    if(uart_set_pin(CFG_GPS_UART_NUM_d, CFG_GPS_TX_PIN_d,
                                 CFG_GPS_RX_PIN_d, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE) != ESP_OK) {
                                    psatErr_postError(psatErr_gps_uartPinSet_failed ,psatFSM_component_gps , psatFSM_getCurrentState());
                                    return;
                                 }
    if(uart_driver_install(
        CFG_GPS_UART_NUM_d, GPS_RX_BUFFER_SIZE, 0, 0, NULL, 0)!=ESP_OK){
            psatErr_postError(psatErr_gps_uartDriverInstall_failed ,psatFSM_component_gps , psatFSM_getCurrentState());
            return;
        }
}

void gps_deinit()
{
    ESP_LOGI("GPS", "GPS Deinit");
    if(uart_driver_delete(CFG_GPS_UART_NUM_d) != ESP_OK){
        psatErr_postError(psatErr_gps_uartDriverUninstall_failed ,psatFSM_component_gps , psatFSM_getCurrentState());
        return;
    }
}

void gps_startTask()
{
    ESP_LOGI("GPS", "Starting GPS Task");
    runTask = true;
    if(xTaskCreatePinnedToCore(gps_task, "gps_task", GPS_TASK_STACK,
                            NULL, GPS_TASK_PRIO, &gpsTask_s, 0) != pdPASS){
                                psatErr_postError(psatErr_gps_startTask_failed, psatFSM_component_gps , psatFSM_getCurrentState());
                                return;
                            }
}

void gps_killTask()
{
    ESP_LOGI("GPS", "Killing GPS Task");

    if (!gpsTask_s)
        return;

    xTaskNotifyGive(gpsTask_s);
    runTask   = false;
    gpsTask_s = NULL;
}
