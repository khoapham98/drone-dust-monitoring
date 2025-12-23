/**
 * @file    gps.h
 * @brief   ATGM336H driver header file
 */
#ifndef _GPS_H_
#define _GPS_H_
#include <stdint.h>

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

/**
 * @brief   Map coordinates to grid indices and identify location key
 * @param   key Address of string pointer to update with location key (e.g., "us", "hs")
 * @param   row Pointer to store the calculated row index
 * @param   col Pointer to store the calculated column index
 * @param   lat Input latitude
 * @param   lon Input longitude
 * @return  none
 */
void getGridPosition(char** key, int* row, int* col, double lat, double lon); 

/**
 * @brief   Parse NMEA to GPS coordinates
 * @param   buf is NMEA string to parse from
 * @param   latitude is latitude address to store data
 * @param   longitude is longitude address to store data
 * @return  none
 */
void getGpsCoordinates(char* buf, double* latitude, double* longitude);

/**
 * @brief   Read GPS data
 * @param   buf is buffer address to store NMEA string
 * @param   len is buffer length
 * @return  none
 */
void readGpsData(uint8_t* buf, int len);

/**
 * @brief   Initialize the UART interface for GPS communication
 * @param   uart_file_path is file path of UART
 * @return  0 if success; -1 otherwise
 */
int GPS_uart_init(char* uart_file_path);

#endif