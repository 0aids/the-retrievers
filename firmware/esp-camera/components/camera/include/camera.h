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

#include "uart.h"


#define portTICK_RATE_MS        portTICK_PERIOD_MS
#define MOUNT_POINT             "/sdcard"



esp_err_t init_camera(void);
void take_pics(void);