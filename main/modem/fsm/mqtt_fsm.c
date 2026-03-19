/**
 * @file    mqtt_fsm.c
 * @brief   MQTT state handlers for FSM control and state transition logic
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_service.h"
#include "mqtt_fsm.h"
#include "fsm_manager.h"

static const char* TAG = "mqtt_fsm";

static const char* mqttStateStr[] = {
    "MQTT_STATE_RESET",
    "MQTT_STATE_START",
    "MQTT_STATE_ACCQ",
    "MQTT_STATE_CONNECT",
    "MQTT_STATE_READY"
};

static mqttClient client = {0};
static mqttServer server = {0};
static mqttPubMsg pubMsg = {0};
static mqttSubMsg subMsg = {0};

static eMqttState preState = MQTT_STATE_RESET; 

extern MessageBufferHandle_t msgBufHandle;

static void updateMqttState(eModemResult res, eMqttState backState, eMqttState nextState)
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
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void mqttResetStatusHandler(void)
{
    eModemResult res = mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);

    if (res == PASS)
        mqttReleaseClient(client.index);
    
    mqttStopService();

    updateMqttState(PASS, MQTT_STATE_RESET, MQTT_STATE_START);
}

static void mqttStartStatusHandler(void)
{
    eModemResult res = mqttStartService();
    updateMqttState(res, MQTT_STATE_RESET, MQTT_STATE_ACCQ);
}

static void mqttAccquireStatusHandler(void)
{
    eModemResult res = mqttAcquireClient(client.index, client.ID, server.type);

    if (res != FAIL) {
        updateMqttState(res, MQTT_STATE_RESET, MQTT_STATE_CONNECT);
        return;
    }

    mqttDisconnect(client.index, DISCONNECT_TIMEOUT_180S);
    mqttReleaseClient(client.index);
    updateMqttState(FAIL, MQTT_STATE_RESET, MQTT_STATE_CONNECT);
}

static void mqttConnectStatusHandler(void)
{
    eModemResult res = mqttConnect(&client, &server);
    if (res != PASS) goto end;

    res = mqttSubscribeTopic(client.index, subMsg.topic, subMsg.topicLength, subMsg.qos);

end:
    updateMqttState(res, MQTT_STATE_START, MQTT_STATE_READY);
}

static void mqttReadyStatusHandler(void)
{
    char payloadBuffer[PAYLOAD_BUFFER_SIZE] = {0};
    int payloadLen = xMessageBufferReceive(msgBufHandle, payloadBuffer, sizeof(payloadBuffer), portMAX_DELAY);

    eModemResult res = mqttSetPublishTopic(client.index, pubMsg.topic, pubMsg.topicLength);
    if (res != PASS) goto end;

    res = mqttSetPayload(client.index, payloadBuffer, payloadLen);
    if (res != PASS) goto end;
   
    res = mqttPublish(client.index, pubMsg.qos, pubMsg.publishTimeout);
    if (res == PASS) return;

end:
    updateMqttState(res, MQTT_STATE_ACCQ, MQTT_STATE_READY);
}

static void mqttClientInit(mqttClient* cli)
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
        ESP_LOGD(TAG, "Using default client ID: %s", client.ID);
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

static void mqttServerInit(mqttServer* ser)
{
    server.type = ser->type;

    if (ser->addr != NULL && strlen(ser->addr) >= SERVER_ADDR_MIN_LEN_BYTE 
    && strlen(ser->addr) < SERVER_ADDR_MAX_LEN_BYTE) {
        server.addr = ser->addr;
        ESP_LOGD(TAG, "Set server address to: %s", server.addr);
    } else {
        server.addr = SERVER_ADDR_DEFAULT;
        ESP_LOGW(TAG, "Using default server address: %s", server.addr);
    }
}

static void mqttPublishMessageConfig(mqttPubMsg* msg)
{
    if (msg->topic == NULL) {
        ESP_LOGE(TAG, "Topic name is missing - configure publish message failed");
        return;
    }

    pubMsg.topic = msg->topic;
    pubMsg.topicLength = msg->topicLength;
    pubMsg.qos = msg->qos;
    pubMsg.publishTimeout = msg->publishTimeout;
}

static void mqttSubscribeMessageConfig(mqttSubMsg* msg)
{
    if (msg->topic == NULL) {
        ESP_LOGE(TAG, "Topic name is missing - configure subscribe message failed");
        return;
    }

    subMsg.topic = msg->topic;
    subMsg.topicLength = msg->topicLength;
    subMsg.qos = msg->qos;
}

void mqtt_context_init(mqttClient* cli, mqttServer* ser, mqttPubMsg* pubMsg, mqttSubMsg* subMsg)
{
    if (cli != NULL) mqttClientInit(cli);

    if (ser != NULL) mqttServerInit(ser);

    if (pubMsg != NULL) mqttPublishMessageConfig(pubMsg);

    if (subMsg != NULL) mqttSubscribeMessageConfig(subMsg);

    configASSERT(msgBufHandle != NULL);
}

void mqttFsmHandler(eMqttState state)
{
    ESP_LOGI(TAG, "Current state: %s", mqttStateStr[state]);
    switch (state)
    {
    case MQTT_STATE_RESET:
        mqttResetStatusHandler();
        break;
    case MQTT_STATE_START:
        mqttStartStatusHandler();
        break;
    case MQTT_STATE_ACCQ:
        mqttAccquireStatusHandler();
        break;
    case MQTT_STATE_CONNECT:
        mqttConnectStatusHandler();
        break;
    case MQTT_STATE_READY:
        mqttReadyStatusHandler();
        break;
    default:
        break;
    }
}