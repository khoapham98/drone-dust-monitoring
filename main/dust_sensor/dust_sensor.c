/**
 * @file    dust_sensor.c
 * @brief   PMS7003 driver source file
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "board.h"
#include "dust_sensor.h"

extern TaskHandle_t dustTaskHandle;

/* logger */
static const char* TAG = "dust_sensor";

/* gpio */
static const gpio_config_t gpio_cfg = {
    .pin_bit_mask = (1ULL << UART_SW_RX_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

/* rmt */
static bool rmt_rx_done_callback(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx);

static rmt_channel_handle_t rmt_rx_channel;

static rmt_symbol_word_t rmt_buf[128] = {0};

static volatile size_t totalSymbols = 0;

static const rmt_rx_channel_config_t rmt_rx_cfg = {
    .gpio_num = UART_SW_RX_PIN,         
    .clk_src  = RMT_CLK_SRC_DEFAULT,    // 80MHz
    .intr_priority = 0,
    .resolution_hz = 1000000,           // 1MHz ~ 1us
    .mem_block_symbols = 128            // 128 RMT items ~ 512 bytes
};

static const rmt_receive_config_t rmt_recv_cfg = {
    .signal_range_min_ns = 3000,        // 3us
    .signal_range_max_ns = 30000000     // 30ms
};

static const rmt_rx_event_callbacks_t rmt_rx_cb = {
    .on_recv_done = rmt_rx_done_callback
};

/* dust data */
dust_ctx_t dust = {0};

static const int aqiRanges[AQI_LEVEL_COUNT][2] = {
    {0, 50}, 
    {51, 100},
    {101, 150},
    {151, 200},
    {201, 300},
    {301, 500}
};

static const float pm25Ranges[AQI_LEVEL_COUNT][2] = {
    {0.0f, 12.0f},
    {12.1f, 35.4f},
    {35.5f, 55.4f},
    {55.5f, 150.4f},
    {150.5f, 250.4f},
    {250.5f, 500.0f}
};

static eAqiLevel getPm25AqiLevel(uint16_t pm25)
{
    for (int i = 0; i < AQI_LEVEL_COUNT - 1; i++) {
        if ((float) pm25 <= pm25Ranges[i][1])
            return (eAqiLevel) i;
    }

    return AQI_HAZARDOUS;
}

static void pm2_5ToAqi(void)
{
    eAqiLevel level = getPm25AqiLevel(dust.pm2_5);

    aqi_calc_t aqi = {
        .iLow  = aqiRanges[level][0],
        .iHigh = aqiRanges[level][1],
        .cLow  = pm25Ranges[level][0],
        .cHigh = pm25Ranges[level][1]
    };

    if (aqi.cHigh == aqi.cLow) {
        ESP_LOGE(TAG, "Invalid PM2.5 breakpoint - keep previous data");
        return;
    }

    float rangeAqi = (float) (aqi.iHigh - aqi.iLow);
    float rangeConcentration = (float) (aqi.cHigh - aqi.cLow);
    float concentrationDiff  = (float) (dust.pm2_5 - aqi.cLow);

    dust.aqi = (rangeAqi / rangeConcentration) * concentrationDiff + (float) aqi.iLow;
}

static bool checkHeaderBytes(uint8_t* buf)
{
    if (buf[0] == START_CHARACTER_1 && buf[1] == START_CHARACTER_2)
        return true;
    
    ESP_LOGD(TAG, "Invalid header bytes: %02X %02X", buf[0], buf[1]);
    return false;
}

void parseRmtToUart(uint8_t* recv_buf)
{
    uint8_t tmp[10] = {0};
    uint8_t bitCount = 0;
    uint8_t bitIndex = 0;
    uint8_t recvIndex = 0;

    for (int i = 0; i < totalSymbols; i++) {
        uint8_t bitNum = rmt_buf[i].duration0 / 104;
        uint8_t bitLevel = rmt_buf[i].level0;
        bitCount += bitNum;

        for (int j = bitIndex; j < bitCount; j++) {
            tmp[j] = bitLevel;
        }
        bitIndex = bitCount;

        bitNum = rmt_buf[i].duration1 / 104;
        bitLevel = rmt_buf[i].level1;
        bitCount += bitNum;

        for (int j = bitIndex; j < bitCount; j++) {
            tmp[j] = bitLevel;
        }
        bitIndex = bitCount;

        if (bitCount >= 10) {
            if (tmp[0] == 0 && tmp[9] == 1) {
                for (int j = 1; j < 9; j++) {
                    recv_buf[recvIndex] |= tmp[j] << (j - 1);
                }

                recvIndex++;

                if (recvIndex >= 32) 
                    return;
            }

            bitCount = 0;
            bitIndex = 0;
        }
    }
}

bool getDustData(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    ESP_LOGI(TAG, "data received - total symbols: %zu", totalSymbols);

    // rmtPrint(rmt_buf, totalSymbols);

    esp_err_t ret = rmt_receive(rmt_rx_channel, rmt_buf, sizeof(rmt_buf), &rmt_recv_cfg);
    if (ret != ESP_OK) 
        ESP_LOGE(TAG, "Start RMT RX failed (0x%3X)", ret);

    return true;
}

static bool rmt_rx_done_callback(rmt_channel_handle_t rx_chan, const rmt_rx_done_event_data_t *edata, void *user_ctx)
{
    BaseType_t ret = pdFALSE;

    totalSymbols = edata->num_symbols;

    vTaskNotifyGiveFromISR(dustTaskHandle, &ret);

    return ret;
}

int dust_sensor_sw_uart_init(void)
{
    esp_err_t ret = gpio_config(&gpio_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed");
        return -1;
    }

    ret = rmt_new_rx_channel(&rmt_rx_cfg, &rmt_rx_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Create RMT RX channel failed");
        return -1;
    }

    ret = rmt_rx_register_event_callbacks(rmt_rx_channel, &rmt_rx_cb, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT RX channel set callback failed");
        return -1;
    }

    ret = rmt_enable(rmt_rx_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Enable RMT RX channel failed");
        return -1;
    }

    ret = rmt_receive(rmt_rx_channel, rmt_buf, sizeof(rmt_buf), &rmt_recv_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Init RMT RX channel failed (0x%3X)", ret);
        return -1;
    }

	return 0;
}