#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <stdint.h>

typedef struct {
    const char** names;
    const int* pins;
#if ENABLE_ADC_FEATURE
    const int *adc_channels;
#endif
} pin_configuration_t;


#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"
#define EXAMPLE_IS_UHS1    (SDMMC_SPEED_UHS_I_SDR50 || SDMMC_SPEED_UHS_I_DDR50)

#define PIN_CLK     14
#define PIN_CMD     15
#define PIN_D0      2

static const char* names[] = {"CLK", "CMD", "D0"};
static const int pins[] = {PIN_CLK,
                    PIN_CMD,
                    PIN_D0
                    };

static const int pin_count = sizeof(pins)/sizeof(pins[0]);

#if ENABLE_ADC_FEATURE
const int adc_channels[] = {ADC_PIN_CLK,
                            ADC_PIN_CMD,
                            ADC_PIN_D0
                            };
#endif //ENABLE_ADC_FEATURE

static pin_configuration_t config = {
    .names = names,
    .pins = pins,
#if ENABLE_ADC_FEATURE
    .adc_channels = adc_channels,
#endif
};

void init_sd_card(void);