/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_HW_TRAINING_CONFIG
#ifdef DDR_HW_READ_ADJ_CONFIG
/*
 * Adjust rdqs and dq after hw read training.
 * When define ddrtrn_TRAINING_ADJUST_DISABLE, MUST define ddrtrn_HW_READ_ADJ_CONFIG.
 */
void ddrtrn_hal_hw_read_adj(void)
{
	int i;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	ddrtrn_debug("DDR hw read adjust");
	/* check hw read adjust bypass bit */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_HW_ADJ_MASK) != DDR_FALSE)
		return;
	/* assume read dataeye window on left */
	for (i = 0; i < byte_num; i++) {
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i))
						 + (PHY_DQ_MIDDLE_VAL << PHY_BDL_DQ_BIT),
						 base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i))
						 + (PHY_DQ_MIDDLE_VAL << PHY_BDL_DQ_BIT),
						 base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i))
						 + (PHY_RDQS_MIDDLE_VAL << PHY_RDQS_BDL_BIT),
						 base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
	}
}
#else
void ddrtrn_hal_hw_read_adj(void) {}
#endif /* ddrtrn_HW_READ_ADJ_CONFIG */

/* sync rank1 WDQSPH/WDQPH to rank0 */
static void ddrtrn_hal_set_rank1_wdq_to_rank0(unsigned int base_phy, unsigned int byte_num)
{
	unsigned int byte_idx;
	for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxwdqsdly(1, byte_idx)),
						 base_phy + ddrtrn_hal_phy_dxwdqsdly(0, byte_idx));
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnwdqdly(1, byte_idx)),
						 base_phy + ddrtrn_hal_phy_dxnwdqdly(0, byte_idx));
	}
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/* For wudangstick cs when training rank1
 * This function is used to prevent logic bugs
 */
int ddrtrn_hal_hw_training_normal_conf(void)
{
	int result;
	unsigned int byte_idx;
	unsigned int base_phy;
	unsigned int byte_num;
	unsigned int cur_item = ddrtrn_hal_get_cur_item();
	struct ddrtrn_hal_training_dq_adj_rank wdq_rank0_byte;
	ddrtrn_set_data(&wdq_rank0_byte, 0, sizeof(struct ddrtrn_hal_training_dq_adj_rank));
	base_phy = ddrtrn_hal_get_cur_phy();
	byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	if (ddrtrn_hal_get_rank_id() == 0) {
		/* WL */
		result = ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_WL_EN);
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
			ddrtrn_hal_set_cur_byte(byte_idx);
			ddrtrn_hal_wdqs_bdl2phase(base_phy, 0, byte_idx);
		}
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_HW_GP_NORMAL);
	} else { /* rank1 */
		/* save rank0 WDQSPH/WDQPH of all byte */
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++)
			ddrtrn_hal_get_dly_value(&wdq_rank0_byte.byte_dq_adj[byte_idx], base_phy, 0, byte_idx);
		/* WL */
		result = ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_WL_EN);
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++) {
			ddrtrn_hal_set_cur_byte(byte_idx);
			ddrtrn_hal_wdqs_bdl2phase(base_phy, 1, byte_idx);
		}
		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);
		/* GATE/GDS/WL2/RDET/WDET */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_HW_GP_NORMAL_RANK1);
		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);
		/* HVREFT/DVREFT */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_HVREFT_EN);
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_DVREFT_EN);
		/* sync rank1 WDQSPH/WDQPH to rank0 */
		ddrtrn_hal_set_rank1_wdq_to_rank0(base_phy, byte_num);
		/* TDQSST */
		result += ddrtrn_hal_hw_training_process(cur_item & PHY_PHYINITCTRL_PIC_TDQSST);
		/* restore rank0 WDQSPH/WDQPH of all byte */
		for (byte_idx = 0; byte_idx < byte_num; byte_idx++)
			ddrtrn_hal_restore_dly_value(&wdq_rank0_byte.byte_dq_adj[byte_idx], base_phy, 0, byte_idx);
	}
	return result;
}

