/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

#ifdef DDR_RETRAIN_CONFIG
/* update cfg_wl & cfg_rl */
static void ddrtrn_retrain_update_cfg_wl_rl(unsigned int base_phy)
{
	unsigned int ddr_phy_misc;
	unsigned int addr_delay;
	unsigned int trfc_mpc_cmd_dly;
	unsigned int tphy_wrdata;
	unsigned int dficlk_ratio_sel;
	unsigned int cfg_rl, cfg_wl;
	ddr_phy_misc = ddrtrn_hal_get_misc_val(base_phy);
	addr_delay = (ddr_phy_misc >> PHY_MISC_ADDR_DELAY_BIT) & PHY_MISC_ADDR_DELAY_MASK ;
	if (addr_delay > PHY_MISC_ADDR_DELAY_MASK)
		return;
	trfc_mpc_cmd_dly = (ddrtrn_hal_get_trfc_threshold1_val(base_phy) >>
						PHY_TRFC_MPC_CMD_DLY_BIT) & PHY_TRFC_MPC_CMD_DLY_MASK;
	if (trfc_mpc_cmd_dly > PHY_TRFC_MPC_CMD_DLY_MASK)
		return;
	tphy_wrdata = (ddrtrn_hal_get_dmsel(base_phy) >> PHY_DMSEL_TPHY_WRDATA_BIT) &
				  PHY_DMSEL_TPHY_WRDATA_MASK;
	if (tphy_wrdata > PHY_DMSEL_TPHY_WRDATA_MASK)
		return;
	dficlk_ratio_sel = (ddrtrn_hal_get_phyctrl0(base_phy) >>
						PHY_DFICLK_RATIO_BIT) & PHY_DFICLK_RATIO_MASK;
	if (dficlk_ratio_sel > PHY_DFICLK_RATIO_MASK)
		return;
	cfg_rl = (ddr_phy_misc >> PHY_MISC_CFG_RL_BIT) & PHY_MISC_CFG_RL_MASK;
	cfg_wl = (ddr_phy_misc >> PHY_MISC_CFG_WL_BIT) & PHY_MISC_CFG_WL_MASK;
	cfg_rl = cfg_rl -
			 (4 + addr_delay - trfc_mpc_cmd_dly * tphy_wrdata) * /* 4: Logical design requirements */
			 dficlk_ratio_sel * 2; /* 2: Logical design requirements */
	cfg_wl = cfg_wl -
			 (4 + addr_delay - trfc_mpc_cmd_dly * tphy_wrdata) * /* 4: Logical design requirements */
			 dficlk_ratio_sel * 2; /* 2: Logical design requirements */
	ddr_phy_misc &= ~((PHY_MISC_CFG_RL_MASK << PHY_MISC_CFG_RL_BIT) |
					  (PHY_MISC_CFG_WL_MASK << PHY_MISC_CFG_WL_BIT));
	ddr_phy_misc |= (cfg_rl << PHY_MISC_CFG_RL_BIT) | (cfg_wl << PHY_MISC_CFG_WL_BIT);
	ddrtrn_hal_set_misc_val(ddr_phy_misc, base_phy);
}

static void ddrtrn_retrain_process(unsigned int base_phy)
{
	unsigned int trfc_ctrl;
	/* enable rdqs anti-aging */
	ddrtrn_hal_enable_rdqs_anti_aging(base_phy);
	/* check trfc retrain, controlled by boot reg file */
	trfc_ctrl = ddrtrn_hal_get_trfc_ctrl_val(base_phy);
	if (trfc_ctrl == 0)
		return;
	/* update cfg_wl & cfg_rl */
	ddrtrn_retrain_update_cfg_wl_rl(base_phy);
	/* enable retrain */
	ddrtrn_hal_set_trfc_ctrl((trfc_ctrl & (~(DDR_TRFC_EN_MASK << DDR_TRFC_EN_BIT))) |
							 (0x1 << DDR_TRFC_EN_BIT), base_phy);
}

/* enable retrain and rdqs anti-aging after ddr training */
void ddrtrn_retrain_enable(void)
{
	ddrtrn_retrain_process(DDR_REG_BASE_PHY0);
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_retrain_process(DDR_REG_BASE_PHY1);
#endif
#ifdef DDR_REG_BASE_PHY2
	ddrtrn_retrain_process(DDR_REG_BASE_PHY2);
#endif
}
#else
void ddrtrn_retrain_enable(void)
{
	return;
}
#endif
