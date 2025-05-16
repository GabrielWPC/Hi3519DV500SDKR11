/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

#ifdef DDR_WL_TRAINING_CONFIG
void ddrtrn_hal_wl_bdl_add(unsigned int *raw, unsigned int val)
{
	if (((*raw) + val) > PHY_BDL_MASK)
		*raw = PHY_BDL_MASK;
	else
		*raw += val;
}

void ddrtrn_hal_wl_bdl_sub(unsigned int *raw, unsigned int val)
{
	if ((*raw) > val)
		*raw -= val;
	else
		*raw = 0;
}

/* DDR PHY DQ phase increase */
void ddrtrn_hal_wl_phase_inc(unsigned int *raw)
{
#if defined(DDR_PHY_T28_CONFIG) || defined(DDR_PHY_T16_CONFIG) || \
	defined(DDR_PHY_T12_V100_CONFIG) || defined(DDR_PHY_T12_V101_CONFIG)
	if ((*raw) < (PHY_WDQS_PHASE_MASK - 1)) {
		if (((*raw) & 0x3) == 0x2)
			*raw += 0x2;
		else
			*raw += 0x1;
	}
#else
	if ((*raw) < PHY_WDQS_PHASE_MASK)
		*raw += 0x1;
#endif
}

/* DDR PHY DQ phase decrease */
void ddrtrn_hal_wl_phase_dec(unsigned int *raw)
{
#if defined(DDR_PHY_T28_CONFIG) || defined(DDR_PHY_T16_CONFIG) || \
	defined(DDR_PHY_T12_V100_CONFIG) || defined(DDR_PHY_T12_V101_CONFIG)
	if ((*raw) > 0x1) {
		if (((*raw) & 0x3) == 0x3)
			*raw -= 0x2;
		else
			*raw -= 0x1;
	}
#else
	if ((*raw) > 0x0)
		*raw -= 0x1;
#endif
}

/* DQ bdl add or sub */
void ddrtrn_hal_wl_dq_bdl_operate(unsigned int base_phy,
								  unsigned int addr_offset, unsigned int val, unsigned int is_add)
{
	unsigned int tmp;
	unsigned int dq_bdl[DDR_PHY_REG_DQ_NUM];
	int i;
	tmp = ddrtrn_reg_read(base_phy + addr_offset);
	dq_bdl[0] = (tmp >> PHY_BDL_DQ0_BIT) & PHY_BDL_MASK; /* wdq0bdl */
	dq_bdl[1] = (tmp >> PHY_BDL_DQ1_BIT) & PHY_BDL_MASK; /* wdq1bdl */
	dq_bdl[2] = (tmp >> PHY_BDL_DQ2_BIT) & PHY_BDL_MASK; /* wdq2bdl */
	dq_bdl[3] = (tmp >> PHY_BDL_DQ3_BIT) & PHY_BDL_MASK; /* wdq3bdl */
	for (i = 0; i < DDR_PHY_REG_DQ_NUM; i++) {
		if (is_add)
			ddrtrn_hal_wl_bdl_add(&dq_bdl[i], val);
		else
			ddrtrn_hal_wl_bdl_sub(&dq_bdl[i], val);
	}
	tmp = (dq_bdl[3] << PHY_BDL_DQ3_BIT) + (dq_bdl[2] << PHY_BDL_DQ2_BIT) + /* wdq3bdl and wdq2bdl */
		  (dq_bdl[1] << PHY_BDL_DQ1_BIT) + (dq_bdl[0] << PHY_BDL_DQ0_BIT); /* wdq1bdl and wdq0bdl */
	ddrtrn_reg_write(tmp, base_phy + addr_offset);
}

