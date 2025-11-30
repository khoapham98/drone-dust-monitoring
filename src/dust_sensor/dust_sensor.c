/**
 * @file    dust_sensor.c
 * @brief   PMS7003 driver source file
 */
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "sys/log.h"
#include "../../sys/log.h"
#include "src/dust_sensor/dust_sensor.h"

void parseDustDataToJson(char* dest, uint8_t* src, size_t len)
{
	int pm2_5 = src[12] << 8 | src[13]; 
	snprintf(dest, len, "\"sensor_value\":%d", pm2_5);
}

void printDustData(uint8_t* buf)
{
	int data[7] = {0};

	for (int i = 1, j = 4; i < 7; i++, j += 2)
	{
		data[i] = buf[j] << 8 | buf[j + 1];
	}
	printf("+-------+-------------------------------------\n");
	printf("| PM1.0 | %d ug/m3 (CF=1)  -  %d ug/m3 (ATM)\n", data[1], data[4]); 
	printf("+-------+-------------------------------------\n");
	printf("| PM2.5 | %d ug/m3 (CF=1)  -  %d ug/m3 (ATM)\n", data[2], data[5]); 
	printf("+-------+-------------------------------------\n");
	printf("| PM10  | %d ug/m3 (CF=1)  -  %d ug/m3 (ATM)\n", data[3], data[6]); 
	printf("+-------+-------------------------------------\n");
}

void checkDustData(uint8_t* buf)
{
	const char* str[2] = {"FAILED", "OK"};
	printf("Start character 1: 0x%02X [%s]\n", buf[0], str[buf[0] == 0x42]);
	printf("Start character 2: 0x%02X [%s]\n", buf[1], str[buf[1] == 0x4d]);
	int frame_len = buf[2] << 8 | buf[3];
	printf("Frame length: %d bytes [%s]\n", frame_len, str[frame_len == 28]);
}

void readDustData(int fd, uint8_t* rx_buf, int len)
{
	if (len < DUST_DATA_FRAME) 
		LOG_WRN("dust_sensor: Cannot receive data fully");

	int ret = read(fd, rx_buf, len);
	if (ret < 0)
		perror("read");
}