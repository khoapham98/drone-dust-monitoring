/**
 * @file    fsm.h
 * @brief   Finite State Machine coordinator for SIM and transport layers
 */
#ifndef _FSM_H_
#define _FSM_H_

enum simState {
    SIM_STATE_RESET,
    SIM_STATE_AT_SYNC,
    SIM_STATE_SIM_READY,
    SIM_STATE_NET_READY,
    SIM_STATE_PDP_ACTIVE,        
};

enum mqttState {
    MQTT_STATE_RESET,
    MQTT_STATE_START,
    MQTT_STATE_ACCQ,
    MQTT_STATE_CONNECT,
    MQTT_STATE_READY
};

enum fsmLayer {
    FSM_LAYER_SIM,
    FSM_LAYER_TRANSPORT
};

enum transportType {
    TRANSPORT_HTTP,
    TRANSPORT_MQTT
};

typedef enum fsmLayer  eFsmLayer; 
typedef enum simState  eSimState;
typedef enum mqttState eMqttState;
typedef enum transportType eTransportType;

struct fsm_context_t
{
    eFsmLayer layer;
    eSimState simState;
    eTransportType transType;
    eMqttState mqttState;
};

typedef struct fsm_context_t fsm_ctx_t;

/**
 * @brief Main FSM dispatcher, handle current layer and state.
 * @return none.
 */
void fsmHandler(void);

/**
 * @brief Set current FSM layer.
 * @param layer FSM layer to switch to.
 * @return none.
 */
void setFsmLayer(eFsmLayer layer);

/**
 * @brief Set current SIM state.
 * @param state SIM state to set.
 * @return none.
 */
void setSimState(eSimState state);

/**
 * @brief Get current SIM state.
 * @return Current SIM state.
 */
eSimState getSimState(void);

/**
 * @brief Set current MQTT state.
 * @param state MQTT state to set.
 * @return none.
 */
void setMqttState(eMqttState state);

/**
 * @brief Get current MQTT state.
 * @return Current MQTT state.
 */
eMqttState getMqttState(void);

/**
 * @brief Initialize FSM context and default states.
 * @return none.
 */
void fsm_context_init(void);

#endif