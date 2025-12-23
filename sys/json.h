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
 * @brief   Format data to JSON string and store into ring buffer
 * @param   rb Address of ring buffer to store the JSON string
 * @param   key Site identifier string (e.g., "us" or "hs")
 * @param   row Calculated row index
 * @param   col Calculated column index
 * @param   pm2_5 PM2.5 sensor value
 * @return  none
 */
void parseAllDataToJson(ring_buffer_t* rb, char* key, int row, int col, uint16_t pm2_5);

#endif