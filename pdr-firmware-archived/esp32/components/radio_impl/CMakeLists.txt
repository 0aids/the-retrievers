file(GLOB radio_impl_c_files "*.c")

idf_component_register(
    SRCS ${radio_impl_c_files}
    INCLUDE_DIRS "include"
    REQUIRES driver sm
)
