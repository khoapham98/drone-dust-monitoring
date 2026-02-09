/**
 * @file    dust_sensor.h
 * @brief   PMS7003 driver header file
 */
#ifndef _PMS7003_H_
#define _PMS7003_H_
#include <stdint.h>
#include <stdbool.h>

#define DUST_DATA_FRAME     32
#define AQI_LEVEL_COUNT     6

#define START_CHARACTER_1   0x42
#define START_CHARACTER_2   0x4D

#define START_BIT   0
#define STOP_BIT    9

enum aqiLevel {
    AQI_GOOD,
    AQI_MODERATE,
    AQI_SENSITIVE,
    AQI_UNHEALTHY,
    AQI_VERY_UNHEALTHY,
    AQI_HAZARDOUS
};

struct aqi_ctx {
    int iHigh;
    int iLow;
    float cHigh;
    float cLow;
};

struct dust_ctx {
    uint16_t pm1_0;
    uint16_t pm2_5;
    uint16_t pm10;
    float aqi;
};

typedef enum aqiLevel eAqiLevel;
typedef struct aqi_ctx aqi_calc_t;
typedef struct dust_ctx dust_ctx_t;

/**
 * @brief   Initialize the software UART interface for dust sensor communication
 * @return  0 if success; -1 otherwise
 */
int dust_sensor_sw_uart_init(void);

/**
 * @brief   Retrieve dust sensor data.
 * @return  true if valid dust data is successfully received; false otherwise.
 */
bool getDustData(void);

#endif