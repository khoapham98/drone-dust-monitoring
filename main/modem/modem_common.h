/**
 * @file    modem_common.h
 * @brief   Common modem definitions shared across all modem layers
 */
#ifndef _MODEM_COMMON_H_
#define _MODEM_COMMON_H_

enum modem_result {
    PASS,
    WAIT,
    FAIL
};

typedef enum modem_result eModemResult;

#define RESP_BUFFER_SIZE    256
#define CMD_BUFFER_SIZE     512

#endif