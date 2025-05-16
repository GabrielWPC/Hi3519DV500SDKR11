/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

static void ddrtrn_hal_wdqs_sync_wdm(int offset)
{
	unsigned int wdqnbdl;
	int wdm;
	wdqnbdl = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
							  ddrtrn_hal_phy_dxnwdqnbdl2(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
	wdm = (wdqnbdl >> PHY_WDM_BDL_BIT) & PHY_WDM_BDL_MASK;
	wdm += offset;
	if (wdm < 0)
		wdm = 0;
	else if (wdm > (int)PHY_WDM_BDL_MAX)
		wdm = PHY_WDM_BDL_MAX;
	wdqnbdl = wdqnbdl & (~(PHY_WDM_BDL_MASK << PHY_WDM_BDL_BIT));
	ddrtrn_reg_write(wdqnbdl | ((unsigned int)wdm << PHY_WDM_BDL_BIT),
					 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl2(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
}

/* Get PHY DQ value */
unsigned int ddrtrn_hal_phy_get_dq_bdl(void)
{
	unsigned int val;
	unsigned int offset;
	unsigned int dq;
	unsigned int byte_index = ddrtrn_hal_get_cur_byte();
	unsigned int rank = ddrtrn_hal_get_rank_id();
	dq = ddrtrn_hal_get_cur_dq() & 0x7;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE) {
		if (dq < DDR_DQ_NUM_EACH_REG) /* [DXNWDQNBDL0] 4 bdl: wdq0bdl-wdq3bdl */
			offset = ddrtrn_hal_phy_dxnwdqnbdl0(rank, byte_index);
		else /* [DXNWDQNBDL1] 4 bdl: wdq4bdl-wdq7bdl */
			offset = ddrtrn_hal_phy_dxnwdqnbdl1(rank, byte_index);
	} else {
		if (dq < DDR_DQ_NUM_EACH_REG) /* [DXNRDQNBDL0] 4 bdl: rdq0bdl-rdq3bdl */
			offset = ddrtrn_hal_phy_dxnrdqnbdl0(rank, byte_index);
		else /* [DXNRDQNBDL1] 4 bdl: rdq4bdl-rdq7bdl */
			offset = ddrtrn_hal_phy_dxnrdqnbdl1(rank, byte_index);
	}
	dq &= 0x3; /* one register contains 4 dq */
	val = (ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + offset) >>
		   ((dq << DDR_DQBDL_SHIFT_BIT) + PHY_BDL_DQ_BIT)) & PHY_BDL_MASK;
	return val;
}

static void ddrtrn_hal_wdqs_sync_rank_wdq(int offset)
{
	int dq_val;
	int i;
	unsigned int cur_mode = ddrtrn_hal_get_cur_mode();
	ddrtrn_hal_set_cur_mode(DDR_MODE_WRITE);
	/* sync other rank wdm */
	ddrtrn_hal_wdqs_sync_wdm(offset);
	/* sync other rank wdq */
	ddrtrn_debug("Before sync rank[%x] byte[%x] dq[%x = %x][%x = %x] offset[%x]",
				 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte(),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnwdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnwdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 offset);
	for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
		ddrtrn_hal_set_cur_dq(i);
		dq_val = (int)ddrtrn_hal_phy_get_dq_bdl();
		dq_val += offset;
		if (dq_val < 0)
			dq_val = 0;
		else if (dq_val > (int)PHY_BDL_MAX)
			dq_val = PHY_BDL_MAX;
		ddrtrn_hal_phy_set_dq_bdl(dq_val);
	}
	ddrtrn_hal_set_cur_mode(cur_mode); /* restore to current mode */
	ddrtrn_debug("After sync rank[%x] byte[%x] dq[%x = %x][%x = %x]",
				 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte(),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnwdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnwdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())));
}

