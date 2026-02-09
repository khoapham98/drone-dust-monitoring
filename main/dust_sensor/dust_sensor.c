/**
 * @file    dust_sensor.c
 * @brief   PMS7003 driver source file
 */
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "board.h"
#include "dust_sensor.h"

/* ring buffer */
static RingbufHandle_t rbHandle;

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
static const rmt_config_t rmt_cfg = {
    .rmt_mode = RMT_MODE_RX,
    .channel  = RMT_CHANNEL_0,
    .gpio_num = UART_SW_RX_PIN, 
    .clk_div  = RMT_DEFAULT_CLK_DIV,
    .mem_block_num = 2, 

    .rx_config = {
        .filter_en = true,
        .idle_threshold = 3000,
        .filter_ticks_thresh = 10
    }
};

/* dust data */
pm25_aqi_ctx_t ctx = {0};

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

static void pm25ToAqi(void)
{
    eAqiLevel level = getPm25AqiLevel(ctx.pm25);

    ctx.iLow  = aqiRanges[level][0];
    ctx.iHigh = aqiRanges[level][1];
    ctx.cLow  = pm25Ranges[level][0];
    ctx.cHigh = pm25Ranges[level][1];

    if (ctx.cHigh == ctx.cLow) {
        ESP_LOGE(TAG, "Invalid PM2.5 breakpoint - keep previous data");
        return;
    }

    float rangeAqi = (float) (ctx.iHigh - ctx.iLow);
    float rangeConcentration = (float) (ctx.cHigh - ctx.cLow);
    float concentrationDiff  = (float) (ctx.pm25 - ctx.cLow);

    ctx.aqi = (rangeAqi / rangeConcentration) * concentrationDiff + (float) ctx.iLow;
}

static bool checkHeaderBytes(uint8_t* buf)
{
    if (buf[0] == START_CHARACTER_1 && buf[1] == START_CHARACTER_2)
        return true;
    
    ESP_LOGD(TAG, "Invalid header bytes: %02X %02X", buf[0], buf[1]);
    return false;
}

void rmtParser(rmt_item32_t* rmt_buf, uint8_t* recv_buf, size_t totalItems) 
{
    for (int i = 0; i < totalItems; i++) {
        ESP_LOGI(TAG, "rmt_buf[%d]:\n \
                        duration0 = %d - level0 = %d\n \
                        duration1 = %d - level1 = %d\n \
                        ==============================",
                        i, 
                        (int) rmt_buf[i].duration0, (int) rmt_buf[i].level0,
                        (int) rmt_buf[i].duration1, (int) rmt_buf[i].level1);
    }
}

bool getDustData(void)
{
    size_t totalBytes = 0;
    rmt_item32_t* pItemBuffer = (rmt_item32_t*) xRingbufferReceive(rbHandle, &totalBytes, portMAX_DELAY);

    if (totalBytes <= 0 || pItemBuffer == NULL) {
        ESP_LOGE(TAG, "No item found");
        return false;
    }

    uint8_t dustBuffer[DUST_DATA_FRAME] = {0};
    size_t totalItems = totalBytes / sizeof(rmt_item32_t);

    // rmtParser(pItemBuffer, dustBuffer, totalItems);

    ESP_LOGI(TAG, "data received - total items: %d", totalItems);

    vRingbufferReturnItem(rbHandle, pItemBuffer);

    return true;
}

int dust_sensor_sw_uart_init(void)
{
    esp_err_t ret = gpio_config(&gpio_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed");
        return -1;
    }

    ret = rmt_config(&rmt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT parameters configuration failed");
        return -1;
    }

    ret = rmt_driver_install(rmt_cfg.channel, 1024, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT driver install failed");
        return -1;
    }

    ret = rmt_get_ringbuf_handle(rmt_cfg.channel, &rbHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT ringbuffer handle get failed");
        return -1;
    }

    ret = rmt_rx_start(rmt_cfg.channel, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RMT RX start failed");
        return -1;
    }

	return 0;
}