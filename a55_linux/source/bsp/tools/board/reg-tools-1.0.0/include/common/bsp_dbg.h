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

#ifndef BSP_DBG_H
#define BSP_DBG_H

#include "bsp_type.h"
#include "bsp_message.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if defined(DEBUG)
#define  dbg_assert(expression) assert(expression)
#else
#define dbg_assert(expression)   ((void)0)
#endif

#define LOG_LEVEL_NORMAL    0
#define LOG_LEVEL_FATAL     1
#define LOG_LEVEL_ERROR     2
#define LOG_LEVEL_WARN      3
#define LOG_LEVEL_INFO      4

#define LOG_LEVEL_DEBUG     5
#define LOG_LEVEL_DEBUG1    6
#define LOG_LEVEL_DEBUG2    7

#define LOG_LEVEL_ALL       10
#define LOG_LEVEL_NONE      0

#define LOG_LEVEL_DEFAULT   LOG_LEVEL_INFO
#define LOG_LEVEL_MINI      LOG_LEVEL_ERROR
#define LOG_LEVEL_MAX       LOG_LEVEL_ALL

#ifdef LOGQUEUE

static inline int log_create(unsigned char level, unsigned long msglen)
{
	log_queue *g_log_queue = log_get_global_queue();
	if (g_log_queue == NULL)
		return TD_FAILURE;
	return bsp_log_create(g_log_queue, level, msglen);
}

static inline void log_setlevel(unsigned char level)
{
	log_queue *g_log_queue = log_get_global_queue();
	if (g_log_queue == NULL)
		return;
	g_log_queue->uc_min_level = level;
}

static inline unsigned char log_getlevel(void)
{
	log_queue *g_log_queue = log_get_global_queue();
	if (g_log_queue == NULL)
		return 0;
	return g_log_queue->uc_min_level;
}

static inline void log_destroy(void)
{
	log_queue *g_log_queue = log_get_global_queue();
	if (g_log_queue == NULL)
		return;
	bsp_log_destroy(g_log_queue);
}


#define write_log_normal(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_NORMAL, fmt, ##args)

#define write_log_info(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_INFO, fmt, ##args)

#define write_log_fatal(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_FATAL, fmt, ##args)

#define write_log_error(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_ERROR, fmt, ##args)

#define write_log_warn(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_WARN, fmt, ##args)

#define write_log_debug(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_DEBUG, "{%s:%d}"fmt, __FILE__, __LINE__, ##args)

#define write_log_debug1(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_DEBUG1, "{%s:%d}"fmt, __FILE__, __LINE__, ##args)

#define write_log_debug2(fmt, args...) \
	bsp_log_msg(log_get_global_queue(), LOG_LEVEL_DEBUG2, "{%s:%d}"fmt, __FILE__, __LINE__, ##args)

#define write_log(i_level, fmt, args...) \
	bsp_log_msg(log_get_global_queue(), i_level, fmt, ##args)

#else

static inline int log_create(unsigned char level, unsigned long msglen)
{
	return TD_SUCCESS;
}

static inline void log_setlevel(unsigned char level)
{
	return;
}

static inline unsigned char log_getlevel(void)
{
	return 0;
}

static inline void log_destroy(void)
{
	return;
}


#define write_log_normal(fmt, args...)

#define write_log_info(fmt, args...)

#define write_log_fatal(fmt, args...)

#define write_log_error(fmt, args...)

#define write_log_warn(fmt, args...)

#define write_log_debug(fmt, args...)

#define write_log_debug1(fmt, args...)

#define write_log_debug2(fmt, args...)

#define write_log(fmt, args...)

#endif

void bsp_hexdump(FILE *stream, const void *src, size_t len, size_t width);
void bsp_hexdump2(FILE *stream, const void *src, size_t len, size_t width);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* BSP_DBG_H */
