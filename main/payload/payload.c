/**
 * @file    payload.c
 * @brief   Implementation of payload builder
 */ 
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "payload.h"

extern MessageBufferHandle_t msgBufHandle;

static const char* TAG = "payload";

void telemetryEnqueueJson(float lat, float lng, float alt, uint32_t pm2_5, float aqi)
{
    char json_buf[JSON_BUFFER_SIZE] = {0};

    snprintf(json_buf, sizeof(json_buf),
        "{\"lat\":%f,"
        "\"lng\":%f,"
        "\"alt\":%f,"
        "\"pm2_5\":%ld,"
        "\"aqi\":%f}",
        lat, lng, alt, pm2_5, aqi);

    size_t bytesSent = xMessageBufferSend(msgBufHandle, 
                            json_buf, strlen(json_buf), 
                            pdMS_TO_TICKS(2000));

    if (bytesSent == 0)
        ESP_LOGW(TAG, "Telemetry JSON dropped: message buffer full");
}