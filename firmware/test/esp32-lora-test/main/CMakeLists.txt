idf_component_register(
    SRCS "esp32-lora-test.c"
    INCLUDE_DIRS "."
    REQUIRES radio_impl driver psat_fsm esp_timer
)
