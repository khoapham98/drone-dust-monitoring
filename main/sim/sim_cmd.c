/**
 * @file    sim_cmd.c
 * @brief   High-level SIM API source file built on top of the AT command driver
 */
#include <stdio.h>
#include <string.h>
#include "at.h"
#include "sim_cmd.h"
// #include "transport/mqtt.h"

/* ===== BASIC AT ===== */

int simEnterCmdMode(void)
{
    return at_send(CMD_ENTER_CMD_MODE, strlen(CMD_ENTER_CMD_MODE));
}

int simEnterDataMode(void)
{
    return at_send(CMD_ENTER_DATA_MODE, strlen(CMD_ENTER_DATA_MODE));
}

eSimResult simCheckAlive(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_BASIC_CHECK, resp, sizeof(resp), 500) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eSimResult simEchoOn(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_ECHO_ON, resp, sizeof(resp), 500) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eSimResult simEchoOff(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_ECHO_OFF, resp, sizeof(resp), 500) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult simCheckReady(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_CHECK_READY, resp, sizeof(resp), 1000) < 0)
        return WAIT;

    if (strstr(resp, "CPIN: READY"))
        return PASS;

    return FAIL;
}

eSimResult simCheckRegEps(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_CHECK_REG_EPS, resp, sizeof(resp), 1000) < 0)
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

eSimResult simSetPdpContext(void)
{
#if VIETTEL
    char* apn = "v-internet";
#elif MOBIFONE
    char* apn = "m-wap";
#elif VINAPHONE
    char* apn = "m3-world";
#endif
    char resp[RESP_FRAME] = {0};
    char cmd[128] = {0};

    snprintf(cmd, sizeof(cmd), AT_CMD_SET_PDP_CONTEXT, apn);

    if (at_send_wait(cmd, resp, sizeof(resp), 1500) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))
        return FAIL;

    memset(resp, 0, sizeof(resp));
    
    if (at_send_wait(AT_CMD_CHECK_PDP_CONTEXT, resp, sizeof(resp), 1000) < 0)
        return WAIT;

    if (strstr(resp, apn))
        return PASS;

    return FAIL;
}

eSimResult simAttachGprs(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_ATTACH_GPRS, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))    
        return FAIL;
    
    memset(resp, 0, sizeof(resp));

    if (at_send_wait(AT_CMD_CHECK_ATTACH_GPRS, resp, sizeof(resp), 1000) < 0)
        return WAIT;

    if (strstr(resp, "CGATT: 1"))
        return PASS;

    return FAIL;
}

eSimResult simActivatePdp(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_ACTIVATE_PDP, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    if (strstr(resp, "ERROR"))    
        return FAIL;
    
    memset(resp, 0, sizeof(resp));

    if (at_send_wait(AT_CMD_CHECK_PDP_ACTIVE, resp, sizeof(resp), 1000) < 0)
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

/* ===== MQTT ===== */

eSimResult mqttStartService(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_MQTT_START, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        return PASS;
    }

    char* str = strstr(resp, "CMQTTSTART");
    if (str == NULL) 
        return FAIL;

    eMqttResult err = -1;
    sscanf(str, "CMQTTSTART: %d", (int*) &err);

    if (err == MQTT_RES_OK) 
        return PASS;

    return FAIL; 
}

eSimResult mqttReleaseClient(int index)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_RELEASE, 
            index);
    
    if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    char* str = strstr(resp, "CMQTTREL");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttResult err = -1;
    sscanf(str, "CMQTTREL: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_NOT_ACQUIRED)
        return PASS;
    
    return FAIL;
}

eSimResult mqttAcquireClient(int index, char* id, int type)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_ACQUIRE, 
            index, id, type);
    
    if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    if (strstr(resp, "OK"))    
        return PASS;
    
    char* str = strstr(resp, "CMQTTACCQ");
    if (str == NULL) 
        return FAIL;

    int clientIndex = -1;
    eMqttResult err = -1;
    sscanf(str, "CMQTTACCQ: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED)
        return PASS;

    return FAIL;
}

