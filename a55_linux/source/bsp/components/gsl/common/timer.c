/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include <types.h>
#include <platform.h>
#include <lib.h>

#define TIMER_DIVIDER_MS  (TIMER_FEQ / TIMER_DIV / 1000)

unsigned long timer_get_divider()
{
	return TIMER_DIVIDER_MS;
}
