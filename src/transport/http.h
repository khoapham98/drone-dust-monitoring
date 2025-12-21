/**
 * @file    http.h
 * @brief   HTTP state handlers for FSM control and state transition logic
 */
#ifndef _HTTP_H_
#define _HTTP_H_

#define     MAX_HTTP_HEADERS        10
#define     MAX_HEADER_LEN          256

enum connectionTimeout {
    HTTP_CONNECTION_TIMEOUT_20S = 20,
    HTTP_CONNECTION_TIMEOUT_40S = 40,
    HTTP_CONNECTION_TIMEOUT_60S = 60,
    HTTP_CONNECTION_TIMEOUT_80S = 80,
    HTTP_CONNECTION_TIMEOUT_100S= 100,
    HTTP_CONNECTION_TIMEOUT_120S= 120
};

enum inputTimeout {
    HTTP_DATA_INPUT_TIMEOUT_10S  = 10,
    HTTP_DATA_INPUT_TIMEOUT_20S  = 20,
    HTTP_DATA_INPUT_TIMEOUT_30S  = 30,
    HTTP_DATA_INPUT_TIMEOUT_40S  = 40,
    HTTP_DATA_INPUT_TIMEOUT_60S  = 60,
    HTTP_DATA_INPUT_TIMEOUT_90S  = 90,
    HTTP_DATA_INPUT_TIMEOUT_120S = 120,
    HTTP_DATA_INPUT_TIMEOUT_180S = 180,
    HTTP_DATA_INPUT_TIMEOUT_300S = 300,
    HTTP_DATA_INPUT_TIMEOUT_600S = 600,
    HTTP_DATA_INPUT_TIMEOUT_1800S= 1800,
    HTTP_DATA_INPUT_TIMEOUT_3600S= 3600
};


enum httpMethod {
    GET,
    POST,
    HEAD,
    DELETE,
    PUT
};

typedef enum httpMethod eHttpMethod;
typedef enum connectionTimeout eConnTimeout;
typedef enum inputTimeout eInputTimeout;

struct http_context_t
{
    char* url;
    char* contentType;
    char* acceptType;
    char* header[MAX_HTTP_HEADERS];
    int headerCount;
    eHttpMethod  method;
    eConnTimeout ConnTimeout; 
    eInputTimeout inputTimeout;
};

typedef struct http_context_t http_ctx_t;

/**
 * @brief Handle HTTP finite state machine.
 * @param state Current HTTP FSM state.
 * @return None.
 */
void httpFsmHandler(eHttpState state);

/**
 * @brief Initialize HTTP context structure.
 * @param http_ctx Pointer to HTTP context.
 * @return None.
 */
void http_context_init(http_ctx_t* http_ctx);

#endif