static void ddrtrn_hal_sync_wdqsbdl(int offset)
{
	unsigned int wdqsdly;
	int wdqsbdl;
	wdqsdly = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
							  ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
	wdqsbdl = (wdqsdly >> PHY_WDQS_BDL_BIT) & PHY_WDQS_BDL_MASK;
	wdqsbdl += offset;
	if (wdqsbdl < 0)
		wdqsbdl = 0;
	else if (wdqsbdl > (int)PHY_WDQS_BDL_MAX)
		wdqsbdl = PHY_WDQS_BDL_MAX;
	wdqsdly = wdqsdly & (~(PHY_WDQS_BDL_MASK << PHY_WDQS_BDL_BIT));
	ddrtrn_reg_write(wdqsdly | ((unsigned int)wdqsbdl << PHY_WDQS_BDL_BIT),
					 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
}

static void ddrtrn_hal_judge_wdq_rank(unsigned int byte_idx,
									  struct ddrtrn_hal_training_dq_adj *wdq_rank0, struct ddrtrn_hal_training_dq_adj *wdq_rank1)
{
	int skew;
	int phase2bdl;
	int wdqphase_rank0_tmp0, wdqphase_rank1_tmp0, wdqphase_rank0_tmp1,
		wdqphase_rank1_tmp1;
	phase2bdl = ((ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				  PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK) / PHY_WDQSPHASE_NUM_T;
	wdqphase_rank0_tmp0 = wdq_rank0->wdqphase & 0xf; /* 0xf:bit[3:0] */
	wdqphase_rank1_tmp0 = wdq_rank1->wdqphase & 0xf; /* 0xf:bit[3:0] */
	/*
	 * Remove phase holes
	 * phase hole in every 4 wdqphase reg value
	 */
	wdqphase_rank0_tmp1 = wdqphase_rank0_tmp0 - (wdqphase_rank0_tmp0 + 1) / 4; /* 4 wdqphase */
	wdqphase_rank1_tmp1 = wdqphase_rank1_tmp0 - (wdqphase_rank1_tmp0 + 1) / 4; /* 4 wdqphase */
	if (wdqphase_rank0_tmp1 >= wdqphase_rank1_tmp1) {
		skew = wdqphase_rank0_tmp1 - wdqphase_rank1_tmp1;
		ddrtrn_hal_set_rank_id(0); /* 0: adjust rank0 */
		if ((skew > (PHY_WDQPHASE_NUM_T >> 1)) &&
				(wdq_rank1->wdqphase >= PHY_WDQSPHASE_REG_NUM_T)) {
			skew = PHY_WDQPHASE_NUM_T - skew;
			wdq_rank1->wdqphase = wdq_rank1->wdqphase - PHY_WDQSPHASE_REG_NUM_T;
			ddrtrn_hal_set_rank_id(1); /* 1: adjust rank1 */
		}
	} else {
		skew = wdqphase_rank1_tmp1 - wdqphase_rank0_tmp1;
		ddrtrn_hal_set_rank_id(1); /* 1: adjust rank1 */
		if ((skew > (PHY_WDQPHASE_NUM_T >> 1)) &&
				(wdq_rank0->wdqphase >= PHY_WDQSPHASE_REG_NUM_T)) {
			skew = PHY_WDQPHASE_NUM_T - skew;
			wdq_rank0->wdqphase = wdq_rank0->wdqphase - PHY_WDQSPHASE_REG_NUM_T;
			ddrtrn_hal_set_rank_id(0); /* 0: adjust rank0 */
		}
	}
	if (ddrtrn_hal_get_rank_id() == 0) {
		wdq_rank0->wdqphase = wdq_rank0->wdqphase -
							  (unsigned int)wdqphase_rank0_tmp0 + (unsigned int)wdqphase_rank1_tmp0;
		ddrtrn_reg_write((wdq_rank0->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
						 (wdq_rank0->wdqphase << PHY_WDQ_PHASE_BIT),
						 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), byte_idx));
	} else if (ddrtrn_hal_get_rank_id() == 1) {
		wdq_rank1->wdqphase = wdq_rank1->wdqphase -
							  (unsigned int)wdqphase_rank1_tmp0 + (unsigned int)wdqphase_rank0_tmp0;
		ddrtrn_reg_write((wdq_rank1->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
						 (wdq_rank1->wdqphase << PHY_WDQ_PHASE_BIT),
						 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), byte_idx));
	}
	ddrtrn_hal_wdqs_sync_rank_wdq(phase2bdl * skew);
}

