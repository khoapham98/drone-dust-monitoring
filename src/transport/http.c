/**
 * @file    http.c
 * @brief   HTTP state handlers for FSM control and state transition logic
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/log.h"
#include "sys/ringbuffer.h"
#include "sys/json.h"
#include "fsm/fsm.h"
#include "sim/sim_cmd.h"
#include "http.h"

static const char* httpStateStr[] = {
    "HTTP_STATE_PREPARE",
    "HTTP_STATE_SEND",
    "HTTP_STATE_STOP"
};

static http_ctx_t ctx = {0};
static char data[RING_BUFFER_SIZE] = {0};
bool isHttpFsmRunning = false;

extern ring_buffer_t json_ring_buf;
extern bool jsonReady;
extern pthread_cond_t jsonCond;
extern pthread_mutex_t jsonLock;

static void httpPrepareStatusHandler(void)
{
    eSimResult res = httpStartService();
    if (res != PASS) {
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
    eSimResult res = httpSendData(data, strlen(data), ctx.inputTimeout);
    memset(data, 0, sizeof(data));
    if (res != PASS)
        goto end;

    httpSendAction(ctx.method);

end:
    setHttpState(HTTP_STATE_STOP);
}

static void httpStopStatusHandler(void)
{
    httpStopService();
    setHttpState(HTTP_STATE_PREPARE);
}

void httpFsmHandler(eHttpState state)
{
    if (state == HTTP_STATE_PREPARE && !isHttpFsmRunning) {
        pthread_mutex_lock(&jsonLock);
        while (!jsonReady)
            pthread_cond_wait(&jsonCond, &jsonLock);

        getJsonData(&json_ring_buf, data);
        jsonReady = false;
        pthread_mutex_unlock(&jsonLock);
        isHttpFsmRunning = true;
    }

    if (!isHttpFsmRunning)
        return;

    LOG_INF("%s", httpStateStr[state]);
    switch (state)
    {
    case HTTP_STATE_PREPARE:
        httpPrepareStatusHandler();
        break;
    case HTTP_STATE_SEND:
        httpSendStatusHandler();
        break;
    case HTTP_STATE_STOP:
        httpStopStatusHandler();
        isHttpFsmRunning = false;
        break;
    default:
        break;
    }
}

void http_context_init(http_ctx_t* http_ctx)
{
    if (http_ctx->url == NULL) {
        LOG_WRN("HTTP URL is missing!!!");
        LOG_ERR("Configure HTTP context failed");     
        return;
    }

    ctx.url = http_ctx->url;
    ctx.method = http_ctx->method;
    ctx.ConnTimeout  = http_ctx->ConnTimeout;
    ctx.inputTimeout = http_ctx->inputTimeout;

    if (ctx.headerCount > MAX_HTTP_HEADERS)
        ctx.headerCount = MAX_HTTP_HEADERS;
    else 
        ctx.headerCount = http_ctx->headerCount;

    if (http_ctx->contentType == NULL) {
        LOG_WRN("Set HTTP content type to default \"text/plain\"");
        goto header_init;
    }

    if (strlen(http_ctx->contentType) > MAX_HEADER_LEN) {
        LOG_WRN("HTTP content type too long (%d bytes) - skip", strlen(http_ctx->contentType));
        goto header_init;
    }
    ctx.contentType = http_ctx->contentType;

    if (http_ctx->acceptType == NULL) {
        LOG_WRN("Set HTTP accept type to default \"*/*\"");
        goto header_init;
    }
    
    if (strlen(http_ctx->acceptType) > MAX_HEADER_LEN) {
        LOG_WRN("HTTP accept type too long (%d bytes) - skip", strlen(http_ctx->acceptType));
        goto header_init;
    }
    ctx.acceptType = http_ctx->acceptType;


    int cnt = 0;
header_init:
    for (int i = 0; i < ctx.headerCount; i++) {
        if (strlen(http_ctx->header[i]) > MAX_HEADER_LEN) {
            LOG_WRN("HTTP header too long (%d bytes) - skip", strlen(http_ctx->header[i]));
            continue;
        }

        ctx.header[cnt++] = http_ctx->header[i];
    }
    
    if (cnt != ctx.headerCount) {
        LOG_WRN("Total %d HTTP headers have been skipped", ctx.headerCount - cnt);
        ctx.headerCount = cnt;
    }
}
