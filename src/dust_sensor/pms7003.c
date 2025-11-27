#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "pms7003.h"

void pms7003_data_to_json(char* dest, size_t size, uint8_t* src, float lat, float lon)
{
	int pm1_0 = src[10] << 8 | src[11]; 
	int pm2_5 = src[12] << 8 | src[13]; 
	int pm10 = src[14] << 8 | src[15]; 

	snprintf(dest, size,
		"{\"pm1_0\":%d,"
		"\"pm2_5\":%d,"
		"\"pm10\":%d,"
		"\"lat\":%.6f,"
		"\"lon\":%.6f,"
		"\"perimeter\":[[%.6f,%.6f],[%.6f,%.6f],[%.6f,%.6f],[%.6f,%.6f]]}\n",
		pm1_0, pm2_5, pm10, lat, lon,
		lat + 0.00025, lon - 0.00025,
		lat + 0.00025, lon + 0.00025,
		lat - 0.00025, lon + 0.00025,
		lat - 0.00025, lon - 0.00025);
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
}
