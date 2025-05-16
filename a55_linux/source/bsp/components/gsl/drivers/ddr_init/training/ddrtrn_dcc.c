/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

/* s40/t28/t16 not support dcc training */
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#if defined(DDR_DCC_TRAINING_CONFIG) || defined(DDR_WAKEUP_CONFIG) || defined(DDR_LOW_FREQ_CONFIG)
/*
 * config ddrc exit self-refresh or exit powerdown
 * bit[3] 0x1:exit self-refresh
 * bit[3] 0x0:exit powerdown
 */
void ddrtrn_sref_cfg(struct ddrtrn_dmc_cfg_sref *cfg_sref, unsigned int value)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++) {
		cfg_sref->val[i] = ddrtrn_hal_dmc_get_sref_cfg(i);
		ddrtrn_hal_dmc_set_sref_cfg(i,
									(cfg_sref->val[i] & (~DMC_CFG_INIT_XSREF_PD_MASK)) | value);
	}
}
#endif

#ifdef DDR_DCC_TRAINING_CONFIG
/* DCC RDET training */
static int ddrtrn_dcc_dataeye_read(void)
{
	int result;
	/* 0:PHY_TRAINCTRL0_DTR_RANK0, 1:PHY_TRAINCTRL0_DTR_RANK1 */
	ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_cur_phy(), ddrtrn_hal_get_rank_id());
	result = ddrtrn_hal_hw_training_process(PHY_PHYINITCTRL_RDET_EN);
	ddrtrn_hal_dcc_rdet_enable();
	return result;
}

/* Duty Correction Control get win data */
static unsigned int ddrtrn_dcc_get_win(struct ddrtrn_dcc_data *dcc_data, int ck_index, int val_index)
{
	unsigned int win;
	unsigned int rdqsbdl_right;
	unsigned int rdqsbdl_left;
	rdqsbdl_right =
		(dcc_data->ck[ck_index].val[val_index] >> PHY_DXNRDBOUND_RIGHT_BIT) &
		PHY_DXNRDBOUND_MASK;
	rdqsbdl_left =
		(dcc_data->ck[ck_index].val[val_index] >> PHY_DXNRDBOUND_LEFT_BIT) &
		PHY_DXNRDBOUND_MASK;
	win = rdqsbdl_right - rdqsbdl_left;
	return win;
}

/* Duty Correction Control get the min win of two byte */
static int ddrtrn_dcc_get_min_win(struct ddrtrn_dcc_data *dcc_data, int ck_index)
{
	int i;
	unsigned int win_min;
	unsigned int cur_win;
	win_min = ddrtrn_dcc_get_win(dcc_data, ck_index, 0);
	for (i = 0; i < DDR_CK_RESULT_MAX; i++) {
		cur_win = ddrtrn_dcc_get_win(dcc_data, ck_index, i);
		ddrtrn_debug("CK win[%x] = [%x]", i, cur_win);
		if (cur_win < win_min)
			win_min = cur_win;
	}
	return win_min;
}

/* Duty Correction Control get ck0 min win */
static unsigned int ddrtrn_dcc_get_ck0_win(struct ddrtrn_dcc_data *dcc_data, unsigned int ck0_win_min)
{
	const int ck_index = 0;
	unsigned int byte_index;
	unsigned int ck0_win;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	for (byte_index = 0; byte_index < (byte_num / 2); /* byte_num/2:ck0 value include byte0 and byte1 */
			byte_index++)
		dcc_data->ck[ck_index].val[byte_index] =
			ddrtrn_hal_dcc_get_dxnrdbound(byte_index);
	ck0_win = ddrtrn_dcc_get_min_win(dcc_data, ck_index);
	if (ck0_win < ck0_win_min)
		ck0_win_min = ck0_win;
	return ck0_win_min;
}

/* Duty Correction Control get ck1 min win */
static unsigned int ddrtrn_dcc_get_ck1_win(struct ddrtrn_dcc_data *dcc_data, unsigned int ck1_win_min)
{
	const int ck_index = 1;
	unsigned int byte_index;
	unsigned int ck1_win;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	/* ck1 value include byte2 and byte3 */
	for (byte_index = 2; byte_index < byte_num; byte_index++)
		/* store value from byte_idex-2, one ck include two value */
		dcc_data->ck[ck_index].val[byte_index - 2] =
			ddrtrn_hal_dcc_get_dxnrdbound(byte_index);
	ck1_win = ddrtrn_dcc_get_min_win(dcc_data, ck_index);
	if (ck1_win < ck1_win_min)
		ck1_win_min = ck1_win;
	return ck1_win_min;
}

