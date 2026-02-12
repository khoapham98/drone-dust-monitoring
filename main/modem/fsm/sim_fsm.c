/**
 * @file    sim_fsm.c
 * @brief   Implementation of SIM state handlers and transition logic
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "modem_common.h"
#include "sim_service.h"
#include "sim_fsm.h"
#include "fsm_manager.h"

static const char* TAG = "sim_fsm";

static const char* simStateStr[] = {
    "SIM_STATE_RESET",
    "SIM_STATE_AT_SYNC",
    "SIM_STATE_SIM_READY",
    "SIM_STATE_NET_READY",
    "SIM_STATE_PDP_ACTIVE"
};

static void updateSimState(eModemResult res, eSimState nextState)
{
    if (res == FAIL) {
        setSimState(SIM_STATE_RESET);
    } 
    else if (res == PASS) {
        if (getSimState() == SIM_STATE_PDP_ACTIVE) {
            setFsmLayer(FSM_LAYER_TRANSPORT);
            return;
        }

        setSimState(nextState);
    } 
    else {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void simResetStatusHandler(void)
{
    while (1) {
        if (simCheckAlive() == PASS)
            break;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    updateSimState(PASS, SIM_STATE_AT_SYNC);
} 

static void atSyncStatusHandler(void)
{
    eModemResult res = PASS;
    if (simCheckAlive() == FAIL || simEchoOff() == FAIL) {
        res = FAIL;
    }

    updateSimState(res, SIM_STATE_SIM_READY);
}

static void simReadyStatusHandler(void)
{
    eModemResult res = simCheckReady();
    updateSimState(res, SIM_STATE_NET_READY);
}

static void netReadyStatusHandler(void)
{
    eModemResult res = simCheckRegEps();
    updateSimState(res, SIM_STATE_PDP_ACTIVE);
}

static void pdpActiveStatusHandler(void)
{
    eModemResult res = simSetPdpContext();
    if (res != PASS)    
        goto end;

    res = simAttachGprs();
    if (res != PASS)
        goto end;

    res = simActivatePdp();

end:
    updateSimState(res, SIM_STATE_PDP_ACTIVE);
}

void simFsmHandler(eSimState state)
{
    ESP_LOGI(TAG, "Current state: %s", simStateStr[state]);
    switch (state)
    {
    case SIM_STATE_RESET:
        simResetStatusHandler();
        break;
    case SIM_STATE_AT_SYNC:
        atSyncStatusHandler();
        break;
    case SIM_STATE_SIM_READY:
        simReadyStatusHandler();
        break;
    case SIM_STATE_NET_READY:
        netReadyStatusHandler();
        break;
    case SIM_STATE_PDP_ACTIVE:
        pdpActiveStatusHandler();
        break;
    }
}