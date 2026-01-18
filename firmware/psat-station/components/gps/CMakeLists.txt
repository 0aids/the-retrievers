idf_component_register(
    SRCS "gps_driver.c" "gps_data.c"  "minmea.c"
    INCLUDE_DIRS "include"
    REQUIRES driver config
)