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
    TEST_ASSERT_EQUAL(expected ,ldr_preflightTest());
}