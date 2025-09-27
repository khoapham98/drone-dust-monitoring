#include <stdio.h>
#include <unistd.h>
#include "inc/ads1115.h"
#include "inc/gp2y1010au0f.h"

#define 	I2C_FILE_PATH			"/dev/i2c-2"
#define 	GPIO_FILE_PATH			"/sys/class/gpio/gpio20"

int main()
{
	int i2c_fd = ads1115_init(I2C_FILE_PATH, ADS1115_BASE_ADDR);
	ads1115_config(i2c_fd, AIN0_GND, SINGLE_SHOT);
	gpio_set_mode(GPIO_FILE_PATH, GPIO_OUTPUT);
	sleep(1);

	while (1)
	{
		float res = gp2y_get_dust_density(i2c_fd);
		printf("%.2f\n", res);
		sleep(3);
	}

	return 0;
}
