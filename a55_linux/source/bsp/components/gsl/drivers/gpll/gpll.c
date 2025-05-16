/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "types.h"
#include "platform.h"
#include "lib.h"
#include "err_print.h"
#include "common.h"
#include "../uart/serial_pl011.h"
#include "gpll.h"

static void delay_cl(u32 cl)
{
	u32 cl_count;
	for (cl_count = ((cl) >> 1); cl_count != 0; --cl_count)
		__asm__("nop");
}

static u32 otp_shadow_get_boot_mode(void)
{
	return reg_getbits(OTP_SOC_FUSE_ADDR_BSAE, BOOT_FREQ_MODE_SEL_BIT_OFFSET, BOOT_FREQ_MODE_SEL_BIT_WIDTH);
}

u8 get_ddr_scrub_and_gpll_switch(void)
{
	return (u8)reg_getbits(OTP_SHADOWED_ATE_CHIP_VER,
			DDR_SCRUB_AND_GPLL_SWITHC_OFFSET,
			DDR_SCRUB_AND_GPLL_SWITHC_WIDTH);
}

static void low_freq_mode_switch(cpu_and_sys_sub_state state)
{
	u32 reg_value;

	reg_value = reg_get(REG_BASE_CRG + REG_PERI_CRG2064);
	reg_value &= ~(A55_FCM_CKSEL_MASK | A55_CKSEL_MASK);
	if (state == SETTING)
		reg_value |= (A55_FCM_CKSEL_24M | A55_CKSEL_24M); /* step1.1: cpu clk switch */
	else if (state == RESUME)
		reg_value |= (A55_FCM_CKSEL_675M | A55_CKSEL_675M); /* step3.1: resume cpu clk switch */
	reg_set(REG_BASE_CRG + REG_PERI_CRG2064, reg_value);

	reg_value = reg_get(REG_BASE_CRG + REG_PERI_CRG2048);
	reg_value &= ~(DDRAXI_CKSEL_MASK);
	if (state == SETTING)
		reg_value |= (DDRAXI_CKSEL_594M); /* step1.1: sys bus clk switch */
	else if (state == RESUME)
		reg_value |= (DDRAXI_CKSEL_540M);  /* step3.1: resume sys bus clk switch */

	reg_set(REG_BASE_CRG + REG_PERI_CRG2048, reg_value);
}

static void high_freq_mode_switch(cpu_and_sys_sub_state state)
{
	u32 reg_value;

	reg_value = reg_get(REG_BASE_CRG + REG_PERI_CRG2064);
	reg_value &= ~(A55_FCM_CKSEL_MASK | A55_CKSEL_MASK);
	if (state == SETTING)
		reg_value |= (A55_FCM_CKSEL_24M | A55_CKSEL_24M); /* step1.1: cpu clk switch */
	else if (state == RESUME)
		reg_value |= (A55_FCM_CKSEL_675M | A55_CKSEL_833M); /* step3.1: resume cpu subsys clk switch */

	reg_set(REG_BASE_CRG + REG_PERI_CRG2064, reg_value);
}

static void config_cpu_and_sys_bus_clk(void)
{
	u32 boot_mode = otp_shadow_get_boot_mode();
	if (boot_mode == LOW_FREQ_BOOT_MODE) {
		low_freq_mode_switch(SETTING);
		return;
	}
	if (boot_mode == HIGH_FREQ_BOOT_MODE) {
		high_freq_mode_switch(SETTING);
		return;
	}
}

static void close_realative_clk(u32 channel_type)
{
	/* close fmc clk */
	if (channel_type == BOOT_SEL_FLASH)
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG4048, FMC_CLK_EN_OFFSET, FMC_CLK_EN_WIDTH, FMC_CLK_DIS_VAL);
	/* close mmc clk */
	if (channel_type == BOOT_SEL_EMMC)
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG3376, MMC_CLK_EN_OFFSET, MMC_CLK_EN_WIDTH, MMC_CLK_DIS_VAL);
	/* close uart0 clk */
	if (channel_type == BOOT_SEL_UART) {
		if (wait_uart_tx_busy_state() != EXT_SEC_SUCCESS) {
			err_print(UART_BUSY_WATE_TIMEOUT_ERR);
			call_reset();
		}
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG4192, UART0_CLK_EN_OFFSET, UART0_CLK_EN_WIDTH, UART0_CLK_DIS_VAL);
	}
}

