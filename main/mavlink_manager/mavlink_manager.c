/**
 * @file    mavlink_manager.c
 * @brief   Implementation of MAVLink message parsing and dispatch 
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "board.h"
#include "mavlink_manager.h"

static const char* TAG = "mavlink_manager";

/* uart1 */
static const uart_port_t uart_num = UART_NUM_1;

static const uart_config_t uart_cfg = {
    .baud_rate = 57600,
    .data_bits = UART_DATA_8_BITS, 
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

/* mavlink */
mavlink_system_t mavlink_system = {
    .sysid = 2, 
    .compid = MAV_COMP_ID_ONBOARD_COMPUTER
};

static mavlink_message_t mav_msg;
static mavlink_status_t  mav_status;
static QueueHandle_t msgQueueTable[MAVLINK_MAX_SUBSCRIBED_MSG] = {0};

void mavlinkSubscribeMsg(QueueHandle_t queue, uint32_t msgid)
{
    if (queue == NULL || msgid >= MAVLINK_MAX_SUBSCRIBED_MSG) {
        ESP_LOGE(TAG, "Register failed: queue=%p msgid=%d", queue, (int) msgid);
        return;
    }

    if (msgQueueTable[msgid] != NULL) {
        ESP_LOGW(TAG, "MAVLink msgid %d already registered, overwrite", (int) msgid);
    }

    msgQueueTable[msgid] = queue;
}

static void mavlinkManagerTask(void *pvParameters)
{
	while (1) {
        uint8_t byte;

        if (uart_read_bytes(uart_num, &byte, 1, portMAX_DELAY) <= 0)
            continue;

        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &mav_msg, &mav_status)) {
            if (mav_msg.msgid >= MAVLINK_MAX_SUBSCRIBED_MSG) {
                ESP_LOGW(TAG, "MAVLink msgid %d out of range", (int) mav_msg.msgid);
                continue;
            }

            QueueHandle_t queue = msgQueueTable[mav_msg.msgid];
            if (queue == NULL) {
                ESP_LOGW(TAG, "No subscriber for MAVLink msgid %d", (int) mav_msg.msgid);
                continue;
            }

            if (xQueueSend(queue, (void*) &mav_msg, 
            pdMS_TO_TICKS(MAVLINK_QUEUE_SEND_TIMEOUT_MS) != pdTRUE)) {
                ESP_LOGW(TAG, "Queue full, drop MAVLink msgid %d", (int) mav_msg.msgid);
            }
        }
	}
}

int setupMavlinkManager(void)
{
    esp_err_t ret = uart_param_config(uart_num, &uart_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Set UART configuration parameters failed");
        return -1;
    }

    ret = uart_set_pin(uart_num, UART1_TX_PIN, UART1_RX_PIN, -1, -1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pin failed");
        return -1;
    }

    ret = uart_driver_install(uart_num, 2048, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Install UART driver failed");
        return -1;
    }

    TaskHandle_t mavlinkTaskHandle;
    if (xTaskCreate(mavlinkManagerTask, 
            "mavlink manager task", 
            4096, NULL, 0, 
            &mavlinkTaskHandle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create mavlink manager task!");
        return -1;
    }

    ESP_LOGI(TAG, "mavlink manager task created");
	return 0;
}
