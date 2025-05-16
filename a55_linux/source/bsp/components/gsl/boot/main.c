/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "common.h"
#include "lib.h"
#include "platform.h"
#include "flash_map.h"
#include "checkup.h"
#include "board_type.h"
#include "lpds.h"
#include "backup.h"
#include "cipher.h"
#include "securecutil.h"
#include "share_drivers.h"
#include "err_print.h"
#include "../drivers/otp/otp.h"
#include "../drivers/ddr_init/init_regs.h"
#include "../drivers/ddr_init/include/ddrtrn_api.h"
#include "../drivers/uart/uart.h"
#include "../drivers/ipc/ipc.h"
#include "../drivers/tzasc/tzasc.h"
#include "../drivers/otp/otp.h"
#include "../share_drivers/include/soc_errno.h"
#include "../gpll/gpll.h"
#include "../drivers/svb/svb.h"

#define EMMC_READ_BLOCK_MAX 10
#define SHIFT_OFF   32
#define REG_LOW_16BIT  0x0000ffff

unsigned long jump_addr;
static backup_img_params_s g_backup_params;
unsigned long __stack_chk_guard;

#ifndef LLVM_COMPILER
#pragma GCC push_options
#pragma GCC optimize ("-fno-stack-protector")
void __stack_chk_fail(void)
{
	err_print(STACK_CHK_FAIL);
	call_reset();
}

#ifdef __LP64__
void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
	__stack_chk_guard <<= SHIFT_OFF;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard |= random;
}
#endif

#ifdef __LP32__
void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
}
#endif

#pragma GCC pop_options
#else
__attribute__((no_stack_protector))
void __stack_chk_fail(void)
{
	err_print(STACK_CHK_FAIL);
	call_reset();
}
__attribute__((no_stack_protector))
void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
	__stack_chk_guard <<= SHIFT_OFF;
}
#endif

static u8 get_self_boot_flag(void)
{
	return (u8)reg_getbits(SELF_BOOT_FLAG_REG_ADDR, SELF_BOOT_DEV_REG_BIT_OFFSET, SELF_BOOT_DEV_REG_BIT_WIDTH);
}

void err_print(u8 err_idx)
{
	if (err_idx >= MAX_ERR_BITS)
		return;
	/* print to uart */
	s8 err_str[MAX_STR_LEN + 1] = {(err_idx / DIV_100) % DIV_10 + '0', \
								   (err_idx / DIV_10) % DIV_10 + '0', \
								   (err_idx % DIV_10 + '0'), '\0'
								  };
	u8 i = 0;
	for (i = 0; i < MAX_STR_LEN; i++) {
		if (err_str[i] != '0')
			break;
	}
	serial_putc(REG_UART0_BASE, '\n');
	if (i == MAX_STR_LEN)
		serial_putc(REG_UART0_BASE, '0');
	else
		log_serial_puts(&err_str[i]);
	mdelay(ERR_PRINT_DELAY_MS);
}

static void tee_img_failure_process()
{
	u32 val;
	val = reg_get(TEE_FAILURE_SOURCE);
	val = set_tee_failure_source_type(val, VERIFY_TEE_OS_FAILED);
	reg_set(TEE_FAILURE_SOURCE, val);
	call_reset();
}

static void failure_process(failure_source_type type)
{
	u32 val;
	if (get_low_power_mode() == SUSPEND_TO_LPDS) {
		clear_lpds();
		goto clear_verify_backup_img_reg;
	}
	if (get_self_boot_flag() == SELF_BOOT_FLASH) {
		if (opt_get_boot_backup_enable() == BACKUP_ENABLE) {
			if (get_verify_backup_img_reg() == BOOT_FROM_PRIME) {
				/* Set verify_backup_img flag to BOOT_FROM_BACKUP */
				set_verify_backup_img_reg(BOOT_FROM_BACKUP);
				goto clean_boot_img_addr_size_reg;
			}
		}
	}
	/* Set failure_source */
	log_serial_puts((const s8 *)"\n\rG:Boot failed");
	val = reg_get(TEE_FAILURE_SOURCE);
	val = set_tee_failure_source_type(val, type);
	reg_set(TEE_FAILURE_SOURCE, val);
clear_verify_backup_img_reg:
	set_verify_backup_img_reg(BOOT_FROM_PRIME);
clean_boot_img_addr_size_reg:
	clean_boot_img_addr_size();
	log_serial_puts((const s8 *)"\n\rG:soft rst");
	if (is_sec_dbg_enable() == AUTH_SUCCESS)
		__asm__ __volatile__("b .");
	call_reset();
}

