/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

/* s40/t28/t16 not support dcc training */
#ifdef DDR_DCC_TRAINING_CONFIG
unsigned int ddrtrn_hal_dcc_get_ioctl21(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_ACIOCTL21);
}

void ddrtrn_hal_dcc_set_ioctl21(unsigned int ioctl21)
{
	ddrtrn_reg_write(ioctl21, ddrtrn_hal_get_cur_phy() + DDR_PHY_ACIOCTL21);
}

unsigned int ddrtrn_hal_dcc_get_gated_bypass(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_AC_GATED_BYPASS);
}

void ddrtrn_hal_dcc_set_gated_bypass(unsigned int gated_bypass_temp)
{
	ddrtrn_reg_write(gated_bypass_temp, ddrtrn_hal_get_cur_phy() + DDR_PHY_AC_GATED_BYPASS);
}

void ddrtrn_hal_dcc_rdet_enable(void)
{
	ddrtrn_reg_write(PHY_PHYINITCTRL_RDET_EN,
					 ddrtrn_hal_get_cur_phy() + DDR_PHY_PHYINITSTATUS);
}

unsigned int ddrtrn_hal_dcc_get_dxnrdbound(unsigned int byte_index)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdbound(byte_index));
}
#endif /* DDR_DCC_TRAINING_CONFIG */
