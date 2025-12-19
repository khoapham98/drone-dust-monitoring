/**
 * @file    fsm.c
 * @brief   Finite State Machine coordinator for SIM and transport layers
 */
#include "sim/sim.h"
#include "transport/mqtt.h"
#include "fsm.h"

static fsm_ctx_t ctx = {0};

void fsmHandler(void)
{
    switch (ctx.layer)
    {
    case FSM_LAYER_SIM:
        simFsmHandler(ctx.simState);
        break;
    
    case FSM_LAYER_TRANSPORT:
        switch (ctx.transType)
        {
        case TRANSPORT_HTTP:
            /* code */
            break;
        case TRANSPORT_MQTT:
            mqttFsmHandler(ctx.mqttState);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

void setFsmLayer(eFsmLayer layer)
{
    ctx.layer = layer;
}

void setSimState(eSimState state)
{
    ctx.simState = state;
}

eSimState getSimState(void)
{
    return ctx.simState;
}

void setMqttState(eMqttState state)
{
    ctx.mqttState = state;
}

eMqttState getMqttState(void)
{
    return ctx.mqttState;
}

void fsm_context_init(void)
{
    ctx.layer = FSM_LAYER_SIM;
    ctx.transType  = TRANSPORT_MQTT;
    ctx.simState = SIM_STATE_RESET;
    ctx.mqttState = MQTT_STATE_RESET;
}
