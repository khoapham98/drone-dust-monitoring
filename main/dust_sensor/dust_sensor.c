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
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "esp_attr.h" 
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "../boards/board.h"
#include "dust_sensor.h"

/* message buffer */
static MessageBufferHandle_t msgBuffer;
static const size_t msgBufferSizeBytes = 64;

/* logger */
static const char* TAG = "dust_sensor";

/* uart software management */
static bool isFirstAlarmEnable = false;
static uint16_t bitIndex = START_BIT;
static uint8_t byteIndex = 0;

/* gpio */
static const gpio_config_t gpio_cfg = {
    .pin_bit_mask = (1ULL << UART_SW_RX_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
};

/* timer */
static gptimer_handle_t gptimerHandle;

static bool IRAM_ATTR gptimer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

static gptimer_alarm_config_t first_alarm = {
    .alarm_count = 52,
    .reload_count = 0,
    .flags.auto_reload_on_alarm = false
};

static gptimer_alarm_config_t bit_alarm = {
    .alarm_count = 104,
    .reload_count = 0,
    .flags.auto_reload_on_alarm = true
};

static const gptimer_config_t gptimer_cfg = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000,
    .intr_priority = 1
};

static const gptimer_event_callbacks_t gptimer_cb = {
    .on_alarm = gptimer_alarm_cb
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

bool getDustData(void)
{
    uint8_t dustBuffer[DUST_DATA_FRAME] = {0};

    size_t receivedBytes = xMessageBufferReceive(msgBuffer, dustBuffer, DUST_DATA_FRAME, portMAX_DELAY);
    
    if (receivedBytes != DUST_DATA_FRAME) {
        ESP_LOGE(TAG, "Dust frame size mismatch: expected %d bytes, got %d bytes",
                DUST_DATA_FRAME, receivedBytes);
        return false;
    }

    if (!checkHeaderBytes(dustBuffer)) {
        ESP_LOGE(TAG, "Dust frame header invalid");
        return false;
    }

    ctx.pm25 = (dustBuffer[12] << 8) | dustBuffer[13]; 

    pm25ToAqi();

    return true;
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    if (gpio_get_level(UART_SW_RX_PIN) != 0)
        return;

    if (gpio_intr_disable(UART_SW_RX_PIN) != ESP_OK)
        return;

    byteIndex = 0;

    if (gptimer_set_alarm_action(gptimerHandle, &first_alarm) != ESP_OK)
        return;

    isFirstAlarmEnable = true;

    gptimer_start(gptimerHandle);
}

static bool gptimer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    if (isFirstAlarmEnable) {
        if (gptimer_set_alarm_action(gptimerHandle, &bit_alarm) != ESP_OK)
            return false;

        isFirstAlarmEnable = false;
    }
    
    static uint8_t byte = 0;
    static uint8_t bitShift = 0;
    static uint8_t buf[DUST_DATA_FRAME] = {0};

    if (bitIndex == START_BIT) {
        bitIndex++;
    }
    else if (bitIndex >= STOP_BIT) {
        if (gpio_get_level(UART_SW_RX_PIN) == 1) {
            buf[byteIndex++] = byte;
        }

        bitIndex = START_BIT;
        bitShift = 0;
        byte = 0;
    }
    else {
        uint8_t bit = gpio_get_level(UART_SW_RX_PIN);
        byte |= (bit << bitShift++);
        bitIndex++;
    }

    if (byteIndex >= DUST_DATA_FRAME) {
        xMessageBufferSendFromISR(msgBuffer, (void*) buf, DUST_DATA_FRAME, NULL);
        gptimer_stop(gptimerHandle);
        gpio_intr_enable(UART_SW_RX_PIN);
    }

    return true;
}

int dust_sensor_sw_uart_init(void)
{
    msgBuffer = xMessageBufferCreate(msgBufferSizeBytes);
    if (msgBuffer == NULL) {
        ESP_LOGE(TAG, "Create message buffer failed");
        return -1;
    }

    esp_err_t ret = gpio_config(&gpio_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO configuration failed");
        return -1;
    }

    ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Install GPIO ISR handler failed");
        return -1;
    }

    ret = gpio_isr_handler_add(UART_SW_RX_PIN, gpio_isr_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Add GPIO ISR handler failed");
        return -1;
    }
    
    ret = gpio_intr_enable(UART_SW_RX_PIN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Enable GPIO interrupt failed");
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