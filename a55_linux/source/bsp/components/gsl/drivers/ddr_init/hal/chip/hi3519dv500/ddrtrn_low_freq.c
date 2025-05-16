/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_low_freq.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_LOW_FREQ_CONFIG
static void ddrtrn_hal_phy_ctrl_cfg(struct ddrtrn_phy_ctrl *phy_ctrl_st)
{
	/* do nothing */
	return;
}

static void ddrtrn_hal_phy_ctrl_restore(const struct ddrtrn_phy_ctrl *phy_ctrl_st)
{
	/* do nothing */
	return;
}

static unsigned int ddrtrn_hal_low_freq_get_dficlk_ratio(unsigned int base_phy)
{
	return (ddrtrn_reg_read(base_phy + DDR_PHY_PHYCTRL0) >> PHY_DFICLK_RATIO_BIT) &
		   PHY_DFICLK_RATIO_MASK;
}

static unsigned int ddrtrn_hal_low_freq_get_dpll_cfg0(void)
{
	return ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDR_PLL_CFG0);
}

static unsigned int ddrtrn_hal_low_freq_get_dpll_cfg1(void)
{
	return ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDR_PLL_CFG1);
}

static void ddrtrn_hal_low_freq_phy_clk(unsigned int clk_status)
{
	unsigned int gated;
	unsigned int base_phy;
	unsigned int phy_idx;
	/* bit[31], bit[14:0] */
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
			continue;
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		gated = ddrtrn_reg_read(base_phy + DDR_PHY_CLKGATED);
		ddrtrn_reg_write((gated & (~PHY_CLKGATED_MASK)) | clk_status,
						 base_phy + DDR_PHY_CLKGATED);
	}
}

/* config DDR PLL power down or power up */
static void ddrtrn_hal_low_freq_pll_power(unsigned int pll_status)
{
	unsigned int pll_ctrl;
	unsigned int base_phy;
	unsigned int phy_idx;
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
			continue;
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		pll_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_PLLCTRL);
		ddrtrn_reg_write((pll_ctrl & (~(PHY_PLL_PWDN_MASK << PHY_PLL_PWDN_BIT))) |
						 (pll_status << PHY_PLL_PWDN_BIT), base_phy + DDR_PHY_PLLCTRL);
	}
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US); /* wait 10us */
}

static int ddrtrn_pll_lock(void)
{
	int lock = 1;
	unsigned int base_phy;
	unsigned int phy_idx;
	unsigned int ac_pll_lock;

	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		ac_pll_lock = (ddrtrn_reg_read(base_phy + DDR_PHY_PLLCTRL_AC) >> PHY_AC_PLL_LOCK_BIT) & 0x1;
		lock &= ac_pll_lock;
	}
	return lock;
}

/* enter or exit auto self-refresh when ddr low freq start */
static int ddrtrn_hal_low_freq_ctrl_easr(struct ddrtrn_dmc_cfg_sref *cfg_sref, unsigned int sref_req)
{
	unsigned int i;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return -1;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
			ddrtrn_sref_cfg_save(cfg_sref);
			ddrtrn_sref_cfg(cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD); /* bit[3:2] 0x3 */
		}
		if (ddrtrn_training_ctrl_easr(sref_req))
			return -1;
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			ddrtrn_sref_cfg_restore(cfg_sref);
	}
	return 0;
}

static void ddrtrn_hal_get_ck_phase(unsigned int base_phy, struct ddrtrn_ck_phase *ck_phase)
{
	unsigned int acphyctl7;
	acphyctl7 = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL7);
	ck_phase->ck_phase[0] = ((acphyctl7 >> PHY_ACPHY_DCLK0_BIT) & PHY_ACPHY_DCLK_MASK) |
							(((acphyctl7 >> PHY_ACPHY_DRAMCLK0_BIT) & PHY_ACPHY_DRAMCLK_MASK) << PHY_ACPHY_DRAMCLK_EXT_BIT);
	ck_phase->ck_phase[1] = ((acphyctl7 >> PHY_ACPHY_DCLK1_BIT) & PHY_ACPHY_DCLK_MASK) |
							(((acphyctl7 >> PHY_ACPHY_DRAMCLK1_BIT) & PHY_ACPHY_DRAMCLK_MASK) << PHY_ACPHY_DRAMCLK_EXT_BIT);
}

