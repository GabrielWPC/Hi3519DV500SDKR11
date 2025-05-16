/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdarg.h>
#include <errno.h>
#include "log.h"

void log_info(const char *prefix, const char *file, int line,
              const char *fmt, ...)
{
    if (prefix == NULL || file == NULL || fmt == NULL) {
        return;
    }
    int errsv = errno;
    va_list va;

    va_start(va, fmt);
    (void)fprintf(stderr, "%s:(%s:%d): ", prefix, file, line);
    (void)vfprintf(stderr, fmt, va);
    va_end(va);
    errno = errsv;

    return;
}
