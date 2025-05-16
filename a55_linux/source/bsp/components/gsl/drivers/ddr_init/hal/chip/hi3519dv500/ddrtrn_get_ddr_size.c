/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_get_ddr_size.h"
#include "ddrtrn_capacity_adapt.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_CAPAT_ADAPT_CFG
static void ddrtrn_dmc_rank_cfg(void)
{
	unsigned int ddrmode_val;
	unsigned int rank_num, dmc_num, dmc_idx, base_dmc;

	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();

	for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
		base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
		ddrmode_val = ddrtrn_reg_read(base_dmc + DDR_DMC_CFG_DDRMODE);
		if (rank_num == DDR_1_RANK) {
			/* Rank set 1 */
			ddrtrn_reg_write((ddrmode_val & (~(DMC_DDRMODE_RANK_MASK << DMC_DDRMODE_RANK_BIT))) |
				(DMC_DDRMODE_RANK_1 << DMC_DDRMODE_RANK_BIT), base_dmc + DDR_DMC_CFG_DDRMODE);
		} else if (rank_num == DDR_2_RANK) {
			/* Rank set 2 */
			ddrtrn_reg_write((ddrmode_val & (~(DMC_DDRMODE_RANK_MASK << DMC_DDRMODE_RANK_BIT))) |
				(DMC_DDRMODE_RANK_2 << DMC_DDRMODE_RANK_BIT), base_dmc + DDR_DMC_CFG_DDRMODE);
		}
	}
}

/* Identify DDR capacity by winding
 * return: ddr size (KB)
 */
static unsigned int ddrtrn_winding_identification(unsigned int step, unsigned int max_capat, unsigned int width_mask)
{
	int cnt = 0;
	unsigned int ddr_size;

	/* dmc rank config */
	ddrtrn_dmc_rank_cfg();

	do {
		cnt++;

		/* winding */
		ddrtrn_reg_write(DDR_WINDING_NUM1, DDRT_CFG_BASE_ADDR);
		while ((step * cnt) < max_capat) {
			if ((ddrtrn_reg_read(DDRT_CFG_BASE_ADDR + ((step * cnt) << DDR_10_BIT)) & width_mask)
				== (DDR_WINDING_NUM1 & width_mask)) {
				break;
			}
			cnt++;
		}

		if ((step * cnt) >= max_capat) {
			return DDR_MAX_CAPAT; /* The return value is determined based on the project */
		}

		/* check winding */
		ddrtrn_reg_write(DDR_WINDING_NUM2, DDRT_CFG_BASE_ADDR);
		if ((ddrtrn_reg_read(DDRT_CFG_BASE_ADDR + ((step * cnt) << DDR_10_BIT)) & width_mask)
			== (DDR_WINDING_NUM2 & width_mask)) {
			if (cnt == 1) {
				return 0; /* widing fail */
			}
		}
	} while ((ddrtrn_reg_read(DDRT_CFG_BASE_ADDR + ((step * cnt) << DDR_10_BIT)) & width_mask) !=
		(DDR_WINDING_NUM2 & width_mask));

	ddr_size = step * cnt;

	return ddr_size;
}

