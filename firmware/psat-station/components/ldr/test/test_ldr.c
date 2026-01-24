#include "ldr.h"
#include <limits.h>
#include <string.h>
#include "unity.h"

TEST_CASE("ldr unity tests", "[ldr]")
{
    
    ldr_preflightTest_t expected = {
    .stateBefore.stateString    = "",
    .stateMiddle.stateString    = "",
    .sampleData                 = 1000,
    .stateAfter.stateString     = "",
    };
    ldr_preflightTest_t input = {};

    
    ldr_callocTestState(&input);
    ldr_preflightTest(&input);

    // Compares the two strings
    bool matchBefore = (strcmp(expected.stateBefore.stateString, input.stateBefore.stateString) == 0);
    bool matchMiddle = (strcmp(expected.stateMiddle.stateString, input.stateMiddle.stateString) == 0);
    bool matchAfter  = (strcmp(expected.stateAfter.stateString,  input.stateAfter.stateString)  == 0);

    printf("%d\n", input.sampleData);
    if (!matchBefore) printf("State before error:\n %s",  input.stateBefore.stateString);
    if (!matchMiddle) printf("State middle error:\n %s",  input.stateMiddle.stateString);
    if (!matchAfter)  printf("State after error:\n %s",   input.stateAfter.stateString);
    // Free Memory
    ldr_freeTestState(&input);

    TEST_ASSERT_TRUE_MESSAGE(matchBefore, "State Before did not match!");
    TEST_ASSERT_EQUAL_UINT8(expected.sampleData, input.sampleData);
    TEST_ASSERT_TRUE_MESSAGE(matchMiddle, "State Middle did not match!");
    TEST_ASSERT_TRUE_MESSAGE(matchAfter,  "State After did not match!");

}