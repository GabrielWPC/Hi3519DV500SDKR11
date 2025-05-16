/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_suspend_resume.h"
#include "ddrtrn_training.h"

#ifdef DDR_WAKEUP_CONFIG
static struct ddrtrn_hal_phy_param_s g_ddrtrn_hal_phy_param[] = {
	/* reg addr, reg number, addr to save parameter */
	{0x028, 1, lpds_ddrtrn_hal_phy_parab(169)},
	{0x174, 1, lpds_ddrtrn_hal_phy_parab(170)},
	{0x120, 1, lpds_ddrtrn_hal_phy_parab(171)},
	{0x1d4, 1, lpds_ddrtrn_hal_phy_parab(172)},
	{0x128, 1, lpds_ddrtrn_hal_phy_parab(173)},
	{0x140, 5, lpds_ddrtrn_hal_phy_parab(174)},
	{0x158, 1, lpds_ddrtrn_hal_phy_parab(179)},

	{0x210, 13, lpds_ddrtrn_hal_phy_parab(180)},
	{0x290, 13, lpds_ddrtrn_hal_phy_parab(193)},
	{0x310, 13, lpds_ddrtrn_hal_phy_parab(206)},
	{0x390, 13, lpds_ddrtrn_hal_phy_parab(219)},
	{0x610, 7, lpds_ddrtrn_hal_phy_parab(232)},
	{0x630, 5, lpds_ddrtrn_hal_phy_parab(239)},
	{0x690, 7, lpds_ddrtrn_hal_phy_parab(244)},
	{0x6b0, 5, lpds_ddrtrn_hal_phy_parab(251)},

	/* 0x1800 */
	{0x710, 7, lpds_hpp_reseved_areac(169)},
	{0x730, 5, lpds_hpp_reseved_areac(176)},
	{0x790, 7, lpds_hpp_reseved_areac(181)},
	{0x7b0, 5, lpds_hpp_reseved_areac(188)},

	{0x270, 2, lpds_hpp_reseved_areac(193)},
	{0x2f0, 2, lpds_hpp_reseved_areac(195)},
	{0x370, 2, lpds_hpp_reseved_areac(197)},
	{0x3f0, 2, lpds_hpp_reseved_areac(199)},

	{0x674, 1, lpds_hpp_reseved_areac(201)},
	{0x6f4, 1, lpds_hpp_reseved_areac(202)},
	{0x774, 1, lpds_hpp_reseved_areac(203)},
	{0x7f4, 1, lpds_hpp_reseved_areac(204)},

	{0x80c, 5, lpds_hpp_reseved_areac(205)},
	{0x820, 5, lpds_hpp_reseved_areac(210)},
	{0x834, 5, lpds_hpp_reseved_areac(215)},
	{0x848, 5, lpds_hpp_reseved_areac(220)},
	{0x85c, 5, lpds_hpp_reseved_areac(225)},
	{0x870, 5, lpds_hpp_reseved_areac(230)},
	{0x884, 5, lpds_hpp_reseved_areac(235)},
	{0x898, 5, lpds_hpp_reseved_areac(240)},

	{0x1f4, 1, lpds_hpp_reseved_areac(245)},
	{0x10a4, 1, lpds_hpp_reseved_areac(246)},

	{0x270, 1, lpds_hpp_reseved_areac(247)},
	{0x2f0, 1, lpds_hpp_reseved_areac(248)},
	{0x370, 1, lpds_hpp_reseved_areac(249)},
	{0x3f0, 1, lpds_hpp_reseved_areac(250)}
};

/*
 * Function: ddrtrn_imp_status1_cfg
 * Description: IMP_STATUS1 register bit[15:0] can not be writed
 *              the value of bit[15:0] set to bit[31:16]
 */
