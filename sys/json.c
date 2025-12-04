/**
 * @file    json.c
 * @brief   JSON source file
 */
#include <stdio.h>
#include <string.h>
#include "sys/ringbuffer.h"
#include "sys/json.h"

void getJsonData(ring_buffer_t* rb, char* buf) 
{
    ring_buffer_size_t ring_buf_size = ring_buffer_num_items(rb);
    ring_buffer_dequeue_arr(rb, buf, ring_buf_size);
}

void parseAllDataToJson(ring_buffer_t* rb, double lat, double lon, uint16_t pm2_5)
{
    char json_buf[256] = {0};
    snprintf(json_buf, sizeof(json_buf), 
            "{"         
            "\"latitude\":%lf,\"longitude\":%lf,"
            "\"altitude_m\":%d,"
            "\"sensor_value\":%u"
            "}", 
            lat, lon, 50, pm2_5);
    ring_buffer_queue_arr(rb, json_buf, strlen(json_buf));
}

void parseGpsDataToJson(ring_buffer_t* rb, double lat, double lon)
{
	char gps_json_buf[128] = {0};
	snprintf(gps_json_buf, sizeof(gps_json_buf),
			"\"latitude\":%lf,\"longitude\":%lf", lat, lon);
	ring_buffer_queue_arr(rb, gps_json_buf, strlen(gps_json_buf));
}

void parseDustDataToJson(ring_buffer_t* rb, uint16_t pm2_5)
{
	char dust_json_buf[64] = {0};
	snprintf(dust_json_buf, sizeof(dust_json_buf),
			"\"sensor_value\":%d", pm2_5);
	ring_buffer_queue_arr(rb, dust_json_buf, strlen(dust_json_buf));
}

void appendLeftBrace(ring_buffer_t* rb)
{
	ring_buffer_queue(rb, '{');	
}

void appendRightBrace(ring_buffer_t* rb)
{
	ring_buffer_queue(rb, '}');	
}