static void resume_relative_clk(u32 channel_type)
{
	/* resume fmc clk */
	if (channel_type == BOOT_SEL_FLASH)
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG4048, FMC_CLK_EN_OFFSET, FMC_CLK_EN_WIDTH, FMC_CLK_EN_VAL);
	/* resume mmc clk */
	if (channel_type == BOOT_SEL_EMMC)
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG3376, MMC_CLK_EN_OFFSET, MMC_CLK_EN_WIDTH, MMC_CLK_EN_VAL);
	/* resume uart0 clk */
	if (channel_type == BOOT_SEL_UART)
		reg_setbits(REG_BASE_CRG + REG_PERI_CRG4192, UART0_CLK_EN_OFFSET, UART0_CLK_EN_WIDTH, UART0_CLK_EN_VAL);
}

static u32 get_pll_final_status(u32 pll_lock_status_reg)
{
	return reg_getbits(pll_lock_status_reg, PLL_FIANL_LOCK_BIT_OFFSET, PLL_FIANL_LOCK_BIT_WIDTH);
}

static void query_plls_final_status(u32 *gpll_final_status)
{
	u32 i = 0;

	if (gpll_final_status == NULL)
		return;
	for (i = 0; i < PLL_LOCK_QUERY_MAX_TIMES; i++) {
		if (*gpll_final_status != PLL_LOCKED) {
			if (get_pll_final_status(GPLL_LOCK_STATUS_REG) == PLL_FINAL_LOCKED)
				*gpll_final_status = PLL_LOCKED;
		}

		if (*gpll_final_status == PLL_LOCKED) {
			break;
		}
		delay_cl(_1_US_);
	}
}

static void set_pll_power_down(u32 pll_ctrl_reg)
{
	reg_setbits(pll_ctrl_reg, PLL_POWER_SWITCH_BIT_OFFSET, PLL_POWER_SWITCH_BIT_WIDTH, POWER_DOWN);
}

static void set_pll_power_on(u32 pll_ctrl_reg)
{
	reg_setbits(pll_ctrl_reg, PLL_POWER_SWITCH_BIT_OFFSET, PLL_POWER_SWITCH_BIT_WIDTH, POWER_ON);
}

static int config_gpll(void)
{
	u32 i = 0;
	u32 gpll_final_status = PLL_POWER_DOWN;

	for (i = 0; i < PLL_LOCK_STATUS_COUNT; i++) {
		/* step2.1 */
		set_pll_power_down(GPLL_CTRL_REG1);
		/* step2.2 */
		reg_set(GPLL_CTRL_REG0, GPLL_CTRL_REG0_VAL);
		delay_cl(_1_US_);
		/* step2.3 */
		set_pll_power_on(GPLL_CTRL_REG1);
		delay_cl(_20_US_);
		/* step2.4 */
		query_plls_final_status(&gpll_final_status);
		if (gpll_final_status == PLL_LOCKED) {
			return EXT_SUCCESS;
		}
	}
	return EXT_FAILURE;
}

static void resume_cpu_and_sys_bus_clk(void)
{
	u32 boot_mode = otp_shadow_get_boot_mode();
	if (boot_mode == LOW_FREQ_BOOT_MODE) {
		low_freq_mode_switch(RESUME);
		return;
	}
	if (boot_mode == HIGH_FREQ_BOOT_MODE) {
		high_freq_mode_switch(RESUME);
		return;
	}
}

void bsp_gpll_config(u32 channel_type)
{
	if (get_ddr_scrub_and_gpll_switch() != OPEN)
		return;
	/* step1 */

	/* step1.1 config cpu and sys sub clk */
	config_cpu_and_sys_bus_clk();

	/* step1.2 close relative mode clk */
	close_realative_clk(channel_type);

	/* step1.3  check cpu and sys sub clk  or delay 20us */
	delay_cl(_20_US_);

	/* step2 config gpll */
	if (config_gpll() != EXT_SUCCESS) {
		if (channel_type != BOOT_SEL_UART)
			err_print(GPLL_CONFIG_ERR);
		call_reset();
	}

	/* step3 */

	/* step3.1 resume cpu and sys sub clk */
	resume_cpu_and_sys_bus_clk();

	/* step3.2  resume relative mode clk  */
	resume_relative_clk(channel_type);

	/* step3.2  resume relative mode clk  or delay 20us */
	delay_cl(_20_US_);
}
