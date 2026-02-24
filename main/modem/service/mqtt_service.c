/**
 * @file    mqtt_service.c
 * @brief   Implementation of MQTT protocol operations over cellular modem
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "at.h"
#include "modem_common.h"
#include "mqtt_service.h"

static const char* TAG = "mqtt_service";

static const char* const mqtt_err_str[] = {
    "Operation succeeded",
    "Failed",
    "Bad UTF-8 string",
    "Sock connect fail",
    "Sock create fail",
    "Sock close fail",
    "Message receive fail",
    "Network open fail",
    "Network close fail",
    "Network not opened",
    "Client index error",
    "No connection",
    "Invalid parameter",
    "Not supported operation",
    "Client is busy",
    "Require connection fail",
    "Sock sending fail",
    "Timeout",
    "Topic is empty",
    "Client is used",
    "Client not acquired",
    "Client not released",
    "Length out of range",
    "Network is opened",
    "Packet fail",
    "DNS error",
    "Socket is closed by server",
    "Connection refused: unaccepted protocol version",
    "Connection refused: identifier rejected",
    "Connection refused: server unavailable",
    "Connection refused: bad user name or password",
    "Connection refused: not authorized",
    "Handshake fail",
    "Not set certificate",
    "Open session failed",
    "Disconnect from server failed"
};

eModemResult mqttStartService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_MQTT_START, resp, sizeof(resp), "CMQTTSTART", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1))
        return PASS;

    char* str = strstr(resp, "+CMQTTSTART");
    if (str == NULL)
        return FAIL;

    if (!strncmp(str, "+CMQTTSTART: 0", sizeof("+CMQTTSTART: 0") - 1))
        return PASS;

    return FAIL; 
}

eModemResult mqttStopService(void)
{
    char resp[RESP_BUFFER_SIZE] = {0};

    if (at_send_wait(AT_CMD_MQTT_STOP, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "OK", sizeof("OK") - 1)) 
        return PASS;

    if (!strncmp(resp, "ERROR", sizeof("ERROR") - 1))
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
    
    if (at_send_wait(cmd, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "OK", sizeof("OK") - 1)) 
        return PASS;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;
    eMqttError err = atoi(str);
        
    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_NOT_ACQUIRED) {
        ESP_LOGD(TAG, "%s", mqtt_err_str[err]);
        return PASS;
    }

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);   
    return FAIL;
}

eModemResult mqttAcquireClient(int index, char* id, int type)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_ACQUIRE, 
            index, id, type);
    
    if (at_send_wait(cmd, resp, sizeof(resp), "OK", "ERROR", 500) < 0)
        return WAIT;

    if (!strncmp(resp, "OK", sizeof("OK") - 1)) 
        return PASS;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);
  
    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED) {
        ESP_LOGD(TAG, "%s", mqtt_err_str[err]);
        return PASS;
    }

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);   
    return FAIL;
}

eModemResult mqttDisconnect(int index, int timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_DISCONNECT,
            index, timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), "CMQTTDISC", NULL, 500) < 0)
        return WAIT;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    if (err == MQTT_RES_OK || err == MQTT_RES_NO_CONNECTION) {
        ESP_LOGD(TAG, "%s", mqtt_err_str[err]);
        return PASS;
    }

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);   
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

    if (at_send_wait(cmd, resp, sizeof(resp), "CMQTTCONNECT", NULL, 1000) < 0)
        return WAIT;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    if (err == MQTT_RES_OK || err == MQTT_RES_CLIENT_USED) {
        ESP_LOGD(TAG, "%s", mqtt_err_str[err]);
        return PASS;
    } else if (err == MQTT_RES_REQUIRE_CONN_FAIL) {
        ESP_LOGW(TAG, "%s (retry)", mqtt_err_str[err]);
        return WAIT;
    }

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);
    return FAIL;
}

eModemResult mqttSetPublishTopic(int index, char* topic, int len)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_TOPIC,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), ">", "ERROR", 1000) < 0)
        return WAIT;

    if (strchr(resp, '>') == NULL)
        return FAIL;

    if (at_send_wait(topic, resp, sizeof(resp), "OK", "ERROR", 1000) < 0)
        return WAIT;
    
    if (!strncmp(resp, "OK", sizeof("OK") - 1))    
        return PASS; 

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);
    return FAIL;
}

eModemResult mqttSetPayload(int index, char* msg, int len)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PAYLOAD,
            index, len);

    if (at_send_wait(cmd, resp, sizeof(resp), ">", "ERROR", 1000) < 0)
        return WAIT;

    if (strchr(resp, '>') == NULL)
        return FAIL;

    if (at_send_wait(msg, resp, sizeof(resp), "OK", "ERROR", 1000) < 0)
        return WAIT;
    
    if (!strncmp(resp, "OK", sizeof("OK") - 1))    
        return PASS; 

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);
    return FAIL;
}

eModemResult mqttPublish(int index, int qos, int pub_timeout)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PUBLISH,
            index, qos, pub_timeout);

    if (at_send_wait(cmd, resp, sizeof(resp), "CMQTTPUB", NULL, 500) < 0)
        return WAIT;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    if (err == MQTT_RES_OK)
        return PASS;

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);
    return FAIL;
}

eModemResult mqttSubscribeTopic(int index, char* topic, int len, int qos)
{
    char resp[RESP_BUFFER_SIZE] = {0};
    char cmd[CMD_BUFFER_SIZE] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_SUBSCRIBE,
            index, len, qos);

    if (at_send_wait(cmd, resp, sizeof(resp), ">", "ERROR", 500) < 0)
        return WAIT;

    if (strchr(resp, '>') == NULL)
        return FAIL;

    if (at_send_wait(topic, resp, sizeof(resp), "CMQTTSUB", NULL, 500) < 0)
        return WAIT;

    char* str = strchr(resp, ',');
    if (str == NULL)
        return FAIL;

    str += 1;

    eMqttError err = atoi(str);

    if (err == MQTT_RES_OK)
        return PASS;

    ESP_LOGE(TAG, "%s", mqtt_err_str[err]);   
    return FAIL;
}