static void ddrtrn_hal_clear_wdqs_register(unsigned int base_phy, unsigned int rank_idx,
										   unsigned int byte_idx)
{
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

static void ddrtrn_hal_get_wdqs_average_in_lowfreq_by_rank(const struct ddrtrn_delay_ckph *delay_ck,
														   struct ddrtrn_hal_training_dq_adj_all *dq_adj_all, int cycle)
{
	int wdqsphase;
	unsigned int byte_idx;
	unsigned int wdqsdly_lowfreq;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int cur_phy = ddrtrn_hal_get_cur_phy();
	struct ddrtrn_hal_training_dq_adj *byte_dq_st = NULL;
	if (ddrtrn_hal_get_cur_phy_total_byte_num() > DDR_PHY_BYTE_MAX)
		return;
	for (byte_idx = 0; byte_idx < ddrtrn_hal_get_cur_phy_total_byte_num(); byte_idx++) {
		byte_dq_st = &dq_adj_all->phy_dq_adj[phy_idx].rank_dq_adj[rank_idx].byte_dq_adj[byte_idx];
		ddrtrn_hal_get_dly_value(byte_dq_st, cur_phy, rank_idx, byte_idx);
		wdqsphase = (int)(byte_dq_st->wdqsphase);
		wdqsphase = wdqsphase - (wdqsphase + 1) / 4; /* 4 wdqphase */
		if ((wdqsphase > PHY_WDQS_PHASE_MASK) || (byte_dq_st->wlsl > PHY_WLSL_MASK))
			return;
		if (delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] > PHY_RDQS_CYC_MASK)
			return;
		if (delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx] >
				(PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T))
			return;
		wdqsdly_lowfreq = (unsigned int)(wdqsphase * (int)(delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx]) +
										 (int)(byte_dq_st->wdqsbdl) + ((int)(byte_dq_st->wlsl) - 1) *
										 (int)(delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx]));
		byte_dq_st->wdqs_sum += wdqsdly_lowfreq;
		/* get wdqs average */
		if (cycle == (WL_TIME - 1))
			byte_dq_st->wdqs_average = byte_dq_st->wdqs_sum / WL_TIME;
		/* set wdq register value to 0 */
		ddrtrn_hal_clear_wdqs_register(cur_phy, rank_idx, byte_idx);
	}
}

static void ddrtrn_hal_get_wdqs_average_in_lowfreq_by_phy(const struct ddrtrn_delay_ckph *delay_ck,
														  struct ddrtrn_hal_training_dq_adj_all *dq_adj_all, int cycle)
{
	unsigned int rank_idx;
	for (rank_idx = 0; rank_idx < ddrtrn_hal_get_cur_phy_rank_num(); rank_idx++) {
		ddrtrn_hal_set_rank_id(rank_idx);
		ddrtrn_hal_get_wdqs_average_in_lowfreq_by_rank(delay_ck, dq_adj_all, cycle);
	}
}

static int ddrtrn_hal_get_wdqs_average_in_lowfreq(const struct ddrtrn_delay_ckph *delay_ck,
												  struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	int result = 0;
	int i;
	unsigned int phy_idx;
	for (i = 0; i < WL_TIME; i++) {
		ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_WL_EN); /* bit[4] bit[0] */
		result += ddrtrn_hw_training();
		for (phy_idx = 0; phy_idx < DDR_PHY_NUM; phy_idx++) {
			ddrtrn_hal_set_phy_id(phy_idx);
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_cur_phy_addr());
			if (ddrtrn_hal_get_rank_item_hw(phy_idx, 0) == 0)
				continue;
			ddrtrn_hal_get_wdqs_average_in_lowfreq_by_phy(delay_ck, dq_adj_all, i);
		}
	}
	return result;
}

