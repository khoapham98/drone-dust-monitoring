/**
 * @file    sim_service.c
 * @brief   Implementation of SIM management operations using AT commands
 */
#include <stdio.h>
#include <string.h>
#include "at.h"
#include "modem_common.h"
#include "sim_service.h"

eModemResult simCheckAlive(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_BASIC_CHECK, resp, sizeof(resp), "OK", "ERROR", 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult simEchoOn(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_ECHO_ON, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult simEchoOff(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_ECHO_OFF, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eModemResult simCheckReady(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_CHECK_READY, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "CPIN: READY"))
        return PASS;

    return FAIL;
}

eModemResult simCheckRegEps(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_CHECK_REG_EPS, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))
        return FAIL;

    int n = -1;
    int stat = -1;

    char* str = strstr(resp, "CEREG");
    if (str == NULL) 
        return FAIL;

    sscanf(str, "CEREG: %d,%d", &n, &stat);

    if (stat == 1 || stat == 5)
        return PASS;
    else if (stat == 3)
        return FAIL;

    return WAIT;
}

eModemResult simSetPdpContext(void)
{
#if VIETTEL
    char* apn = "v-internet";
#elif MOBIFONE
    char* apn = "m-wap";
#elif VINAPHONE
    char* apn = "m3-world";
#endif
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), AT_CMD_SET_PDP_CONTEXT, apn);

    if (at_send_wait(cmd, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))
        return FAIL;

    memset(resp, 0, sizeof(resp));
    
    if (at_send_wait(AT_CMD_CHECK_PDP_CONTEXT, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, apn))
        return PASS;

    return FAIL;
}

eModemResult simAttachGprs(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_ATTACH_GPRS, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))    
        return FAIL;
    
    memset(resp, 0, sizeof(resp));

    if (at_send_wait(AT_CMD_CHECK_ATTACH_GPRS, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "CGATT: 1"))
        return PASS;

    return FAIL;
}

eModemResult simActivatePdp(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_ACTIVATE_PDP, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))    
        return FAIL;
    
    memset(resp, 0, sizeof(resp));

    if (at_send_wait(AT_CMD_CHECK_PDP_ACTIVE, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    char* str = strstr(resp, "CGACT");
    if (str == NULL) 
        return FAIL;

    int cid = -1;
    int state = -1;
    sscanf(str, "CGACT: %d,%d", &cid, &state);

    if (state == 1)
        return PASS;

    return FAIL;
}
