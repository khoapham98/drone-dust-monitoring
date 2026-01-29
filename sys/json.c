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

void parseAllDataToJson(ring_buffer_t* rb, float lat, float lng, float alt, uint32_t pm25, float aqi)
{
    char json_buf[256] = {0};

    snprintf(json_buf, sizeof(json_buf),
        "{\"lat\":%f,"
        "\"lng\":%f,"
        "\"alt\":%f,"
        "\"pm2_5\":%d,"
        "\"aqi\":%f}",
        lat, lng, alt, pm25, aqi);

    ring_buffer_queue_arr(rb, json_buf, strlen(json_buf));
}