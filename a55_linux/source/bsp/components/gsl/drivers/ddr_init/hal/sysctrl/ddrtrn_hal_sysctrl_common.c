/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

unsigned int ddrtrn_hal_get_sysctrl_cfg(unsigned int cfg_offset_addr)
{
	return ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + cfg_offset_addr);
}

void ddrtrn_hal_set_sysctrl_cfg(unsigned int item)
{
	ddrtrn_reg_write(item, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG);
}

#ifdef SYSCTRL_DDR_TRAINING_VERSION_FLAG
void ddrtrn_hal_version_flag(void)
{
	unsigned int tmp_reg;
	/* DDR training version flag */
	tmp_reg = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_VERSION_FLAG);
	tmp_reg = (tmp_reg & 0xffff0000) | DDR_VERSION;
	ddrtrn_reg_write(tmp_reg, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_VERSION_FLAG);
}
#endif

int ddrtrn_hal_check_sw_item(void)
{
#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	if ((ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG) == DDR_BYPASS_ALL_MASK) &&
			(ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC) == DDR_BYPASS_ALL_MASK))
		return 0;
#else
	if (ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG) == DDR_BYPASS_ALL_MASK)
		return 0;
#endif
	return -1;
}

int ddrtrn_hal_check_not_dcc_item(void)
{
	unsigned int dcc_bypass;
	dcc_bypass = DDR_BYPASS_PHY0_MASK | DDR_BYPASS_PHY1_MASK | DDR_BYPASS_PHY2_MASK | DDR_BYPASS_DCC_MASK;
#ifdef SYSCTRL_DDR_TRAINING_CFG_SEC
	if (((ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG) & ~dcc_bypass) != ~dcc_bypass) ||
			((ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG_SEC) & ~dcc_bypass) != ~dcc_bypass))
		return 0;
#else
	if ((ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_CFG) & ~dcc_bypass) != ~dcc_bypass)
		return 0;
#endif
	return -1;
}

void ddrtrn_hal_set_adjust(unsigned int adjust)
{
	ddrtrn_reg_write(adjust, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_ADJUST);
}

unsigned int ddrtrn_hal_get_adjust(void)
{
	return ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_ADJUST);
}

void ddrtrn_hal_clear_sysctrl_stat_reg(void)
{
	ddrtrn_reg_write(0x0, DDR_REG_BASE_SYSCTRL + SYSCTRL_DDR_TRAINING_STAT);
}