/* get ck_phase and rdqscyc in low freq */
static void ddrtrn_hal_freq_change_get_val_in_lowfreq(struct ddrtrn_delay_ckph *delay_ck)
{
	unsigned int phy_idx, byte_idx;
	unsigned int base_phy;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return;
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		ddrtrn_hal_get_ck_phase(base_phy, &(delay_ck->tr_delay_ck[phy_idx].ckph_st[0])); /* 0: in low freq */
		if (ddrtrn_hal_get_phy_total_byte_num(phy_idx) > DDR_PHY_BYTE_MAX)
			return;
		for (byte_idx = 0; byte_idx < ddrtrn_hal_get_phy_total_byte_num(phy_idx); byte_idx++) {
			delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] =
				(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				 PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK;
			delay_ck->tr_delay_ck[phy_idx].phase2bdl_lowfreq[byte_idx] =
				delay_ck->tr_delay_ck[phy_idx].rdqscyc_lowfreq[byte_idx] / PHY_WDQSPHASE_NUM_T;
		}
	}
}

static void ddrtrn_hal_freq_change_get_val_in_highfreq(struct ddrtrn_delay_ckph *delay_ck)
{
	unsigned int phy_idx, byte_idx;
	unsigned int base_phy;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return;
	for (phy_idx = 0; phy_idx < ddrtrn_hal_get_phy_num(); phy_idx++) {
		base_phy = ddrtrn_hal_get_phy_addr(phy_idx);
		ddrtrn_hal_get_ck_phase(base_phy, &(delay_ck->tr_delay_ck[phy_idx].ckph_st[1])); /* 1: in high freq */
		if (ddrtrn_hal_get_phy_total_byte_num(phy_idx) > DDR_PHY_BYTE_MAX)
			return;
		for (byte_idx = 0; byte_idx < ddrtrn_hal_get_phy_total_byte_num(phy_idx); byte_idx++) {
			delay_ck->tr_delay_ck[phy_idx].rdqscyc_highfreq[byte_idx] =
				(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(byte_idx)) >>
				 PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK;
			delay_ck->tr_delay_ck[phy_idx].phase2bdl_highfreq[byte_idx] =
				delay_ck->tr_delay_ck[phy_idx].rdqscyc_highfreq[byte_idx] / PHY_WDQSPHASE_NUM_T;
		}
	}
}

static void ddrtrn_hal_freq_change_set_wdqs_val_process(const struct ddrtrn_delay_ckph *delay_ck,
														const struct ddrtrn_hal_training_dq_adj *byte_dq_st, unsigned int wlsl, int wdqsdly_highfreq)
{
	unsigned int wdqsdly;
	unsigned int wdqsphase;
	unsigned int wdqsbdl;
	unsigned int wdqphase;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	unsigned int byte_idx = ddrtrn_hal_get_cur_byte();
	wdqsphase = 0;
	wdqsdly = byte_dq_st->wdqsdly;
	wdqsbdl = (unsigned int)wdqsdly_highfreq;
	while ((wdqsbdl > delay_ck->tr_delay_ck[ddrtrn_hal_get_phy_id()].phase2bdl_highfreq[byte_idx]) &&
			(wdqsphase < (PHY_WDQSPHASE_REG_MAX - 8))) { /* 8: 1/2T */
		wdqsbdl = wdqsbdl - delay_ck->tr_delay_ck[ddrtrn_hal_get_phy_id()].phase2bdl_highfreq[byte_idx];
		wdqsphase++;
		/* if bit[1:0] value is 0x3, avoid phase cavity */
		if ((wdqsphase & 0x3) == 0x3) /* 0x3:bit[1:0] */
			wdqsphase++;
	}
	wdqphase = 0;
	wdqsdly = (wdqsdly & (~(PHY_WDQS_BDL_MASK << PHY_WDQS_BDL_BIT))) | (wdqsbdl << PHY_WDQS_BDL_BIT);
	wdqsdly = (wdqsdly & (~(PHY_WDQS_PHASE_MASK << PHY_WDQS_PHASE_BIT))) |
			  (wdqsphase << PHY_WDQS_PHASE_BIT);
	ddrtrn_reg_write(wdqsdly, base_phy + ddrtrn_hal_phy_dxwdqsdly(rank_idx, byte_idx));
	ddrtrn_reg_write((byte_dq_st->dxnwlsl & (~(PHY_WLSL_MASK << PHY_WLSL_BIT))) | (wlsl << PHY_WLSL_BIT),
					 base_phy + ddrtrn_hal_phy_dxnwlsl(rank_idx, byte_idx));
	ddrtrn_reg_write((byte_dq_st->wdqdly & (~(PHY_WDQ_PHASE_MASK << PHY_WDQ_PHASE_BIT))) |
					 (wdqphase << PHY_WDQ_PHASE_BIT),
					 base_phy + ddrtrn_hal_phy_dxnwdqdly(rank_idx, byte_idx));
	/* set wdq */
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank_idx, byte_idx));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank_idx, byte_idx));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

