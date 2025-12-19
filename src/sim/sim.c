/**
 * @file    sim.c
 * @brief   SIM state handlers for FSM control and state transition logic
 */
#include <unistd.h>
#include "sys/log.h"
#include "sim_cmd.h"
#include "sim.h"
#include "fsm/fsm.h"

static eSimState preState = SIM_STATE_RESET;

static const char* simStateStr[] = {
    "SIM_STATE_RESET",
    "SIM_STATE_AT_SYNC",
    "SIM_STATE_SIM_READY",
    "SIM_STATE_NET_READY",
    "SIM_STATE_PDP_ACTIVE"
};

static void updateSimState(eSimResult res, eSimState backState, eSimState nextState)
{
    if (res == FAIL) {
        setSimState(preState);
        preState = backState;
    } 
    else if (res == PASS) {
        if (getSimState() == SIM_STATE_PDP_ACTIVE) {
            setFsmLayer(FSM_LAYER_TRANSPORT);
            return;
        }

        preState = getSimState();
        setSimState(nextState);
    } 
    else {
        sleep(1);
    }
}

static void simResetStatusHandler(void)
{
    while (1) {
        if (simCheckAlive() == PASS)
            break;
        sleep(1);
    }

    updateSimState(PASS, SIM_STATE_RESET, SIM_STATE_AT_SYNC);
} 

static void atSyncStatusHandler(void)
{
    eSimResult res = PASS;
    if (simCheckAlive() == FAIL || simEchoOff() == FAIL) {
        res = FAIL;
    }

    updateSimState(res, SIM_STATE_RESET, SIM_STATE_SIM_READY);
}

static void simReadyStatusHandler(void)
{
    eSimResult res = simCheckReady();
    updateSimState(res, SIM_STATE_RESET, SIM_STATE_NET_READY);
}

static void netReadyStatusHandler(void)
{
    eSimResult res = simCheckRegEps();
    updateSimState(res, SIM_STATE_AT_SYNC, SIM_STATE_PDP_ACTIVE);
}

static void pdpActiveStatusHandler(void)
{
    eSimResult res = simSetPdpContext();
    if (res != PASS)    
        goto end;

    res = simAttachGprs();
    if (res != PASS)
        goto end;

    res = simActivatePdp();

end:
    updateSimState(res, SIM_STATE_SIM_READY, SIM_STATE_PDP_ACTIVE);
}

void simFsmHandler(eSimState state)
{
    LOG_INF("%s", simStateStr[state]);
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