void ddrtrn_hal_hw_rdqs_offset_cfg(unsigned int base_phy, unsigned int byte_num,
								   unsigned int dram_type)
{
	unsigned int i;
	unsigned int rdqsdly;
	for (i = 0; i < byte_num; i++) {
		rdqsdly = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
		if (dram_type == PHY_DRAMCFG_TYPE_LPDDR4) {
			ddrtrn_reg_write(rdqsdly + DDR_RDQS_OFFSET_LPDDR4, base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
		} else {
			ddrtrn_reg_write(rdqsdly + DDR_RDQS_OFFSET, base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
		}
	}
}

void ddrtrn_hal_training_get_rdqs(struct ddrtrn_hal_bdl *rdqs)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	for (i = 0; i < byte_num; i++)
		rdqs->bdl[i] = ddrtrn_reg_read(base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

void ddrtrn_hal_training_set_rdqs(struct ddrtrn_hal_bdl *rdqs)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	for (i = 0; i < byte_num; i++)
		ddrtrn_reg_write(rdqs->bdl[i], base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

/* ca odt disable, DRAM_RST and DRAM_INIT are required to take effect
 * The DRAM_RST cannot be performed more than once
 */
static int ddrtrn_hal_hw_ca_odt_disable(void)
{
	int result;
	unsigned int temp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();
	temp = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG01);
	ddrtrn_reg_write(temp & 0x8fffffff, base_phy + DDR_PHY_MODEREG01); /* ca odt disable */
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_DRAM_RESET);
	ddrtrn_reg_write(temp, base_phy + DDR_PHY_MODEREG01); /* restore */
	return result;
}

/* CA Vref Sync, rank0 and rank1 */
static int ddrtrn_hal_hw_ca_vref_sync(void)
{
	int result;
	unsigned int temp;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();
	temp = ddrtrn_reg_read(base_phy + DDR_PHY_TRAINCTRL0);
	ddrtrn_reg_write(temp & (~PHY_TRAINCTRL0_MASK), base_phy + DDR_PHY_TRAINCTRL0); /* select rank0 */
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_AC);
	ddrtrn_reg_write((temp & (~PHY_TRAINCTRL0_MASK)) | 0x1,
					 base_phy + DDR_PHY_TRAINCTRL0); /* select rank1 */
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_AC);
	ddrtrn_reg_write(temp, base_phy + DDR_PHY_TRAINCTRL0); /* restore */
	return result;
}

static int ddrtrn_hal_hw_dram_mr_init(void)
{
	int result;
	unsigned int tx_odt_mode;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();
	tx_odt_mode = (ddrtrn_reg_read(base_phy + DDR_PHY_ACIOCTL) >> PHY_AC_IOCTL_TX_MODE_BIT) &
				  PHY_AC_IOCTL_TX_MODE_MASK;
	if (tx_odt_mode == DDR_PHY_LPDDR4X_MODE) {
		unsigned int temp;
		unsigned int temp1;
		/* rank0 */
		temp = ddrtrn_reg_read(base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write((temp & (~DDR_PHY_RANKEN_MASK)) | 0x1,
						 base_phy + DDR_PHY_RANKEN); /* select rank0 */
		temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG23); /* store the contents of the Mode Register */
		ddrtrn_reg_write(temp1 & 0xffffffc7, base_phy + DDR_PHY_MODEREG23); /* rank0 ck/cs/ca odt enable */
		result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN); /* rank0 draminit */
		/* restore */
		ddrtrn_reg_write(temp, base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write(temp1, base_phy + DDR_PHY_MODEREG23);
		/* rank1 */
		temp = ddrtrn_reg_read(base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write((temp & (~DDR_PHY_RANKEN_MASK)) | 0x2, /* 0x2:bit1 set 1 */
						 base_phy + DDR_PHY_RANKEN); /* select rank1 */
		temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG23);
		/* rank1 ck/caodt diable, rank1 cs odt enable */
		ddrtrn_reg_write((temp1 & 0xffffffc7) | 0x28, base_phy + DDR_PHY_MODEREG23);
		result += ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
		/* restore */
		ddrtrn_reg_write(temp, base_phy + DDR_PHY_RANKEN);
		ddrtrn_reg_write(temp1, base_phy + DDR_PHY_MODEREG23);
	} else {
		result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
	}
	return result;
}