static void ddrtrn_hal_freq_change_set_wdqs_val_by_rank(const struct ddrtrn_delay_ckph *delay_ck,
														const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int byte_idx;
	unsigned int wlsl;
	unsigned int clk_index;
	unsigned int ck_phase_freq[PHY_ACCTL_DCLK_NUM]; /* [0]:lowfreq [1]:highfreq */
	int wdqsdly_highfreq;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int rank_idx = ddrtrn_hal_get_rank_id();
	const struct ddrtrn_delay_ckph_phy *tr_delay_ck = &delay_ck->tr_delay_ck[phy_idx];
	if (ddrtrn_hal_get_cur_phy_total_byte_num() > DDR_PHY_BYTE_MAX)
		return;
	for (byte_idx = 0; byte_idx < ddrtrn_hal_get_cur_phy_total_byte_num(); byte_idx++) {
		ddrtrn_hal_set_cur_byte(byte_idx);
		const struct ddrtrn_hal_training_dq_adj *byte_dq_st =
				&dq_adj_all->phy_dq_adj[phy_idx].rank_dq_adj[rank_idx].byte_dq_adj[byte_idx];
		wlsl = 0x1;
		clk_index = (byte_idx < 2) ? 0 : 1; /* 2: byte 0/1 */
		ck_phase_freq[0] = tr_delay_ck->ckph_st[0].ck_phase[clk_index] -
						   (tr_delay_ck->ckph_st[0].ck_phase[clk_index] + 1) / 4; /* 4 wdqsphase */
		ck_phase_freq[1] = tr_delay_ck->ckph_st[1].ck_phase[clk_index] -
						   (tr_delay_ck->ckph_st[1].ck_phase[clk_index] + 1) / 4; /* 4 wdqsphase */
		if (ck_phase_freq[0] > CK_PHASE_MAX || ck_phase_freq[1] > CK_PHASE_MAX)
			return;
		if (tr_delay_ck->phase2bdl_lowfreq[byte_idx] > (PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T) ||
				tr_delay_ck->phase2bdl_highfreq[byte_idx] > (PHY_RDQS_CYC_MASK / PHY_WDQSPHASE_NUM_T))
			return;
		wdqsdly_highfreq = (int)(ck_phase_freq[1] * tr_delay_ck->phase2bdl_highfreq[byte_idx]) -
						   ((int)(ck_phase_freq[0] * tr_delay_ck->phase2bdl_lowfreq[byte_idx]) - byte_dq_st->wdqs_average);
		/* wdqs back 120 */
		wdqsdly_highfreq -= (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]) / 3; /* 1/3 T */
		while ((wdqsdly_highfreq < 0) && (wlsl > 0)) {
			wdqsdly_highfreq = wdqsdly_highfreq + (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]);
			wlsl--;
		}
		while ((wdqsdly_highfreq >= (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx])) &&
				(wlsl < 2)) { /* 2: wlsl val 0/1/2 */
			wdqsdly_highfreq = wdqsdly_highfreq - (int)(tr_delay_ck->rdqscyc_highfreq[byte_idx]);
			wlsl++;
		}
		if (wdqsdly_highfreq < 0)
			wdqsdly_highfreq = 0;
		else if (wdqsdly_highfreq > PHY_BDL_MASK)
			wdqsdly_highfreq = PHY_BDL_MASK;
		ddrtrn_hal_freq_change_set_wdqs_val_process(delay_ck, byte_dq_st, wlsl, wdqsdly_highfreq);
	}
}

