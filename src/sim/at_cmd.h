/**
 * @file    at_cmd.h
 * @brief   Low-level AT command header file for UART communication
 */
#ifndef _AT_CMD_H_
#define _AT_CMD_H_
#include <stdint.h>

#define RESP_FRAME                  128

/* SWITCH BETWEEN MODES */
#define CMD_ENTER_CMD_MODE          "+++\r\n"
#define CMD_ENTER_DATA_MODE         "ATO\r\n"

/* BASIC CHECK */
#define AT_CMD_BASIC_CHECK          "AT\r\n"
#define AT_CMD_ECHO_ON              "ATE1\r\n"
#define AT_CMD_ECHO_OFF             "ATE0\r\n"
#define AT_CMD_GET_FW_VERSION       "AT+GMR\r\n"
#define AT_CMD_GET_MODEL            "AT+CGMM\r\n"
#define AT_CMD_GET_MANUFACTURER     "AT+CGMI\r\n"
#define AT_CMD_GET_IMEI             "AT+CGSN\r\n"
#define AT_CMD_GET_FUN_MODE         "AT+CFUN?\r\n"

/* SIM & SIGNAL */
#define AT_CMD_READ_ICCID           "AT+CICCID\r\n"
#define AT_CMD_CHECK_READY          "AT+CPIN?\r\n"
#define AT_CMD_CHECK_SIGNAL         "AT+CSQ\r\n"
#define AT_CMD_CHECK_REG_NET        "AT+CREG?\r\n"
#define AT_CMD_CHECK_REG_GPRS       "AT+CGREG?\r\n"
#define AT_CMD_CHECK_REG_EPS        "AT+CEREG?\r\n"
#define AT_CMD_CHECK_OPERATOR       "AT+COPS?\r\n"
#define AT_CMD_CHECK_PDP_CONTEXT    "AT+CGDCONT?\r\n"

/* PDP CONTEXT / PACKET DATA */
#define AT_CMD_SET_PDP_CONTEXT      "AT+CGDCONT=1,\"IP\",\"%s\"\r\n"   // APN
#define AT_CMD_ATTACH_GPRS          "AT+CGATT=1\r\n"
#define AT_CMD_DETACH_GPRS          "AT+CGATT=0\r\n"
#define AT_CMD_ACTIVATE_PDP         "AT+CGACT=1,1\r\n"
#define AT_CMD_DEACTIVATE_PDP       "AT+CGACT=0,1\r\n"
#define AT_CMD_GET_IP_ADDR          "AT+CGPADDR=1\r\n"

/**
 * @brief   Attach an external buffer to store the next AT command response.
 * @param   buf Pointer to the buffer to store response (or NULL to detach).
 * @param   len Size of the buffer in bytes.
 * @return  none
 */
void at_attach_resp_buffer(char* buf, size_t len);

/**
 * @brief   Send an AT command and wait for the response.
 * @param   cmd Null-terminated AT command string (without newline).
 * @param   timeout_ms Maximum time to wait for the response, in milliseconds.
 * @return  0 on success (response received); -1 on error.
 */
int at_send_wait(char* cmd, uint64_t timeout_ms);

/**
 * @brief   Send raw data over UART without waiting for a response.
 * @param   cmd Pointer to the data buffer to send.
 * @param   len Number of bytes to send.
 * @return  Number of bytes written on success; -1 on error.
 */
int at_send(char* cmd, size_t len);

/**
 * @brief   Read data from UART with a timeout.
 * @param   buf Buffer to store the received data.
 * @param   max_len Maximum number of bytes to read.
 * @param   timeout_ms Maximum time to wait for data, in milliseconds.
 * @return  Number of bytes read; -1 on error.
 */
int at_read(char* buf, size_t max_len, uint64_t timeout_ms);

/**
 * @brief   Initialize the UART interface for AT communication.
 * @param   uart_file_path Path to the UART device (e.g. "/dev/ttyS1").
 * @return  0 on success; -1 on error.
 */
int sim_init(char* uart_file_path);

#endif