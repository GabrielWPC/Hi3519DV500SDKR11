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

#include <stdlib.h>
#include <string.h>
#include "bsp_type.h"
#include "strfunc.h"

#define HEX_HEAD_LEN 2
#define DECIMAL 10
#define HEXADECIMAL 16

int str2number(const char *str, unsigned long *number)
{
	const char *start_ptr = NULL;
	char *end_ptr = NULL;
	int radix;

	if (str == NULL || number == NULL)
		return TD_FAILURE;

	if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
		if (*(str + HEX_HEAD_LEN) == '\0')
			return TD_FAILURE;
		start_ptr = str + HEX_HEAD_LEN;
		radix = HEXADECIMAL;
	} else {
		start_ptr = str;
		radix = DECIMAL;
	}

	*number = strtoul(start_ptr, &end_ptr, radix);
	if ((start_ptr + strlen(start_ptr)) != end_ptr)
		return TD_FAILURE;

	return TD_SUCCESS;
}