static void ddrtrn_ddr4_2die_cfg(void)
{
	unsigned int rank_num, dmc_num, rank_idx, dmc_idx, base_dmc;
	unsigned int winding_position;
	unsigned int rnkvol, qos_rnkvol;
	struct ddrtrn_rank_rnkvol_data rnkvol_data;

	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	for (rank_idx = 0; rank_idx < rank_num; rank_idx++) {
		for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
			base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
			rnkvol = ddrtrn_reg_read(base_dmc + ddr_dmc_cfg_rnkvol(rank_idx));
			qos_rnkvol = ddrtrn_reg_read(DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(dmc_idx, rank_idx));

			rnkvol_data.rnkvol_rank[rank_idx].rnkvol[dmc_idx] = rnkvol;
			rnkvol = (rnkvol & (~(DMC_RNKVOL_MEM_COL_MASK << DMC_RNKVOL_MEM_COL_BIT)) &
				(~(DMC_RNKVOL_MEM_ROW_MASK << DMC_RNKVOL_MEM_ROW_BIT)) &
				(~(DMC_RNKVOL_MEM_BANK_MASK << DMC_RNKVOL_MEM_BANK_BIT)) &
				(~(DMC_RNKVOL_MEM_BG_MASK << DMC_CFG_MEM_BG_BIT))) |
				(DMC_RNKVOL_MEM_COL_10 << DMC_RNKVOL_MEM_COL_BIT) |
				(DMC_RNKVOL_MEM_ROW_17 << DMC_RNKVOL_MEM_ROW_BIT) |
				(DMC_RNKVOL_MEM_BANK_16 << DMC_RNKVOL_MEM_BANK_BIT) |
				(DMC_RNKVOL_MEM_BG_4 << DMC_CFG_MEM_BG_BIT);
			ddrtrn_reg_write(rnkvol, base_dmc + ddr_dmc_cfg_rnkvol(rank_idx));

			rnkvol_data.rnkvol_rank[rank_idx].qos_rnkvol[dmc_idx] = qos_rnkvol;
			qos_rnkvol = (qos_rnkvol & (~(DMC_RNKVOL_MEM_COL_MASK << DMC_RNKVOL_MEM_COL_BIT)) &
				(~(DMC_RNKVOL_MEM_ROW_MASK << DMC_RNKVOL_MEM_ROW_BIT)) &
				(~(DMC_RNKVOL_MEM_BANK_MASK << DMC_RNKVOL_MEM_BANK_BIT)) &
				(~(DMC_RNKVOL_MEM_BG_MASK << DMC_CFG_MEM_BG_BIT))) |
				(DMC_RNKVOL_MEM_COL_10 << DMC_RNKVOL_MEM_COL_BIT) |
				(DMC_RNKVOL_MEM_ROW_17 << DMC_RNKVOL_MEM_ROW_BIT) |
				(DMC_RNKVOL_MEM_BANK_16 << DMC_RNKVOL_MEM_BANK_BIT) |
				(DMC_RNKVOL_MEM_BG_4 << DMC_CFG_MEM_BG_BIT);
			ddrtrn_reg_write(qos_rnkvol, DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(dmc_idx, rank_idx));
		}
	}

	winding_position =
		ddrtrn_winding_identification(WINDING_STEP_FOR_DDR4_2DIE, DDR_MAX_CAPAT_FOR_DDR4_2DIE, DDR_REG_32BIT_MASK);
	if ((winding_position == DDR4_WINDING_POSITION1) || (winding_position == DDR4_WINDING_POSITION2)) {
		for (rank_idx = 0; rank_idx < rank_num; rank_idx++) {
			for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
				base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
				ddrtrn_reg_write(rnkvol_data.rnkvol_rank[rank_idx].rnkvol[dmc_idx],
					base_dmc + ddr_dmc_cfg_rnkvol(rank_idx));
				ddrtrn_reg_write(rnkvol_data.rnkvol_rank[rank_idx].qos_rnkvol[dmc_idx],
					DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(dmc_idx, rank_idx));
			}
		}
	}
}

static void ddrtrn_store_ddr(struct ddr_data *ddr_reg_val)
{
	unsigned int i;

	for (i = 0; i < DDR_STORE_NUM; i++) {
		ddr_reg_val->reg_val[i] = ddrtrn_reg_read(DDRT_CFG_BASE_ADDR + DDR_REG_OFFSET * i);
	}
}

static void ddrtrn_restore_ddr(struct ddr_data *ddr_reg_val)
{
	unsigned int i;

	for (i = 0; i < DDR_STORE_NUM; i++) {
		ddrtrn_reg_write(ddr_reg_val->reg_val[i], DDRT_CFG_BASE_ADDR + DDR_REG_OFFSET * i);
	}
}

