/**
 * @file    device_setup.c
 * @brief   setup device source file
 */
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "sys/log.h"
#include "sys/ringbuffer.h"
#include "device_setup.h"
#include "src/drivers/uart.h"
#include "src/dust_sensor/dust_sensor.h"
#include "src/gps/gps.h"
#include "sys/json.h"
#include "src/sim/at.h"
#include "transport/mqtt.h"
#include "transport/http.h"
#include "transport/transport_config.h"
#include "fsm/fsm.h"

/* external status flag */
extern bool isHttpFsmRunning;

/* thread array for manage */
pthread_t thread[MAX_THREADS];
int threadCount = 0;

/* semaphore for dust data and GPS data */
sem_t dustDataDoneSem;
sem_t dustDataReadySem;
sem_t gpsDataDoneSem;
sem_t gpsDataReadySem;
sem_t jsonDataReadySem;

/* dust data */
extern pm25_aqi_ctx_t dust;

/* GPS data */
extern gps_ctx_t gps;

/* json ring buffer */
ring_buffer_t json_ring_buf;
char json_ring_buf_data[RING_BUFFER_SIZE];
bool jsonReady = false;
pthread_cond_t jsonCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t jsonLock = PTHREAD_MUTEX_INITIALIZER;

/* web map position and JSON location identifier */
int row = 0;
int column = 0;
char* locationKey = LOCATION_TDTU;

void* updateDustDataTask(void* arg)
{
	while (1) {
        sem_wait(&dustDataDoneSem);
        getDustData();
        sem_post(&dustDataReadySem);
	}

	return arg;
}

void* updateGPSTask(void* arg)
{
	while (1) {
        sem_wait(&gpsDataDoneSem);
        gpsReadMavlink();
        sem_post(&gpsDataReadySem);
        sleep(1);
	}

	return arg;
}

void* send2WebTask(void* arg)
{
    fsm_context_init();

	while (1) {
        fsmHandler();
	}

	return arg;
}

void* dataHandlerTask(void* arg)
{
	while (1) {
#if GPS_ENABLE
        sem_wait(&gpsDataReadySem);        
        double lat = gps.lat;
        double lon = gps.lon;
        double alt = gps.alt;
        sem_post(&gpsDataDoneSem);
#else
        sleep(1);
        double lat = DEFAULT_LATITUDE;
        double lon = DEFAULT_LONGITUDE;
        double alt = DEFAULT_ALTITUDE;
#endif

#if DUST_SENSOR_ENABLE
        sem_wait(&dustDataReadySem);        
        float aqi = dust.aqi;
        uint32_t pm25 = dust.pm25;
        sem_post(&dustDataDoneSem);
#else
        float aqi = 0;
        float pm25 = 0;
#endif

        if (!isReadyToUpload())
            continue;

        pthread_mutex_lock(&jsonLock);
        parseAllDataToJson(&json_ring_buf, lat, lon, alt, pm25, aqi);
        jsonReady = true;
        LOG_INF("New JSON data has been pushed");
        pthread_mutex_unlock(&jsonLock);
        pthread_cond_signal(&jsonCond);
	}

	return arg;
}

static int setupDustSensor(void) 
{
#if BBB
    int err = dustSensor_uart_init(UART1_FILE_PATH);    
#elif RPI
    int err = dustSensor_uart_init(USB0_FILE_PATH);    
#endif

    if (err != 0)
        return err;

    sem_init(&dustDataReadySem, 0, 0);
    sem_init(&dustDataDoneSem, 0, 1);

    err = pthread_create(&thread[threadCount], NULL, updateDustDataTask, NULL);
    if (err != 0) {
        LOG_ERR("pthread_create: %d", err);
        return err;
    }

    threadCount++;
    return 0;
}

static int setupGPS(void) 
{
#if BBB
    int err = GPS_uart_init(UART2_FILE_PATH);    
#elif RPI
    int err = GPS_uart_init(USB1_FILE_PATH);    
#endif

    if (err != 0)
        return err;

    sem_init(&gpsDataReadySem, 0, 0);
    sem_init(&gpsDataDoneSem, 0, 1);

    err = pthread_create(&thread[threadCount], NULL, updateGPSTask, NULL);
    if (err != 0) {
        LOG_ERR("pthread_create: %d", err);
        return err;
    }

    threadCount++;
    return 0;
}

static int setupSim(void) 
{
#if BBB
    int err = sim_uart_init(UART5_FILE_PATH);    
#elif RPI
    int err = sim_uart_init(UART0_FILE_PATH);    
#endif

    if (err != 0)
        return err;

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

    mqttClientInit(&client);
    mqttServerInit(&server);
    mqttPublishMessageConfig(&message);

    err = pthread_create(&thread[threadCount], NULL, send2WebTask, NULL);
    if (err != 0) {
        LOG_ERR("pthread_create: %d", err);
        return err;
    }

    threadCount++;
    return 0;
}

static int setupDataHandler(void) 
{
    int err = pthread_create(&thread[threadCount], NULL, dataHandlerTask, NULL);
    if (err != 0) {
        LOG_ERR("pthread_create: %d", err);
        return err;
    }

    LOG_INF("Data handler setup successfully");
    threadCount++;
    return 0;
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
