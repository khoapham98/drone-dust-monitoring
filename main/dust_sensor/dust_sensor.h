/**
 * @file    dust_sensor.h
 * @brief   Dust sensor module interface
 */
#ifndef _DUST_SENSOR_H_
#define _DUST_SENSOR_H_
#include <stdint.h>
#include <stdbool.h>

#define DUST_DATA_FRAME     32
#define AQI_LEVEL_COUNT     6

#define START_CHARACTER_1   0x42
#define START_CHARACTER_2   0x4D

#define RMT_BUFFER_SIZE         128
#define DURATION0_PHASE         0
#define DURATION1_PHASE         1

#define RMT_SIGNAL_MIN_1US      1000
#define RMT_SIGNAL_MIN_2US      2000
#define RMT_SIGNAL_MIN_3US      3000

#define RMT_SIGNAL_MAX_10MS     10000000    
#define RMT_SIGNAL_MAX_20MS     20000000    
#define RMT_SIGNAL_MAX_30MS     30000000    
#define RMT_SIGNAL_MAX_50MS     50000000

#define UART_BAUDRATE           9600
#define US_PER_SECOND           1000000
#define UART_BIT_DURATION_US    (US_PER_SECOND / UART_BAUDRATE)

#define UART_DATA_BITS          8
#define UART_START_BIT_COUNT    1

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