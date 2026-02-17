/**
 * @file    device_setup.c
 * @brief   setup device source file
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "device_setup.h"
#include "dust_sensor.h"
#include "gps.h"
#include "at.h"
#include "fsm_manager.h"
#include "payload.h"
#include "transport_config.h"
#include "mavlink_manager.h"

static const char* TAG = "device_setup";

/* dust */
TaskHandle_t dustTaskHandle = NULL;
extern dust_ctx_t dust;

/* gps */
TaskHandle_t gpsTaskHandle = NULL;
extern gps_ctx_t gps;

/* modem */
TaskHandle_t simTaskHandle = NULL;

/* data sync */
TaskHandle_t dataSyncTaskHandle = NULL;
EventGroupHandle_t eventGroupHandle = NULL;

static void simManagerTask(void *pvParameters)
{
    fsm_context_init();

	while (1) {
        fsmHandler();
	}
}

static void dustUpdateTask(void *pvParameters)
{
	while (1) {
        if (getDustData()) {
            ESP_LOGD(TAG, "Dust data received: PM2.5: %d - AQI: %.2f",
                    dust.pm2_5, dust.aqi);
            
            xEventGroupSetBits(eventGroupHandle, DUST_EVENT_BIT);
        }
	}
}

static void gpsUpdateTask(void *pvParameters)
{
	while (1) {
        if (getGpsData()) {
            ESP_LOGD(TAG, "GPS data received: lat: %.7f - lon: %.7f - alt: %.2f m",
                    gps.lat, gps.lon, gps.alt);

            xEventGroupSetBits(eventGroupHandle, GPS_EVENT_BIT);
        }
	}
}

static void dataSyncTask(void *pvParameters)
{
    BaseType_t waitForAllBits = DUST_SENSOR_ENABLE && GPS_ENABLE;

	while (1) {
#if DUST_SENSOR_ENABLE || GPS_ENABLE
        xEventGroupWaitBits(
            eventGroupHandle, 
            DUST_EVENT_BIT | GPS_EVENT_BIT,
            pdTRUE, waitForAllBits, portMAX_DELAY);
#endif

#if !DUST_SENSOR_ENABLE
        dust.pm2_5 = 30;
        dust.aqi = 50;
#endif

#if !GPS_ENABLE
        gps.lat = DEFAULT_LATITUDE;
        gps.lon = DEFAULT_LONGITUDE;
        gps.alt = DEFAULT_ALTITUDE;
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
 
        telemetryEnqueueJson(gps.lat, gps.lon, gps.alt, dust.pm2_5, dust.aqi);
	}
}

static int setupSim(void) 
{
    sim_uart_init();    

    mqttClient client = {
        .index = FIRST,
        .ID    = MQTT_CLIENT_ID,
        .userName = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .keepAliveTime = MQTT_KEEPALIVE_600S,
        .cleanSession  = MQTT_PERSIST_SESSION
    };

    mqttServer server = {
        .type = TCP,
        .addr = MQTT_SERVER_ADDR
    };

    mqttPubMsg message = {
        .qos   = MQTT_QOS_1,
        .topic = MQTT_PUB_TOPIC,
        .topicLength = strlen(message.topic),
        .publishTimeout = PUBLISH_TIMEOUT_30S
    };

    mqtt_context_init(&client, &server, &message);

    BaseType_t ret = xTaskCreate(simManagerTask, "sim manager task", 4096, NULL, 0, &simTaskHandle);	
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Sim task!");
        return -1;
    }

    ESP_LOGI(TAG, "Sim task created");
    return 0;
}

static int setupDustSensor(void) 
{
    dust_sensor_sw_uart_init();    

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
    gps_init();

    BaseType_t ret = xTaskCreate(gpsUpdateTask, "gps update task", 4096, NULL, 0, &gpsTaskHandle);	
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create GPS task!");
        return -1;
    }

    ESP_LOGI(TAG, "GPS task created");
    return 0;
}

static int setupSyncTask(void) 
{
    eventGroupHandle = xEventGroupCreate();
    if (eventGroupHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return -1;
    }
    
    BaseType_t ret = xTaskCreate(dataSyncTask, "data sync task", 2048, NULL, 0, &dataSyncTaskHandle);	
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Data sync task!");
        return -1;
    }

    ESP_LOGI(TAG, "Data sync task created");
    return 0;
}

int deviceSetup(void)
{
    int err = setupSyncTask();
    if (err != 0)
        ESP_LOGE(TAG, "Failed to setup data sync");

#if SIM_ENABLE
    err = setupSim();
    if (err != 0)
        ESP_LOGE(TAG, "Failed to setup sim");
#endif 

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

    err = setupMavlinkManager();
    if (err != 0)
        ESP_LOGE(TAG, "Failed to setup MAVLink manager");

    return err;
}
