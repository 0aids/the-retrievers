#include "gps_data.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "shared_state.h"

#define TEAM_ID 1
#define TIMEOUT 20 // max time for the semaphore to lock

static gps_data_t                gpsData_s;
static gps_psatTelemetryPacket_t psatTelemetryPacket_s;
static SemaphoreHandle_t         gpsStateMutex_s;

int*               gps_linesRecieved = &gpsData_s.linesRecieved;

static const char* TAG = "GPS";

void               gps_stateInit(void)
{
    memset(&gpsData_s, 0, sizeof(gpsData_s));
    memset(&psatTelemetryPacket_s, 0, sizeof(psatTelemetryPacket_s));

    // by default set everything being valid to false
    gpsData_s.positionValid = false;
    gpsData_s.navValid      = false;
    gpsData_s.fixInfoValid  = false;
    gpsData_s.altitudeValid = false;

    if (!gpsStateMutex_s)
    {
        gpsStateMutex_s = xSemaphoreCreateMutex();
        configASSERT(gpsStateMutex_s);
    }
}

void gps_stateGetSnapshot(gps_data_t* out)
{
    if (!out || !gpsStateMutex_s)
        return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) ==
        pdTRUE)
    {
        memcpy(out, &gpsData_s, sizeof(gpsData_s));
        xSemaphoreGive(gpsStateMutex_s);
    }
}

void gps_telemtryPacketGetSnapshot(gps_psatTelemetryPacket_t* out)
{
    if (!out || !gpsStateMutex_s)
        return;

    if (xSemaphoreTake(gpsStateMutex_s, pdMS_TO_TICKS(TIMEOUT)) ==
        pdTRUE)
    {
        memcpy(out, &psatTelemetryPacket_s,
               sizeof(psatTelemetryPacket_s));
        xSemaphoreGive(gpsStateMutex_s);
    }
}

#define BOOL_TO_STRING(expr) ((expr) ? "true" : "false")
void gps_logGpsSnapshot(gps_data_t* gps)
{
    static const gps_data_t zeroStruct;

    if (!gps)
    {
        ESP_LOGW(TAG, "NULL GPS snapshot");
        return;
    }

    if (memcmp(gps, &zeroStruct, sizeof(*gps)) == 0)
    {

        ESP_LOGW(TAG, "GPS struct all zeros (no data received yet)");
        return;
    }

    ESP_LOGI(TAG, "===START OF GPS DATA===");

    ESP_LOGI(TAG, "Position Data Valid: %s",
             BOOL_TO_STRING(gps->positionValid));
    ESP_LOGI(TAG, "Coordinates: (%f, %f)", gps->latitude,
             gps->longitude);

    ESP_LOGI(TAG, "Navigation Data Valid: %s",
             BOOL_TO_STRING(gps->navValid));
    ESP_LOGI(TAG, "Speed: %.2f kph", gps->speedKph);
    ESP_LOGI(TAG, "Course: %.2f", gps->courseDeg);
    ESP_LOGI(TAG, "Date: %02d-%02d-%04d", gps->day, gps->month,
             gps->year);
    ESP_LOGI(TAG, "Time: %02d:%02d:%02d", gps->hours, gps->minutes,
             gps->seconds);

    ESP_LOGI(TAG, "Fix Valid: %s", BOOL_TO_STRING(gps->fixInfoValid));
    ESP_LOGI(TAG, "Fix Quality: %d", gps->fixQuality);
    ESP_LOGI(TAG, "Satellites Tracked: %d", gps->satellitesTracked);
    ESP_LOGI(TAG, "HDOP: %f", gps->hdop);

    ESP_LOGI(TAG, "Altitude Valid: %s",
             BOOL_TO_STRING(gps->altitudeValid));
    ESP_LOGI(TAG, "Altitude: %f", gps->altitude);
    ESP_LOGI(TAG, "Satellites in View: %d", gps->satsInView);

    ESP_LOGI(TAG, "===END OF GPS DATA===\n\n");
}

// Forward declarations of static functions for internal state use:
static void
gps_stateUpdateFromRMC(const struct minmea_sentence_rmc* rmc);
static void
gps_stateUpdateFromGGA(const struct minmea_sentence_gga* gga);
static void
gps_stateUpdateFromGSV(const struct minmea_sentence_gsv* gsv);
static void
     gps_encodeBinaryFromGGA(uint8_t                           out[14],
                             const struct minmea_sentence_gga* gga);

void gps_processLine(const char* gpsBuffer_c)
{
    if (!minmea_check(gpsBuffer_c, false))
    {
        ESP_LOGW(TAG, "Bad Checksum: %s", gpsBuffer_c);
        return;
    }

    //increasing lines read for preflight test
    gpsData_s.linesRecieved++;

    ESP_LOGD(TAG, "Processing Received Line");

    switch (minmea_sentence_id(gpsBuffer_c, false))
    {
        case MINMEA_SENTENCE_RMC:
        {
            struct minmea_sentence_rmc frame;
            if (!minmea_parse_rmc(&frame, gpsBuffer_c))
                break;
            if (xSemaphoreTake(gpsStateMutex_s,
                               pdMS_TO_TICKS(TIMEOUT)) != pdTRUE)
            {
                break;
            }
            gps_stateUpdateFromRMC(&frame);
        }
        break;

        case MINMEA_SENTENCE_GGA:
        {
            struct minmea_sentence_gga frame;
            if (!minmea_parse_gga(&frame, gpsBuffer_c))
                break;
            if (xSemaphoreTake(gpsStateMutex_s,
                               pdMS_TO_TICKS(TIMEOUT)) != pdTRUE)
            {
                break;
            }
            gps_stateUpdateFromGGA(&frame);
        }
        break;

        case MINMEA_SENTENCE_GSV:
        {
            struct minmea_sentence_gsv frame;
            if (!minmea_parse_gsv(&frame, gpsBuffer_c))
                break;
            if (xSemaphoreTake(gpsStateMutex_s,
                               pdMS_TO_TICKS(TIMEOUT)) != pdTRUE)
            {
                break;
            }
            gps_stateUpdateFromGSV(&frame);
        }
        break;

        default: return;
    }

    xSemaphoreGive(gpsStateMutex_s);
}

