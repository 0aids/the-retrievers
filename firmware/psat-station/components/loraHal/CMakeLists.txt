# CMakeLists.txt for loraHal component

idf_component_register(SRCS "loraHal.c"
                    INCLUDE_DIRS "include"
                    # REQUIRES loraCfg
                    PRIV_REQUIRES esp_driver_gpio esp_driver_spi
)
