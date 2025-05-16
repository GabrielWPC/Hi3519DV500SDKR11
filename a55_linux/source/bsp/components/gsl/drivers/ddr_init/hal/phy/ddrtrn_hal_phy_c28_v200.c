/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

void ddrtrn_hal_vref_get_host_max(unsigned int rank, int *val)
{
	if ((rank) == 0)
		(*val) = PHY_VRFTRES_HVREF_MASK;
	else
		(*val) = PHY_VRFTRES_RXDIFFCAL_MASK;
}

void ddrtrn_hal_vref_phy_host_get(unsigned long base_phy, unsigned int rank,
	unsigned int byte_index, unsigned int *val)
{
	if ((rank) == 0) {
		(*val) = ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_hvreft_status(rank, byte_index)) & PHY_VRFTRES_HVREF_MASK;
	} else {
		(*val) = (ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_hvreft_status(rank, byte_index)) >>
			PHY_VRFTRES_RXDIFFCAL_BIT) & PHY_VRFTRES_RXDIFFCAL_MASK;
	}
}

void ddrtrn_hal_vref_phy_dram_get(unsigned long base_phy, unsigned int *val, unsigned int byte_index)
{
	(*val) = ddrtrn_reg_read((base_phy) + ddrtrn_hal_phy_dvreft_status(byte_index)) &
		PHY_VRFTRES_DVREF_MASK;
}

/* PHY t28 DDR4 RDQS synchronize to RDM */
void ddrtrn_hal_phy_rdqs_sync_rdm(int val)
{
	ddrtrn_hal_rdqs_sync(val);
}
/* dqs swap */
void ddrtrn_hal_dqsswap_save_func(unsigned int *swapdfibyte_en, unsigned long base_phy)
{
	(*swapdfibyte_en) = ddrtrn_reg_read((base_phy) + DDR_PHY_DMSEL);
	ddrtrn_reg_write((*swapdfibyte_en) & PHY_DMSEL_SWAPDFIBYTE,
		(base_phy) + DDR_PHY_DMSEL);
}

void ddrtrn_hal_dqsswap_restore_func(unsigned int swapdfibyte_en, unsigned long base_phy)
{
	ddrtrn_reg_write(swapdfibyte_en, (base_phy) + DDR_PHY_DMSEL);
}

void ddrtrn_hal_phy_switch_rank(unsigned int base_phy, unsigned int val)
{
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL0) &
		(~PHY_TRAINCTRL0_MASK)) | val, base_phy + DDR_PHY_TRAINCTRL0);
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_HVRFTCTRL) & (~((unsigned int)0x1 << PHY_HRXDIFFCAL_EN_BIT))) |
		(val << PHY_HRXDIFFCAL_EN_BIT), base_phy + DDR_PHY_HVRFTCTRL);
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + ddr_phy_dxphyctrl2(0)) & (~(0x1 << PHY_DXCTL_REG_TX_PHASE_RNK_BIT))) |
		(val << PHY_DXCTL_REG_TX_PHASE_RNK_BIT), base_phy + ddr_phy_dxphyctrl2(0));
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + ddr_phy_dxphyctrl2(1)) & (~(0x1 << PHY_DXCTL_REG_TX_PHASE_RNK_BIT))) |
		(val << PHY_DXCTL_REG_TX_PHASE_RNK_BIT), base_phy + ddr_phy_dxphyctrl2(1));
	ddrtrn_hal_phy_cfg_update(base_phy);
}
