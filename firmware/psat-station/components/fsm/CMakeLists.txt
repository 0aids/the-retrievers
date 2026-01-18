idf_component_register(
    SRCS "sm.c" "state_handlers.c"
    INCLUDE_DIRS "include"
    REQUIRES ldr gps buttons config timers buzzers servo shared
)