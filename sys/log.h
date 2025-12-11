/**
 * @file    log.h
 * @brief   print log header file
 */
#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "device_setup.h"

static inline void log_output(const char *tag, const char *fmt, ...) {
    va_list args;

    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info); 
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%d-%m-%Y %H:%M:%S", &tm_info);

#if LOG_TO_CONSOLE
    va_start(args, fmt);
    printf("[%s] [%s] ", time_buf, tag);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
#endif

#if LOG_TO_FILE
    FILE *fp = fopen(LOG_FILE_PATH, "a");
    if (fp) {
        va_start(args, fmt);
        fprintf(fp, "[%s] [%s] ", time_buf, tag);
        vfprintf(fp, fmt, args);
        fprintf(fp, "\n");
        fclose(fp);
        va_end(args);
    }
#endif
}

#define LOG_INF(fmt, ...) log_output("INF", fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) log_output("WRN", fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) log_output("ERR", fmt, ##__VA_ARGS__)

#endif