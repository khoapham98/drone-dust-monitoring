/**
 * @file    at.c
 * @brief   Low-level AT command source file for send AT commands & receive responses
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"
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

/* message buffer */
static MessageBufferHandle_t cmd_resp_handle = NULL;
static MessageBufferHandle_t urc_handle = NULL;

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
    ESP_LOGD(TAG, "[TX] %s", cmd);

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

    return ctx.response_length;
}

static void modem_cmd_resp_handle(char* buf, int resp_len)
{
    if (!ctx.active || !buf || !resp_len) return;

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
        if (!strncmp(buf, "+CMQTTRXSTART", sizeof("+CMQTTRXSTART") - 1)) {
            payload_len = 0;
            memset(payload, 0, sizeof(payload));
            mqtt_parser.state = WAIT_TOPIC;
            mqtt_parser.active = true;
        }
        break;

    case WAIT_TOPIC:
        if (!strncmp(buf, "+CMQTTRXTOPIC", sizeof("+CMQTTRXTOPIC") - 1)) {
            mqtt_parser.state = WAIT_PAYLOAD;
        }
        break;

    case WAIT_PAYLOAD:
        if (!strncmp(buf, "+CMQTTRXPAYLOAD", sizeof("+CMQTTRXPAYLOAD") - 1)) {
            char* str = strchr(buf, ',');
            if (str == NULL) break;
            str += 1;
            sub_payload_len = atoi(str);
            mqtt_parser.state = COLLECT_PAYLOAD;
        }
        break;

    case COLLECT_PAYLOAD:
        if (!strncmp(buf, "+CMQTTRXEND", sizeof("+CMQTTRXEND") - 1)) {
            payload[sub_payload_len] = 0;
            mqtt_parser.active = false;
            mqtt_parser.state = IDLE;

            ESP_LOGI(TAG, "Receive MQTT message:\n%s", payload);
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

static bool is_mqtt_urc(const char* buf)
{
    if (mqtt_parser.active) 
        return true;

    if (!strncmp(buf, MQTT_URC_PREFIX, MQTT_URC_PREFIX_LEN))
        return true;

    return false;
}

static bool is_urc(const char* buf)
{
    if (is_mqtt_urc(buf))
        return true;

    return false;
}

static void urc_handler_task(void *pvParameters)
{
    char buf[256] = {0};

    while (1) {
        size_t len = xMessageBufferReceive(urc_handle, buf, sizeof(buf), portMAX_DELAY);  

        buf[len] = 0;

        ESP_LOGD(TAG, "[URC] %s", buf);

        mqtt_urc_handle(buf, len);
    }
}

static void cmd_response_handler_task(void *pvParameters)
{
    char buf[256] = {0};

    while (1) {
        size_t len = xMessageBufferReceive(cmd_resp_handle, buf, sizeof(buf), portMAX_DELAY);  

        buf[len] = 0;

        ESP_LOGD(TAG, "[RX] %s", buf);

        modem_cmd_resp_handle(buf, len);
    }
}

static void modem_uart_rx_task(void *pvParameters)
{
    char buf[256] = {0};
    int cnt = 0;
    char byte;

    while (1) {
        if (uart_read_bytes(uart_num, &byte, 1, pdMS_TO_TICKS(10)) <= 0)
            continue;

        if (cnt > sizeof(buf) - 1) {
            ESP_LOGW(TAG, "UART RX line overflow - Dropping partial line");
            cnt = 0;
            continue;
        }

        buf[cnt++] = byte;

        if (byte == '\n' || byte == '>') {
            if ((buf[0] == '\r' && buf[1] == '\n')) {
                cnt = 0;
                continue;
            }

            if (is_urc(buf)) {
                xMessageBufferSend(urc_handle, buf, cnt, pdMS_TO_TICKS(10));
            } else {
                xMessageBufferSend(cmd_resp_handle, buf, cnt, pdMS_TO_TICKS(10));
            }

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

    if (xTaskCreate(modem_uart_rx_task, "modem uart rx task", 4096, NULL, 2, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Create modem uart rx task failed");
        return -1;
    }

    cmd_resp_handle = xMessageBufferCreate(2048);
    if (cmd_resp_handle == NULL) {
        ESP_LOGE(TAG, "Create message buffer for at command response failed");
        return -1;
    }

    if (xTaskCreate(cmd_response_handler_task, "cmd response handle task", 4096, NULL, 1, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Create cmd response handle task failed");
        return -1;
    }

    urc_handle = xMessageBufferCreate(2048);
    if (urc_handle == NULL) {
        ESP_LOGE(TAG, "Create message buffer for urc failed");
        return -1;
    }

    if (xTaskCreate(urc_handler_task, "urc handle task", 4096, NULL, 1, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Create urc handle task failed");
        return -1;
    }

    return 0;
}