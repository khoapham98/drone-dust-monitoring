/**
 * @file    gps.h
 * @brief   ATGM336H driver header file
 */
#ifndef _GPS_H_
#define _GPS_H_
#include <stdint.h>

/* Default latitude and longitude values.
 * Here set to the coordinates of Ton Duc Thang University (TDTU), Ho Chi Minh City. 
 */
#define     DEFAULT_LATITUDE        10.7318f 
#define     DEFAULT_LONGITUDE       106.6981f
#define     DEFAULT_ALTITUDE        10

#define     NMEA_FRAME              1024 
#define     LATITUDE_FIELD_NUM      4
#define     LONGTITUDE_FIELD_NUM    6

#define     LOCATION_TDTU           "us"
#define     LOCATION_NHT            "hs"

#define     COORD_LIMITS            2  // 0: Min, 1: Max
#define     HS_GRID_ROWS            7
#define     HS_GRID_COLUMNS         7
#define     US_GRID_ROWS            17
#define     US_GRID_COLUMNS         24

#define     FIRST_IDX               0
#define     HS_LAST_ROW_IDX         6
#define     HS_LAST_COL_IDX         6
#define     US_LAST_ROW_IDX         16
#define     US_LAST_COL_IDX         23

#define     MIN_BOUND   0
#define     MAX_BOUND   1

typedef struct {
    double lat;
    double lon;
    double alt;
} gps_ctx_t;

/**
 * @brief   Get GPS coordinates
 * @param   lat is latitude address to store data
 * @param   lon is longitude address to store data
 * @return  none
 */
void getGpsCoordinates(gps_ctx_t* ctx);

/**
 * @brief   Read and parse MAVLink messages from GPS
 * @return  none
 */
void gpsReadMavlink(void);

/**
 * @brief   Initialize the UART interface for GPS communication
 * @param   uart_file_path is file path of UART
 * @return  0 if success; -1 otherwise
 */
int GPS_uart_init(char* uart_file_path);

#endif