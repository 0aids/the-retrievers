#include "camera.h"
#include "sd_card.h"
#include "uart.h"



void app_main(void){
    init_uart();
    init_sd_card();
    esp_err_t error = init_camera();
    burst_pic();
    esp_camera_deinit();
}