/**
 * @file    sim_service.c
 * @brief   Implementation of SIM management operations using AT commands
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

    if (at_send_wait(AT_CMD_ECHO_ON, resp, sizeof(resp), "OK", "ERROR", 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eModemResult simEchoOff(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_ECHO_OFF, resp, sizeof(resp), "OK", "ERROR", 200) < 0)
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

    if (!strncmp(resp, "+CPIN: READY", sizeof("+CPIN: READY") - 1)) 
        return PASS; 

    return FAIL;
}

eModemResult simCheckRegEps(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_CHECK_REG_EPS, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1)) 
        return FAIL; 

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    int stat = atoi(str);

    if (stat == 1 || stat == 5)
        return PASS;
    else if (stat == 3)
        return FAIL;

    return WAIT;
}

eModemResult simSetPdpContext(void)
{
#if VIETTEL
    const char* apn = "v-internet";
#elif MOBIFONE
    const char* apn = "m-wap";
#elif VINAPHONE
    const char* apn = "m3-world";
#endif
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};

    snprintf(cmd, sizeof(cmd), AT_CMD_SET_PDP_CONTEXT, apn);

    int recvBytes = at_send_wait(cmd, resp, sizeof(resp), "OK", "ERROR", 500);
    if (recvBytes < 0)
        return WAIT;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1)) 
        return FAIL; 

    memset(resp, 0, recvBytes);
    
    if (at_send_wait(AT_CMD_CHECK_PDP_CONTEXT, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (strstr(resp, apn))
        return PASS;

    return FAIL;
}

eModemResult simAttachGprs(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    int recvBytes = at_send_wait(AT_CMD_ATTACH_GPRS, resp, sizeof(resp), "OK", "ERROR", 500);
    if (recvBytes < 0)
        return WAIT;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1)) 
        return FAIL;  
        
    memset(resp, 0, recvBytes);

    if (at_send_wait(AT_CMD_CHECK_ATTACH_GPRS, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "+CGATT: 1", sizeof("+CGATT: 1") - 1))
        return PASS;

    return FAIL;
}

eModemResult simActivatePdp(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    int recvBytes = at_send_wait(AT_CMD_ACTIVATE_PDP, resp, sizeof(resp), "OK", "ERROR", 500);
    if (recvBytes < 0)
        return WAIT;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1)) 
        return FAIL;   

    memset(resp, 0, recvBytes);

    if (at_send_wait(AT_CMD_CHECK_PDP_ACTIVE, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    char* str = strchr(resp, ',');    
    if (str == NULL) 
        return FAIL;

    str += 1;

    int state = atoi(str);

    if (state == 1)
        return PASS;

    return FAIL;
}