eSimResult mqttDisconnect(int index, int timeout)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_DISCONNECT,
            index, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTDISC");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttResult err = -1;
    sscanf(str, "CMQTTDISC: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK)
        return PASS;

    return FAIL;
}

// eSimResult mqttConnect(mqttClient* cli, mqttServer* ser)
// {
//     char resp[RESP_FRAME] = {0};
//     char cmd[CMD_LEN] = {0};
//     snprintf(cmd, sizeof(cmd),
//             AT_CMD_MQTT_CONNECT,
//             cli->index, ser->addr, cli->keepAliveTime, 
//             cli->cleanSession, cli->userName, cli->password);

//     if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
//         return WAIT;

//     char* str = strstr(resp, "CMQTTCONNECT");
//     if (str == NULL)
//         return FAIL;

//     int clientIndex = -1;
//     eMqttResult err = -1;
//     sscanf(str, "CMQTTCONNECT: %d,%d", &clientIndex, (int*) &err);

//     if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED)
//         return PASS;

//     return FAIL;
// }

eSimResult mqttSetPublishTopic(int index, char* topic, int len)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_TOPIC,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        char* str = strstr(resp, "CMQTTTOPIC");
        if (str == NULL)
            return FAIL;

        int clientIndex = -1;
        eMqttResult err = -1;
        sscanf(str, "CMQTTTOPIC: %d,%d", &clientIndex, (int*) &err);

        if (err == MQTT_RES_OK)
            return PASS;

        return FAIL;
    }

    if (strchr(resp, '>') == NULL)
        return FAIL;

    memset(resp, 0, sizeof(resp));

    if (at_send_wait(topic, resp, sizeof(resp), 150) < 0)
        return WAIT;
    
    if (strstr(resp, "OK"))    
        return PASS; 

    return FAIL;
}

eSimResult mqttSetPayload(int index, char* msg, int len)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PAYLOAD,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        char* str = strstr(resp, "CMQTTPAYLOAD");
        if (str == NULL)
            return FAIL;

        int clientIndex = -1;
        eMqttResult err = -1;
        sscanf(str, "CMQTTPAYLOAD: %d,%d", &clientIndex, (int*) &err);

        if (err == MQTT_RES_OK)
            return PASS;

        return FAIL;
    }

    if (strchr(resp, '>') == NULL)
        return FAIL;

    memset(resp, 0, sizeof(resp));

    if (at_send_wait(msg, resp, sizeof(resp), 150) < 0)
        return WAIT;
    
    if (strstr(resp, "OK"))    
        return PASS; 

    return FAIL;
}

eSimResult mqttPublish(int index, int QoS, int pub_timeout)
{
    char resp[RESP_FRAME] = {0};
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PUBLISH,
            index, QoS, pub_timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTPUB");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttResult err = -1;
    sscanf(str, "CMQTTPUB: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK)
        return PASS;

    return FAIL;
}

/* ===== HTTP ===== */

eSimResult httpStartService(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_HTTP_START, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eSimResult httpStopService(void)
{
    char resp[RESP_FRAME] = {0};

    if (at_send_wait(AT_CMD_HTTP_STOP, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "ERROR") || strstr(resp, "OK")) 
        return PASS;

    return FAIL;
}

eSimResult httpSetUrl(const char* url)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_URL, url);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    return FAIL;
}

eSimResult httpSetContent(const char* content)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_CONTENT, content);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSetAccept(const char* acptType)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_ACCEPT, acptType);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSetConnectionTimeout(int timeout)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_CONN_TIMEOUT, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSetReceptionTimeout(int timeout)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_RECV_TIMEOUT, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSetSslContextId(int ctx_id)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_SSL, ctx_id);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSetCustomHeader(const char* header)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SET_USER_DATA, header);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;
    
    return FAIL;
}

eSimResult httpSendData(char* data, int len, int time)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
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

eSimResult httpSendAction(int method)
{
    char resp[RESP_FRAME] = {0};
    char cmd[RESP_FRAME] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_HTTP_SEND_ACTION, method);

    if (at_send_wait(cmd, resp, sizeof(resp), 2000) < 0)
        return WAIT;

    return PASS;
}
