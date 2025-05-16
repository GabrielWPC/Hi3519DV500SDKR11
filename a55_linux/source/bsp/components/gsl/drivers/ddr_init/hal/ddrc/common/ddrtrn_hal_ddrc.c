/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"
#include "hal/ddrtrn_hal_context.h"

/* set auto refresh */
void ddrtrn_hal_set_timing(unsigned int base_dmc, unsigned int timing)
{
	ddrtrn_hal_training_delay(DDR_AUTO_TIMING_DELAY);
	ddrtrn_reg_write(timing, base_dmc + DDR_DMC_TIMING2);
	/* need to delay 1 ns */
	ddrtrn_hal_training_delay(DDR_AUTO_TIMING_DELAY);
}

unsigned int ddrtrn_hal_ddrc_get_bank_group(void)
{
	return (ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc() +
							ddr_dmc_cfg_rnkvol(ddrtrn_hal_get_rank_id())) >>
			DMC_CFG_MEM_BG_BIT) & DMC_CFG_MEM_BG_MASK;
}

#if defined(DDR_WL_TRAINING_CONFIG) || defined(DDR_MPR_TRAINING_CONFIG)
/* Excute DMC sfc command */
void ddrtrn_hal_dmc_sfc_cmd(unsigned int base_dmc, unsigned int sfc_cmd,
							unsigned int sfc_addr, unsigned int sfc_bank)
{
	unsigned int count = 0;
	/* set sfc cmd */
	ddrtrn_hal_dmc_sfc_cmd_write(sfc_cmd, base_dmc + DDR_DMC_SFCCMD);
	/* set col and row */
	ddrtrn_reg_write(sfc_addr, base_dmc + DDR_DMC_SFCADDR);
	/* set bank */
	ddrtrn_hal_dmc_sfc_bank_write(sfc_bank, base_dmc + DDR_DMC_SFCBANK);
	/* excute cmd */
	ddrtrn_reg_write(0x1, base_dmc + DDR_DMC_SFCREQ);
	ddr_asm_dsb();
	while (count < DDR_SFC_WAIT_TIMEOUT) { /* wait command finished */
		if ((ddrtrn_reg_read(base_dmc + DDR_DMC_SFCREQ) & 0x1) == 0)
			break;
		count++;
	}
	if (count >= DDR_HWR_WAIT_TIMEOUT)
		ddrtrn_error("SFC cmd wait timeout");
}
#endif

#if !defined(DDR_TRAINING_CUT_CODE_CONFIG) || defined(DDR_TRAINING_CMD)
/* Save register value before training */
void ddrtrn_hal_save_reg(struct ddrtrn_hal_training_relate_reg *relate_reg, unsigned int mask)
{
	unsigned int base_dmc = ddrtrn_hal_get_cur_dmc();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	/* save reg value */
	relate_reg->auto_ref_timing = ddrtrn_reg_read(base_dmc + DDR_DMC_TIMING2);
	relate_reg->power_down = ddrtrn_reg_read(base_dmc + DDR_DMC_CFG_PD);
	relate_reg->misc_scramb = ddrtrn_reg_read(base_phy + DDR_PHY_MISC);
	/* Static register have to read two times to get the right value. */
	relate_reg->ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL4);
	relate_reg->ac_phy_ctl = ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTL4);
	/* set new value */
	switch (mask) {
	case DDR_BYPASS_WL_MASK:
	case DDR_BYPASS_LPCA_MASK:
		/* disable auto refresh */
		ddrtrn_hal_set_timing(base_dmc,
							  relate_reg->auto_ref_timing & DMC_AUTO_TIMING_DIS);
		break;
	case DDR_BYPASS_GATE_MASK:
		/* disable auto refresh */
		ddrtrn_hal_set_timing(base_dmc,
							  relate_reg->auto_ref_timing & DMC_AUTO_TIMING_DIS);
		if ((ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG) &
				PHY_DRAMCFG_MA2T) == 0) /* set 1T */
			ddrtrn_reg_write(0x0, base_phy + DDR_PHY_ACPHYCTL4);
		break;
	case DDR_BYPASS_HW_MASK:
		if ((ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG) &
				PHY_DRAMCFG_MA2T) == 0) /* set 1T */
			ddrtrn_reg_write(0x0, base_phy + DDR_PHY_ACPHYCTL4);
		break;
	default:
		break;
	}
	ddrtrn_reg_write(relate_reg->power_down & DMC_POWER_DOWN_DIS,
					 base_dmc + DDR_DMC_CFG_PD);
	ddrtrn_reg_write(relate_reg->misc_scramb & PHY_MISC_SCRAMB_DIS,
					 base_phy + DDR_PHY_MISC);
	ddrtrn_hal_dqsswap_save_func(&(relate_reg->swapdfibyte_en), base_phy);
	ddrtrn_hal_axi_save_func(relate_reg);
	ddrtrn_hal_rnkvol_save_func(relate_reg, base_dmc);
	/* save customer reg */
	ddr_training_save_reg_func((void *)relate_reg, mask);
	ddrtrn_hal_phy_cfg_update(base_phy);
	ddr_asm_dsb();
}

