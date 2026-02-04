/**
 * @file    device_setup.c
 * @brief   setup device source file
 */
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "device_setup.h"
#include "dust_sensor.h"
#include "gps.h"

static const char* TAG = "device_setup";

/* dust */
extern pm25_aqi_ctx_t ctx;

/* gps */
extern gps_ctx_t gps;

void dustUpdateTask(void *pvParameters)
{
	while (1) {
        if (getDustData()) {
            ESP_LOGI(TAG, "Dust data received: PM2.5: %d - AQI: %.2f",
                    ctx.pm25, ctx.aqi);
        }
	}
}

void gpsUpdateTask(void *pvParameters)
{
	while (1) {
        if (getGpsData()) {
            ESP_LOGI(TAG, "GPS data received: lat: %.7f - lon: %.7f - alt: %.2f m",
                    gps.lat, gps.lon, gps.alt);
        }
	}
}

static int setupDustSensor(void) 
{
    dust_sensor_sw_uart_init();    

    TaskHandle_t dustTaskHandle;
    BaseType_t ret = xTaskCreate(dustUpdateTask, "dust update task", 2048, NULL, 0, &dustTaskHandle);	

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Dust task!");
        return -1;
    }

    ESP_LOGI(TAG, "Dust task created");
    return 0;
}

static int setupGPS(void) 
{
    gps_uart_init();

    TaskHandle_t gpsTaskHandle;
    BaseType_t ret = xTaskCreate(gpsUpdateTask, "gps update task", 4096, NULL, 0, &gpsTaskHandle);	

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create GPS task!");
        return -1;
    }

    ESP_LOGI(TAG, "GPS task created");
    return 0;
}

int deviceSetup(void)
{
    int err = 0;

#if DUST_SENSOR_ENABLE
    err = setupDustSensor();
    if (err != 0)
        ESP_LOGE(TAG, "Failed to setup dust sensor");
#endif 

#if GPS_ENABLE
    err = setupGPS();
    if (err != 0)
        ESP_LOGE(TAG, "Failed to setup gps");
#endif 

    return err;
}
