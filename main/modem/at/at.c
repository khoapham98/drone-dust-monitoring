/**
 * @file    at.c
 * @brief   Low-level AT command source file for send AT commands & receive responses
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "board.h"
#include "at.h"

static const char* TAG = "at";

static at_cmd_ctx_t ctx = {0};

static mqtt_parser_t mqtt_parser = {
    .active = false,
    .state = IDLE
};

/* uart2 */
static const uart_port_t uart_num = UART_NUM_2;

static const uart_config_t uart_cfg = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS, 
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

int at_send_wait(char* cmd, char* recv_buf, size_t recv_buf_size, char* success_token, char* error_token, uint64_t timeout_ms)
{
    if (!cmd || !recv_buf || !recv_buf_size) return -1;

    ctx.active = true;
    ctx.error_token = error_token;
    ctx.success_token = success_token;
    ctx.response_length = 0;
    memset(ctx.response_buffer, 0, sizeof(ctx.response_buffer));
    xSemaphoreTake(ctx.done_sem, 0);

    uart_write_bytes(uart_num, cmd, strlen(cmd));
    ESP_LOGD(TAG, "Sent: %s", cmd);

    if (xSemaphoreTake(ctx.done_sem, pdMS_TO_TICKS(timeout_ms)) == pdFALSE)
        return -1;

    if (recv_buf_size < ctx.response_length) {
        ESP_LOGE(TAG, "Not enough space");
        return -1;
    }

    memcpy(recv_buf, ctx.response_buffer, ctx.response_length);

    if (recv_buf_size > ctx.response_length) {
        recv_buf[ctx.response_length] = 0;
    }

    ESP_LOGD(TAG, "Response: %s", recv_buf);
    return ctx.response_length;
}

static void modem_cmd_resp_handle(char* buf, int resp_len)
{
    if (!ctx.active || !buf || !resp_len || mqtt_parser.active) return;

    if (ctx.response_length + resp_len + 1 > sizeof(ctx.response_buffer)) {
        ESP_LOGE(TAG,
            "Resp buffer overflow: curr=%d incoming=%d max=%d",
            ctx.response_length,
            resp_len,
            sizeof(ctx.response_buffer));
        return;
    }

    memcpy(ctx.response_buffer + ctx.response_length, buf, resp_len);

    ctx.response_length += resp_len;
    ctx.response_buffer[ctx.response_length] = 0;

    if (ctx.error_token != NULL) {
        if (strstr(ctx.response_buffer, ctx.error_token)) {
            goto done;
        }
    } 
    
    if (ctx.success_token != NULL) {
        if (strstr(ctx.response_buffer, ctx.success_token)) {
            goto done;
        }
    }
    return;

done:
    ctx.active = false;
    xSemaphoreGive(ctx.done_sem);
    return;
}

static void mqtt_urc_handle(char* buf, int resp_len)
{
    static char payload[256];
    static int payload_len = 0;
    static int sub_payload_len = 0;

    switch (mqtt_parser.state)
    {
    case IDLE:
        if (strstr(buf, "CMQTTRXSTART")) {
            payload_len = 0;
            memset(payload, 0, sizeof(payload));
            mqtt_parser.state = WAIT_TOPIC;
        }
        break;

    case WAIT_TOPIC:
        if (strstr(buf, "CMQTTRXTOPIC")) {
            mqtt_parser.state = WAIT_PAYLOAD;
        }
        break;

    case WAIT_PAYLOAD:
        if (strstr(buf, "CMQTTRXPAYLOAD")) {
            sscanf(buf, "+CMQTTRXPAYLOAD: %*d,%d", &sub_payload_len);
            mqtt_parser.state = COLLECT_PAYLOAD;
        }
        break;

    case COLLECT_PAYLOAD:
        if (strstr(buf, "CMQTTRXEND")) {
            payload[sub_payload_len] = 0;
            mqtt_parser.active = false;
            mqtt_parser.state = IDLE;

            ESP_LOGW(TAG, "Receive MQTT message:\n%s", payload);
            break;
        }

        memcpy(payload + payload_len, buf, resp_len);
        payload_len += resp_len;
        break;

    default:
        mqtt_parser.active = false;
        mqtt_parser.state = IDLE;
        break;
    }
}

static void modem_urc_handle(char* buf, int resp_len)
{
    if (!buf || resp_len <= 0) return;

    if (mqtt_parser.active) {
        mqtt_urc_handle(buf, resp_len);
        return;
    }

    if (strstr(buf, "CMQTTRXSTART")) {
        mqtt_parser.active = true;
        mqtt_parser.state = IDLE;
        mqtt_urc_handle(buf, resp_len);
    }
}

static void modem_rx_dispatch(char* buf, int resp_len)
{
    if (buf == NULL || resp_len <= 0 || (resp_len == 1 && buf[0] == '\n')) return;

    modem_urc_handle(buf, resp_len);

    modem_cmd_resp_handle(buf, resp_len);
}

static void modem_uart_rx_task(void *pvParameters)
{
    char buf[256] = {0};
    int cnt = 0;
    char byte;

    while (1) {
        if (uart_read_bytes(uart_num, &byte, 1, pdMS_TO_TICKS(10)) <= 0)
            continue;

        buf[cnt++] = byte;
        if (byte == '\n' || byte == '>') {
            buf[cnt] = 0;
            modem_rx_dispatch(buf, cnt);
            cnt = 0;
        }
    }
}

int sim_uart_init(void)
{
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, UART2_TX_PIN, UART2_RX_PIN, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, 2048, 0, 0, NULL, 0));

    ctx.done_sem = xSemaphoreCreateBinary();
    if (ctx.done_sem == NULL) {
        ESP_LOGE(TAG, "Create binary semaphore failed");
        return -1;
    }
    xSemaphoreTake(ctx.done_sem, 0);

    if (xTaskCreate(modem_uart_rx_task, "modem uart rx task", 3072, NULL, 2, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Create modem uart rx task failed");
        return -1;
    }

    return 0;
}