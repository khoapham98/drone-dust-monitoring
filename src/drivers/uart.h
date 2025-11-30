/**
 * @file    uart.h
 * @brief   uart driver header file
 */
#ifndef _UART_H_
#define _UART_H_

/* file path of UART1 in BBB */
#define 	UART1_FILE_PATH			"/dev/ttyS1"

/**
 * @brief   Initialize UART 
 * @param   UART_PATH is file path of UART
 * @return  uart_fd: success
 *          -1: error
 */
int uart_init(char* UART_PATH);

#endif