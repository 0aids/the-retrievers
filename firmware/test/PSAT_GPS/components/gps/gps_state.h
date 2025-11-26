
#include <stdbool.h>

void gps_state_init(void);

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
    } sat_info[32];
} gps_state_t;
