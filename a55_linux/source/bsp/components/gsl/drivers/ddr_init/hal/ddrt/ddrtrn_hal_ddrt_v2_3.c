/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

/* Check ddrt test result. Success return 0, fail return -1 */
int ddrtrn_ddrt_check(void)
{
	unsigned int byte_index_to_dmc = ddrtrn_hal_get_cur_byte();
	unsigned int cur_dq = ddrtrn_hal_get_cur_dq();

	/* ddrt test the byte relate to dmc, make sure not overflow */
	if (ddrtrn_hal_get_cur_byte() >= (ddrtrn_hal_get_dmc_id() << 1))
		byte_index_to_dmc = ddrtrn_hal_get_cur_byte() - (ddrtrn_hal_get_dmc_id() << 1);

	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED0);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED1);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED2);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED3);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED4);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED5);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED6);
	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED7);
	ddrt_reg_write(0, DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	if (ddrtrn_ddrt_test(DDRT_WR_COMPRARE_MODE | DDRT_PATTERM_PRBS11,
		byte_index_to_dmc, cur_dq))
		return -1;

	ddrt_reg_write(ddrtrn_hal_get_cur_pattern(), DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	if (ddrtrn_ddrt_test(DDRT_WR_COMPRARE_MODE | DDRT_PATTERM_PRBS11,
		byte_index_to_dmc, cur_dq))
		return -1;

	ddrt_reg_write(DDRT_PRBS_SEED_DEFAULT_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED0);
	ddrt_reg_write(DDRT_PRBS_SEED1_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED1);
	ddrt_reg_write(DDRT_PRBS_SEED2_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED2);
	ddrt_reg_write(DDRT_PRBS_SEED3_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED3);
	ddrt_reg_write(DDRT_PRBS_SEED4_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED4);
	ddrt_reg_write(DDRT_PRBS_SEED5_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED5);
	ddrt_reg_write(DDRT_PRBS_SEED6_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED6);
	ddrt_reg_write(DDRT_PRBS_SEED7_VAL, DDR_REG_BASE_DDRT + DDRT_PRBS_SEED7);
	ddrt_reg_write(0, DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	if (ddrtrn_ddrt_test(DDRT_WR_COMPRARE_MODE | DDRT_PATTERM_PRBS11,
		byte_index_to_dmc, cur_dq))
		return -1;

	ddrt_reg_write(ddrtrn_hal_get_cur_pattern(), DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	if (ddrtrn_ddrt_test(DDRT_WR_COMPRARE_MODE | DDRT_PATTERM_PRBS11,
		byte_index_to_dmc, cur_dq))
		return -1;

	return 0;
}
