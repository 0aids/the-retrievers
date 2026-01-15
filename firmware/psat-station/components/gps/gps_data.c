#include "gps_data.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "shared_state.h"

#define TIMEOUT 20  // max time for the semaphore to lock

static gps_data_t gpsData_s;
static SemaphoreHandle_t gpsStateMutex_s;

void gps_stateInit(void) {
    memset(&gpsData_s, 0, sizeof(gpsData_s));

    // by default set everything being valid to false
    gpsData_s.positionValid = false;
    gpsData_s.navValid = false;
    gpsData_s.fixInfoValid = false;
    gpsData_s.altitudeValid = false;

    if (!gpsStateMutex_s) {
        gpsStateMutex_s = xSemaphoreCreateMutex();
        configASSERT(gpsStateMutex_s);
    }
}

void gps_stateGetSnapshot(gps_data_t* out) {
    if (!out || !gpsStateMutex_s) return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) == pdTRUE) {
        memcpy(out, &gpsData_s, sizeof(gpsData_s));
        xSemaphoreGive(gpsStateMutex_s);
    }
}

void gps_stateCompleteOverwrite(const gps_data_t* src) {
    if (!src || !gpsStateMutex_s) return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) == pdTRUE) {
        memcpy(&gpsData_s, src, sizeof(gpsData_s));
        xSemaphoreGive(gpsStateMutex_s);
    }
}

void gps_logGpsSnapshot(gps_data_t* gps) {
    ESP_LOGI("GPS", "===START OF GPS DATA===");

    if (!gps) {
        ESP_LOGW("GPS", "NULL GPS snapshot");
        return;
    }

    ESP_LOGI("GPS", "Position Data Valid: %d", gps->positionValid);
    ESP_LOGI("GPS", "Coordinates: (%f, %f)", gps->latitude, gps->longitude);
    ESP_LOGI("GPS", "Navigation Data Valid: %d", gps->navValid);
    ESP_LOGI("GPS", "Speed: %.2f kph", gps->speedKph);
    ESP_LOGI("GPS", "Course: %.2f", gps->courseDeg);
    ESP_LOGI("GPS", "Date: %d-%d-%d", gps->day, gps->month, gps->year);
    ESP_LOGI("GPS", "Time: %d:%d:%d", gps->hours, gps->minutes, gps->seconds);
    ESP_LOGI("GPS", "Fix Valid: %d", gps->fixInfoValid);
    ESP_LOGI("GPS", "Fix Quality: %d", gps->fixQuality);
    ESP_LOGI("GPS", "Satellites Tracked: %d", gps->satellitesTracked);
    ESP_LOGI("GPS", "HDOP: %f", gps->hdop);
    ESP_LOGI("GPS", "Altitude Valid: %d", gps->altitudeValid);
    ESP_LOGI("GPS", "Altitude: %f", gps->altitude);
    ESP_LOGI("GPS", "Satellites in View: %d", gps->satsInView);
    ESP_LOGI("GPS", "===END OF GPS DATA===\n\n");
}

// Static functions for internal state use:

static void gps_stateUpdateFromRMC(const struct minmea_sentence_rmc* rmc);
static void gps_stateUpdateFromGGA(const struct minmea_sentence_gga* gga);
static void gps_stateUpdateFromGSV(const struct minmea_sentence_gsv* gsv);

void gps_processLine(const char* gpsBuffer_c) {
    if (!minmea_check(gpsBuffer_c, false)) {
        ESP_LOGW("GPS", "Bad Checksum: %s", gpsBuffer_c);
        return;
    }

    ESP_LOGD("GPS", "Processing Received Line");

    switch (minmea_sentence_id(gpsBuffer_c, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (!minmea_parse_rmc(&frame, gpsBuffer_c)) break;
            if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) !=
                pdTRUE) {
                break;
            }
            gps_stateUpdateFromRMC(&frame);
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (!minmea_parse_gga(&frame, gpsBuffer_c)) break;
            if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) !=
                pdTRUE) {
                break;
            }
            gps_stateUpdateFromGGA(&frame);
        } break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (!minmea_parse_gsv(&frame, gpsBuffer_c)) break;
            if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) !=
                pdTRUE) {
                break;
            }
            gps_stateUpdateFromGSV(&frame);
        } break;

        default:
            return;
    }

    xSemaphoreGive(gpsStateMutex_s);
}

static inline double knotsToKph(double knots) { return knots * 1.852; }

static void gps_stateUpdateFromRMC(const struct minmea_sentence_rmc* rmc) {
    ESP_LOGD("GPS", "Updating State from received RMC string");
    if (!rmc) return;

    if (!rmc->valid) {
        gpsData_s.navValid = false;
        return;
    }

    gpsData_s.latitude = minmea_tocoord(&rmc->latitude);
    gpsData_s.longitude = minmea_tocoord(&rmc->longitude);
    gpsData_s.positionValid = true;

    gpsData_s.speedKnots = minmea_tofloat(&rmc->speed);
    gpsData_s.speedKph = (float)knotsToKph(gpsData_s.speedKnots);
    gpsData_s.courseDeg = minmea_tofloat(&rmc->course);
    gpsData_s.navValid = true;

    gpsData_s.hours = rmc->time.hours;
    gpsData_s.minutes = rmc->time.minutes;
    gpsData_s.seconds = rmc->time.seconds;

    gpsData_s.day = rmc->date.day;
    gpsData_s.month = rmc->date.month;
    gpsData_s.year = 2000 + rmc->date.year;
}

static void gps_stateUpdateFromGGA(const struct minmea_sentence_gga* gga) {
    ESP_LOGD("GPS", "Updating State from received GGA string");
    if (!gga) return;

    gpsData_s.fixQuality = gga->fix_quality;
    gpsData_s.fixInfoValid = (gga->fix_quality > 0);

    // since both gga and rmc give us the coords, only use the gga ones if the
    // rmc one was bad
    if (gga->fix_quality > 0 && !gpsData_s.positionValid) {
        gpsData_s.latitude = minmea_tocoord(&gga->latitude);
        gpsData_s.longitude = minmea_tocoord(&gga->longitude);
        gpsData_s.positionValid = true;
    }

    gpsData_s.satellitesTracked = gga->satellites_tracked;
    gpsData_s.hdop = minmea_tofloat(&gga->hdop);

    gpsData_s.altitude = minmea_tofloat(&gga->altitude);
    gpsData_s.geoidalSep = minmea_tofloat(&gga->height);
    gpsData_s.altitudeValid = true;
}

static void gps_stateUpdateFromGSV(const struct minmea_sentence_gsv* gsv) {
    ESP_LOGD("GPS", "Updating State from received GSV string");
    if (!gsv) return;

    if (gsv->msg_nr == 1) {  // only update on first message and ignore rest
        gpsData_s.satsInView = gsv->total_sats;
    }
}
