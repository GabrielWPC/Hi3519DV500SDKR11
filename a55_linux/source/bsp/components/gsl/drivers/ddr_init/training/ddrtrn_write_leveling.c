/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

#ifdef DDR_WL_TRAINING_CONFIG
void ddrtrn_wl_enable(unsigned int base_dmc, unsigned int base_phy)
{
	ddrtrn_hal_wl_switch(base_dmc, base_phy, DDR_TRUE);
}

void ddrtrn_wl_disable(unsigned int base_dmc, unsigned int base_phy)
{
	ddrtrn_hal_wl_switch(base_dmc, base_phy, DDR_FALSE);
}

void ddrtrn_wl_error(unsigned int type, unsigned int byte_num,
					 unsigned int wl_result, unsigned int base_phy)
{
	int j;
	if (type == DDR_DELAY_BDL) {
		ddrtrn_fatal("PHY[%x] WL fail, result[%x]", base_phy, wl_result);
		for (j = 0; j < byte_num; j++) {
			if ((wl_result & (1 << j)) == 0)
				ddrtrn_hal_training_stat(DDR_ERR_WL, base_phy, j, -1);
		}
	} else {
		ddrtrn_debug("PHY[%x] WL not found phase, result[%x]", base_phy,
					 wl_result);
	}
}

/*
 * Find WDQS delay, sync to WDQ delay and OE delay.
 * WL depend default WDQS phase value in register init table.
 */
int ddrtrn_write_leveling(void)
{
	unsigned int i;
	struct ddrtrn_hal_delay wdqs_old;
	struct ddrtrn_hal_delay wdqs_new;
	int result;
	unsigned int base_phy = ddrtrn_hal_get_cur_phy();
	unsigned int base_dmc = ddrtrn_hal_get_cur_dmc();
	unsigned int byte_num = ddrtrn_hal_get_byte_num();
	ddrtrn_debug("DDR Write Leveling training");
	/* init wdqs */
	for (i = 0; i < byte_num; i++)
		ddrtrn_hal_wl_init_wdqs(&wdqs_old, &wdqs_new, i);
	/* enable sw write leveling mode */
	ddrtrn_wl_enable(base_dmc, base_phy);
	/* find first WDQS phase, assume CLK delay > DQS delay. */
	result = ddrtrn_hal_wl_process(DDR_DELAY_PHASE, &wdqs_new);
	/* check phase result */
	for (i = 0; i < byte_num; i++)
		ddrtrn_hal_wl_check_phase_result(wdqs_new, i, result);
	/* find WDQS bdl */
	result = ddrtrn_hal_wl_process(DDR_DELAY_BDL, &wdqs_new);
	/* disable sw write leveling mode */
	ddrtrn_wl_disable(base_dmc, base_phy);
	if (result) {
		/* restore default value when find WDQS fail */
		for (i = 0; i < byte_num; i++)
			ddrtrn_hal_wl_restore_default_value(&wdqs_old, i);
		ddrtrn_hal_phy_cfg_update(base_phy);
		return -1;
	}
	/* sync delay */
	ddrtrn_hal_wl_bdl_sync(&wdqs_new, &wdqs_old);
#ifdef DDR_WL_DATAEYE_ADJUST_CONFIG
	/* adjust WDQ for dataeye */
	ddrtrn_hal_wl_wdq_adjust(&wdqs_new, &wdqs_old);
#endif
	return 0;
}

int ddrtrn_wl_func(void)
{
	struct ddrtrn_hal_training_relate_reg relate_reg;
	int result = 0;
	/* write leveling disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_WL_MASK) != DDR_FALSE)
		return 0;
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_WL_MASK);
	result += ddrtrn_write_leveling();
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_wl_func(void)
{
	ddrtrn_warning("Not support DDR WL training");
	return 0;
}
#endif /* DDR_WL_TRAINING_CONFIG */
