/*
 *  armboot - Startup Code for ARM Cortex CPU-core
 */
#include <arch/aarch64/asm_macros.S>
#include <platform.h>
#include <system.h>

.globl  runtime_exceptions
.globl  sync_exception_sp_el0
.globl  irq_sp_el0
.globl  fiq_sp_el0
.globl  serror_sp_el0

.globl  sync_exception_sp_elx
.globl  irq_sp_elx
.globl  fiq_sp_elx
.globl  serror_sp_elx

.globl  sync_exception_aarch64

.globl _start
_start:
	b reset
.align 3 /* data must be 8 bytes aligned*/

reset:
/* Initialize the Registers to avoid X status */
	ldr x0, =0
	ldr x1, =0
	ldr x2, =0
	ldr x3, =0
	ldr x4, =0
	ldr x5, =0
	ldr x6, =0
	ldr x7, =0
	ldr x8, =0
	ldr x9, =0
	ldr x10, =0
	ldr x11, =0
	ldr x12, =0
	ldr x13, =0
	ldr x14, =0
	ldr x15, =0
	ldr x16, =0
	ldr x17, =0
	ldr x18, =0
	ldr x19, =0
	ldr x20, =0
	ldr x21, =0
	ldr x22, =0
	ldr x23, =0
	ldr x24, =0
	ldr x25, =0
	ldr x26, =0
	ldr x27, =0
	ldr x28, =0
	ldr x29, =0
	ldr x30, =0

	/*
	 * Could be EL3/EL2/EL1, Initial State:
	 * Little Endian, MMU Disabled, i/dCache Disabled
	 */
	adr x0, runtime_exceptions

	msr vbar_el3, x0
	mrs x0, scr_el3
	orr x0, x0, #0xf            /* SCR_EL3.NS|IRQ|FIQ|EA */
	msr scr_el3, x0
	msr cptr_el3, xzr            /* Enable FP/SIMD */

	bl get_current_core
	cbnz x0, minor_core_boot

	/* clear_stack*/
	ldr x0, =(SRAM_BASE)
	mov x1, #0
	ldr x2, =(STACK_ADDR)
	subs x2, x2, x0
	bl clear_data

	/* set stack for C code  */
	ldr x0, =(STACK_ADDR)
	bic sp, x0, #0xf    /* 16-byte alignment for ABI compliance */

	/* clear_bss */
	ldr x0, =__bss_start
	mov x1, #0
	ldr x2, =__bss_end
	subs x2, x2, x0
	/* void * clear_data(void * s,int c,size_t count) */
	bl clear_data
	bl stack_chk_guard_setup
	/********init sys watchdog******/
	bl sys_watchdog_enable
	b main_entry

minor_core_boot:
	ldr x1, _jump_addr
	ldr x23, [x1]
	mov x0, x23

	b switch_from_el3_to_el1


.align 3 /* data must be 8 bytes aligned, the data will be copy to bootram */
non_tee_copy_begin:
	/* clear GSL Code */
	mov x0, x21
	mov x1, x22
	mov w2, #0x0
	mov x3, #0x4
do_code_clear_loop:
	str w2, [x0]
	add x0, x0, x3
	cmp x0, x1
	blt do_code_clear_loop
	b set_scs_finish