/* select which rank to adjust */
static int ddrtrn_hal_adjust_wdqs_select_rank(unsigned int byte_idx,
											  struct ddrtrn_hal_training_dq_adj *wdqs_rank0, struct ddrtrn_hal_training_dq_adj *wdqs_rank1)
{
	int skew;
	int wdqsphase_rank0_tmp0, wdqsphase_rank1_tmp0;
	int wdqsphase_rank0_tmp1, wdqsphase_rank1_tmp1;
	wdqsphase_rank0_tmp0 = wdqs_rank0->wdqsphase & 0xf; /* 0xf:bit[4:0] */
	wdqsphase_rank1_tmp0 = wdqs_rank1->wdqsphase & 0xf; /* 0xf:bit[4:0] */
	/*
	 * Remove phase holes
	 * phase hole in every 4 wdqsphase reg value
	 */
	wdqsphase_rank0_tmp1 = wdqsphase_rank0_tmp0 - (wdqsphase_rank0_tmp0 + 1) / 4; /* 4 wdqsphase */
	wdqsphase_rank1_tmp1 = wdqsphase_rank1_tmp0 - (wdqsphase_rank1_tmp0 + 1) / 4; /* 4 wdqsphase */
	if (wdqsphase_rank0_tmp1 >= wdqsphase_rank1_tmp1) {
		skew = wdqsphase_rank0_tmp1 - wdqsphase_rank1_tmp1;
		ddrtrn_hal_set_rank_id(0); /* 0: adjust rank0 */
		if ((skew > (PHY_WDQPHASE_NUM_T >> 1)) && (wdqs_rank1->wlsl >= 1) &&
				(wdqs_rank1->wdqphase < (PHY_WDQ_PHASE_MASK - PHY_WDQPHASE_REG_NUM_T))) {
			skew = PHY_WDQSPHASE_NUM_T - skew;
			wdqs_rank1->wlsl = wdqs_rank1->wlsl - 1;
			wdqs_rank1->wdqphase = wdqs_rank1->wdqphase + PHY_WDQPHASE_REG_NUM_T;
			ddrtrn_reg_write((wdqs_rank1->dxnwlsl & (~(PHY_WLSL_MASK << PHY_WLSL_BIT))) |
							 (wdqs_rank1->wlsl << PHY_WLSL_BIT), ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwlsl(1, byte_idx));
			ddrtrn_reg_write((wdqs_rank1->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
							 (wdqs_rank1->wdqphase << PHY_WDQS_PHASE_BIT),
							 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqdly(1, byte_idx));
			ddrtrn_hal_set_rank_id(1); /* 1: adjust rank1 */
		}
	} else {
		skew = wdqsphase_rank1_tmp1 - wdqsphase_rank0_tmp1;
		ddrtrn_hal_set_rank_id(1); /* 1: adjust rank1 */
		if ((skew > (PHY_WDQPHASE_NUM_T >> 1)) && (wdqs_rank0->wlsl >= 1) &&
				(wdqs_rank0->wdqphase < (PHY_WDQ_PHASE_MASK - PHY_WDQPHASE_REG_NUM_T))) {
			skew = PHY_WDQSPHASE_NUM_T - skew;
			wdqs_rank0->wlsl = wdqs_rank0->wlsl - 1;
			wdqs_rank0->wdqphase = wdqs_rank0->wdqphase + PHY_WDQPHASE_REG_NUM_T;
			ddrtrn_reg_write((wdqs_rank0->dxnwlsl & (~(PHY_WLSL_MASK << PHY_WLSL_BIT))) |
							 (wdqs_rank0->wlsl << PHY_WLSL_BIT),
							 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwlsl(0, byte_idx));
			ddrtrn_reg_write((wdqs_rank0->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
							 (wdqs_rank0->wdqphase << PHY_WDQS_PHASE_BIT),
							 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqdly(0, byte_idx));
			ddrtrn_hal_set_rank_id(0); /* 0: adjust rank0 */
		}
	}
	return skew;
}

static void ddrtrn_hal_judge_wdqs_rank(unsigned int byte_idx,
									   struct ddrtrn_hal_training_dq_adj *wdqs_rank0, struct ddrtrn_hal_training_dq_adj *wdqs_rank1)
{
	int skew;
	int phase2bdl;
	int wdqsphase_rank0_tmp0, wdqsphase_rank1_tmp0;
	phase2bdl = ((ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				  PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK) / PHY_WDQSPHASE_NUM_T;
	wdqsphase_rank0_tmp0 = wdqs_rank0->wdqsphase & 0xf; /* 0xf:bit[4:0] */
	wdqsphase_rank1_tmp0 = wdqs_rank1->wdqsphase & 0xf; /* 0xf:bit[4:0] */
	skew = ddrtrn_hal_adjust_wdqs_select_rank(byte_idx, wdqs_rank0, wdqs_rank1);
	if (ddrtrn_hal_get_rank_id() == 0) {
		wdqs_rank0->wdqsphase = wdqs_rank0->wdqsphase -
								(unsigned int)wdqsphase_rank0_tmp0 + (unsigned int)wdqsphase_rank1_tmp0;
		ddrtrn_reg_write((wdqs_rank0->wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) |
						 (wdqs_rank0->wdqsphase << PHY_WDQS_PHASE_BIT),
						 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), byte_idx));
	} else if (ddrtrn_hal_get_rank_id() == 1) {
		wdqs_rank1->wdqsphase = wdqs_rank1->wdqsphase -
								(unsigned int)wdqsphase_rank1_tmp0 + (unsigned int)wdqsphase_rank0_tmp0;
		ddrtrn_reg_write((wdqs_rank1->wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) |
						 (wdqs_rank1->wdqsphase << PHY_WDQS_PHASE_BIT),
						 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxwdqsdly(ddrtrn_hal_get_rank_id(), byte_idx));
	}
	ddrtrn_hal_sync_wdqsbdl(phase2bdl * skew);
}

static void ddrtrn_hal_set_rdqs(int val)
{
	unsigned int delay;
	delay = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(ddrtrn_hal_get_cur_byte()));
	ddrtrn_hal_phy_rdqs_sync_rdm(val);
	/* clear rdqs bdl */
	delay = delay & (~(PHY_RDQS_BDL_MASK << PHY_RDQS_BDL_BIT));
	ddrtrn_reg_write(delay | ((unsigned int)val << PHY_RDQS_BDL_BIT),
					 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(ddrtrn_hal_get_cur_byte()));
}

