/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

unsigned int ddrtrn_hal_get_gt_status(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS) & PHY_INITSTATUS_GT_MASK;
}

unsigned int ddrtrn_hal_get_trfc_ctrl_val(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_TRFC_CTRL);
}

void ddrtrn_hal_set_trfc_ctrl(unsigned int trfc_ctrl, unsigned int base_phy)
{
	ddrtrn_reg_write(trfc_ctrl, base_phy + DDR_PHY_TRFC_CTRL);
}

unsigned int ddrtrn_hal_get_misc_val(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_MISC);
}

void ddrtrn_hal_set_misc_val(unsigned int misc_val, unsigned int base_phy)
{
	ddrtrn_reg_write(misc_val, base_phy + DDR_PHY_MISC);
}

unsigned int ddrtrn_hal_get_trfc_threshold1_val(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_TRFC_THRESHOLD1);
}

unsigned int ddrtrn_hal_get_dmsel(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_DMSEL);
}

unsigned int ddrtrn_hal_get_phyctrl0(unsigned int base_phy)
{
	return ddrtrn_reg_read(base_phy + DDR_PHY_PHYCTRL0);
}

/* Some special circumstances need read register repeatedly */
unsigned int ddrtrn_hal_read_repeatedly(unsigned int base_addr, unsigned int offset_addr)
{
	unsigned int val;
	val = ddrtrn_reg_read(base_addr + offset_addr);
	val = ddrtrn_reg_read(base_addr + offset_addr);
	val = ddrtrn_reg_read(base_addr + offset_addr);
	return val;
}

/* enable rdqs anti-aging */
void ddrtrn_hal_enable_rdqs_anti_aging(unsigned int base_phy)
{
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_PHYRSCTRL) &
					  (~(0x1 << DDR_CFG_RX_AGE_COMPST_EN_BIT))) |
					 (0x1 << DDR_CFG_RX_AGE_COMPST_EN_BIT),
					 base_phy + DDR_PHY_PHYRSCTRL);
}
