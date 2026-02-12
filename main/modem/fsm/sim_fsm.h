/**
 * @file    sim_fsm.h
 * @brief   SIM layer state machine handlers and transition logic
 */
#ifndef _SIM_FSM_H_
#define _SIM_FSM_H_
#include "fsm_manager.h"

/**
 * @brief Handle SIM layer FSM based on current SIM state.
 * @param state Current SIM state to be processed.
 * @return none.
 */
void simFsmHandler(eSimState state);

#endif