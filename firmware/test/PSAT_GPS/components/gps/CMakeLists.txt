file(GLOB_RECURSE GPS_SOURCES "*.c")

idf_component_register(
    SRCS ${GPS_SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES driver
)
