/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __LIB_H__
#define __LIB_H__

#include <types.h>

errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count);
errno_t memset_s(void *dest, size_t dest_max, int c, size_t count);
errno_t strcpy_s(char *strDest, size_t dest_max, const char *strSrc);

#ifdef CFG_DEBUG
size_t strnlen(const char *s, size_t count);
void printf(const char *fmt, ...);
void set_time_mark();
unsigned long get_time_ms();

#else /* CFG_DEBUG */

#define printf(...)
#define set_time_mark(...)
#define get_time_ms(...)

#endif /* CFG_DEBUG */

int malloc_init(unsigned int start, unsigned size);

errno_t memmove_s(void *dest, size_t dest_max, const void *src, size_t count);
void *malloc(size_t size);
void free(void *mem_ptr);

void udelay(unsigned long us);
void mdelay(unsigned long msec);

#ifndef readl
#define readl(addr) (*(volatile unsigned int *)(uintptr_t)(addr))
#endif

#ifndef writel
#define writel(val, addr) (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#endif

#endif /* __LIB_H__ */