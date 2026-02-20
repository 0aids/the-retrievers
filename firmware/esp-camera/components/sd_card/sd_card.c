#include "sd_card.h"

static const char *TAG = "example";

void init_sd_card(void){
    // Inside your app_main or setup function
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    // CRITICAL: Set usage to 1-bit to free up pins
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1; 
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    
    sdmmc_card_t *card;
    esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
}