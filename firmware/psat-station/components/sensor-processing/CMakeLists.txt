idf_component_register(SRCS "sensor-processing.c"
                    INCLUDE_DIRS "include"
                    REQUIRES shared bmp280 bmi323 highg I2C)
