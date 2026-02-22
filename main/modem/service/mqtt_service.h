/**
 * @file    mqtt_service.h
 * @brief   MQTT protocol operations over cellular modem
 */
#ifndef _MQTT_SERVICE_H_
#define _MQTT_SERVICE_H_
#include "modem_common.h"
#include "mqtt_fsm.h"

enum mqtt_err {
    MQTT_RES_OK = 0,
    MQTT_RES_FAILED = 1,
    MQTT_RES_BAD_UTF8 = 2,
    MQTT_RES_SOCK_CONNECT_FAIL = 3,
    MQTT_RES_SOCK_CREATE_FAIL = 4,
    MQTT_RES_SOCK_CLOSE_FAIL = 5,
    MQTT_RES_MSG_RECV_FAIL = 6,
    MQTT_RES_NET_OPEN_FAIL = 7,
    MQTT_RES_NET_CLOSE_FAIL = 8,
    MQTT_RES_NET_NOT_OPENED = 9,
    MQTT_RES_CLIENT_INDEX_ERR = 10,
    MQTT_RES_NO_CONNECTION = 11,
    MQTT_RES_INVALID_PARAM = 12,
    MQTT_RES_NOT_SUPPORTED = 13,
    MQTT_RES_CLIENT_BUSY = 14,
    MQTT_RES_REQUIRE_CONN_FAIL = 15,
    MQTT_RES_SOCK_SEND_FAIL = 16,
    MQTT_RES_TIMEOUT = 17,
    MQTT_RES_TOPIC_EMPTY = 18,
    MQTT_RES_CLIENT_USED = 19,
    MQTT_RES_CLIENT_NOT_ACQUIRED = 20,
    MQTT_RES_CLIENT_NOT_RELEASED = 21,
    MQTT_RES_LEN_OUT_OF_RANGE = 22,
    MQTT_RES_NET_ALREADY_OPENED = 23,
    MQTT_RES_PACKET_FAIL = 24,
    MQTT_RES_DNS_ERR = 25,
    MQTT_RES_SOCK_CLOSED_BY_SERVER = 26,
    MQTT_RES_CONN_REFUSED_PROTO = 27,
    MQTT_RES_CONN_REFUSED_ID = 28,
    MQTT_RES_CONN_REFUSED_UNAVAILABLE = 29,
    MQTT_RES_CONN_REFUSED_BAD_AUTH = 30,
    MQTT_RES_CONN_REFUSED_NOT_AUTH = 31,
    MQTT_RES_HANDSHAKE_FAIL = 32,
    MQTT_RES_CERT_NOT_SET = 33,
    MQTT_RES_OPEN_SESSION_FAIL = 34,
    MQTT_RES_DISCONNECT_FAIL = 35
};

typedef enum mqtt_err eMqttError;

#define AT_CMD_MQTT_START           "AT+CMQTTSTART\r\n"
#define AT_CMD_MQTT_STOP            "AT+CMQTTSTOP\r\n"
#define AT_CMD_MQTT_ACQUIRE         "AT+CMQTTACCQ=%d,\"%s\",%d\r\n"   
#define AT_CMD_MQTT_RELEASE         "AT+CMQTTREL=%d\r\n"
#define AT_CMD_MQTT_SSL_CFG         "AT+CMQTTSSLCFG\r\n"
#define AT_CMD_MQTT_CONNECT         "AT+CMQTTCONNECT=%d,\"%s\",%d,%d,\"%s\",\"%s\"\r\n"
#define AT_CMD_MQTT_DISCONNECT      "AT+CMQTTDISC=%d,%d\r\n"
#define AT_CMD_MQTT_TOPIC           "AT+CMQTTTOPIC=%d,%d\r\n"
#define AT_CMD_MQTT_PAYLOAD         "AT+CMQTTPAYLOAD=%d,%d\r\n"
#define AT_CMD_MQTT_PUBLISH         "AT+CMQTTPUB=%d,%d,%d\r\n"
#define AT_CMD_MQTT_SUBSCRIBE       "AT+CMQTTSUB=%d,%d,%d\r\n"

/**
 * @brief Stop MQTT service.
 * @return PASS if service stoped,
 *         FAIL if stoped rejected or already failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttStopService(void);

/**
 * @brief Start MQTT service.
 * @return PASS if service started,
 *         FAIL if start rejected or already failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttStartService(void);

/**
 * @brief Release an MQTT client.
 * @param index Client index.
 * @return PASS if client released,
 *         FAIL if release failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttReleaseClient(int index);

/**
 * @brief Acquire an MQTT client.
 * @param index Client index.
 * @param id Pointer to client ID.
 * @param type Server type.
 * @return PASS if client acquired successfully,
 *         FAIL if acquire rejected,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttAcquireClient(int index, char* id, int type);

/**
 * @brief Connect to MQTT server.
 * @param cli Pointer to MQTT client.
 * @param ser Pointer to MQTT server.
 * @return PASS if connected successfully,
 *         FAIL if connection failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttConnect(mqttClient* cli, mqttServer* ser);

/**
 * @brief Disconnect from MQTT server.
 * @param index Client index.
 * @param timeout The timeout value for disconnection.
 * @return PASS if connected successfully,
 *         FAIL if connection failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttDisconnect(int index, int timeout);

/**
 * @brief Set MQTT publish topic.
 * @param index Client index.
 * @param topic Pointer to topic string.
 * @param len Topic length.
 * @return PASS if topic accepted,
 *         FAIL if topic rejected,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttSetPublishTopic(int index, char* topic, int len);

/**
 * @brief Set MQTT publish payload.
 * @param index Client index.
 * @param msg Pointer to payload data.
 * @param len Payload length.
 * @return PASS if payload accepted,
 *         FAIL if payload rejected,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttSetPayload(int index, char* msg, int len);

/**
 * @brief Publish MQTT message.
 * @param index Client index.
 * @param QoS Publish QoS level.
 * @param pub_timeout Publish timeout value.
 * @return PASS if publish successful,
 *         FAIL if publish failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttPublish(int index, int QoS, int pub_timeout);

/**
 * @brief Subscribe a message to MQTT server.
 * @param index Client index.
 * @param topic Pointer to MQTT subscribe topic string.
 * @param len Topic length.
 * @param qos The public message's QoS level.
 * @return PASS if subscribe successful,
 *         FAIL if subscribe failed,
 *         WAIT if command send failed or response not ready.
 */
eModemResult mqttSubscribeTopic(int index, char* topic, int len, int qos);
 #endif