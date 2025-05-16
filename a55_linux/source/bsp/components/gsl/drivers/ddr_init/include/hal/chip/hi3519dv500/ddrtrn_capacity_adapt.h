/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDRTRN_CAPACITY_ADAPT_H
#define DDRTRN_CAPACITY_ADAPT_H

#include "ddrtrn_training.h"

void ddrtrn_chsel_remap_func(void);
void ddrtrn_capat_adpat_cfg_nonlpddr4(struct ddr_capat_phy_all *capat_phy_all);
void ddrtrn_capat_adpat_cfg_lpddr4(struct ddr_capat_phy_all *capat_phy_all);

#endif /* DDRTRN_CAPACITY_ADAPT_H */
