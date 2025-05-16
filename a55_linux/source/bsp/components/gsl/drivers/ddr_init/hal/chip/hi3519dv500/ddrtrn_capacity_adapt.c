/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_capacity_adapt.h"
#include "ddrtrn_get_ddr_size.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_CAPAT_ADAPT_CFG
static struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy0_lpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050050},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy0_nonlpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050088},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy1_lpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050052},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy1_nonlpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050089},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

/* DDR4 1rank 16bit 512MB,1GB,2GB */
static struct ddrtrn_reg_val_conf ddr4_2g_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x40050088}, /* single channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x00ff0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
};

/* DDR4 1rank 32bit 1.5GB:512MB,1GB */
static struct ddrtrn_reg_val_conf ddr4_1p5g_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x30050088}, /* single channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x003f0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x1700}, /* enable Region1 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1, 0x30040088}, /* PHY0 single channel */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP2, 0x001f0080}, /* start address:2GB, total capacity:1GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB2, 0x40}, /* Channel offset: 1GB */
	{DDR_REG_BASE_DMC0 + DDR_DMC_TEST_GENPOSE0, 0x0004480d},
};

/* DDR4 1rank 32bit 3GB:1GB,2GB */
static struct ddrtrn_reg_val_conf ddr4_3g_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x30050088}, /* single channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x007f0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x1700}, /* enable Region1 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1, 0x30040088}, /* PHY0 single channel */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP2, 0x003f00c0}, /* start address:2GB, total capacity:1GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB2, 0x80}, /* Channel offset: 1GB */
	{DDR_REG_BASE_DMC0 + DDR_DMC_TEST_GENPOSE0, 0x0004480f},
};

/* DDR4 1rank 32bit 512MB:256MB,256MB; 1GB:512MB,512MB; 2GB:1GB,1GB; 4GB:2GB,2GB */
static struct ddrtrn_reg_val_conf ddr4_4g_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x30050088}, /* single channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x00ff0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
};

/* LPDDR4 1rank 512MB,1GB,2GB,4GB */
static struct ddrtrn_reg_val_conf lpddr4_4g_1rank_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050050}, /* Dual-channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x00ff0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
	{DDR_REG_BASE_DMC0 + ddr_dmc_cfg_rnkvol(0), 0x162}, /* PHY0 CHA row address bit width:17 bits */
	{DDR_REG_BASE_DMC1 + ddr_dmc_cfg_rnkvol(0), 0x162}, /* PHY0 CHB row address bit width:17 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x0, 0), 0x162}, /* PHY0 CHA row address bit width:17 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x1, 0), 0x162}, /* PHY0 CHB row address bit width:17 bits */
};

/* LPDDR4 2rank 2GB:1GB,1GB */
static struct ddrtrn_reg_val_conf lpddr4_2g_2rank_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050050}, /* Dual-channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x00ff0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
	{DDR_REG_BASE_DMC0 + ddr_dmc_cfg_rnkvol(0), 0x142}, /* PHY0 CHA row address bit width:15 bits */
	{DDR_REG_BASE_DMC1 + ddr_dmc_cfg_rnkvol(0), 0x142}, /* PHY0 CHB row address bit width:15 bits */
	{DDR_REG_BASE_DMC0 + ddr_dmc_cfg_rnkvol(1), 0x142}, /* PHY0 CHA row address bit width:15 bits */
	{DDR_REG_BASE_DMC1 + ddr_dmc_cfg_rnkvol(1), 0x142}, /* PHY0 CHB row address bit width:15 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x0, 0), 0x142}, /* PHY0 CHA row address bit width:15 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x1, 0), 0x142}, /* PHY0 CHB row address bit width:15 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x0, 1), 0x142}, /* PHY0 CHA row address bit width:15 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x1, 1), 0x142}, /* PHY0 CHB row address bit width:15 bits */
};

/* LPDDR4 2rank 4GB:2GB,2GB */
static struct ddrtrn_reg_val_conf lpddr4_4g_2rank_table[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700}, /* enable Region0 */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050050}, /* Dual-channel interleaving */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x00ff0040}, /* start address:0x40000000, total capacity:4GB */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0}, /* Channel offset: 0 */
	{DDR_REG_BASE_DMC0 + ddr_dmc_cfg_rnkvol(0), 0x152}, /* PHY0 CHA row address bit width:16 bits */
	{DDR_REG_BASE_DMC1 + ddr_dmc_cfg_rnkvol(0), 0x152}, /* PHY0 CHB row address bit width:16 bits */
	{DDR_REG_BASE_DMC0 + ddr_dmc_cfg_rnkvol(1), 0x152}, /* PHY0 CHA row address bit width:16 bits */
	{DDR_REG_BASE_DMC1 + ddr_dmc_cfg_rnkvol(1), 0x152}, /* PHY0 CHB row address bit width:16 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x0, 0), 0x152}, /* PHY0 CHA row address bit width:16 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x1, 0), 0x152}, /* PHY0 CHB row address bit width:16 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x0, 1), 0x152}, /* PHY0 CHA row address bit width:16 bits */
	{DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(0x1, 1), 0x152}, /* PHY0 CHB row address bit width:16 bits */
};

