idf_component_register(SRCS "bmp280.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver config I2C shared)
