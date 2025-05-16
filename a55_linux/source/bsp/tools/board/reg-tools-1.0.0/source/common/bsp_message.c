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


#include <stdarg.h>
#include "securec.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include "bsp_config.h"
#include "bsp_type.h"
#include "bsp_dbg.h"
#include "bsp_message.h"

#if defined(MULTI_TASK_LOGQUEUE)
#ifdef OS_LINUX
#include <pthread.h>
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static log_queue g_log_queue;

static const char *gppsz_type[] = {
	" ", "[fatal]:", "[error]:", "[warn]:",
	"[info]:", "[debug]:", "[debug1]:", "[debug2]:"
};

static void bsp_log_print_msg(log_queue *p_log_queue, unsigned int uc_log_level,
			const char *psz_format, va_list _args);

#if defined(MULTI_TASK_LOGQUEUE)
static void bsp_log_init_lock(log_queue *p_log_queue);
static int  bsp_log_lock(log_queue *p_log_queue);
static int  bsp_log_unlock(log_queue *p_log_queue);
static void bsp_log_destroy_lock(log_queue *p_log_queue);

/*-------- Lock Function , Maybe Replace later ---*/


static int bsp_log_lock(log_queue *p_log_queue)
{
#if defined(OS_LINUX)
	return pthread_mutex_lock(&(p_log_queue->mutex_lock));
#else
	return 0;
#endif
}

static int bsp_log_unlock(log_queue *p_log_queue)
{
#if defined(OS_LINUX)
	return pthread_mutex_unlock(&(p_log_queue->mutex_lock));
#else
	return 0;
#endif
}

static void bsp_log_init_lock(log_queue *p_log_queue)
{
#if defined(OS_LINUX)
	pthread_mutex_init(&p_log_queue->mutex_lock, NULL);
#else
	return 0;
#endif
}
static void bsp_log_destroy_lock(log_queue *p_log_queue)
{
	bsp_log_unlock(p_log_queue);
#if defined(OS_LINUX)
	pthread_mutex_destroy(&p_log_queue->mutex_lock);
#else
#endif
}

#else
static int bsp_log_lock(log_queue *p_log_queue)
{
	return 0;
}

static int bsp_log_unlock(log_queue *p_log_queue)
{
	return 0;
}

static void bsp_log_init_lock(log_queue *p_log_queue)
{
	return;
}

static void bsp_log_destroy_lock(log_queue *p_log_queue)
{
	return;
}
#endif

/* g_log_queue is a Global Var, now defined at bsp_message.c and
  declare at bsp_dbg.h, it you use this log functions outside,
  you can declare your owner "g_log_queue" */
log_queue *log_get_global_queue(void)
{
	return &g_log_queue;
}

/* --------- Message Functions -----*/
int bsp_log_create(log_queue *p_log_queue, unsigned char min_level, unsigned long len_msg)
{
	dbg_assert(p_log_queue);
	bsp_log_init_lock(p_log_queue);
	p_log_queue->uc_min_level   = min_level;
	p_log_queue->len_msg = len_msg;
	if (len_msg == 0)
		return TD_FAILURE;
	p_log_queue->p_msg = malloc(len_msg);
	if (p_log_queue->p_msg == NULL)
		return TD_FAILURE;
	return TD_SUCCESS;
}

void bsp_log_destroy(log_queue *p_log_queue)
{
	dbg_assert(p_log_queue);
	free(p_log_queue->p_msg);
	bsp_log_destroy_lock(p_log_queue);
	p_log_queue->p_msg = NULL;
}

void bsp_log_msg(log_queue *p_log_queue, unsigned int uc_log_level,
		 char *psz_format, ...)
{
	if (psz_format == NULL)
		return;

	dbg_assert(p_log_queue);

	if (p_log_queue->uc_min_level < uc_log_level)
		return;

	va_list args;

	va_start(args, psz_format);
	bsp_log_print_msg(p_log_queue, uc_log_level, psz_format, args);
	va_end(args);
}

void bsp_log_msg_va(log_queue *p_log_queue, unsigned int uc_log_level,
		const char *psz_format, va_list args)
{
	bsp_log_print_msg(p_log_queue, uc_log_level, psz_format, args);
}

/* following functions are local */

static void bsp_log_print_msg(log_queue *p_log_queue, unsigned int uc_log_level,
			 const char *psz_format, va_list _args)
{
	va_list  args;

	if (p_log_queue == NULL || psz_format == NULL)
		return;

	bsp_log_lock(p_log_queue);
	va_copy(args, _args);
	if (vsnprintf_s(p_log_queue->p_msg, p_log_queue->len_msg, p_log_queue->len_msg - 1, psz_format, args) < 0)
		printf("bsp_log_print_msg error.\n");

	va_end(args);
	p_log_queue->p_msg[ p_log_queue->len_msg - 1 ] = 0;

	if (uc_log_level)
		fprintf(stdout, "%s %s", gppsz_type[uc_log_level], p_log_queue->p_msg);
	else
		fprintf(stdout, "%s", p_log_queue->p_msg);

	bsp_log_unlock(p_log_queue);

	(void)fflush(stdout);
	return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