/* config DDR hw item */
void ddrtrn_hal_hw_item_cfg(unsigned int form_value)
{
	ddrtrn_hal_set_rank_item_hw(0, 0,
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK0) & form_value);
	ddrtrn_hal_set_rank_item_hw(0, 1,
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY0_RANK1) & form_value);
#ifdef DDR_REG_BASE_PHY1
	ddrtrn_hal_set_rank_item_hw(1, 0,
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK0) & form_value);
	ddrtrn_hal_set_rank_item_hw(1, 1,
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY1_RANK1) & form_value);
#endif
#ifdef DDR_REG_BASE_PHY2
	ddrtrn_hal_set_rank_item_hw(2, 0, /* phy2 */
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK0) & form_value);
	ddrtrn_hal_set_rank_item_hw(2, 1, /* phy2 */
								ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_HW_PHY2_RANK1) & form_value);
#endif
}

/* config DDR sw item */
void ddrtrn_hal_sw_item_cfg(unsigned int form_value)
{
	/* rank 0 */
	ddrtrn_hal_set_rank_item(0, 0,
							 ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG) & form_value);
	/* rank 1 */
#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	ddrtrn_hal_set_rank_item(0, 1,
							 ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC) & form_value);
#endif
}

/* Check DDR training item whether by pass */
int ddrtrn_hal_check_bypass(unsigned int mask)
{
	/* training item disable */
	if ((ddrtrn_hal_get_cur_item()) & mask) {
		ddrtrn_debug("DDR training [%x] is disable, rank[%x] cfg[%x]", mask,
					 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_item());
		return DDR_TRUE;
	} else {
		return DDR_FALSE;
	}
}

/*
 * PHY reset
 * To issue reset signal to PHY, this field should be set to a '1'.
 */
void ddrtrn_hal_phy_reset(unsigned int base_phy)
{
	unsigned int tmp;
	tmp = ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL);
	/* set 1 to issue PHY reset signal */
	tmp |= (1 << PHY_RST_BIT);
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_PHYINITCTRL);
	/* set 0 to end the PHY reset signal */
	tmp &= ~(1 << PHY_RST_BIT);
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_PHYINITCTRL);
}

/*
 * Update delay setting in registers to PHY immediately.
 * Make delay setting take effect.
 */
void ddrtrn_hal_phy_cfg_update(unsigned int base_phy)
{
	unsigned int tmp;
	tmp = ddrtrn_reg_read(base_phy + DDR_PHY_MISC);
	tmp |= (1 << PHY_MISC_UPDATE_BIT);
	/* update new config to PHY */
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_MISC);
	tmp &= ~(1 << PHY_MISC_UPDATE_BIT);
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_MISC);
	tmp = ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL);
	/* set 1 to issue PHY counter reset signal */
	tmp |= (1 << PHY_PHYCONN_RST_BIT);
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_PHYINITCTRL);
	/* set 0 to end the reset signal */
	tmp &= ~(1 << PHY_PHYCONN_RST_BIT);
	ddrtrn_reg_write(tmp, base_phy + DDR_PHY_PHYINITCTRL);
	ddr_asm_dsb();
}