static unsigned int ddrtrn_imp_status1_cfg(unsigned int addr, unsigned int value)
{
	if (addr == DDR_PHY_IMP_STATUS1)
		value = (value & (~(PHY_IMP_STATUS1_USED_MASK << PHY_IMP_STATUS1_USED_BIT))) |
				((value & PHY_IMP_STATUS1_USED_MASK) << PHY_IMP_STATUS1_USED_BIT); /* bit[15:0] set to bit[31:16] */
	return value;
}

/*
 * Function: ddrtrn_imp_ctrl1_cfg
 * Description: config phy register 0x170 when resume.
 *              bit8 set to 0
 */
static void ddrtrn_imp_ctrl1_cfg(void)
{
	unsigned int i;
	unsigned int imp_ctrl1;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM) {
		ddrtrn_error("loop upper limit ddrtrn_hal_get_phy_num() out of range");
		return;
	}
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		imp_ctrl1 = ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_IMP_CTRL1);
		/* ac_vddq_cal_en set to 0 */
		ddrtrn_reg_write(imp_ctrl1 & (~(0x1 << PHY_AC_VDDQ_CAL_EN_BIT)),
						 ddrtrn_hal_get_cur_phy() + DDR_PHY_IMP_CTRL1);
	}
}

/* RDET/WDET Method Selection */
static void ddrtrn_hal_det_mod_sel(void)
{
	unsigned int i;
	unsigned int base_phy;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM) {
		ddrtrn_error("loop upper limit ddrtrn_hal_get_phy_num() out of range");
		return;
	}
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		base_phy = ddrtrn_hal_get_phy_addr(i);
		/* rdet by MPC_R */
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL2) | (0x1 << PHY_RDET_METHOD_SEL_BIT),
						 base_phy + DDR_PHY_TRAINCTRL2);
		/* wdet by MPC_R/W */
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL3) | (0x1 << PHY_NOPRE4WRDET_BIT) |
						 (0x1 << PHY_WDET_METROD_SEL_BIT), base_phy + DDR_PHY_TRAINCTRL3);
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL5) | (0x1 << PHY_TRAINCTL_NOPOSTPRE_BIT),
						 base_phy + DDR_PHY_TRAINCTRL5);
	}
}

