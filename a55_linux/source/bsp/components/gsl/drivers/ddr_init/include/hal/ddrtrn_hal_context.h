/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDRTN_HAL_CONTEXT_H
#define DDRTN_HAL_CONTEXT_H

struct ddrtrn_hal_dmc {
	unsigned int addr;
	unsigned int byte_num;
	unsigned int ddrt_pattern; /* ddrt reversed data */
};

struct ddrtrn_hal_rank {
	unsigned int item;      /* software training item */
	unsigned int item_hw; /* hardware training item */
};

struct ddrtrn_hal_phy {
	unsigned int addr;
	unsigned int dram_type;
	unsigned int dmc_num;
	unsigned int rank_num;
	unsigned int total_byte_num;
	struct ddrtrn_hal_dmc dmc[DDR_DMC_PER_PHY_MAX];
	struct ddrtrn_hal_rank rank[DDR_RANK_NUM];
};

struct ddrtrn_hal_phy_all {
	struct ddrtrn_hal_phy phy[DDR_PHY_NUM];
};

struct ddrtrn_hal_context {
	unsigned int phy_num;
	unsigned int cur_phy;        /* current training phy addr */
	unsigned int cur_dmc;        /* current training dmc addr */
	unsigned int cur_item;        /* current SW or HW training item */
	unsigned int cur_pattern;    /* current ddrt pattern */
	unsigned int cur_mode;        /* read or write */
	unsigned int cur_byte;        /* current training byte index */
	unsigned int cur_dq;        /* current training dq index */
	unsigned int phy_idx;        /* current training phy index */
	unsigned int rank_idx;        /* current training rank index */
	unsigned int dmc_idx;        /* current training dmc index */
	unsigned int cmd_flag;       /* Flag indicating whether the command is executed */
	unsigned int res_flag;
};

struct ddrtrn_hal_phy_all *ddrtrn_hal_get_phy(void);
struct ddrtrn_hal_context *ddrtrn_hal_get_ctx(void);

static inline unsigned int ddrtrn_hal_get_phy_num(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->phy_num;
}
static inline unsigned int ddrtrn_hal_get_cur_phy(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_phy;
}

static inline unsigned int ddrtrn_hal_get_cur_dmc(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_dmc;
}
static inline unsigned int ddrtrn_hal_get_cur_item(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_item;
}
static inline unsigned int ddrtrn_hal_get_cur_pattern(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_pattern;
}
static inline unsigned int ddrtrn_hal_get_cur_mode(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_mode;
}
static inline unsigned int ddrtrn_hal_get_cur_byte(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_byte;
}
static inline unsigned int ddrtrn_hal_get_cur_dq(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cur_dq;
}
static inline unsigned int ddrtrn_hal_get_phy_id(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->phy_idx;
}
static inline unsigned int ddrtrn_hal_get_rank_id(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->rank_idx;
}

static inline unsigned int ddrtrn_hal_get_phy_addr(unsigned int idx)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[idx].addr;
}

static inline unsigned int ddrtrn_hal_get_phy_total_byte_num(unsigned int idx)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[idx].total_byte_num;
}
static inline unsigned int ddrtrn_hal_get_phy_rank_num(unsigned int idx)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[idx].rank_num;
}
static inline unsigned int ddrtrn_hal_get_phy_dram_type(unsigned int idx)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[idx].dram_type;
}

static inline unsigned int ddrtrn_hal_get_phy_dmc_num(unsigned int idx)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[idx].dmc_num;
}

static inline unsigned int ddrtrn_hal_get_rank_item_hw(unsigned int phy_id, unsigned int rank_id)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[phy_id].rank[rank_id].item_hw;
}

static inline unsigned int ddrtrn_hal_get_rank_item(unsigned int phy_id, unsigned int rank_id)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[phy_id].rank[rank_id].item;
}

static inline struct ddrtrn_hal_dmc *ddrtrn_hal_get_dmc(unsigned int phy_id, unsigned int dmc_id)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return &(phy_all->phy[phy_id].dmc[dmc_id]);
}

static inline unsigned int ddrtrn_hal_get_dmc_id(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->dmc_idx;
}

static inline unsigned int ddrtrn_hal_get_dmc_byte_num(unsigned int phy_id, unsigned int dmc_id)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[phy_id].dmc[dmc_id].byte_num;
}

static inline unsigned int ddrtrn_hal_get_dmc_ddrt_pattern(unsigned int phy_id, unsigned int dmc_id)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[phy_id].dmc[dmc_id].ddrt_pattern;
}

static inline void ddrtrn_hal_set_phy_num(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->phy_num = value;
}

static inline void ddrtrn_hal_set_cur_dmc(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_dmc = value;
}

static inline void ddrtrn_hal_set_cur_phy(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_phy = value;
}

static inline void ddrtrn_hal_set_cur_item(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_item = value;
}

static inline void ddrtrn_hal_set_cur_pattern(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_pattern = value;
}
static inline void ddrtrn_hal_set_cur_mode(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_mode = value;
}

static inline void ddrtrn_hal_set_cur_byte(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_byte = value;
}

static inline void ddrtrn_hal_set_cur_dq(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cur_dq = value;
}
static inline void ddrtrn_hal_set_phy_id(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->phy_idx = value;
}

static inline void ddrtrn_hal_set_rank_id(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->rank_idx = value;
}

