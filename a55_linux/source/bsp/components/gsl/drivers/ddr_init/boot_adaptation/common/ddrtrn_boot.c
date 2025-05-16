/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

#if (BOOTLOADER == DDR_BOOT_TYPE_AUX_CODE) || (BOOTLOADER == DDR_BOOT_TYPE_HSL)
#include "serial_pl011.h"
#define DDR_BOOT_ADAPTATION_PUTC serial_putc
#define DDR_BOOT_ADAPTATION_PUT_HEX serial_put_hex
#endif /* BOOTLOADER */

#if BOOTLOADER == DDR_BOOT_TYPE_UBOOT
#define DDR_BOOT_ADAPTATION_PUTC uart_early_putc
#define DDR_BOOT_ADAPTATION_PUT_HEX uart_early_put_hex
#endif /* BOOTLOADER */

/* Save DDR tarining result */
void ddrtrn_result_data_save(const struct ddrtrn_training_result_data *training)
{
	/* nothing to do when ddr training on power up */
	unused(training);
}

void ddrtrn_lpca_data_save(const struct ddrtrn_ca_data *data)
{
	/* nothing to do when ddr training on power up */
	unused(data);
}

/* Get DDRT test address */
unsigned int ddrtrn_ddrt_get_test_addr(void)
{
	return DDRT_CFG_TEST_ADDR_BOOT;
}

#ifdef DDR_TRAINING_UART_CONFIG
#ifdef DDR_TRAINING_MINI_LOG_CONFIG
/* Display DDR training error when boot */
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	DDR_BOOT_ADAPTATION_PUTC('E');
	DDR_BOOT_ADAPTATION_PUT_HEX(mask);
	DDR_BOOT_ADAPTATION_PUTC('P');
	DDR_BOOT_ADAPTATION_PUT_HEX(phy);
	DDR_BOOT_ADAPTATION_PUTC('B');
	DDR_BOOT_ADAPTATION_PUT_HEX(byte);
	DDR_BOOT_ADAPTATION_PUTC('D');
	DDR_BOOT_ADAPTATION_PUT_HEX(dq);
}
void ddrtrn_training_start(void)
{
	DDR_BOOT_ADAPTATION_PUTC('D');
	DDR_BOOT_ADAPTATION_PUTC('D');
	DDR_BOOT_ADAPTATION_PUTC('R');
}
void ddrtrn_training_success(void)
{
	DDR_BOOT_ADAPTATION_PUTC('S');
}
#else
/* Define string to print */
void ddr_training_local_str(void)
{
	asm volatile("str_wl:\n\t"
		".asciz \"WL\"\n\t"
		".align 2\n\t"

		"str_hwg:\n\t"
		".asciz \"HWG\"\n\t"
		".align 2\n\t"

		"str_gate:\n\t"
		".asciz \"Gate\"\n\t"
		".align 2\n\t"

		"str_ddrt:\n\t"
		".asciz \"DDRT\"\n\t"
		".align 2\n\t"

		"str_hwrd:\n\t"
		".asciz \"HWRD\"\n\t"
		".align 2\n\t"

		"str_mpr:\n\t"
		".asciz \"MPR\"\n\t"
		".align 2\n\t"

		"str_dataeye:\n\t"
		".asciz \"Dataeye\"\n\t"
		".align 2\n\t"

		"str_lpca:\n\t"
		".asciz \"LPCA\"\n\t"
		".align 2\n\t"

		"str_err:\n\t"
		".asciz \" Err:\"\n\t"
		".align 2\n\t"

		"str_phy:\n\t"
		".asciz \"Phy\"\n\t"
		".align 2\n\t"

		"str_byte:\n\t"
		".asciz \"Byte\"\n\t"
		".align 2\n\t"

		"str_dq:\n\t"
		".asciz \"DQ\"\n\t"
		".align 2\n\t"

		"str_ddrtr_start:\n\t"
		".asciz \"\r\\nDDRTR \"\n\t"
		".align 2\n\t"

		"str_ddrtr_suc:\n\t"
		".asciz \"Suc\"\n\t"
		".align 2\n\t");
}

