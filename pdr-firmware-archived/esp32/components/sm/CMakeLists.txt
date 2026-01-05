idf_component_register(
    SRCS "sm_types.c" "sm.c" "state_handlers.c" "timers.c"
    INCLUDE_DIRS "include"
    REQUIRES gps driver deployment esp_timer audio servo mock_buttons 
)