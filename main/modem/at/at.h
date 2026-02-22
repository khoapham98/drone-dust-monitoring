/**
 * @file    at.h
 * @brief   Low-level AT command header file for send AT commands & receive responses
 */
#ifndef _AT_H_
#define _AT_H_
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct at_cmd_ctx {
    const char* success_token;
    const char* error_token;
    SemaphoreHandle_t done_sem;
    size_t response_length;
    char response_buffer[512];
    bool active;
};

enum prefix {
    PREFIX_SIM,
    PREFIX_MQTT,
    PREFIX_NONE
};

enum mqttRxState {
    IDLE,
    WAIT_TOPIC,
    WAIT_PAYLOAD,
    COLLECT_PAYLOAD
};

typedef enum prefix ePrefix;
typedef enum mqttRxState eMqttRxState;
typedef struct at_cmd_ctx  at_cmd_ctx_t;

struct mqtt_parser {
    bool active;
    eMqttRxState state;
};

typedef struct mqtt_parser mqtt_parser_t;

#define URC_MAX     2

/**
 * @brief   Send an AT command and wait for the response.
 * @param   cmd Null-terminated AT command string (without newline).
 * @param   recv_buf Pointer to buffer to store response message.
 * @param   len Length of the receive buffer.
 * @param   timeout_ms Maximum time to wait for the response, in milliseconds.
 * @return  Total number of bytes received on success; -1 on error.
 */
int at_send_wait(char* cmd, char* recv_buf, size_t recv_buf_size, char* success_token, char* error_token, uint64_t timeout_ms);

/**
 * @brief   Initialize the UART interface for AT communication.
 * @return  0 on success; -1 on error.
 */
int sim_uart_init(void);

#endif