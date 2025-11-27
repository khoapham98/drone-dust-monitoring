#ifndef _UART_H_
#define _UART_H_

/**
 * @brief   Initialize UART 
 * @param   UART_PATH is file path of UART
 * @return  uart_fd: success
 *          -1: error
 */
int uart_init(char* UART_PATH);

/* UART file descriptor */
int uart1_fd;

/* file path of UART1 in BBB */
#define 	UART1_FILE_PATH			"/dev/ttyS1"

#endif