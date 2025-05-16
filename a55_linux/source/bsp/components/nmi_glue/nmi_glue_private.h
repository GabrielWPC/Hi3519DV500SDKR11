/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef FIQ_GLUE_PRIVATE_H
#define FIQ_GLUE_PRIVATE_H

#ifdef CONFIG_ARM64
#include <asm/smp_plat.h>
#endif
#include <linux/list.h>
#include <linux/arm-smccc.h>

#ifdef CONFIG_ARM64
#define SMC_FIQ_GLUE_ID 0x83000004
#define SMC_FIQ_GLUE_RETURN_CODE 0x83000005
#define SMC_FIQ_RETURN_GLUE_ID 0x83000006

struct fiq_pt_regs;

/* buffer for register from EL3 and DFX */
#define NMI_CTX_SP_SIZE (512)
#define NMI_CTX_BUFFER_DFX_SIZE (4 * 1024)
#endif

struct nmi_glue_handler {
#ifdef CONFIG_ARM64
	void (*fiq)(struct nmi_glue_handler *handler);
#endif
#ifdef CONFIG_ARM
	void (*fiq)(struct nmi_glue_handler *h, void *regs, void *svc_sp);
#endif
	void (*resume)(struct nmi_glue_handler *h);
	void *private;
};

struct fiq_func_params {
	struct nmi_glue_handler *h;
	void *regs;
	void *svc_sp;
#ifdef CONFIG_ARM64
	uint64_t fiq_num;
#endif
};
#ifdef CONFIG_ARM64
struct fiq_pt_regs {
	union {
		struct user_pt_regs user_regs;
		struct {
			uint64_t regs[31];
			uint64_t sp;
			uint64_t pc;
			uint64_t pstate;
		};
	};
	uint64_t orig_x0;
	uint64_t syscallno;
};

struct fiq_secure_buf {
	struct fiq_pt_regs regs;
	uint64_t fiq_num;
	uint64_t crc32_check_sum;
	char private[200];
};
#endif

#ifdef CONFIG_ARM64
uint64_t *nmi_glue_reg(uint64_t *sp_addr);
void fiq_el1_trigger(void);
uint32_t restore_context(void);
int nmi_glue_register_handler(void);
extern uint64_t *nmi_virt_dfx_base;
#endif

#endif /* FIQ_GLUE_PRIVATE_H */
