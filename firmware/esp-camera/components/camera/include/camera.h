#include <stdio.h>
#include "sdkconfig.h"

#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "driver/gpio.h"



#define portTICK_RATE_MS        portTICK_PERIOD_MS
#define MOUNT_POINT             "/sdcard"

#include "driver/uart.h"

#define CAM_UART_NUM            UART_NUM_1 //pin def
#define UART_MESSAGE(message)   (uart_write_bytes(CAM_UART_NUM, message, strlen(message))) //helper


esp_err_t init_camera(void);
void take_pics(void);