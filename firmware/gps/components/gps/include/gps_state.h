#ifndef GPS_STATE_H
#define GPS_STATE_H

#include <stdbool.h>

#include "minmea.h"

#define SATS_TO_STORE 32

typedef struct {
    bool position_valid;
    double latitude;
    double longitude;

    bool nav_valid;
    double speed_knots;
    double speed_kph;  // this was derived not given by gps chip
    double course_deg;

    int day, month, year;
    int hours, minutes, seconds;

    bool fix_info_valid;
    int fix_quality;
    int satellites_tracked;
    double hdop;

    bool altitude_valid;
    double altitude;
    double geoidal_sep;

    int sats_in_view;
    // might honestly remove this later as it takes up a lot of space and i dont
    // think we will care about each indivisual satelite
    struct {
        int nr;
        int elevation;
        int azimuth;
        int snr;
    } sat_info[SATS_TO_STORE];
    int sat_info_count;  // since we get in groups of 4, we need a way to count
} gps_state_t;

void gps_state_init(void);
void gps_get_snapshot(gps_state_t* out);
void gps_state_update_from_rmc(const struct minmea_sentence_rmc* rmc);
void gps_state_update_from_gga(const struct minmea_sentence_gga* gga);
void gps_state_update_from_gsv(const struct minmea_sentence_gsv* gsv);

#endif