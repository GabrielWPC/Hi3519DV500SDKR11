/*
 *
 * Copyright (c) 2012-2021 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "memmap.h"
#include "bsp_config.h"
#include "bsp_type.h"
#include "bsp_dbg.h"
#include "bsp_error.h"
#include "strfunc.h"
#include "securec.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define BSPMD_DEFAULT_LEN 256
#define BSPMD_MAX_LEN (64 * 1024) /* 64K */
#define BSPMD_DISPLAY_BYTE_WIDTH 16
#define NUM_2 2
#define NUM_3 3

int bspmd_l(int argc, char *argv[])
{
	unsigned long ul_addr = 0;
	void *p_mem  = NULL;
	unsigned long len;

	if (argc < NUM_2) {
		printf("usage: %s <address> [length]. sample: %s 0x80040000 0x100\n", argv[0], argv[0]);
		err_exit("", -1);
	}

	if (argc < NUM_3 || (str2number(argv[NUM_2], &len) != TD_SUCCESS))
		len = BSPMD_DEFAULT_LEN;
	if (len > BSPMD_MAX_LEN) {
		printf("[Warning]:the input length should not more than 0x%x\n", BSPMD_MAX_LEN);
		len = BSPMD_MAX_LEN;
	}

	if (str2number(argv[1], &ul_addr) == TD_SUCCESS) {
		printf("====dump memory 0x%08lX====\n", ul_addr);

		p_mem = memmap(ul_addr, len);
		if (p_mem == NULL)
			err_exit("Memory Map error", -1);

		bsp_hexdump2(stdout, p_mem, len, BSPMD_DISPLAY_BYTE_WIDTH);
	} else {
		printf("Please input address like 0x12345\n");
	}

	return 0;
}

int bspmd(int argc, char *argv[])
{
	unsigned long ul_addr = 0;
	void *p_mem  = NULL;
	unsigned long len;

	if (argc < NUM_2) {
		printf("usage: %s <address>. sample: %s 0x80040000\n", argv[0], argv[0]);
		err_exit("", -1);
	}

	if (argc < NUM_3 || str2number(argv[NUM_2], &len) != TD_SUCCESS)
		len = BSPMD_DEFAULT_LEN;

	if (len > BSPMD_MAX_LEN) {
		printf("[Warning]:the input length should not more than 0x%x\n", BSPMD_MAX_LEN);
		len = BSPMD_MAX_LEN;
	}

	if (str2number(argv[1], &ul_addr) == TD_SUCCESS) {
		printf("====dump memory %#010lX====\n", ul_addr);

		p_mem = memmap(ul_addr, len);
		if (p_mem == NULL)
			err_exit("Memory Map error", -1);

		bsp_hexdump(stdout, p_mem, len, BSPMD_DISPLAY_BYTE_WIDTH);
	} else {
		printf("Please input address like 0x40000000\n");
	}

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
