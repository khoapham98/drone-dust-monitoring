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
 * @brief   read dust data 
 * @param   fd UART file descriptor
 * @param   rx_buf buffer address to store data
 * @param   len length of buffer - must be atlease 32 bytes 
 * @return  none
 *  */
void readDustData(int fd, uint8_t* rx_buf, int len);

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
 * @brief   parse dust data to JSON
 * @param   dest JSON buffer address
 * @param   src dust data buffer address
 * @return  none
 */
void parseDustDataToJson(char* dest, uint8_t* src, size_t len);

#endif