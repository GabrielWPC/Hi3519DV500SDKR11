/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

struct ddrtrn_hal_phy_all *ddrtrn_hal_get_phy(void)
{
	struct ddrtrn_hal_phy_all *phy_position = NULL;
	phy_position = (struct ddrtrn_hal_phy_all *)((uintptr_t)ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL +
																			SYSCTRL_DDR_TRAINING_PHY));
	return phy_position;
}

struct ddrtrn_hal_context *ddrtrn_hal_get_ctx(void)
{
	struct ddrtrn_hal_context *ctx_position = NULL;
	ctx_position = (struct ddrtrn_hal_context *)((uintptr_t)ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL +
																			SYSCTRL_DDR_TRAINING_CTX));
	return ctx_position;
}

void ddrtrn_hal_set_cfg_addr(uintptr_t ctx_addr, uintptr_t phy_all_addr)
{
	ddrtrn_reg_write(ctx_addr, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CTX);
	ddrtrn_reg_write(phy_all_addr, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_PHY);
}

static void ddrtrn_hal_training_cfg_set_rank(struct ddrtrn_hal_phy_all *phy_all)
{
	phy_all->phy[0].rank_num = 1;
	phy_all->phy[0].rank[0].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG);
	phy_all->phy[0].rank[0].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK0);
#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	phy_all->phy[0].rank[1].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC);
#endif
	phy_all->phy[0].rank[1].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK1);
	if (ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK1))
		/* rank number equal 2 if SYSCTRL_DDR_HW_PHY0_RANK1 has bean define in boot table */
		phy_all->phy[0].rank_num = 2; /* 2 rank */
	ddrtrn_info("Rank number PHY0 [%x]", phy_all->phy[0].rank_num);
	ddrtrn_info("HW training item PHY0[%x = %x][%x = %x]",
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK0), phy_all->phy[0].rank[0].item_hw,
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK1), phy_all->phy[0].rank[1].item_hw);
#ifdef DDR_REG_BASE_PHY1
	phy_all->phy[1].rank_num = 1;
	phy_all->phy[1].rank[0].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG);
	phy_all->phy[1].rank[0].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK0);
	phy_all->phy[1].rank[1].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC);
	phy_all->phy[1].rank[1].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK1);
	if (ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK1))
		/* rank number equal 2 if SYSCTRL_DDR_HW_PHY1_RANK1 has bean define in boot table */
		phy_all->phy[1].rank_num = 2; /* 2 rank */
	ddrtrn_info("Rank number PHY1[%x]", phy_all->phy[1].rank_num);
	ddrtrn_info("HW training item PHY1[%x = %x][%x = %x]",
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK0), phy_all->phy[1].rank[0].item_hw,
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK1), phy_all->phy[1].rank[1].item_hw);
#endif
#ifdef DDR_REG_BASE_PHY2
	phy_all->phy[2].rank_num = 1;
	phy_all->phy[2].rank[0].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG); /* phy2 */
	phy_all->phy[2].rank[0].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK0); /* phy2 */
	phy_all->phy[2].rank[1].item = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC); /* phy2 */
	phy_all->phy[2].rank[1].item_hw = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK1); /* phy2 */
	if (ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK1))
		/* rank number equal 2 if SYSCTRL_DDR_HW_PHY1_RANK1 has bean define in boot table */
		phy_all->phy[2].rank_num = 2; /* 2 rank */
	ddrtrn_info("Rank number PHY2[%x]", phy->rank_num); /* phy2 */
	ddrtrn_info("HW training item PHY2[%x = %x][%x = %x]",
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK0), phy_all->phy[2].rank[0].item_hw, /* phy2 */
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK1), phy_all->phy[2].rank[1].item_hw); /* phy2 */
#endif
	ddrtrn_info("SW training item Rank0[%x = %x]",
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG), phy_all->phy[0].rank[0].item);
#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	ddrtrn_info("SW training item Rank1[%x = %x]",
				(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC), phy_all->phy[0].rank[1].item);
#endif
}

