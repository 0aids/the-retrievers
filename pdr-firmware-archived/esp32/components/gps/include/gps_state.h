#ifndef GPS_STATE_H
#define GPS_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "minmea.h"

#define SATS_TO_STORE 32

typedef struct {
    float latitude;
    float longitude;
    float speed_knots;
    float speed_kph;
    float course_deg;
    float hdop;
    float altitude;
    float geoidal_sep;

    int32_t day, month, year;
    int32_t hours, minutes, seconds;
    int32_t fix_quality;
    int32_t satellites_tracked;
    int32_t sats_in_view;

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
