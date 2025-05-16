/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "ddrtrn_interface.h"

#ifdef DDR_LPCA_TRAINING_CONFIG
/* Reset address bdl training data */
static void ddrtrn_lpca_reset(struct ddrtrn_ca_data *data)
{
	unsigned int index;
	for (index = 0; index < DDR_PHY_CA_MAX; index++) {
		data->left[index] = -1;
		data->right[index] = -1;
	}
	data->min = PHY_ACADDR_BDL_MASK;
	data->max = 0;
	data->done = 0;
}

/* Get ca bit relation */
void ddrtrn_lpca_get_bit(struct ddrtrn_ca_data *data)
{
	unsigned int index;
	/* get ca bit in four register */
#ifdef DDR_LPCA_GET_BIT
	for (index = 0; index < (DDR_PHY_CA_REG_MAX - 1); index++)
		ddrtrn_hal_lpca_get_bit(index, data);
#else
	for (index = 0; index < (DDR_PHY_CA_REG_MAX - 1); index++) {
		data->bits[index * 2].bit_p = index * 4 + 0; /* ca 0/2/4/6 Rising edge, dq*4+0:0/4/8/12 */
		data->bits[index * 2].bit_n = index * 4 + 1; /* ca 0/2/4/6 Falling edge, dq*4+1:1/5/9/13 */
		data->bits[index * 2 + 1].bit_p = index * 4 + 2; /* ca*2+1: bit 1/3/5/7 Rising edge, dq*4+2:2/6/10/14 */
		data->bits[index * 2 + 1].bit_n = index * 4 + 3; /* ca*2+1: bit 1/3/5/7 Falling edge, dq*4+3:2/6/10/14 */
	}
#endif
	/*
	 * set ca bit for ca4 and ca9
	 * ca4 equal ca0 ca9 equal ca5
	 */
	for (index = 8; index > 4; index--) { /* set ca bit for ca5 to ca8 equal ca4 to ca7 */
		data->bits[index].bit_p = data->bits[index - 1].bit_p;
		data->bits[index].bit_n = data->bits[index - 1].bit_n;
	}
	data->bits[4].bit_p = data->bits[0].bit_p; /* ca4 equal ca0 */
	data->bits[4].bit_n = data->bits[0].bit_n; /* ca4 equal ca0 */
	data->bits[9].bit_p = data->bits[5].bit_p; /* ca9 equal ca5 */
	data->bits[9].bit_n = data->bits[5].bit_n; /* ca9 equal ca5 */
#if defined(DDR_TRAINING_CMD)
	for (index = 0; index < DDR_PHY_CA_MAX; index++) {
		ddrtrn_info("CA[%x] bit_p[%x]", index, data->bits[index].bit_p);
		ddrtrn_info("CA[%x] bit_n[%x]", index, data->bits[index].bit_n);
	}
#endif
}

/* Init data before training */
static void ddrtrn_lpca_init(unsigned int base_dmc, unsigned int base_phy, struct ddrtrn_ca_data *data)
{
	data->base_dmc = base_dmc;
	data->base_phy = base_phy;
	/* gat ca bit relation */
	ddrtrn_hal_lpca_get_bit(data);
	/* get ac addr bdl default value */
	ddrtrn_hal_lpca_get_def(data);
	/* reset training data */
	ddrtrn_lpca_reset(data);
}

/* Wait lpca command done */
static void ddrtrn_lpca_wait(volatile union ddrtrn_phy_cat_config *ca)
{
	unsigned int count = 0;
	while (count < DDR_LPCA_WAIT_TIMEOUT) {
		if (ca->bits.sw_cat_dqvalid == 1) {
			ca->bits.sw_cat_dqvalid = 0; /* clear */
			break;
		}
		count++;
	}
	/* generally, count is 0 */
	if (count >= DDR_LPCA_WAIT_TIMEOUT)
		ddrtrn_error("LPCA wait timeout");
}

