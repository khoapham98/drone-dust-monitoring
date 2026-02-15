/**
 * @file    at.c
 * @brief   Low-level AT command source file for send AT commands & receive responses
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/uart.h"
#include "board.h"
#include "at.h"

static const char* TAG = "at";

/* uart2 */
static const uart_port_t uart_num = UART_NUM_2;

static const uart_config_t uart_cfg = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS, 
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

static bool isFinalResponse(char* recv_buf)
{
    bool isOkDetected = (strstr(recv_buf, "\r\nOK\r\n") != NULL);
    bool isErrorDetected = (strstr(recv_buf, "\r\nERROR\r\n") != NULL);
    bool isPromptDetected =  (strstr(recv_buf, "\r\n> ") != NULL);

    if (isOkDetected || isErrorDetected || isPromptDetected)
        return true;

    return false;
}

int at_send_wait(char* cmd, char* recv_buf, size_t len, uint64_t timeout_ms)
{
    if (!cmd || !recv_buf || len == 0)
        return -1;

    uart_flush(uart_num);

    ESP_LOGD(TAG, "Sent: %s", cmd);

    uart_write_bytes(uart_num, cmd, strlen(cmd));

    uint64_t last_rx_time = esp_timer_get_time() / 1000;
    size_t total = 0;

    while (1) {
        uint8_t tmp[128];

        int n = uart_read_bytes(uart_num, tmp, sizeof(tmp), pdMS_TO_TICKS(20));

        if (n > 0) {
            size_t copy = n;
            if (total + copy >= len)
                copy = len - total - 1;

            memcpy(recv_buf + total, tmp, copy);
            total += copy;
            recv_buf[total] = 0;

            if (isFinalResponse(recv_buf)) {
                ESP_LOGD(TAG, "Response:\n%s", recv_buf);
                return total;
            }

            last_rx_time = esp_timer_get_time() / 1000;
        }

        uint64_t now = esp_timer_get_time() / 1000;

        if ((now - last_rx_time) >= timeout_ms) {
            if (total == 0) {
                ESP_LOGW(TAG, "Timeout for: %s", cmd);
                return -1;
            }

            ESP_LOGD(TAG, "Response:\n%s", recv_buf);

            return total;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int at_send(char* cmd, size_t len)
{
    return uart_write_bytes(uart_num, cmd, len);
}

int sim_uart_init(void)
{
    esp_err_t ret = uart_param_config(uart_num, &uart_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Set UART configuration parameters failed");
        return -1;
    }

    ret = uart_set_pin(uart_num, UART2_TX_PIN, UART2_RX_PIN, -1, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed");
        return -1;
    }

    ret = uart_set_rx_timeout(uart_num, 101);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set threshold timeout failed");
        return -1;
    }

    ret = uart_driver_install(uart_num, 2048, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Install UART driver failed");
        return -1;
    }

    return 0;
}