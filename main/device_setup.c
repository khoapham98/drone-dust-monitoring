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

extern pm25_aqi_ctx_t ctx;

static const char* TAG = "device_setup";

void dustUpdateTask(void *pvParameters)
{
	while (1) {
        if (getDustData()) {
            ESP_LOGI(TAG, "Dust data received - PM2.5: %d - AQI: %.2f", ctx.pm25, ctx.aqi);
        }
	}
}

static int setupDustSensor(void) 
{
    dust_sensor_sw_uart_init();    

    TaskHandle_t dustTaskHandle;
    BaseType_t ret;

    ret = xTaskCreate(dustUpdateTask, "dust update task", 2056, NULL, 0, &dustTaskHandle);	

    if (ret != pdPASS) {
        vTaskDelete(dustTaskHandle);
        return -1;
    }

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

    return err;
}