/* Disable or enable DDR write leveling mode */
void ddrtrn_hal_wl_switch(unsigned int base_dmc, unsigned int base_phy, int val)
{
	unsigned int mr1_raw;
	unsigned int sfc_cmd;
	unsigned int sfc_bank;
	/* Set Rank = 0, Cmd = MRS, No Precharch CMD */
	mr1_raw =
		ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG01) >> PHY_MODEREG01_MR1_BIT;
	sfc_cmd = DMC_CMD_TYPE_LMR;
	sfc_bank = DMC_BANK_MR1;
	if (val == DDR_TRUE) /* enable DDR wl */
		/* Set A7 equal 1 */
		sfc_cmd += (mr1_raw | DMC_CMD_MRS_A7) << DMC_SFC_CMD_MRS_BIT;
	else
		/* Set A7 equal 0 */
		sfc_cmd += (mr1_raw & ((~DMC_CMD_MRS_A7) & DMC_CMD_MRS_MASK)) <<
				   DMC_SFC_CMD_MRS_BIT;
	ddrtrn_hal_dmc_sfc_cmd(base_dmc, sfc_cmd, 0x0, sfc_bank);
	/* clear */
	if (val == DDR_FALSE) {
		ddrtrn_reg_write(0x0, base_dmc + DDR_DMC_SFCBANK);
		ddrtrn_reg_write(0x0, base_dmc + DDR_DMC_SFCREQ);
	}
	/* phy sw write leveling mode */
	ddrtrn_reg_write(val, base_phy + DDR_PHY_SWTMODE);
}

void ddrtrn_hal_wl_init_wdqs(struct ddrtrn_hal_delay *wdqs_old,
							 struct ddrtrn_hal_delay *wdqs_new, unsigned int index)
{
	unsigned int tmp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_index = ddrtrn_hal_get_rank_id();
	tmp = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_index, index));
	wdqs_old->phase[index] = (tmp >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK;
	wdqs_old->bdl[index] = (tmp >> PHY_WDQS_BDL_BIT) & PHY_BDL_MASK;
	wdqs_new->phase[index] = wdqs_old.phase[index];
	wdqs_new->bdl[index] = 0;
	/* clear wdqs bdl */
	ddrtrn_reg_write(wdqs_new->phase[index] << PHY_WDQS_PHASE_BIT,
					 base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_index, index));
}

void ddrtrn_hal_wl_check_phase_result(struct ddrtrn_hal_delay *wdqs_new, unsigned int index, int result)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_index = ddrtrn_hal_get_rank_id();
	/* find phase error, keep max value to find bdl. */
	/* find phase ok, decrease to find bdl. */
	if (result == 0)
		ddrtrn_hal_wl_phase_dec(&wdqs_new.phase[index]);
	ddrtrn_reg_write(wdqs_new.phase[index] << PHY_WDQS_PHASE_BIT,
					 base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_index, index));
}

void ddrtrn_hal_wl_restore_default_value(
	struct ddrtrn_hal_delay *wdqs_old, unsigned int index)
{
	unsigned int tmp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_index = ddrtrn_hal_get_rank_id();
	tmp = (wdqs_old->phase[index] << PHY_WDQS_PHASE_BIT) + (wdqs_old->bdl[index] << PHY_WDQS_BDL_BIT);
	ddrtrn_reg_write(tmp, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_index, index));
}

#ifdef DDR_WL_DATAEYE_ADJUST_CONFIG
/* Adjust dataeye WDQ after Write leveling */
void ddrtrn_hal_wl_wdq_adjust(struct ddrtrn_hal_delay *wdqs_new, struct ddrtrn_hal_delay *wdqs_old)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	ddrtrn_debug("DDR WL write adjust");
	/* check wl write adjust bypass bit */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_WL_ADJ_MASK) != DDR_FALSE)
		return;
	/* adjust wdq phase, wdq bdl, wdm bdl */
	for (i = 0; i < byte_num; i++) {
		if (wdqs_new->phase[i] == wdqs_old->phase[i] && wdqs_new->bdl[i] == wdqs_old->bdl[i])
			continue;
		ddrtrn_hal_wl_wdq_cmp(wdqs_new, wdqs_old, i);
	}
	ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
}
#endif /* DDR_WL_DATAEYE_ADJUST_CONFIG */

