/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __TD_TYPE_H__
#define __TD_TYPE_H__

#ifdef __KERNEL__
#include <linux/types.h>
#elif (defined(__LITEOS__) || defined(TD_SUPPORT_STDLIB) || defined(CONFIG_CGRA_CORE))
#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif
#endif

/* defines */
#define TD_NULL             NULL
#define TD_NULL_PTR         TD_NULL
#define TD_FALSE 0
#define TD_TRUE  1

typedef unsigned char           td_uchar;
typedef unsigned char           td_u8;
typedef unsigned short          td_u16;
typedef unsigned int            td_u32;
typedef unsigned long long      td_u64;
typedef unsigned long           td_ulong;
typedef char                    td_char;
typedef signed char             td_s8;
typedef short                   td_s16;
typedef int                     td_s32;
typedef long long               td_s64;
typedef long                    td_slong;
typedef float                   td_float;
typedef double                  td_double;
typedef void                    td_void;
typedef td_u8                   td_bool;
typedef td_u64                  td_handle;
typedef td_u8                   td_byte;
typedef td_byte                *td_pbyte;
typedef td_void                *td_pvoid;
typedef volatile td_u32         td_u32_reg;
typedef td_ulong           	td_size_t;
typedef signed long           	td_ssize_t;
typedef td_handle      		td_mem_size_t;
typedef td_s64               	td_mem_handle_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TD_TYPES_H__ */

