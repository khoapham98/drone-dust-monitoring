/**
 * @file    sim.c
 * @brief   High-level SIM API source file built on top of the AT command driver
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "sys/log.h"
#include "sim.h"
#include "at_cmd.h"

typedef int(*callback_t)(void);

static bool sim_resp_ok(char* buf)
{
    if (buf == NULL) return false;

    if (strstr(buf, "OK") != NULL)
        return true;
    else
        return false;
}

static int retry(callback_t cb, size_t retry_cnt)
{
    if (cb == NULL) return -1;

    size_t cnt = 0;
    char resp[RESP_FRAME] = {0};

    at_attach_resp_buffer(resp, sizeof(resp));

    while (cnt <= retry_cnt) {
        memset(resp, 0, sizeof(resp));

        cb();

        if (sim_resp_ok(resp) == true) 
            break;
        
        cnt++;
        if (cnt <= retry_cnt) {
            sleep(1);
            LOG_WRN("retry #%d", cnt);
        }
    }

    at_attach_resp_buffer(NULL, 0);
    if (cnt > retry_cnt) return -1;
    return cnt; 
}

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

int simSetPdpContext(void)
{
#if VIETTEL
    char* apn = "v-internet";
#elif MOBIFONE
    char* apn = "m-wap";
#elif VINAPHONE
    char* apn = "m3-world";
#endif
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

int simCheckPdp(void)
{
    return at_send_wait(AT_CMD_CHECK_PDP_CONTEXT, 1000);
}

/* ===== MODULE SIM SELF-TEST ===== */

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

void simModuleInitCheck(void)
{
    int err = 0;

    LOG_INF("Module Check start");

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
    
    LOG_INF("Module check done");
}

void simSetup4G(void)
{
    int err = 0;

    LOG_INF("setup 4G start");

    err = retry(simCheckReady, 5);
    if (err < 0) {
        LOG_ERR("SIM not ready (CPIN?)");
        return;
    }

    err = simReadICCID();
    if (err < 0) {
        LOG_ERR("Read ICCID failed");
        return;
    }

    err = retry(simCheckSignal, 5);
    if (err < 0) {
        LOG_ERR("Read signal (CSQ) failed");
        return;
    }

    err = retry(simCheckRegEps, 8);
    if (err < 0) {
        LOG_ERR("Network registration (CEREG) failed");
        return;
    }

    err = simCheckOperator();
    if (err < 0) {
        LOG_ERR("Read operator (COPS) failed");
        return;
    }
    
    err = retry(simAttachGprs, 5);
    if (err < 0) {
        LOG_ERR("Attach GPRS failed");
        return;
    }

    err = simSetPdpContext();
    if (err < 0) {
        LOG_ERR("Set PDP context failed");
        return;
    }

    err = simCheckPdp();
    if (err < 0) {
        LOG_ERR("Check PDP context failed");
        return;
    }

    err = retry(simActivatePdp, 5);
    if (err < 0) {
        LOG_ERR("Activate PDP failed");
        return;
    }

    err = retry(simGetIpAddr, 5);
    if (err < 0) {
        LOG_ERR("Get IP address failed");
        return;
    }

    LOG_INF("Setup 4G done");
}
