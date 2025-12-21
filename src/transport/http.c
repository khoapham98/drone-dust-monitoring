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
    "HTTP_STATE_START",
    "HTTP_STATE_SET_PARAM",
    "HTTP_STATE_INPUT_DATA",
    "HTTP_STATE_ACTION",
    "HTTP_STATE_STOP"
};

static http_ctx_t ctx = {0};

static eHttpState preState;

extern ring_buffer_t json_ring_buf;
extern bool jsonReady;
extern pthread_cond_t jsonCond;
extern pthread_mutex_t jsonLock;

static void updateHttpState(eSimResult res, eHttpState backState, eHttpState nextState)
{
    if (res == FAIL) {
        if (getHttpState() == HTTP_STATE_START) {
            setFsmLayer(FSM_LAYER_SIM);
            return;
        }

        setHttpState(preState);
        preState = backState;
    }
    else if (res == PASS) {
        preState = getHttpState();
        setHttpState(nextState);
    }
    else {
        sleep(1);
    }
}

static void httpStartStatusHandler(void)
{
    eSimResult res = httpStartService();
    updateHttpState(res, HTTP_STATE_START, HTTP_STATE_SET_PARAM);
}

static void httpSetParamStatusHandler(void)
{
    eSimResult res = PASS;
    if (ctx.url != NULL) {
        res = httpSetUrl(ctx.url);
        if (res != PASS) goto end;
    }

    res = httpSetConnectionTimeout(ctx.ConnTimeout);
    if (res != PASS) goto end;

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
    updateHttpState(res, HTTP_STATE_START, HTTP_STATE_INPUT_DATA);
}

static void httpStopStatusHandler(void)
{
    eSimResult res = httpStopService();
    updateHttpState(res, HTTP_STATE_INPUT_DATA, HTTP_STATE_START);
}

static void httpInputDataStatusHandler(char* data, size_t len)
{
    eSimResult res = httpSendData(data, len, ctx.inputTimeout);
    updateHttpState(res, HTTP_STATE_START, HTTP_STATE_ACTION);
}

static void httpActionStatusHandler(void)
{
    eSimResult res = httpSendAction(ctx.method);
    updateHttpState(res, HTTP_STATE_SET_PARAM, HTTP_STATE_STOP);
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

bool isHttpFsmRunning = false;
static char data[RING_BUFFER_SIZE] = {0};

void httpFsmHandler(eHttpState state)
{
    if (state == HTTP_STATE_START && !isHttpFsmRunning) {
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
    case HTTP_STATE_START:
        httpStartStatusHandler();
        break;
    case HTTP_STATE_SET_PARAM:
        httpSetParamStatusHandler();
        break;
    case HTTP_STATE_INPUT_DATA:
        httpInputDataStatusHandler(data, strlen(data));
        memset(data, 0, sizeof(data));
        break;
    case HTTP_STATE_ACTION:
        httpActionStatusHandler();
        break;
    case HTTP_STATE_STOP:
        httpStopStatusHandler();
        isHttpFsmRunning = false;
        break;
    default:
        break;
    }
}