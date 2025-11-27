idf_component_register(
    SRCS "gps_driver_esp32.c" "gps_parser.c" "gps_state.c" "minmea.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
)