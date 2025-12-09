/**
 * @file    sim.c
 * @brief   High-level SIM API source file built on top of the AT command driver
 */
#include <stdio.h>
#include "sys/log.h"
#include "sim.h"
#include "at_cmd.h"
#include "src/drivers/uart.h"

/* ===== SWITCH MODE ===== */

int simEnterCmdMode(void)
{
    return at_send_wait(CMD_ENTER_CMD_MODE, 1500);
}

int simEnterDataMode(void)
{
    return at_send_wait(CMD_ENTER_DATA_MODE, 1500);
}

/* ===== BASIC CHECK ===== */

int simCheckAlive(void)
{
    return at_send_wait(AT_CMD_BASIC_CHECK, 500);
}

int simEchoOn(void)
{
    return at_send_wait(AT_CMD_ECHO_ON, 500);
}

int simEchoOff(void)
{
    return at_send_wait(AT_CMD_ECHO_OFF, 500);
}

/* ===== SIM & SIGNAL ===== */

int simReadICCID(void)
{
    return at_send_wait(AT_CMD_READ_ICCID, 1000);
}

int simCheckReady(void)
{
    return at_send_wait(AT_CMD_CHECK_READY, 1000);
}

int simCheckSignal(void)
{
    return at_send_wait(AT_CMD_CHECK_SIGNAL, 1000);
}

int simCheckRegNet(void)
{
    return at_send_wait(AT_CMD_CHECK_REG_NET, 1500);
}

int simCheckRegGprs(void)
{
    return at_send_wait(AT_CMD_CHECK_REG_GPRS, 1500);
}

int simCheckRegEps(void)
{
    return at_send_wait(AT_CMD_CHECK_REG_EPS, 1500);
}

int simCheckOperator(void)
{
    return at_send_wait(AT_CMD_CHECK_OPERATOR, 1500);
}

/* ===== PDP CONTEXT / PACKET DATA ===== */

int simSetPdpContext(const char* apn)
{
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), AT_CMD_SET_PDP_CONTEXT, apn);
    return at_send_wait(cmd, 2000);
}

int simAttachGprs(void)
{
    return at_send_wait(AT_CMD_ATTACH_GPRS, 5000);
}

int simDetachGprs(void)
{
    return at_send_wait(AT_CMD_DETACH_GPRS, 3000);
}

int simActivatePdp(void)
{
    return at_send_wait(AT_CMD_ACTIVATE_PDP, 8000);
}

int simDeactivatePdp(void)
{
    return at_send_wait(AT_CMD_DEACTIVATE_PDP, 5000);
}

int simGetIpAddr(void)
{
    return at_send_wait(AT_CMD_GET_IP_ADDR, 2000);
}

/* ===== SIM SELF TEST ===== */

int simCheckFwVersion(void)
{
    return at_send_wait(AT_CMD_GET_FW_VERSION, 2000);
}

int simCheckModel(void)
{
    return at_send_wait(AT_CMD_GET_MODEL, 2000);
}

int simCheckManufacturer(void)
{
    return at_send_wait(AT_CMD_GET_MANUFACTURER, 2000);
}

int simCheckImei(void)
{
    return at_send_wait(AT_CMD_GET_IMEI, 2000);
}

int simCheckFunMode(void)
{
    return at_send_wait(AT_CMD_GET_FUN_MODE, 2000);
}

/* ===== WRAPPER APIs ===== */

void simSelfTestBasic(void)
{
    int err = 0;
    LOG_INF("Sim self-test start");

    err = simCheckFwVersion();
    if (err < 0) {
        LOG_ERR("Check Firmware version failed");
        return;
    }

    err = simCheckModel();
    if (err < 0) {
        LOG_ERR("Check model failed");
        return;
    }

    err = simCheckManufacturer();
    if (err < 0) {
        LOG_ERR("Check manufacturer failed");
        return;
    }

    err = simCheckImei();
    if (err < 0) {
        LOG_ERR("Check IMEI failed");
        return;
    }

    err = simCheckFunMode();
    if (err < 0) {
        LOG_ERR("Check CFun mode failed");
        return;
    }

    LOG_INF("Sim self-test done");
}

void simInitialCheck(void)
{
    int err = 0;
    LOG_INF("Start sim check");

    err = simEnterCmdMode();
    if (err < 0) {
        LOG_ERR("Enter CMD mode failed");
        return;
    }

    err = simCheckAlive();
    if (err < 0) {
        LOG_ERR("AT check failed");
        return;
    }

    err = simEchoOff();
    if (err < 0) {
        LOG_ERR("Disable echo failed");
        return;
    }

    err = simCheckReady();
    if (err < 0) {
        LOG_ERR("SIM not ready (CPIN?)");
        return;
    }

    err = simReadICCID();
    if (err < 0) {
        LOG_ERR("Read ICCID failed");
        return;
    }

    err = simCheckSignal();
    if (err < 0) {
        LOG_ERR("Read signal (CSQ) failed");
        return;
    }

    err = simCheckRegEps();
    if (err < 0) {
        LOG_ERR("Network registration (CEREG) failed");
        return;
    }

    err = simCheckOperator();
    if (err < 0) {
        LOG_ERR("Read operator (COPS) failed");
        return;
    }

    LOG_INF("Sim initial check done");
}
