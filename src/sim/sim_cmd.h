/**
 * @file    sim_cmd.h
 * @brief   High-level SIM API header file built on top of the AT command driver
 */
#ifndef _SIM_CMD_H_
#define _SIM_CMD_H_
#include "transport/mqtt.h"

#define     VIETTEL         1
#define     MOBIFONE        0
#define     VINAPHONE       0

#define     CMD_LEN         512

enum sim_cmd_t {
    CMD_RESET = 0,        // Module reset / ATREADY
    CMD_SYNC,             // AT sync cơ bản
    CMD_SIM_READY,        // CPIN ready
    CMD_NET_READY,        // CEREG registered
    CMD_ACTIVATE_PDP,       // PDP context active
    CMD_MQTT_START,       // MQTT service started
    CMD_MQTT_ACCQ,        // MQTT client acquired
    CMD_MQTT_CONNECTED,   // MQTT connected to broker
    CMD_MQTT_READY,       // Ready to publish (logical state)
};

enum sim_result {
    PASS,
    WAIT,
    FAIL
};

enum mqtt_result {
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

typedef enum sim_cmd_t  eSimCmd;
typedef enum sim_result eSimResult;
typedef enum mqtt_result eMqttResult;

/* SWITCH BETWEEN MODES */
#define CMD_ENTER_CMD_MODE          "+++\r\n"
#define CMD_ENTER_DATA_MODE         "ATO\r\n"

/* BASIC CHECK */
#define AT_CMD_BASIC_CHECK          "AT\r\n"
#define AT_CMD_ECHO_ON              "ATE1\r\n"
#define AT_CMD_ECHO_OFF             "ATE0\r\n"

/* SIM & SIGNAL */
#define AT_CMD_READ_ICCID           "AT+CICCID\r\n"
#define AT_CMD_CHECK_READY          "AT+CPIN?\r\n"
#define AT_CMD_CHECK_SIGNAL         "AT+CSQ\r\n"
#define AT_CMD_CHECK_REG_EPS        "AT+CEREG?\r\n"
#define AT_CMD_CHECK_PDP_CONTEXT    "AT+CGDCONT?\r\n"

/* PDP CONTEXT / PACKET DATA */
#define AT_CMD_SET_PDP_CONTEXT      "AT+CGDCONT=1,\"IP\",\"%s\"\r\n"   // APN
#define AT_CMD_ATTACH_GPRS          "AT+CGATT=1\r\n"
#define AT_CMD_DETACH_GPRS          "AT+CGATT=0\r\n"
#define AT_CMD_CHECK_ATTACH_GPRS    "AT+CGATT?\r\n"
#define AT_CMD_ACTIVATE_PDP         "AT+CGACT=1,1\r\n"
#define AT_CMD_DEACTIVATE_PDP       "AT+CGACT=0,1\r\n"
#define AT_CMD_CHECK_PDP_ACTIVE     "AT+CGACT?\r\n"
#define AT_CMD_GET_IP_ADDR          "AT+CGPADDR=1\r\n"
#define AT_CMD_MQTT_START           "AT+CMQTTSTART\r\n"
#define AT_CMD_MQTT_STOP            "AT+CMQTTSTOP\r\n"

/* MQTT */
#define AT_CMD_MQTT_ACQUIRE          "AT+CMQTTACCQ=%d,\"%s\",%d\r\n"   
#define AT_CMD_MQTT_RELEASE          "AT+CMQTTREL=%d\r\n"
#define AT_CMD_MQTT_SSL_CFG          "AT+CMQTTSSLCFG\r\n"

#define AT_CMD_MQTT_CONNECT          "AT+CMQTTCONNECT=%d,\"%s\",%d,%d,\"%s\",\"%s\"\r\n"
#define AT_CMD_MQTT_DISCONNECT       "AT+CMQTTDISC=%d,%d\r\n"

#define AT_CMD_MQTT_TOPIC            "AT+CMQTTTOPIC=%d,%d\r\n"
#define AT_CMD_MQTT_PAYLOAD          "AT+CMQTTPAYLOAD=%d,%d\r\n"
#define AT_CMD_MQTT_PUBLISH          "AT+CMQTTPUB=%d,%d,%d\r\n"

/******************************************************************************/
/* Basic AT operations */
/******************************************************************************/

/**
 * @brief Enter command mode using "+++".
 * @return 0 on success, -1 on failure.
 */
int simEnterCmdMode(void);

/**
 * @brief Enter data mode using "ATO".
 * @return 0 on success, -1 on failure.
 */
int simEnterDataMode(void);

/**
 * @brief Check if the module is alive (AT → OK).
 * @return PASS if response OK,
 *         FAIL if response ERROR,
 *         WAIT if command cannot be sent or response is incomplete.
 */
eSimResult simCheckAlive(void);

/**
 * @brief Enable AT command echo (ATE1).
 * @return PASS if echo enabled successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simEchoOn(void);

/**
 * @brief Disable AT command echo (ATE0).
 * @return PASS if echo disabled successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simEchoOff(void);

/**
 * @brief Check SIM card readiness (AT+CPIN?).
 * @return PASS if SIM is ready,
 *         FAIL if SIM error or PIN required,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simCheckReady(void);

/**
 * @brief Check EPS/LTE registration status (AT+CEREG?).
 * @return PASS if registered (stat = 1 or 5),
 *         FAIL if not registered,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simCheckRegEps(void);

/**
 * @brief Set PDP context APN (AT+CGDCONT).
 * @return PASS if PDP context set successfully,
 *         FAIL if module returns error,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simSetPdpContext(void);

/**
 * @brief Attach to GPRS service (AT+CGATT=1).
 * @return PASS if attached successfully,
 *         FAIL if attach rejected,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simAttachGprs(void);

/**
 * @brief Activate PDP context (AT+CGACT=1,1).
 * @return PASS if PDP activated,
 *         FAIL if activation failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult simActivatePdp(void);

/******************************************************************************/
/* MQTT */
/******************************************************************************/

/**
 * @brief Start MQTT service.
 * @return PASS if service started,
 *         FAIL if start rejected or already failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttStartService(void);

/**
 * @brief Release an MQTT client.
 * @param index Client index.
 * @return PASS if client released,
 *         FAIL if release failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttReleaseClient(int index);

/**
 * @brief Acquire an MQTT client.
 * @param index Client index.
 * @param id Pointer to client ID.
 * @param type Server type.
 * @return PASS if client acquired successfully,
 *         FAIL if acquire rejected,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttAcquireClient(int index, char* id, int type);

/**
 * @brief Connect to MQTT server.
 * @param cli Pointer to MQTT client.
 * @param ser Pointer to MQTT server.
 * @return PASS if connected successfully,
 *         FAIL if connection failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttConnect(mqttClient* cli, mqttServer* ser);

/**
 * @brief Disconnect from MQTT server.
 * @param index Client index.
 * @param timeout The timeout value for disconnection.
 * @return PASS if connected successfully,
 *         FAIL if connection failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttDisconnect(int index, int timeout);

/**
 * @brief Set MQTT publish topic.
 * @param index Client index.
 * @param topic Pointer to topic string.
 * @param len Topic length.
 * @return PASS if topic accepted,
 *         FAIL if topic rejected,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttSetPublishTopic(int index, char* topic, int len);

/**
 * @brief Set MQTT publish payload.
 * @param index Client index.
 * @param msg Pointer to payload data.
 * @param len Payload length.
 * @return PASS if payload accepted,
 *         FAIL if payload rejected,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttSetPayload(int index, char* msg, int len);

/**
 * @brief Publish MQTT message.
 * @param index Client index.
 * @param QoS Publish QoS level.
 * @param pub_timeout Publish timeout value.
 * @return PASS if publish successful,
 *         FAIL if publish failed,
 *         WAIT if command send failed or response not ready.
 */
eSimResult mqttPublish(int index, int QoS, int pub_timeout);

#endif