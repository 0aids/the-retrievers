idf_component_register(
    SRCS "gps_driver.c" "gps_parser.c" "gps_state.c" "minmea.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
)