static ALWAYS_INLINE void fun_redun_check(int ret, int err, failure_source_type type)
{
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		if (ret != EXT_SEC_SUCCESS) {
			err_print(err);
			failure_process(type);
		}
	}
}

static void sec_module_init(void)
{
	ext_drv_func func_list;
	func_list.malloc = malloc;
	func_list.free = free;
	func_list.serial_putc = (func_serial_putc)serial_putc;
	func_list.delay_ticks = NULL;
	func_list.cache_flush = NULL;
	func_list.cache_inv = NULL;
	uapi_drv_register_func(&func_list);
	uapi_drv_cipher_init();
}

static void start_flow_prepare(void)
{
	malloc_init(get_gsl_heap_addr(), MALLOC_SIZE);
	timer_init();
	sec_module_init();
	return;
}

static void configure_ddr_parameters(void)
{
	u32 tables_base = get_ddr_param_data_addr();
	ddr_table_info *boot_table_head_addr = (ddr_table_info *)(uintptr_t)tables_base;
	if (boot_table_head_addr->magic == BOOT_TABLE_MAGIC) {
		log_serial_puts((const s8 *)"\nboot table version     :");
		log_serial_puts((const s8 *)(boot_table_head_addr->ddr_table_version));
		log_serial_puts((const s8 *)"\nboot table build time  :");
		log_serial_puts((const s8 *)boot_table_head_addr->ddr_table_gen_time);
		log_serial_puts((const s8 *)"\nboot table file name   :");
		log_serial_puts((const s8 *)boot_table_head_addr->ddr_table_name);
		tables_base += HEADINFO_LENTH;
	}
	init_registers(tables_base, 0);
}

#ifdef DDR_SCRAMB_ENABLE
static u32 get_random_num(void)
{
	u32 reg_val;
	u32 data_cnt;

	do {
		reg_val = reg_get(TRNG_DATA_ST_REG);
		data_cnt = reg_val & TRNG_FIFO_DATA_CNT_MASK;
	} while (data_cnt == 0);

	reg_val = reg_get(TRNG_DSTA_FIFO_DATA_REG);
	return reg_val;
}

static void ddr_scramb_update(void)
{
	reg_set(DDRCA_REE_UPDATE_REG, 0x0);
	reg_set(DDRCA_REE_UPDATE_REG, 0x1);
}

static void ddr_scramb_start(void)
{
	reg_set(DDRCA_REE_RANDOM_L_REG, get_random_num());
	reg_set(DDRCA_REE_RANDOM_H_REG, get_random_num());
	reg_set(DDRCA_EN_REG, SCRAMB_BYPASS_DIS);
	ddr_scramb_update();
	reg_set(DDRCA_LOCK_REG, SCRAMB_LOCK);

	/* clear the old random value */
	get_random_num();
}

static void ddr_scramb_turn_off(void)
{
	reg_set(DDRCA_EN_REG, SCRAMB_BYPASS);
	ddr_scramb_update();
}

