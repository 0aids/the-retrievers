idf_component_register(SRCS "packets.c" "loraFsm.c" INCLUDE_DIRS "include"
					REQUIRES shared lora loraImpl esp_timer fsm gps helpers)