/* dcc training data init */
static void ddrtrn_dcc_data_init(struct ddrtrn_dcc_data *dcc_data)
{
	dcc_data->ck[0].win_min_ctl = 0xffffffff;
	dcc_data->ck[0].win_max_ctl = 0x0;
	dcc_data->ck[1].win_min_ctl = 0xffffffff;
	dcc_data->ck[1].win_max_ctl = 0x0;
	dcc_data->ck[0].idx_duty = 0;
	dcc_data->ck[0].idx_duty_ctl = 0;
	dcc_data->ck[0].idx_ctl = 0;
	dcc_data->ck[1].idx_duty = 0;
	dcc_data->ck[1].idx_duty_ctl = 0;
	dcc_data->ck[1].idx_ctl = 0;
	dcc_data->ck[0].bypass_ck_bit = PHY_BYPASS_CK0_BIT;
	dcc_data->ck[0].acioctl21_ctl_bit = PHY_ACIOCTL21_CTL0_BIT;
	dcc_data->ck[0].acioctl21_ck_bit = PHY_ACIOCTL21_CK0_BIT;
	dcc_data->ck[1].bypass_ck_bit = PHY_BYPASS_CK1_BIT;
	dcc_data->ck[1].acioctl21_ctl_bit = PHY_ACIOCTL21_CTL1_BIT;
	dcc_data->ck[1].acioctl21_ck_bit = PHY_ACIOCTL21_CK1_BIT;
}

/* dcc training get window by rank */
static int ddrtrn_dcc_get_win_by_rank(struct ddrtrn_dcc_data *dcc_data)
{
	unsigned int i;
	int result = 0;
	int result_rank;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	for (i = 0; i < rank_num; i++) {
		ddrtrn_debug("cur_rank = [%x]", i);
		ddrtrn_hal_set_rank_id(i);
		/* RDET */
		result_rank = ddrtrn_dcc_dataeye_read();
		if (result_rank != 0) {
			/* dcc dataeye read error */
			dcc_data->ck[0].win = 0x0;
			dcc_data->ck[1].win = 0x0;
		} else {
			/* Get win */
			dcc_data->ck[0].win = ddrtrn_dcc_get_ck0_win(dcc_data, dcc_data->ck[0].win);
			ddrtrn_debug("ck0 win = [%x]", dcc_data->ck[0].win);
			if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
				dcc_data->ck[1].win = ddrtrn_dcc_get_ck1_win(dcc_data, dcc_data->ck[1].win);
				ddrtrn_debug("ck1 win = [%x]", dcc_data->ck[1].win);
			}
		}
		/* Restore two rank bdl */
		ddrtrn_hal_restore_rdqbdl(&dcc_data->rank[ddrtrn_hal_get_rank_id()]);
		result += result_rank;
	}
	return result;
}

static void ddrtrn_dcc_get_duty(struct ddrtrn_dcc_data *dcc_data, int ck_num, unsigned int cur_duty)
{
	int ck_idx;
	for (ck_idx = 0; ck_idx < ck_num; ck_idx++) {
		if (dcc_data->ck[ck_idx].win < dcc_data->ck[ck_idx].win_min_duty)
			dcc_data->ck[ck_idx].win_min_duty = dcc_data->ck[ck_idx].win;
		if (dcc_data->ck[ck_idx].win > dcc_data->ck[ck_idx].win_max_duty) {
			dcc_data->ck[ck_idx].win_max_duty = dcc_data->ck[ck_idx].win;
			dcc_data->ck[ck_idx].idx_duty = cur_duty;
		}
		ddrtrn_debug("ck[%x] duty_win_min[%x] duty_win_max[%x] duty_index[%x]",
					 ck_idx, dcc_data->ck[ck_idx].win_min_duty,
					 dcc_data->ck[ck_idx].win_max_duty, dcc_data->ck[ck_idx].idx_duty);
	}
}