/* Description: store DDR PHY register paraments for normal boot */
void ddrtrn_suspend_store_para(void)
{
	unsigned int i, j;
	unsigned int tmp_value;
	unsigned int rank_idx = 0;
	unsigned int offset = 0;
	ddrtrn_hal_cfg_init();
	ddrtrn_hal_switch_rank_all_phy(0x0); /* 0x0: switch to rank0 */
	for (i = 0; i < sizeof(g_ddrtrn_hal_phy_param) / sizeof(struct ddrtrn_hal_phy_param_s); i++) {
		if (g_ddrtrn_hal_phy_param[i].reg_addr == ddrtrn_hal_phy_dvreft_status(0)) { /* DVREFT_STATUS */
			ddrtrn_hal_switch_rank_all_phy(rank_idx);
			rank_idx = 0x1;
		}
		for (j = 0; j < g_ddrtrn_hal_phy_param[i].reg_num; j++) {
			/* save ddr phy0 parameters */
			tmp_value = ddrtrn_reg_read(DDR_REG_BASE_PHY0 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
			ddrtrn_reg_write(tmp_value, DATA_PHY_BASE_ADDR + lpds_ddrtrn_hal_phy_parab(j) + offset);
			/* save ddr phy1 parameters */
			tmp_value = ddrtrn_reg_read(DDR_REG_BASE_PHY1 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
			ddrtrn_reg_write(tmp_value, DATA_PHY_BASE_ADDR + lpds_hpp_reseved_areac(j) + offset);
			/* save ddr phy2 parameters */
			tmp_value = ddrtrn_reg_read(DDR_REG_BASE_PHY2 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
			ddrtrn_reg_write(tmp_value, DATA_PHY_BASE_ADDR + g_ddrtrn_hal_phy_param[i].save_addr + j * DDR_REG_OFFSET);
		}
		offset += g_ddrtrn_hal_phy_param[i].reg_num * DDR_REG_OFFSET;
	}
	ddrtrn_hal_switch_rank_all_phy(0x0); /* 0x0: switch to rank0 */
}

/* Description: restore DDR PHY register paraments before resume */
static void ddrtrn_resume_restore_para(void)
{
	unsigned int i, j;
	unsigned int tmp_value;
	unsigned int rank_idx = 0;
	unsigned int offset = 0;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM) {
		ddrtrn_error("loop upper limit ddrtrn_hal_get_phy_num() out of range");
		return;
	}
	ddrtrn_hal_switch_rank_all_phy(0x0); /* 0x0: switch to rank0 */
	ddrtrn_imp_ctrl1_cfg();
	for (i = 0; i < sizeof(g_ddrtrn_hal_phy_param) / sizeof(struct ddrtrn_hal_phy_param_s); i++) {
		if (g_ddrtrn_hal_phy_param[i].reg_addr == ddrtrn_hal_phy_dvreft_status(0)) { /* DVREFT_STATUS */
			ddrtrn_hal_switch_rank_all_phy(rank_idx);
			rank_idx = 0x1;
		}
		for (j = 0; j < g_ddrtrn_hal_phy_param[i].reg_num; j++) {
			/* save ddr phy0 parameters */
			tmp_value = ddrtrn_reg_read(DATA_PHY_BASE_ADDR + lpds_ddrtrn_hal_phy_parab(j) + offset);
			tmp_value = ddrtrn_imp_status1_cfg(g_ddrtrn_hal_phy_param[i].reg_addr, tmp_value);
			ddrtrn_reg_write(tmp_value, DDR_REG_BASE_PHY0 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
			/* save ddr phy1 parameters */
			tmp_value = ddrtrn_reg_read(DATA_PHY_BASE_ADDR + lpds_hpp_reseved_areac(j) + offset);
			tmp_value = ddrtrn_imp_status1_cfg(g_ddrtrn_hal_phy_param[i].reg_addr, tmp_value);
			ddrtrn_reg_write(tmp_value, DDR_REG_BASE_PHY1 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
			/* save ddr phy2 parameters */
			tmp_value = ddrtrn_reg_read(DATA_PHY_BASE_ADDR + g_ddrtrn_hal_phy_param[i].save_addr + j * DDR_REG_OFFSET);
			tmp_value = ddrtrn_imp_status1_cfg(g_ddrtrn_hal_phy_param[i].reg_addr, tmp_value);
			ddrtrn_reg_write(tmp_value, DDR_REG_BASE_PHY2 + g_ddrtrn_hal_phy_param[i].reg_addr + j * DDR_REG_OFFSET);
		}
		offset += g_ddrtrn_hal_phy_param[i].reg_num * DDR_REG_OFFSET;
	}
	ddrtrn_hal_switch_rank_all_phy(0x0); /* 0x0: switch to 0 */
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		ddrtrn_hal_phy_reset(ddrtrn_hal_get_cur_phy());
		ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
	}
}

static void ddrtrn_resume_lp_en_ctrl(void)
{
	unsigned int ddrphy_lp_en;
	ddrphy_lp_en = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRPHY_LP_EN);
	/* Cancel ddr retention */
	ddrphy_lp_en = (ddrphy_lp_en &
					(~(SYS_DDRPHY_IO_OE_MASK << SYS_DDRPHY_IO_OE_BIT)) &
					(~(SYS_DDRPHY_LP_EN0_MASK << SYS_DDRPHY_LP_EN0_BIT)) &
					(~(SYS_DDRPHY_LP_EN1_MASK << SYS_DDRPHY_LP_EN1_BIT)));
	ddrtrn_reg_write(ddrphy_lp_en, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRPHY_LP_EN);
	/* enable io oe */
	ddrphy_lp_en = ddrphy_lp_en | (SYS_DDRPHY_IO_OE_MASK << SYS_DDRPHY_IO_OE_BIT);
	ddrtrn_reg_write(ddrphy_lp_en, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDRPHY_LP_EN); /* Cancel ddr retention */
	ddrtrn_hal_training_delay(DDR_SET_FRE_DELAY_100NS); /* wait 100ns */
}

/* adjust wdqphse */
static void ddrtrn_resume_wdqphase_adjust(int offset)
{
	int wdqphase;
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int cur_mode = ddrtrn_hal_get_cur_mode();
	ddrtrn_hal_set_cur_mode(DDR_MODE_WRITE);
	if (byte_num > DDR_PHY_BYTE_MAX) {
		ddrtrn_error("byte num error, byte_num = %x", byte_num);
		return;
	}
	for (i = 0; i < byte_num; i++) {
		ddrtrn_hal_set_cur_byte(i);
		wdqphase = ddrtrn_hal_adjust_get_val();
		wdqphase += offset;
		wdqphase = (wdqphase < 0) ? 0 : wdqphase;
		wdqphase = (wdqphase > PHY_WDQ_PHASE_MASK) ? PHY_WDQ_PHASE_MASK : wdqphase;
		ddrtrn_hal_adjust_set_val(wdqphase);
	}
	ddrtrn_hal_set_cur_mode(cur_mode); /* restore to current mode */
}

static int ddrtrn_resume_process(struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	int result = 0;
	unsigned int i, j;
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return -1;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		/* exit powerdown */
		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_phy_id(i);
			ddrtrn_sref_cfg(cfg_sref, DMC_CFG_INIT_XSREF); /* bit[3:2] 0x2 */
		}
		/* RDET/WDET Method Selection */
		ddrtrn_hal_det_mod_sel();
		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_phy_id(i);
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
			if (ddrtrn_hal_get_cur_phy_rank_num() > DDR_SUPPORT_RANK_MAX)
				return -1;
			/* adjust wdqphse */
			for (j = 0; j < ddrtrn_hal_get_cur_phy_rank_num(); j++) {
				ddrtrn_hal_set_rank_id(j);
				ddrtrn_resume_wdqphase_adjust(DDR_WDQPHASE_ADJUST_OFFSET);
			}
			ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
		}
		/* enable gt/rdet/wdet */
		ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_GT_EN |
							   PHY_PHYINITCTRL_RDET_EN | PHY_PHYINITCTRL_WDET_EN);
		result += ddrtrn_hw_training();
	} else {
		for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
			ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
			ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
		}
	}
	return result;
}

