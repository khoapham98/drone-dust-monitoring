/**
 * @file    sim.h
 * @brief   High-level SIM API header file built on top of the AT command driver
 */
#ifndef _SIM_H_
#define _SIM_H_

#define     VIETTEL         1
#define     MOBIFONE        0
#define     VINAPHONE       0

/******************************************************************************/
/* Mode switch */
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


/******************************************************************************/
/* Basic AT operations */
/******************************************************************************/

/**
 * @brief Check if the module is alive (AT → OK).
 * @return 0 on success, -1 on failure.
 */
int simCheckAlive(void);

/**
 * @brief Enable AT command echo (ATE1).
 * @return 0 on success, -1 on failure.
 */
int simEchoOn(void);

/**
 * @brief Disable AT command echo (ATE0).
 * @return 0 on success, -1 on failure.
 */
int simEchoOff(void);


/******************************************************************************/
/* SIM & Signal */
/******************************************************************************/

/**
 * @brief Check SIM card readiness (AT+CPIN?).
 * @return 0 on success, -1 on failure.
 */
int simCheckReady(void);

/**
 * @brief Read SIM ICCID (AT+CICCID).
 * @return 0 on success, -1 on failure.
 */
int simReadICCID(void);

/**
 * @brief Read signal strength (AT+CSQ).
 * @return 0 on success, -1 on failure.
 */
int simCheckSignal(void);


/******************************************************************************/
/* Network Registration */
/******************************************************************************/

/**
 * @brief Check circuit-switched network registration (AT+CREG?).
 * @return 0 on success, -1 on failure.
 */
int simCheckRegNet(void);

/**
 * @brief Check GPRS/packet-switched registration (AT+CGREG?).
 * @return 0 on success, -1 on failure.
 */
int simCheckRegGprs(void);

/**
 * @brief Check EPS/LTE registration (AT+CEREG?).
 * @return 0 on success, -1 on failure.
 */
int simCheckRegEps(void);

/**
 * @brief Read current operator (AT+COPS?).
 * @return 0 on success, -1 on failure.
 */
int simCheckOperator(void);


/******************************************************************************/
/* PDP Context */
/******************************************************************************/

/**
 * @brief Set PDP context APN (AT+CGDCONT).
 * @return 0 on success, -1 on failure.
 */
int simSetPdpContext(void);

/**
 * @brief Attach to GPRS service (AT+CGATT=1).
 * @return 0 on success, -1 on failure.
 */
int simAttachGprs(void);

/**
 * @brief Detach from GPRS service (AT+CGATT=0).
 * @return 0 on success, -1 on failure.
 */
int simDetachGprs(void);

/**
 * @brief Activate PDP context (AT+CGACT=1,1).
 * @return 0 on success, -1 on failure.
 */
int simActivatePdp(void);

/**
 * @brief Deactivate PDP context (AT+CGACT=0,1).
 * @return 0 on success, -1 on failure.
 */
int simDeactivatePdp(void);

/**
 * @brief Read assigned IP address (AT+CGPADDR).
 * @return 0 on success, -1 on failure.
 */
int simGetIpAddr(void);

/**
 * @brief Check PDP context (AT+CGDCONT).
 * @return 0 on success, -1 on failure.
 */
int simCheckPdp(void); 

/******************************************************************************/
/* Self Test */
/******************************************************************************/

/**
 * @brief  Query firmware version using AT+GMR.
 * @return 0 on success, -1 on failure.
 */
int simCheckFwVersion(void);

/**
 * @brief  Query the hardware model using AT+CGMM.
 * @return 0 on success, -1 on failure.
 */
int simCheckModel(void);

/**
 * @brief  Query the manufacturer information using AT+CGMI.
 * @return 0 on success, -1 on failure.
 */
int simCheckManufacturer(void);

/**
 * @brief  Query the IMEI using AT+CGSN.
 * @return 0 on success, -1 on failure.
 */
int simCheckImei(void);

/**
 * @brief  Query the current functionality mode using AT+CFUN?.
 * @return 0 on success, -1 on failure.
 */
int simCheckFunMode(void);

/******************************************************************************/
/* Precheck */
/******************************************************************************/

/**
 * @brief  Module self-test
 * @return none
 */
void simModuleInitCheck(void);

/**
 * @brief  Setup 4G
 * @return none
 */
void simSetup4G(void);

#endif