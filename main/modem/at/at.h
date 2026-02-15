/**
 * @file    at.h
 * @brief   Low-level AT command header file for send AT commands & receive responses
 */
#ifndef _AT_H_
#define _AT_H_
#include <stdint.h>

/**
 * @brief   Send an AT command and wait for the response.
 * @param   cmd Null-terminated AT command string (without newline).
 * @param   recv_buf Pointer to buffer to store response message.
 * @param   len Length of the receive buffer.
 * @param   timeout_ms Maximum time to wait for the response, in milliseconds.
 * @return  Total number of bytes received on success; -1 on error.
 */
int at_send_wait(char* cmd, char* recv_buf, size_t len, uint64_t timeout_ms);

/**
 * @brief   Send raw data over UART without waiting for a response.
 * @param   cmd Pointer to the data buffer to send.
 * @param   len Number of bytes to send.
 * @return  Number of bytes written on success; -1 on error.
 */
int at_send(char* cmd, size_t len);

/**
 * @brief   Wait for AT response from UART.
 * @param   recv_buf Pointer to buffer to store response message.
 * @param   len Length of the receive buffer.
 * @param   timeout_ms Maximum time to wait for the response, in milliseconds.
 * @return  Total number of bytes received on success; -1 on error.
 */
int at_wait(char* recv_buf, size_t len, uint64_t timeout_ms);

/**
 * @brief   Initialize the UART interface for AT communication.
 * @return  0 on success; -1 on error.
 */
int sim_uart_init(void);

#endif