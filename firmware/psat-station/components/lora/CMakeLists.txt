idf_component_register(SRCS "shared_lora.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES loraImpl helpers
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC 
    "lora_hasMalloc_d"
	"lora_defaultBufferSizeMalloc_d=(1<<20)"
)
