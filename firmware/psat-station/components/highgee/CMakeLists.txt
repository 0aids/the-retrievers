idf_component_register(SRCS "highgee.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver config I2C shared fsm)
