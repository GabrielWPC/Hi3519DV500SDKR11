/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

void *ddrtrn_copy_data(void *dst, const void *src, unsigned int len)
{
	const char *s = src;
	char *d = dst;
	while (len--)
		*d++ = *s++;
	return dst;
}

void *ddrtrn_set_data(void *b, int c, unsigned int len)
{
	char *bp = b;
	while (len--)
		*bp++ = (unsigned char)c;
	return b;
}

static void ddrtrn_rdq_offset_cfg_by_byte(void)
{
	unsigned int byte_idx;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_hal_set_cur_byte(byte_idx);
		ddrtrn_hal_rdqs_sync_rank_rdq(DDR_RDQ_OFFSET);
	}
}

static void ddrtrn_rdq_offset_cfg_by_rank(void)
{
	unsigned int rank_idx;
	unsigned int rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	for (rank_idx = 0; rank_idx < rank_num; rank_idx++) {
		ddrtrn_hal_set_rank_id(rank_idx);
		ddrtrn_rdq_offset_cfg_by_byte();
	}
}

void ddrtrn_rdq_offset_cfg_by_phy(void)
{
	unsigned int phy_idx;
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		ddrtrn_hal_set_phy_id(phy_idx);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(phy_idx));
		ddrtrn_rdq_offset_cfg_by_rank();
	}
}