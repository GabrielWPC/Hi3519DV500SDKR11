/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "share_drivers.h"

#include "ddrtrn_training.h"
#include "ddrtrn_boot_adapt.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

void ddrtrn_reg_config(const struct ddrtrn_reg_val_conf *reg_val, unsigned int array_size)
{
	unsigned int i;
	const struct ddrtrn_reg_val_conf *pt = reg_val;

	for (i = 0; i < array_size; i++) {
		if (i != 0) {
			pt++;
		}
		ddrtrn_reg_write(pt->val, pt->offset);
	}
}

/* Save DERT default result: rdq/rdqs/rdm/ bdl */
static void ddrtrn_save_rdqbdl_rank(struct ddrtrn_dq_data_rank *dq_rank)
{
	unsigned int i;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();

	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_save_rdqbdl(&dq_rank->rank[i]);
	}
}

void ddrtrn_save_rdqbdl_phy(struct ddrtrn_dq_data_phy *dq_phy)
{
	unsigned int i;

	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		ddrtrn_save_rdqbdl_rank(&dq_phy->phy[i]);
	}
}

/* restore rdq/rdqs/rdm/ bdl */
static void ddrtrn_restore_rdqbdl_rank(struct ddrtrn_dq_data_rank *dq_rank)
{
	unsigned int i;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();

	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_restore_rdqbdl(&dq_rank->rank[i]);
	}
}

void ddrtrn_restore_rdqbdl_phy(struct ddrtrn_dq_data_phy *dq_phy)
{
	unsigned int i;

	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		ddrtrn_restore_rdqbdl_rank(&dq_phy->phy[i]);
	}
}
