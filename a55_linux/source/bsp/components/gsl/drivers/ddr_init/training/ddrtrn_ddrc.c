/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

void ddrtrn_sref_cfg_save(struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		cfg_sref->val[i] = ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc_addr(i) + DDR_DMC_CFG_SREF);
}

/* Restore DMC_CFG_SREF config */
void ddrtrn_sref_cfg_restore(const struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		ddrtrn_hal_dmc_set_sref_cfg(i, cfg_sref->val[i]);
}

#if defined(DDR_HW_TRAINING_CONFIG) || defined(DDR_DCC_TRAINING_CONFIG)
/* DDR hw/dcc training exit or enter auto self-refresh */
int ddrtrn_training_ctrl_easr(unsigned int sref_req)
{
	int result = 0;
	unsigned int i;
	if (ddrtrn_hal_get_cur_phy_dmc_num() > DDR_DMC_PER_PHY_MAX) {
		ddrtrn_error("loop upper limit cfg->dmc_num out of range");
		return -1;
	}
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		result += ddrtrn_hal_ddrc_easr(ddrtrn_hal_get_cur_dmc_addr(i), sref_req);
	return result;
}

void ddrtrn_training_restore_timing(struct ddrtrn_hal_timing *timing)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc_addr(i), timing->val[i]);
}

static void ddrtrn_hal_timing8_trfc_ab_cfg_by_phy(struct ddr_capat_by_phy *phy_capat)
{
	unsigned int dmc_idx, dmc_num;
	unsigned int mem_width;

	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
	for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
		ddrtrn_hal_set_dmc_id(dmc_idx);
		ddrtrn_hal_set_cur_dmc(ddrtrn_hal_get_cur_dmc_addr(dmc_idx));
		mem_width = ddrtrn_hal_ddrt_get_mem_width();
		if (mem_width == MEM_WIDTH_32BIT) {
			if (phy_capat->capat_low16bit >= DDR_SIZE_2G || phy_capat->capat_high16bit >= DDR_SIZE_2G) {
				return;
			}
		} else if (phy_capat->capacity >= DDR_SIZE_2G) {
			return;
		}
		ddrtrn_hal_timing8_trfc_ab_cfg_by_dmc();
	}
}

void ddrtrn_hal_timing8_trfc_ab_cfg(struct ddr_capat_phy_all *capat_phy_all)
{
	unsigned int phy_idx, phy_num;

	phy_num = ddrtrn_hal_get_phy_num();
	for (phy_idx = 0; phy_idx < phy_num; phy_idx++) {
		ddrtrn_hal_set_phy_id(phy_idx);
		ddrtrn_hal_timing8_trfc_ab_cfg_by_phy(&capat_phy_all->phy_capat[phy_idx]);
	}
}
#endif /* DDR_HW_TRAINING_CONFIG ||  DDR_DCC_TRAINING_CONFIG */