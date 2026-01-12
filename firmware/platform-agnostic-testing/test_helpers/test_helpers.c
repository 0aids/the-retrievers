#include "test_helpers.h"

// This file implements helper functions that provide a clean interface
// for test files to manipulate the behavior of the mocked loraImpl.

void testHelpers_forceTxTimeout(bool enabled)
{
    loraImpl_testing_forceTxTimeout = enabled;
}

void testHelpers_forceRxError(bool enabled)
{
    loraImpl_testing_forceRxError = enabled;
}

void testHelpers_setPacketLossPercent(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }
    loraImpl_testing_packetLossPercent = percent;
}

void testHelpers_setTxDelayMs(uint32_t delay_ms)
{
    loraImpl_testing_txDelayMs = delay_ms;
}

void testHelpers_setManglingType(loraImpl_testing_manglingType_e type)
{
    loraImpl_testing_manglingType = type;
}

void testHelpers_dropPacketNumber(uint32_t packet_num_to_drop)
{
    loraImpl_testing_dropPacketNumber = packet_num_to_drop;
}

void testHelpers_resetAllConfigs(void)
{
    loraImpl_testing_forceTxTimeout = false;
    loraImpl_testing_forceRxError = false;
    loraImpl_testing_packetLossPercent = 0;
    loraImpl_testing_txDelayMs = 0;
    loraImpl_testing_dropPacketNumber = 0;
    loraImpl_testing_manglingType = loraImpl_manglingType_none;

    // Reset the internal counter in loraImpl.c
    loraImpl_resetSendCounter();
}
