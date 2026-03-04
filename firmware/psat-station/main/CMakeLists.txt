idf_component_register(SRCS "psat-station.c" 
    INCLUDE_DIRS "." 
    REQUIRES fsm timers config driver gps buttons shared helpers sensor-processing bmi323 highg camera
)
