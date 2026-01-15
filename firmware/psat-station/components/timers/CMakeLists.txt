idf_component_register(
    SRCS "timers.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer fsm
)