static void ddrtrn_hal_training_cfg_set_phy0_dmc(struct ddrtrn_hal_phy_all *phy_all)
{
	unsigned int ddrt_pattern;
	if (phy_all->phy[0].dram_type == PHY_DRAMCFG_TYPE_LPDDR4) {
		phy_all->phy[0].dmc_num = 2; /* lpddr4: 2 dmc per phy */
		ddrt_pattern = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN);
		phy_all->phy[0].dmc[0].addr = DDR_REG_BASE_DMC0;
		phy_all->phy[0].dmc[0].ddrt_pattern = ddrt_pattern & 0xffff; /* bit[15-0]:dmc0 ddrt pattern */
		phy_all->phy[0].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC0);
		phy_all->phy[0].dmc[1].addr = DDR_REG_BASE_DMC1;
		phy_all->phy[0].dmc[1].ddrt_pattern = ddrt_pattern >> 16; /* bit[31-16]:dmc1 ddrt pattern */
		phy_all->phy[0].dmc[1].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC1);
		phy_all->phy[0].total_byte_num = phy_all->phy[0].dmc[0].byte_num + phy_all->phy[0].dmc[1].byte_num;
	} else {
		phy_all->phy[0].dmc_num = 1; /* other: 1 dmc per phy */
		phy_all->phy[0].dmc[0].addr = DDR_REG_BASE_DMC0;
		phy_all->phy[0].dmc[0].ddrt_pattern = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN);
		phy_all->phy[0].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC0);
		phy_all->phy[0].total_byte_num = phy_all->phy[0].dmc[0].byte_num;
	}
	ddrtrn_info("phy[0] total_byte_num[%x] dram_type[%x]", phy_all->phy[0].total_byte_num, phy_all->phy[0].dram_type);
}

#ifdef DDR_REG_BASE_PHY1
static void ddrtrn_hal_training_cfg_set_phy1_dmc(struct ddrtrn_hal_phy_all *phy_all)
{
	unsigned int ddrt_pattern;
	if (phy_all->phy[1].dram_type == PHY_DRAMCFG_TYPE_LPDDR4) {
		phy_all->phy[1].dmc_num = 2; /* lpddr4: 2 dmc per phy */
		ddrt_pattern = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN_SEC);
		phy_all->phy[1].dmc[0].addr = DDR_REG_BASE_DMC2;
		phy_all->phy[1].dmc[0].ddrt_pattern = ddrt_pattern & 0xffff; /* bit[15-0]:dmc0 ddrt pattern */
		phy_all->phy[1].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC2);
		phy_all->phy[1].dmc[1].addr = DDR_REG_BASE_DMC3;
		phy_all->phy[1].dmc[1].ddrt_pattern = ddrt_pattern >> 16; /* bit[31-16]:dmc1 ddrt pattern */
		phy_all->phy[1].dmc[1].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC3);
		phy_all->phy[1].total_byte_num = phy_all->phy[1].dmc[0].byte_num + phy_all->phy[1].dmc[1].byte_num;
	} else {
		phy_all->phy[1].dmc_num = 1; /* other: 1 dmc per phy */
#ifdef DDR_CHANNEL_MAP_PHY0_DMC0_PHY1_DMC2
		phy_all->phy[1].dmc[0].addr = DDR_REG_BASE_DMC2;
		phy_all->phy[1].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC2);
#else
		phy_all->phy[1].dmc[0].addr = DDR_REG_BASE_DMC1;
		phy_all->phy[1].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC1);
#endif
		phy_all->phy[1].dmc[0].ddrt_pattern = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN_SEC);
		phy_all->phy[1].total_byte_num = phy_all->phy[1].dmc[0].byte_num;
	}
	ddrtrn_info("phy[1] total_byte_num[%x] dram_type[%x]", phy_all->phy[1].total_byte_num, phy_all->phy[1].dram_type);
}
#endif

