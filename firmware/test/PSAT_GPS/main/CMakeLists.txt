idf_component_register(
    SRCS "main.c" "lib/gps.c" "lib/minmea.c"
    INCLUDE_DIRS "." "lib"
    REQUIRES driver
)