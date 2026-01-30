idf_component_register(SRCS "BMP280.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver config I2C shared)