/* Restore register value after training */
void ddrtrn_hal_restore_reg(const struct ddrtrn_hal_training_relate_reg *relate_reg)
{
	unsigned int base_dmc = ddrtrn_hal_get_cur_dmc();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	/* enable auto refresh */
	ddrtrn_hal_set_timing(base_dmc, relate_reg->auto_ref_timing);
	ddrtrn_reg_write(relate_reg->power_down, base_dmc + DDR_DMC_CFG_PD);
	ddrtrn_reg_write(relate_reg->misc_scramb, base_phy + DDR_PHY_MISC);
	if ((ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG) & PHY_DRAMCFG_MA2T) == 0)
		ddrtrn_reg_write(relate_reg->ac_phy_ctl, base_phy + DDR_PHY_ACPHYCTL4);
	ddrtrn_hal_dqsswap_restore_func(relate_reg->swapdfibyte_en, base_phy);
	ddrtrn_hal_axi_restore_func(relate_reg);
	ddrtrn_hal_rnkvol_restore_func(relate_reg, base_dmc);
	/* restore customer reg */
	ddr_training_restore_reg_func((void *)relate_reg);
	ddrtrn_hal_phy_cfg_update(base_phy);
	ddr_asm_dsb();
}

/* Switch AXI to DMC0/DMC1/DMC2/DMC3 for DDRT test */
void ddrtrn_hal_training_switch_axi(void)
{
	ddrtrn_hal_axi_chsel_remap_func();
	ddrtrn_hal_axi_switch_func();
	ddrtrn_debug("AXI region0[%x = %x]",
				 (DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1),
				 ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1));
	ddrtrn_debug("AXI region1[%x = %x]",
				 (DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1),
				 ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION1_ATTRIB1));
	ddrtrn_hal_rnkvol_set_func();
}
#endif

#if defined(DDR_HW_TRAINING_CONFIG) || defined(DDR_DCC_TRAINING_CONFIG)
/* Exit or enter auto self-refresh */
int ddrtrn_hal_ddrc_easr(unsigned int base_dmc, unsigned int sref_req)
{
	unsigned int count = DDR_HWR_WAIT_TIMEOUT;
	if (sref_req == DDR_EXIT_SREF) {
		/* Exit Auto-self refresh */
		ddrtrn_reg_write(DMC_CTRL_SREF_EXIT, base_dmc + DDR_DMC_CTRL_SREF);
		while (count--) {
			if ((ddrtrn_reg_read(base_dmc + DDR_DMC_CURR_FUNC) & DMC_CURR_FUNC_IN_SREF_MASK) == 0)
				break;
		}
	} else if (sref_req == DDR_ENTER_SREF) {
		/* Enter Auto-self refresh */
		ddrtrn_reg_write(DMC_CTRL_SREF_ENTER, base_dmc + DDR_DMC_CTRL_SREF);
		while (count--) {
			if (ddrtrn_reg_read(base_dmc + DDR_DMC_CURR_FUNC) & DMC_CURR_FUNC_IN_SREF_MASK)
				break;
		}
	}
	if (count == 0xffffffff) {
		ddrtrn_fatal("SREF wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, -1, -1, -1);
		return -1;
	}
	return 0;
}

void ddrtrn_hal_save_timing(struct ddrtrn_hal_timing *timing)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++) {
		timing->val[i] = ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc_addr(i) + DDR_DMC_TIMING2);
		/* disable auto refresh */
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc_addr(i), timing->val[i] & DMC_AUTO_TIMING_DIS);
	}
}
#endif /* DDR_HW_TRAINING_CONFIG || DDR_DCC_TRAINING_CONFIG */

