/**
 * @file	uart.c
 * @brief	uart driver source file
 */
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include "uart.h"
#include "sys/log.h"

void readUART(int fd, uint8_t* buf, int len)
{
	int ret = read(fd, buf, len);
	if (ret < 0) 
		LOG_ERR("Read failed: %s", strerror(errno));
}

int uart_init(char* UART_PATH, bool isSim)
{	
	int uart_fd = -1;
	if (isSim) 
		uart_fd = open(UART_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK);
	 else 
		uart_fd = open(UART_PATH, O_RDWR);

	if (uart_fd < 0) {
		LOG_ERR("Open %s failed: %s", UART_PATH, strerror(errno));
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
