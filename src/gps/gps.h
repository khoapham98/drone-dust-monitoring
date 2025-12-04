/**
 * @file    gps.h
 * @brief   ATGM336H driver header file
 */
#ifndef _GPS_H_
#define _GPS_H_
#include <stdio.h>
#include <stdint.h>

#define     NMEA_FRAME              1024 
#define     LATITUDE_FIELD_NUM      4
#define     LONGTITUDE_FIELD_NUM    6

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
 * @brief   Initialize GPS
 * @param   uart_file_path is file path of UART
 * @return  0 if success; -1 otherwise
 */
int GPS_init(char* uart_file_path);

#endif