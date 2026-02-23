/**
 * @file    device_setup.h
 * @brief   setup device header file
 */
#ifndef _DEVICE_SETUP_H_
#define _DEVICE_SETUP_H_

/* macros are used to turn modules ON/OFF for testing */
#define DUST_SENSOR_ENABLE      1
#define GPS_ENABLE              1
#define SIM_ENABLE              1            

/**
 * @brief   setup device 
 * @return  none
 */
int deviceSetup(void);

#endif