void ddrtrn_hal_ck_cfg(unsigned int base_phy)
{
	unsigned int acphyctl7, acphyctl7_tmp;
	acphyctl7 = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	/* set the opposite val of ck: bit[25] bit[24] bit[11:9] bit[8:6] */
	acphyctl7_tmp = acphyctl7 ^ ((PHY_ACPHY_DRAMCLK_MASK << PHY_ACPHY_DRAMCLK0_BIT) |
								 (PHY_ACPHY_DRAMCLK_MASK << PHY_ACPHY_DRAMCLK1_BIT) |
								 (PHY_ACPHY_DCLK_MASK << PHY_ACPHY_DCLK0_BIT) |
								 (PHY_ACPHY_DCLK_MASK << PHY_ACPHY_DCLK1_BIT));
	ddrtrn_reg_write(acphyctl7_tmp, base_phy + DDR_PHY_ACPHYCTL7);
	/* restore acphyctl7 */
	ddrtrn_reg_write(acphyctl7, base_phy + DDR_PHY_ACPHYCTL7);
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/* Set delay value of the bit delay line of the DATA block */
void ddrtrn_hal_phy_set_dq_bdl(unsigned int value)
{
	unsigned int val;
	unsigned int offset;
	unsigned int dq;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_index = ddrtrn_hal_get_cur_byte();
	unsigned int rank = ddrtrn_hal_get_rank_id();
	dq = ddrtrn_hal_get_cur_dq() & 0x7;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE) {
		if (dq < DDR_DQ_NUM_EACH_REG) /* [DXNWDQNBDL0] 4 bdl: wdq0bdl-wdq3bdl */
			offset = ddrtrn_hal_phy_dxnwdqnbdl0(rank, byte_index);
		else /* [DXNWDQNBDL1] 4 bdl: wdq4bdl-wdq7bdl */
			offset = ddrtrn_hal_phy_dxnwdqnbdl1(rank, byte_index);
	} else {
		if (dq < DDR_DQ_NUM_EACH_REG) /* [DXNRDQNBDL0] 4 bdl: rdq0bdl-rdq3bdl */
			offset = ddrtrn_hal_phy_dxnrdqnbdl0(rank, byte_index);
		else /* [DXNRDQNBDL1] 4 bdl: rdq4bdl-rdq7bdl */
			offset = ddrtrn_hal_phy_dxnrdqnbdl1(rank, byte_index);
	}
	dq &= 0x3; /* one register contains 4 dq */
	val = ddrtrn_reg_read(base_phy + offset);
	val &= ~(0xFF << (dq << DDR_DQBDL_SHIFT_BIT));
	val |= ((PHY_BDL_MASK & value) << ((dq << DDR_DQBDL_SHIFT_BIT) + PHY_BDL_DQ_BIT));
	ddrtrn_reg_write(val, base_phy + offset);
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/*
 * Function: ddrtrn_hal_switch_rank_all_phy
 * Description: config phy register 0x48.
 * bit[3:0] set to 0, select rank0;
 * bit[3:0] set to 1, select rank1;
 */
void ddrtrn_hal_switch_rank_all_phy(unsigned int rank_idx)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++)
		ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_phy_addr(i), rank_idx);
}

/* Get value which need to adjust */
int ddrtrn_hal_adjust_get_val(void)
{
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ) {
		return (ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								ddrtrn_hal_phy_dxnrdqsdly(ddrtrn_hal_get_cur_byte())) >>
				PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	} else {
		return (ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())) >>
				PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
	}
}

/* Set value which need to adjust */
void ddrtrn_hal_adjust_set_val(int val)
{
	unsigned int delay;
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ) {
		ddrtrn_hal_set_rdqs(val);
	} else {
		delay = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
		/* clear wdq phase */
		delay = delay & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT));
		ddrtrn_reg_write(delay | ((unsigned int)val << PHY_WDQ_PHASE_BIT),
						 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnwdqdly(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
	}
	ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
}

void ddrtrn_hal_get_dly_value(struct ddrtrn_hal_training_dq_adj *wdq,
							  unsigned int base_phy, unsigned int rank_idx, unsigned int byte_idx)
{
	/* wdqs */
	wdq->wdqsdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	wdq->wdqsbdl = (wdq->wdqsdly >> PHY_WDQS_BDL_BIT) & PHY_WDQS_BDL_MASK;
	wdq->wdqsphase = (wdq->wdqsdly >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK;
	/* wdq */
	wdq->wdqdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));
	wdq->wdqphase = (wdq->wdqdly >> PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
	/* wlsl */
	wdq->dxnwlsl = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwlsl(rank_idx, byte_idx));
	wdq->wlsl = (wdq->dxnwlsl >> PHY_WLSL_BIT) & PHY_WLSL_MASK;
}

