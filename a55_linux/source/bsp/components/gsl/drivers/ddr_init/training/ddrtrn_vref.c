/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_VREF_TRAINING_CONFIG
/* Get DDR Vref value */
static unsigned int ddrtrn_vref_status_get(void)
{
	unsigned int val = 0;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ) /* HOST vref */
		ddrtrn_hal_vref_phy_host_get(ddrtrn_hal_get_cur_phy(), ddrtrn_hal_get_rank_id(),
									 ddrtrn_hal_get_cur_byte(), &val);
	else /* DRAM vref */
		ddrtrn_hal_vref_phy_dram_get(ddrtrn_hal_get_cur_phy(), &val, ddrtrn_hal_get_cur_byte());
	ddrtrn_info("byte[%x] mode[%x] get vref [%x]", ddrtrn_hal_get_cur_byte(),
				ddrtrn_hal_get_cur_mode(), val);
	return val;
}

/* Get totol win number of training result */
static unsigned int ddrtrn_vref_get_win(struct ddrtrn_training_result_data *training, int vref)
{
	const int vref_min = 0;
	int vref_max = DDR_VREF_DRAM_VAL_MAX;
	unsigned int vref_set;
	training->ddrtrn_win_sum = 0;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ)
		ddrtrn_hal_vref_get_host_max(ddrtrn_hal_get_rank_id(), &vref_max);
	if (vref < vref_min)
		vref_set = vref_min;
	else if (vref > vref_max)
		vref_set = vref_max;
	else
		vref_set = vref;
	ddrtrn_hal_vref_status_set(vref_set);
	ddrtrn_dataeye_deskew(training);
	return training->ddrtrn_win_sum;
}

/* Find the best vref which win number is max */
static unsigned int ddrtrn_vref_find_best(struct ddrtrn_training_result_data *training, int vref, int step)
{
	int cur_vref;
	unsigned int best_vref;
	unsigned int cur_win;
	unsigned int max_win;
	unsigned int lower_times = 0;
	const int vref_min = 0;
	int vref_max = DDR_VREF_DRAM_VAL_MAX;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ)
		ddrtrn_hal_vref_get_host_max(ddrtrn_hal_get_rank_id(), &vref_max);
	max_win = 0;
	cur_vref = vref + step;
	if (vref < vref_min)
		best_vref = vref_min;
	else if (vref > vref_max)
		best_vref = vref_max;
	else
		best_vref = vref;
	/* find parabola vertex */
	while ((cur_vref >= vref_min) && (cur_vref <= vref_max)) {
		cur_win = ddrtrn_vref_get_win(training, cur_vref);
		ddrtrn_debug("byte[%x] vref[%x] win[%x] mode[%x]", ddrtrn_hal_get_cur_byte(), cur_vref,
					 cur_win, ddrtrn_hal_get_cur_mode());
		if (cur_win < max_win) {
			lower_times++;
			if (lower_times == DDR_VREF_COMPARE_TIMES) {
				/* Continuous decline, mean found vertex */
				break;
			}
		} else {
			lower_times = 0;
			max_win = cur_win;
			best_vref = cur_vref;
		}
		cur_vref = cur_vref + step;
	}
	return best_vref;
}

/* DDR Vref calibrate and set the best value */
static void ddrtrn_vref_cal(struct ddrtrn_training_result_data *training)
{
	unsigned int def_vref;
	unsigned int best_vref;
	unsigned int left_win;
	unsigned int right_win;
	int left_vref, right_vref;
	def_vref = ddrtrn_vref_status_get();
	left_vref = (int)def_vref - DDR_VREF_COMPARE_STEP;
	right_vref = (int)def_vref + DDR_VREF_COMPARE_STEP;
	left_win = ddrtrn_vref_get_win(training, left_vref);
	right_win = ddrtrn_vref_get_win(training, right_vref);
	ddrtrn_debug("byte[%x] default vref[%x] win[%x][%x] mode[%x]", ddrtrn_hal_get_cur_byte(),
				 def_vref, left_win, right_win, ddrtrn_hal_get_cur_mode());
	/* With vref increments, WIN number is a parabola.
		So firstly determine the result on left or right. */
	/* parabola vertex */
	if (left_win < right_win) { /* the result on right */
		best_vref = ddrtrn_vref_find_best(training, def_vref, 1);
	} else if (left_win > right_win) { /* the result on left */
		best_vref = ddrtrn_vref_find_best(training, def_vref, -1);
	} else {
		/* when (left_win == right_win), check def_vref */
		int vref_max = DDR_VREF_DRAM_VAL_MAX;
		if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ)
			ddrtrn_hal_vref_get_host_max(ddrtrn_hal_get_rank_id(), &vref_max);
		if (def_vref < ((unsigned int)vref_max >> 1))
			best_vref = ddrtrn_vref_find_best(training, def_vref, 1);
		else
			best_vref = ddrtrn_vref_find_best(training, def_vref, -1);
	}
	ddrtrn_debug("byte[%x] best vref[%x] mode[%x]", ddrtrn_hal_get_cur_byte(), best_vref,
				 ddrtrn_hal_get_cur_mode());
	ddrtrn_hal_vref_status_set(best_vref);
}