/* Excute lpca command and check result */
static void ddrtrn_lpca_excute(struct ddrtrn_ca_data *data, unsigned int bdl, unsigned int is_ca49)
{
	volatile union ddrtrn_phy_cat_config *ca = (union ddrtrn_phy_cat_config *)
												   (data->base_phy + DDR_PHY_CATCONFIG);
	if (is_ca49)
		ca->bits.sw_cat_mrw48 = 1;
	else
		ca->bits.sw_cat_mrw41 = 1;
	ddrtrn_lpca_wait(ca);
	ca->bits.sw_cat_cke_low = 1;
	ddrtrn_lpca_wait(ca);
	ca->bits.sw_cat_strobe = 1;
	ddrtrn_lpca_wait(ca);
	/* check PHYDQRESULT */
	ddr_lpca_check(data, bdl, is_ca49);
	ca->bits.sw_cat_cke_high = 1;
	ddrtrn_lpca_wait(ca);
	ca->bits.sw_cat_mrw42 = 1;
	ddrtrn_lpca_wait(ca);
}

/* Find address bdl */
static int ddrtrn_lpca_find_bdl(struct ddrtrn_ca_data *data)
{
	unsigned int bdl;
	for (bdl = 0; bdl <= PHY_ACADDR_BDL_MASK; bdl++) {
		/* update bdl */
		ddrtrn_lpca_set_bdl(data->base_phy, bdl);
		/* ca0~ca3, ca5~ca8 */
		ddrtrn_lpca_excute(data, bdl, DDR_FALSE);
		/* ca4, ca9 */
		ddrtrn_lpca_excute(data, bdl, DDR_TRUE);
	}
	if (data->done == PHY_CAT_PATTERN_MASK)
		return 0;
	return -1;
}

/* Loop phase to find valid bdl and phase */
static int ddrtrn_lpca_loop_phase(struct ddrtrn_ca_data *data, int step)
{
	volatile union ddrtrn_phy_addr_phbound *ph = (union ddrtrn_phy_addr_phbound *)
													 (data->base_phy + DDR_PHY_ADDRPHBOUND);
	unsigned int phase;
	unsigned int addrph_def = ph->bits.addrph_a;
	int addrph = addrph_def;
	for (phase = 0; phase <= PHY_ADDRPH_MASK; phase++) {
		/* reset ca training data */
		ddrtrn_lpca_reset(data);
		/* find bdl */
		if (ddrtrn_lpca_find_bdl(data) == 0)
			return 0;
		addrph += step;
		if (addrph < 0 || addrph > PHY_ADDRPH_MASK)
			break;
		ph->bits.addrph_a = addrph;
		ddrtrn_hal_phy_cfg_update(data->base_phy);
	}
	/* restore default value */
	ddrtrn_debug("current phase[%x = %x], restore default[%x]", ph, *ph, addrph_def);
	ph->bits.addrph_a = addrph_def;
	return -1;
}

/* Find a valid phase */
static int ddrtrn_lpca_find_phase(struct ddrtrn_ca_data *data)
{
	/* increase default value to find */
	if (ddrtrn_lpca_loop_phase(data, 1) == 0)
		return 0;
	/* decrease default value to find */
	if (ddrtrn_lpca_loop_phase(data, -1) == 0)
		return 0;
	return -1;
}

/* Set step to adjust address window */
static int ddrtrn_lpca_set_step(struct ddrtrn_ca_data *data)
{
	/* max window, no need to found */
	if (data->min == 0 && data->max == PHY_ACADDR_BDL_MASK)
		return 0;
	if (data->min == 0)
		return -1; /* window on left, move to right */
	else
		return 1; /* window on right, move to left */
}

/*
 * Adjust address window via change phase.
 * Increase phase, window will move to left.
 */
