/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"

#ifdef DDR_AC_TRAINING_CONFIG

/* Check CS value */
int ddrtrn_ac_check_cs(unsigned int base_phy, unsigned int def_cs, unsigned int step)
{
	ddrtrn_hal_ac_set_cs(base_phy, def_cs + step);
	ddrtrn_hal_phy_cfg_update(base_phy);
	ddrtrn_hal_ac_ddrt_test(DDRT_WRITE_ONLY_MODE, base_phy);
	ddrtrn_hal_ac_set_cs(base_phy, def_cs); /* restore default to check */
	ddrtrn_hal_phy_cfg_update(base_phy);
	return ddrtrn_hal_ac_ddrt_test(DDRT_READ_ONLY_MODE, base_phy);
}

/* Find CS difference */
int ddrtrn_ac_find_cs(unsigned int base_phy)
{
	unsigned int def_cs, step;
	def_cs = ddrtrn_hal_ac_get_cs(base_phy);
	for (step = 1; step <= (PHY_BDL_MASK - def_cs); step++) {
		if (ddrtrn_ac_check_cs(base_phy, def_cs, step)) {
			ddrtrn_debug("PHY[%x] default cs[%x], find diff_cs[%x]", base_phy, def_cs, step);
			break;
		}
	}
	return step;
}

/* DDR AC training */
int ddrtrn_ac_training(void)
{
	int i;
	unsigned int diff_cs, diff_clk;
	unsigned int clk_phase, cs_bdl, phase_tmp;
	unsigned int byte_num;
	unsigned int def_clk, def_cs;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	ddrtrn_debug("DDR AC training");
	byte_num = ddrtrn_hal_get_byte_num();
	diff_cs = ddrtrn_ac_find_cs(base_phy); /* setup time(bdl) */
	diff_clk = ddrtrn_hal_ac_find_clk(); /* hold time(phase) */
	/* cs bdl transform to clk phase */
	phase_tmp = diff_cs >> DDR_BDL_PHASE_REL;
	if (diff_clk > phase_tmp) {
		clk_phase = (diff_clk - phase_tmp) >> 1;
		def_clk = ddrtrn_hal_ac_get_clk(base_phy);
		/* set new value */
		ddrtrn_hal_ac_set_clk(base_phy, def_clk + clk_phase);
		for (i = 0; i < byte_num; i++)
			ddrtrn_hal_ac_restore_value(i, clk_phase);
		ddrtrn_debug("PHY[%x] def clk[%x] add phase[%x]", base_phy, def_clk, clk_phase);
	} else {
		def_cs = ddrtrn_hal_ac_get_cs(base_phy);
		cs_bdl = 0;
		if (diff_cs > (diff_clk << DDR_BDL_PHASE_REL))
			cs_bdl = diff_cs - (diff_clk << DDR_BDL_PHASE_REL);
		ddrtrn_hal_ac_set_cs(base_phy, def_cs + cs_bdl);
		ddrtrn_debug("PHY[%x] def cs[%x] add bdl[%x]", base_phy, def_cs, cs_bdl);
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
	return 0;
}

int ddrtrn_ac_training_func(void)
{
	int result = 0;
	struct ddrtrn_hal_training_relate_reg relate_reg;
	/* AC training disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_AC_MASK) != DDR_FALSE)
		return 0;
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_AC_MASK);
	ddrtrn_hal_training_switch_axi();
	ddrtrn_ddrt_init(DDR_DDRT_MODE_DATAEYE);
	result += ddrtrn_ac_training();
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_ac_training_func(void)
{
	ddrtrn_warning("Not support DDR AC training");
	return 0;
}
#endif /* DDR_AC_TRAINING_CONFIG */
