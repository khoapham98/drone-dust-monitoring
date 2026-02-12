/**
 * @file    http_service.c
 * @brief   Implementation of HTTP communication operations over cellular modem
 */
#include <stdio.h>
#include <string.h>
#include "at.h"
#include "modem_common.h"
#include "http_service.h"

eModemResult httpStartService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_HTTP_START, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult httpStopService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_HTTP_STOP, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "ERROR") || strstr(resp, "OK")) 
        return PASS;

    return FAIL;
}

eModemResult httpSetUrl(const char* url)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[RESP_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_URL, url);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult httpSetContent(const char* content)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_CONTENT, content);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSetAccept(const char* acptType)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_ACCEPT, acptType);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSetConnectionTimeout(int timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_CONN_TIMEOUT, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSetReceptionTimeout(int timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_RECV_TIMEOUT, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSetSslContextId(int ctx_id)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_SSL, ctx_id);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSetCustomHeader(const char* header)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_USER_DATA, header);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult httpSendData(char* data, int len, int time)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_HTTP_INPUT_DATA,
            len, time);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (!strstr(resp, "DOWNLOAD"))
        return FAIL;

    memset(resp, 0, sizeof(resp));

    if (at_send_wait(data, resp, sizeof(resp), 200) < 0)
        return WAIT;
   
    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult httpSendAction(int method)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SEND_ACTION, method);

    if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    return PASS;
}