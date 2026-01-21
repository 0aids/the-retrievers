#include "helpers.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_log.h"
#include <stdlib.h>

/**
 * @brief Print detailed heap usage information
 */
void helpers_printHeapDetails(void)
{
    multi_heap_info_t info;

    ESP_LOGI(__FUNCTION__, "========== Heap Status ==========");

    // Default heap (all capabilities)
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    ESP_LOGI(__FUNCTION__, "DEFAULT HEAP:");
    ESP_LOGI(__FUNCTION__, "  Total free bytes        : %u", info.total_free_bytes);
    ESP_LOGI(__FUNCTION__, "  Minimum free bytes ever : %u", info.minimum_free_bytes);
    ESP_LOGI(__FUNCTION__, "  Largest free block      : %u", info.largest_free_block);
    ESP_LOGI(__FUNCTION__, "  Allocated blocks        : %u", info.allocated_blocks);
    ESP_LOGI(__FUNCTION__, "  Free blocks             : %u", info.free_blocks);
    ESP_LOGI(__FUNCTION__, "  Total allocated bytes   : %u", info.total_allocated_bytes);

    // Internal RAM
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
    ESP_LOGI(__FUNCTION__, "INTERNAL RAM:");
    ESP_LOGI(__FUNCTION__, "  Free bytes              : %u", info.total_free_bytes);
    ESP_LOGI(__FUNCTION__, "  Largest free block      : %u", info.largest_free_block);

    // SPI RAM (if enabled)
#if CONFIG_SPIRAM
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    ESP_LOGI(__FUNCTION__, "SPIRAM:");
    ESP_LOGI(__FUNCTION__, "  Free bytes              : %u", info.total_free_bytes);
    ESP_LOGI(__FUNCTION__, "  Largest free block      : %u", info.largest_free_block);
#endif

    ESP_LOGI(__FUNCTION__, "=================================");
}

bool helpers_malloc(helpers_malloced_t *data, uint32_t bufferSize)
{
    uint8_t* p_ma = malloc(bufferSize);
    if (!p_ma) return false;

    data->buffer = p_ma;
    data->bufferSize = bufferSize;
    return true;
}
bool helpers_realloc(helpers_malloced_t *data, uint32_t bufferSize)
{
    uint8_t* new_ma= realloc(data->buffer, bufferSize);
    if (!new_ma)
    {
        return false;
    }
    data->buffer = new_ma;
    data->bufferSize = bufferSize;
    return true;
}
bool helpers_free(helpers_malloced_t *data)
{
    if (data->buffer)
        free(data->buffer);
    data->buffer = NULL;
    data->bufferSize = 0;
    return true;
}
bool helpers_smartAlloc(helpers_malloced_t *data, uint32_t bufferSize)
{
    if (data->buffer && bufferSize <= data->bufferSize)
        return true;

    if (data->buffer && bufferSize > data->bufferSize)
    {
        return helpers_realloc(data, bufferSize);
    }

    if (!data->buffer)
    {
        return helpers_malloc(data, bufferSize);
    }
    // if you reach here then you should quit coding and go play mc
    // // if you reach here then you should quit coding and go play mc
    // // if you reach here then you should quit coding and go play mc
    // // if you reach here then you should quit coding and go play mc

    return false;
}
