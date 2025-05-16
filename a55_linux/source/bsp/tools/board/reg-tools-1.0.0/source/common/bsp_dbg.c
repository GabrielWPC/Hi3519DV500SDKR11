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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "memmap.h"
#include "bsp_config.h"

#if defined(DEBUG)
#include <assert.h>
#endif

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_ROW (0x8000000) /* =max_len/16 */
#define BSP_HEXDUMP2_PRINT_WITH 4

void bsp_hexdump(FILE *stream, const void *src, size_t len, size_t width)
{
	unsigned int rows, pos, i;
	const unsigned char *start = NULL;
	const unsigned char *row_pos = NULL;
	const unsigned char *data = NULL;

	if (width == 0)
		return;

	data = src;
	start = data;
	pos = 0;
	rows = (len % width) == 0 ? len / width : len / width + 1;
	for (i = 0; i < rows; i++) {
		row_pos = data;
		(void)fprintf(stream, "%05x: ", pos);
		do {
			unsigned int val = *data++ & 0xff;
			if ((size_t)(data - start) <= len)
				(void)fprintf(stream, " %02x", val);
			else
				(void)fprintf(stream, "   ");
		} while (((data - row_pos) % width) != 0);

		(void)fprintf(stream, "  |");
		data -= width;
		do {
			char c = *data++;
			if (isprint(c) == 0 || c == '\t')
				c = '.';
			if ((size_t)(data - start) <= len)
				(void)fprintf(stream, "%c", c);
			else
				(void)fprintf(stream, " ");
		} while (((data - row_pos) % width) != 0);
		(void)fprintf(stream, "|\n");
		pos += width;
	}
	(void)fflush(stream);
}

void bsp_hexdump2(FILE *stream, const void *src, size_t len, size_t width)
{
	unsigned int c, i, rows;
	const unsigned int *row_pos = NULL;
	const unsigned int *data = NULL;

	if (width == 0)
		return;

	data = src;
	rows = (len % width) == 0 ? len / width : len / width + 1;

	if (rows > MAX_ROW) {
		printf("error:Input param(len) is greater than 2GB!\n");
		return;
	}

	for (i = 0; i < rows; i++) {
		row_pos = data;
		(void)fprintf(stream, "%04x: ", i * 0x10);
		do {
			c = *data++;
			(void)fprintf(stream, " %08x", c);
		} while (((data - row_pos) % BSP_HEXDUMP2_PRINT_WITH) != 0);
		(void)fprintf(stream, "\n");
	}
	(void)fflush(stream);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
