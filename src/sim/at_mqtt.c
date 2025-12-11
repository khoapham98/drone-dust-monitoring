/**
 * @file    at_mqtt.c 
 * @brief   Contains low-level APIs for sending MQTT commands from the SIM module
 */
#include <stdio.h>
#include "at_cmd.h"
#include "at_mqtt.h"
#include "mqtt.h"

int mqttStartService(void)
{
    return at_send_wait(AT_CMD_MQTT_START, 2000);
}

int mqttStopService(void)
{
    return at_send_wait(AT_CMD_MQTT_STOP, 2000);
}

int mqttAcquireClient(int index, char* id, int type)
{
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd), 
            AT_CMD_MQTT_ACQUIRE, 
            index, id, type);
    
    return at_send_wait(cmd, 2000);
}

int mqttReleaseClient(void)
{
    return at_send_wait(AT_CMD_MQTT_RELEASE, 2000);
}

int mqttSetSSL(void)
{
    return at_send_wait(AT_CMD_MQTT_SSL_CFG, 2000);
}

int mqttSetWillTopic(void)
{
    return at_send_wait(AT_CMD_MQTT_WILL_TOPIC, 2000);
}

int mqttSetWillMessage(void)
{
    return at_send_wait(AT_CMD_MQTT_WILL_MSG, 2000);
}

int mqttConnect(mqttClient* cli, mqttServer* ser)
{
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_CONNECT,
            cli->index, ser->addr, cli->keepAliveTime, 
            cli->cleanSession, cli->userName, cli->password);

    return at_send_wait(cmd, 2000);
}

int mqttDisconnect(void)
{
    return at_send_wait(AT_CMD_MQTT_DISCONNECT, 2000);
}

int mqttSetPublishTopic(int index, int len)
{
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_TOPIC,
            index, len);

    return at_send_wait(cmd, 2000);
}

int mqttSetPayload(int index, int len)
{
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PAYLOAD,
            index, len);

    return at_send_wait(cmd, 2000);
}

int mqttPublish(int index, int QoS, int pub_timeout)
{
    char cmd[CMD_LEN] = {0};
    snprintf(cmd, sizeof(cmd),
            AT_CMD_MQTT_PUBLISH,
            index, QoS, pub_timeout);

    return at_send_wait(cmd, 2000);
}

int sendInputData(char* data)
{
    return at_send_wait(data, 1000);
}

int mqttSetSubscribeTopic(void)
{
    return at_send_wait(AT_CMD_MQTT_SUB_TOPIC, 2000);
}

int mqttSubscribe(void)
{
    return at_send_wait(AT_CMD_MQTT_SUBSCRIBE, 2000);
}

int mqttSetUnsubscribeTopic(void)
{
    return at_send_wait(AT_CMD_MQTT_UNSUB_TOPIC, 2000);
}

int mqttUnsubscribe(void)
{
    return at_send_wait(AT_CMD_MQTT_UNSUBSCRIBE, 2000);
}

int mqttConfig(void)
{
    return at_send_wait(AT_CMD_MQTT_CONFIG, 2000);
}