void ddrtrn_hal_restore_dly_value(const struct ddrtrn_hal_training_dq_adj *wdq_st,
								  unsigned int base_phy, unsigned int rank_idx, unsigned int byte_idx)
{
	ddrtrn_reg_write(wdq_st->wdqsdly, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	ddrtrn_reg_write(wdq_st->wdqdly, base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));
	ddrtrn_reg_write(wdq_st->dxnwlsl, base_phy + ddrtrn_hal_phy_dxnwlsl(rank_idx, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

void ddrtrn_hal_wdqs_bdl2phase(unsigned int base_phy, unsigned int rank, unsigned int byte_idx)
{
	unsigned int wdqsdly, wdqdly, wdqsbdl, wdqsphase, wdqphase;
	unsigned int wlsl;
	unsigned int phase2bdl;
	ddrtrn_debug("wdqsbdl adjust phy[%x] rank[%x] byte[%x]", base_phy, rank, byte_idx);
	phase2bdl = ((ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >> PHY_RDQS_CYC_BIT) &
				 PHY_RDQS_CYC_MASK) / PHY_WDQSPHASE_NUM_T;
	wdqsdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, byte_idx));
	wdqdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, byte_idx));
	wdqsbdl = (wdqsdly >> PHY_WDQS_BDL_BIT) & PHY_WDQS_BDL_MASK;
	wdqsphase = (wdqsdly >> PHY_WDQS_PHASE_BIT) & PHY_WDQS_PHASE_MASK;
	wdqphase = (wdqdly >> PHY_WDQ_PHASE_BIT) & PHY_WDQ_PHASE_MASK;
	wlsl = (ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx)) >>
			PHY_WLSL_BIT) & PHY_WLSL_MASK; /* [17:16] valid value:0/1/2 */
	if ((wdqsbdl > PHY_WDQS_BDL_MASK) || (phase2bdl == 0))
		return;
	while (wdqsbdl > phase2bdl) {
		if (wdqsphase < PHY_WDQSPHASE_REG_MAX) {
			wdqsbdl -= phase2bdl;
			wdqsphase++;
			/* if bit[1:0] value is 0x3, avoid phase cavity */
			if ((wdqsphase & 0x3) == 0x3) /* 0x3:bit[1:0] */
				wdqsphase++;
		} else if (wlsl < PHY_WLSL_MAX) {
			wdqsbdl -= phase2bdl;
			wlsl++;
			wdqsphase = PHY_WDQSPHASE_REG_MIN;
			if (wdqphase < PHY_WDQPHASE_REG_NUM_T) {
				wdqphase = 0;
			} else {
				wdqphase -= PHY_WDQPHASE_REG_NUM_T;
			}
		} else {
			break;
		}
	}
	wdqsdly = (wdqsdly & (~(PHY_WDQS_BDL_MASK << PHY_WDQS_BDL_BIT))) | (wdqsbdl << PHY_WDQS_BDL_BIT);
	wdqsdly = (wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) | (wdqsphase << PHY_WDQS_PHASE_BIT);
	ddrtrn_reg_write(wdqsdly, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank, byte_idx));
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx)) &
					  (~(PHY_WLSL_MASK << PHY_WLSL_BIT))) | (wlsl << PHY_WLSL_BIT),
					 base_phy + ddrtrn_hal_phy_dxnwlsl(rank, byte_idx));
	ddrtrn_reg_write((wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) | (wdqphase << PHY_WDQ_PHASE_BIT),
					 base_phy + ddrtrn_hal_phy_dxnwdqdly(rank, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

void ddrtrn_hal_training_adjust_wdq(void)
{
	unsigned int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int cur_rank = ddrtrn_hal_get_rank_id();
	struct ddrtrn_hal_training_dq_adj wdq_rank0;
	struct ddrtrn_hal_training_dq_adj wdq_rank1;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_total_byte_num(); i++) {
		ddrtrn_hal_set_cur_byte(i);
		ddrtrn_hal_get_dly_value(&wdq_rank0, base_phy, 0, i);
		ddrtrn_hal_get_dly_value(&wdq_rank1, base_phy, 1, i);
		/* select which rank to adjust */
		ddrtrn_hal_judge_wdq_rank(i, &wdq_rank0, &wdq_rank1);
	}
	ddrtrn_hal_set_rank_id(cur_rank); /* restore to current rank */
	ddrtrn_hal_phy_cfg_update(base_phy);
}

void ddrtrn_hal_training_adjust_wdqs(void)
{
	unsigned int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int cur_rank = ddrtrn_hal_get_rank_id();
	struct ddrtrn_hal_training_dq_adj wdqs_rank0;
	struct ddrtrn_hal_training_dq_adj wdqs_rank1;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_total_byte_num(); i++) {
		ddrtrn_hal_set_cur_byte(i);
		ddrtrn_hal_get_dly_value(&wdqs_rank0, base_phy, 0, i);
		ddrtrn_hal_get_dly_value(&wdqs_rank1, base_phy, 1, i);
		/* select which rank to adjust */
		ddrtrn_hal_judge_wdqs_rank(i, &wdqs_rank0, &wdqs_rank1);
	}
	ddrtrn_hal_set_rank_id(cur_rank); /* restore to current rank */
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/* 2GHz CPU run 2000 "nop" in 1 us */
void ddrtrn_hal_training_delay(unsigned int cnt)
{
	unsigned int tmp = cnt;
	while (tmp--)
		ddr_asm_nop();
}

#ifdef DDR_TRAINING_STAT_CONFIG
/* Save training result in stat register */
static void ddrtrn_hal_save(unsigned int mask, unsigned int phy, int byte, int dq)
{
	unsigned int stat;
	unsigned int phy_index;
	stat = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_STAT);
	/* only record the first error */
	if (stat)
		return;
	stat = mask;
	if (phy != 0) {
		phy_index = (phy == DDR_REG_BASE_PHY0) ? DDR_ERR_PHY0 : DDR_ERR_PHY1;
		stat |= phy_index;
	}
	if (byte != -1)
		stat |= ((unsigned int)byte << DDR_ERR_BYTE_BIT);
	if (dq != -1)
		stat |= ((unsigned int)dq << DDR_ERR_DQ_BIT);
	ddrtrn_reg_write(stat, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_STAT);
}
#endif

