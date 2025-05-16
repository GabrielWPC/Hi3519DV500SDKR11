/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDRTRN_BOOT_ADAPT_H
#define DDRTRN_BOOT_ADAPT_H

#include "types.h"
#include "ddrtrn_boot_common.h"
#include "ddrtrn_log.h"

/*
 * DDR_BOOT_TYPE_AUX_CODE : aux_code,
 * DDR_BOOT_TYPE_HSL : hsl,
 * DDR_BOOT_TYPE_UBOOT : uboot
*/
#define BOOTLOADER DDR_BOOT_TYPE_HSL

/* register operations */
static inline unsigned int ddrtrn_reg_read(unsigned long addr)
{
	return (*(volatile unsigned int *)((uintptr_t)addr));
}
static inline void ddrtrn_reg_write(unsigned int val, unsigned long addr)
{
	(*(volatile unsigned int *)((uintptr_t)addr)) = val;
}

#else /* DDRTRN_BOOT_ADAPT_H */
#if BOOTLOADER != DDR_BOOT_TYPE_HSL
#error "do not input more than one ddrtrn_boot_adapt.h"
#endif /* BOOTLOADER */
#endif /* DDRTRN_BOOT_ADAPT_H */
