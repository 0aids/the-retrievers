idf_component_register(SRCS "./loraImpl.c"
                    INCLUDE_DIRS "include"
                    REQUIRES utilsImpl loraHal loraCfg
                    PRIV_REQUIRES esp_driver_spi esp_driver_gpio esp_timer
)
