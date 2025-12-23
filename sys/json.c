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

void parseAllDataToJson(ring_buffer_t* rb, char* key, int row, int col, uint16_t pm2_5)
{
    char json_buf[256] = {0};
    snprintf(json_buf, sizeof(json_buf), 
            "{\"%s\":"
                "{\"%s-%d-%d\":"
                    "{\"sensor_value\":%d}"
                "}"
            "}",
            key, key, col, row, pm2_5);
    ring_buffer_queue_arr(rb, json_buf, strlen(json_buf));
}
