/**
 * @file    mqtt.h
 * @brief   It orchestrates the low-level functions from at_mqtt and provides a simpler interface for upper layers
 */
#ifndef _MQTT_H_
#define _MQTT_H_

enum ClientIndex {
    FIRST,
    SECOND
};

enum ServerType {
    TCP,
    SSL_TLS
};

typedef enum ServerType  eServerType;
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
 * @brief Start the MQTT session on the modem.
 * @return none.
 */
void mqttStartSession();

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
 * @brief Create an MQTT client on the modem.
 * @return none.
 */
void mqttCreateClient(void);

/**
 * @brief Connect the MQTT client to the configured broker.
 * @return none.
 */
void mqttConnectBroker(void);

/**
 * @brief Set the publish topic for the current MQTT session.
 * @return none.
 */
void mqttSetTopic(void);

/**
 * @brief Publish a message to the previously configured topic.
 * @param msg Pointer to message data buffer.
 * @param msg_len Length of the message in bytes.
 * @return none.
 */
void mqttPulishMessage(char* msg, int msg_len);

#endif