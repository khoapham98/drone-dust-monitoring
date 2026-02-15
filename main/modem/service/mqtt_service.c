/**
 * @file    mqtt_service.c
 * @brief   Implementation of MQTT protocol operations over cellular modem
 */
#include <stdio.h>
#include <string.h>
#include "at.h"
#include "modem_common.h"
#include "mqtt_service.h"

eModemResult mqttStartService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_MQTT_START, resp, sizeof(resp), 100) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        return PASS;
    }

    char* str = strstr(resp, "CMQTTSTART");
    if (str == NULL) 
        return FAIL;

    eMqttError err = -1;
    sscanf(str, "CMQTTSTART: %d", (int*) &err);

    if (err == MQTT_RES_OK) 
        return PASS;

    return FAIL; 
}

eModemResult mqttStopService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_MQTT_STOP, resp, sizeof(resp), 100) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTSTOP");
    if (str == NULL) 
        return FAIL;

    eMqttError err = -1;
    sscanf(str, "CMQTTSTOP: %d", (int*) &err);

    if (err == MQTT_RES_OK) 
        return PASS;

    return FAIL; 
}

eModemResult mqttReleaseClient(int index)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_RELEASE, 
            index);
    
    if (at_send_wait(cmd, resp, sizeof(resp), 100) < 0)
        return WAIT;

    if (strstr(resp, "OK"))
        return PASS;

    char* str = strstr(resp, "CMQTTREL");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttError err = -1;
    sscanf(str, "CMQTTREL: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_NOT_ACQUIRED)
        return PASS;
    
    return FAIL;
}

eModemResult mqttAcquireClient(int index, char* id, int type)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_ACQUIRE, 
            index, id, type);
    
    if (at_send_wait(cmd, resp, sizeof(resp), 100) < 0)
        return WAIT;

    if (strstr(resp, "OK"))    
        return PASS;
    
    char* str = strstr(resp, "CMQTTACCQ");
    if (str == NULL) 
        return FAIL;

    int clientIndex = -1;
    eMqttError err = -1;
    sscanf(str, "CMQTTACCQ: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED)
        return PASS;

    return FAIL;
}

eModemResult mqttDisconnect(int index, int timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_DISCONNECT,
            index, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTDISC");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttError err = -1;
    sscanf(str, "CMQTTDISC: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK)
        return PASS;

    return FAIL;
}

eModemResult mqttConnect(mqttClient* cli, mqttServer* ser)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_CONNECT,
            cli->index, ser->addr, cli->keepAliveTime, 
            cli->cleanSession, cli->userName, cli->password);

    if (at_send_wait(cmd, resp, sizeof(resp), 200) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTCONNECT");
    if (str == NULL)
        return FAIL;

    int clientIndex = -1;
    eMqttError err = -1;
    sscanf(str, "CMQTTCONNECT: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED)
        return PASS;

    return FAIL;
}

eModemResult mqttSetPublishTopic(int index, char* topic, int len)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_TOPIC,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), 50) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        char* str = strstr(resp, "CMQTTTOPIC");
        if (str == NULL)
            return FAIL;

        int clientIndex = -1;
        eMqttError err = -1;
        sscanf(str, "CMQTTTOPIC: %d,%d", &clientIndex, (int*) &err);

        if (err == MQTT_RES_OK)
            return PASS;

        return FAIL;
    }

    if (strchr(resp, '>') == NULL)
        return FAIL;

    memset(resp, 0, sizeof(resp));

    if (at_send_wait(topic, resp, sizeof(resp), 80) < 0)
        return WAIT;
    
    if (strstr(resp, "OK"))    
        return PASS; 

    return FAIL;
}

eModemResult mqttSetPayload(int index, char* msg, int len)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PAYLOAD,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), 50) < 0)
        return WAIT;

    if (strstr(resp, "ERROR")) {
        char* str = strstr(resp, "CMQTTPAYLOAD");
        if (str == NULL)
            return FAIL;

        int clientIndex = -1;
        eMqttError err = -1;
        sscanf(str, "CMQTTPAYLOAD: %d,%d", &clientIndex, (int*) &err);

        if (err == MQTT_RES_OK)
            return PASS;

        return FAIL;
    }

    if (strchr(resp, '>') == NULL)
        return FAIL;

    memset(resp, 0, sizeof(resp));

    if (at_send_wait(msg, resp, sizeof(resp), 80) < 0)
        return WAIT;
    
    if (strstr(resp, "OK"))    
        return PASS; 

    return FAIL;
}

eModemResult mqttPublish(int index, int QoS, int pub_timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PUBLISH,
            index, QoS, pub_timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), 30) < 0)
        return WAIT;

    char* str = strstr(resp, "CMQTTPUB");

    if (str == NULL) {
        if (at_wait(resp + strlen(resp), sizeof(resp), 30) < 0)
            return FAIL;    

        str = strstr(resp, "CMQTTPUB");
        if (str == NULL)
            return FAIL;
    }

    int clientIndex = -1;
    eMqttError err = -1;
    sscanf(str, "CMQTTPUB: %d,%d", &clientIndex, (int*) &err);

    if (err == MQTT_RES_OK)
        return PASS;

    return FAIL;
}