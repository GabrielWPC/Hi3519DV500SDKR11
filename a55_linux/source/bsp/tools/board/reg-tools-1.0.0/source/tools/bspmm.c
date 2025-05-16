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

#define DEFAULT_MD_LEN 128
#define NUM_2 2
#define NUM_3 3

int bspmm(int argc, char *argv[])
{
	unsigned long ul_addr = 0;
	unsigned long ul_old, ul_new;
	void *p_mem  = NULL;

	if (argc <= 1) {
		printf("usage: %s <address> <Value>. sample: %s 0x80040000 0x123\n", argv[0], argv[0]);
		err_exit("", -1);
	}

	if (argc == NUM_2) {
		if (str2number(argv[1], &ul_addr) == TD_SUCCESS) {
			char str_new[16];
			printf("====dump memory %#lX====\n", ul_addr);

			p_mem = memmap(ul_addr, DEFAULT_MD_LEN);
			if (p_mem == NULL) {
				printf("memmap failed!\n");
				return -1;
			}

			ul_old = *(unsigned int *)p_mem;
			printf("%s: 0x%08lX\n", argv[1], ul_old);
			printf("NewValue:");
			(void)fflush(stdout);
			if (scanf_s("%s", str_new, sizeof(str_new)) < 0) {
				printf("scanf_s failed!\n");
				return -1;
			}

			if (str2number(str_new, &ul_new) == TD_SUCCESS)
				*(unsigned int *)p_mem = ul_new;
			else
				printf("Input Error\n");
		} else {
			printf("Please input address like 0x12345\n");
		}
	} else if (argc == NUM_3) {
		if (str2number(argv[1], &ul_addr) == TD_SUCCESS &&
			str2number(argv[NUM_2], &ul_new) == TD_SUCCESS) {
			p_mem = memmap(ul_addr, DEFAULT_MD_LEN);
			if (p_mem == NULL) {
				printf("memmap failed!\n");
				return -1;
			}
			ul_old = *(unsigned int *)p_mem;
			printf("%s: 0x%08lX --> 0x%08lX \n", argv[1], ul_old, ul_new);
			*(unsigned int *)p_mem = ul_new;
		}
	} else {
		printf("xxx\n");
	}
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

