#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "loraImpl_testing_config.h"
#include <stdbool.h>
#include <stdint.h>

// --- Test Helper Functions ---
// These functions provide a simple API to configure the loraImpl testing
// module to simulate various edge cases.

// Force the next TX operation to result in a timeout.
void testHelpers_forceTxTimeout(bool enabled);

// Force the next RX operation to result in a generic error.
void testHelpers_forceRxError(bool enabled);

// Set the percentage chance (0-100) of packet loss for subsequent TX operations.
void testHelpers_setPacketLossPercent(uint8_t percent);

// Set an artificial delay for subsequent TX operations.
void testHelpers_setTxDelayMs(uint32_t delay_ms);

// Set the type of corruption to apply to the next TX operation.
void testHelpers_setManglingType(loraImpl_testing_manglingType_e type);

// Instruct the loraImpl to drop the Nth packet in the sequence.
void testHelpers_dropPacketNumber(uint32_t packet_num_to_drop);

// Reset all testing configurations to their default (non-error) state.
// This also resets the internal send counter used for deterministic packet dropping.
void testHelpers_resetAllConfigs(void);

// This is declared in loraImpl.c and is needed by the reset function.
void loraImpl_resetSendCounter(void);


#endif // TEST_HELPERS_H
