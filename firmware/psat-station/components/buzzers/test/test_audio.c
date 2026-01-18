#include "../include/buzzer.h"
#include "unity.h"

TEST_CASE("checks if the beep works", "[audio]") {
    buzzer_init();
    buzzer_beep(2500);
    // not really a proper unit test as no assertion and its confirmed by human
    // verification
}
