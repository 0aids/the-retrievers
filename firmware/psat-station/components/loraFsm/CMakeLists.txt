idf_component_register(SRCS "./loraFsm.c" "./packets.c"
                 	INCLUDE_DIRS "include"
					REQUIRES shared lora loraImpl esp_timer fsm gps helpers)
