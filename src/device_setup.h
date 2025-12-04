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
#define     ENABLE_4G               1            
#define     PARSE_EACH              0

/**
 * @brief   setup device 
 * @return  none
 */
int deviceSetup(void);

#endif