/* Display DDR training error when boot */
#if (BOOTLOADER == DDR_BOOT_TYPE_AUX_CODE) || (BOOTLOADER == DDR_BOOT_TYPE_HSL)
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	DDR_BOOT_ADAPTATION_PUTC('\r');
	DDR_BOOT_ADAPTATION_PUTC('\n');
	/* error type */
	switch (mask) {
	case DDR_ERR_WL:
		asm volatile("adr    r0, str_wl\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_HW_GATING:
		asm volatile("adr    r0, str_hwg\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_GATING:
		asm volatile("adr    r0, str_gate\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_DDRT_TIME_OUT:
		asm volatile("adr    r0, str_ddrt\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_HW_RD_DATAEYE:
		asm volatile("adr    r0, str_hwrd\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_MPR:
		asm volatile("adr    r0, str_mpr\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_DATAEYE:
		asm volatile("adr    r0, str_dataeye\n\t"
			"bl serial_puts");
		break;
	case DDR_ERR_LPCA:
		asm volatile("adr    r0, str_lpca\n\t"
			"bl serial_puts");
		break;
	default:
		break;
	}

	/* error string */
	asm volatile("adr    r0, str_err\n\t"
		"bl serial_puts");

	/* error phy */
	if (phy != 0) {
		asm volatile("adr    r0, str_phy\n\t"
			"bl serial_puts");
		DDR_BOOT_ADAPTATION_PUT_HEX(phy);
	}

	/* error byte */
	if (byte != -1) {
		asm volatile("adr    r0, str_byte\n\t"
			"bl serial_puts");
		DDR_BOOT_ADAPTATION_PUT_HEX(byte);
	}

	/* error dq */
	if (dq != -1) {
		asm volatile("adr    r0, str_dq\n\t"
			"bl serial_puts");
		DDR_BOOT_ADAPTATION_PUT_HEX(dq);
	}
}

/* Display DDR training start when boot */
void ddr_training_start(void)
{
	asm volatile("push   {lr}\n\t"
		"adr    r0, str_ddrtr_start\n\t"
		"bl serial_puts\n\t"
		"pop    {lr}");
}

/* Display DDR training result when boot */
void ddr_training_success(void)
{
	asm volatile("push   {lr}\n\t"
		"adr    r0, str_ddrtr_suc\n\t"
		"bl serial_puts\n\t"
		"pop    {lr}");
}
#endif
#if BOOTLOADER == DDR_BOOT_TYPE_UBOOT
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	uart_early_putc('\r');
	uart_early_putc('\n');
	/* error type */
	switch (mask) {
	case DDR_ERR_WL:
		asm volatile(
				"adr	r0, str_wl\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_HW_GATING:
		asm volatile(
				"adr	r0, str_hwg\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_GATING:
		asm volatile(
				"adr	r0, str_gate\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_DDRT_TIME_OUT:
		asm volatile(
				"adr	r0, str_ddrt\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_HW_RD_DATAEYE:
		asm volatile(
				"adr	r0, str_hwrd\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_MPR:
		asm volatile(
				"adr	r0, str_mpr\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_DATAEYE:
		asm volatile(
				"adr	r0, str_dataeye\n\t"
				"bl	uart_early_puts"
				);
		break;
	case DDR_ERR_LPCA:
		asm volatile(
				"adr	r0, str_lpca\n\t"
				"bl	uart_early_puts"
				);
		break;
	default:
		break;
	}

	/* error string */
	asm volatile(
			"adr	r0, str_err\n\t"
			"bl	uart_early_puts"
			);

	/* error phy */
	if (phy != 0) {
		asm volatile(
				"adr	r0, str_phy\n\t"
				"bl	uart_early_puts"
				);
		uart_early_put_hex(phy);
	}

	/* error byte */
	if (byte != -1) {
		asm volatile(
				"adr	r0, str_byte\n\t"
				"bl	uart_early_puts"
				);
		uart_early_put_hex(byte);
	}

	/* error dq */
	if (dq != -1) {
		asm volatile(
				"adr	r0, str_dq\n\t"
				"bl	uart_early_puts"
				);
		uart_early_put_hex(dq);
	}
}

/* Display DDR training start when boot */
void ddr_training_start(void)
{
	asm volatile(
			"push	{lr}\n\t"
			"adr	r0, str_ddrtr_start\n\t"
			"bl	uart_early_puts\n\t"
			"pop	{lr}"
			);
}

/* Display DDR training result when boot */
void ddr_training_success(void)
{
	asm volatile(
			"push	{lr}\n\t"
			"adr	r0, str_ddrtr_suc\n\t"
			"bl	uart_early_puts\n\t"
			"pop	{lr}"
			);
}
#endif /* BOOTLOADER */
#endif /* DDR_TRAINING_CUT_CODE_CONFIG */
#else
void ddrtrn_training_error(unsigned int mask, unsigned int phy, int byte, int dq)
{
	unused(mask);
	unused(phy);
	unused(byte);
	unused(dq);
	return;
}
void ddrtrn_training_success(void)
{
	return;
}
void ddrtrn_training_start(void)
{
	return;
}
#endif /* DDR_TRAINING_UART_CONFIG */
