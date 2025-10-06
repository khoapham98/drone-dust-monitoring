#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "../inc/pms7003.h"
#include <string.h>

void pms7003_data_to_json(char* dest, uint8_t* src, float lat, float lon)
{
	int data[7] = {0};

	for (int i = 1, j = 4; i < 7; i++, j += 2)
	{
		data[i] = src[j] << 8 | src[j + 1];
	}

	sprintf(dest, "{\n");
	sprintf(dest + strlen(dest), "\"PM1.0\": %d,\n", data[4]);
	sprintf(dest + strlen(dest), "\"PM2.5\": %d,\n", data[5]);
	sprintf(dest + strlen(dest), "\"PM10\": %d,\n", data[6]);
	sprintf(dest + strlen(dest), "\"lat\": %f,\n", lat);
	sprintf(dest + strlen(dest), "\"lon\": %f}", lon);
	sprintf(dest, "\n{");
}

void pms7003_get_PM(uint8_t* buf)
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

void pms7003_get_start_char(uint8_t* buf)
{
	printf("Start character 1: 0x%02X\n", buf[0]);
	printf("Start character 2: 0x%02X\n", buf[1]);
	int frame_len = buf[2] << 8 | buf[3];
	const char* str[2] = {"Wrong", "Correct"};
	int check = (frame_len == 28) ? 1 : 0; 
	printf("Frame length = %d bytes - %s\n", frame_len, str[check]);
}

void pms7003_read(int fd, uint8_t* rx_buf, int len)
{
	int ret = read(fd, rx_buf, len);

	if (ret < 0)
		perror("read");
	else 
		printf("byte return: %d\n", ret);
}
