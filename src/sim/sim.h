/**
 * @file    sim.h
 * @brief   SIM state handlers for FSM control and state transition logic
 */
#ifndef _SIM_H_
#define _SIM_H_
#include "sim_cmd.h"
#include "fsm/fsm.h"

/**
 * @brief Handle SIM layer FSM based on current SIM state.
 * @param state Current SIM state to be processed.
 * @return none.
 */
void simFsmHandler(eSimState state);

#endif