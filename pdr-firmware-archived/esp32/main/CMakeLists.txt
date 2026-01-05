idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES gps deployment sm driver lora_task
)