static void ddrtrn_hal_freq_change_set_wdqs_val_by_phy(const struct ddrtrn_delay_ckph *delay_ck,
													   const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_rank_num(); i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_freq_change_set_wdqs_val_by_rank(delay_ck, dq_adj_all);
	}
}

static void ddrtrn_hal_freq_change_set_wdqs_val(const struct ddrtrn_delay_ckph *delay_ck,
												const struct ddrtrn_hal_training_dq_adj_all *dq_adj_all)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		if (ddrtrn_hal_get_rank_item_hw(i, 0) == 0)
			continue;
		ddrtrn_hal_freq_change_set_wdqs_val_by_phy(delay_ck, dq_adj_all);
	}
}

/* Configure ddr frequency */
static void ddrtrn_hal_low_freq_cfg_freq_process(unsigned int dpll_cfg1_init_val, unsigned int dpll_cfg0_init_val)
{
	unsigned int soc_2_fre;
	unsigned int soc_freq_conf;
	unsigned int ddraxi_sc_seled;
	unsigned int gpll_ssmod_ctrl;
	soc_2_fre = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDR_CKSEL);
	soc_freq_conf = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	gpll_ssmod_ctrl = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_GPLL_SSMOD_CTRL);
	/* DDR switch to 24MHZ */
	ddrtrn_reg_write((soc_2_fre & (~(DDR_CKSEL_MASK << DDR_CKSEL_BIT))) | (CRG_DDR_CKSEL_24M << DDR_CKSEL_BIT),
					 CRG_REG_BASE_ADDR + CRG_DDR_CKSEL);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US); /* wait 10us */
	/* ddraxi cksel switch to 24MHZ */
	soc_freq_conf = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	ddrtrn_reg_write((soc_freq_conf & (~(DDR_AXI_CKSEL_MASK << DDR_AXI_CKSEL_BIT))) |
					 (CRG_DDRAXI_CKSEL_24M << DDR_AXI_CKSEL_BIT),
					 CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US); /* wait 10us */
	/* check ddr axi clock stat */
	ddraxi_sc_seled = (ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_SOC_FREQ_INDICATE) >>
					   DDR_AXI_SC_SELED_BIT) & DDR_AXI_SC_SELED_MASK;
	if (ddraxi_sc_seled != CRG_DDRAXI_CKSEL_24M)
		return;
	/* ssmod disble */
	ddrtrn_reg_write((gpll_ssmod_ctrl | (0x1 << SSMOD_DISABLE_BIT)) | (0x1 << SSMOD_CKEN_BIT),
					 CRG_REG_BASE_ADDR + CRG_GPLL_SSMOD_CTRL);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_50US); /* wait 50us */
	/* ssmod ck close */
	ddrtrn_reg_write((gpll_ssmod_ctrl | (0x1 << SSMOD_DISABLE_BIT)) & (~(0x1 << SSMOD_CKEN_BIT)),
					 CRG_REG_BASE_ADDR + CRG_GPLL_SSMOD_CTRL);
	/* Configure DPLL */
	ddrtrn_reg_write(dpll_cfg1_init_val | (0x1 << DDR_DPLL_PD_BIT), /* bit[20] power down */
					 CRG_REG_BASE_ADDR + CRG_DDR_PLL_CFG1);
	ddrtrn_reg_write(dpll_cfg0_init_val, CRG_REG_BASE_ADDR + CRG_DDR_PLL_CFG0);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_1US); /* wait 1us */
	ddrtrn_reg_write(dpll_cfg1_init_val & (~(0x1 << DDR_DPLL_PD_BIT)), /* bit[20] power up */
					 CRG_REG_BASE_ADDR + CRG_DDR_PLL_CFG1);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_100US); /* wait 100us */
	/* restore ssmod ck */
	ddrtrn_reg_write((gpll_ssmod_ctrl | (0x1 << SSMOD_DISABLE_BIT)),
					 CRG_REG_BASE_ADDR + CRG_GPLL_SSMOD_CTRL);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_1US); /* wait 1us */
	/* restore ssmod disable */
	ddrtrn_reg_write(gpll_ssmod_ctrl, CRG_REG_BASE_ADDR + CRG_GPLL_SSMOD_CTRL);
	/* restore  CRG_DDR_CKSEL and CRG_SOC_FREQ_CONFIG */
	ddrtrn_reg_write(soc_freq_conf, CRG_REG_BASE_ADDR + CRG_SOC_FREQ_CONFIG);
	ddrtrn_reg_write(soc_2_fre, CRG_REG_BASE_ADDR + CRG_DDR_CKSEL);
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_10US);
}

