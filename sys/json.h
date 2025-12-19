/**
 * @file    json.h
 * @brief   JSON header file
 */
#ifndef _JSON_H_
#define _JSON_H_
#include "sys/ringbuffer.h"

/**
 * @brief   Get JSON data from ring buffer 
 * @param   rb is ring buffer address to get data from
 * @param   buf is buffer address to store data
 * @return  none
 */
void getJsonData(ring_buffer_t* rb, char* buf); 

/**
 * @brief   Parse and store dust data to JSON ring buffer
 * @param   rb is ring buffer address to put data in 
 * @param   lat is latitude
 * @param   lon is longitude
 * @param   pm2_5 is PM2.5 value
 * @return  none
 */
void parseAllDataToJson(ring_buffer_t* rb, double lat, double lon, uint16_t pm2_5);

/**
 * @brief   Parse and store dust data to JSON ring buffer
 * @param   rb is ring buffer address to put data in 
 * @param   pm2_5 is PM2.5 value
 * @return  none
 */
void parseDustDataToJson(ring_buffer_t* rb, uint16_t pm2_5);

/**
 * @brief   Parse and store gps data to JSON ring buffer
 * @param   rb is ring buffer address to put data in
 * @param   lat is latitude
 * @param   lon is longitude
 * @return  none
 */
void parseGpsDataToJson(ring_buffer_t* rb, double lat, double lon);

/**
 * @brief   add left brace to JSON ring buffer
 * @param   rb is ring buffer address to put data in 
 * @return  none
 */
void appendLeftBrace(ring_buffer_t* rb);

/**
 * @brief   add right brace to JSON ring buffer
 * @param   rb is ring buffer address to put data in 
 * @return  none
 */
void appendRightBrace(ring_buffer_t* rb);

#endif