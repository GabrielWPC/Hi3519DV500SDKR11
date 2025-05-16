/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

void ddrtrn_hal_dmc_auto_pd_by_phy(unsigned int base_phy, unsigned int dmc0, unsigned int dmc1)
{
	unsigned int dmc_pd0;
	unsigned int pd_prd;
	dmc_pd0 = ddrtrn_reg_read(dmc0 + DDR_DMC_CFG_PD);
	pd_prd = (dmc_pd0 >> DMC_CFG_PD_PRD_BIT) & DMC_CFG_PD_PRD_MASK;
	if (pd_prd == 0)
		return;
	if ((ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG) &
			PHY_DRAMCFG_TYPE_MASK) == PHY_DRAMCFG_TYPE_LPDDR4) {
		ddrtrn_reg_write(dmc_pd0 | (0x1 << DMC_CFG_PD_EN_BIT), dmc0 + DDR_DMC_CFG_PD);
		ddrtrn_reg_write(ddrtrn_reg_read(dmc1 + DDR_DMC_CFG_PD) | (0x1 << DMC_CFG_PD_EN_BIT), dmc1 + DDR_DMC_CFG_PD);
	} else {
		ddrtrn_reg_write(dmc_pd0 | (0x1 << DMC_CFG_PD_EN_BIT), dmc0 + DDR_DMC_CFG_PD);
	}
}

