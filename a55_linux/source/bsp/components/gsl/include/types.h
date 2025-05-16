/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __TYPES_H__
#define __TYPES_H__
#include "soc_errno.h"
#include "td_type.h"

typedef td_ulong 		uintptr_t;
typedef td_s8 			s8;
typedef td_u8 			u8;
typedef td_s16 			s16;
typedef td_u16 			u16;
typedef td_s32             	s32;
typedef td_u32 			u32;
typedef td_u64 			u64;
typedef td_ulong 		size_t;
typedef s32 			errno_t;
typedef errno_t 		bool;

#define BITS_PER_LONG 		32

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#define TRUE    		1
#define FALSE   		0

#define ERROR   		-1
#define OK      		0
#define DATA_EMPTY		-2

#define AUTH_SUCCESS		(0x3CA5965A)
#define AUTH_FAILURE		(0xC35A69A5)

#define PRIVATE 		static

#define BYTE_NUMS 		4
#define ZERO 			0
#define TEE_VERSION_BOUND 	64

#endif

