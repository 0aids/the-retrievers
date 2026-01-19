#include "ldr.h"
#include <limits.h>
#include "unity.h"

TEST_CASE("ldr preflight tests", "[ldr]")
{
    ldr_preflightTest_t expected = {
    .stateBefore.stateString = "",
    .stateMiddle.stateString = "",
    .stateAfter.stateString = "",
    };

    ldr_preflightTest_t input = {
    .stateBefore.stateString = "",
    .stateMiddle.stateString = "",
    .stateAfter.stateString = "",
    };
    // input = ldr_preflightTest()
    TEST_ASSERT_EQUAL(expected ,input);
}