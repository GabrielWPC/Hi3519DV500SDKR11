/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "training/ddrtrn_console.h"
#include "hal/uart/ddrtrn_uart.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"

#ifdef DDR_TRAINING_CONSOLE_CONFIG

/* DDR read line */
char *ddrtrn_readline(char *str, int len)
{
	unsigned int c;
	char *p = str;
	while (len > 0) {
		c = ddrtrn_hal_console_getc();
		switch (c) {
		case '\r':
		case '\n':
			*p = '\0';
			DDR_PUTC('\r');
			DDR_PUTC('\n');
			return str;
		case 0x08:
		case 0x7F:
			if (p > str) {
				p--;
				len++;
				DDR_PUTC('\b');
				DDR_PUTC(' ');
				DDR_PUTC('\b');
			}
			break;
		default:
			if (isprint(c) != 0) {
				(*p++) = (char)c;
				len--;
				DDR_PUTC(c);
			}
			break;
		}
	}
	(*--p) = '\0';
	return str;
}

/* HEX to INT */
int hex2int(char **ss, unsigned int *n)
{
	unsigned char *s = (unsigned char *)(*ss);
	while (isspace(*s) != 0)
		s++;
	if ((*s) == NULL)
		return -1;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
		s += 2; /* Skip the first 2 characters: 0x */
	for ((*n) = 0; isxdigit(*s) != 0; s++) {
		(*n) = ((*n) << 4); /* shift left 4 */
		if ((*s) >= '0' && (*s) <= '9') {
			(*n) |= ((*s) - '0');
		} else if ((*s) >= 'a' && (*s) <= 'f') {
			(*n) |= ((*s) + 10 - 'a'); /* transfer a-f to 10-15 */
		} else if ((*s) >= 'A' && (*s) <= 'F') {
			(*n) |= ((*s) + 10 - 'A'); /* transfer A-F to 10-15 */
		}
	}
	if (isspace(*s) != 0 || (*s) == NULL) {
		while (isspace(*s) != 0)
			s++;
		(*ss) = (char *)s;
		return 0;
	}
	return -1;
}
/*
 * DDR do memory write.
 * mw address value [count]
 */
int ddrtrn_do_memory_write(char *cmd)
{
	unsigned int address;
	unsigned int value;
	unsigned int count = ALIGN_COUNT;
	if (hex2int(&cmd, &address))
		return -1;
	if (hex2int(&cmd, &value))
		return -1;
	if ((*cmd) && hex2int(&cmd, &count))
		return -1;
	if ((address & 0x03) || (count & 0x03)) {
		ddrtrn_info("parameter should align with 4 bytes.\n");
		return -1;
	}
	for (; count > 0; count -= ALIGN_COUNT, address += ALIGN_COUNT) /* align with 4 bytes */
		ddrtrn_reg_write(value, address);
	return 0;
}
/*
 * DDR do memory display.
 * md address [count]
 */
int ddrtrn_do_memory_display(char *cmd)
{
	unsigned int ix;
	unsigned int loop;
	unsigned int address;
	unsigned int count = 64;
	if (hex2int(&cmd, &address))
		return -1;
	if ((*cmd) && hex2int(&cmd, &count))
		return -1;
	if (count < ALIGN_COUNT)
		count = ALIGN_COUNT;
	address &= ~0x03;
	loop = (count & ~0x03);
	while (loop > 0) {
		DDR_PUTC('0');
		DDR_PUTC('x');
		DDR_PUT_HEX(address);
		DDR_PUTC(':');
		for (ix = 0; ix < ALIGN_COUNT && loop > 0;
				ix++, loop -= ALIGN_COUNT, address += ALIGN_COUNT) {
			DDR_PUTC(' ');
			DDR_PUT_HEX(ddrtrn_reg_read(address));
		}
		DDR_PUTC('\r');
		DDR_PUTC('\n');
	}
	return 0;
}

int ddrtrn_do_sw_training(char *cmd)
{
	int result;
	ddrtrn_hal_cfg_init();
#ifdef DDR_TRAINING_CMD
	ddrtrn_hal_res_flag_enable();
#endif
	result = ddrtrn_training_all();
	result += ddrtrn_dcc_training_func();
	return result;
}

int ddrtrn_do_hw_training(char *cmd)
{
	int result;
	ddrtrn_hal_cfg_init();
	result = ddrtrn_hw_training_func();
	return result;
}

/* Do DDR training console if sw training or hw training fail */
int ddrtrn_training_console(void)
{
	char str[256]; /* 256: Command length range */
	char *p = NULL;
	while (1) {
		DDR_PUTC('d');
		DDR_PUTC('d');
		DDR_PUTC('r');
		DDR_PUTC('#');
		p = ddrtrn_readline(str, sizeof(str));
		while (isspace(*p) != 0)
			p++;
		if (p[0] == 'q')
			break;
		if (p[0] == 'm' && p[1] == 'w')
			ddrtrn_do_memory_write(p + 2); /* p[2]:Third character */
		else if (p[0] == 'm' && p[1] == 'd')
			ddrtrn_do_memory_display(p + 2); /* p[2]:Third character */
		else if (p[0] == 's' && p[1] == 'w')
			ddrtrn_do_sw_training(p + 2); /* p[2]:Third character */
		else if (p[0] == 'h' && p[1] == 'w')
			ddrtrn_do_hw_training(p + 2); /* p[2]:Third character */
	}
	return 0;
}
#else
int ddrtrn_training_console(void)
{
	ddrtrn_warning("Not support DDR training console");
	return 0;
}
#endif /* DDR_TRAINING_CONSOLE_CONFIG */

int ddrtrn_training_console_if(void)
{
	return DDR_TRAINING_CONSOLE();
}
