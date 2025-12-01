#ifndef GPS_STATE_H
#define GPS_STATE_H

#include <stdbool.h>

#include "minmea.h"

#define SATS_TO_STORE 32

typedef struct {
    double latitude;
    double longitude;
    double speed_knots;
    double speed_kph;
    double course_deg;
    double hdop;
    double altitude;
    double geoidal_sep;

    int day, month, year;
    int hours, minutes, seconds;
    int fix_quality;
    int satellites_tracked;
    int sats_in_view;

    bool position_valid;  // lat, long
    bool nav_valid;       // knots, kph & course
    bool fix_info_valid;  // fix quality, sats tracked
    bool altitude_valid;  // altitude, geoidal
} gps_state_t;

void gps_state_init(void);
void gps_get_snapshot(gps_state_t* out);
void gps_state_update_from_rmc(const struct minmea_sentence_rmc* rmc);
void gps_state_update_from_gga(const struct minmea_sentence_gga* gga);
void gps_state_update_from_gsv(const struct minmea_sentence_gsv* gsv);

#endif