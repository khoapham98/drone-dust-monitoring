/**
 * @file    sim_service.h
 * @brief   SIM management operations using AT command interface
 */
#ifndef _SIM_SERVICE_H_
#define _SIM_SERVICE_H_
#include "modem_common.h"

#define     VIETTEL         1
#define     MOBIFONE        0
#define     VINAPHONE       0

#define CMD_ENTER_CMD_MODE          "+++\r\n"
#define CMD_ENTER_DATA_MODE         "ATO\r\n"
#define AT_CMD_BASIC_CHECK          "AT\r\n"
#define AT_CMD_ECHO_ON              "ATE1\r\n"
#define AT_CMD_ECHO_OFF             "ATE0\r\n"
#define AT_CMD_READ_ICCID           "AT+CICCID\r\n"
#define AT_CMD_CHECK_READY          "AT+CPIN?\r\n"
#define AT_CMD_CHECK_SIGNAL         "AT+CSQ\r\n"
#define AT_CMD_CHECK_REG_EPS        "AT+CEREG?\r\n"
#define AT_CMD_CHECK_PDP_CONTEXT    "AT+CGDCONT?\r\n"
#define AT_CMD_SET_PDP_CONTEXT      "AT+CGDCONT=1,\"IP\",\"%s\"\r\n"   // APN
#define AT_CMD_ATTACH_GPRS          "AT+CGATT=1\r\n"
#define AT_CMD_DETACH_GPRS          "AT+CGATT=0\r\n"
#define AT_CMD_CHECK_ATTACH_GPRS    "AT+CGATT?\r\n"
#define AT_CMD_ACTIVATE_PDP         "AT+CGACT=1,1\r\n"
#define AT_CMD_DEACTIVATE_PDP       "AT+CGACT=0,1\r\n"
#define AT_CMD_CHECK_PDP_ACTIVE     "AT+CGACT?\r\n"
#define AT_CMD_GET_IP_ADDR          "AT+CGPADDR=1\r\n"

/**
 * @brief Enter command mode using "+++".
 * @return 0 on success, -1 on failure.
 */
int simEnterCmdMode(void);

/**
 * @brief Enter data mode using "ATO".
 * @return 0 on success, -1 on failure.
 */
int simEnterDataMode(void);

/**
 * @brief Check if the module is alive (AT → OK).
 * @return PASS if response OK,
 *         FAIL if response ERROR,
 *         WAIT if command cannot be sent or response is incomplete.
 */
eModemResult simCheckAlive(void);

/**
 * @brief Enable AT command echo (ATE1).
 * @return PASS if echo enabled successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simEchoOn(void);

/**
 * @brief Disable AT command echo (ATE0).
 * @return PASS if echo disabled successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simEchoOff(void);

/**
 * @brief Check SIM card readiness (AT+CPIN?).
 * @return PASS if SIM is ready,
 *         FAIL if SIM error or PIN required,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simCheckReady(void);

/**
 * @brief Check EPS/LTE registration status (AT+CEREG?).
 * @return PASS if registered (stat = 1 or 5),
 *         FAIL if not registered,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simCheckRegEps(void);

/**
 * @brief Set PDP context APN (AT+CGDCONT).
 * @return PASS if PDP context set successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simSetPdpContext(void);

/**
 * @brief Attach to GPRS service (AT+CGATT=1).
 * @return PASS if attached successfully,
 *         FAIL if attach rejected,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simAttachGprs(void);

/**
 * @brief Activate PDP context (AT+CGACT=1,1).
 * @return PASS if PDP activated,
 *         FAIL if activation failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult simActivatePdp(void);

#endif