#ifdef DDR_WL_DATAEYE_ADJUST_CONFIG
void ddrtrn_hal_wl_wdq_cmp(
	struct ddrtrn_hal_delay *wdqs_new, struct ddrtrn_hal_delay *wdqs_old, unsigned int byte_idx)
{
	unsigned int val;
	unsigned int phase_adj;
	unsigned int wdq_phase;
	unsigned int wdm_bdl;
	unsigned int bdl_adj = 0; /* for write dataeye */
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	phase_adj = 0;
	wdq_phase =
		(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx)) >>
		 PHY_WDQ_PHASE_BIT) &
		PHY_WDQ_PHASE_MASK;
	wdm_bdl =
		(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx)) >>
		 PHY_WDM_BDL_BIT) &
		PHY_BDL_MASK;
	if (wdqs_new->bdl[byte_idx] > wdqs_old->bdl[byte_idx]) {
		val = wdqs_new->bdl[byte_idx] - wdqs_old->bdl[byte_idx];
		phase_adj = val >> DDR_BDL_PHASE_REL;
		wdq_phase = wdq_phase + phase_adj;
		if (wdq_phase > PHY_WDQ_PHASE_MASK)
			wdq_phase = PHY_WDQ_PHASE_MASK;
		/* adjust wdq bdl and dm bdl in opposite direction */
		bdl_adj = phase_adj << DDR_BDL_PHASE_REL;
		ddrtrn_hal_wl_dq_bdl_operate(base_phy,
									 ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx), bdl_adj, DDR_FALSE);
		ddrtrn_hal_wl_dq_bdl_operate(base_phy,
									 ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx), bdl_adj, DDR_FALSE);
		ddrtrn_hal_wl_bdl_sub(&wdm_bdl, bdl_adj);
	} else if (wdqs_new->bdl[byte_idx] < wdqs_old->bdl[byte_idx]) {
		val = wdqs_old->bdl[byte_idx] - wdqs_new->bdl[byte_idx];
		phase_adj = val >> DDR_BDL_PHASE_REL;
		wdq_phase = (wdq_phase > phase_adj) ? (wdq_phase - phase_adj) : 0;
		/* adjust wdq bdl and dm bdl in opposite direction */
		bdl_adj = phase_adj << DDR_BDL_PHASE_REL;
		ddrtrn_hal_wl_dq_bdl_operate(base_phy,
									 ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx), bdl_adj, DDR_TRUE);
		ddrtrn_hal_wl_dq_bdl_operate(base_phy,
									 ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx), bdl_adj, DDR_TRUE);
		ddrtrn_hal_wl_bdl_add(&wdm_bdl, bdl_adj);
	}
	ddrtrn_info("Byte[%x] WDQ adjust phase[%x] bdl[%x]", byte_idx, phase_adj,
				bdl_adj);
	ddrtrn_reg_write(wdq_phase << PHY_WDQ_PHASE_BIT,
					 base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));
	ddrtrn_reg_write(wdm_bdl << PHY_WDM_BDL_BIT,
					 base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx));
}
#endif /* DDR_WL_DATAEYE_ADJUST_CONFIG */

