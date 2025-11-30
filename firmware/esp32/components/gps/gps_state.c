#include "gps_state.h"

#include <string.h>

#define KNOTS_TO_KPH 1.852
#define SATS_PER_MSG 4

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

void gps_get_snapshot(gps_state_t* out) {
    memcpy(out, &gps_state, sizeof(gps_state));
}

void gps_state_update_from_rmc(const struct minmea_sentence_rmc* rmc) {
    if (!rmc) return;

    if (!rmc->valid) {
        gps_state.nav_valid = false;
        return;
    }

    gps_state.latitude = minmea_tocoord(&rmc->latitude);
    gps_state.longitude = minmea_tocoord(&rmc->longitude);
    gps_state.position_valid = true;

    gps_state.speed_knots = minmea_tofloat(&rmc->speed);
    gps_state.speed_kph = gps_state.speed_knots * KNOTS_TO_KPH;
    gps_state.course_deg = minmea_tofloat(&rmc->course);
    gps_state.nav_valid = true;

    gps_state.hours = rmc->time.hours;
    gps_state.minutes = rmc->time.minutes;
    gps_state.seconds = rmc->time.seconds;

    gps_state.day = rmc->date.day;
    gps_state.month = rmc->date.month;
    gps_state.year = rmc->date.year;
}

void gps_state_update_from_gga(const struct minmea_sentence_gga* gga) {
    if (!gga) return;

    gps_state.fix_quality = gga->fix_quality;
    gps_state.fix_info_valid = (gga->fix_quality > 0);

    // since both gga and rmc give us the coords, only use the gga ones if the
    // rmc one was bad
    if (gga->fix_quality > 0 && !gps_state.position_valid) {
        gps_state.latitude = minmea_tocoord(&gga->latitude);
        gps_state.longitude = minmea_tocoord(&gga->longitude);
        gps_state.position_valid = true;
    }

    gps_state.satellites_tracked = gga->satellites_tracked;
    gps_state.hdop = minmea_tofloat(&gga->hdop);

    gps_state.altitude = minmea_tofloat(&gga->altitude);
    gps_state.geoidal_sep = minmea_tofloat(&gga->height);
    gps_state.altitude_valid = true;
}

void gps_state_update_from_gsv(const struct minmea_sentence_gsv* gsv) {
    if (!gsv) return;

    if (gsv->msg_nr == 1) {  // only update on first message and ignore rest
        gps_state.sats_in_view = gsv->total_sats;
    }
}