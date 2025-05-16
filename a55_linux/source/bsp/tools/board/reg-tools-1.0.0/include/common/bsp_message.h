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
#if !defined( BSP_MESSAGE_H__ )

#if defined(MULTI_TASK_LOGQUEUE)
#ifdef OS_LINUX
#include <pthread.h>
#endif
#endif

#define BSP_MESSAGE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef struct log_queue_stru {
	/** Message queue lock */
#if defined(MULTI_TASK_LOGQUEUE)
	pthread_mutex_t mutex_lock;
#endif
	unsigned char uc_min_level;
	unsigned long len_msg;
	char *p_msg;
} log_queue;


log_queue *log_get_global_queue(void);

int bsp_log_create(log_queue *p_log_queue, unsigned char min_level, unsigned long len_msg);

void bsp_log_destroy(log_queue *p_log_queue);


void bsp_log_msg(log_queue *p_log_queue,
		unsigned int uc_log_level,
		char *psz_format, ...);

void bsp_log_msg_va(log_queue  *p_log_queue,
		   unsigned int uc_log_level,
		   const char *psz_format, va_list args);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
