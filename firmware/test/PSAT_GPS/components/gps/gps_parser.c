
#include <stdio.h>

// #include "esp_log.h"
#include "gps_state.h"
#include "minmea.h"

// static const char* TAG = "GPS";

// more info about this shit:
// https://tavotech.com/gps-nmea-sentence-structure/
// https://w3.cs.jmu.edu/bernstdh/web/common/help/nmea-sentences.php

void gps_process_nmea(const char* gps_buffer) {
    switch (minmea_sentence_id(gps_buffer, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (!minmea_parse_rmc(&frame, gps_buffer)) break;
            gps_state_update_from_rmc(&frame);
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (!minmea_parse_gga(&frame, gps_buffer)) break;
            gps_state_update_from_gga(&frame);
        } break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (!minmea_parse_gsv(&frame, gps_buffer)) break;
            gps_state_update_from_gsv(&frame);
        } break;

        default:
            // ESP_LOGI(TAG, "smth else: %s", gps_buffer);
            break;
    }
}
