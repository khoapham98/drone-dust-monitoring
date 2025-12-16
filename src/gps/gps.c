/**
 * @file    gps.c
 * @brief   ATGM336H driver source file
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "sys/log.h"
#include "sys/ringbuffer.h"
#include "src/gps/gps.h"
#include "src/drivers/uart.h"

static int uart_fd = 0;
static int findSubStr(char* haystack, char* needle);
static size_t getMessageLen(char* str);

void getGpsCoordinates(char* buf, double* latitude, double* longitude)
{
	int firstIndex = findSubStr(buf, "$GNRMC");
	if (firstIndex < 0) {
		LOG_ERR("Can't find RMC message from GPS");
		return;
	}

	char new_buf[128] = {0};
	size_t newLen = getMessageLen(&buf[firstIndex]);
	if (newLen > sizeof(new_buf)) newLen = sizeof(new_buf);
	strncpy(new_buf, &buf[firstIndex], newLen);

	volatile int field = 1;
	int lat_degree = 0;
	int lon_degree = 0;
	double lat_minute = 0;
	double lon_minute = 0;
	bool dataErr = false;

	for (size_t i = 0; i < newLen; i++) {
		if (new_buf[i] != ',') continue;

        field++;
        if (field > 6) break;

		if (field == LATITUDE_FIELD_NUM) {
			if (new_buf[i + 1] == ',') {
				LOG_ERR("Invalid latitude data");
				dataErr = true;
				continue;
			}
			
			/* get latitude data */
			lat_degree = (new_buf[i + 1] - '0') * 10 + (new_buf[i + 2] - '0'); 			
            float divide = 10;
			bool isDecimal = true;
			for (int j = i + 3; new_buf[j] != ','; j++) {
				if (new_buf[j] == '.') {
					isDecimal = false;
					continue;
				}

				if (isDecimal) {
					lat_minute *= 10;
					lat_minute += new_buf[j] - '0';
				} else {
					lat_minute += (new_buf[j] - '0') / divide;	
					divide *= 10;
				}
			}
		} 
		else if (field == LONGTITUDE_FIELD_NUM) {
			if (new_buf[i + 1] == ',') {
			    LOG_ERR("Invalid longitude data");
				dataErr = true;
				continue;
			}

			/* get longtitude data */
			lon_degree = (new_buf[i + 1] - '0') * 100 + (new_buf[i + 2] - '0') * 10 + (new_buf[i + 3] - '0');
			float divide = 10;
			bool isDecimal = true;
			for (size_t j = i + 4; new_buf[j] != ','; j++) {
				if (new_buf[j] == '.') {
					isDecimal = false;
					continue;
				}

				if (isDecimal) {
					lon_minute *= 10;
					lon_minute += new_buf[j] - '0';
				} else {
					lon_minute += (new_buf[j] - '0') / divide;	
					divide *= 10;
				}
			}
		}
	}

	if (dataErr) {
		LOG_ERR("GPS data error, keeping previous values");
		return;
	}

	*latitude = lat_degree + (lat_minute / 60);
	*longitude = lon_degree + (lon_minute / 60);
}

void readGpsData(uint8_t* buf, int len)
{
	if (len < NMEA_FRAME) 
		LOG_WRN("gps: May not receive data fully");

	readUART(uart_fd, buf, len);	
}

int GPS_uart_init(char* uart_file_path)
{
	uart_fd = uart_init(uart_file_path, false);
    if (uart_fd < 0) {
        return -1;
	}
    
	LOG_INF("GPS Initialization successful");
	return 0;
}

static size_t getMessageLen(char* str)
{
	int index = 0;
	while (str[index] != '\r' && str[index + 1] != '\n') {
		index++;
	}

	return index;
}

static int findSubStr(char* haystack, char* needle)
{
	if (haystack == NULL || needle == NULL) 
		return -1;

	size_t haystackLen = strlen(haystack);
	size_t needleLen = strlen(needle);

	if (haystackLen < needleLen)
		return -1;

	for (size_t i = 0; i < haystackLen; i++) {
		if (haystack[i] == needle[0]) {
			bool bothAreSame = true;		
			for (size_t j = 0; j < needleLen; j++) {
				if (haystack[i] != needle[j]) {
					bothAreSame = false;
					break;	
				}
				i++;
			}

			if (bothAreSame) {
				return i - needleLen;
			}
			i--;
		}
	}	

	return -1;
}