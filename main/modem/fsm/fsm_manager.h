 /**
 * @file    fsm_manager.h
 * @brief   Central coordinator for modem finite state machines
 */
#ifndef _FSM_MANAGER_H_
#define _FSM_MANAGER_H_
#include "sim_fsm.h"
#include "mqtt_fsm.h"
#include "http_fsm.h"

enum fsmLayer {
    FSM_LAYER_SIM,
    FSM_LAYER_TRANSPORT
};

enum transportType {
    TRANSPORT_HTTP,
    TRANSPORT_MQTT
};

typedef enum fsmLayer eFsmLayer; 
typedef enum transportType eTransportType;

struct fsm_context_t {
    eFsmLayer layer;
    eSimState simState;
    eMqttState mqttState;
    eHttpState httpState;
    eTransportType transType;
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
 * @brief Set current HTTP state.
 * @param state HTTP state to set.
 * @return none.
 */
void setHttpState(eHttpState state);

/**
 * @brief Get current HTTP state.
 * @return Current HTTP state.
 */
eHttpState getHttpState(void);

/**
 * @brief Initialize FSM context and default states.
 * @return none.
 */
void fsm_context_init(void);

#endif