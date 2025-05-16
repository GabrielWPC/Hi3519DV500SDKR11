/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#ifdef DDR_AC_TRAINING_CONFIG

void ddrtrn_hal_ac_restore_value(int i, unsigned int clk_phase)
{
	unsigned int wdqs_phase, wdq_phase;
	unsigned int wdqs_phase_range, wdq_phase_range, phase_range;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	wdqs_phase =
		ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), i));
	wdq_phase =
		ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), i));
	wdqs_phase_range = PHY_WDQS_PHASE_MASK -
					   ((wdqs_phase >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK);
	wdq_phase_range = PHY_WDQ_PHASE_MASK -
					  ((wdq_phase >> PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK);
	phase_range = (wdqs_phase_range < wdq_phase_range) ? wdqs_phase_range : wdq_phase_range;
	phase_range = (phase_range < clk_phase) ? phase_range : clk_phase;
	ddrtrn_reg_write(wdqs_phase + (phase_range << PHY_WDQS_PHASE_BIT),
					 base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), i));
	ddrtrn_reg_write(wdq_phase + (phase_range << PHY_WDQ_PHASE_BIT),
					 base_phy + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), i));
}

int ddrtrn_hal_ac_ddrt_test(unsigned int mask, unsigned int base_phy)
{
	unsigned int regval;
	unsigned int times = 0;
	ddrtrn_reg_write(mask | DDRT_CFG_START, DDR_REG_BASE_DDRT + DDRT_OP);
	ddrtrn_reg_write(0, DDR_REG_BASE_DDRT + DDRT_STATUS);
	do {
		regval = ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_STATUS);
		times++;
	} while (((regval & DDRT_TEST_DONE_MASK) == 0) && (times < DDRT_WAIT_TIMEOUT));
	if (times >= DDRT_WAIT_TIMEOUT) {
		ddrtrn_fatal("DDRT wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_DDRT_TIME_OUT, base_phy, -1, -1);
		return -1;
	}
	/* DDRT_WRITE_ONLY_MODE */
	if ((mask & DDRT_TEST_MODE_MASK) == DDRT_WRITE_ONLY_MODE)
		return 0;
	/* DDRT_READ_ONLY_MODE */
	if (regval & DDRT_TEST_PASS_MASK) /* No error occurred, test pass. */
		return 0;
	else
		return -1;
}

/*
 * Get clk value.
 * Assume clk0 and clk1 is the same.
 */
int ddrtrn_hal_ac_get_clk(unsigned int base_phy)
{
	unsigned int val;
	unsigned int ac_phy_ctl;
	/* Static register have to read two times to get the right value. */
	ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	/* halft_dramclk0 */
	val = (ac_phy_ctl >> PHY_ACPHY_DRAMCLK0_BIT) & PHY_ACPHY_DRAMCLK_MASK;
	val = (val << PHY_ACPHY_DRAMCLK_EXT_BIT) |
		  ((ac_phy_ctl >> PHY_ACPHY_DCLK0_BIT) & PHY_ACPHY_DCLK_MASK);
	return val;
}

/* Set clk0 and clk1 the same value */
void ddrtrn_hal_ac_set_clk(unsigned int base_phy, unsigned int val)
{
	unsigned int ac_phy_ctl, dramclk, dclk;
	dclk = val & PHY_ACPHY_DCLK_MASK;
	dramclk = (val >> PHY_ACPHY_DRAMCLK_EXT_BIT) & PHY_ACPHY_DRAMCLK_MASK;
	/* Static register have to read two times to get the right value. */
	ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	/* clear cp1p_dclk0 */
	ac_phy_ctl &= (~(PHY_ACPHY_DCLK_MASK << PHY_ACPHY_DCLK0_BIT));
	/* clear ck2p_dclk1 */
	ac_phy_ctl &= (~(PHY_ACPHY_DCLK_MASK << PHY_ACPHY_DCLK1_BIT));
	/* clear halft_dramclk0 */
	ac_phy_ctl &= (~(PHY_ACPHY_DRAMCLK_MASK << PHY_ACPHY_DRAMCLK0_BIT));
	/* clear halft_dramclk1 */
	ac_phy_ctl &= (~(PHY_ACPHY_DRAMCLK_MASK << PHY_ACPHY_DRAMCLK1_BIT));
	ac_phy_ctl |= (dclk << PHY_ACPHY_DCLK0_BIT); /* set cp1p_dclk0 */
	ac_phy_ctl |= (dclk << PHY_ACPHY_DCLK1_BIT); /* set cp2p_dclk1 */
	/* set halft_dramclk0 */
	ac_phy_ctl |= (dramclk << PHY_ACPHY_DRAMCLK0_BIT);
	/* set halft_dramclk1 */
	ac_phy_ctl |= (dramclk << PHY_ACPHY_DRAMCLK1_BIT);
	ddrtrn_reg_write(ac_phy_ctl, base_phy + DDR_PHY_ACPHYCTL7);
}