static void ddr_scrambling(void)
{
	u32 status;
	u32 dmc0_isvalid = 0;
	u32 dmc1_isvalid = 0;

	if (get_ddr_scrub_and_gpll_switch() != OPEN)
		return;
	/*
	 * read ddrc_cfg_ddrmode register,
	 * if bit[3:0] is not 0x0 ,
	 * the channel is valid.
	 */
	dmc0_isvalid = (reg_get(DMC0_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;
	dmc1_isvalid = (reg_get(DMC1_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;

	/* set ddrc to enter self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);
	if (dmc1_isvalid)
		reg_set(DMC1_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);

	/* wait the status of ddrc to be self-refresh (status == 1). */
	do {
		status = 1;
		status &= dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 1;
		status &= dmc1_isvalid ? (reg_get(DMC1_DDRC_CURR_FUNC_REG) & 0x1) : 1;
	} while (status != 1);

	/* start ddr scrambling */
	ddr_scramb_start();

	/* set ddrc to exit self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);
	if (dmc1_isvalid)
		reg_set(DMC1_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);

	/* wait the status of ddrc to be normal (status == 0). */
	do {
		status = 0;
		status |= dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 0;
		status |= dmc1_isvalid ? (reg_get(DMC1_DDRC_CURR_FUNC_REG) & 0x1) : 0;
	} while (status != 0);

	return;
}
#endif

static void ddr_training(void)
{
	/* ddr hw/sw training */
	bsp_ddrtrn_training_item(0xffffffff, 0xffffffff, 0x1);
	/* disable it when sample back */
	bsp_ddrtrn_retrain_enable();
	/* ddr DMC auto power down config */
	bsp_ddrtrn_dmc_auto_power_down_cfg();
}

static void ddr_capat_adapt(void)
{
	if ((reg_get(SYSBOOT14) & REG_LOW_16BIT) >= BOOT_REG_VERSION) {
		unsigned int ddr_size;
		ddr_size = bsp_ddrtrn_capat_adapt();
		if (ddr_size == 0) {
			log_serial_puts((const s8 *)"\nGet DDR size fail!");
		} else {
			log_serial_puts((const s8 *)"\nDDR size: 0x");
			serial_put_hex(ddr_size);
			log_serial_puts((const s8 *)"MB");
		}
	}
}

static void system_init(u32 channel_type)
{
	/* config gpll */
	bsp_gpll_config(channel_type);
	/* config ddr table params */
	configure_ddr_parameters();
	/* turn off ddr scramb */
#ifdef DDR_SCRAMB_ENABLE
	ddr_scramb_turn_off();
#endif
	/* SVB */
	start_svb();
	/* ddr tranning */
	ddr_training();
	/* ddr capacity adapt */
	ddr_capat_adapt();
	/* ddr scrambling */
#ifdef DDR_SCRAMB_ENABLE
	ddr_scrambling();
#endif
	end_svb();
}

static u32 get_data_channel_type(void)
{
	u32 channel_type;
	channel_type = reg_get(REG_SYSCTRL_BASE + DATA_CHANNEL_TYPE_REG);
	switch (channel_type) {
	case BOOT_SEL_SDIO:
	case BOOT_SEL_USB:
	case BOOT_SEL_UART:
	case BOOT_SEL_FLASH:
	case BOOT_SEL_EMMC:
		break;
	default:
		err_print(GET_DATA_CHANNEL_TYPE_UNKNOW);
		channel_type = BOOT_SEL_UNKNOW;
		break;
	}
	return channel_type;
}

static void get_img_backup_params(backup_img_params_s *backup_params)
{
	u32 boot_zone;
	memset_s(backup_params, sizeof(backup_img_params_s), 0, sizeof(backup_img_params_s));
	boot_zone = get_verify_backup_img_reg();
	if ((get_self_boot_flag() == SELF_BOOT_FLASH) &&
			(opt_get_boot_backup_enable() == BACKUP_ENABLE) &&
			(boot_zone != BOOT_FROM_PRIME)) {
		if (reg_get(BOOT_IMG_ADDR_REG_ADDR) != 0) {
			backup_params->offset_addr = reg_get(BOOT_IMG_ADDR_REG_ADDR);
			backup_params->enable = 1;
		}
	}
}

static void start_secondary_core(u32 core_id, u32 entrypoint)
{
	u32 tmp;
	if (core_id == 0) {
		log_serial_puts((const s8 *)"invalid core_id\n");
		call_reset();
	}
	if (entrypoint == 0) {
		log_serial_puts((const s8 *)"invalid entrypoint\n");
		call_reset();
	}
	reg_set(REG_PERI_CPU_RVBARADDR, entrypoint >> ENTRY_POINT_OFFSET); /* psci_entrypoint */
	if (core_id == CORE_1) {
		tmp = reg_get(REG_PERI_CRG2067_CORE1);
		tmp |= 0x1 << REG_PERI_OFFSET;    /* enable cpu cken */
		tmp &= ~(0x3 << 0); /* clear cpu po_srst_req and  srst_req */
		reg_set(REG_PERI_CRG2067_CORE1, tmp);
	} else if (core_id == CORE_2) {
		tmp = reg_get(REG_PERI_CRG2068_CORE2);
		tmp |= 0x1 << REG_PERI_OFFSET;    /* enable cpu cken */
		tmp &= ~(0x3 << 0); /* clear cpu po_srst_req and  srst_req */
		reg_set(REG_PERI_CRG2068_CORE2, tmp);
	} else if (core_id == CORE_3) {
		tmp = reg_get(REG_PERI_CRG2069_CORE3);
		tmp |= 0x1 << REG_PERI_OFFSET;    /* enable cpu cken */
		tmp &= ~(0x3 << 0); /* clear cpu po_srst_req and  srst_req */
		reg_set(REG_PERI_CRG2069_CORE3, tmp);
	} else {
		log_serial_puts((const s8 *)"invalid cpu\n");
		call_reset();
	}
}

#ifdef CFG_DEBUG_INFO
static u32 print_current_el(u32 core)
{
	size_t current_el;
	asm volatile("mrs %0, CurrentEL\n\t" : "=r"(current_el) : : "memory");
	size_t el = (current_el >> CURRENT_EL_SHIFT) & CURRENT_EL_MASK;
	log_serial_puts((const s8 *)"\ncore");
	serial_put_dec(core);
	log_serial_puts((const s8 *)" running:");
	serial_put_dec(el);
	log_serial_puts((const s8 *)"\n");
	return el;
}
#endif

void sram_to_npu_info(void)
{
	log_serial_puts((const s8 *)"\nsram to npu!\n");
}

static int config_ddr_security(size_t sec_rgn_size)
{
	int ret;
	u32 rgn_attr;
	const size_t ddr_region_size = DDR_END - DDR_BASE;
	rgn_attr = TZASC_ATTR_SEC_R | TZASC_ATTR_SEC_W | TZASC_ATTR_NOSEC_R |
			   TZASC_ATTR_NOSEC_W | TZASC_ATTR_SEC_INV;
	config_tzasc(TZASC_RGN_1, rgn_attr, DDR_BASE, ddr_region_size);
	rgn_attr = TZASC_ATTR_SEC_R | TZASC_ATTR_SEC_W  |
			   TZASC_ATTR_SEC_INV;
	config_tzasc(TZASC_RGN_2, rgn_attr, SEC_DDR_BASE, sec_rgn_size);
	ret = tzasc_sec_config_read_back(TZASC_RGN_2, SEC_DDR_BASE, sec_rgn_size);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = tzasc_bypass_disable();
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int load_tee_img(uintptr_t addr)
{
	int ret;
	tee_code_info *tee_info = NULL;
	size_t sec_rgn_size = SECONDARY_CORE_CODE_SIZE + BL31_SIZE + BL32_SIZE + TEE_OS_KEY_SIZE;
	ret = config_ddr_security(sec_rgn_size);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(DDR_CONFIG_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = dma_copy(TEE_OS_KEY_ADDR, TEE_KEY_SIZE + TEE_CODE_INFO_SIZE, addr);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = handle_tee_key((uintptr_t)TEE_OS_KEY_ADDR);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_KEY_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = handle_tee_info((uintptr_t)TEE_OS_KEY_ADDR, (uintptr_t)(TEE_OS_KEY_ADDR + TEE_KEY_SIZE));
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_INFO_ERR);
		return EXT_SEC_FAILURE;
	}
	tee_info = (tee_code_info *)(uintptr_t)(TEE_OS_KEY_ADDR + TEE_KEY_SIZE);
	ret = handle_atf_code(addr, (uintptr_t)(tee_info), (uintptr_t)BL31_BASE);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_ATF_CODE_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = handle_tee_code(addr, (uintptr_t)(tee_info), (uintptr_t)BL32_LOAD_ADDR);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_CODE_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

static void start_os(uintptr_t bl31_addr, uintptr_t bl32_addr)
{
	void (*atf_entry)(uintptr_t bl32_ep);
	atf_entry = (void(*)(uintptr_t bl32_ep))bl31_addr;
	atf_entry(bl32_addr);
}

static void ipc_send_ack(int node)
{
	struct ipc_share_msg msg;
	msg.cmd = IPC_CMD_ACK;
	msg.len = 0;
	ipc_send_msg(node, &msg);
}

static void ipc_send_nack(int node, u32 cmd)
{
	struct ipc_share_msg msg;
	msg.cmd = cmd;
	msg.len = 0;
	ipc_send_msg(node, &msg);
}

static uintptr_t recv_fw_addr(void)
{
	struct ipc_share_msg msg;
	uintptr_t fw_addr;
	while (1) {
		int ret;
		ret = ipc_recv_msg(IPC_NODE_CORE0, &msg, 1);
		if (ret <= 0) {
			log_serial_puts((const s8 *)"core0 got msg error\n");
			continue;
		}
		if (is_soc_tee_enable() == AUTH_FAILURE) {
			if (msg.cmd == IPC_CMD_START_BL31)
				break;
			ipc_send_nack(IPC_NODE_CORE1, IPC_CMD_NEED_BL31);
			continue;
		} else {
			if (msg.cmd == IPC_CMD_START_TEEIMG)
				break;
			ipc_send_nack(IPC_NODE_CORE1, IPC_CMD_NEED_TEEIMG);
			continue;
		}
		log_serial_puts((const s8 *)"core0 got invalid msg\n");
	}
	fw_addr = ((size_t)msg.buf[0] << FW_ADDR_OFFSET) | msg.buf[1];
	ipc_send_ack(IPC_NODE_CORE1);
	return fw_addr;
}

static void load_and_start_os(void)
{
	int ret;
	uintptr_t chk_sum_sbrk1 = ~EXT_DRV_KLAD_KEY_TYPE_SBRK1;
	uintptr_t fw_addr = recv_fw_addr();
	if (is_soc_tee_enable() == AUTH_FAILURE) {
		if (fw_addr != BL31_BASE) {
			ret = dma_copy(BL31_BASE, BL31_SIZE, (uintptr_t)fw_addr);
			if (ret != EXT_SEC_SUCCESS) {
				log_serial_puts((const s8 *)"copy atf code error\n");
				call_reset();
			}
		}
		log_serial_puts((const s8 *)"core0 start ATF ...\n");
		start_os(BL31_BASE, (uintptr_t)NULL);
	} else {
		ret = load_tee_img(fw_addr);
		if (ret != EXT_SEC_SUCCESS)
			tee_img_failure_process();
		(void)uapi_drv_klad_rootkey_disable(EXT_DRV_KLAD_KEY_TYPE_SBRK1, chk_sum_sbrk1);
		log_serial_puts((const s8 *)"core0 start TEE Image ...\n");
		start_os((uintptr_t)BL31_BASE, (uintptr_t)BL32_BASE);
	}
}

static ALWAYS_INLINE void store_booting_param()
{
	u32 boot_code_addr;
	uboot_code_info *uboot_info = NULL;
	uboot_info = (uboot_code_info *)get_uboot_info_download_ddr_addr();
	boot_code_addr = uboot_info->uboot_entry_point;
	/* The DDR is not initialized in the U-boot */
	reg_set(REG_SYSCTRL_BASE + REG_SC_GEN4, 0);
	jump_addr = boot_code_addr;
}

#ifdef CFG_DEBUG_INFO
static void print_addr_size(u32 addr, u32 size, const s8 *s)
{
	log_serial_puts(s);
	serial_put_hex(addr);
	log_serial_puts((const s8 *)" size:0x");
	serial_put_hex(size);
}

static void dump_addr_info()
{
	print_addr_size(get_gsl_code_info_addr(), GSL_CODE_INFO_SIZE, (const s8 *)"\n\ngsl info addr  :0x");
	print_addr_size(get_gsl_code_addr(), get_gsl_code_area_len(), (const s8 *)"\ngsl code addr  :0x");
	print_addr_size(get_gsl_heap_addr(), get_gsl_heap_len(), (const s8 *)"\ngsl heap addr  :0x");
	print_addr_size(get_ree_key_area_addr(), REE_BOOT_KEY_AREA_SIZE, (const s8 *)"\nree key  addr  :0x");
	print_addr_size(get_ddr_param_info_addr(), PARM_AREA_INFO_SIZE, (const s8 *)"\nparam info addr:0x");
	log_serial_puts((const s8 *)"\nparam max num  :0x");
	serial_put_hex(get_ddr_param_cnt());
	print_addr_size(get_ddr_param_data_addr(), get_ddr_param_len(), (const s8 *)"\nparam data addr:0x");
	print_addr_size(get_uboot_info_download_ddr_addr(), UBOOT_CODE_INFO_SIZE, (const s8 *)"\nuboot info addr:0x");
	print_addr_size(get_uboot_code_ddr_addr(), get_uboot_code_size(), (const s8 *)"\nuboot info addr:0x");
	log_serial_puts((const s8 *)"\nuboot entry    :0x");
	serial_put_hex(get_uboot_entrypoint_ddr_addr());
}

static void print_boot_time(void)
{
	td_u32 l_value;
	if (is_sec_dbg_enable() == AUTH_SUCCESS)
		return;
	log_serial_puts((const s8 *)"\nsyscnt   addr:0x");
	serial_put_hex(SYSCNT_AREA_START_ADDR);
	log_serial_puts((const s8 *)"\nbootrom start:0x");
	l_value = *((td_u32 *)(SYSCNT_AREA_START_ADDR + \
						   SYS_CNT_BOOTROM_START_OFFSET * SYS_CNT_RAM_UNIT + \
						   SYS_CNT_UNIT_LOW_OFFSET));
	serial_put_hex(l_value / SYS_CNT_FREQ_24MHZ);
	log_serial_puts((const s8 *)" us");
	log_serial_puts((const s8 *)"\nbootrom   end:0x");
	l_value = *((td_u32 *)(SYSCNT_AREA_START_ADDR + \
						   SYS_CNT_BOOTROM_END_OFFSET * SYS_CNT_RAM_UNIT + \
						   SYS_CNT_UNIT_LOW_OFFSET));
	serial_put_hex(l_value / SYS_CNT_FREQ_24MHZ);
	log_serial_puts((const s8 *)" us");
	log_serial_puts((const s8 *)"\ngsl     start:0x");
	l_value = *((td_u32 *)(SYSCNT_AREA_START_ADDR + \
						   SYS_CNT_GSL_START_OFFSET * SYS_CNT_RAM_UNIT + \
						   SYS_CNT_UNIT_LOW_OFFSET));
	serial_put_hex(l_value / SYS_CNT_FREQ_24MHZ);
	log_serial_puts((const s8 *)" us");
	log_serial_puts((const s8 *)"\nuboot   start:0x");
	l_value = *((td_u32 *)(SYSCNT_AREA_START_ADDR + \
						   SYS_CNT_UBOOT_START_OFFSET * SYS_CNT_RAM_UNIT + \
						   SYS_CNT_UNIT_LOW_OFFSET));
	serial_put_hex(l_value / SYS_CNT_FREQ_24MHZ);
	log_serial_puts((const s8 *)" us");
}
#endif

static int copy_uboot_code_to_entry_point(void)
{
	int ret = EXT_SEC_FAILURE;
	u32 boot_img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)boot_img_int_ddr_addr;
	u32 dst_len = uboot_info->code_area_len;
	ret = dma_copy(uboot_info->uboot_entry_point, dst_len, boot_img_int_ddr_addr + UBOOT_CODE_INFO_SIZE);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int copy_ree_key_to_entry_point(void)
{
	errno_t err;
	u32 boot_img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)boot_img_int_ddr_addr;
	u32 dst_len;
	u32 dst;
	u32 src_len;

	dst_len = UBOOT_CODE_INFO_SIZE;
	dst = uboot_info->uboot_entry_point - UBOOT_CODE_INFO_SIZE;
	src_len = sizeof(uboot_code_info);
	err = memcpy_s((void *)(uintptr_t)dst, dst_len, (void *)(uintptr_t)(uboot_info), src_len);
	if (err != EOK)
		return EXT_SEC_FAILURE;
	dst_len = REE_BOOT_KEY_AREA_SIZE;
	dst = uboot_info->uboot_entry_point - UBOOT_CODE_INFO_SIZE - REE_BOOT_KEY_AREA_SIZE;
	src_len = sizeof(ree_key_area_s);
	err = memcpy_s((void *)(uintptr_t)dst, dst_len, (void *)(uintptr_t)(get_ree_key_area_addr()), src_len);
	if (err != EOK)
		return EXT_SEC_FAILURE;
	dst_len = TP_EXT_KEY_AREA_SIZE;
	dst = uboot_info->uboot_entry_point - UBOOT_CODE_INFO_SIZE - REE_BOOT_KEY_AREA_SIZE - TP_EXT_KEY_AREA_SIZE;
	src_len = sizeof(tp_key_area_s);
	err = memcpy_s((void *)(uintptr_t)dst, dst_len, (void *)(uintptr_t)(TP_EXT_KEY_AREA_ADDR), src_len);
	if (err != EOK)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

#define WDG_SYS_TIMEOUT        	(1000)  /* 1s */
#define WDG_CLK_3MHZ    	3000000
td_void sys_watchdog_enable(td_void)
{
	watchdog_enable(0, WDG_SYS_TIMEOUT, WDG_CLK_3MHZ);
}

static void set_sram_secure_region(u32 end)
{
	u32 secure_region_size;
	u32 value;
	secure_region_size = end - SRAM_BASE;
	if (secure_region_size % SIZE_1K != 0)
		secure_region_size += SIZE_1K;
	secure_region_size = secure_region_size / SIZE_1K;
	value = reg_get(SEC_BOOTRAM_SEC_CFG) & 0xffffff00;
	value |= secure_region_size;
	reg_set(SEC_BOOTRAM_SEC_CFG, value);
	value = reg_get(SEC_BOOTRAM_SEC_CFG) & 0x000000ff;
	if (value != secure_region_size) {
		log_serial_puts((const s8 *)"secure sram set error\n");
		call_reset();
	}
}

static void set_sram_secure_lock(void)
{
	reg_set(SEC_BOOTRAM_SEC_CFG_LOCK1, 1);
}

static int first_recv_len_check(size_t lenth)
{
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	int right_len = REE_BOOT_KEY_AREA_SIZE + PARM_AREA_INFO_SIZE + \
					para_info->para_area_addr + para_info->para_area_len * 1;
	if (lenth == right_len)
		return EXT_SEC_SUCCESS;
	err_print(FIRST_RECV_LEN_CHECK_ERR);
	return EXT_SEC_FAILURE;
}

static int second_recv_len_check(size_t lenth)
{
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)get_uboot_info_download_ddr_addr();
	int right_len = UBOOT_CODE_INFO_SIZE + uboot_info->code_area_len;
	if (lenth == right_len)
		return EXT_SEC_SUCCESS;
	err_print(SECOND_RECV_LEN_CHECK_ERR);
	return EXT_SEC_FAILURE;
}

static int get_ree_key_and_param_from_uart()
{
	size_t uart_rcv_len;
	int ret = EXT_SEC_FAILURE;
	ret = copy_from_uart((void *)(uintptr_t)(get_ree_key_area_addr()), &uart_rcv_len, FIRST_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_REE_KEY_AND_PARAM_FROM_UART_ERR);
		return EXT_SEC_FAILURE;
	}
	if (first_recv_len_check(uart_rcv_len) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}
#define BOARD_TYPE_FRAME_SEND_LEN   	11
#define BOARD_TYPE_RCV_LEN 		14
#define BOARD_TYPE_CRC_LEN      	2
#define BOARD_TYPE_PAYLOAD_SEND_LEN  	8
#define FRAM_CRC_H			0x8
#define FRAM_OFF0			0x0
#define FTAM_OFF1 			0x1
#define FRAM_OFF2 			0x2
#define FRAM_CRC_H_OFF			(BOARD_TYPE_RCV_LEN - 1 - 1)
#define FRAM_CRC_L_OFF			(BOARD_TYPE_RCV_LEN - 1)
#define CRC_LEN				(0x2)
#define TIMEOUT_USB			2000
#define CRC_NUMS			0xFFFF

#include "uart.h"
#include "burn_protocol.h"
#include "crc.h"
static int send_board_type_to_usb(u32 type)
{
	frame_info frame;
	unsigned char board_type_frame_send_buf[BOARD_TYPE_FRAME_SEND_LEN];
	unsigned char board_type_frame_rcv_buf[BOARD_TYPE_RCV_LEN];
	int i;
	size_t usb_rcv_len = 0;
	volatile int ret = EXT_SEC_FAILURE;
	unsigned short crc = 0;
	unsigned short rev_crc;
	if (memset_s(&frame, sizeof(frame_info), 0, sizeof(frame_info)) != EOK)
		return EXT_SEC_FAILURE;
	usb3_driver_init(); /* re init */
	int crc_nums = CRC_NUMS;
	while (1) {
		if (crc_nums < 0) {
			log_serial_puts((const s8 *)"connect timeout\n");
			return EXT_SEC_FAILURE;
		}
		crc_nums--;
		ret = copy_from_usb((void *)board_type_frame_rcv_buf, &usb_rcv_len, BOARD_TYPE_RCV_LEN);
		if (ret != EXT_SEC_SUCCESS)
			continue;
		if (usb_rcv_len != BOARD_TYPE_RCV_LEN)
			continue;
		if (board_type_frame_rcv_buf[FRAM_OFF0] != XBOARD || \
				(board_type_frame_rcv_buf[FTAM_OFF1] != (u8)(~board_type_frame_rcv_buf[FRAM_OFF2])))
			continue;
		rev_crc = (unsigned short)((board_type_frame_rcv_buf[FRAM_CRC_H_OFF] << FRAM_CRC_H) | \
								   (board_type_frame_rcv_buf[FRAM_CRC_L_OFF]));
		crc = 0;
		for (i = 0; i < BOARD_TYPE_RCV_LEN - CRC_LEN; i++) {
			crc = cal_crc_perbyte(board_type_frame_rcv_buf[i], crc);
		}
		if (rev_crc == crc) {
			(void)build_board_type_frame(type, board_type_frame_send_buf, BOARD_TYPE_FRAME_SEND_LEN);
			send_to_usb((char *)board_type_frame_send_buf, BOARD_TYPE_FRAME_SEND_LEN);
			log_serial_puts((const s8 *)"send_board_type_to_usb ok\n");
			return EXT_SEC_SUCCESS;
		}
	}
	return EXT_SEC_FAILURE;
}

static int get_ree_key_and_param_from_usb(void)
{
	size_t usb_rcv_len;
	int ret;
	ret = copy_from_usb((void *)(uintptr_t)(get_ree_key_area_addr()), &usb_rcv_len, FIRST_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_REE_KEY_AND_PARAM_FROM_USB_ERR);
		return EXT_SEC_FAILURE;
	}
	if (first_recv_len_check(usb_rcv_len) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int get_uboot_info_and_code_from_usb()
{
	u32 img_int_ddr_addr;
	size_t usb_rcv_len;
	int ret = EXT_SEC_FAILURE;
	img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	ret = copy_from_usb((void *)(uintptr_t)img_int_ddr_addr, &usb_rcv_len, SECOND_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_UBOOT_INFO_AND_CODE_FROM_USB_ERR);
		return EXT_SEC_FAILURE;
	}
	if ((second_recv_len_check(usb_rcv_len) != EXT_SEC_SUCCESS))
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int get_uboot_info_and_code_from_uart()
{
	u32 img_int_ddr_addr;
	size_t img_total_len = 0;
	int ret = EXT_SEC_FAILURE;
	img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	ret = copy_from_uart((void *)(uintptr_t)img_int_ddr_addr, &img_total_len, SECOND_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_UBOOT_INFO_AND_CODE_FROM_UART_ERR);
		return EXT_SEC_FAILURE;
	}
	if ((second_recv_len_check(img_total_len) != EXT_SEC_SUCCESS))
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static void handle_ree_key_and_param(u32 channel_type, u32 board_param_index)
{
	int ret = EXT_SEC_FAILURE;
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		watchdog_disable(0);
		if (channel_type == BOOT_SEL_UART) {
			ret = send_board_type_to_uart(board_param_index);
			if (ret != EXT_SEC_SUCCESS)
				failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
			ret = get_ree_key_and_param_from_uart();
		} else if (channel_type == BOOT_SEL_USB) {
			ret = send_board_type_to_usb(board_param_index);
			if (ret != EXT_SEC_SUCCESS)
				failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
			ret = get_ree_key_and_param_from_usb();
		}
		if (ret != EXT_SEC_SUCCESS)
			failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
		sys_watchdog_enable();
	} else if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC || channel_type == BOOT_SEL_SDIO) {
		ret = verify_all_lpds_data();
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
		get_img_backup_params(&g_backup_params);
		ret = get_ree_key_and_paras_info_from_device(&g_backup_params, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	} else {
		call_reset();
	}

	fun_redun_check(handle_ree_key_area(), HANDLE_REE_KEY_ERR, VERIFY_REE_KEY_FAILED);
	fun_redun_check(handle_ddr_param_info(board_param_index), HANDLE_PARAM_AREA_INFO_ERR, VERIFY_PARAMS_FAILED);
	if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC || channel_type == BOOT_SEL_SDIO) {
		ret = get_paras_data_from_flash(&g_backup_params, board_param_index, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	}
	fun_redun_check(handle_ddr_param(board_param_index), HANDLE_PARAM_AREA_ERR, VERIFY_PARAMS_FAILED);
}

static void handle_uboot(u32 channel_type)
{
	int ret = EXT_SEC_FAILURE;
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		watchdog_disable(0);
		if (channel_type == BOOT_SEL_UART)
			ret = get_uboot_info_and_code_from_uart();
		else if (channel_type == BOOT_SEL_USB)
			ret = get_uboot_info_and_code_from_usb();
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
		sys_watchdog_enable();
	} else if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC || channel_type == BOOT_SEL_SDIO) {
		ret = get_uboot_info_from_flash(&g_backup_params, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
	} else {
		call_reset();
	}
	fun_redun_check(handle_uboot_info(), HANDLE_UBOOT_CODE_INFO_ERR, VERIFY_UBOOT_FAILED);
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		ret = copy_uboot_code_to_entry_point();
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
	}
	if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC || channel_type == BOOT_SEL_SDIO) {
		ret = get_uboot_code_from_flash(&g_backup_params, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	}
	fun_redun_check(handle_uboot_code(), HANDLE_UBOOT_CODE_ERR, VERIFY_UBOOT_FAILED);
}

void main_entry(void)
{
	u32 channel_type;
	u32 board_param_index = 0;
	uintptr_t chk_sum = ~EXT_DRV_KLAD_KEY_TYPE_SBRK0;
	uintptr_t chk_sum_abrk0 = ~EXT_DRV_KLAD_KEY_TYPE_ABRK0;
	save_cur_point_syscnt();
	(void)uapi_drv_klad_rootkey_disable(EXT_DRV_KLAD_KEY_TYPE_SBRK0, chk_sum);
	(void)uapi_drv_klad_rootkey_disable(EXT_DRV_KLAD_KEY_TYPE_ABRK0, chk_sum_abrk0);
	start_flow_prepare();
	board_param_index = get_board_param_index();
	channel_type = get_data_channel_type();
	if (channel_type == BOOT_SEL_UNKNOW)
		failure_process(DETECT_BOOT_DEVICE_FAILED);
	handle_ree_key_and_param(channel_type, board_param_index);
	system_init(channel_type);
	handle_uboot(channel_type);
#ifdef CFG_DEBUG_INFO
	dump_addr_info();
#endif
	copy_ree_key_to_entry_point();
	set_sram_secure_region(SRAM_END - SIZE_1K);
	set_sram_secure_lock();
	calculate_all_lpds_data();
	enable_lpds_lock();
	store_booting_param();
	watchdog_disable(0);
#ifdef CFG_DEBUG_INFO
	print_current_el(0);
#endif
	save_cur_point_syscnt();
#ifdef CFG_DEBUG_INFO
	print_boot_time();
#endif
	start_secondary_core(1, GSL_CODE_AREA_ADDR);
	load_and_start_os();
}

