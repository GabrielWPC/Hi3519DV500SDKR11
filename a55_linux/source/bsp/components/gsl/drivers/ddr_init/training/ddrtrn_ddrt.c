/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

/* Init DDRT register before DDRT test */
void ddrtrn_ddrt_init(unsigned int mode)
{
	unsigned int mem_width;
	unsigned int mem_config;
	unsigned int offset = 0;
	if (ddrtrn_hal_get_rank_id() == 1)
		offset = ddrtrn_hal_get_rank_size();
	ddr_training_ddrt_prepare_func();
	mem_width = ddrtrn_hal_ddrt_get_mem_width();
	mem_config = ((mem_width - 1) << DDRT_DDR_MEM_WIDTH) |
				 DDRT_DDR_COL_WIDTH | DDRT_DDR_ROW_WIDTH | DDRT_DDR_BANK_WIDTH;
	/* DDRT SDRAM config */
	ddrt_reg_write(mem_config, DDR_REG_BASE_DDRT + DDRT_MEM_CONFIG);
	/* DDR Address Base */
	ddrt_reg_write(ddrt_get_test_addr(DDRT_CFG_BASE_ADDR),
				   DDR_REG_BASE_DDRT + DDRT_DDR_BASE_ADDR);
	/* DDRT test DDR using space */
	ddrt_reg_write(ddrt_get_test_addr(ddrtrn_ddrt_get_test_addr() + offset),
				   DDR_REG_BASE_DDRT + DDRT_ADDR);
	ddrt_reg_write(DDRT_CFG_SEED, DDR_REG_BASE_DDRT + DDRT_SEED);
	if (mode == DDR_DDRT_MODE_GATE) {
		/* Read or Write Once */
		ddrt_reg_write(DDRT_CFG_BURST_CFG_GATE,
					   DDR_REG_BASE_DDRT + DDRT_BURST_CONFIG);
		ddrt_reg_write(0x0,  DDR_REG_BASE_DDRT + DDRT_BURST_NUM);
		ddrt_reg_write(0x0,  DDR_REG_BASE_DDRT + DDRT_ADDR_NUM);
		ddrt_reg_write(0x0,  DDR_REG_BASE_DDRT + DDRT_LOOP_NUM);
		ddrt_reg_write(DDRT_CFG_REVERSED,
					   DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	} else {
		/* reversed data form register init table */
		/* 128bit BURST4 */
		ddrt_reg_write(DDRT_CFG_BURST_CFG_DATAEYE,
					   DDR_REG_BASE_DDRT + DDRT_BURST_CONFIG);
		ddrt_reg_write(ddrtrn_hal_get_cur_dmc_ddrt_pattern(ddrtrn_hal_get_dmc_id()),
					   DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
		ddrt_reg_write(DDRT_CFG_BURST_NUM,
					   DDR_REG_BASE_DDRT + DDRT_BURST_NUM);
		ddrt_reg_write(DDRT_CFG_ADDR_NUM,
					   DDR_REG_BASE_DDRT + DDRT_ADDR_NUM);
		ddrt_reg_write(DDRT_CFG_LOOP_NUM,
					   DDR_REG_BASE_DDRT + DDRT_LOOP_NUM);
	}
	ddrtrn_debug("DDRT ADDR[%x = %x]", (DDR_REG_BASE_DDRT + DDRT_ADDR),
				 ddrtrn_hal_ddrt_get_addr());
}

static int ddrtrn_ddrt_test_process(int byte, int dq)
{
	unsigned int err_ovfl;
	unsigned int err_cnt;
	unsigned int dq_num;
	unsigned int dq_tmp;
	if (dq != -1) { /* check for dq */
		dq_num = ((unsigned int)byte << DDR_BYTE_DQ) + dq;
		err_ovfl = ddrt_reg_read(DDR_REG_BASE_DDRT + DDRT_DQ_ERR_OVFL) & (1 << dq_num);
		if (err_ovfl)
			return -1;
		if (dq > 3) /* 3: dq0-dq3 */
			dq_tmp = (unsigned int)(dq - 4) << DDR_BYTE_DQ; /* 4 dq: dq0-dq3,dq4-dq7 */
		else
			dq_tmp = (unsigned int)dq << DDR_BYTE_DQ;
		err_cnt = ddrt_reg_read(DDR_REG_BASE_DDRT +
								ddrt_dq_err_cnt(((unsigned int)byte << 1) + ((unsigned int)dq >> 2))); /* shift left 2: 4 dq */
		err_cnt = err_cnt & (0xff << dq_tmp);
		if (err_cnt)
			return -1;
	} else if (byte != -1) { /* check for byte */
		err_ovfl = ddrt_reg_read(DDR_REG_BASE_DDRT +
								 DDRT_DQ_ERR_OVFL) & (0xff << ((unsigned int)byte << DDR_BYTE_DQ));
		if (err_ovfl)
			return -1;
		err_cnt  = ddrt_reg_read(DDR_REG_BASE_DDRT +
								 ddrt_dq_err_cnt((unsigned int)byte << 1));
		err_cnt += ddrt_reg_read(DDR_REG_BASE_DDRT +
								 ddrt_dq_err_cnt(((unsigned int)byte << 1) + 1));
		if (err_cnt)
			return -1;
	}
	return 0;
}

/*
 * @mask : DDRT option mask.
 * @byte : DDR byte index.
 * @dq   : DDR dq index.
 * DDRT test. Support read_only mode and write_read_compare mode.
 * Success return 0, fail return -1.
 */
int ddrtrn_ddrt_test(unsigned int mask, int byte, int dq)
{
	int result;
	unsigned int regval;
	unsigned int times = 0;
	ddrt_reg_write(mask | DDRT_CFG_START, DDR_REG_BASE_DDRT + DDRT_OP);
	ddrt_reg_write(0, DDR_REG_BASE_DDRT + DDRT_STATUS);
	ddr_asm_dsb();
	do {
		regval = ddrt_reg_read(DDR_REG_BASE_DDRT + DDRT_STATUS);
		times++;
	} while (((regval & DDRT_TEST_DONE_MASK) == 0) && (times < DDRT_WAIT_TIMEOUT));
	if (times >= DDRT_WAIT_TIMEOUT) {
		ddrtrn_fatal("DDRT wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_DDRT_TIME_OUT, 0, -1, -1);
		return -1;
	}
	/* DDRT_READ_ONLY_MODE */
	if ((mask & DDRT_TEST_MODE_MASK) == DDRT_READ_ONLY_MODE)
		return 0;   /* return when DDRT finish */
	/* DDRT_WR_COMPRARE_MODE No error occurred, test pass. */
	if (regval & DDRT_TEST_PASS_MASK)
		return 0;
	result = ddrtrn_ddrt_test_process(byte, dq);
	return result;
}
