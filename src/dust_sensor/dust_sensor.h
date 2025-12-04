/**
 * @file    dust_sensor.h
 * @brief   PMS7003 driver header file
 */
#ifndef _PMS7003_H_
#define _PMS7003_H_

#include <stdint.h>
#include <stdio.h>

#define DUST_DATA_FRAME     32

/**
 * @brief   get PM2.5 value 
 * @param   buf is buffer address that contain dust sensor data
 * @param   pm2_5 is variable address to store PM2.5 value
 * @return  none
 *  */
void getPm2_5(uint8_t* buf, uint16_t* pm2_5);

/**
 * @brief   read dust data 
 * @param   buf buffer address to store data
 * @param   len length of buffer - must be atlease 32 bytes 
 * @return  none
 *  */
void readDustData(uint8_t* buf, int len);

/**
 * @brief   check dust data 
 * @param   buf buffer address
 * @return  none
 */
void checkDustData(uint8_t* buf);

/**
 * @brief   print dust data
 * @param   buf buffer address
 * @return none
 */
void printDustData(uint8_t* buf);

/**
 * @brief   Initialize dust sensor 
 * @param   uart_file_path is file path of UART
 * @return  0 if success; -1 otherwise
 */
int dustSensor_init(char* uart_file_path);

#endif