/**
 * @file	uart.c
 * @brief	uart driver source file
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "uart.h"

void readUART(int fd, uint8_t* buf, int len)
{
	int ret = read(fd, buf, len);
	if (ret < 0)
		perror("read");
}

int uart_init(char* UART_PATH)
{	
	int uart_fd = open(UART_PATH, O_RDWR);
	if (uart_fd < 0)
	{
		perror("open");
		close(uart_fd);
		return -1;
	}

	struct termios uart;
	tcgetattr(uart_fd, &uart);
	cfmakeraw(&uart);
	cfsetispeed(&uart, B9600);
	cfsetospeed(&uart, B9600);
	uart.c_cflag |= CREAD;
	tcsetattr(uart_fd, TCSANOW, &uart);

	return uart_fd;
}
