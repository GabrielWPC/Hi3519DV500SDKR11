/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_HW_TRAINING_CONFIG
static void ddrtrn_hw_training_adjust_rdqs(struct ddrtrn_rdqs_data *rdqs)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int cur_rank = ddrtrn_hal_get_rank_id();
	int offset;
	for (i = 0; i < byte_num; i++) {
		offset = ddrtrn_hal_hw_restore_rdqsbdl(rdqs->rank[0].bdl[i], rdqs->rank[1].bdl[i], i);
		ddrtrn_hal_rdqs_sync_rank_rdq(offset);
	}
	ddrtrn_hal_set_rank_id(cur_rank); /* restore to current rank */
	ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
}

/* Dataeye hardware training. */
int ddrtrn_hw_dataeye_read(void)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int i;
	int result;
	ddrtrn_hal_cfg_init();
	/* clear */
	for (i = 0; i < byte_num; i++)
		ddrtrn_hal_hw_clear_rdq(i);
	ddrtrn_hal_phy_cfg_update(base_phy);
	result = ddrtrn_hal_hw_training_process(PHY_PHYINITCTRL_RDET_EN);
	ddrtrn_hal_hw_read_adj();
	return result;
}

/* DDR HW training control */
static int ddrtrn_hw_training_ctl(struct ddrtrn_rdqs_data *rdqs)
{
	int result = 0;
	unsigned int byte_idx;
	unsigned int item = ddrtrn_hal_get_cur_item();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int ddrtrn_temp;
	struct ddrtrn_hal_bdl_dly bdl_dly_s;
	if (item == 0 || (rdqs == NULL))
		return 0;
	ddrtrn_hal_phy_cfg_update(base_phy);
	/* NOTE: not support array when boot */
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_CNT_RESET_START);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_PLL);
	if ((item & PHY_PHYINITCTRL_PLL_INIT_EN) != 0)
		ddrtrn_hal_ck_cfg(base_phy);
#ifdef DDR_OE_CONFIG
	if ((item & PHY_PHYINITCTRL_ZCAL_EN) != 0) {
		ddrtrn_ac_oe_enable();
		ddrtrn_dummy_io_oe_enable();
	}
#endif
	/* rdqs value add offset */
#ifdef DDR_RDQS_OFFSET_CONFIG
	if (ddrtrn_hal_get_cur_item() & PHY_PHYINITCTRL_DLYMEAS_EN)
		ddrtrn_hal_hw_rdqs_offset_cfg(base_phy, byte_num, ddrtrn_hal_get_cur_phy_dram_type());
#endif
	/* save rdqs bdl after PHY_PHYINITCTRL_DLYMEAS_EN */
	if (ddrtrn_hal_get_rank_id() == 0) {
		ddrtrn_hal_training_get_rdqs(&rdqs->origin);
		ddrtrn_hal_hw_save_rdqsbdl();
	}
	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_hal_set_cur_byte(byte_idx);
		ddrtrn_hal_rdqbdl_adj(&bdl_dly_s);
	}
	result += ddrtrn_hal_hw_dataeye_adapt(&ddrtrn_temp);
	result += ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_CAT_EN);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_CS);
	result += ddrtrn_hal_hw_dataeye_vref_set();
	result += ddrtrn_hal_hw_training_normal_conf();
#ifdef DDR_WRITE_DM_DISABLE
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_DDR4)
		result += ddrtrn_hal_hw_restore_write_dm(ddrtrn_temp);
#endif
	ddrtrn_hal_phy_cfg_update(base_phy);
	return result;
}

static int ddrtrn_hw_training_by_rank(struct ddrtrn_rdqs_data *rdqs)
{
	ddrtrn_debug("PHY[%x][%x] Rank[%x] itme[%x]", ddrtrn_hal_get_phy_id(), ddrtrn_hal_get_cur_phy(),
				 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_item());
	/* 0:PHY_TRAINCTRL0_DTR_RANK0, 1:PHY_TRAINCTRL0_DTR_RANK1 */
	ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_cur_phy(), ddrtrn_hal_get_rank_id());
	return ddrtrn_hw_training_ctl(rdqs);
}

int ddrtrn_hw_training_by_phy(void)
{
	int result = 0;
	unsigned int i;
	struct ddrtrn_rdqs_data rdqs;
	struct ddrtrn_hal_timing timing;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	/* disable auto refresh */
	ddrtrn_hal_save_timing(&timing);
	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_set_cur_item(ddrtrn_hal_get_cur_rank_item_hw(i));
		result += ddrtrn_hw_training_by_rank(&rdqs);
		if (rank_num != DDR_SUPPORT_RANK_MAX)
			break;
		/* save rank rdqs bdl */
		ddrtrn_hal_training_get_rdqs(&rdqs.rank[i]);
		/* restore PHY_PHYINITCTRL_DLYMEAS_EN rdqs before training next rank */
		if ((rank_num - 1) != i)
			ddrtrn_hal_training_set_rdqs(&rdqs.origin);
	}
	if (rank_num == DDR_SUPPORT_RANK_MAX) {
		ddrtrn_hw_training_adjust_rdqs(&rdqs);
		ddrtrn_hal_training_adjust_wdq();
		ddrtrn_hal_training_adjust_wdqs();
		ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_cur_phy(), 0x0); /* switch to rank0 */
	}
	/* restore auto refresh */
	ddrtrn_training_restore_timing(&timing);
	return result;
}

/* DDR hardware training */
int ddrtrn_hw_training(void)
{
	int result = 0;
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		result += ddrtrn_hw_training_by_phy();
	}
	return result;
}
#endif /* DDR_HW_TRAINING_CONFIG */
