#include "../include/audio.h"
#include "../include/audio2.h"
#include "unity.h"

TEST_CASE("checks if the beep works", "[audio]") {
    audio_init();
    audio_beep(2500);
}

TEST_CASE("check if the rickroll works", "[audio]") {
    audio2_init();
    passive_buzzer_play_song();
}
