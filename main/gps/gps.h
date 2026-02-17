/**
 * @file    gps.h
 * @brief   GPS data consumer from Pixhawk via MAVLink
 */
#ifndef _GPS_H_
#define _GPS_H_
#include <stdint.h>

/* Default latitude and longitude values.
 * Here set to the coordinates of Ton Duc Thang University (TDTU), Ho Chi Minh City. 
 */
#define DEFAULT_LATITUDE        10.7318f 
#define DEFAULT_LONGITUDE       106.6981f
#define DEFAULT_ALTITUDE        10

#define HOVER_SPEED_THRESHOLD_CM_S      20
#define HOVER_TIME_REQUIRED_MS          4000       

typedef struct {
    double lat;
    double lon;
    double alt;
} gps_ctx_t;

/**
 * @brief   Check if drone is hovering based on horizontal velocity
 * @return  none
 */
bool isDroneHovering(void);

/**
 * @brief   Read and parse MAVLink messages from GPS
 * @return  none
 */
bool getGpsData(void);

/**
 * @brief   Initialize the UART interface for GPS communication
 * @return  0 if success; -1 otherwise
 */
int gps_init(void);

#endif