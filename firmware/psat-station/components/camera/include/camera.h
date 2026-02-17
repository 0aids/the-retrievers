#include "sdkconfig.h"

#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "driver/gpio.h"

esp_err_t init_camera(void);
#if defined(CONFIG_CAMERA_AF_SUPPORT) && CONFIG_CAMERA_AF_SUPPORT
void maybe_init_autofocus(void);
#endif