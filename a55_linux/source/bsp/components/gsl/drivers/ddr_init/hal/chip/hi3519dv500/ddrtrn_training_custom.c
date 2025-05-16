/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

/*
 * Do some prepare before copy code from DDR to SRAM.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_prepare_copy(void)
{
	return;
}

/*
 * Save site before DDR training command execute .
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_save(void)
{
	return;
}

/*
 * Restore site after DDR training command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_restore(void)
{
	return;
}

static void ddrtrn_hal_anti_aging_disable(struct ddrtrn_hal_training_custom_reg *custom_reg,
										  unsigned int base_phy, unsigned int phy_idx)
{
	/* disable rdqs anti-aging */
	custom_reg->age_compst_en[phy_idx] =
		ddrtrn_reg_read(base_phy + DDR_PHY_PHYRSCTRL);
	ddrtrn_reg_write(custom_reg->age_compst_en[phy_idx] &
					 (~((unsigned int)0x1 << DDR_CFG_RX_AGE_COMPST_EN_BIT)),
					 base_phy + DDR_PHY_PHYRSCTRL);
}

static int ddrtrn_hal_disable_retrain(struct ddrtrn_hal_training_custom_reg *custom_reg,
									  unsigned int base_phy, unsigned int phy_idx)
{
	unsigned int cnt = DDR_LOOP_COUNT;
	custom_reg->trfc_en[phy_idx] =
		ddrtrn_reg_read(base_phy + DDR_PHY_TRFC_CTRL);
	ddrtrn_reg_write(custom_reg->trfc_en[phy_idx] &
					 (~(DDR_TRFC_EN_MASK << DDR_TRFC_EN_BIT)),
					 base_phy + DDR_PHY_TRFC_CTRL);
	while (cnt--) {
		if (((ddrtrn_reg_read(base_phy + DDR_PHY_TRFC_CTRL) >> DDR_TRFC_EN_BIT) &
				DDR_TRFC_EN_MASK) == 0)
			break;
	}
	if (cnt == DDR_LOOP_COUNT)
		return -1;
	return 0;
}

/*
 * Save site before DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	int result;
	if (custom_reg == NULL)
		return -1;
	custom_reg->ddrt_ctrl = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	ddrtrn_reg_write(custom_reg->ddrt_ctrl | (0x1 << DDRT_MST_SEL_BIT), DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	/* turn on ddrt clock */
	custom_reg->ddrt_clk_reg = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT);
	/* enable ddrt0 clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg | (0x1 << DDR_TEST0_CKEN_BIT), CRG_REG_BASE_ADDR + CRG_DDRT);
	ddr_asm_nop();
	/* disable ddrt0 soft reset */
	ddrtrn_reg_write(ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT) & (~(0x1 << DDR_TEST0_SRST_REQ_BIT)),
					 CRG_REG_BASE_ADDR + CRG_DDRT);
	/* disable rdqs anti-aging */
	ddrtrn_hal_anti_aging_disable(custom_reg, DDR_REG_BASE_PHY0, 0); /* 0: phy0 */
	/* disable retrain */
	result = ddrtrn_hal_disable_retrain(custom_reg, DDR_REG_BASE_PHY0, 0); /* 0: phy0 */
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_hal_anti_aging_disable(custom_reg, DDR_REG_BASE_PHY1, 1); /* 1: phy1 */
	result += ddrtrn_hal_disable_retrain(custom_reg, DDR_REG_BASE_PHY1, 1); /* 1: phy1 */
#endif
	return result;
}

/*
 * Restore site after DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	if (custom_reg == NULL)
		return;
	ddrtrn_reg_write(custom_reg->ddrt_ctrl, DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	/* restore ddrt clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg, CRG_REG_BASE_ADDR + CRG_DDRT);
	/* restore rdqs anti-aging */
	ddrtrn_reg_write(custom_reg->age_compst_en[0], DDR_REG_BASE_PHY0 + DDR_PHY_PHYRSCTRL);
	/* restore retrain */
	ddrtrn_reg_write(custom_reg->trfc_en[0], DDR_REG_BASE_PHY0 + DDR_PHY_TRFC_CTRL);
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_reg_write(custom_reg->age_compst_en[1], DDR_REG_BASE_PHY1 + DDR_PHY_PHYRSCTRL);
	ddrtrn_reg_write(custom_reg->trfc_en[1], DDR_REG_BASE_PHY1 + DDR_PHY_TRFC_CTRL);
#endif
}

int ddrtrn_hw_training_init(unsigned int hw_item_mask, int low_freq_flag)
{
	int result;
	unsigned int dram_type;
	struct ddrtrn_hal_training_custom_reg reg;
	ddrtrn_set_data(&reg, 0, sizeof(struct ddrtrn_hal_training_custom_reg));
	dram_type = ddrtrn_reg_read(DDR_REG_BASE_PHY0 + DDR_PHY_DRAMCFG) & PHY_DRAMCFG_TYPE_MASK;
	ddrtrn_hal_cfg_init();
	if (ddrtrn_hal_boot_cmd_save(&reg) != 0)
		return -1;
	if ((low_freq_flag == DDR_LOW_FREQ_START_TRUE) && (dram_type == PHY_DRAMCFG_TYPE_LPDDR4)) {
		result = ddrtrn_low_freq_start(hw_item_mask);
	} else {
		ddrtrn_hal_hw_item_cfg(hw_item_mask);
		result = ddrtrn_hw_training_if();
	}
	ddrtrn_hal_boot_cmd_restore(&reg);
	return result;
}
