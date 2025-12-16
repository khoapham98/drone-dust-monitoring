/**
 * @file    sim.c
 * @brief   SIM state handlers for FSM control and state transition logic
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sys/log.h"
#include "sim_cmd.h"
#include "sim.h"

static mqttClient client = {0};
static mqttServer server = {0};
static mqttPubMsg message = {0};

eSimState preState = STATE_RESET;
eSimState simState = STATE_RESET;

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

static void updateSimState(eSimResult res, eSimState backState, eSimState nextState)
{
    if (res == FAIL) {
        simState = preState;
        preState = backState;
    } else if (res == PASS) {
        preState = simState;
        simState = nextState;
    } else {
        sleep(1);
    }
}

void simResetStatusHandler(void)
{
    while (1) {
        if (simCheckAlive() == PASS)
            break;
        sleep(1);
    }

    eSimResult res = mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);

    if (res == PASS)
        mqttReleaseClient(client.index);

    updateSimState(PASS, STATE_RESET, STATE_AT_SYNC);
} 

void atSyncStatusHandler(void)
{
    eSimResult res = PASS;
    if (simCheckAlive() == FAIL || simEchoOff() == FAIL) {
        res = FAIL;
    }

    updateSimState(res, STATE_RESET, STATE_SIM_READY);
}

void simReadyStatusHandler(void)
{
    eSimResult res = simCheckReady();
    updateSimState(res, STATE_RESET, STATE_NET_READY);
}

void netReadyStatusHandler(void)
{
    eSimResult res = simCheckRegEps();
    updateSimState(res, STATE_AT_SYNC, STATE_PDP_ACTIVE);
}

void pdpActiveStatusHandler(void)
{
    eSimResult res = simSetPdpContext();
    if (res != PASS)    
        goto end;

    res = simAttachGprs();
    if (res != PASS)
        goto end;

    res = simActivatePdp();

end:
    updateSimState(res, STATE_SIM_READY, STATE_MQTT_START);
}

void mqttStartStatusHandler(void)
{
    eSimResult res = mqttStartService();
    updateSimState(res, STATE_NET_READY, STATE_MQTT_ACCQ);
}

void mqttAccquiredStatusHandler(void)
{
    eSimResult res = mqttAcquireClient(client.index, client.ID, server.type);

    if (res != FAIL) {
        updateSimState(res, STATE_PDP_ACTIVE, STATE_MQTT_CONNECT);
        return;
    }

    mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);
    mqttReleaseClient(client.index);
    updateSimState(FAIL, STATE_PDP_ACTIVE, STATE_MQTT_CONNECT);
}

void mqttConnectedStatusHandler(void)
{
    eSimResult res = mqttConnect(&client, &server);
    updateSimState(res, STATE_MQTT_START, STATE_MQTT_READY);
}

void mqttReadyStatusHandler(char* msg, int len)
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
    updateSimState(res, STATE_MQTT_ACCQ, STATE_MQTT_READY);
}
