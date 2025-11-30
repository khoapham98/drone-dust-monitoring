/**
 * @file    log.h
 * @brief   print log header file
 */
#ifndef _LOG_H_
#define _LOG_H_

#define LOG_INF(fmt, ...) printf("[INF] " fmt "\n", ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) printf("[WRN] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) printf("[ERR] " fmt "\n", ##__VA_ARGS__)

#endif