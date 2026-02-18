#include "camera.h"



void app_main(void){
    init_camera();
    take_pic();
    esp_camera_deinit();
}