/* DDR HW training adapt dram type */
int ddrtrn_hal_hw_dataeye_adapt(unsigned int *ddr_temp)
{
	int result;
	unsigned int modereg45;
	unsigned int modereg67;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	modereg45 = 0;
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		unsigned int dramtimer1;
		dramtimer1 = ddrtrn_reg_read(base_phy + DDR_PHY_DRAMTIMER1);
		ddrtrn_reg_write(dramtimer1 & (~(DDR_PHY_T_MOD_MASK << DDR_PHY_T_MOD_BIT)),
						 base_phy + DDR_PHY_DRAMTIMER1); /* TMOD:0 */
		result = ddrtrn_hal_hw_ca_odt_disable(); /* CA odt disable */
		result += ddrtrn_hal_hw_ca_vref_sync(); /* CA vref sync */
		result += ddrtrn_hal_hw_dram_mr_init(); /* in WR0 */
		modereg67 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG67);
		/* turn to WR1 */
		ddrtrn_reg_write(modereg67 | (0x1 << PHY_MODEREG67_LP4_FSPWR_BIT),
						 base_phy + DDR_PHY_MODEREG67); /* bit6 set 1 */
		result += ddrtrn_hal_hw_dram_mr_init();
		result += ddrtrn_hal_hw_ca_vref_sync(); /* CA vref sync */
		/* turn to WR0 */
		ddrtrn_reg_write(modereg67 & (~(0x1 << PHY_MODEREG67_LP4_FSPWR_BIT)),
						 base_phy + DDR_PHY_MODEREG67); /* bit6 set 0 */
		result += ddrtrn_hal_hw_dram_mr_init();
		/* restore DRAMTIMER1 */
		ddrtrn_reg_write(dramtimer1, base_phy + DDR_PHY_DRAMTIMER1);
	} else {
#ifdef DDR_WRITE_DM_DISABLE
		if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_DDR4) {
			modereg45 = ddrtrn_reg_read(base_phy + DDR_PHY_MODEREG45);
			ddrtrn_reg_write((modereg45 & 0xFBFFFFFF) | 0x8000000, base_phy + DDR_PHY_MODEREG45); /* write dm disable */
		}
#endif
		*ddr_temp = modereg45; /* for restore 0xe0 in ddr_hw_training_ctl */
		result = ddrtrn_hal_hw_training_process(ddrtrn_hal_get_cur_item() & PHY_HW_GP_DRAM_RESET);
	}
	return result;
}


int ddrtrn_hal_hw_dataeye_vref_set(void)
{
	int result;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int item = ddrtrn_hal_get_cur_item();
	unsigned int dvrft_ctrl;
	dvrft_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_DVRFTCTRL);
	ddrtrn_reg_write(dvrft_ctrl & (~PHY_DVRFTCTRL_PDAEN_EN),
					 base_phy + DDR_PHY_DVRFTCTRL);
	/* DDR_PHY_VREFTCTRL 31bit:1 do vref dram set twice */
	ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_VREFTCTRL) &
					  (~((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT))) |
					 ((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT),
					 base_phy + DDR_PHY_VREFTCTRL);
	result = ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	/* DDR_PHY_VREFTCTRL 31bit:0 do vref dram set once */
	ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_VREFTCTRL) &
					 (~((unsigned int)0x1 << PHY_VREFS_MRS_ENTER_BIT)),
					 base_phy + DDR_PHY_VREFTCTRL);
	result += ddrtrn_hal_hw_training_process(item & PHY_HW_GP_VREF_DQ);
	ddrtrn_reg_write(dvrft_ctrl, base_phy + DDR_PHY_DVRFTCTRL);
	return result;
}

