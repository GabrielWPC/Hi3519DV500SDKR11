/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

#ifdef DDR_GATE_TRAINING_CONFIG
/* Find gate phase */
int ddrtrn_hal_gate_find_phase(struct ddrtrn_hal_delay *rdqsg)
{
	int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		for (rdqsg->phase[i] = PHY_RDQSG_PHASE_MAX;
				rdqsg->phase[i] > PHY_GATE_PHASE_MARGIN;
				rdqsg->phase[i] -= PHY_RDQSG_PHASE_STEP) {
			ddrtrn_reg_write(rdqsg->phase[i] << PHY_RDQSG_PHASE_BIT,
							 base_phy + ddrtrn_hal_phy_dxnrdqsgdly(ddrtrn_hal_get_rank_id(), i));
			ddrtrn_hal_phy_cfg_update(base_phy);
			if (ddrtrn_ddrt_test(DDRT_WR_COMPRARE_MODE, i, -1) == 0)
				break;
		}
		if (rdqsg->phase[i] <= PHY_GATE_PHASE_MARGIN) {
			/* find gate phase fail */
			ddrtrn_fatal("find gate phase[%x] fail", rdqsg->phase[i]);
			ddrtrn_hal_training_stat(DDR_ERR_GATING, base_phy, -1, -1);
			return -1;
		} else {
			/* decrease one setp to find bdl */
			rdqsg->phase[i] -= PHY_RDQSG_PHASE_STEP;
			ddrtrn_reg_write(rdqsg->phase[i] << PHY_RDQSG_PHASE_BIT,
							 base_phy + ddrtrn_hal_phy_dxnrdqsgdly(ddrtrn_hal_get_rank_id(), i));
		}
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
	return 0;
}

void ddrtrn_hal_gate_find_bdl_by_byte(struct ddrtrn_hal_delay *rdqsg,
									  unsigned int byte_num, unsigned int gate_result)
{
	int j;
	unsigned int tmp;
	for (j = 0; j < byte_num; j++) {
		if ((gate_result & (1 << j)) == 0) {
			rdqsg->bdl[j] += DDR_GATE_BDL_STEP;
			if (rdqsg->bdl[j] > PHY_BDL_MASK)
				tmp = ((rdqsg->bdl[j] - PHY_BDL_MASK - 1) << PHY_RDQSG_TX_BDL_BIT) +
					  (rdqsg->phase[j] << PHY_RDQSG_PHASE_BIT) + (PHY_BDL_MASK - 1);
			else
				tmp = (rdqsg->phase[j] << PHY_RDQSG_PHASE_BIT) + rdqsg->bdl[j];
			ddrtrn_reg_write(tmp,
							 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsgdly(ddrtrn_hal_get_rank_id(), j));
		}
	}
}

/* Find gate bdl. */
int ddrtrn_hal_gate_find_bdl(struct ddrtrn_hal_delay *rdqsg)
{
	int i, j;
	unsigned int gate_result;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	unsigned int swtmode = ddrtrn_reg_read(base_phy + DDR_PHY_SWTMODE);
	for (i = 0; i < byte_num; i++)
		rdqsg->bdl[i] = 0;
	/* enable phy sw gate training mode */
	ddrtrn_reg_write(swtmode | (1 << PHY_SWTMODE_SW_GTMODE_BIT), base_phy + DDR_PHY_SWTMODE);
	for (i = 0; i < PHY_GATE_BDL_MAX; i++) {
		ddrtrn_hal_phy_cfg_update(base_phy);
		ddrtrn_ddrt_test(DDRT_READ_ONLY_MODE, -1, -1);
		gate_result = (ddrtrn_reg_read(base_phy + DDR_PHY_SWTRLT) >> PHY_SWTRLT_GATE_BIT) & PHY_SWTRLT_GATE_MASK;
		if (gate_result == ((1 << byte_num) - 1))
			break;
		ddrtrn_hal_gate_find_bdl_by_byte(rdqsg, byte_num, gate_result);
	}
	/* disable phy sw gate training mode */
	ddrtrn_reg_write(swtmode & (~(1 << PHY_SWTMODE_SW_GTMODE_BIT)), base_phy + DDR_PHY_SWTMODE);
	if (i == PHY_GATE_BDL_MAX) { /* find gate bdl fail */
		ddrtrn_fatal("PHY[%x] find gate bdl fail. result[%x]", base_phy, gate_result);
		for (j = 0; j < byte_num; j++) {
			if ((gate_result & (1 << j)) == 0)
				ddrtrn_hal_training_stat(DDR_ERR_GATING, base_phy, j, -1);
		}
		return -1;
	} else {
		return 0;
	}
}

unsigned int ddrtrn_hal_gate_get_dxn_rdqsg_dly(unsigned int index)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	return ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsgdly(ddrtrn_hal_get_rank_id(), index));
}

void ddrtrn_hal_gate_set_dxn_rdqsg_dly(unsigned int index, unsigned int val)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	ddrtrn_reg_write(val,
					 base_phy + ddrtrn_hal_phy_dxnrdqsgdly(ddrtrn_hal_get_rank_id(), index));
}

unsigned int ddrtrn_hal_gate_get_init_status()
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_PHYINITSTATUS) &
		   PHY_INITSTATUS_GT_MASK;
}
#endif /* DDR_GATE_TRAINING_CONFIG */
