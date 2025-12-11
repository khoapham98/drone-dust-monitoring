/**
 * @file    at_cmd.c
 * @brief   Low-level AT command source file for UART communication
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "sys/log.h"
#include "at_cmd.h"
#include "drivers/uart.h"

static int uart_fd = 0;
static char* resp_ptr = NULL;
static size_t resp_size = 0;

static uint64_t now_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void at_attach_resp_buffer(char* buf, size_t len)
{
    resp_ptr = buf;
    resp_size = len;
}

int at_send_wait(char* cmd, uint64_t timeout_ms)
{
    char resp[RESP_FRAME] = {0};

    int written = at_send(cmd, strlen(cmd));
    if (written < 0) 
        return -1;

    int num = at_read(resp, sizeof(resp), timeout_ms);
    if (num < 0) 
        return -1;

    if (resp_ptr != NULL && resp_size > 0) {
        int len = (strlen(resp) > resp_size) ? resp_size : strlen(resp); 
        strncpy(resp_ptr, resp, len);
    }

    LOG_INF("Send: %s\nResponse:%s", cmd, resp);
    return 0;    
}

int at_send(char* cmd, size_t len)
{
    if (uart_fd < 0)
        return -1;

    size_t total = 0;

    while (total < len) {
        ssize_t ret = write(uart_fd, cmd + total, len - total);
        if (ret <= 0) {
            if (errno == EINTR) {
                continue;
            }
            else {
                LOG_ERR("Write failed: %s", strerror(errno));
                return -1;
            }
        }
        total += ret;
    }
    return total;
}

int at_read(char* buf, size_t max_len, uint64_t timeout_ms)
{
    if (uart_fd < 0 || buf == NULL || max_len == 0)
        return -1;

    size_t idx = 0;
    uint64_t start = now_ms();
    uint64_t last_rx = now_ms();   

    const uint64_t QUIET_MS = 80;  

    while (1)
    {
        if (idx >= max_len - 1)
            break;

        char c;
        ssize_t ret = read(uart_fd, &c, 1);

        if (ret > 0) {
            buf[idx++] = c;
            last_rx = now_ms();
        }
        else if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERR("Read failed: %s", strerror(errno));
            return -1;
        }

        buf[idx] = '\0';

        bool have_ok    = (strstr(buf, "\r\nOK\r\n")    != NULL);
        bool have_error = (strstr(buf, "\r\nERROR\r\n") != NULL);

        if ((have_ok || have_error) && (now_ms() - last_rx >= QUIET_MS)) 
            break;

        if (now_ms() - start >= timeout_ms)
            break;

        usleep(1000);
    }

    buf[idx] = '\0';
    return idx;
}

int sim_init(char* uart_file_path)
{
    uart_fd = uart_init(uart_file_path, true);
    if (uart_fd < 0) {
        return -1;
	}
    
	LOG_INF("Sim Initialization successful");
    return 0;
}