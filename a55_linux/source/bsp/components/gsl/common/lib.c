/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include <lib.h>
#include <common.h>
#include <platform.h>

#ifdef CFG_DEBUG
#ifdef PRINT_TO_MEM
#define MEM_FOR_PRINT_BASE 0xf8400000
u32 mem_print_base;
#endif

#define CFG_PBSIZE 256

void printf(const char *fmt, ...)
{
}

size_t strnlen(const char *s, size_t count)
{
	const char *sc;
	for (sc = s; count-- && *sc != '\0'; ++sc);
	return sc - s;
}

void set_time_mark()
{
	timer_start();
}

unsigned long  get_time_ms()
{
	return timer_get_val() / timer_get_divider();
}
#endif