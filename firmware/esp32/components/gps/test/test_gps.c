#include "../include/gps_driver_esp32.h"
#include "unity.h"

TEST_CASE("checks if we have a gps signal/fix", "[gps]") {
    gps_init();
    gps_start();

    gps_state_t gps_snapshot;
    gps_get_snapshot(&gps_snapshot);

    log_gps_data(&gps_snapshot);
    TEST_ASSERT_TRUE(gps_snapshot.fix_info_valid);
}
