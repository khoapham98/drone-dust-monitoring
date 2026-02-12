/**
 * @file    http_service.h
 * @brief   HTTP communication operations over cellular modem
 */
#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_
#include "modem_common.h"

#define AT_CMD_HTTP_START               "AT+HTTPINIT\r\n"          
#define AT_CMD_HTTP_STOP                "AT+HTTPTERM\r\n"          
#define AT_CMD_HTTP_SET_URL             "AT+HTTPPARA=\"URL\",\"%s\"\r\n"          
#define AT_CMD_HTTP_SET_CONN_TIMEOUT    "AT+HTTPPARA=\"CONNECTTO\",%d\r\n"
#define AT_CMD_HTTP_SET_RECV_TIMEOUT    "AT+HTTPPARA=\"RECVTO\",%d\r\n"
#define AT_CMD_HTTP_SET_CONTENT         "AT+HTTPPARA=\"CONTENT\",\"%s\"\r\n"
#define AT_CMD_HTTP_SET_ACCEPT          "AT+HTTPPARA=\"ACCEPT\",\"%s\"\r\n"
#define AT_CMD_HTTP_SET_SSL             "AT+HTTPPARA=\"SSLCFG\",%d\r\n"
#define AT_CMD_HTTP_SET_USER_DATA       "AT+HTTPPARA=\"USERDATA\",\"%s\"\r\n"
#define AT_CMD_HTTP_SET_READ_MODE       "AT+HTTPPARA=\"READMODE\",%d\r\n"
#define AT_CMD_HTTP_SEND_ACTION         "AT+HTTPACTION=%d\r\n"
#define AT_CMD_HTTP_INPUT_DATA          "AT+HTTPDATA=%d,%d\r\n"

/**
 * @brief Initialize and start HTTP service.
 * @return PASS if HTTP service started successfully,
 *         FAIL if initialization failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpStartService(void);

/**
 * @brief Stop and terminate HTTP service.
 * @return PASS if HTTP service stopped successfully,
 *         FAIL if termination failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpStopService(void);

/**
 * @brief Set target URL for HTTP request.
 * @param url Pointer to URL string.
 * @return PASS if URL set successfully,
 *         FAIL if URL is invalid or rejected,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetUrl(const char* url);

/**
 * @brief Set Content-Type header for HTTP request.
 * @param content Pointer to content type string (e.g. "application/json").
 * @return PASS if content type set successfully,
 *         FAIL if parameter is invalid,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetContent(const char* content);

/**
 * @brief Set Accept header for HTTP request.
 * @param acptType Pointer to accept type string.
 * @return PASS if accept type set successfully,
 *         FAIL if parameter is invalid,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetAccept(const char* acptType);

/**
 * @brief Set connection timeout for HTTP session.
 * @param timeout Timeout value in seconds.
 * @return PASS if timeout set successfully,
 *         FAIL if timeout value is invalid,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetConnectionTimeout(int timeout);

/**
 * @brief Set data reception timeout for HTTP response.
 * @param timeout Timeout value in seconds.
 * @return PASS if reception timeout set successfully,
 *         FAIL if timeout value is invalid,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetReceptionTimeout(int timeout);

/**
 * @brief Bind SSL context to HTTP service.
 * @param ctx_id SSL context identifier.
 * @return PASS if SSL context set successfully,
 *         FAIL if context ID is invalid,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetSslContextId(int ctx_id);

/**
 * @brief Set custom HTTP header.
 * @param header Pointer to custom header string.
 * @return PASS if header set successfully,
 *         FAIL if header is invalid or too long,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSetCustomHeader(const char* header);

/**
 * @brief Send HTTP action (GET, POST, PUT, etc.).
 * @param method HTTP method identifier.
 * @return PASS if action executed successfully,
 *         FAIL if method is invalid or request failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSendAction(int method);

/**
 * @brief Send HTTP payload data.
 * @param data Pointer to data buffer.
 * @param len Length of data in bytes.
 * @param time Timeout for data input in seconds.
 * @return PASS if data sent successfully,
 *         FAIL if data transmission failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult httpSendData(char* data, int len, int time);

#endif