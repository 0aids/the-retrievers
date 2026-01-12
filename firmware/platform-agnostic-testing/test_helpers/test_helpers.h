#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "loraImpl_testing_config.h"
#include <stdbool.h>
#include <stdint.h>

// --- Test Helper Functions ---
// These functions provide a simple API to configure the loraImpl testing
// module to simulate various edge cases.

// Force the next TX operation to result in a timeout.
void test_helpers_force_tx_timeout(bool enabled);

// Force the next RX operation to result in a generic error.
void test_helpers_force_rx_error(bool enabled);

// Set the percentage chance (0-100) of packet loss for subsequent TX operations.
void test_helpers_set_packet_loss_percent(uint8_t percent);

// Set an artificial delay for subsequent TX operations.
void test_helpers_set_tx_delay_ms(uint32_t delay_ms);

// Set the type of corruption to apply to the next TX operation.
void test_helpers_set_mangling_type(loraImpl_testing_mangling_type_e type);

// Instruct the loraImpl to drop the Nth packet in the sequence.
void test_helpers_drop_packet_number(uint32_t packet_num_to_drop);

// Reset all testing configurations to their default (non-error) state.
// This also resets the internal send counter used for deterministic packet dropping.
void test_helpers_reset_all_configs(void);

// This is declared in loraImpl.c and is needed by the reset function.
void loraImpl_reset_send_counter(void);


#endif // TEST_HELPERS_H
