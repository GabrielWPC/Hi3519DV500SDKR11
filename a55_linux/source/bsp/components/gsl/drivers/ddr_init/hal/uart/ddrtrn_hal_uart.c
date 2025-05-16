/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/uart/ddrtrn_uart.h"

/* DDR console get char */
int ddrtrn_hal_console_getc(void)
{
	unsigned int count;
	unsigned int data;
	/* Wait until there is data in the FIFO */
	count = DDR_HWR_WAIT_TIMEOUT;
	while (count--) {
		if ((ddrtrn_reg_read(DDR_UART_BASE_REG + UART_PL01X_FR) & UART_PL01X_FR_RXFE) == 0)
			break;
	}
	if (count == DDR_HWR_WAIT_TIMEOUT) {
		ddrtrn_error("ddr console get char error");
		return -1;
	}
	data = ddrtrn_reg_read(DDR_UART_BASE_REG + UART_PL01X_DR);
	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		ddrtrn_reg_write(0xFFFFFFFF, DDR_UART_BASE_REG + UART_PL01X_ECR);
		return -1;
	}
	return (int)data;
}
