#include "sd_card.h"

sdmmc_card_t *card;

esp_err_t init_sd_card(void){
    // Inside your app_main or setup function
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4; 
    slot_config.clk = PIN_CLK;
    slot_config.cmd = PIN_CMD;
    slot_config.d0 = PIN_D0;
    slot_config.d1 = PIN_D1;
    slot_config.d2 = PIN_D2;
    slot_config.d3 = PIN_D3;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    
    return esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    
}

void deinit_sd_card(void){
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
}