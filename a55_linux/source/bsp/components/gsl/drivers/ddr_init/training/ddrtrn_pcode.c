/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"

#ifdef DDR_PCODE_TRAINING_CONFIG

int ddrtrn_pcode_trainig_by_phy(void)
{
	unsigned int times;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	int osc_cnt_rdata;
	int ddrtrn_freq;
	int pcode_value;
	/* test start */
	times = ddrtrn_hal_pcode_trainig_start();
	if (times >= DDRT_PCODE_WAIT_TIMEOUT) {
		ddrtrn_fatal("IO pcode training wait timeout");
		return -1;
	}
	osc_cnt_rdata = ddrtrn_hal_pcode_trainig_get_osc_cnt_rdata();
	ddrtrn_hal_pcode_trainig_stop();
	ddrtrn_freq = ddrtrn_hal_pcode_get_cksel();
	/* get pcode value */
	pcode_value =
		(490960 - (89 * osc_cnt_rdata * ddrtrn_freq) / 300) / 10000; /* y equal (490960 - (89*x*fre)/300)/10000 */
	ddrtrn_debug("pcode value[%x]", pcode_value);
	if (pcode_value < PHY_PCODE_MIN)
		pcode_value = PHY_PCODE_MIN;
	else if (pcode_value > PHY_PCODE_MAX)
		pcode_value = PHY_PCODE_MAX;
	/* set pcode value */
	ddrtrn_hal_pcode_set_value(base_phy, pcode_value);
	return 0;
}

int ddrtrn_pcode_training(void)
{
	struct ddrtrn_hal_training_relate_reg relate_reg;
	int result = 0;
	int i;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		ddrtrn_hal_set_cur_item(ddrtrn_hal_get_rank_item(i, 0));
		if (ddrtrn_hal_check_bypass(1 << (ddrtrn_hal_get_phy_id())) != DDR_FALSE)
			continue;
		/* pcode training disable */
		if (ddrtrn_hal_check_bypass(DDR_BYPASS_PCODE_MASK) != DDR_FALSE)
			continue;
		ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_PCODE_MASK);
		result += ddrtrn_pcode_trainig_by_phy();
		ddrtrn_hal_restore_reg(&relate_reg);
	}
	return result;
}

int ddrtrn_pcode_training_func(void)
{
	ddrtrn_hal_cfg_init();
	return ddrtrn_pcode_training();
}
#else
int ddrtrn_pcode_training(void)
{
	ddrtrn_warning("Not support DDR pcode training");
	return 0;
}

int ddrtrn_pcode_training_func(void)
{
	ddrtrn_warning("Not support DDR pcode training");
	return 0;
}
#endif
