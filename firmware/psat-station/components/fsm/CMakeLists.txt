idf_component_register(
    SRCS "states.c" "components.c" "errors.c" "sm.c" "state_handlers.c" "on_entry_handlers.c" "on_exit_handlers.c"
    INCLUDE_DIRS "include"
    REQUIRES ldr gps buttons config timers buzzers servo shared loraFsm
)