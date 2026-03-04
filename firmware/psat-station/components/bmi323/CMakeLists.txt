idf_component_register(SRCS "bmi323.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver config I2C shared fsm)
