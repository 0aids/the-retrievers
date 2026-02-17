idf_component_register(SRCS "psat-station.c" 
    INCLUDE_DIRS "../components/camera/include" 
    REQUIRES fsm timers config driver gps buttons shared helpers camera
)
