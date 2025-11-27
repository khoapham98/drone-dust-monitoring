#ifndef _LOG_H_
#define _LOG_H_

#define LOG_INF(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) printf("[ERRO] " fmt "\n", ##__VA_ARGS__)

#endif