void ddrtrn_chsel_remap_func(void)
{
	unsigned int num;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	struct ddrtrn_reg_val_conf *chsel_remap_reg = NULL;

	ddrtrn_hal_axi_special_intlv_en();

	num = sizeof(chsel_remap_reg_val_phy0_lpddr4) / sizeof(chsel_remap_reg_val_phy0_lpddr4[0]);
	if (phy_idx == 0) {
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			chsel_remap_reg = &chsel_remap_reg_val_phy0_lpddr4[0];
		else
			chsel_remap_reg = &chsel_remap_reg_val_phy0_nonlpddr4[0];
	} else if (phy_idx == 1) {
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			chsel_remap_reg = &chsel_remap_reg_val_phy1_lpddr4[0];
		else
			chsel_remap_reg = &chsel_remap_reg_val_phy1_nonlpddr4[0];
	}

	ddrtrn_reg_config(chsel_remap_reg, num);
}

void ddrtrn_capat_adpat_cfg_nonlpddr4(struct ddr_capat_phy_all *capat_phy_all)
{
	/* DDR4 1rank 16bit 512MB,1GB,2GB */
	if (capat_phy_all->phy_capat[0].capat_high16bit == 0) {
		ddrtrn_reg_config(&ddr4_2g_table[0], sizeof(ddr4_2g_table) / sizeof(ddr4_2g_table[0]));
		return;
	}

	/* DDR4 1rank 32bit 1.5GB:512MB,1GB */
	if (capat_phy_all->phy_capat[0].capat_low16bit == DDR_SIZE_1G &&
		capat_phy_all->phy_capat[0].capat_high16bit == DDR_SIZE_512M) {
		ddrtrn_reg_config(&ddr4_1p5g_table[0], sizeof(ddr4_1p5g_table) / sizeof(ddr4_1p5g_table[0]));
		return;
	}

	/* DDR4 1rank 32bit 3GB:2GB,1GB */
	if (capat_phy_all->phy_capat[0].capat_low16bit == DDR_SIZE_2G &&
		capat_phy_all->phy_capat[0].capat_high16bit == DDR_SIZE_1G) {
		ddrtrn_reg_config(&ddr4_3g_table[0], sizeof(ddr4_3g_table) / sizeof(ddr4_3g_table[0]));
		return;
	}

	/* DDR4 1rank 32bit 512MB:256MB,256MB;1GB:512MB,512MB;2GB:1GB,1GB;4GB:2GB,2GB */
	if (capat_phy_all->phy_capat[0].capat_low16bit == capat_phy_all->phy_capat[0].capat_high16bit &&
		capat_phy_all->phy_capat[0].capat_low16bit != 0) {
		ddrtrn_reg_config(&ddr4_4g_table[0], sizeof(ddr4_4g_table) / sizeof(ddr4_4g_table[0]));
		return;
	}
}

void ddrtrn_capat_adpat_cfg_lpddr4(struct ddr_capat_phy_all *capat_phy_all)
{
	/* LPDDR4 1rank 512MB,1GB,2GB,4GB */
	if (capat_phy_all->cur_phy_rank_num[0] == DDR_1_RANK) {
		ddrtrn_reg_config(&lpddr4_4g_1rank_table[0], sizeof(lpddr4_4g_1rank_table) / sizeof(lpddr4_4g_1rank_table[0]));
		return;
	}

	/* LPDDR4 2rank 2GB:1GB,1GB */
	if (capat_phy_all->cur_phy_rank_num[0] == DDR_2_RANK && capat_phy_all->phy_capat[0].capacity == DDR_SIZE_2G) {
		ddrtrn_reg_config(&lpddr4_2g_2rank_table[0], sizeof(lpddr4_2g_2rank_table) / sizeof(lpddr4_2g_2rank_table[0]));
		return;
	}

	/* LPDDR4 2rank 4GB:2GB,2GB */
	if (capat_phy_all->cur_phy_rank_num[0] == DDR_2_RANK && capat_phy_all->phy_capat[0].capacity == DDR_SIZE_4G) {
		ddrtrn_reg_config(&lpddr4_4g_2rank_table[0], sizeof(lpddr4_4g_2rank_table) / sizeof(lpddr4_4g_2rank_table[0]));
		return;
	}
}
#endif /* DDR_CAPAT_ADAPT_CFG */
