#include "gps_data.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "shared_state.h"

#define TIMEOUT 20  // max time for the semaphore to lock

static SemaphoreHandle_t gpsStateMutex_s;

void gps_stateInit(void) {
    memset(&psat_globalState.gpsData, 0, sizeof(psat_globalState.gpsData));

    // by default set everything being valid to false
    psat_globalState.gpsData.positionValid = false;
    psat_globalState.gpsData.navValid = false;
    psat_globalState.gpsData.fixInfoValid = false;
    psat_globalState.gpsData.altitudeValid = false;

    if (!gpsStateMutex_s) {
        gpsStateMutex_s = xSemaphoreCreateMutex();
        configASSERT(gpsStateMutex_s);
    }
}

void gps_stateGetSnapshot(gps_data_t* out) {
    if (!out || !gpsStateMutex_s) return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) == pdTRUE) {
        memcpy(out, &psat_globalState.gpsData,
               sizeof(psat_globalState.gpsData));
        xSemaphoreGive(gpsStateMutex_s);
    }
}

void gps_stateCompleteOverwrite(const gps_data_t* src) {
    if (!src || !gpsStateMutex_s) return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) == pdTRUE) {
        memcpy(&psat_globalState.gpsData, src,
               sizeof(psat_globalState.gpsData));
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
        psat_globalState.gpsData.navValid = false;
        return;
    }

    psat_globalState.gpsData.latitude = minmea_tocoord(&rmc->latitude);
    psat_globalState.gpsData.longitude = minmea_tocoord(&rmc->longitude);
    psat_globalState.gpsData.positionValid = true;

    psat_globalState.gpsData.speedKnots = minmea_tofloat(&rmc->speed);
    psat_globalState.gpsData.speedKph =
        (float)knotsToKph(psat_globalState.gpsData.speedKnots);
    psat_globalState.gpsData.courseDeg = minmea_tofloat(&rmc->course);
    psat_globalState.gpsData.navValid = true;

    psat_globalState.gpsData.hours = rmc->time.hours;
    psat_globalState.gpsData.minutes = rmc->time.minutes;
    psat_globalState.gpsData.seconds = rmc->time.seconds;

    psat_globalState.gpsData.day = rmc->date.day;
    psat_globalState.gpsData.month = rmc->date.month;
    psat_globalState.gpsData.year = 2000 + rmc->date.year;
}

static void gps_stateUpdateFromGGA(const struct minmea_sentence_gga* gga) {
    ESP_LOGD("GPS", "Updating State from received GGA string");
    if (!gga) return;

    psat_globalState.gpsData.fixQuality = gga->fix_quality;
    psat_globalState.gpsData.fixInfoValid = (gga->fix_quality > 0);

    // since both gga and rmc give us the coords, only use the gga ones if the
    // rmc one was bad
    if (gga->fix_quality > 0 && !psat_globalState.gpsData.positionValid) {
        psat_globalState.gpsData.latitude = minmea_tocoord(&gga->latitude);
        psat_globalState.gpsData.longitude = minmea_tocoord(&gga->longitude);
        psat_globalState.gpsData.positionValid = true;
    }

    psat_globalState.gpsData.satellitesTracked = gga->satellites_tracked;
    psat_globalState.gpsData.hdop = minmea_tofloat(&gga->hdop);

    psat_globalState.gpsData.altitude = minmea_tofloat(&gga->altitude);
    psat_globalState.gpsData.geoidalSep = minmea_tofloat(&gga->height);
    psat_globalState.gpsData.altitudeValid = true;
}

static void gps_stateUpdateFromGSV(const struct minmea_sentence_gsv* gsv) {
    ESP_LOGD("GPS", "Updating State from received GSV string");
    if (!gsv) return;

    if (gsv->msg_nr == 1) {  // only update on first message and ignore rest
        psat_globalState.gpsData.satsInView = gsv->total_sats;
    }
}