/*
 * Get cs bdl value.
 * Assume cs0 and cs 1 is the same.
 */
int ddrtrn_hal_ac_get_cs(unsigned int base_phy)
{
	return (ddrtrn_reg_read(base_phy + DDR_PHY_ACCMDBDL2) >> 1) & PHY_BDL_MASK;
}

/* Set CS value */
void ddrtrn_hal_ac_set_cs(unsigned int base_phy, unsigned int val)
{
	unsigned int ac_cmd_bdl;
	ac_cmd_bdl = ddrtrn_reg_read(base_phy + DDR_PHY_ACCMDBDL2);
	ac_cmd_bdl &= (~(PHY_BDL_MASK << PHY_ACCMD_CS0_BIT)); /* clear cs0_bdl */
	ac_cmd_bdl &= (~(PHY_BDL_MASK << PHY_ACCMD_CS1_BIT)); /* clear cs1_bdl */
	ac_cmd_bdl |= (val << PHY_ACCMD_CS0_BIT);              /* set cs0_bdl */
	ac_cmd_bdl |= (val << PHY_ACCMD_CS1_BIT);              /* set cs1_bdl */
	ddrtrn_reg_write(ac_cmd_bdl, base_phy + DDR_PHY_ACCMDBDL2);
}
/* Check CLK value */
int ddrtrn_hal_ac_check_clk(unsigned int def_clk,
							struct ddrtrn_hal_delay *def_phase, unsigned int step)
{
	int i;
	unsigned int wdqs_phase_range, wdq_phase_range, phase_range;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	/* set new value */
	ddrtrn_hal_ac_set_clk(base_phy, def_clk + step);
	for (i = 0; i < byte_num; i++) {
		wdqs_phase_range = PHY_WDQS_PHASE_MASK -
						   ((def_phase->phase[i] >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK);
		wdq_phase_range = PHY_WDQ_PHASE_MASK -
						  ((def_phase->bdl[i] >> PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK);
		phase_range = (wdqs_phase_range < wdq_phase_range) ? wdqs_phase_range : wdq_phase_range;
		phase_range = (phase_range < step) ? phase_range : step;
		ddrtrn_reg_write(def_phase->phase[i] + (phase_range << PHY_WDQS_PHASE_BIT),
						 base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(def_phase->bdl[i] + (phase_range << PHY_WDQ_PHASE_BIT),
						 base_phy + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), i));
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
	ddrtrn_hal_ac_ddrt_test(DDRT_WRITE_ONLY_MODE, base_phy);
	/* restore default to check */
	ddrtrn_hal_ac_set_clk(base_phy, def_clk);
	for (i = 0; i < byte_num; i++) {
		ddrtrn_reg_write(def_phase->phase[i],
						 base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(def_phase->bdl[i],
						 base_phy + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), i));
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
	return ddrtrn_hal_ac_ddrt_test(DDRT_READ_ONLY_MODE, base_phy);
}

/* Find CLK difference */
int ddrtrn_hal_ac_find_clk(void)
{
	int i;
	unsigned int def_clk, step;
	struct ddrtrn_hal_delay def_phase;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	def_clk = ddrtrn_hal_ac_get_clk(base_phy);
	for (i = 0; i < byte_num; i++) {
		/* WDQS phase */
		def_phase.phase[i] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), i));
		/* WDQ phase */
		def_phase.bdl[i] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), i));
	}
	for (step = 1; step <= (PHY_ACPHY_CLK_MAX - def_clk); step++) {
		if (ddrtrn_hal_ac_check_clk(def_clk, &def_phase, step)) {
			ddrtrn_debug("PHY[%x] default clk[%x], find diff_clk[%x]", base_phy, def_clk, step);
			break;
		}
	}
	return step;
}
#endif /* DDR_AC_TRAINING_CONFIG */
