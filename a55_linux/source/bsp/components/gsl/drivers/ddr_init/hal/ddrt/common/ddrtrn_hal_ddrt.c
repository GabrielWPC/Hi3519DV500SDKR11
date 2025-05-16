/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"
#include "td_type.h"

#ifdef DDR_DDRT_SPECIAL_CONFIG
/* Some special DDRT need read register repeatedly */
unsigned int ddrt_reg_read(unsigned int addr)
{
	int times = 0;
	unsigned int data0, data1, data2;
	do {
		data0 = ddrtrn_reg_read(addr);
		data1 = ddrtrn_reg_read(addr);
		data2 = ddrtrn_reg_read(addr);
		times++;
	} while (((data0 != data1) || (data1 != data2)) && (times < DDRT_READ_TIMEOUT));
	if (times >= DDRT_READ_TIMEOUT) {
		ddrtrn_fatal("DDRT wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_DDRT_TIME_OUT, 0, -1, -1);
	}
	return data0;
}

/* Some special DDRT need write twice register */
void ddrt_reg_write(unsigned int data, unsigned int addr)
{
	unsigned int tmp;
	tmp = ddrtrn_reg_read(addr);
	ddrtrn_reg_write(data, addr);
	ddrtrn_reg_write(data, addr);
}
#else
/* Some special DDRT need read register repeatedly */
unsigned int ddrt_reg_read(unsigned int addr)
{
	return (*(volatile unsigned int *)((uintptr_t)addr));
}
void ddrt_reg_write(unsigned int val, unsigned int addr)
{
	(*(volatile unsigned int *)((uintptr_t)addr)) = val;
}
#endif /* DDR_DDRT_SPECIAL_CONFIG */

unsigned int ddrtrn_hal_ddrt_get_mem_width(void)
{
	return ((ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc() + DDR_DMC_CFG_DDRMODE) >>
			 DMC_MEM_WIDTH_BIT) & DMC_MEM_WIDTH_MASK);
}

unsigned int ddrtrn_hal_ddrt_get_addr(void)
{
	return ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_ADDR);
}
