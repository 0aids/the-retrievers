#include "gps_state.h"

// this stores the GPS state privately for internal use inside this folder
static gps_state_t gps_state;

void gps_state_init(void) {
    memset(&gps_state, 0, sizeof(gps_state));

    // set all to invalid by default
    gps_state.position_valid = false;
    gps_state.nav_valid = false;
    gps_state.fix_info_valid = false;
    gps_state.altitude_valid = false;
}