int ddrtrn_vref_training(void)
{
	struct ddrtrn_training_result_data tmp_result;
	struct ddrtrn_training_result_data *training = &tmp_result;
	struct ddrtrn_hal_trianing_dq_data dq_data;
	int result = 0;
	unsigned int i;
	ddrtrn_debug("DDR Vref[%x] training PHY[%x][%x] DMC[%x][%x] Rank[%x]",
				 ddrtrn_hal_get_cur_mode(), ddrtrn_hal_get_phy_id(), ddrtrn_hal_get_cur_phy(),
				 ddrtrn_hal_get_dmc_id(), ddrtrn_hal_get_cur_dmc(),
				 ddrtrn_hal_get_rank_id());
	ddrtrn_hal_vref_save_bdl(&dq_data);
	ddrtrn_set_data(training, 0, sizeof(struct ddrtrn_training_result_data));
	/* vref calibrate */
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ) {
		/* only training byte0 and byte2 */
		for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
			ddrtrn_hal_set_cur_byte(i + (ddrtrn_hal_get_dmc_id() << 1)); /* byte index accord to phy */
			if (ddrtrn_hal_get_cur_byte() == 1 ||
					ddrtrn_hal_get_cur_byte() == 3) /* not training byte 1 and byte 3 */
				continue;
			ddrtrn_vref_cal(training);
		}
	} else {
		unsigned int dram_type = ddrtrn_hal_get_cur_phy_dram_type();
		unsigned int bank_group = ddrtrn_hal_ddrc_get_bank_group();
		if ((dram_type != PHY_DRAMCFG_TYPE_LPDDR4) && (dram_type != PHY_DRAMCFG_TYPE_DDR4))
			return 0;
		if (dram_type == PHY_DRAMCFG_TYPE_LPDDR4)
			bank_group = DMC_CFG_MEM_2BG; /* lpddr4 not training byte1 byte3 */
		for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
			ddrtrn_hal_set_cur_byte(i + (ddrtrn_hal_get_dmc_id() << 1)); /* byte index accord to phy */
			/* byte1 and byte3 bypass when 2 Bank Group */
			if ((bank_group == DMC_CFG_MEM_2BG) && ((i == 1) || (i == 3))) /* bypass byte1 and byte3 */
				continue;
			ddrtrn_vref_cal(training);
		}
	}
#if !defined(DDR_VREF_WITHOUT_BDL_CONFIG) || defined(DDR_TRAINING_CMD)
	/* dataeye deskew again on best vref. */
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		ddrtrn_hal_set_cur_byte(i + (ddrtrn_hal_get_dmc_id() << 1)); /* byte index accord to phy */
		result += ddrtrn_dataeye_deskew(training);
	}
#endif
	ddrtrn_hal_vref_restore_bdl(&dq_data);
	ddrtrn_result_data_save(training);
	return result;
}

int ddrtrn_vref_training_func(void)
{
	struct ddrtrn_hal_training_relate_reg relate_reg;
	int result = 0;
	ddrtrn_hal_set_dq_type(DDR_CHECK_TYPE_DDRT);
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_VREF_HOST_MASK);
	ddrtrn_hal_training_switch_axi();
	ddrtrn_ddrt_init(DDR_DDRT_MODE_DATAEYE);
	/* host vref training disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_VREF_HOST_MASK) == DDR_FALSE) {
		ddrtrn_hal_set_cur_mode(DDR_MODE_READ);
		result += ddrtrn_vref_training();
	}
	/* dram vref training enable && DDR4 */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_VREF_DRAM_MASK) == DDR_FALSE) {
		ddrtrn_hal_set_cur_mode(DDR_MODE_WRITE);
		result += ddrtrn_vref_training();
	}
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_vref_training_func(void)
{
	ddrtrn_warning("Not support DDR vref training");
	return 0;
}
#endif /* DDR_VREF_TRAINING_CONFIG */