static void ddrtrn_low_freq_cfg_freq(unsigned int base_phy)
{
	unsigned int dficlk_ratio;
	unsigned int dpll_cfg1;
	unsigned int dpll_cfg0;
	/* dramCK Clock Ratio */
	dficlk_ratio = ddrtrn_hal_low_freq_get_dficlk_ratio(base_phy);
	switch (dficlk_ratio) {
	case 0x0: /* 1:1 */
		dpll_cfg1 = CRG_PLL7_TMP_1TO1; /* PLL 1600MHz */
		dpll_cfg0 = CRG_PLL6_TMP_1TO1;
		break;
	case 0x1: /* 1:2 */
		dpll_cfg1 = CRG_PLL7_TMP_1TO2; /* PLL 800MHz */
		dpll_cfg0 = CRG_PLL6_TMP_1TO2;
		break;
	case 0x2: /* 1:4 */
	/* Fall through to PLL 400MHz */
	default:
		dpll_cfg1 = CRG_PLL7_TMP_1TO4; /* PLL 400MHz */
		dpll_cfg0 = CRG_PLL6_TMP_1TO4;
		break;
	}
	/* Configure DDR low frequency */
	ddrtrn_hal_low_freq_cfg_freq_process(dpll_cfg1, dpll_cfg0);
}