/* Identifying the DDR Capacity in Different Scenarios
 * DDR4: Distinguish upper and lower 16 bits
 */
static void ddrtrn_winding_identy_scence(struct ddr_capat_by_phy *phy_capat)
{
	unsigned int dmc_num, dmc_idx;
	unsigned int mem_width;
	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();

	if (ddrtrn_hal_get_cur_phy_dram_type() != PHY_DRAMCFG_TYPE_LPDDR4) {
		for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
			ddrtrn_hal_set_cur_dmc(ddrtrn_hal_get_cur_dmc_addr(dmc_idx));
			mem_width = ddrtrn_hal_ddrt_get_mem_width();
			if (mem_width == MEM_WIDTH_32BIT) {
				phy_capat->capat_low16bit = (ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT,
					DDR_REG_LOW_16BIT_MASK) >> 1) >> DDR_10_BIT;
				phy_capat->capat_high16bit = (ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT,
					DDR_REG_HIGH_16BIT_MASK) >> 1) >> DDR_10_BIT;
				phy_capat->capacity = phy_capat->capat_low16bit + phy_capat->capat_high16bit;
			} else {
				phy_capat->capat_low16bit = 0;
				phy_capat->capat_high16bit = 0;
				phy_capat->capacity =
					ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT, DDR_REG_32BIT_MASK) >> DDR_10_BIT;
			}
		}
	} else {
		phy_capat->capacity =
			ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT, DDR_REG_32BIT_MASK) >> DDR_10_BIT;
	}
}

/* return DDR size, unit:MB */
static void ddrtrn_get_ddr_size(struct ddr_capat_phy_all *capat_phy_all)
{
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	struct ddr_data ddr_reg_val;

	/* Address mapping to PHY */
	ddrtrn_chsel_remap_func();

	/* store ddr_base value */
	ddrtrn_store_ddr(&ddr_reg_val);

	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_DDR4) {
		ddrtrn_ddr4_2die_cfg();
	}

	ddrtrn_winding_identy_scence(&capat_phy_all->phy_capat[phy_idx]);

	/* restore ddr base value */
	ddrtrn_restore_ddr(&ddr_reg_val);
}

/* config DDR */
static void ddrtrn_capat_adpat_cfg(struct ddr_capat_phy_all *capat_phy_all)
{
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_DDR4) {
		ddrtrn_hal_timing8_trfc_ab_cfg(capat_phy_all);
	}
	if (ddrtrn_hal_get_cur_phy_dram_type() != PHY_DRAMCFG_TYPE_LPDDR4) {
		ddrtrn_capat_adpat_cfg_nonlpddr4(capat_phy_all);
	} else if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		ddrtrn_capat_adpat_cfg_lpddr4(capat_phy_all);
	}
}

/* return DDR capacity (MB) */
static unsigned int ddrtrn_capat_adapt(void)
{
	int i;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();
	struct ddr_capat_phy_all capat_phy_all;

	ddrtrn_set_data((char *)&capat_phy_all, 0, sizeof(struct ddr_capat_phy_all));
	for (i = 0; i < phy_num; i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		capat_phy_all.cur_phy_rank_num[i] = ddrtrn_hal_get_cur_phy_rank_num();
		ddrtrn_get_ddr_size(&capat_phy_all);
		capat_phy_all.ddr_capat_sum += capat_phy_all.phy_capat[i].capacity;
	}

	ddrtrn_capat_adpat_cfg(&capat_phy_all);

	return capat_phy_all.ddr_capat_sum;
}

unsigned int ddrtrn_capat_adapt_func(void)
{
	ddrtrn_hal_cfg_init();
	return ddrtrn_capat_adapt();
}
#else
unsigned int ddrtrn_capat_adapt_func(void)
{
	return 0;
}
#endif /* DDR_CAPAT_ADAPT_CFG */