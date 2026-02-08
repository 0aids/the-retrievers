#pragma once
#include <stdbool.h>
#include <stdint.h>
// Convention:
// _oma suffix - Owning Malloc Pointer -- if you receive this that means you now own the pointer.
// _nma suffix - Non-owning malloc pointer -- if you receive this you do not own the pointer.

/**
 * @brief Print detailed heap usage information
 */
void helpers_printHeapDetails(void);

typedef struct
{
    uint32_t bufferSize;
    uint8_t* buffer;
} helpers_malloced_t;

// Malloc, returns false on failure. On failure, *data is not modified
bool helpers_malloc(helpers_malloced_t* data, uint32_t bufferSize);
// Realloc, returns false on failure. On failure, *data is not modified
bool helpers_realloc(helpers_malloced_t* data, uint32_t bufferSize);
// Free, returns false on failure. On failure, *data is not modified
bool helpers_free(helpers_malloced_t* data);
// Mallocs if NULL, reallocs if not.
// Does not realloc if the size is already enough.
// returns false on failure.
// On failure, *data is not modified
bool helpers_smartAlloc(helpers_malloced_t* data,
                        uint32_t            bufferSize);
// Frees if not NULL, otherwise not. returns false on failure. On failure, *data is not modified
bool helpers_smartFree(helpers_malloced_t* data);

// Mallocs a new spot, moves all the data, and then frees the previous.
// On failure returns false and doesn't modify anything.
bool helpers_smartMoveRealloc(helpers_malloced_t* data,
                              uint32_t            newBufferSize);