static void ddrtrn_dcc_get_ctrl(struct ddrtrn_dcc_data *dcc_data, int ck_num, unsigned int cur_ctl)
{
	int ck_idx;
	for (ck_idx = 0; ck_idx < ck_num; ck_idx++) {
		if (dcc_data->ck[ck_idx].win_min_duty < dcc_data->ck[ck_idx].win_min_ctl)
			dcc_data->ck[ck_idx].win_min_ctl = dcc_data->ck[ck_idx].win_min_duty;
		if (dcc_data->ck[ck_idx].win_max_duty >
				dcc_data->ck[ck_idx].win_max_ctl) {
			dcc_data->ck[ck_idx].win_max_ctl =
				dcc_data->ck[ck_idx].win_max_duty;
			dcc_data->ck[ck_idx].idx_duty_ctl = dcc_data->ck[ck_idx].idx_duty;
			dcc_data->ck[ck_idx].idx_ctl = cur_ctl;
		}
		ddrtrn_debug("ck[%x] win_min_ctl[%x] win_max_ctl[%x] ctl_index0[%x] "
					 "duty_ctl_idx0[%x]",
					 ck_idx, dcc_data->ck[ck_idx].win_min_ctl,
					 dcc_data->ck[ck_idx].win_max_ctl, dcc_data->ck[ck_idx].idx_ctl,
					 dcc_data->ck[ck_idx].idx_duty_ctl);
	}
}

/* Duty Correction */
static unsigned int ddrtrn_dcc_correct_duty(unsigned int cur_duty, unsigned int duty_def)
{
	unsigned int ioctl21;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		/* Correct CK0 & CK1 duty */
		ioctl21 = (duty_def & (~(PHY_ACIOCTL21_MASK << PHY_ACIOCTL21_CK0_BIT)) &
				   (~(PHY_ACIOCTL21_MASK << PHY_ACIOCTL21_CK1_BIT))) |
				  (cur_duty << PHY_ACIOCTL21_CK0_BIT) |
				  (cur_duty << PHY_ACIOCTL21_CK1_BIT);
		ddrtrn_hal_dcc_set_ioctl21(ioctl21);
	} else {
		/* Correct CK0 duty */
		ioctl21 = (duty_def & (~(PHY_ACIOCTL21_MASK << PHY_ACIOCTL21_CK0_BIT))) |
				  (cur_duty << PHY_ACIOCTL21_CK0_BIT);
		ddrtrn_hal_dcc_set_ioctl21(ioctl21);
	}
	return ioctl21;
}

/* Duty direction control */
static unsigned int ddrtrn_dcc_ck_ctl(unsigned int ioctl21_def, unsigned int ctl_index)
{
	unsigned int ioctl21;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		ioctl21 = (ioctl21_def & (~(1 << PHY_ACIOCTL21_CTL0_BIT)) &
				   (~(1 << PHY_ACIOCTL21_CTL1_BIT))) |
				  (ctl_index << PHY_ACIOCTL21_CTL0_BIT) |
				  (ctl_index << PHY_ACIOCTL21_CTL1_BIT);
		ddrtrn_hal_dcc_set_ioctl21(ioctl21);
	} else {
		ioctl21 = (ioctl21_def & (~(1 << PHY_ACIOCTL21_CTL0_BIT))) |
				  (ctl_index << PHY_ACIOCTL21_CTL0_BIT);
		ddrtrn_hal_dcc_set_ioctl21(ioctl21);
	}
	return ioctl21;
}

static int ddrtrn_dcc_process(struct ddrtrn_dcc_data *dcc_data, int ck_num, unsigned int ioctl21_def)
{
	unsigned int cur_ctl;
	unsigned int cur_duty;
	for (cur_ctl = 0; cur_ctl < DDR_DUTY_CTL_NUM; cur_ctl++) {
		dcc_data->ck[0].win_min_duty = 0xffffffff;
		dcc_data->ck[0].win_max_duty = 0x0;
		if (ck_num > 1) {
			dcc_data->ck[1].win_min_duty = 0xffffffff;
			dcc_data->ck[1].win_max_duty = 0x0;
		}
		ddrtrn_debug("cur_ctl = [%x]", cur_ctl);
		if (ddrtrn_training_ctrl_easr(DDR_ENTER_SREF))
			return -1;
		/* Correct CK duty dirrection control */
		dcc_data->ioctl21_tmp = ddrtrn_dcc_ck_ctl(ioctl21_def, cur_ctl);
		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;
		for (cur_duty = 0; cur_duty < DDR_DUTY_NUM;
				cur_duty += PHY_AC_IOCTL21_STEP) {
			dcc_data->ck[0].win = 0xffffffff;
			if (ck_num > 1)
				dcc_data->ck[1].win = 0xffffffff;
			ddrtrn_debug("cur_duty = [%x]", cur_duty);
			/* Correct ck0 and ck1 duty */
			if (ddrtrn_training_ctrl_easr(DDR_ENTER_SREF))
				return -1;
			dcc_data->ioctl21_tmp = ddrtrn_dcc_correct_duty(cur_duty,
															dcc_data->ioctl21_tmp);
			if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
				return -1;
			ddrtrn_debug("Cur ACIOCTL21[%x]", dcc_data->ioctl21_tmp);
			if (ddrtrn_dcc_get_win_by_rank(dcc_data) != 0)
				break;
			ddrtrn_dcc_get_duty(dcc_data, ck_num, cur_duty);
		}
		ddrtrn_dcc_get_ctrl(dcc_data, ck_num, cur_ctl);
	}
	return 0;
}

