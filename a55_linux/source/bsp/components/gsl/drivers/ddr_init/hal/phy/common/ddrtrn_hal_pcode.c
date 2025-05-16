/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

#ifdef DDR_PCODE_TRAINING_CONFIG
/*
 * DDR clock select.
 * For ddr osc training.
 */
int ddrtrn_hal_pcode_get_cksel(void)
{
	int freq;
	unsigned int ddrtrn_cksel;
	ddrtrn_cksel = (ddrtrn_reg_read(CRG_REG_BASE_ADDR + PERI_CRG_DDRCKSEL) >> 0x3) & 0x7;
	switch (ddrtrn_cksel) {
	case 0x0:
		freq = 24; /* 24 MHz */
		break;
	case 0x1:
		freq = 450; /* 450 MHz */
		break;
	case 0x3:
		freq = 300; /* 300 MHz */
		break;
	case 0x4:
		freq = 297; /* 297 MHz */
		break;
	default:
		freq = 300; /* 300 MHz */
		break;
	}
	return freq;
}

/* Set pcode value to register IMPSTATUS and DDR_PHY_IMP_STATUS1 */
void ddrtrn_hal_pcode_set_value(unsigned int base_phy, unsigned int pcode_value)
{
	unsigned int imp_ctrl1;
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_IMPSTATUS) &
					  (~(PHY_ZCODE_PDRV_MASK << PHY_ZCODE_PDRV_BIT))) |
					 (pcode_value << PHY_ZCODE_PDRV_BIT),
					 base_phy + DDR_PHY_IMPSTATUS);
	ddrtrn_debug("cur IMPSTATUS [%x] = [%x]", base_phy + DDR_PHY_IMPSTATUS,
				 ddrtrn_reg_read(base_phy + DDR_PHY_IMPSTATUS));
	imp_ctrl1 = ddrtrn_reg_read(base_phy + DDR_PHY_IMP_CTRL1);
	/* ac_vddq_cal_en set 0 */
	ddrtrn_reg_write(imp_ctrl1 & (~(0x1 << PHY_AC_VDDQ_CAL_EN_BIT)),
					 base_phy + DDR_PHY_IMP_CTRL1);
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_IMP_STATUS1) &
					  (~(PHY_ACCTL_PDRV_LATCH_MASK << PHY_ACCTL_PDRV_LATCH_BIT))) |
					 (pcode_value << PHY_ACCTL_PDRV_LATCH_BIT),
					 base_phy + DDR_PHY_IMP_STATUS1);
	ddrtrn_debug("cur IMP_STATUS1 [%x] = [%x]", base_phy + DDR_PHY_IMP_STATUS1,
				 ddrtrn_reg_read(base_phy + DDR_PHY_IMP_STATUS1));
	/* restore ac_vddq_cal_en */
	ddrtrn_reg_write(imp_ctrl1, base_phy + DDR_PHY_IMP_CTRL1);
}

void ddrtrn_hal_pcode_trainig_start(void)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int osc_rpt_vld;
	unsigned int times = 0;
	ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_CORNER_DETECTOR) |
					 PHY_OSC_START_MASK, base_phy + DDR_PHY_CORNER_DETECTOR);
	do {
		osc_rpt_vld = (ddrtrn_reg_read(base_phy + DDR_PHY_CORNER_DETECTOR) >>
					   PHY_OSC_RPT_VLD) & PHY_OSC_RPT_VLD_MASK;
		times++;
	} while ((osc_rpt_vld == 0) && (times < DDRT_PCODE_WAIT_TIMEOUT));
	return times;
}

void ddrtrn_hal_pcode_trainig_get_osc_cnt_rdata(void)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	return (int)((ddrtrn_reg_read(base_phy + DDR_PHY_CORNER_DETECTOR) >>
				  PHY_OSC_CNT_RDATA_BIT) & PHY_OSC_CNT_RDATA_MASK);
}

void ddrtrn_hal_pcode_trainig_stop(void)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	/* test stop */
	ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_CORNER_DETECTOR) &
					 (~PHY_OSC_START_MASK),
					 base_phy + DDR_PHY_CORNER_DETECTOR);
}
#endif