int ddrtrn_low_freq_start(unsigned int hw_item_mask)
{
	int result;
	unsigned int i;
	unsigned int pll_loop_number, pll_lock_loop_number;
	unsigned int dpll_cfg0_init_val, dpll_cfg1_init_val;
	struct ddrtrn_dmc_cfg_sref cfg_sref;
	struct ddrtrn_hal_training_dq_adj_all dq_adj_all;
	struct ddrtrn_delay_ckph delay_ck;
	struct ddrtrn_phy_ctrl phy_ctrl_st;
	struct ddrtrn_dq_data_phy dq_phy;
	ddrtrn_set_data(&dq_adj_all, 0, sizeof(struct ddrtrn_hal_training_dq_adj_all));
	/* Save DERT default result: rdq/rdqs/rdm/ bdl */
	ddrtrn_save_rdqbdl_phy(&dq_phy);
	dpll_cfg1_init_val = ddrtrn_hal_low_freq_get_dpll_cfg1();
	dpll_cfg0_init_val = ddrtrn_hal_low_freq_get_dpll_cfg0();
	/* ddr phy PHYCTRL1/DXPHYCTRL cfg */
	ddrtrn_hal_phy_ctrl_cfg(&phy_ctrl_st);

	/* close phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_CLOSE);

	pll_loop_number = 0;
	while (pll_loop_number < DDR_PLL_LOOP_NUM) {
		pll_lock_loop_number = 0;
		/* DDR PLL power down */
		ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_DOWN);
		/* Configure DDR low frequency, does not distinguish PHY */
		ddrtrn_low_freq_cfg_freq(ddrtrn_hal_get_phy_addr(0));
		/* DDR PLL power up */
		ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_UP);

		/* AC PLL LOCK status */
		while ((ddrtrn_pll_lock() == 0) && (pll_lock_loop_number < DDR_PLL_LOCK_TIME)) {
			ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_1US); /* wait 1us */
			pll_lock_loop_number++;
		}
		if (pll_lock_loop_number < DDR_PLL_LOCK_TIME) {
			break;
		}
		pll_loop_number++;
	}
	if (pll_loop_number == DDR_PLL_LOOP_NUM) {
		log_serial_puts((const s8 *)"\nDDR PLL LOCK Fail.\n");
	}

	/* open phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_OPEN);

	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++)
		ddrtrn_hal_ck_cfg(ddrtrn_hal_get_phy_addr(i));

	/* Perform partial initialization of DDR */
	ddrtrn_hal_hw_item_cfg(DDR_FROM_VALUE0 & hw_item_mask);
	result = ddrtrn_hw_training();
	/* get value in low freq */
	ddrtrn_hal_freq_change_get_val_in_lowfreq(&delay_ck);
	ddrtrn_hal_get_wdqs_average_in_lowfreq(&delay_ck, &dq_adj_all);
	/* enter auto self-refresh */
	if (ddrtrn_hal_low_freq_ctrl_easr(&cfg_sref, DDR_ENTER_SREF))
		return -1;
	/* close phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_CLOSE);

	pll_loop_number = 0;
	while (pll_loop_number < DDR_PLL_LOOP_NUM) {
		pll_lock_loop_number = 0;
		/* DDR PLL power down */
		ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_DOWN);
		/* Configure DDR high frequency */
		ddrtrn_hal_low_freq_cfg_freq_process(dpll_cfg1_init_val, dpll_cfg0_init_val);
		/* restore ddr PHYCTRL1/DXPHYCTRL */
		ddrtrn_hal_phy_ctrl_restore(&phy_ctrl_st);
		/* DDR PLL power up */
		ddrtrn_hal_low_freq_pll_power(PHY_PLL_POWER_UP);

		/* AC PLL LOCK status */
		while ((ddrtrn_pll_lock() == 0) && (pll_lock_loop_number < DDR_PLL_LOCK_TIME)) {
			ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_1US); /* wait 1us */
			pll_lock_loop_number++;
		}
		if (pll_lock_loop_number < DDR_PLL_LOCK_TIME) {
			break;
		}
		pll_loop_number++;
	}
	if (pll_loop_number == DDR_PLL_LOOP_NUM) {
		log_serial_puts((const s8 *)"\nDDR PLL LOCK Fail.\n");
	}

	/* open phy clock gated */
	ddrtrn_hal_low_freq_phy_clk(PHY_CLK_GATED_OPEN);
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++)
		ddrtrn_hal_ck_cfg(ddrtrn_hal_get_phy_addr(i));
	/* exit auto self-refresh */
	if (ddrtrn_hal_low_freq_ctrl_easr(&cfg_sref, DDR_EXIT_SREF))
		return -1;
	ddrtrn_hal_hw_item_cfg((PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_DLYMEAS_EN) & hw_item_mask); /* bit[2][0] */
	result += ddrtrn_hw_training();
	/* get rdqscyc in high freq */
	ddrtrn_hal_freq_change_get_val_in_highfreq(&delay_ck);
	ddrtrn_hal_freq_change_set_wdqs_val(&delay_ck, &dq_adj_all);
	/* restore two rank: rdq/rdqs/rdm/ bdl */
	ddrtrn_restore_rdqbdl_phy(&dq_phy);
	/* Perform partial initialization of DDR */
	ddrtrn_hal_hw_item_cfg(DDR_FROM_VALUE1 & hw_item_mask);
	result += ddrtrn_hw_training();
	return result;
}
#else
int ddrtrn_low_freq_start(unsigned int hw_item_mask)
{
	return 0;
}
#endif /* DDR_LOW_FREQ_CONFIG */