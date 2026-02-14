/**
 * @file    payload.h
 * @brief   Payload builder for sensor data transmission
 */
#ifndef _PAYLOAD_H_
#define _PAYLOAD_H_

#define JSON_BUFFER_SIZE    128

#define DUST_EVENT_BIT      BIT0
#define GPS_EVENT_BIT       BIT1

void telemetryEnqueueJson(float lat, float lng, float alt, uint32_t pm2_5, float aqi);

#endif