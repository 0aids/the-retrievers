// info about this shit:
// https://tavotech.com/gps-nmea-sentence-structure/
// https://w3.cs.jmu.edu/bernstdh/web/common/help/nmea-sentences.php

#include <stdio.h>

#include "esp_log.h"
#include "minmea.h"

static const char* TAG = "GPS";

void log_nmea(const char* gps_buffer) {
    switch (minmea_sentence_id(gps_buffer, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (minmea_parse_rmc(&frame, gps_buffer)) {
                ESP_LOGI(
                    TAG, "RMC Coordinates (%f, %f), Speed: %f kn = %f kph\n",
                    minmea_tocoord(&frame.latitude),
                    minmea_tocoord(&frame.longitude),
                    minmea_tofloat(&frame.speed),
                    minmea_tofloat(&frame.speed) * 1.852);  // convert to km / h
            }
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, gps_buffer)) {
                ESP_LOGI(TAG, "GGA: fix quality: %d\n", frame.fix_quality);
            }
        } break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (minmea_parse_gsv(&frame, gps_buffer)) {
                char buf[256];
                int pos = 0;

                pos +=
                    snprintf(buf + pos, sizeof(buf) - pos,
                             "GSV: %d satellites in view (msg %d/%d): ",
                             frame.total_sats, frame.msg_nr, frame.total_msgs);

                for (int i = 0; i < 4; i++) {
                    if (frame.sats[i].nr == 0) continue;

                    pos += snprintf(buf + pos, sizeof(buf) - pos,
                                    "[Sat %d: elev %d°, az %d°, snr %d dB] ",
                                    frame.sats[i].nr, frame.sats[i].elevation,
                                    frame.sats[i].azimuth, frame.sats[i].snr);
                }

                ESP_LOGI(TAG, "%s", buf);
            }
        } break;

        default:
            // ESP_LOGI(TAG, "smth else: %s", gps_buffer);
            break;
    }
}