/* Sync WDQ phase, WDQ bdl, WDM bdl, OEN bdl, WDQ SOE bdl by WDQS value */
void ddrtrn_hal_wl_bdl_sync(struct ddrtrn_hal_delay *wdqs_new, struct ddrtrn_hal_delay *wdqs_old)
{
	unsigned int tmp;
	unsigned int val;
	int i;
	unsigned int oen_bdl, wdqsoe_bdl, wdm_bdl;
	unsigned int wdq_phase;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	unsigned int rank_index = ddrtrn_hal_get_rank_id();
	/* sync wdq phase, wdq bdl, wdm bdl, oen bdl, wdq soe bdl */
	for (i = 0; i < byte_num; i++) {
		if (wdqs_new->phase[i] == wdqs_old->phase[i] && wdqs_new->bdl[i] == wdqs_old->bdl[i])
			continue;
		ddrtrn_debug("Byte[%x] new[%x][%x] old[%x][%x]", i, wdqs_new->phase[i],
					 wdqs_new->bdl[i], wdqs_old->phase[i], wdqs_old->bdl[i]);
		/* wdq phase */
		wdq_phase = (ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_index, i)) >>
					 PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
		/* always new_phase >= old_phase */
		wdq_phase = wdq_phase + (wdqs_new->phase[i] - wdqs_old->phase[i]);
		/* bdl */
		tmp = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnoebdl(rank_index, i));
		oen_bdl = (tmp >> PHY_OEN_BDL_BIT) & PHY_BDL_MASK;
		wdqsoe_bdl = (tmp >> PHY_WDQSOE_BDL_BIT) & PHY_BDL_MASK;
		wdm_bdl = (ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_index, i)) >>
				   PHY_WDM_BDL_BIT) & PHY_BDL_MASK;
		if (wdqs_new->bdl[i] > wdqs_old->bdl[i]) {
			val = wdqs_new->bdl[i] - wdqs_old->bdl[i];
			ddrtrn_hal_wl_dq_bdl_operate(base_phy, ddrtrn_hal_phy_dxnwdqnbdl0(rank_index, i), val, DDR_TRUE);
			ddrtrn_hal_wl_dq_bdl_operate(base_phy, ddrtrn_hal_phy_dxnwdqnbdl1(rank_index, i), val, DDR_TRUE);
			ddrtrn_hal_wl_bdl_add(&oen_bdl, val);
			ddrtrn_hal_wl_bdl_add(&wdqsoe_bdl, val);
			ddrtrn_hal_wl_bdl_add(&wdm_bdl, val);
		} else if (wdqs_new->bdl[i] < wdqs_old->bdl[i]) {
			val = wdqs_old->bdl[i] - wdqs_new->bdl[i];
			ddrtrn_hal_wl_dq_bdl_operate(base_phy, ddrtrn_hal_phy_dxnwdqnbdl0(rank_index, i), val, DDR_FALSE);
			ddrtrn_hal_wl_dq_bdl_operate(base_phy, ddrtrn_hal_phy_dxnwdqnbdl1(rank_index, i), val, DDR_FALSE);
			ddrtrn_hal_wl_bdl_sub(&oen_bdl, val);
			ddrtrn_hal_wl_bdl_sub(&wdqsoe_bdl, val);
			ddrtrn_hal_wl_bdl_sub(&wdm_bdl, val);
		}
		if (wdq_phase > PHY_WDQ_PHASE_MASK)
			wdq_phase = PHY_WDQ_PHASE_MASK;
		ddrtrn_reg_write(wdq_phase << PHY_WDQ_PHASE_BIT,
						 base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_index, i));
		ddrtrn_reg_write((wdqsoe_bdl << PHY_WDQSOE_BDL_BIT) + (oen_bdl << PHY_OEN_BDL_BIT),
						 base_phy + ddrtrn_hal_phy_dxnoebdl(rank_index, i));
		ddrtrn_reg_write((wdm_bdl << PHY_WDM_BDL_BIT),
						 base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_index, i));
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/*
 * Write leveling process.
 * WL depend default WDQS phase value in register init table.
 */
int ddrtrn_hal_wl_process(unsigned int type, struct ddrtrn_hal_delay *wdqs)
{
	int i, j;
	unsigned int wl_result;
	unsigned int length;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	if (type == DDR_DELAY_PHASE)
		length = PHY_WDQS_PHASE_MASK;
	else
		length = PHY_BDL_MASK;
	/* find WDQS phase or bdl, assume CLK Delay > DQS Delay */
	for (i = 0; i <= length; i++) {
		ddrtrn_hal_phy_cfg_update(base_phy);
		ddrtrn_reg_write(0x1, base_phy + DDR_PHY_SWTWLDQS);
		ddr_asm_dsb();
		wl_result =
			ddrtrn_reg_read(base_phy + DDR_PHY_SWTRLT) & PHY_SWTRLT_WL_MASK;
		ddrtrn_reg_write(0x0, base_phy + DDR_PHY_SWTWLDQS);
		if ((wl_result & ((1 << byte_num) - 1)) == ((1 << byte_num) - 1))
			break;
		for (j = 0; j < byte_num; j++) {
			ddrtrn_info(
				"type[0x%x] byte[0x%x] phase[0x%x] bdl[0x%x] wl_result[0x%x]",
				type, j, wdqs->phase[j], wdqs->bdl[j], wl_result);
			if (wl_result & (1 << j))
				continue;
			if (type == DDR_DELAY_PHASE)
				ddrtrn_hal_wl_phase_inc(&wdqs->phase[j]);
			else
				wdqs->bdl[j] += DDR_WL_BDL_STEP;
			ddrtrn_reg_write((wdqs->phase[j] << PHY_WDQS_PHASE_BIT) +
							 (wdqs->bdl[j] << PHY_WDQS_BDL_BIT),
							 base_phy + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), j));
		}
	}
	if (i > length) { /* wl error, not find wdqs delay */
		ddrtrn_wl_error(type, byte_num, wl_result, base_phy);
		return -1;
	} else {
		return 0;
	}
}
#endif /* DDR_WL_TRAINING_CONFIG */