int ddrtrn_resume(void)
{
	int result;
	unsigned int i;
	struct ddrtrn_dmc_cfg_sref cfg_sref;
	ddrtrn_hal_cfg_init();
	/* enable PLL and dram reset */
	ddrtrn_hal_hw_item_cfg(PHY_PHYINITCTRL_INIT_EN | PHY_PHYINITCTRL_PLL_INIT_EN |
						   PHY_PHYINITCTRL_DRAM_RST | PHY_PHYINITCTRL_DLYMEAS_EN);
	result = ddrtrn_hw_training();
	/* restore DDR PHY register paraments */
	ddrtrn_resume_restore_para();
	ddrtrn_resume_lp_en_ctrl();
	if (ddrtrn_hal_get_phy_num() > DDR_PHY_NUM)
		return -1;
	if (ddrtrn_resume_process(&cfg_sref))
		return -1;
	/* exit auto self-refresh */
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4)
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF | DMC_CFG_SREF_PD); /* bit[3:2] 0x3 */
		else
			ddrtrn_sref_cfg(&cfg_sref, DMC_CFG_INIT_XSREF); /* bit[3:2] 0x2 */
	}
	return result;
}
#else
void ddrtrn_suspend_store_para(void)
{
	return;
}

int ddrtrn_resume(void)
{
	return 0;
}
#endif /* DDR_WAKEUP_CONFIG */
