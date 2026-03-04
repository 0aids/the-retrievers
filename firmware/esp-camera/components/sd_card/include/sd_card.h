#include <stdint.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"

#include "uart.h"


#define EXAMPLE_MAX_CHAR_SIZE   (64)
#define MOUNT_POINT             "/sdcard"

#define PIN_CLK     (14)
#define PIN_CMD     (15)
#define PIN_D0      (2)
#define PIN_D1      (4)
#define PIN_D2      (12)
#define PIN_D3      (13)


esp_err_t init_sd_card(void);
void deinit_sd_card(void);
void log_data(const char* data);