static void ddrtrn_lpca_adjust(struct ddrtrn_ca_data *data)
{
	int step;
	volatile union ddrtrn_phy_addr_phbound *ph = (union ddrtrn_phy_addr_phbound *)
													 (data->base_phy + DDR_PHY_ADDRPHBOUND);
	unsigned int phase;
	unsigned int addrph_last = ph->bits.addrph_a;
	int addrph_cur = addrph_last;
	/* set step to increase or decrease phase */
	step = ddrtrn_lpca_set_step(data);
	if (step == 0)
		return;
	for (phase = 0; phase <= PHY_ADDRPH_MASK; phase++) {
		addrph_cur += step;
		if (addrph_cur < 0 || addrph_cur > PHY_ADDRPH_MASK)
			return;
		ph->bits.addrph_a = addrph_cur;
		ddrtrn_hal_phy_cfg_update(data->base_phy);
		/* reset ca training data */
		ddrtrn_lpca_reset(data);
		if (ddrtrn_lpca_find_bdl(data)) {
			/* not find bdl, restore last value */
			addrph_cur -= step;
			ddrtrn_lpca_find_bdl(data);
			return;
		}
		/* max window: ------- */
		if (data->min == 0 && data->max == PHY_ACADDR_BDL_MASK)
			return;
		/* last window: -----xx */
		if (data->min == 0 && step == 1) {
			/* last value is best */
			addrph_cur -= step;
			ph->bits.addrph_a = addrph_cur;
			ddrtrn_hal_phy_cfg_update(data->base_phy);
			ddrtrn_lpca_reset(data);
			ddrtrn_lpca_find_bdl(data);
			return;
		}
		/* best window: x-----x */
		if (data->min > 0 && step == -1)
			return;
	}
}

/* Low power DDR CA training */
int ddrtrn_lpca_training(void)
{
	volatile union ddrtrn_phy_cat_config *ca = (union ddrtrn_phy_cat_config *)
												   (ddrtrn_hal_get_cur_phy() + DDR_PHY_CATCONFIG);
	struct ddrtrn_ca_data data;
	int ret;
	ddrtrn_debug("DDR LPCA training");
	/* init data */
	ddrtrn_lpca_init(ddrtrn_hal_get_cur_dmc(), ddrtrn_hal_get_cur_phy(), &data);
	/* enable sw ca training, wait 62.5ns */
	ca->bits.sw_cat_en = 1;
	/* find a valid phase first */
	ret = ddrtrn_lpca_find_phase(&data);
	/* display training result */
	ddrtrn_lpca_display(&data);
	if (ret) {
		/* restore default value when fail */
		ddrtrn_lpca_restore_def(&data);
		ddrtrn_error("PHY[%x] found phase fail, result[%x]", ddrtrn_hal_get_cur_phy(), data.done);
		ddrtrn_hal_training_stat(DDR_ERR_LPCA, ddrtrn_hal_get_cur_phy(), -1, -1);
	} else {
		/* adjust window via phase */
		ddrtrn_lpca_adjust(&data);
		ddrtrn_lpca_display(&data);
		/* set training result */
		ddrtrn_lpca_update_bdl(&data);
	}
	/* disable sw ca training */
	ca->bits.sw_cat_en = 0;
	/* save lpca result data to printf */
	ddrtrn_lpca_data_save(&data);
	return ret;
}

int ddrtrn_lpca_training_func(void)
{
	int result = 0;
	struct ddrtrn_hal_training_relate_reg relate_reg;
	/* LPCA training disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_LPCA_MASK) != DDR_FALSE)
		return 0;
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_LPCA_MASK);
	/* only lowpower ddr3 support */
	if (ddrtrn_hal_check_lowpower_support())
		result += ddrtrn_lpca_training(void);
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_lpca_training_func(void)
{
	ddrtrn_warning("Not support LPDDR CA training");
	return 0;
}
#endif /* DDR_LPCA_TRAINING_CONFIG */