/* Record error code in register */
void ddrtrn_hal_training_stat(unsigned int mask, unsigned int phy, int byte, int dq)
{
	ddrtrn_training_error(mask, phy, byte, dq);
#ifdef DDR_TRAINING_STAT_CONFIG
	ddrtrn_hal_save(mask, phy, byte, dq);
#endif
}

static void ddrtrn_hal_rdqs_sync_rdm(int offset)
{
	unsigned int rdqnbdl;
	int rdm;
	rdqnbdl = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
							  ddrtrn_hal_phy_dxnrdqnbdl2(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
	rdm = (rdqnbdl >> PHY_RDM_BDL_BIT) & PHY_RDM_BDL_MASK;
	rdm += offset;
	if (rdm < 0)
		rdm = 0;
	else if (rdm > (int)PHY_RDM_BDL_MAX)
		rdm = PHY_RDM_BDL_MAX;
	rdqnbdl = rdqnbdl & (~(PHY_RDM_BDL_MASK << PHY_RDM_BDL_BIT));
	ddrtrn_reg_write(rdqnbdl | ((unsigned int)rdm << PHY_RDM_BDL_BIT), ddrtrn_hal_get_cur_phy() +
					 ddrtrn_hal_phy_dxnrdqnbdl2(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()));
}

void ddrtrn_hal_rdqs_sync_rank_rdq(int offset)
{
	int dq_val;
	int i;
	unsigned int cur_mode = ddrtrn_hal_get_cur_mode();
	ddrtrn_hal_set_cur_mode(DDR_MODE_READ);
	/* sync other rank rdm */
	ddrtrn_hal_rdqs_sync_rdm(offset);
	/* sync other rank rdq */
	ddrtrn_debug("Before sync rank[%x] byte[%x] dq[%x = %x][%x = %x] offset[%x]",
				 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte(),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 offset);
	for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
		ddrtrn_hal_set_cur_dq(i);
		dq_val = (int)ddrtrn_hal_phy_get_dq_bdl();
		dq_val += offset;
		if (dq_val < 0)
			dq_val = 0;
		else if (dq_val > (int)PHY_BDL_MAX)
			dq_val = PHY_BDL_MAX;
		ddrtrn_hal_phy_set_dq_bdl(dq_val);
	}
	ddrtrn_hal_set_cur_mode(cur_mode); /* restore to current mode */
	ddrtrn_debug("After sync rank[%x] byte[%x] dq[%x = %x][%x = %x]",
				 ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte(),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())),
				 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte()),
				 ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() +
								 ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), ddrtrn_hal_get_cur_byte())));
}