/* DDR HW training process */
int ddrtrn_hal_hw_training_process(unsigned int item)
{
	unsigned int count;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int init_ctrl = ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL);
	if (item == 0)
		return 0;
	ddrtrn_debug("base_phy[%x] itme[%x]", base_phy, item);
	/* hardware training enable */
	ddrtrn_reg_write(item | PHY_PHYINITCTRL_INIT_EN | init_ctrl,
					 base_phy + DDR_PHY_PHYINITCTRL);

	if ((item & PHY_PHYINITCTRL_DRAM_RST) != 0 && (item & PHY_PHYINITCTRL_DRAM_INIT_EN) != 0) {
		if (ddrtrn_training_ctrl_easr(DDR_EXIT_SREF))
			return -1;
	}
	count = DDR_HWR_WAIT_TIMEOUT;
	/* auto cleared to 0 after training finished */
	while (count--) {
		if ((ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITCTRL) &
				PHY_PHYINITCTRL_MASK) == 0) {
			break;
		}
	}
	if (count == 0xffffffff) {
		ddrtrn_fatal("HWR wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, base_phy, item,
								 ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		return -1;
	}
	if (ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS) &
			(~PHY_PHYINITSTATUS_ZCAL_ERROR)) {
		ddrtrn_fatal("Phy[%x] hw[%x] failed[%x]", base_phy, item,
					 ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		ddrtrn_hal_training_stat(DDR_ERR_HW_RD_DATAEYE, base_phy, item,
								 ddrtrn_reg_read(base_phy + DDR_PHY_PHYINITSTATUS));
		return -1;
	}
	return 0;
}

void ddrtrn_hal_hw_clear_rdq(unsigned int i)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqnbdl0(ddrtrn_hal_get_rank_id(), i));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqnbdl1(ddrtrn_hal_get_rank_id(), i));
	ddrtrn_reg_write(0, base_phy + ddrtrn_hal_phy_dxnrdqsdly(i));
}

static void ddrtrn_hal_set_phy_dxnrdqsdly(unsigned int index, unsigned int value)
{
	ddrtrn_reg_write(value,
					 ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(index));
}
static unsigned int ddrtrn_hal_get_phy_dxnrdqsdly(unsigned int index)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + ddrtrn_hal_phy_dxnrdqsdly(index));
}

static void ddrtrn_hal_set_phy_rain_ctrl(unsigned int value)
{
	ddrtrn_reg_write(value, ddrtrn_hal_get_cur_phy() + DDR_PHY_TRAINCTRL12);
}

static unsigned int ddrtrn_hal_get_phy_rain_ctrl(void)
{
	return ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_TRAINCTRL12);
}