unsigned int ddrtrn_hal_dmc_get_sref_cfg(unsigned int i)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc_addr(i) + DDR_DMC_CFG_SREF);
}

void ddrtrn_hal_dmc_set_sref_cfg(unsigned int i, unsigned int value)
{
	ddrtrn_reg_write(value, ddrtrn_hal_get_cur_dmc_addr(i) + DDR_DMC_CFG_SREF);
}

/* Get byte number */
unsigned int ddrtrn_hal_phy_get_byte_num(unsigned int base_dmc)
{
	unsigned int byte_num;
	/* memery width -> byte number */
	byte_num =
		((ddrtrn_reg_read(base_dmc + DDR_DMC_CFG_DDRMODE) >> DMC_MEM_WIDTH_BIT) & DMC_MEM_WIDTH_MASK) <<
		1;
	/* for codedex */
	if (byte_num > DDR_PHY_BYTE_MAX) {
		byte_num = DDR_PHY_BYTE_MAX;
		ddrtrn_error("get byte num fail");
	}
	return byte_num;
}

unsigned int ddrtrn_hal_get_rank_size(void)
{
	unsigned int base_dmc = ddrtrn_hal_get_cur_dmc();
	unsigned int rnkvol;
	unsigned int mem_bank, mem_row, mem_col, mem_width;
	unsigned int size;
	mem_width = (ddrtrn_reg_read(base_dmc + DDR_DMC_CFG_DDRMODE) >> DMC_MEM_WIDTH_BIT) & DMC_MEM_WIDTH_MASK;
	rnkvol = ddrtrn_reg_read(base_dmc + ddr_dmc_cfg_rnkvol(0));
	mem_bank = (rnkvol >> DMC_RNKVOL_MEM_BANK_BIT) & DMC_RNKVOL_MEM_BANK_MASK;
	mem_row = (rnkvol >> DMC_RNKVOL_MEM_ROW_BIT) & DMC_RNKVOL_MEM_ROW_MASK;
	mem_col = rnkvol & DMC_RNKVOL_MEM_COL_MASK;
	/*
	 * mem_bank
	 * 0: 4 Bank(2 bank address)
	 * 1: 8 Bank(3 bank address)
	 * 2: 16 BANK(4 bank address)
	 * 3: reserved
	 * mem_row
	 * 000: 11 bit
	 * 001: 12 bit
	 * 010: 13 bit
	 * 011: 14 bit
	 * 100: 15 bit
	 * 101: 16 bit
	 * 110: 17 bit
	 * 111: 18 bit
	 * mem_width
	 * 000: 8 bit
	 * 001: 9 bit
	 * 010: 10 bit
	 * 011: 11 bit
	 * 100: 12 bit
	 */
	size = 1UL << ((mem_bank + 2) + (mem_row + 11) + (mem_col + 8) + /* 2:2 bank; 11:11 bit; 8:8 bit */
				   mem_width);
	ddrtrn_debug("rank size[%x]", size);
	return size;
}

/* Special interleave path mode */
void ddrtrn_hal_axi_special_intlv_en(void)
{
	unsigned int action;

	action = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_ACTION);
	ddrtrn_reg_write((action & (~(0x1 << SPECIAL_INTLV_EN_BIT))) | (0x1 << SPECIAL_INTLV_EN_BIT),
		DDR_REG_BASE_AXI + DDR_AXI_ACTION);
}

/* get DDRC TIMING8 trfc_ab clock cycle */
void ddrtrn_hal_timing8_trfc_ab_cfg_by_dmc(void)
{
	unsigned int base_dmc;
	unsigned int timing8, trfc_ab_table_def, trfc_ab;

	base_dmc = ddrtrn_hal_get_cur_dmc();
	timing8 = ddrtrn_reg_read(base_dmc + DDR_DMC_TIMING8);
	trfc_ab_table_def = (timing8 >> DMC_TIMING8_TRFC_AB_BIT) & DMC_TIMING8_TRFC_AB_MASK;
	trfc_ab = trfc_ab_table_def * DMC_TRFC_AB_350 / DMC_TRFC_AB_550 + 1;
	ddrtrn_reg_write((timing8 & (~(DMC_TIMING8_TRFC_AB_MASK << DMC_TIMING8_TRFC_AB_BIT))) |
		(trfc_ab << DMC_TIMING8_TRFC_AB_BIT), base_dmc + DDR_DMC_TIMING8);
}
