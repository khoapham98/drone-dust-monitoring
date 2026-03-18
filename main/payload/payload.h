/**
 * @file    payload.h
 * @brief   Payload builder for sensor data transmission
 */
#ifndef _PAYLOAD_H_
#define _PAYLOAD_H_

#define JSON_BUFFER_SIZE    128
#define MESSAGE_BUFFER_SIZE 4096

#define DUST_EVENT_BIT      BIT0
#define GPS_EVENT_BIT       BIT1

/**
 * @brief Create a JSON telemetry message and enqueue it into the payload buffer.
 * @param lat Latitude in degrees.
 * @param lng Longitude in degrees.
 * @param alt Altitude in meters.
 * @param pm2_5 PM2.5 value.
 * @param aqi Air Quality Index.
 * @return none.
 */
void telemetryEnqueueJson(float lat, float lng, float alt, uint32_t pm2_5, float aqi);

/**
 * @brief Initialize the payload message buffer used for storing telemetry data before transmission.
 * @return none.
 */
void payload_buffer_init(void);

#endif