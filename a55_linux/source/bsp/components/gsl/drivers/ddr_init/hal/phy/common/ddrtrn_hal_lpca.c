/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

#ifdef DDR_LPCA_TRAINING_CONFIG
/* Get address bdl default value */
void ddrtrn_hal_lpca_get_def(struct ddrtrn_ca_data *data)
{
	unsigned int index;
	for (index = 0; index < DDR_PHY_CA_REG_MAX; index++)
		data->def[index] = ddrtrn_reg_read(data->base_phy + ddr_phy_acaddrbdl(index));
}

/* Restore address bdl default value */
void ddrtrn_lpca_restore_def(struct ddrtrn_ca_data *data)
{
	unsigned int index;
	for (index = 0; index < DDR_PHY_CA_REG_MAX; index++)
		ddrtrn_reg_write(data->def[index], data->base_phy + ddr_phy_acaddrbdl(index));
	ddrtrn_hal_phy_cfg_update(data->base_phy);
}

/* Set address bdl value */
void ddrtrn_lpca_set_bdl(unsigned int base_phy, unsigned int bdl)
{
	unsigned int index;
	for (index = 0; index < DDR_PHY_CA_REG_MAX; index++)
		ddrtrn_reg_write(bdl | (bdl << PHY_ACADDRBDL_ADDR1_BIT),
						 base_phy + ddr_phy_acaddrbdl(index));
	ddrtrn_hal_phy_cfg_update(base_phy);
}

/* Update address bdl value with training result */
void ddrtrn_lpca_update_bdl(struct ddrtrn_ca_data *data)
{
	unsigned int index;
	unsigned int addr0, addr1;
	for (index = 0; index < DDR_PHY_CA_REG_MAX; index++) {
		addr0 = (data->left[index * 2] + data->right[index * 2]) >> 1; /* 2:ca middle value */
		addr1 = (data->left[index * 2 + 1] + data->right[index * 2 + 1]) >> 1; /* 2:ca middle value */
		ddrtrn_reg_write(addr0 | (addr1 << PHY_ACADDRBDL_ADDR1_BIT),
						 data->base_phy + ddr_phy_acaddrbdl(index));
	}
	ddrtrn_hal_phy_cfg_update(data->base_phy);
}

/* Display training result */
void ddrtrn_lpca_display(struct ddrtrn_ca_data *data)
{
#if defined(DDR_TRAINING_CMD)
	unsigned int index;
	ddrtrn_debug("CA phase[%x = %x]",
				 data->base_phy + DDR_PHY_ADDRPHBOUND,
				 ddrtrn_reg_read(data->base_phy + DDR_PHY_ADDRPHBOUND));
	for (index = 0; index < DDR_PHY_CA_MAX; index++)
		ddrtrn_debug("CA[%x] left[%x] right[%x]",
					 index, data->left[index], data->right[index]);
	ddrtrn_debug("min[%x] max[%x] done[%x]", data->min, data->max, data->done);
#endif
}

static void ddr_lpca_get_data(struct ddrtrn_ca_data *data, unsigned int index, unsigned int bdl)
{
	/* pass */
	if (data->left[index] == -1) {
		data->left[index] = bdl;
		/* set min left bound */
		if (bdl < data->min)
			data->min = bdl;
	}
	/* unstable border value or abnormal value */
	if ((data->right[index] != -1) && ((bdl - data->right[index]) > 1))
		ddrtrn_warning("CA[%x] bdl[%x] right[%x] ph[%x]",
					   index, bdl, data->right[index],
					   ddrtrn_reg_read(data->base_phy + DDR_PHY_ADDRPHBOUND));
	data->right[index] = bdl;
	data->done |= (0x1 << index);
	/* set max right bound */
	if (data->right[index] > data->max)
		data->max = data->right[index];
}

/* Compare dq result and pattern */
static int ddrtrn_hal_lpca_compare(struct ddrtrn_ca_bit *ca_bit,
								   unsigned int dq_result, unsigned int pattern_p,
								   unsigned int pattern_n, unsigned int index)
{
	if (((dq_result >> ca_bit->bit_p) & 0x1) != ((pattern_p >> index) & 0x1))
		return -1;
	if (((dq_result >> ca_bit->bit_n) & 0x1) != ((pattern_n >> index) & 0x1))
		return -1;
	return 0;
}

/* Check each CA whether pass */
void ddr_lpca_check(struct ddrtrn_ca_data *data, unsigned int bdl, unsigned int is_ca49)
{
	unsigned int dq_result = ddrtrn_reg_read(data->base_phy + DDR_PHY_PHYDQRESULT);
	unsigned int pattern_p = ddrtrn_reg_read(data->base_phy +
											 DDR_PHY_SWCATPATTERN_P) & PHY_CAT_PATTERN_MASK;
	unsigned int pattern_n = ddrtrn_reg_read(data->base_phy +
											 DDR_PHY_SWCATPATTERN_N) & PHY_CAT_PATTERN_MASK;
	unsigned int index;
	for (index = 0; index < DDR_PHY_CA_MAX; index++) {
		if (is_ca49) {
			if (index != 4 && index != 9) /* ca4 ca9 */
				continue;
		} else {
			if (index == 4 || index == 9) /* ca4 ca9 */
				continue;
		}
		/* compare result and pattern */
		if (ddrtrn_hal_lpca_compare(&data->bits[index], dq_result, pattern_p, pattern_n, index) == 0)
			ddr_lpca_get_data(data, index, bdl); /* pass */
	}
}

#ifdef DDR_LPCA_GET_BIT
void ddrtrn_hal_lpca_get_bit(unsigned int index, struct ddrtrn_ca_data *data)
{
	unsigned int swap_sel;
	ddrtrn_reg_write(index + 1, data->base_phy + DDR_PHY_CATSWAPINDEX);
	swap_sel = ddrtrn_reg_read(data->base_phy + DDR_PHY_CATSWAPSEL);
	data->bits[index * 2].bit_p = /* ca 0/2/4/6 Rising edge */
		swap_sel & PHY_CATSWAPSEL_BIT_MASK;
	data->bits[index * 2].bit_n = /* ca 0/2/4/6 Falling edge */
		(swap_sel >> 8) & PHY_CATSWAPSEL_BIT_MASK; /* bit8 */
	data->bits[index * 2 + 1].bit_p = /* ca * 2 + 1: bit 1/3/5/7 Rising edge */
		(swap_sel >> 16) & PHY_CATSWAPSEL_BIT_MASK; /* bit16 */
	data->bits[index * 2 + 1].bit_n = /* ca * 2 + 1: bit 1/3/5/7 Falling edge */
		(swap_sel >> 24) & PHY_CATSWAPSEL_BIT_MASK; /* bit24 */
}
#endif

unsigned int ddrtrn_hal_check_lowpower_support(void)
{
	if ((ddrtrn_reg_read(ddrtrn_hal_get_cur_phy() + DDR_PHY_DRAMCFG) & PHY_DRAMCFG_TYPE_LPDDR3) ==
			PHY_DRAMCFG_TYPE_LPDDR3)
		return 1;
	return 0;
}
#endif /* DDR_LPCA_TRAINING_CONFIG */