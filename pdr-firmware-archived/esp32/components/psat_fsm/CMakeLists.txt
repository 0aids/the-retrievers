file(GLOB psat_fsm_c_files "*.c")

idf_component_register(
    SRCS ${psat_fsm_c_files}
    INCLUDE_DIRS "include" "include/packets"
    REQUIRES esp_timer gps radio_impl sm servo
)
