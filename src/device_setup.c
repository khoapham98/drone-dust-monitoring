/**
 * @file    device_setup.c
 * @brief   setup device source file
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "sys/log.h"
#include "sys/ringbuffer.h"
#include "device_setup.h"
#include "src/drivers/uart.h"
#include "src/dust_sensor/dust_sensor.h"
#include "src/gps/gps.h"
#include "sys/json.h"
#include "src/sim/at_cmd.h"
#include "src/sim/sim.h"
#include "src/sim/mqtt.h"
#include "src/sim/mqtt_config.h"

/* thread array for manage */
pthread_t thread[MAX_THREADS];

/* semaphore for dust data and GPS data */
sem_t dustDataDoneSem;
sem_t dustDataReadySem;
sem_t gpsDataDoneSem;
sem_t gpsDataReadySem;
sem_t jsonDataReadySem;

/* dust data */
uint16_t pm2_5 = DEFAULT_PM25;

/* GPS data */
double latitude  = DEFAULT_LATITUDE;
double longitude = DEFAULT_LONGITUDE;

/* json ring buffer */
ring_buffer_t json_ring_buf;
char json_ring_buf_data[RING_BUFFER_SIZE];

void* updateDustDataTask(void* arg)
{
	while (1) {
        sem_wait(&dustDataDoneSem);
        uint8_t dust_buf[DUST_DATA_FRAME] = {0};
	    readDustData(dust_buf, sizeof(dust_buf));
        getPm2_5(dust_buf, &pm2_5);
        sem_post(&dustDataReadySem);
	}

	return arg;
}

void* updateGPSTask(void* arg)
{
	while (1) {
        sem_wait(&gpsDataDoneSem);
        char gps_buf[NMEA_FRAME] = {0};
        readGpsData((uint8_t*) gps_buf, NMEA_FRAME);
        getGpsCoordinates(gps_buf, &latitude, &longitude);
        sem_post(&gpsDataReadySem);
	}

	return arg;
}

void* send2WebTask(void* arg)
{
	while (1) {
        sem_wait(&jsonDataReadySem);
        if (!ring_buffer_is_empty(&json_ring_buf)) {
            char msg_buf[RING_BUFFER_SIZE] = {0};
            getJsonData(&json_ring_buf, msg_buf);

            if (strlen(msg_buf) > 100) {
                LOG_WRN("Data package invalid (%d bytes) - skip", strlen(msg_buf));
            } else {
                mqttPublishMessage(msg_buf, strlen(msg_buf));
                LOG_INF("Message published!");
            }
        }
	}

	return arg;
}

void* dataHandlerTask(void* arg)
{
	while (1) {
#if GPS_ENABLE
        sem_wait(&gpsDataReadySem);        
        double lat = latitude;
        double lon = longitude;
        sem_post(&gpsDataDoneSem);
#else
        double lat = latitude;
        double lon = longitude;
#endif

#if DUST_SENSOR_ENABLE
        sem_wait(&dustDataReadySem);        
        uint16_t pm25 = pm2_5;
        sem_post(&dustDataDoneSem);
#else
        uint16_t pm25 = pm2_5;
#endif

        parseAllDataToJson(&json_ring_buf, lat, lon, pm25);
        LOG_INF("New JSON data pushed: lat = %.6f, lon = %.6f, pm25 = %d", lat, lon, pm25);
        sem_post(&jsonDataReadySem);
	}

	return arg;
}

static int setupDustSensor(void) 
{
    int err = dustSensor_uart_init(UART1_FILE_PATH);    
    if (err != 0)
        return err;

    sem_init(&dustDataReadySem, 0, 0);
    sem_init(&dustDataDoneSem, 0, 1);

    err = pthread_create(&thread[0], NULL, updateDustDataTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d", err);

    return err;
}

static int setupGPS(void) 
{
    int err = GPS_uart_init(UART2_FILE_PATH);    
    if (err != 0)
        return err;

    sem_init(&gpsDataReadySem, 0, 0);
    sem_init(&gpsDataDoneSem, 0, 1);

    err = pthread_create(&thread[1], NULL, updateGPSTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d", err);

    return err;
}

static int setupSim(void) 
{
    int err = sim_init(UART5_FILE_PATH);    
    if (err != 0)
        return err;

    simModuleInitCheck();
    simSetup4G();

    mqttClient client = {
        .index = FIRST,
        .ID    = MQTT_CLIENT_ID,
        .userName = MQTT_USERNAME,
        .password = MQTT_PASSWORD,
        .keepAliveTime = MQTT_KEEPALIVE_600S,
        .cleanSession  = MQTT_PERSIST_SESSION
    };

    mqttServer server = {
        .addr = MQTT_SERVER_ADDR,
        .type = TCP
    };

    mqttPubMsg message = {
        .topic = MQTT_PUB_TOPIC,
        .topicLength = strlen(message.topic),
        .qos = MQTT_QOS_1,
        .publishTimeout = PUBLISH_TIMEOUT_30S
    };

    mqttStartSession();
    mqttClientInit(&client);
    mqttServerInit(&server);
    mqttPublishMessageConfig(&message);
    mqttCreateClient();
    mqttConnectBroker();

    err = pthread_create(&thread[2], NULL, send2WebTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d", err);

    return err;
}

static int setupDataHandler(void) 
{
    int err = pthread_create(&thread[3], NULL, dataHandlerTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d", err);
    else 
        LOG_INF("Data handler setup successfully");

    return err;
}

int deviceSetup(void)
{
    ring_buffer_init(&json_ring_buf, json_ring_buf_data, sizeof(json_ring_buf_data));

    int err = 0;

#if SIM_ENALBE
    err = setupSim();
    if (err != 0)
        LOG_ERR("Failed to setup 4G module");
#endif

#if DUST_SENSOR_ENABLE
    err = setupDustSensor();
    if (err != 0)
        LOG_ERR("Failed to setup dust sensor");
#endif 

#if GPS_ENABLE
    err = setupGPS();
    if (err != 0)
        LOG_ERR("Failed to setup GPS");
#endif

    err = setupDataHandler();
    if (err != 0)
        LOG_ERR("Failed to setup data handler");

    return err;
}
