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
#include "bsp_config.h"
#include "bsp_dbg.h"
#include "bsp_error.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#define STRFMT_ERRCODE "%#010lX"

#ifdef DEBUG
void err_exit(const char *msg, int err_code)
{
	if (msg == NULL) {
		printf("msg is null!\n");
		return;
	}

	write_log_error("%s exit:"STRFMT_ERRCODE".{%s:%d}\n", msg, err_code, __FILE__, __LINE__);
	printf("[END]\n");
	exit(err_code);
}
#else
void err_exit(const char *msg, int err_code)
{
	if (msg == NULL) {
		printf("msg is null!\n");
		return;
	}

	printf("[END]\n");
	(exit(err_code));
}
#endif
