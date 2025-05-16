/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_api.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

int bsp_ddrtrn_training_item(unsigned int hw_item_mask, unsigned int sw_item_mask, int low_freq_flag)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	uintptr_t ctx_addr = (uintptr_t)&ctx;
	uintptr_t phy_all_addr = (uintptr_t)&phy_all;
	ddrtrn_hal_set_cfg_addr(ctx_addr, phy_all_addr);
	if (ddrtrn_hw_training_init(hw_item_mask, low_freq_flag) == -1 ||
		ddrtrn_sw_training_if(sw_item_mask) == -1 ||
		bsp_ddrtrn_suspend() == -1) {
		return -1;
	}
	return 0;
}

unsigned int bsp_ddrtrn_capat_adapt(void)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	uintptr_t ctx_addr = (uintptr_t)&ctx;
	uintptr_t phy_all_addr = (uintptr_t)&phy_all;
	ddrtrn_hal_set_cfg_addr(ctx_addr, phy_all_addr);
	return ddrtrn_capat_adapt_func();
}