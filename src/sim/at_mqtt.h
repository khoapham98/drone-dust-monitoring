/**
 * @file    at_mqtt.h
 * @brief   Contains low-level APIs for sending MQTT commands from the SIM module
 */
#ifndef _AT_MQTT_H_
#define _AT_MQTT_H_
#include "mqtt.h"

#define     CMD_LEN                 512

#define AT_CMD_MQTT_START            "AT+CMQTTSTART\r\n"
#define AT_CMD_MQTT_STOP             "AT+CMQTTSTOP\r\n"

#define AT_CMD_MQTT_ACQUIRE          "AT+CMQTTACCQ=%d,\"%s\",%d\r\n"   
#define AT_CMD_MQTT_RELEASE          "AT+CMQTTREL\r\n"

#define AT_CMD_MQTT_SSL_CFG          "AT+CMQTTSSLCFG\r\n"

#define AT_CMD_MQTT_WILL_TOPIC       "AT+CMQTTWILLTOPIC\r\n"
#define AT_CMD_MQTT_WILL_MSG         "AT+CMQTTWILLMSG\r\n"

#define AT_CMD_MQTT_CONNECT          "AT+CMQTTCONNECT=%d,\"%s\",%d,%d,\"%s\",\"%s\"\r\n"
#define AT_CMD_MQTT_DISCONNECT       "AT+CMQTTDISC\r\n"

#define AT_CMD_MQTT_TOPIC            "AT+CMQTTTOPIC=%d,%d\r\n"
#define AT_CMD_MQTT_PAYLOAD          "AT+CMQTTPAYLOAD=%d,%d\r\n"
#define AT_CMD_MQTT_PUBLISH          "AT+CMQTTPUB=%d,%d,%d\r\n"

#define AT_CMD_MQTT_SUB_TOPIC        "AT+CMQTTSUBTOPIC\r\n"
#define AT_CMD_MQTT_SUBSCRIBE        "AT+CMQTTSUB\r\n"

#define AT_CMD_MQTT_UNSUB_TOPIC      "AT+CMQTTUNSUBTOPIC\r\n"
#define AT_CMD_MQTT_UNSUBSCRIBE      "AT+CMQTTUNSUB\r\n"

#define AT_CMD_MQTT_CONFIG           "AT+CMQTTCFG\r\n"

/**
 * @brief Start the MQTT service.
 * @return 0 on success, -1 on failure.
 */
int mqttStartService(void);

/**
 * @brief Stop the MQTT service.
 * @return 0 on success, -1 on failure.
 */
int mqttStopService(void);

/**
 * @brief Acquire an MQTT client.
 * @param index Client index.
 * @param id Pointer to Client ID.
 * @param type Server type.
 * @return 0 on success, -1 on failure.
 */
int mqttAcquireClient(int index, char* id, int type);

/**
 * @brief Release an MQTT client.
 * @return 0 on success, -1 on failure.
 */
int mqttReleaseClient(void);

/**
 * @brief Set the SSL context for MQTT (only for SSL/TLS).
 * @return 0 on success, -1 on failure.
 */
int mqttSetSSL(void);

/**
 * @brief Set the topic for the MQTT will message.
 * @return 0 on success, -1 on failure.
 */
int mqttSetWillTopic(void);

/**
 * @brief Set the content for the MQTT will message.
 * @return 0 on success, -1 on failure.
 */
int mqttSetWillMessage(void);

/**
 * @brief Connect to the MQTT server.
 * @param cli Pointer to MQTT client.
 * @param ser Pointer to MQTT server.
 * @return 0 on success, -1 on failure.
 */
int mqttConnect(mqttClient* cli, mqttServer* ser);

/**
 * @brief Disconnect from the MQTT server.
 * @return 0 on success, -1 on failure.
 */
int mqttDisconnect(void);

/**
 * @brief Set the topic for MQTT publish.
 * @param index Client index.
 * @param len Topic length.
 * @return 0 on success, -1 on failure.
 */
int mqttSetPublishTopic(int index, int len);

/**
 * @brief Register the MQTT topic.
 * @param data Pointer to the data to send.
 * @return 0 on success, -1 on failure.
 */
int sendInputData(char* data);

/**
 * @brief Set the payload for MQTT publish.
 * @param index Client index.
 * @param len Message length.
 * @return 0 on success, -1 on failure.
 */
int mqttSetPayload(int index, int len);

/**
 * @brief Publish a message to the MQTT server.
 * @param index Client index.
 * @param QoS The publish message's qos.
 * @param pub_timeout The publishing timeout interval value.
 * @return 0 on success, -1 on failure.
 */
int mqttPublish(int index, int QoS, int pub_timeout);

/**
 * @brief Set the topic for MQTT subscribe.
 * @return 0 on success, -1 on failure.
 */
int mqttSetSubscribeTopic(void);

/**
 * @brief Subscribe to a topic on the MQTT server.
 * @return 0 on success, -1 on failure.
 */
int mqttSubscribe(void);

/**
 * @brief Set the topic for MQTT unsubscribe.
 * @return 0 on success, -1 on failure.
 */
int mqttSetUnsubscribeTopic(void);

/**
 * @brief Unsubscribe from a topic on the MQTT server.
 * @return 0 on success, -1 on failure.
 */
int mqttUnsubscribe(void);

/**
 * @brief Configure the MQTT context.
 * @return 0 on success, -1 on failure.
 */
int mqttConfig(void);

#endif