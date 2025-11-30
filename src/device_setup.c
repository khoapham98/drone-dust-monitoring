/**
 * @file    device_setup.c
 * @brief   setup device source file
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "sys/log.h"
#include "sys/ringbuffer.h"
#include "device_setup.h"
#include "src/drivers/uart.h"
#include "src/dust_sensor/dust_sensor.h"

/* thread array for manage */
pthread_t thread[MAX_THREADS];

/* UART file descriptor */
int uart1_fd = 0;

/* check flag */
bool dustDataReady = false;
bool gpsDataReady = false;

/* semaphore for dust data and GPS data */
sem_t dustDataDoneSem;
sem_t dustDataReadySem;
sem_t gpsDataDoneSem;
sem_t gpsDataReadySem;

/* dust data buffer */
uint8_t dust_buf[DUST_DATA_FRAME];

/* json ring buffer */
ring_buffer_t json_ring_buf;
char json_ring_buf_data[RING_BUFFER_SIZE];

void* updateDustDataTask(void* arg)
{
	while (1) {
        if (dustDataReady) {
            sem_wait(&dustDataDoneSem);
        }

        /* read dust sensor data */
		readDustData(uart1_fd, dust_buf, sizeof(dust_buf));
        // checkDustData(dust_buf);
        dustDataReady = true;
        sem_post(&dustDataReadySem);
	}

	return arg;
}

void* updateGPSTask(void* arg)
{
	while (1) {
        if (gpsDataReady) {
            sem_wait(&gpsDataDoneSem);
        }

        /* read gps data */
        gpsDataReady = true;
        sem_post(&gpsDataReadySem);
	}

	return arg;
}

void* send2WebTask(void* arg)
{
	while (1) {
        // /* put gps data to json buffer */
        // sem_wait(&gpsDataReadySem);        
        // gpsDataReady = false;
        // sem_post(&gpsDataDoneSem);

        /* wait for dust data is ready */
        sem_wait(&dustDataReadySem);        
        /* parse dust data to json format */
        char dust2json_buf[64] = {0}; 
        parseDustDataToJson(dust2json_buf, dust_buf, sizeof(dust2json_buf));
        /* put dust data json format to ring buffer */
        ring_buffer_queue_arr(&json_ring_buf, dust2json_buf, strlen(dust2json_buf));
        /* signal that dust data has been processed */
        dustDataReady = false;
        sem_post(&dustDataDoneSem);
        
        /* send to web */
        char web_buf[256] = {0};
        ring_buffer_size_t ring_buf_size = ring_buffer_num_items(&json_ring_buf);
        ring_buffer_dequeue_arr(&json_ring_buf, web_buf, ring_buf_size);
        printf("%s\n", web_buf);
	}

	return arg;
}

int setupDustSensor(void) 
{
    /* Initialize UART1 Rx for receiving data from sensor */
    uart1_fd = uart_init(UART1_FILE_PATH);
    if (uart1_fd == -1) {
        LOG_ERR("Failed to initialize UART");
        return -1;
    }

    sem_init(&dustDataReadySem, 0, 0);
    sem_init(&dustDataDoneSem, 0, 1);

    /* create thread for update dust sensor data */
    int err = pthread_create(&thread[0], NULL, updateDustDataTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d\n", err);

    return err;
}

int setupGPS(void) 
{
    /* create thread for update gps data */
    int err = pthread_create(&thread[1], NULL, updateGPSTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d\n", err);

    sem_init(&gpsDataReadySem, 0, 0);
    sem_init(&gpsDataDoneSem, 0, 1);

    return err;
}

int setup4G(void) 
{
    /* Initialize ring buffer to store JSON data */
    ring_buffer_init(&json_ring_buf, json_ring_buf_data, sizeof(json_ring_buf_data));

    /* create thread for push data to web */
    int err = pthread_create(&thread[2], NULL, send2WebTask, NULL);
    if (err != 0) 
        LOG_ERR("pthread_create: %d\n", err);

    return err;
}

int deviceSetup(void)
{
    int err = 0;

    err = setupDustSensor();
    if (err != 0)
        LOG_ERR("Failed to setup dust sensor\n");

    // err = setupGPS();
    // if (err != 0)
    //     LOG_ERR("Failed to setup GPS\n");

    err = setup4G();
    if (err != 0)
        LOG_ERR("Failed to setup 4G module\n");

    return err;
}
