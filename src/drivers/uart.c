/**
 * @file	uart.c
 * @brief	uart driver source file
 */
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "uart.h"

int uart_init(char* UART_PATH)
{	
	int uart_fd = open(UART_PATH, O_RDWR);
	if (uart_fd < 0)
	{
		perror("open");
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
