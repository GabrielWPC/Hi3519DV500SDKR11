/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

void ddrtrn_hal_axi_save_func(struct ddrtrn_hal_training_relate_reg *relate_reg)
{
	(relate_reg)->ddrc.region_attrib[0] =
		ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1);
	(relate_reg)->ddrc.region_attrib[1] =
		ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1);
	(relate_reg)->ddrc.chsel_remap =
		ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_CHSEL_REMAP);
}

void ddrtrn_hal_axi_restore_func(const struct ddrtrn_hal_training_relate_reg *relate_reg)
{
	ddrtrn_reg_write((relate_reg)->ddrc.region_attrib[0],
					 DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1);
	ddrtrn_reg_write((relate_reg)->ddrc.region_attrib[1],
					 DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1);
	ddrtrn_reg_write((relate_reg)->ddrc.chsel_remap,
					 DDR_REG_BASE_AXI + DDR_AXI_CHSEL_REMAP);
}

void ddrtrn_hal_axi_chsel_remap_func(void)
{
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		ddrtrn_reg_write(AXI_CHSEL_REMAP_LPDDR4, DDR_REG_BASE_AXI + DDR_AXI_CHSEL_REMAP);
	else
		ddrtrn_reg_write(AXI_CHSEL_REMAP_NONLPDDR4, DDR_REG_BASE_AXI + DDR_AXI_CHSEL_REMAP);
}

void ddrtrn_hal_dmc_sfc_cmd_write(unsigned int sfc_cmd, unsigned long addr)
{
	ddrtrn_reg_write((sfc_cmd) | (1 << DMC_SFC_PRE_DIS_BIT), addr);
}

void ddrtrn_hal_dmc_sfc_bank_write(unsigned int sfc_bank, unsigned long addr)
{
	ddrtrn_reg_write((sfc_bank) | (DMC_CMD_RANK0 << DMC_SFC_RANK_BIT), addr);
}

void ddrtrn_hal_axi_switch_func(void)
{
	unsigned int ch_start = ddrtrn_hal_get_phy_id();
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
		ch_start = (ddrtrn_hal_get_phy_id() << 1) + ddrtrn_hal_get_dmc_id();
	ddrtrn_reg_write((ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1) &
					  AXI_REGION_ATTRIB_CH_MASK) |
					 AXI_RNG_ATTR_CH_MODE | ch_start,
					 DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1);
	ddrtrn_reg_write((ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1) &
					  AXI_REGION_ATTRIB_CH_MASK) |
					 AXI_RNG_ATTR_CH_MODE | ch_start,
					 DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1);
}

/* save rank0 for ddrt address */
void ddrtrn_hal_rnkvol_save_func(struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int base_dmc)
{
	relate_reg->ddrc.rnkvol = ddrtrn_reg_read(base_dmc + ddr_dmc_cfg_rnkvol(0));
}
void ddrtrn_hal_rnkvol_restore_func(const struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int base_dmc)
{
	ddrtrn_reg_write(relate_reg->ddrc.rnkvol,
					 base_dmc + ddr_dmc_cfg_rnkvol(0));
}

/* set mem_row to 0 */
void ddrtrn_hal_rnkvol_set_func(void)
{
	if (ddrtrn_hal_get_rank_id() == 1)
		ddrtrn_reg_write(ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc() + ddr_dmc_cfg_rnkvol(0)) &
						 (~DMC_RNKVOL_MEM_ROW_MASK),
						 ddrtrn_hal_get_cur_dmc() + ddr_dmc_cfg_rnkvol(0));
}
