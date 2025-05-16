/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_GATE_TRAINING_CONFIG
/* Find gate phase */
int ddrtrn_gate_training(void)
{
	int result;
	unsigned int i, tmp;
	unsigned int byte_num;
	unsigned int def_delay[DDR_PHY_BYTE_MAX];
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	struct ddrtrn_delay rdqsg;
	ddrtrn_debug("DDR Gate training");
	byte_num = ddrtrn_hal_get_byte_num();
	for (i = 0; i < byte_num; i++)
		def_delay[i] = ddrtrn_hal_gate_get_dxn_rdqsg_dly(i);
	/* find phase first */
	result = ddrtrn_hal_gate_find_phase(&rdqsg);
	/* find bdl */
	if (result == 0)
		result = ddrtrn_hal_gate_find_bdl(&rdqsg);
	/* set new phase */
	if (result == 0) {
		for (i = 0; i < byte_num; i++) {
			rdqsg.phase[i] -= PHY_GATE_PHASE_MARGIN;
			tmp = ddrtrn_hal_gate_get_dxn_rdqsg_dly(i);
			tmp &= ~(PHY_RDQSG_PHASE_MASK << PHY_RDQSG_PHASE_BIT);
			tmp |= rdqsg.phase[i] << PHY_RDQSG_PHASE_BIT;
			ddrtrn_hal_gate_set_dxn_rdqsg_dly(i, tmp);
		}
	} else {
		/* restore default value */
		for (i = 0; i < byte_num; i++)
			ddrtrn_hal_gate_set_dxn_rdqsg_dly(i, tmp);
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
	return 0; /* use default value and not reset */
}

int ddrtrn_gating_func(void)
{
	struct ddrtrn_hal_training_relate_reg relate_reg;
	int result = 0;
	/* gate training disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_GATE_MASK) != DDR_FALSE) {
		/* check hardware gating */
		if (ddrtrn_hal_gate_get_init_status()) {
			ddrtrn_fatal("PHY[%x] hw gating fail", ddrtrn_hal_get_cur_phy());
			ddrtrn_hal_training_stat(DDR_ERR_HW_GATING, ddrtrn_hal_get_cur_phy(), -1, -1);
			return -1;
		}
		return 0;
	}
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_GATE_MASK);
	ddrtrn_hal_training_switch_axi();
	ddrtrn_ddrt_init(DDR_DDRT_MODE_GATE);
	result += ddrtrn_gate_training();
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_gating_func(void)
{
	ddrtrn_warning("Not support DDR gate training");
	return 0;
}
#endif /* DDR_GATE_TRAINING_CONFIG */
