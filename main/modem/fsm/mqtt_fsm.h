/**
 * @file    mqtt_fsm.h
 * @brief   MQTT state handlers for FSM control and state transition logic
 */
#ifndef _MQTT_FSM_H_
#define _MQTT_FSM_H_

enum ClientIndex {
    FIRST,
    SECOND
};

enum ServerType {
    TCP,
    SSL_TLS
};

typedef enum ServerType eServerType;
typedef enum ClientIndex eIndex;

struct ClientConfig {
    eIndex index;
    char* ID; 
    char* userName;
    char* password;
    int keepAliveTime;
    int cleanSession;
};

struct ServerConfig {
    eServerType type;
    char* addr;
};

struct PublishMessageConfig {
    char* topic;
    int topicLength;
    int qos;
    int publishTimeout;
};

struct SubscribeMessageConfig {
    char* topic;
    int topicLength;
    int qos;
};

enum mqttState {
    MQTT_STATE_RESET,
    MQTT_STATE_START,
    MQTT_STATE_ACCQ,
    MQTT_STATE_CONNECT,
    MQTT_STATE_READY
};

typedef enum mqttState eMqttState;
typedef struct ClientConfig mqttClient;
typedef struct ServerConfig mqttServer;
typedef struct PublishMessageConfig mqttPubMsg;
typedef struct SubscribeMessageConfig mqttSubMsg;

#define PAYLOAD_BUFFER_SIZE         160
#define MESSAGE_BUFFER_SIZE         4096

#define CLIENT_ID_DEFAULT           "DefaultClient"
#define SERVER_ADDR_DEFAULT         "tcp://test.mosquitto.org:1883"

#define CLIENT_ID_MIN_LEN_BYTE      1
#define CLIENT_ID_MAX_LEN_BYTE      128  

#define SERVER_ADDR_MIN_LEN_BYTE    9
#define SERVER_ADDR_MAX_LEN_BYTE    256

#define MQTT_KEEPALIVE_30S          30
#define MQTT_KEEPALIVE_60S          60
#define MQTT_KEEPALIVE_120S         120
#define MQTT_KEEPALIVE_300S         300
#define MQTT_KEEPALIVE_600S         600

#define DISCONNECT_TIMEOUT_30S      30
#define DISCONNECT_TIMEOUT_60S      60
#define DISCONNECT_TIMEOUT_120S     120
#define DISCONNECT_TIMEOUT_180S     180

#define PUBLISH_TIMEOUT_30S         30
#define PUBLISH_TIMEOUT_60S         60
#define PUBLISH_TIMEOUT_120S        120
#define PUBLISH_TIMEOUT_180S        180

#define MQTT_CLEAN_SESSION          1   
#define MQTT_PERSIST_SESSION        0   

/* MQTT Quality of Service */
#define MQTT_QOS_0    0         // Fire-and-forget (sensor data, high frequency)
#define MQTT_QOS_1    1         // At least once (important events, tolerate duplicates)
#define MQTT_QOS_2    2         // Exactly once (critical commands, avoid duplicates)

/**
 * @brief Handle MQTT layer FSM based on current MQTT state.
 * @param state Current MQTT state to be processed.
 * @return none.
 */
void mqttFsmHandler(eMqttState state);

/**
 * @brief   Initialize MQTT context.
 * @param   cli Pointer to mqttClient structure to initialize.
 * @param   ser Pointer to mqttServer structure to initialize.
 * @param   pubMsg Pointer to mqttPubMsg structure to initialize.
 * @param   subMsg Pointer to mqttSubMsg structure to initialize.
 * @return  none.
 */
void mqtt_context_init(mqttClient* cli, mqttServer* ser, mqttPubMsg* pubMsg, mqttSubMsg* subMsg);

#endif