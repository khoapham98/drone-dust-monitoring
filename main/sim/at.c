/**
 * @file    at.c
 * @brief   Low-level AT command source file for send AT commands & receive responses
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_attr.h" 
#include "driver/uart.h"
#include "driver/gptimer.h"
#include "board.h"
#include "at.h"

static const char* TAG = "at";

volatile bool isTimeout = false;

/* uart2 */
static const uart_port_t uart_num = UART_NUM_2;
static QueueHandle_t uart_queue;

static const uart_config_t uart_cfg = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS, 
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

/* timer */
static gptimer_handle_t gptimerHandle;

static gptimer_alarm_config_t alarm_cfg = {
    .reload_count = 0,
    .flags.auto_reload_on_alarm = true
};

static const gptimer_config_t gptimer_cfg = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000,
    .intr_priority = 1
};

static bool IRAM_ATTR gptimer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    isTimeout = true;
    
    return isTimeout;
}

static const gptimer_event_callbacks_t gptimer_cb = {
    .on_alarm = gptimer_alarm_cb
};

static void setAlarmTimeout(uint64_t tout_ms)
{
    alarm_cfg.alarm_count = tout_ms * 1000;

    gptimer_set_alarm_action(gptimerHandle, &alarm_cfg);
}

int at_send_wait(char* cmd, char* recv_buf, size_t len, uint64_t timeout_ms)
{
    isTimeout = false;
    setAlarmTimeout(timeout_ms);

    uart_flush(uart_num);
    uart_write_bytes(uart_num, cmd, strlen(cmd));

    gptimer_start(gptimerHandle);

    uart_event_t event;
    int receivedBytes = 0;

    while (!isTimeout) {
        xQueueReceive(uart_queue, &event, portMAX_DELAY);

        if (event.type == UART_DATA) {
            receivedBytes = uart_read_bytes(uart_num, recv_buf, (uint32_t) event.size, pdMS_TO_TICKS(timeout_ms));
            break;
        }
    }

    gptimer_stop(gptimerHandle);

    if (isTimeout) {
        ESP_LOGW(TAG, "Timeout for: %s", cmd);
        return -1;
    }

    if (receivedBytes > 0) {
        recv_buf[receivedBytes] = '\0';
        ESP_LOGD(TAG, "Send: %s\nResponse:%s", cmd, recv_buf);
    } else {
        ESP_LOGE(TAG, "Send: %s\nNo response received", cmd);
    }

    return receivedBytes;
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

    ret = uart_driver_install(uart_num, 2048, 0, 128, &uart_queue, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Install UART driver failed");
        return -1;
    }

    ret = gptimer_new_timer(&gptimer_cfg, &gptimerHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create timer failed");
        return -1;
    }

    ret = gptimer_register_event_callbacks(gptimerHandle, &gptimer_cb, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Set callback for timer failed");
        return -1;
    }

    ret = gptimer_enable(gptimerHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Enable timer failed");
        return -1;
    }
    return 0;
}