static inline double knotsToKph(double knots)
{
    return knots * 1.852;
}

static void
gps_stateUpdateFromRMC(const struct minmea_sentence_rmc* rmc)
{
    ESP_LOGD(TAG, "Updating State from received RMC string");
    if (!rmc)
        return;

    if (!rmc->valid)
    {
        gpsData_s.navValid = false;
        return;
    }

    // Main GPS Struct
    gpsData_s.latitude      = minmea_tocoord(&rmc->latitude);
    gpsData_s.longitude     = minmea_tocoord(&rmc->longitude);
    gpsData_s.positionValid = true;

    gpsData_s.speedKnots = minmea_tofloat(&rmc->speed);
    gpsData_s.speedKph   = (float)knotsToKph(gpsData_s.speedKnots);
    gpsData_s.courseDeg  = minmea_tofloat(&rmc->course);
    gpsData_s.navValid   = true;

    gpsData_s.hours   = rmc->time.hours;
    gpsData_s.minutes = rmc->time.minutes;
    gpsData_s.seconds = rmc->time.seconds;

    gpsData_s.day   = rmc->date.day;
    gpsData_s.month = rmc->date.month;
    gpsData_s.year  = 2000 + rmc->date.year;
}

static void
gps_stateUpdateFromGGA(const struct minmea_sentence_gga* gga)
{
    ESP_LOGD(TAG, "Updating State from received GGA string");
    if (!gga)
        return;

    gpsData_s.fixQuality   = gga->fix_quality;
    gpsData_s.fixInfoValid = (gga->fix_quality > 0);

    // since both gga and rmc give us the coords, only use the gga ones if the
    // rmc one was bad
    if (gga->fix_quality > 0 && !gpsData_s.positionValid)
    {
        gpsData_s.latitude      = minmea_tocoord(&gga->latitude);
        gpsData_s.longitude     = minmea_tocoord(&gga->longitude);
        gpsData_s.positionValid = true;
    }

    gpsData_s.satellitesTracked = gga->satellites_tracked;
    gpsData_s.hdop              = minmea_tofloat(&gga->hdop);

    gpsData_s.altitude      = minmea_tofloat(&gga->altitude);
    gpsData_s.geoidalSep    = minmea_tofloat(&gga->height);
    gpsData_s.altitudeValid = true;

    gps_encodeBinaryFromGGA(psatTelemetryPacket_s.data, gga);
}

static void
gps_stateUpdateFromGSV(const struct minmea_sentence_gsv* gsv)
{
    ESP_LOGD(TAG, "Updating State from received GSV string");
    if (!gsv)
        return;

    if (gsv->msg_nr == 1)
    { // only update on first message and ignore rest
        gpsData_s.satsInView = gsv->total_sats;
    }
}

static void
gps_encodeBinaryFromGGA(uint8_t                           out[14],
                        const struct minmea_sentence_gga* gga)
{
    memset(out, 0, 14);

    uint8_t teamId = TEAM_ID & 0x1F;

    uint8_t hours   = gga->time.hours & 0x1F;
    uint8_t minutes = gga->time.minutes & 0x3F;
    uint8_t seconds = gga->time.seconds & 0x3F;

    out[0] = (teamId << 3) | (hours >> 2);

    out[1] = ((hours & 0x3) << 6) | minutes;

    uint8_t fixOk  = (gga->fix_quality > 0) ? 1 : 0;
    uint8_t difFix = (gga->fix_quality == 2) ? 1 : 0;
    out[2]         = (seconds << 2) | (fixOk << 1) | difFix;

    uint8_t numSats = gga->satellites_tracked & 0x0F;
    int32_t altitudeDm =
        (int32_t)(minmea_tofloat(&gga->altitude) * 10.0f);
    out[3] = (numSats << 4) | ((altitudeDm >> 16) & 0x0F);

    out[4] = (altitudeDm >> 8) & 0xFF;
    out[5] = altitudeDm & 0xFF;

    int32_t lat1e7 =
        (int32_t)(minmea_tocoord(&gga->latitude) * 10000000.0);

    int32_t lon1e7 =
        (int32_t)(minmea_tocoord(&gga->longitude) * 10000000.0);

    out[6] = (lat1e7 >> 24) & 0xFF;
    out[7] = (lat1e7 >> 16) & 0xFF;
    out[8] = (lat1e7 >> 8) & 0xFF;
    out[9] = lat1e7 & 0xFF;

    out[10] = (lon1e7 >> 24) & 0xFF;
    out[11] = (lon1e7 >> 16) & 0xFF;
    out[12] = (lon1e7 >> 8) & 0xFF;
    out[13] = lon1e7 & 0xFF;
}