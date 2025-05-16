/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

#ifdef DDR_AUTO_PD_CONFIG
/* ddr DMC auto power down config
 * the value should config after trainning, or it will cause chip compatibility problems
 */
void ddrtrn_dmc_auto_power_down_cfg(void)
{
	ddrtrn_hal_dmc_auto_pd_by_phy(DDR_REG_BASE_PHY0, DDR_REG_BASE_DMC0, DDR_REG_BASE_DMC1);
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_hal_dmc_auto_pd_by_phy(DDR_REG_BASE_PHY1, DDR_REG_BASE_DMC2, DDR_REG_BASE_DMC3);
#endif
#ifdef DDR_REG_BASE_PHY2
	ddrtrn_hal_dmc_auto_pd_by_phy(DDR_REG_BASE_PHY2, DDR_REG_BASE_DMC4, DDR_REG_BASE_DMC5);
#endif
}
#else
void ddrtrn_dmc_auto_power_down_cfg(void)
{
	return;
}
#endif /* DDR_AUTO_PD_CONFIG */