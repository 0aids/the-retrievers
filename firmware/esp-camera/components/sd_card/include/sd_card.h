#include <stdint.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"

#define EXAMPLE_MAX_CHAR_SIZE   (64)
#define MOUNT_POINT             "/sdcard"

#define PIN_CLK     (14)
#define PIN_CMD     (15)
#define PIN_D0      (2)

esp_err_t init_sd_card(void);