#ifdef DDR_REG_BASE_PHY2
static void ddrtrn_hal_training_cfg_set_phy2_dmc(struct ddrtrn_hal_phy_all *phy_all)
{
	unsigned int ddrt_pattern;
	if (phy_all->phy[2].dram_type == PHY_DRAMCFG_TYPE_LPDDR4) { /* phy2 */
		phy_all->phy[2].dmc_num = 2; /* lpddr4: 2 dmc per phy */
		ddrt_pattern = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN_THIRD);
		phy_all->phy[2].dmc[0].addr = DDR_REG_BASE_DMC4; /* phy2 */
		phy_all->phy[2].dmc[0].ddrt_pattern = ddrt_pattern & 0xffff; /* phy2, bit[15-0]:dmc0 ddrt pattern */
		phy_all->phy[2].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC4);
		phy_all->phy[2].dmc[1].addr = DDR_REG_BASE_DMC5; /* phy2 */
		phy_all->phy[2].dmc[1].ddrt_pattern = ddrt_pattern >> 16; /* phy2, bit[31-16]:dmc1 ddrt pattern */
		phy_all->phy[2].dmc[1].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC5);
		phy_all->phy[2].total_byte_num = phy_all->phy[2].dmc[0].byte_num + phy_all->phy[2].dmc[1].byte_num; /* phy2 */
	} else {
		phy_all->phy[2].dmc_num = 1; /* phy2, other: 1 dmc per phy */
#ifdef DDR_CHANNEL_MAP_PHY0_DMC0_PHY1_DMC2
		phy_all->phy[2].dmc[0].addr = DDR_REG_BASE_DMC4;
		phy_all->phy[2].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC4); /* phy2 */
#else
		phy_all->phy[2].dmc[0].addr = DDR_REG_BASE_DMC2; /* phy2 */
		phy_all->phy[2].dmc[0].byte_num = ddrtrn_hal_phy_get_byte_num(DDR_REG_BASE_DMC2); /* phy2 */
#endif
		/* phy2 */
		phy_all->phy[2].dmc[0].ddrt_pattern =
			ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRT_PATTERN_THIRD); /* phy2 */
		phy_all->phy[2].total_byte_num = phy_all->phy[2].dmc[0].byte_num; /* phy2 */
	}
	/* phy2 */
	ddrtrn_info("phy[2] total_byte_num[%x] dram_type[%x]", phy_all->phy[2].total_byte_num,
				phy_all->phy[2].dram_type); /* phy2 */
}
#endif

/* DDR training phy/dmc/dram_type config init */
static void ddrtrn_hal_training_cfg_set_dmc(struct ddrtrn_hal_phy_all *phy_all)
{
	ddrtrn_hal_training_cfg_set_phy0_dmc(phy_all);
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_hal_training_cfg_set_phy1_dmc(phy_all);
#endif
#ifdef DDR_REG_BASE_PHY2
	ddrtrn_hal_training_cfg_set_phy2_dmc(phy_all);
#endif
}

static void ddrtrn_hal_training_cfg_set_phy(struct ddrtrn_hal_context *ctx, struct ddrtrn_hal_phy_all *phy_all)
{
	ctx->phy_num = DDR_PHY_NUM;
	phy_all->phy[0].addr = DDR_REG_BASE_PHY0;
	phy_all->phy[0].dram_type = ddrtrn_reg_read(DDR_REG_BASE_PHY0 + DDR_PHY_DRAMCFG) &
								PHY_DRAMCFG_TYPE_MASK;
#ifdef DDR_REG_BASE_PHY1
	phy_all->phy[1].addr = DDR_REG_BASE_PHY1;
	phy_all->phy[1].dram_type = ddrtrn_reg_read(DDR_REG_BASE_PHY1 + DDR_PHY_DRAMCFG) &
								PHY_DRAMCFG_TYPE_MASK;
#endif
#ifdef DDR_REG_BASE_PHY2
	phy_all->phy[2].addr = DDR_REG_BASE_PHY2;
	phy_all->phy[2].dram_type = /* phy2 */
		ddrtrn_reg_read(DDR_REG_BASE_PHY2 + DDR_PHY_DRAMCFG) & PHY_DRAMCFG_TYPE_MASK;
#endif
}

void ddrtrn_hal_cfg_init(void)
{
	struct ddrtrn_hal_context *ctx = NULL;
	struct ddrtrn_hal_phy_all *phy_all = NULL;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	ddrtrn_set_data(ctx, 0, sizeof(struct ddrtrn_hal_context));
	ddrtrn_set_data(phy_all, 0, sizeof(struct ddrtrn_hal_phy_all));
	ddrtrn_hal_training_cfg_set_phy(ctx, phy_all);
	ddrtrn_hal_training_cfg_set_dmc(phy_all);
	ddrtrn_hal_training_cfg_set_rank(phy_all);
}
