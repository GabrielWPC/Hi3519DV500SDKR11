/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <asm/stacktrace.h>
#include <asm/system_misc.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "nmi_glue_private.h"

MODULE_LICENSE("GPL");

#ifdef CONFIG_THREAD_INFO_IN_TASK
DECLARE_PER_CPU(struct task_struct *, __entry_task);
#endif

static DEFINE_PER_CPU(struct pt_regs, per_fiq_args);
static DEFINE_MUTEX(fiq_action_lock);

#define NMI_GLUE_REG_BUF_SIZE 200

#define REG_NUMS 31
#define FP_REG 29

static void dump_backtrace_entry(unsigned long where, const char *loglvl)
{
	printk("%s %pS\n", loglvl, (void *)where);
}

static void dump_backtrace_fiq(
	struct pt_regs *regs, struct task_struct *tsk, unsigned long fp, unsigned long pc, const char *loglvl)
{
	struct stackframe frame;
	int skip = 0;

	printk("%s(regs = %p tsk = %p)\n", __func__, regs, tsk);

	if (regs) {
		if (user_mode(regs))
			return;
		skip = 1;
	}

	if (!tsk)
		tsk = current;

	if (!try_get_task_stack(tsk))
		return;

	start_backtrace(&frame, fp, pc);

	printk("%sCall trace:\n", loglvl);
	do {
		if (!skip) {
			dump_backtrace_entry(frame.pc, loglvl);
		} else if (frame.fp == regs->regs[FP_REG]) {
			skip = 0;

			dump_backtrace_entry(regs->pc, loglvl);
		}
	} while (!unwind_frame(tsk, &frame));

	put_task_stack(tsk);
}

static void fiq_save_regs(struct pt_regs *args, void *info, uint64_t cpu)
{
	struct fiq_func_params *params = info;
	struct fiq_pt_regs *regs = params->regs;
	
	if (params->regs) {
		int i = 0;
		for (i = 0; i < REG_NUMS; i++) {
			args->regs[i] = regs->regs[i];
		}
		args->sp = regs->sp;
		args->pc = regs->pc;
		args->pstate = regs->pstate;
		args->orig_x0 = regs->orig_x0;
		args->syscallno = regs->syscallno;
	}

	mutex_lock(&fiq_action_lock);
	__show_regs(args);
	dump_backtrace_fiq(args, NULL, regs->regs[FP_REG], regs->pc, KERN_WARNING);
	mutex_unlock(&fiq_action_lock);
	while (1)
		;
	return;
}

static inline int nmi_glue_action_pack(void *info)
{
	struct pt_regs *args;
	uint64_t cpu;

	cpu = smp_processor_id();

	args = &per_cpu(per_fiq_args, cpu);
	fiq_save_regs(args, info, cpu);

	flush_cache_all();

	return 0;
}

static void nmi_glue_action(struct nmi_glue_handler *handler)
{
	struct fiq_func_params params;
	volatile int i;
	struct arm_smccc_res res;
	void *info = &params;
	void *buf = kmalloc(NMI_CTX_SP_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_ERR "[fiq] buf is NULL.\n");
		return;
	}
	uint64_t *addr = buf;

	for (i = 0; i < 9; i++) { // 9 : need smc nums
		arm_smccc_smc(SMC_FIQ_RETURN_GLUE_ID, i, 0, 0, 0,
			      0, 0, 0, &res);
		(*(addr++)) = res.a0;
		(*(addr++)) = res.a1;
		(*(addr++)) = res.a2;
		(*(addr++)) = res.a3;
	}
	struct fiq_secure_buf *fiq_secure_buf;
	fiq_secure_buf = buf;

	params.h = handler;

	params.regs = &fiq_secure_buf->regs;
	params.svc_sp = NULL;
	params.fiq_num = fiq_secure_buf->fiq_num;

	nmi_glue_action_pack(info);
}

struct nmi_glue_handler nmi_handler = {
	.fiq = nmi_glue_action,
};

static int nmi_glue_handler_set(void)
{
	struct nmi_glue_handler *handler = &nmi_handler;

	handler->resume = NULL;

	return 0;
}

static void update_current(void)
{
#ifdef CONFIG_THREAD_INFO_IN_TASK
	asm volatile("msr sp_el0, %0" : : "r"(raw_cpu_read(__entry_task)));
#endif
}

void fiq_el1_trigger(void)
{
	struct nmi_glue_handler *handler = &nmi_handler;
	update_current();

	handler->fiq(handler);
	restore_context();
}

static int __init nmi_glue_init(void)
{
	nmi_glue_handler_set();
	mutex_init(&fiq_action_lock);

	struct arm_smccc_res res;

	printk("FIQ register EL3!\n");
	arm_smccc_smc(SMC_FIQ_GLUE_ID, (uint64_t)fiq_el1_trigger, 0, 0, 0,
			      0, 0, 0, &res);
	return 0;
}
arch_initcall(nmi_glue_init);
