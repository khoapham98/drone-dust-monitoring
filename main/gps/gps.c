/**
 * @file    gps.c
 * @brief   Implementation of GPS data consumer
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "common/mavlink.h"
#include "mavlink_manager.h"
#include "gps.h"

static const char* TAG = "gps";

/* gps */
gps_ctx_t gps = {
    .lat = DEFAULT_LATITUDE,
    .lon = DEFAULT_LONGITUDE,
    .alt = DEFAULT_ALTITUDE
};

static int16_t vx_cm_s;
static int16_t vy_cm_s;

static bool isGpsValid = false;

/* queue */
static QueueHandle_t gpsRawIntQueue = NULL;
static QueueHandle_t globalPosIntQueue = NULL;

bool isDroneHovering(void)
{
    static bool isHoverTracking = false;
    static uint64_t hover_start_ms = 0;
    int64_t vx = (int64_t) vx_cm_s;
    int64_t vy = (int64_t) vy_cm_s;

    int64_t speed = (vx * vx) + (vy * vy);
    int64_t threshold = HOVER_SPEED_THRESHOLD_CM_S * HOVER_SPEED_THRESHOLD_CM_S;

    uint64_t now = esp_timer_get_time() / 1000;

    if (speed >= threshold) {
        ESP_LOGI(TAG, "Drone is moving...");
        isHoverTracking = false;
        return false;
    }

    if (!isHoverTracking) {
        hover_start_ms = now; 
        isHoverTracking = true;
        return false;
    }
    
    if (now - hover_start_ms < HOVER_TIME_REQUIRED_MS)
        return false;

    return true;
}

static bool gpsRawIntReceive(void)
{
    mavlink_message_t msg;
    xQueueReceive(gpsRawIntQueue, &msg, portMAX_DELAY);

    mavlink_gps_raw_int_t gps_raw;
    mavlink_msg_gps_raw_int_decode(&msg, &gps_raw);

    if (gps_raw.fix_type >= 2 && gps_raw.satellites_visible >= 5) {
        isGpsValid = true;
    } 
    
    if (gps_raw.fix_type < 2 || gps_raw.satellites_visible < 4) {
        isGpsValid = false;
        ESP_LOGW(TAG, "GPS_RAW_INT: Invalid GPS (fix_type: %d, sats: %d)", 
                gps_raw.fix_type, gps_raw.satellites_visible);
    }

    return isGpsValid;
}

static void globalPositionIntReceive(void) 
{
    mavlink_message_t msg;
    xQueueReceive(globalPosIntQueue, &msg, portMAX_DELAY);

    mavlink_global_position_int_t pos;
    mavlink_msg_global_position_int_decode(&msg, &pos);

    gps.lat = (double) pos.lat / 1e7;
    gps.lon = (double) pos.lon / 1e7;
    gps.alt = (double) pos.relative_alt / 1000.0;
    vx_cm_s = pos.vx;
    vy_cm_s = pos.vy;
}

bool getGpsData(void)
{
    if (!gpsRawIntReceive()) 
        return false;
    
    globalPositionIntReceive();
    
    return true;
}

int gps_init(void)
{    
    gpsRawIntQueue = xQueueCreate(16, sizeof(mavlink_message_t));
    configASSERT(gpsRawIntQueue != 0);

    globalPosIntQueue = xQueueCreate(16, sizeof(mavlink_message_t));
    configASSERT(globalPosIntQueue != 0);

    mavlinkSubscribeMsg(gpsRawIntQueue, MAVLINK_MSG_ID_GPS_RAW_INT);    
    
    mavlinkSubscribeMsg(globalPosIntQueue, MAVLINK_MSG_ID_GLOBAL_POSITION_INT);    

	return 0;
}
