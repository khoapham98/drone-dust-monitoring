/**
 * @file    dust_sensor.h
 * @brief   PMS7003 driver header file
 */
#ifndef _PMS7003_H_
#define _PMS7003_H_
#include <stdint.h>

#define DUST_DATA_FRAME     32
#define AQI_LEVEL_COUNT     6

enum aqiLevel{
    AQI_GOOD,
    AQI_MODERATE,
    AQI_SENSITIVE,
    AQI_UNHEALTHY,
    AQI_VERY_UNHEALTHY,
    AQI_HAZARDOUS
};

struct pm25_aqi_ctx{
    int iHigh;
    int iLow;
    float cHigh;
    float cLow;
    float aqi;
    uint16_t pm25;
};

typedef enum aqiLevel eAqiLevel;
typedef struct pm25_aqi_ctx pm25_aqi_ctx_t;

void getDustData(void);

/**
 * @brief   Initialize the UART interface for dust sensor communication
 * @param   uart_file_path is file path of UART
 * @return  0 if success; -1 otherwise
 */
int dustSensor_uart_init(char* uart_file_path);

#endif