void ddrtrn_hal_rdqbdl_adj(struct ddrtrn_hal_bdl_dly *bdl_dly_s)
{
	int i;
	const int value_num = 10;
	unsigned int rank = ddrtrn_hal_get_rank_id();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_idx = ddrtrn_hal_get_cur_byte();
	unsigned int min = 0xffffffff;
	unsigned int rdm, rdqs;
	unsigned int cur_mode = ddrtrn_hal_get_cur_mode();
	ddrtrn_hal_set_cur_mode(DDR_MODE_READ);
	rdm = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank, byte_idx));
	rdqs = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));
	/* get dq value */
	for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
		ddrtrn_hal_set_cur_dq(i);
		bdl_dly_s->value[i] = ddrtrn_hal_phy_get_dq_bdl();
	}
	bdl_dly_s->value[8] = /* bdl[8]: save rdmbdl */
		(rdm >> PHY_RDM_BDL_BIT) & PHY_RDM_BDL_MASK;
	bdl_dly_s->value[9] = /* bdl[9]: rdqsbdl */
		(rdqs >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	for (i = 0; i < value_num; i++) {
		if (bdl_dly_s->value[i] < min)
			min = bdl_dly_s->value[i];
	}
	/* subtract minimum */
	for (i = 0; i < value_num; i++)
		bdl_dly_s->value[i] = bdl_dly_s->value[i] - min;
	/* set dq value */
	for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
		ddrtrn_hal_set_cur_dq(i);
		ddrtrn_hal_phy_set_dq_bdl(bdl_dly_s->value[i]);
	}
	rdm = (rdm & (~(PHY_RDM_BDL_MASK << PHY_RDM_BDL_BIT))) |
		  (bdl_dly_s->value[8] << PHY_RDM_BDL_BIT); /* bdl[8]: save rdmbdl */
	rdqs = (rdqs & (~(PHY_RDQS_BDL_MASK << PHY_RDQS_BDL_BIT))) |
		   (bdl_dly_s->value[9] << PHY_RDQS_BDL_BIT); /* bdl[9]: rdqsbdl */
	ddrtrn_reg_write(rdm, base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank, byte_idx));
	ddrtrn_reg_write(rdqs, base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));
	ddrtrn_hal_set_cur_mode(cur_mode); /* restore to current mode */
}

void ddrtrn_hal_rdqs_sync(int val)
{
	unsigned int rdqsdly;
	unsigned int cur_rank = ddrtrn_hal_get_rank_id();
	int old, offset;
	rdqsdly = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(ddrtrn_hal_get_cur_byte()));
	old = (rdqsdly >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	offset = val - old;
	/* sync rdm */
	ddrtrn_hal_rdqs_sync_rank_rdq(offset);
	if (ddrtrn_hal_get_cur_phy_rank_num() == 1) {
		ddrtrn_debug("Rank number[%x] not need sync another rank",
					 ddrtrn_hal_get_cur_phy_rank_num());
		return;
	}
	/* sync other rank rdm and rdq */
	ddrtrn_hal_set_rank_id(DDR_SUPPORT_RANK_MAX - 1 - cur_rank); /* switch to another rank */
	ddrtrn_hal_rdqs_sync_rank_rdq(offset);
	ddrtrn_hal_set_rank_id(cur_rank); /* resotre to cur rank */
}

/* Save two rank RDET result */
void ddrtrn_hal_save_rdqbdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data)
{
	unsigned int byte_idx;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		rank_dcc_data->dq03[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank_idx, byte_idx));
		rank_dcc_data->dq47[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank_idx, byte_idx));
		rank_dcc_data->rdm[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank_idx, byte_idx));
		rank_dcc_data->rdqs[byte_idx] =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));
		ddrtrn_debug("rank[%x] dq03[%x] dq47[%x] rdm[%x] rdqs[%x]", rank_idx,
			rank_dcc_data->dq03[byte_idx],
			rank_dcc_data->dq47[byte_idx],
			rank_dcc_data->rdm[byte_idx],
			rank_dcc_data->rdqs[byte_idx]);
	}
}

/* Restore two rank RDET result */
void ddrtrn_hal_restore_rdqbdl(struct ddrtrn_hal_trianing_dq_data *rank_dcc_data)
{
	unsigned int byte_idx;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_reg_write(rank_dcc_data->dq03[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->dq47[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->rdm[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqnbdl2(rank_idx, byte_idx));
		ddrtrn_reg_write(rank_dcc_data->rdqs[byte_idx],
			base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx));
	}
}
