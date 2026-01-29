/**
 * @file    gps.c
 * @brief   ATGM336H driver source file
 */
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "sys/log.h"
#include "src/gps/gps.h"
#include "src/drivers/uart.h"
#include "ext/mavlink/c_library_v2/common/mavlink.h"

static int uart_fd = 0;

gps_ctx_t gps = {
    .lat = DEFAULT_LATITUDE,
    .lon = DEFAULT_LONGITUDE,
    .alt = DEFAULT_ALTITUDE
};

static bool gpsValid = false;

static mavlink_message_t mav_msg;
static mavlink_status_t  mav_status;

// Debug flag - set to 1 to log all message IDs
#define GPS_DEBUG_MSG_IDS 0

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
                LOG_INF("GPS_RAW_INT: Valid GPS (fix_type: %d, sats: %d)", 
                        gps_raw.fix_type, gps_raw.satellites_visible);
            } 
            
            if (gps_raw.fix_type < 2 || gps_raw.satellites_visible < 4) {
                gpsValid = false;
                LOG_WRN("GPS_RAW_INT: Invalid GPS (fix_type: %d, sats: %d)", 
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
                LOG_INF("GLOBAL_POSITION_INT: lat: %.7f - lon: %.7f - alt: %.2f", gps.lat, gps.lon, gps.alt);
            }

            break;
        }

        default:
#if GPS_DEBUG_MSG_IDS
            LOG_INF("Received MAVLink message ID: %d", msg->msgid);
#endif
            break;
    }
}

void gpsReadMavlink(void)
{
    uint8_t byte;
    int ret;
    int bytes_read = 0;
    int messages_received = 0;

    for (int i = 0; i < 512; i++) {
        ret = read(uart_fd, &byte, 1);

        if (ret == 1) {
            bytes_read++;
            if (mavlink_parse_char(MAVLINK_COMM_0, byte, &mav_msg, &mav_status)) {
                messages_received++;
                gpsHandleMavlinkMsg(&mav_msg);
            }
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;   
            } else if (errno != 0) {
                LOG_ERR("UART read error: %s (errno: %d)", strerror(errno), errno);
                break;
            } else {
                break;
            }
        }
    }

#if GPS_DEBUG_MSG_IDS
    if (bytes_read > 0)
        LOG_INF("Read %d bytes, received %d MAVLink messages", bytes_read, messages_received);
    else if (bytes_read > 0 && messages_received == 0) 
        LOG_WRN("Read %d bytes but no complete MAVLink message received", bytes_read);
#endif
}

int GPS_uart_init(char* uart_file_path)
{
	uart_fd = uart_init(uart_file_path, B57600, true);
    if (uart_fd < 0) {
        return -1;
	}
    
	LOG_INF("GPS Initialization successful");
	return 0;
}
