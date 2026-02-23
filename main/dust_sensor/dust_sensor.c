/**
 * @file    dust_sensor.c
 * @brief   Dust sensor module implementation
 */
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

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

static rmt_symbol_word_t rmt_buf[RMT_BUFFER_SIZE] = {0};

static volatile size_t totalSymbols = 0;

static const rmt_rx_channel_config_t rmt_rx_cfg = {
    .gpio_num = UART_SW_RX_PIN,         
    .clk_src  = RMT_CLK_SRC_DEFAULT,        // 80MHz
    .intr_priority = 0,
    .resolution_hz = 1000000,               // 1MHz ~ 1us
    .mem_block_symbols = RMT_BUFFER_SIZE    // 128 items ~ 512 bytes
};

static const rmt_receive_config_t rmt_recv_cfg = {
    .signal_range_min_ns = RMT_SIGNAL_MIN_3US,        
    .signal_range_max_ns = RMT_SIGNAL_MAX_30MS     
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
    
    ESP_LOGD(TAG, "Invalid header bytes: 0x%02X 0x%02X", buf[0], buf[1]);
    return false;
}

/**
 * @warning This function can cause stack overflow if RMT total symbols is large
 */
static void rmtDebugPrint(void) 
{
    ESP_LOGD(TAG, "RMT total symbols: %zu", totalSymbols);

    for (int i = 0; i < totalSymbols; i++) {
        ESP_LOGD(TAG, "rmt_buf[%d]:\n \
                        duration0 = %d - level0 = %d\n \
                        duration1 = %d - level1 = %d\n \
                        ==============================",
                        i, 
                        (int) rmt_buf[i].duration0, (int) rmt_buf[i].level0,
                        (int) rmt_buf[i].duration1, (int) rmt_buf[i].level1);
    }
}

static void parseRmtToUart(uint8_t* uartRxBuf)
{
    uint8_t bitsInLevel[2] = {0};
    uint8_t level[2] = {0};
    uint8_t bitCount = 0;
    uint8_t bitShift = 0;
    uint8_t currentByte = 0;
    uint8_t recvIndex = 0;
    bool isStartBitDetected = false;

    for (int i = 0; i < totalSymbols; i++) {
        level[0] = rmt_buf[i].level0;
        level[1] = rmt_buf[i].level1;
        bitsInLevel[0] = (rmt_buf[i].duration0 + UART_BIT_DURATION_US / 2) / UART_BIT_DURATION_US;
        bitsInLevel[1] = (rmt_buf[i].duration1 + UART_BIT_DURATION_US / 2) / UART_BIT_DURATION_US;

        for (int j = 0; j < 2; j++) {
            while (bitsInLevel[j] > 0) {
                if (!isStartBitDetected) {
                    if (level[j] == 0) {
                        isStartBitDetected = true;
                        bitCount = 1;     
                        bitShift = 0;
                        currentByte = 0;
                    }
                    bitsInLevel[j]--;
                    continue;
                }

                if (bitCount >= UART_START_BIT_COUNT && bitCount <= UART_DATA_BITS)
                    currentByte |= (level[j] << bitShift++);

                bitCount++;

                if (bitCount == 10) {
                    if (level[j] == 1) {
                        uartRxBuf[recvIndex++] = currentByte;

                        if (recvIndex >= DUST_DATA_FRAME)
                            return;
                    }

                    isStartBitDetected = false;
                    bitCount = 0;
                    bitShift = 0;
                    currentByte = 0;
                }

                bitsInLevel[j]--;
            }
        }
    }
}

static void readPmValues(uint8_t* buf)
{
    dust.pm1_0 = (buf[10] << 8) | buf[11];
    dust.pm2_5 = (buf[12] << 8) | buf[13]; 
    dust.pm10  = (buf[14] << 8) | buf[15];

    ESP_LOGD(TAG, "PM1.0 = %u - PM2.5 = %u - PM10 = %u - AQI: %.3f",
            dust.pm1_0, dust.pm2_5, dust.pm10, dust.aqi);
}

bool getDustData(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    uint8_t dust_buf[DUST_DATA_FRAME] = {0};

    // rmtDebugPrint();

    parseRmtToUart(dust_buf);

    bool ret = checkHeaderBytes(dust_buf);
    if (ret == false)
        goto start_rmt_rx;

    readPmValues(dust_buf);

    pm2_5ToAqi();

start_rmt_rx:
    rmt_receive(rmt_rx_channel, rmt_buf, sizeof(rmt_buf), &rmt_recv_cfg);

    return ret;
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