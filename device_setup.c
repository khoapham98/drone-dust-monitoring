#include <stdio.h>
#include <stdbool.h>
#include <pthread.>
#include <semaphore.h>
#include "src/drivers/log.h"
#include "device_setup.h"
#include "src/drivers/uart.h"

pthread_t thread[MAX_THREADS];

// /* queue for store data */
// pthread_mutex_t dustMutex;
// pthread_mutex_t gpsMutex;

/* check flag */
bool dustDataReady = false;
bool gpsDataReady = false;

/* semaphore for dust data and GPS data */
sem_t dustDataDoneSem;
sem_t dustDataReadySem;
sem_t GPSDataDoneSem;
sem_t GPSDataReadySem;

 
/* buffer */
uint8_t dustData[32];

/** 
 * ==========================================
 *                  APIs
 * ==========================================
 */

void* updateDustSensorTask(void* arg)
{
	while (1)
	{
        if (dustDataReady) {
            sem_wait(&dustDataDoneSem);
        }

		pms7003_read(uart1_fd, dustData, sizeof(dustData));
        dustDataReady = true;
        sem_post(&dustDataReadySem);
		sleep(1);
	}

	return arg;
}

void* updateGPSTask(void* arg)
{
	while (1)
	{
        if (gpsDataReady) {
            sem_wait(&gpsDataDoneSem);
        }

        // read gps data from sensor
        gpsDataReady = true;
        sem_post(&gpsDataReadySem);
		sleep(1);
	}

	return arg;
}
void* send2WebTask(void* arg)
{
	while (1)
	{
        char json_data[256] = {0};

        sem_wait(&dustDataReadySem);        
		pms7003_data_to_json(json_data, sizeof(json_data), dustData, 10.1235, 123.345);
        dustDataReady = false;
        sem_post(&dustDataDoneSem);

        sem_wait(&gpsDataReadySem);        
        // parse gps data to JSON
        gpsDataReady = false;
        sem_post(&gpsDataDoneSem);
	}

	return arg;
}

int setupDustSensor(void) 
{
    /* Initialize UART1 Rx for receiving data from sensor */
    uart_fd = uart_init(UART1_FILE_PATH);
    if (uart1_fd == -1) {
        LOG_ERR("Failed to initialize UART");
        return -1;
    }

    sem_init(&dustSem, 0, 0);

    /* create thread for update dust sensor data */
    int err = pthread_create(&thread[0], NULL, updateDustSensorTask, NULL);
    if (err != 0) {
        LOG_ERR("setupDustSensor error: %d\n", err);
    }

    return err;
}

int setupGPS(void) 
{
     /* create thread for update gps data */
    int err = pthread_create(&thread[1], NULL, updateGPSTask, NULL);
    if (err != 0) {
        LOG_ERR("setupGPS error: %d\n", err);
    }

    return err;
}

int setup4G(void) 
{
     /* create thread for push data to web */
    int err = pthread_create(&thread[2], NULL, send2WebTask, NULL);
    if (err != 0) {
        LOG_ERR("setup4G error: %d\n", err);
    }

    return err;
}

char deviceSetup(void)
{
    int err;

    err = setupDustSensor():
    if (err != 0) {
        LOG_ERR("Failed to setup dust sensor\n");
        goto error;
    }

    err = setupGPS():
    if (err != 0) {
        LOG_ERR("Failed to setup GPS\n");
        goto error;
    }

    err = setup4G():
    if (err != 0) {
        LOG_ERR("Failed to setup 4G module\n");
        goto error;
    }
error:
    LOG_ERR("Your code suck!");
}
