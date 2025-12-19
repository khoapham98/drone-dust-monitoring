/**
 * @file    mqtt.c
 * @brief   MQTT state handlers for FSM control and state transition logic
 */
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/log.h"
#include "sys/json.h"
#include "ringbuffer.h"
#include "sim/sim_cmd.h"
#include "mqtt.h"
#include "fsm/fsm.h"

static const char* mqttStateStr[] = {
    "MQTT_STATE_RESET",
    "MQTT_STATE_START",
    "MQTT_STATE_ACCQ",
    "MQTT_STATE_CONNECT",
    "MQTT_STATE_READY"
};

static mqttClient client = {0};
static mqttServer server = {0};
static mqttPubMsg message = {0};

static eMqttState preState = MQTT_STATE_RESET; 

extern ring_buffer_t json_ring_buf;
extern bool jsonReady;
extern pthread_cond_t jsonCond;
extern pthread_mutex_t jsonLock;

static void updateMqttState(eSimResult res, eMqttState backState, eMqttState nextState)
{
    if (res == FAIL) {
        if (getMqttState() == MQTT_STATE_START) {
            setFsmLayer(FSM_LAYER_SIM);
            return;
        }

        setMqttState(preState);
        preState = backState;
    } 
    else if (res == PASS) {
        preState = getMqttState();
        setMqttState(nextState);
    }
    else {
        sleep(1);
    }
}

static void mqttResetStatusHandler(void)
{
    eSimResult res = mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);

    if (res == PASS)
        mqttReleaseClient(client.index);

    updateMqttState(PASS, MQTT_STATE_RESET, MQTT_STATE_START);
}

static void mqttStartStatusHandler(void)
{
    eSimResult res = mqttStartService();
    updateMqttState(res, MQTT_STATE_RESET, MQTT_STATE_ACCQ);
}

static void mqttAccquiredStatusHandler(void)
{
    eSimResult res = mqttAcquireClient(client.index, client.ID, server.type);

    if (res != FAIL) {
        updateMqttState(res, MQTT_STATE_RESET, MQTT_STATE_CONNECT);
        return;
    }

    mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);
    mqttReleaseClient(client.index);
    updateMqttState(FAIL, MQTT_STATE_RESET, MQTT_STATE_CONNECT);
}

static void mqttConnectedStatusHandler(void)
{
    eSimResult res = mqttConnect(&client, &server);
    updateMqttState(res, MQTT_STATE_START, MQTT_STATE_READY);
}

static void mqttReadyStatusHandler(char* msg, int len)
{
    if (len > 100) {
        LOG_WRN("Data package invalid (%d bytes) - skip", len);
        return;
    }

    eSimResult res = mqttSetPublishTopic(client.index, message.topic, message.topicLength);
    if (res != PASS)
        goto end;

    res = mqttSetPayload(client.index, msg, len);
    if (res != PASS)
        goto end;
   
    res = mqttPublish(client.index, message.qos, message.publishTimeout);
    if (res == PASS)
        return;

end:
    updateMqttState(res, MQTT_STATE_ACCQ, MQTT_STATE_READY);
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

void mqttFsmHandler(eMqttState state)
{
    LOG_INF("%s", mqttStateStr[state]);
    switch (state)
    {
    case MQTT_STATE_RESET:
        mqttResetStatusHandler();
        break;
    case MQTT_STATE_START:
        mqttStartStatusHandler();
        break;
    case MQTT_STATE_ACCQ:
        mqttAccquiredStatusHandler();
        break;
    case MQTT_STATE_CONNECT:
        mqttConnectedStatusHandler();
        break;
    case MQTT_STATE_READY:
        pthread_mutex_lock(&jsonLock);
        while (!jsonReady)
            pthread_cond_wait(&jsonCond, &jsonLock);

        char msg[RING_BUFFER_SIZE] = {0};
        getJsonData(&json_ring_buf, msg);
        jsonReady = false;
        pthread_mutex_unlock(&jsonLock);
        mqttReadyStatusHandler(msg, strlen(msg));
        break;
    default:
        break;
    }
}