/* ddr dcc training compare result */
static void ddrtrn_dcc_compare_result(struct ddrtrn_dcc_data *dcc_data, int ck_num,
									  unsigned int gated_bypass_def, unsigned int ioctl21_def)
{
	int ck_idx;
	for (ck_idx = 0; ck_idx < ck_num; ck_idx++) {
		/* Config ck0 duty */
		if (dcc_data->ck[ck_idx].win_max_ctl -
				dcc_data->ck[ck_idx].win_min_ctl <=
				DDR_DCC_CTL_WIN_DIFF) {
			dcc_data->ck[ck_idx].def_bp =
				(gated_bypass_def >> dcc_data->ck[ck_idx].bypass_ck_bit) & 0x1;
			dcc_data->ck[ck_idx].def_ctl =
				(ioctl21_def >> dcc_data->ck[ck_idx].acioctl21_ctl_bit) & 0x1;
			dcc_data->ck[ck_idx].def_duty =
				(ioctl21_def >> dcc_data->ck[ck_idx].acioctl21_ck_bit) & PHY_ACIOCTL21_MASK;
			gated_bypass_def = (gated_bypass_def & (~(1 << dcc_data->ck[ck_idx].bypass_ck_bit))) |
							   (dcc_data->ck[ck_idx].def_bp << dcc_data->ck[ck_idx].bypass_ck_bit);
			ddrtrn_hal_dcc_set_gated_bypass(gated_bypass_def);
			ioctl21_def = (ioctl21_def & (~(1 << dcc_data->ck[ck_idx].acioctl21_ctl_bit)) &
						   (~(PHY_ACIOCTL21_MASK << dcc_data->ck[ck_idx].acioctl21_ck_bit))) |
						  (dcc_data->ck[ck_idx].def_ctl << dcc_data->ck[ck_idx].acioctl21_ctl_bit) |
						  (dcc_data->ck[ck_idx].def_duty << dcc_data->ck[ck_idx].acioctl21_ck_bit);
			ddrtrn_hal_dcc_set_ioctl21(ioctl21_def);
			ddrtrn_debug("ck[%x] Final AC_GATED_BYPASS[%x]", ck_idx, gated_bypass_def);
			ddrtrn_debug("ck[%x] Final ACIOCTL21[%x]", ck_idx, ioctl21_def);
		} else {
			ioctl21_def = (ioctl21_def & (~(1 << dcc_data->ck[ck_idx].acioctl21_ctl_bit)) &
						   (~(PHY_ACIOCTL21_MASK << dcc_data->ck[ck_idx].acioctl21_ck_bit))) |
						  (dcc_data->ck[ck_idx].idx_ctl << dcc_data->ck[ck_idx].acioctl21_ctl_bit) |
						  (dcc_data->ck[ck_idx].idx_duty_ctl << dcc_data->ck[ck_idx].acioctl21_ck_bit);
			ddrtrn_hal_dcc_set_ioctl21(ioctl21_def);
			ddrtrn_debug("ck[%x] Final ACIOCTL21[%x]", ck_idx, ioctl21_def);
		}
	}
}

