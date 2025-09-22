#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

unsigned int ads1115_get_reg_val(int fd, unsigned char* reg_addr)
{
	int ret = write(fd, reg_addr, 1);
	if (ret < 0)
		perror("write");

	unsigned char rx_buf[2] = {0};
	ret = read(fd, rx_buf, 2);
	if (ret < 0)
		perror("read");

	unsigned int val = rx_buf[0] << 8 | rx_buf[1];
	return val;
}

void ads1115_reset(int fd)
{
	write(fd, (const void*) 0x06, 1);
}

unsigned int ads1115_init(char* fpath, unsigned int slave_addr)
{
	unsigned int fd = open(fpath, O_RDWR);
	if (fd < 0)
	{
		perror("open");
		return -1;
	}

	int ret = ioctl(fd, I2C_SLAVE, slave_addr);
	if (ret < 0)
	{
		perror("ioctl");
		return -1;
	}
	return fd;
}
