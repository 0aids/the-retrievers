#include "../include/gps_driver_esp32.h"
#include "../include/gps_parser.h"
#include "../include/gps_state.h"
#include "unity.h"

TEST_CASE("checks if gps parser works", "[gps]") {
    char* mock1 =
        "$GPGGA,210230,3855.4487,N,09446.0071,W,1,07,1.1,370.5,M,-29.5,M,,*7A";
    gps_process_nmea(mock1);

    char* mock2 =
        "$GPGSV,2,1,08,02,74,042,45,04,18,190,36,07,67,279,42,12,29,323,36*77";
    gps_process_nmea(mock2);

    char* mock3 =
        "$GPRMC,210230,A,3855.4487,N,09446.0071,W,0.0,076.2,130495,003.8,E*69";
    gps_process_nmea(mock3);

    gps_state_t gps_snapshot;
    gps_get_snapshot(&gps_snapshot);

    log_gps_data(&gps_snapshot);

    TEST_ASSERT_FLOAT_WITHIN(0.0001, 38.924145, gps_snapshot.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.0001, -94.766785, gps_snapshot.longitude);
    TEST_ASSERT_TRUE(gps_snapshot.fix_info_valid == 1);
    TEST_ASSERT_TRUE(gps_snapshot.sats_in_view == 8);
}
