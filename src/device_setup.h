/**
 * @file    device_setup.h
 * @brief   setup device header file
 */
#ifndef _DEVICE_SETUP_H_
#define _DEVICE_SETUP_H_

/* system macros */
#define 	MAX_THREADS				3
#define     RING_BUFFER_SIZE        2048

/* macros are used to turn modules ON/OFF for testing */
#define     DUST_SENSOR_ENABLE      1
#define     GPS_ENABLE              1
#define     SIM_ENALBE              0            

/* macros to enable log */
#define     LOG_TO_CONSOLE          1
#define     LOG_TO_FILE             1
#define     LOG_FILE_PATH           "doc/app.log"

/* select board */
#define     BBB                     1
#define     RPI                     0
/**
 * @brief   setup device 
 * @return  none
 */
int deviceSetup(void);

#endif