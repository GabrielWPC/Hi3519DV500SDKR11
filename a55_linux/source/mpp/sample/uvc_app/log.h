/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __UVC_LOG_H__
#define __UVC_LOG_H__

#include <stdlib.h>
#include <stdio.h>


#define log(...) fprintf(stderr, __VA_ARGS__)
#define rlog(format, ...) fprintf(stderr, "\033[;31m" format "\033[0m\n", ##__VA_ARGS__)

#define info(...) log_info("info", __func__, __LINE__, __VA_ARGS__)
#define err(...) log_info("Error", __func__, __LINE__, __VA_ARGS__)
#define err_on(cond, ...) ((cond) ? err(__VA_ARGS__) : 0)

#define crit(...)                                              \
    do {                                                       \
        log_info("Critical", __func__, __LINE__, __VA_ARGS__); \
        exit(0);                                               \
    } while (0)

#define crit_on(cond, ...)     \
    do {                       \
        if (cond)              \
            crit(__VA_ARGS__); \
    } while (0)

void log_info(const char *prefix, const char *file, int line, const char *fmt, ...);

#endif // __UVC_LOG_H__
