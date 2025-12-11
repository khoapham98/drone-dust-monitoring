/**
 * @file    mqtt.c 
 * @brief   It orchestrates the low-level functions from at_mqtt and provides a simpler interface for upper layers
 */
#include <stdio.h>
#include <string.h>
#include "sys/log.h"
#include "at_cmd.h"
#include "at_mqtt.h"
#include "mqtt.h"

static mqttClient client = {0};
static mqttServer server = {0};
static mqttPubMsg message = {0};

void mqttStartSession(void)
{
    int err = mqttStartService();
    if (err < 0)
        LOG_ERR("Start MQTT service failed");
}

void mqttClientInit(mqttClient* cli)
{
    client.index = cli->index; 
    client.keepAliveTime = cli->keepAliveTime;
    client.cleanSession  = cli->cleanSession;

    if (cli->ID != NULL && 
    strlen(cli->ID) >= CLIENT_ID_MIN_LEN_BYTE && 
    strlen(cli->ID) < CLIENT_ID_MAX_LEN_BYTE) {
        client.ID = cli->ID;
    } else {
        client.ID = CLIENT_ID_DEFAULT;
        LOG_WRN("Using default client ID: %s", client.ID);
    }

    if (cli->userName != NULL)
        client.userName = cli->userName;
    else 
        client.userName = "";

    if (cli->password != NULL)
        client.password = cli->password;
    else
        client.password = "";
}

void mqttServerInit(mqttServer* ser)
{
    server.type = ser->type;

    if (ser->addr != NULL && 
    strlen(ser->addr) >= SERVER_ADDR_MIN_LEN_BYTE && strlen(ser->addr) < SERVER_ADDR_MAX_LEN_BYTE) {
        server.addr = ser->addr;
        LOG_INF("Set server address to: %s", server.addr);
    } else {
        server.addr = SERVER_ADDR_DEFAULT;
        LOG_WRN("Using default server address: %s", server.addr);
    }
}

void mqttPublishMessageConfig(mqttPubMsg* msg)
{
    if (msg->topic == NULL) {
        LOG_WRN("Topic name is missing!!!");
        LOG_ERR("Configure publish message failed");
        return;
    }

    message.topic = msg->topic;
    message.topicLength = msg->topicLength;
    message.qos = msg->qos;
    message.publishTimeout = msg->publishTimeout;
}

void mqttCreateClient(void)
{
    int ret = mqttAcquireClient(client.index, client.ID, server.type);
    if (ret < 0)
        LOG_ERR("Create client failed");
}

void mqttConnectBroker(void)
{
    int ret = mqttConnect(&client, &server);
    if (ret < 0) 
        LOG_ERR("Connect to broker failed");
}

void mqttSetTopic(void)
{
    char resp[128] = {0};
    at_attach_resp_buffer(resp, sizeof(resp));

    mqttSetPublishTopic(client.index, message.topicLength);

    if (strchr(resp, '>') == NULL) {
        LOG_ERR("Set publish topic failed");
        at_attach_resp_buffer(NULL, 0);
        return;
    }

    sendInputData(message.topic);

    at_attach_resp_buffer(NULL, 0);
}

void mqttPulishMessage(char* msg, int msg_len)
{
    mqttSetTopic();

    int req_len = msg_len;
    if (req_len < MESSAGE_MIN_LEN_BYTE || req_len > MESSAGE_MAX_LEN_BYTE) {
        LOG_WRN("Message length invalid");
        req_len = MESSAGE_MAX_LEN_BYTE / 2;
    }

    char resp[128] = {0};
    at_attach_resp_buffer(resp, sizeof(resp));

    mqttSetPayload(client.index, req_len);

    if (strchr(resp, '>') == NULL) {
        LOG_ERR("input message failed");
        at_attach_resp_buffer(NULL, 0);
        return;
    }

    sendInputData(msg);

    mqttPublish(client.index, message.qos, message.publishTimeout);

    at_attach_resp_buffer(NULL, 0);
}