static inline void ddrtrn_hal_set_dmc_id(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->dmc_idx = value;
}

static inline void ddrtrn_hal_set_phy_addr(unsigned int idx, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[idx].addr = value;
}

static inline void ddrtrn_hal_set_phy_total_byte_num(unsigned int idx, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[idx].total_byte_num = value;
}

static inline void ddrtrn_hal_set_phy_rank_num(unsigned int idx, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[idx].rank_num = value;
}

static inline void ddrtrn_hal_set_phy_dram_type(unsigned int idx, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[idx].dram_type = value;
}

static inline void ddrtrn_hal_set_rank_item_hw(unsigned int phy_id, unsigned int rank_id, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[phy_id].rank[rank_id].item_hw = value;
}

static inline void ddrtrn_hal_set_rank_item(unsigned int phy_id, unsigned int rank_id, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[phy_id].rank[rank_id].item = value;
}

static inline void ddrtrn_hal_set_dmc_addr(unsigned int phy_id, unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[phy_id].dmc[dmc_id].addr = value;
}

static inline void ddrtrn_hal_set_dmc_byte_num(unsigned int phy_id, unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[phy_id].dmc[dmc_id].byte_num = value;
}

static inline void ddrtrn_hal_set_dmc_ddrt_pattern(unsigned int phy_id, unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_phy_all *phy_all;
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[phy_id].dmc[dmc_id].ddrt_pattern = value;
}

static inline unsigned int ddrtrn_hal_get_cur_phy_addr(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].addr;
}

static inline unsigned int ddrtrn_hal_get_cur_phy_total_byte_num(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].total_byte_num;
}
static inline unsigned int ddrtrn_hal_get_cur_phy_rank_num(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].rank_num;
}
static inline unsigned int ddrtrn_hal_get_cur_phy_dram_type(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dram_type;
}

static inline unsigned int ddrtrn_hal_get_cur_phy_dmc_num(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dmc_num;
}

static inline unsigned int ddrtrn_hal_get_cur_rank_item_hw(unsigned int rank_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].rank[rank_id].item_hw;
}
static inline unsigned int ddrtrn_hal_get_cur_rank_item(unsigned int rank_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].rank[rank_id].item;
}

static inline struct ddrtrn_hal_dmc *ddrtrn_hal_get_cur_dmc_store(unsigned int dmc_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return &(phy_all->phy[ctx->phy_idx].dmc[dmc_id]);
}

static inline unsigned int ddrtrn_hal_get_cur_dmc_addr(unsigned int dmc_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dmc[dmc_id].addr;
}

static inline unsigned int ddrtrn_hal_get_cur_dmc_byte_num(unsigned int dmc_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dmc[dmc_id].byte_num;
}

static inline unsigned int ddrtrn_hal_get_cur_dmc_ddrt_pattern(unsigned int dmc_id)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dmc[dmc_id].ddrt_pattern;
}

static inline void ddrtrn_hal_set_cur_phy_addr(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].addr = value;
}

static inline void ddrtrn_hal_set_cur_phy_total_byte_num(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].total_byte_num = value;
}
static inline void ddrtrn_hal_set_cur_phy_rank_num(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].rank_num = value;
}
static inline void ddrtrn_hal_set_cur_phy_dram_type(unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].dram_type = value;
}
static inline void ddrtrn_hal_set_cur_rank_item_hw(unsigned int rank_id, unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].rank[rank_id].item_hw = value;
}

static inline void ddrtrn_hal_set_cur_rank_item(unsigned int rank_id, unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].rank[rank_id].item = value;
}

static inline void ddrtrn_hal_set_cur_dmc_addr(unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].dmc[dmc_id].addr = value;
}

static inline void ddrtrn_hal_set_cur_dmc_byte_num(unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].dmc[dmc_id].byte_num = value;
}

static inline void ddrtrn_hal_set_cur_dmc_ddrt_pattern(unsigned int dmc_id, unsigned int value)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	phy_all->phy[ctx->phy_idx].dmc[dmc_id].ddrt_pattern = value;
}

static inline unsigned int ddrtrn_hal_get_byte_num(void)
{
	struct ddrtrn_hal_context *ctx;
	struct ddrtrn_hal_phy_all *phy_all;
	ctx = ddrtrn_hal_get_ctx();
	phy_all = ddrtrn_hal_get_phy();
	return phy_all->phy[ctx->phy_idx].dmc[ctx->dmc_idx].byte_num;
}

static inline void ddrtrn_hal_cmd_flag_enable(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cmd_flag = CMD_ENABLE;
}

static inline void ddrtrn_hal_cmd_flag_disable(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->cmd_flag = CMD_DISABLE;
}

static inline void ddrtrn_hal_res_flag_enable(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->res_flag = RES_ENABLE;
}

static inline void ddrtrn_hal_res_flag_disable(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	ctx->res_flag = RES_DISABLE;
}

static inline unsigned int ddrtrn_hal_get_cmd_flag(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->cmd_flag;
}

static inline unsigned int ddrtrn_hal_get_res_flag(void)
{
	struct ddrtrn_hal_context *ctx;
	ctx = ddrtrn_hal_get_ctx();
	return ctx->res_flag;
}

void ddrtrn_hal_set_cfg_addr(uintptr_t ctx_addr, uintptr_t phy_all_addr);
#endif /* DDRTN_HAL_CONTEXT_H */
