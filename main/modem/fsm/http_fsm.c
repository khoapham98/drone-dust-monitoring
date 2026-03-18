/**
 * @file    http_fsm.c
 * @brief   HTTP state handlers for FSM control and state transition logic
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "fsm_manager.h"
#include "http_service.h"
#include "http_fsm.h"

extern MessageBufferHandle_t msgBufHandle;

static const char* TAG = "http_fsm";

static const char* httpStateStr[] = {
    "HTTP_STATE_WAIT_TRIGGER",
    "HTTP_STATE_PREPARE",
    "HTTP_STATE_SEND",
    "HTTP_STATE_STOP"
};

static http_ctx_t ctx = {0};

static char payloadBuffer[PAYLOAD_BUFFER_SIZE] = {0};
static int payloadLen = 0;

static uint64_t lastUploadTime = 0;

static bool isReadyToUpload(void)
{
    uint64_t now = esp_timer_get_time() / 1000;

    if (now - lastUploadTime>= HTTP_POST_INTERVAL_MS) 
        return true;
   
    return false;
}

static void httpWaitForTriggerStatusHandler() 
{
    payloadLen = xMessageBufferReceive(msgBufHandle, payloadBuffer, sizeof(payloadBuffer), pdMS_TO_TICKS(100));

    if (payloadLen == 0) return;

    if (!isReadyToUpload()) return;
    
    setHttpState(HTTP_STATE_PREPARE);
}

static void httpPrepareStatusHandler(void)
{
    eModemResult res = httpStartService();
    if (res != PASS) {
        httpStopService();
        setFsmLayer(FSM_LAYER_SIM);
        return;
    }

    res = httpSetConnectionTimeout(ctx.ConnTimeout);
    if (res != PASS) goto end;

    if (ctx.url != NULL) {
        res = httpSetUrl(ctx.url);
        if (res != PASS) goto end;
    }

    if (ctx.contentType != NULL) {
        res = httpSetContent(ctx.contentType);
        if (res != PASS) goto end;
    }

    if (ctx.acceptType != NULL) {
        res = httpSetAccept(ctx.acceptType);
        if (res != PASS) goto end;
    }

    for (int i = 0; i < ctx.headerCount; i++) {
        res = httpSetCustomHeader(ctx.header[i]);
        if (res != PASS) goto end;
    }

end:
    if (res != PASS)
        setHttpState(HTTP_STATE_STOP);
    else 
        setHttpState(HTTP_STATE_SEND);
}

static void httpSendStatusHandler(void)
{
    eModemResult res = httpSendData(payloadBuffer, payloadLen, ctx.inputTimeout);

    if (res != PASS) goto end;

    httpSendAction(ctx.method);

end:
    lastUploadTime = esp_timer_get_time() / 1000;

    memset(payloadBuffer, 0, payloadLen);

    setHttpState(HTTP_STATE_STOP);
}

static void httpStopStatusHandler(void)
{
    httpStopService();

    setHttpState(HTTP_STATE_WAIT_TRIGGER);
}

void httpFsmHandler(eHttpState state)
{
    if (state != HTTP_STATE_WAIT_TRIGGER)
        ESP_LOGI(TAG, "Current state: %s", httpStateStr[state]);

    switch (state)
    {
    case HTTP_STATE_WAIT_TRIGGER:
        httpWaitForTriggerStatusHandler();
        break;
    case HTTP_STATE_PREPARE:
        httpPrepareStatusHandler();
        break;
    case HTTP_STATE_SEND:
        httpSendStatusHandler();
        break;
    case HTTP_STATE_STOP:
        httpStopStatusHandler();
        break;
    default:
        break;
    }
}

void http_context_init(http_ctx_t* http_ctx)
{
    if (http_ctx->url == NULL) {
        ESP_LOGW(TAG, "HTTP URL is missing - Configure HTTP context failed");     
        return;
    }

    int cnt = 0;

    configASSERT(msgBufHandle != NULL);

    ctx.url = http_ctx->url;
    ctx.method = http_ctx->method;
    ctx.ConnTimeout  = http_ctx->ConnTimeout;
    ctx.inputTimeout = http_ctx->inputTimeout;

    if (ctx.headerCount > MAX_HTTP_HEADERS)
        ctx.headerCount = MAX_HTTP_HEADERS;
    else 
        ctx.headerCount = http_ctx->headerCount;

    if (http_ctx->contentType == NULL) {
        ESP_LOGW(TAG, "Set HTTP content type to default \"text/plain\"");
        goto set_accept_type;
    }

    if (strlen(http_ctx->contentType) > MAX_HEADER_LEN) {
        ESP_LOGW(TAG, "HTTP content type too long (%zu bytes) - skip", strlen(http_ctx->contentType));
        goto set_accept_type;
    }
    ctx.contentType = http_ctx->contentType;

set_accept_type:
    if (http_ctx->acceptType == NULL) {
        ESP_LOGW(TAG, "Set HTTP accept type to default \"*/*\"");
        goto set_headers;
    }
    
    if (strlen(http_ctx->acceptType) > MAX_HEADER_LEN) {
        ESP_LOGW(TAG, "HTTP accept type too long (%zu bytes) - skip", strlen(http_ctx->acceptType));
        goto set_headers;
    }
    ctx.acceptType = http_ctx->acceptType;

set_headers:
    for (int i = 0; i < ctx.headerCount; i++) {
        if (strlen(http_ctx->header[i]) > MAX_HEADER_LEN) {
            ESP_LOGW(TAG, "HTTP header too long (%zu bytes) - skip", strlen(http_ctx->header[i]));
            continue;
        }

        ctx.header[cnt++] = http_ctx->header[i];
    }
    
    if (cnt != ctx.headerCount) {
        ESP_LOGW(TAG, "Total %d HTTP headers have been skipped", ctx.headerCount - cnt);
        ctx.headerCount = cnt;
    }
}
