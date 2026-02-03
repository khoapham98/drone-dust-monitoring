/**
 * @file    device_setup.h
 * @brief   setup device header file
 */
#ifndef DEVICE_SETUP_H
#define DEVICE_SETUP_H

/* macros are used to turn modules ON/OFF for testing */
#define     DUST_SENSOR_ENABLE      1
#define     GPS_ENABLE              1
#define     SIM_ENALBE              1            

/* Default latitude and longitude values.
 * Here set to the coordinates of Ton Duc Thang University (TDTU), Ho Chi Minh City. 
 */
#define     DEFAULT_LATITUDE        10.7318f 
#define     DEFAULT_LONGITUDE       106.6981f

/**
 * @brief   setup device 
 * @return  none
 */
int deviceSetup(void);

#endif