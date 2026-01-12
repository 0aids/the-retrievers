#include "test_helpers.h"

// This file implements helper functions that provide a clean interface
// for test files to manipulate the behavior of the mocked loraImpl.

void test_helpers_force_tx_timeout(bool enabled)
{
    loraImpl_testing_force_tx_timeout = enabled;
}

void test_helpers_force_rx_error(bool enabled)
{
    loraImpl_testing_force_rx_error = enabled;
}

void test_helpers_set_packet_loss_percent(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }
    loraImpl_testing_packet_loss_percent = percent;
}

void test_helpers_set_tx_delay_ms(uint32_t delay_ms)
{
    loraImpl_testing_tx_delay_ms = delay_ms;
}

void test_helpers_set_mangling_type(loraImpl_testing_mangling_type_e type)
{
    loraImpl_testing_mangling_type = type;
}

void test_helpers_drop_packet_number(uint32_t packet_num_to_drop)
{
    loraImpl_testing_drop_packet_number = packet_num_to_drop;
}

void test_helpers_reset_all_configs(void)
{
    loraImpl_testing_force_tx_timeout = false;
    loraImpl_testing_force_rx_error = false;
    loraImpl_testing_packet_loss_percent = 0;
    loraImpl_testing_tx_delay_ms = 0;
    loraImpl_testing_drop_packet_number = 0;
    loraImpl_testing_mangling_type = LORAIMPL_MANGLING_NONE;

    // Reset the internal counter in loraImpl.c
    loraImpl_reset_send_counter();
}
