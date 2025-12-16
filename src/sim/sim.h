/**
 * @file    sim.h
 * @brief   SIM state handlers for FSM control and state transition logic
 */
#ifndef _SIM_H_
#define _SIM_H_

enum ClientIndex {
    FIRST,
    SECOND
};

enum ServerType {
    TCP,
    SSL_TLS
};

enum simState {
    STATE_RESET,
    STATE_AT_SYNC,
    STATE_SIM_READY,
    STATE_NET_READY,
    STATE_PDP_ACTIVE,        
    STATE_MQTT_START,
    STATE_MQTT_ACCQ,
    STATE_MQTT_CONNECT,
    STATE_MQTT_READY
};

typedef enum simState eSimState;
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

typedef struct ClientConfig mqttClient;
typedef struct ServerConfig mqttServer;
typedef struct PublishMessageConfig mqttPubMsg;

#define     CLIENT_ID_DEFAULT           "DefaultClient"
#define     SERVER_ADDR_DEFAULT         "tcp://test.mosquitto.org:1883"

#define     CLIENT_ID_MIN_LEN_BYTE      1
#define     CLIENT_ID_MAX_LEN_BYTE      128  

#define     SERVER_ADDR_MIN_LEN_BYTE    9
#define     SERVER_ADDR_MAX_LEN_BYTE    256

#define     TOPIC_PUB_MIN_LEN_BYTE      1
#define     TOPIC_PUB_MAX_LEN_BYTE      1024

#define     MESSAGE_MIN_LEN_BYTE        1
#define     MESSAGE_MAX_LEN_BYTE        10240

#define     MQTT_KEEPALIVE_30S          30
#define     MQTT_KEEPALIVE_60S          60
#define     MQTT_KEEPALIVE_120S         120
#define     MQTT_KEEPALIVE_300S         300
#define     MQTT_KEEPALIVE_600S         600

#define     PUBLISH_TIMEOUT_30S         30
#define     PUBLISH_TIMEOUT_60S         60
#define     PUBLISH_TIMEOUT_120S        120
#define     PUBLISH_TIMEOUT_180S        180

#define     MQTT_CLEAN_SESSION          1   
#define     MQTT_PERSIST_SESSION        0   

#define     MQTT_QOS_0    0         // At most once 
#define     MQTT_QOS_1    1         // At least once 
#define     MQTT_QOS_2    2         // Exactly once 

/**
 * @brief Initialize an MQTT client structure with default values.
 * @param cli Pointer to mqttClient structure to initialize.
 * @return none.
 */
void mqttClientInit(mqttClient* cli);

/**
 * @brief Initialize MQTT server configuration.
 * @param ser Pointer to mqttServer structure to initialize.
 * @return none.
 */
void mqttServerInit(mqttServer* ser);

/**
 * @brief Initialize MQTT publish message configuration.
 * @param msg Pointer to mqttPubMsg structure to initialize.
 * @return none.
 */
void mqttPublishMessageConfig(mqttPubMsg* msg);

/**
 * @brief Handle SIM reset state.
 * @return none.
 */
void simResetStatusHandler(void);

/**
 * @brief Handle AT synchronization state.
 * @return none.
 */
void atSyncStatusHandler(void);

/**
 * @brief Handle SIM ready check state.
 * @return none.
 */
void simReadyStatusHandler(void);

/**
 * @brief Handle network registration state.
 * @return none.
 */
void netReadyStatusHandler(void);

/**
 * @brief Handle PDP activation state.
 * @return none.
 */
void pdpActiveStatusHandler(void);

/**
 * @brief Handle MQTT service start state.
 * @return none.
 */
void mqttStartStatusHandler(void);

/**
 * @brief Handle MQTT client acquire state.
 * @return none.
 */
void mqttAccquiredStatusHandler(void);

/**
 * @brief Handle MQTT connection state.
 * @return none.
 */
void mqttConnectedStatusHandler(void);

/**
 * @brief Handle MQTT ready/data state.
 * @param msg Pointer to received message buffer.
 * @param len Length of received message.
 * @return none.
 */
void mqttReadyStatusHandler(char* msg, int len);

#endif