.align 3
tee_copy_begin:
set_scs_finish:
	/* get scs_finish register */
	ldr	x3, __REG_BASE_CA_MISC
	ldr	w1, [x3, #SCS_CTRL]

	/* set scs_finish register */
	and     w1, w1, #0xfffffff0
	mov     w2, #0x5
	orr     w1, w1, w2
	str     w1, [x3, #SCS_CTRL]

excute_bootloader:
	mov lr, x23
	ret

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

.align 3 /* data must be 8 bytes aligned, the data will be copy to bootram */
__REG_BASE_CA_MISC:
        .quad REG_BASE_CA_MISC

non_tee_copy_end:
tee_copy_end:

.align 3
_jump_addr:
	.quad jump_addr

/*
 * void copy_code_to_sram(x0:src, x1:dst, x2:size);
 */
copy_code_to_sram:
	add x2, x0, x2
copy_code_to_sram_loop:
	ldp x10, x11, [x0], #16 /* copy from source address [x0] */
	stp x10, x11, [x1], #16 /* copy to   target address [x1] */
	cmp x0, x2
	ble copy_code_to_sram_loop
	ret

_do_error:
	/* reset */
	ldr x1, =REG_SYSCTRL_BASE
	ldr w0, =1
	str w0, [x1, #REG_SC_SYSRES]
/*
 * void switch_from_el3_to_el1(x0);
 * x0: EL3 return address
 */
switch_from_el3_to_el1:
	/* SCTLR_EL1 with an unknown reset value must be configured */
	ldr x1, =(SCTLR_EL1_UCI_DIS | SCTLR_EL1_EE_LE | SCTLR_EL1_WXN_DIS |\
			 SCTLR_EL1_NTWE_DIS | SCTLR_EL1_NTWI_DIS | SCTLR_EL1_UCT_DIS |\
			 SCTLR_EL1_DZE_DIS | SCTLR_EL1_ICACHE_DIS | SCTLR_EL1_UMA_DIS |\
			 SCTLR_EL1_SED_EN | SCTLR_EL1_ITD_EN | SCTLR_EL1_CP15BEN_DIS |\
			 SCTLR_EL1_SA0_DIS | SCTLR_EL1_SA_DIS | SCTLR_EL1_DCACHE_DIS |\
			 SCTLR_EL1_ALIGN_DIS | SCTLR_EL1_MMU_DIS | SCTLR_EL1_RES1)
	msr sctlr_el1, x1

	/* Initialize HCR_EL2 */
	ldr x1, =HCR_EL2_RW_AARCH64
	msr hcr_el2, x1

	/* AArch64 64bit | SMD ENABLE | Non-secure */
	ldr x1, =(SCR_EL3_RW_AARCH64 & (~SCR_EL3_SMD_DIS) | SCR_EL3_NS_EN)
	msr scr_el3, x1

	/* Return to the EL1_SP1 mode from EL3 */
	ldr x1, =(SPSR_EL_DEBUG_MASK | SPSR_EL_SERR_MASK |\
			  SPSR_EL_IRQ_MASK | SPSR_EL_FIQ_MASK |\
			  SPSR_EL_M_AARCH64 | SPSR_EL_M_EL1H)
	msr spsr_el3, x1

	/* set exception return address */
	msr elr_el3, x0
	eret

get_current_core:
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_AFFLVL_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #MPIDR_AFFINITY_BITS
	ret
/***********************************************************************/
/* Jumping from U-Boot to GSL Through the smc Service  */
/***********************************************************************/
vector_base runtime_exceptions
/* ---------------------------------------------------------------------
 * Current EL with SP_EL0 : 0x0 - 0x200
 * ---------------------------------------------------------------------
 */
vector_entry sync_exception_sp_el0
end_vector_entry sync_exception_sp_el0
vector_entry irq_sp_el0
end_vector_entry irq_sp_el0
vector_entry fiq_sp_el0
end_vector_entry fiq_sp_el0
vector_entry serror_sp_el0
end_vector_entry serror_sp_el0

/* ---------------------------------------------------------------------
 * Current EL with SP_ELx: 0x200 - 0x400
 * ---------------------------------------------------------------------
 */
vector_entry sync_exception_sp_elx
end_vector_entry sync_exception_sp_elx
vector_entry irq_sp_elx
end_vector_entry irq_sp_elx
vector_entry fiq_sp_elx
end_vector_entry fiq_sp_elx
vector_entry serror_sp_elx
end_vector_entry serror_sp_elx

/* ---------------------------------------------------------------------
 * Lower EL using AArch64 : 0x400 - 0x600
 * ---------------------------------------------------------------------
 */
vector_entry sync_exception_aarch64
/*
 * This exception vector will be the entry point for SMCs and traps
 * that are unhandled at lower ELs most commonly. SP_EL3 should point
 * to a valid cpu context where the general purpose and system register
 * state can be saved.
 */
	bl el1_to_el3_entry
end_vector_entry sync_exception_aarch64


el1_to_el3_entry:
	ldr x0, =SECONDARY_CORE_STACK
	bic sp, x0, #0xf    /* 16-byte alignment for ABI compliance */
	ldr x19, =SRAM_BASE
	bl get_ddr_param_data_end_addr
	sub x20, x0, x19

	ldr x0, =power_down_sequence_start
	ldr x1, =power_down_sequence_end
	sub x2, x1, x0  /* code size */
	ldr x1, =SECONDARY_CORE_CODE_ADDR
	bl copy_code_to_sram
	bl sram_to_npu_info
	ldr lr, =SECONDARY_CORE_CODE_ADDR
	ret

.align 3
power_down_sequence_start:
	mov x0, x19
	mov x1, #0
	mov x2, x20
	bl clear_data

	/* set sram to npu */
	mov     x1,#0x5024
	movk    x1,#0x1102,lsl #0x10
	ldr     w0,[x1]
	orr     w0,w0,#0x1
	str     w0,[x1]

	/* set pchn1_enable for core_1 */
	ldr	x3, _MISC_REG_CPU_CTRL6
	ldr	w1, [x3]
	orr     w1, w1, #CPU_HW_STATE_MACHINE
	str     w1, [x3]
	/* cpu power down */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	wfi
	ret

clear_data:
	add x2, x0, x2
	mov x3, #0x8
clear_data_loop:
	str x1, [x0]
	add x0, x0, x3
	cmp x0, x2
	blt clear_data_loop
	ret

.align 3 /* data must be 8 bytes aligned, the data will be copied to bootram */
_MISC_REG_CPU_CTRL6:
	.quad MISC_REG_CPU_CTRL6
power_down_sequence_end:

