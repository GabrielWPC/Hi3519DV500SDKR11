/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_TRAINING_ADJUST_CONFIG
unsigned int ddrtrn_hal_adjust_get_average(void)
{
	unsigned int dq0_3, dq4_7, val;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_index = ddrtrn_hal_get_cur_byte();
	unsigned int rank = ddrtrn_hal_get_rank_id();
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE) {
		/* write */
		dq0_3 = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank, byte_index));
		dq4_7 = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank, byte_index));
	} else {
		/* read */
		dq0_3 = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank, byte_index));
		dq4_7 = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank, byte_index));
	}
	val = ((dq0_3 >> PHY_BDL_DQ0_BIT) & PHY_BDL_MASK) +
		  ((dq0_3 >> PHY_BDL_DQ1_BIT) & PHY_BDL_MASK) +
		  ((dq0_3 >> PHY_BDL_DQ2_BIT) & PHY_BDL_MASK) +
		  ((dq0_3 >> PHY_BDL_DQ3_BIT) & PHY_BDL_MASK) +
		  ((dq4_7 >> PHY_BDL_DQ0_BIT) & PHY_BDL_MASK) +
		  ((dq4_7 >> PHY_BDL_DQ1_BIT) & PHY_BDL_MASK) +
		  ((dq4_7 >> PHY_BDL_DQ2_BIT) & PHY_BDL_MASK) +
		  ((dq4_7 >> PHY_BDL_DQ3_BIT) & PHY_BDL_MASK);
	val = val >> 3; /* shift 3: 8 dq */
	return val;
}

unsigned int ddrtrn_hal_adjust_get_rdqs(void)
{
	return ((ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(ddrtrn_hal_get_cur_byte())) >>
			 PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK);
}
#endif /* DDR_TRAINING_ADJUST_CONFIG */