int ddrtrn_hal_hw_restore_rdqsbdl(unsigned int bdl_0, unsigned int bdl_1, unsigned int index)
{
	unsigned int rdqs_rank0, rdqs_rank1;
	int offset;
	/* struct ddrtrn_rdqs_data store the whole register value */
	rdqs_rank0 =
		(bdl_0 >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	rdqs_rank1 =
		(bdl_1 >> PHY_RDQS_BDL_BIT) & PHY_RDQS_BDL_MASK;
	ddrtrn_hal_set_cur_byte(index);
	if (rdqs_rank0 > rdqs_rank1) {
		offset = rdqs_rank0 - rdqs_rank1;
		ddrtrn_hal_set_phy_dxnrdqsdly(index, bdl_0);
		ddrtrn_hal_set_rank_id(1); /* switch to rank1 for sync rank1 rdq */
	} else {
		offset = rdqs_rank1 - rdqs_rank0;
		ddrtrn_hal_set_phy_dxnrdqsdly(index, bdl_1);
		ddrtrn_hal_set_rank_id(0); /* switch to rank0 for sync rank0 rdq */
	}
	return offset;
}

void ddrtrn_hal_hw_save_rdqsbdl(void)
{
	unsigned int temp;
	temp = ((ddrtrn_hal_get_phy_dxnrdqsdly(0) >>
			 PHY_RDQS_CYC_BIT) & PHY_RDQS_CYC_MASK) >> 2; /* right shift 2: 1/4T */
	temp = (ddrtrn_hal_get_phy_rain_ctrl() &
			(~(PHY_WL_FALLEDGE_BDL_JSTEP_R_MASK <<
			   PHY_WL_FALLEDGE_BDL_JSTEP_R_BIT))) |
		   (temp << PHY_WL_FALLEDGE_BDL_JSTEP_R_BIT);
	ddrtrn_hal_set_phy_rain_ctrl(temp);
}
#ifdef DDR_WRITE_DM_DISABLE
int ddrtrn_hal_hw_restore_write_dm(unsigned int ddrtrn_temp)
{
	int result;
	unsigned int temp1;
	unsigned int temp;
	unsigned int item = ddrtrn_hal_get_cur_item();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	ddrtrn_reg_write(ddrtrn_temp, base_phy + DDR_PHY_MODEREG45); /* restore 0xe0 */
	temp = ddrtrn_reg_read(base_phy + DDR_PHY_MRS_SEQ_PROG);
	temp1 = ddrtrn_reg_read(base_phy + DDR_PHY_DRAMCFG);
	ddrtrn_reg_write(PHY_MRS_SEQ_PROG_VAL, base_phy + DDR_PHY_MRS_SEQ_PROG); /* init MR5 */
	ddrtrn_reg_write(temp1 | PHY_WDM_DISABLE_VAL, base_phy + DDR_PHY_DRAMCFG); /* write dm disable */
	result = ddrtrn_hal_hw_training_process(item & PHY_PHYINITCTRL_DRAM_INIT_EN);
	ddrtrn_reg_write(temp, base_phy + DDR_PHY_MRS_SEQ_PROG); /* restore */
	ddrtrn_reg_write(temp1, base_phy + DDR_PHY_DRAMCFG); /* restore */
	return result;
}
#endif /* DDR_WRITE_DM_DISABLE */

#ifdef DDR_OE_CONFIG
void ddrtrn_ac_oe_enable(void)
{
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_PHYCTRL0) | (0x1 << PHY_PHYCTRL0_CMDOEN_BIT),
					 base_phy + DDR_PHY_PHYCTRL0);
	if (ddrtrn_hal_get_cur_phy_dram_type() == PHY_DRAMCFG_TYPE_LPDDR4) {
		ddrtrn_reg_write(ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTRL0) | ACPHYCTRL0_AC_OE_MASK,
						 base_phy + DDR_PHY_ACPHYCTRL0);
	} else {
		ddrtrn_reg_write((ddrtrn_reg_read(base_phy + DDR_PHY_ACPHYCTRL0) & ~(ACPHYCTRL0_AC_OE_MASK)) |
				  AC_OE_CK1OEN_DISABLE, base_phy + DDR_PHY_ACPHYCTRL0);
	}
}

void ddrtrn_dummy_io_oe_enable(void)
{
	unsigned int i;
	unsigned int byte_num = ddrtrn_hal_get_cur_phy_total_byte_num();
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int block_idx, byte_idx_in_block;
	for (i = 0; i < byte_num; i++) {
		byte_idx_in_block = i % NUM_FOR_REMAINING;
		block_idx = i / NUM_FOR_REMAINING;
		if (byte_idx_in_block == 0) {
			ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddr_phy_ioctl_rx_vcom_ctl0(block_idx)) |
					(0x1 << DUMMY0_IOCTL_OE_BIT), base_phy + ddr_phy_ioctl_rx_vcom_ctl0(block_idx));
		} else {
			ddrtrn_reg_write(ddrtrn_reg_read(base_phy + ddr_phy_ioctl_rx_vcom_ctl1(block_idx)) |
					(0x1 << DUMMY1_IOCTL_OE_BIT), base_phy + ddr_phy_ioctl_rx_vcom_ctl1(block_idx));
		}
	}
}
#endif /* DDR_OE_CONFIG */
#endif /* DDR_HW_TRAINING_CONFIG */
