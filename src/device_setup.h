/**
 * @file    device_setup.h
 * @brief   setup device header file
 */
#ifndef _DEVICE_SETUP_H_
#define _DEVICE_SETUP_H_

/* system macros */
#define 	MAX_THREADS				4
#define     RING_BUFFER_SIZE        8192

/* macros are used to turn modules ON/OFF for testing */
#define     DUST_SENSOR_ENABLE      1
#define     GPS_ENABLE              1
#define     SIM_ENALBE              1            

/* macros to enable log */
#define     LOG_TO_CONSOLE          1
#define     LOG_TO_FILE             1
#define     LOG_FILE_PATH           "doc/app.log"

/* select board */
#define     BBB                     0
#define     RPI                     1


/**
 * @brief   setup device 
 * @return  none
 */
int deviceSetup(void);

#endif