static int ddrtrn_dcc_get_best_duty(struct ddrtrn_dmc_cfg_sref *cfg_sref, struct ddrtrn_dcc_data *dcc_data)
{
	int ck_num;
	unsigned int ioctl21_def;
	unsigned int gated_bypass_def, gated_bypass_temp;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		ck_num = DDR_CK_NUM_LPDDR4; /* lpddr4: 2 ck */
	else
		ck_num = DDR_CK_NUM_NONLPDDR4; /* other: 1 ck */
	ddrtrn_dcc_data_init(dcc_data);
	/* Save ck duty default config. Read two times to get the right static
	 * register value. */
	gated_bypass_def = ddrtrn_hal_dcc_get_gated_bypass();
	gated_bypass_def = ddrtrn_hal_dcc_get_gated_bypass();
	ioctl21_def = ddrtrn_hal_dcc_get_ioctl21();
	ioctl21_def = ddrtrn_hal_dcc_get_ioctl21();
	ddrtrn_debug("gated_bypass_def[%x] ioctl21_def[%x]", gated_bypass_def, ioctl21_def);
	/* DCC training exit self-refresa enter powerdown. */
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		ddrtrn_sref_cfg(cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD);
	/* DDR dcc training enter auto self-refresh. */
	if (ddrtrn_training_ctrl_easr(DDR_ENTER_SREF))
		return -1;
	/* Enable ck0 & ck1 duty. */
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		gated_bypass_temp =
			gated_bypass_def | PHY_CK1_IOCTL_DUTY_EN | PHY_CK_IOCTL_DUTY_EN;
		ddrtrn_hal_dcc_set_gated_bypass(gated_bypass_temp);
	} else {
		gated_bypass_temp = gated_bypass_def | PHY_CK_IOCTL_DUTY_EN;
		ddrtrn_hal_dcc_set_gated_bypass(gated_bypass_temp);
	}
	ddrtrn_debug("Cur GATED_BYPASS[%x]", gated_bypass_temp);
	if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
		return -1;
	if (ddrtrn_dcc_process(dcc_data, ck_num, ioctl21_def))
		return -1;
	/* Config ck duty */
	/* DCC training exit self-refresa enter powerdown. */
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		ddrtrn_sref_cfg(cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD);
	/* DDR dcc training enter auto self-refresh. */
	if (ddrtrn_training_ctrl_easr(DDR_ENTER_SREF))
		return -1;
	/* DDR dcc training compare result. */
	ddrtrn_dcc_compare_result(dcc_data, ck_num, gated_bypass_def, ioctl21_def);
	/* DDR dcc training exit auto self-refresh. */
	if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
		return -1;
	return 0;
}

#ifdef DDR_TRAINING_DEBUG
void ddrtrn_training_break_point(const char *name)
{
	ddrtrn_info(name);
	ddrtrn_training_console_if();
}
#else
void ddrtrn_training_break_point(__attribute__((unused)) const char *name)
{
}
#endif

static int ddrtrn_dcc_training(void)
{
	unsigned int i;
	int result = 0;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	struct ddrtrn_dmc_cfg_sref cfg_sref;
	struct ddrtrn_hal_timing timing;
	struct ddrtrn_dcc_data dcc;
	struct ddrtrn_dcc_data *dcc_data = &dcc;
	ddrtrn_debug("dram_type[%x]", ddrtrn_hal_get_cur_phy_dram_type());
	ddrtrn_debug("rank num[%x]", rank_num);
	/* Save two rank DERT default result: rdq/rdqs/rdm/ bdl */
	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_save_rdqbdl(&dcc_data->rank[ddrtrn_hal_get_rank_id()]);
	}
	/* Disable auto refresh */
	ddrtrn_hal_save_timing(&timing);
	/* Duty Correction Control training. */
	result += ddrtrn_dcc_get_best_duty(&cfg_sref, dcc_data);
	/* Do DERT training again */
	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		dcc_data->item[i] = ddrtrn_hal_get_cur_rank_item_hw(i);
		ddrtrn_hal_set_cur_rank_item_hw(i, PHY_PHYINITCTRL_HVREFT_EN);
		ddrtrn_debug("item_hw[%x]=[%x]", i,
					 ddrtrn_hal_get_cur_rank_item_hw(i));
	}
	result += ddrtrn_hw_training_by_phy();
	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_set_cur_rank_item_hw(i, dcc_data->item[i]);
	}
	/* Enable auto refresh */
	ddrtrn_training_restore_timing(&timing);
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		/* DCC restore DMC_CFG_SREF config. */
		ddrtrn_sref_cfg_restore(&cfg_sref);
	return result;
}

int ddrtrn_dcc_training_func(void)
{
	unsigned int i;
	int result = 0;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		ddrtrn_hal_set_cur_item(ddrtrn_hal_get_rank_item(i, 0));
		if (ddrtrn_hal_check_bypass(1 << (ddrtrn_hal_get_phy_id())) != DDR_FALSE)
			continue;
		/* dpmc training disable */
		if (ddrtrn_hal_check_bypass(DDR_BYPASS_DCC_MASK) == DDR_FALSE)
			result += ddrtrn_dcc_training();
	}
	return result;
}
#else
int ddrtrn_dcc_training_func(void)
{
	ddrtrn_warning("Not support DCC training");
	return 0;
}
#endif /* DDR_DCC_TRAINING_CONFIG */
