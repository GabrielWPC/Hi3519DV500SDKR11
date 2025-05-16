/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_VREF_TRAINING_CONFIG
#ifdef DDR_VREF_WITHOUT_BDL_CONFIG
/* Save dataeye dq bdl before vref training */
void ddrtrn_hal_vref_save_bdl(struct ddrtrn_hal_trianing_dq_data *dq_data)
{
	unsigned int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank = ddrtrn_hal_get_rank_id();
	unsigned int byte_index;
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		byte_index = i + (ddrtrn_hal_get_dmc_id() << 1); /* byte index accord to phy */
		if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE) {
			dq_data->dq03[i] =
				ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank, byte_index));
			dq_data->dq47[i] =
				ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank, byte_index));
			dq_data->wdm[i] =
				ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank, byte_index));
		} else {
			dq_data->dq03[i] =
				ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank, byte_index));
			dq_data->dq47[i] =
				ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank, byte_index));
		}
	}
}

/* Restore dataeye dq bdl after vref training */
void ddrtrn_hal_vref_restore_bdl(struct ddrtrn_hal_trianing_dq_data *dq_data)
{
	unsigned int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int rank = ddrtrn_hal_get_rank_id();
	unsigned int byte_index;
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		byte_index = i + (ddrtrn_hal_get_dmc_id() << 1); /* byte index accord to phy */
		if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE) {
			ddrtrn_reg_write(dq_data->dq03[i],
							 base_phy + ddrtrn_hal_phy_dxnwdqnbdl0(rank, byte_index));
			ddrtrn_reg_write(dq_data->dq47[i],
							 base_phy + ddrtrn_hal_phy_dxnwdqnbdl1(rank, byte_index));
			ddrtrn_reg_write(dq_data->wdm[i],
							 base_phy + ddrtrn_hal_phy_dxnwdqnbdl2(rank, byte_index));
		} else {
			ddrtrn_reg_write(dq_data->dq03[i],
							 base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(rank, byte_index));
			ddrtrn_reg_write(dq_data->dq47[i],
							 base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(rank, byte_index));
		}
	}
}
#else
void ddrtrn_hal_vref_save_bdl(__attribute__((unused)),
							  __attribute__((unused)) struct ddrtrn_hal_trianing_dq_data *dq_data)
{}
void ddrtrn_hal_vref_restore_bdl(__attribute__((unused)),
								 __attribute__((unused)) struct ddrtrn_hal_trianing_dq_data *dq_data)
{}
#endif /* DDR_VREF_WITHOUT_BDL_CONFIG */

#if defined(DDR_PHY_S14_CONFIG) || defined(DDR_PHY_C28_V100_CONFIG) || defined(DDR_PHY_C28_V200_CONFIG)
/* phy s40 not support DRAM vref */
static int ddrtrn_hal_vref_phy_dram_set(unsigned int base_phy, unsigned int val, unsigned int byte_index)
{
	unsigned int count;
	unsigned int dvrftctrl = ddrtrn_reg_read(base_phy + DDR_PHY_DVRFTCTRL);
	unsigned int dvreft =
		ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dvreft_status(byte_index)) &
		(~PHY_VRFTRES_DVREF_MASK);
	ddrtrn_reg_write(dvrftctrl | PHY_DVRFTCTRL_PDAEN_EN,
					 base_phy + DDR_PHY_DVRFTCTRL);
	ddrtrn_reg_write(dvreft | val, base_phy + ddrtrn_hal_phy_dvreft_status(byte_index));
	ddrtrn_reg_write(PHY_PHYINITCTRL_DVREFT_SYNC | PHY_PHYINITCTRL_INIT_EN,
					 base_phy + DDR_PHY_PHYINITCTRL);
	count = DDR_HWR_WAIT_TIMEOUT;
	/* auto cleared to 0 after training finished */
	while (count--) {
		if ((ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL) &
				PHY_PHYINITCTRL_INIT_EN) == 0)
			break;
	}
	if (count == 0xffffffff) {
		ddrtrn_fatal("vref dram set wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, base_phy, byte_index,
								 ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		return -1;
	}
	ddrtrn_reg_write(dvrftctrl & (~PHY_DVRFTCTRL_PDAEN_EN),
					 base_phy + DDR_PHY_DVRFTCTRL);
	return 0;
}
#endif

#if defined(DDR_PHY_S14_CONFIG) || defined(DDR_PHY_C28_V100_CONFIG) || defined(DDR_PHY_C28_V200_CONFIG)
static void ddrtrn_hal_vref_phy_host_set(unsigned int base_phy,
	unsigned int rank, unsigned int byte_index, unsigned int val)
{
	unsigned int hvreft;
	if (rank == 0) {
		hvreft =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index)) &
			(~PHY_VRFTRES_HVREF_MASK);
		ddrtrn_reg_write(hvreft | val,
						 base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index));
		ddrtrn_reg_write(hvreft | val,
						 base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index + 1));
	} else {
		hvreft =
			ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index)) &
			(~(PHY_VRFTRES_RXDIFFCAL_MASK << PHY_VRFTRES_RXDIFFCAL_BIT));
		ddrtrn_reg_write(hvreft | (val << PHY_VRFTRES_RXDIFFCAL_BIT),
						 base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index));
		ddrtrn_reg_write(hvreft | (val << PHY_VRFTRES_RXDIFFCAL_BIT),
						 base_phy + ddrtrn_hal_phy_hvreft_status(rank, byte_index + 1));
	}
}
#endif

/* Set DDR Vref value. */
void ddrtrn_hal_vref_status_set(unsigned int val)
{
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_READ) { /* HOST vref */
		ddrtrn_hal_vref_phy_host_set(ddrtrn_hal_get_cur_phy(), ddrtrn_hal_get_rank_id(),
									 ddrtrn_hal_get_cur_byte(), val);
	} else { /* DRAM vref */
		unsigned int auto_ref_timing =
			ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc() + DDR_DMC_TIMING2);
		/* disable auto refresh */
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc(),
							  auto_ref_timing & DMC_AUTO_TIMING_DIS);
		/* DDR_PHY_VREFTCTRL 31bit:1 do vref dram set twice */
		ddrtrn_reg_write((ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_VREFTCTRL) &
						  (~(0x1 << PHY_VREFS_MRS_ENTER_BIT))) |
						 (0x1 << PHY_VREFS_MRS_ENTER_BIT),
						 ddrtrn_hal_get_cur_phy() + DDR_PHY_VREFTCTRL);
		/* DRAM vref operations */
		ddrtrn_hal_vref_phy_dram_set(ddrtrn_hal_get_cur_phy(), val, ddrtrn_hal_get_cur_byte());
		ddrtrn_hal_vref_phy_dram_set(ddrtrn_hal_get_cur_phy(), val, ddrtrn_hal_get_cur_byte());
		/* DDR_PHY_VREFTCTRL 31bit:0 do vref dram set once */
		ddrtrn_reg_write(ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_VREFTCTRL) &
						 (~(0x1 << PHY_VREFS_MRS_ENTER_BIT)),
						 ddrtrn_hal_get_cur_phy() + DDR_PHY_VREFTCTRL);
		/* DRAM vref operations */
		ddrtrn_hal_vref_phy_dram_set(ddrtrn_hal_get_cur_phy(), val, ddrtrn_hal_get_cur_byte());
		/* enable auto refresh */
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc(), auto_ref_timing);
	}
	ddrtrn_info("byte[%x] mode[%x] set vref [%x]", ddrtrn_hal_get_cur_byte(),
				ddrtrn_hal_get_cur_mode(), val);
}
#endif /* DDR_VREF_TRAINING_CONFIG */
