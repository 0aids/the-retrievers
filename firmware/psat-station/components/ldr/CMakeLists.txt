idf_component_register(
    SRCS "ldr_task.c" "ldr.c"
    INCLUDE_DIRS "include"
    REQUIRES driver config fsm
)