/**
 * @file    sim_fsm.h
 * @brief   SIM layer state machine handlers and transition logic
 */
#ifndef _SIM_FSM_H_
#define _SIM_FSM_H_

enum simState {
    SIM_STATE_RESET,
    SIM_STATE_AT_SYNC,
    SIM_STATE_SIM_READY,
    SIM_STATE_NET_READY,
    SIM_STATE_PDP_ACTIVE,        
};

typedef enum simState eSimState;

/**
 * @brief Handle SIM layer FSM based on current SIM state.
 * @param state Current SIM state to be processed.
 * @return none.
 */
void simFsmHandler(eSimState state);

#endif