idf_component_register(
    SRCS "battery.c"
    INCLUDE_DIRS "include"
    REQUIRES driver config fsm
)