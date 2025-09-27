#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../inc/gp2y1010au0f.h"
#include "../inc/ads1115.h"

static char fpath_direct[100];
static char fpath_value[100];

static float Voc = 0.9;

float gp2y_read_Vo_when_off(int fd)
{
	float Vo = ads1115_get_voltage(fd);
	return Vo;
}

float gp2y_read_Vo_when_clean(int fd)
{	
	gpio_set_value(GPIO_LOW);
	usleep(1000);
	float Vo = ads1115_get_voltage(fd);
	usleep(40);
	gpio_set_value(GPIO_HIGH);
	usleep(8960);

	return Vo;
}

float gp2y_get_dust_density(int fd)
{
	gpio_set_value(GPIO_LOW);
	usleep(320);
	float Vo = ads1115_get_voltage(fd);
	usleep(280);
	gpio_set_value(GPIO_HIGH);
	usleep(9400);

	float pm = ((Vo - Voc) * 0.1) / 0.5;
	if (pm < 0)
		pm = 0;

	return pm;
}

void gpio_set_value(char* value)
{
	int fd = open(fpath_value, O_WRONLY);
	if (fd < 0)
	{
		perror("value open");
		close(fd);
		return;
	}
	
	int ret = write(fd, value, strlen(value));
	if (ret < 0)
		perror("value write");

	close(fd);
}

void gp2y_init()
{
	gpio_set_mode(GPIO_OUTPUT);
	gpio_set_value(GPIO_HIGH);
}

void gpio_set_mode(char* mode)
{
	int fd = open(fpath_direct, O_WRONLY);
	if (fd < 0)
	{
		perror("direction open");
		close(fd);
		return;
	}
	
	int ret = write(fd, mode, strlen(mode));
	if (ret < 0)
		perror("direction write");

	close(fd);
}

void gpio_init(char* fpath)
{
	strcpy(fpath_direct, fpath);
	strcpy(fpath_value, fpath);

	int last_index = strlen(fpath_value) - 1;
	if (fpath_value[last_index] == '/')
		strcat(fpath_value, "value");
	else 
		strcat(fpath_value, "/value");

	last_index = strlen(fpath_direct) - 1;
	if (fpath_direct[last_index] == '/')
		strcat(fpath_direct, "direction");
	else 
		strcat(fpath_direct, "/direction");
}
