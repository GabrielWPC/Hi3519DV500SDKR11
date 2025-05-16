/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/securec.h>
#include <linux/slab.h>
#include "nmi_glue_private.h"

struct fiq_param64 {
	uint64_t a0;
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
	uint64_t a4;
	uint64_t a5;
	uint64_t a6;
};

uint64_t *nmi_virt_dfx_base;

static int smc_call(uint64_t a0, uint64_t a1, uint64_t a2)
{
	asm volatile("mov x0, %0"::"r"(a0));
	asm volatile("mov x1, %0"::"r"(a1));
	asm volatile("mov x2, %0"::"r"(a2));
	asm volatile("smc #0");
	return 0;
}

uint32_t restore_context(void)
{
	struct fiq_param64 param = {0};

	param.a0 = SMC_FIQ_GLUE_RETURN_CODE;
	smc_call(param.a0, param.a1, param.a2);
	return 0;
}
