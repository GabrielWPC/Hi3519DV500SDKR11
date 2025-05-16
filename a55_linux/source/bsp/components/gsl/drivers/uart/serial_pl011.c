/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

/* Simple U-Boot driver for the PrimeCell PL011 UARTs on the IntegratorCP */
/* Should be fairly simple to make it work with the PL010 as well */

#include <platform.h>
#include <types.h>
#include <lib.h>
#include "serial_reg.h"
#include "share_drivers.h"
#include "soc_errno.h"
#include "otp.h"

#ifdef CFG_PL011_SERIAL

/*
 * IntegratorCP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */

#define CONSOLE_PORT CONFIG_CONS_INDEX
#define BAUDRATE CONFIG_BAUDRATE
#define MAX_PORT_NUM 1

/*
notes: all writable global variables should be placed in bss section,
should not have default value except zero
 */

static s32 serial_uart_base_valid(uart_num uart_base)
{
	switch (uart_base) {
	case UART_0:
		return EXT_SEC_SUCCESS;
	default:
		return EXT_SEC_FAILURE;
	}
}

s32 serial_putc(uart_num uart_base, const s8 c)
{
	if (is_sec_dbg_lv_enable() != AUTH_SUCCESS)
		return EXT_SEC_FAILURE;
	if (serial_uart_base_valid(uart_base) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (c == '\n')
		pl011_putc(uart_base, '\r');
	pl011_putc(uart_base, c);
	return EXT_SEC_SUCCESS;
}

s32 serial_puts(uart_num uart_base, const s8 *s)
{
	if (is_sec_dbg_lv_enable() != AUTH_SUCCESS)
		return EXT_SEC_FAILURE;
	if (serial_uart_base_valid(uart_base) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (s == NULL)
		return EXT_SEC_FAILURE;
	while (*s) {
		(void) serial_putc(uart_base, *s++);
	}
	return EXT_SEC_SUCCESS;
}

s32 serial_tstc(uart_num uart_base)
{
	if (serial_uart_base_valid(uart_base) != EXT_SEC_SUCCESS)
		return -1;
	return pl011_tstc(uart_base);
}

s32 serial_getc(uart_num uart_base, u8 *ch)
{
	if (serial_uart_base_valid(uart_base) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (ch == NULL)
		return EXT_SEC_FAILURE;
	if (serial_tstc(uart_base)) {
		*ch = (u8)pl011_getc(uart_base);
		return EXT_SEC_SUCCESS;
	}
	return EXT_SEC_FAILURE;
}

void serial_put_hex(unsigned int hex)
{
	if (is_sec_dbg_lv_enable() != AUTH_SUCCESS)
		return;
	unsigned int i;
	char c;
	for (i = 0; i <= 28; i += 4) { /* u32 value output per 4 bits, 28 is offset of the last 4 bits */
		c = (hex >> (28 - i)) & 0x0F; /* 28 is offset of the last 4 bits */
		/* transform the 4 bits data to ascii */
		if (c < 0xA)
			c += '0';
		else
			c += 'A' - 0xA;
		(void)serial_putc(REG_BASE_SERIAL0, c);
	}
}

void serial_put_dec(int dec)
{
	if (is_sec_dbg_lv_enable() != AUTH_SUCCESS)
		return;
	unsigned int i = 1;
	u32 target;
	if (dec < 0) {
		serial_putc(REG_BASE_SERIAL0, '-');
		target = -1 * dec;
	} else {
		target = (u32)dec;
	}
	do {
		char c = (char)((target % i) + '0');
		serial_putc(REG_BASE_SERIAL0, c);
		i *= UART_MULTIPLE_10;
	} while (i < dec);
}

void debug_puts(const char *buf, u32 hex)
{
	if (is_sec_dbg_lv_enable() != AUTH_SUCCESS)
		return;
	const char *fmt = buf;
	for (; *fmt != '\0'; fmt++) {
		if (*fmt != '%') {
			serial_putc(REG_BASE_SERIAL0, *fmt);
			continue;
		} else {
			fmt ++;
		}
		if (*fmt == '#') {
			serial_puts(REG_BASE_SERIAL0, (const td_s8 *)"0x");
			fmt ++;
		}
		switch (*fmt) {
		case 'x':
		case 'X':
			serial_put_hex(hex);
			fmt ++;
			break;
		case 'd':
		case 'D':
			serial_put_dec((int)hex);
			break;
		default:
			break;
		}
	}
	serial_putc(CONSOLE_PORT, '\r');
	serial_putc(CONSOLE_PORT, '\n');
}

/* wait until tx fifo is idle */
int wait_uart_tx_busy_state(void)
{
	u32 time_out = 0;
	while (time_out < TIMEOUT_UART) {
		if ((reg_get(REG_BASE_SERIAL0 + UART_PL011_FR) & UART_PL01X_FR_BUSY) == 0)
			return EXT_SEC_SUCCESS;
		time_out++;
	}
	return EXT_SEC_FAILURE;
}
#endif /* CFG_PL011_SERIAL */
