/**
 * @file    gps.c
 * @brief   GPS module implementation
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "common/mavlink.h"
#include "board.h"
#include "gps.h"

static const char* TAG = "gps";

/* uart1 */
static const uart_config_t uart_cfg = {
    .baud_rate = 57600,
    .data_bits = UART_DATA_8_BITS, 
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

/* gps */
gps_ctx_t gps = {
    .lat = DEFAULT_LATITUDE,
    .lon = DEFAULT_LONGITUDE,
    .alt = DEFAULT_ALTITUDE
};

static int16_t vx_cm_s;
static int16_t vy_cm_s;

static bool gpsValid = false;

/* mavlink */
static mavlink_message_t mav_msg;
static mavlink_status_t  mav_status;

bool isDroneHovering(void)
{
    static uint8_t stableCounter = 0;

    if (!gpsValid) {
        stableCounter = 0;
        return false;
    }
    
    double speed_cm_s = sqrt((double)(vx_cm_s * vx_cm_s) + 
                            (double)(vy_cm_s * vy_cm_s));

    if (speed_cm_s < HOVER_SPEED_THRESHOLD_CM_S) {
        if (stableCounter < HOVER_TIME_REQUIRED_SEC) {
            stableCounter++;
            ESP_LOGD(TAG, "Drone is stable for %d second...", stableCounter);
        }
    } else {
        stableCounter = 0;
        ESP_LOGI(TAG, "Drone is moving...");
    }

    return (stableCounter >= HOVER_TIME_REQUIRED_SEC);
}

static void gpsHandleMavlinkMsg(mavlink_message_t *msg)
{
    switch (msg->msgid)
    {
        case MAVLINK_MSG_ID_GPS_RAW_INT:
        {
            mavlink_gps_raw_int_t gps_raw;
            mavlink_msg_gps_raw_int_decode(msg, &gps_raw);

            if (gps_raw.fix_type >= 2 && gps_raw.satellites_visible >= 5) {
                gpsValid = true;
            } 
            
            if (gps_raw.fix_type < 2 || gps_raw.satellites_visible < 4) {
                gpsValid = false;
                ESP_LOGW(TAG, "GPS_RAW_INT: Invalid GPS (fix_type: %d, sats: %d)", 
                        gps_raw.fix_type, gps_raw.satellites_visible);
            }

            break;
        }

        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            mavlink_global_position_int_t pos;
            mavlink_msg_global_position_int_decode(msg, &pos);

            if (gpsValid) {
                gps.lat = (double) pos.lat / 1e7;
                gps.lon = (double) pos.lon / 1e7;
                gps.alt = (double) pos.relative_alt / 1000.0;
                vx_cm_s = pos.vx;
                vy_cm_s = pos.vy;
            }

            break;
        }

        default:
            break;
    }
}

bool getGpsData(void)
{
    uint8_t byte;
    if (uart_read_bytes(UART_NUM_1, &byte, 1, portMAX_DELAY)) {
        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &mav_msg, &mav_status)) {
            gpsHandleMavlinkMsg(&mav_msg);

            if (mav_msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT && gpsValid) {
                return true;
            }
        }
    }

    return false;
}

int gps_uart_init(void)
{
    esp_err_t ret = uart_param_config(UART_NUM_1, &uart_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Set UART configuration parameters failed");
        return -1;
    }

    ret = uart_set_pin(UART_NUM_1, UART1_TX_PIN, UART1_RX_PIN, -1, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed");
        return -1;
    }

    ret = uart_driver_install(UART_NUM_1, 2048, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Install UART driver failed");
        return -1;
    }

	return 0;
}
