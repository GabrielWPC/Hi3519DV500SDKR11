/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"
#include "ddrtrn_log.h"
#include "ddrtrn_training_custom.h"

#ifdef DDR_SW_TRAINING_FUNC_PUBLIC

int ddrtrn_training_boot_func(void)
{
	int result = 0;
	/* check hardware gating */
	if (ddrtrn_hal_get_gt_status(ddrtrn_hal_get_cur_phy()) != 0) {
		ddrtrn_fatal("PHY[%x] hw gating fail", ddrtrn_hal_get_cur_phy());
		ddrtrn_hal_training_stat(DDR_ERR_HW_GATING, ddrtrn_hal_get_cur_phy(), -1, -1);
	}
#ifdef DDR_LPCA_TRAINING_CONFIG
	/* lpca */
	result += ddrtrn_lpca_training_func();
#endif
	/* write leveling */
	result += ddrtrn_wl_func();
	/* dataeye/gate/vref need switch axi */
	/* dataeye */
	result += ddrtrn_dataeye_training_func();
#ifdef DDR_HW_TRAINING_CONFIG
	/* hardware read */
	if ((result != 0) && (ddrtrn_hal_check_bypass(DDR_BYPASS_HW_MASK) == DDR_FALSE)) {
		struct ddrtrn_hal_training_relate_reg relate_reg_ac;
		ddrtrn_hal_save_reg(&relate_reg_ac, DDR_BYPASS_HW_MASK);
		result = ddrtrn_hw_dataeye_read();
		ddrtrn_hal_restore_reg(&relate_reg_ac);
		ddrtrn_hal_set_adjust(DDR_DATAEYE_ABNORMAL_ADJUST);
		result += ddrtrn_dataeye_training();
	}
#endif
	/* mpr */
#ifdef DDR_MPR_TRAINING_CONFIG
	result += ddrtrn_mpr_training_func();
#endif
	/* gate */
	result += ddrtrn_gating_func();
	/* vref */
	result += ddrtrn_vref_training_func();
	return result;
}

int ddrtrn_training_by_dmc(void)
{
	if (ddrtrn_hal_get_cmd_flag() != CMD_DISABLE) {
#ifdef DDR_TRAINING_CMD
		return ddrtrn_training_cmd_func();
#endif
	} else {
		return ddrtrn_training_boot_func();
	}
	return 0;
}

int ddrtrn_training_by_rank(void)
{
	int result = 0;
	unsigned int i;
	ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_cur_phy(), ddrtrn_hal_get_rank_id());
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++) {
		ddrtrn_hal_set_dmc_id(i);
		ddrtrn_hal_set_cur_dmc(ddrtrn_hal_get_cur_dmc_addr(i));
		ddrtrn_hal_set_cur_pattern(ddrtrn_hal_get_cur_dmc_ddrt_pattern(i));
		result += ddrtrn_training_by_dmc();
	}
	return result;
}

int ddrtrn_training_by_phy(void)
{
	int result = 0;
	unsigned int i;
	const unsigned int phy_mask = 1 << (ddrtrn_hal_get_phy_id());
	unsigned int rank_num;
	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	for (i = 0; i < rank_num; i++) {
		ddrtrn_hal_set_rank_id(i);
		ddrtrn_hal_set_cur_item(ddrtrn_hal_get_cur_rank_item(i));
		if (ddrtrn_hal_check_bypass(phy_mask) != DDR_FALSE)
			continue;
		result += ddrtrn_training_by_rank();
	}
	if (rank_num == DDR_SUPPORT_RANK_MAX) {
		ddrtrn_hal_training_adjust_wdq();
		ddrtrn_hal_training_adjust_wdqs();
		ddrtrn_hal_phy_switch_rank(ddrtrn_hal_get_cur_phy(), 0x0); /* switch to rank0 */
	}
	return result;
}

int ddrtrn_training_all(void)
{
	int result = 0;
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_phy_num(); i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		result += ddrtrn_training_by_phy();
	}
	return result;
}

/* Support DDRC510 with two PHY */
int ddrtrn_sw_training_func(void)
{
	int result = 0;
	struct ddrtrn_hal_training_custom_reg reg;
#ifdef SYSCTRL_DDR_TRAINING_VERSION_FLAG
	ddrtrn_hal_version_flag();
#endif
	/* check sw ddr training enable */
	if (ddrtrn_hal_check_sw_item() == 0)
		return 0;
	ddrtrn_training_start();
	ddrtrn_set_data(&reg, 0, sizeof(struct ddrtrn_hal_training_custom_reg));
	/* save customer reg */
	if (ddrtrn_hal_boot_cmd_save(&reg) != 0)
		return -1;
#ifdef DDR_TRAINING_STAT_CONFIG
	/* clear stat register */
	ddrtrn_hal_clear_sysctrl_stat_reg();
#endif
	ddrtrn_hal_cmd_flag_disable();
	if (ddrtrn_hal_check_not_dcc_item() == 0)
		result = ddrtrn_training_all();
	result += ddrtrn_dcc_training_func();
	if (result == 0)
		ddrtrn_training_success();
	else
		ddrtrn_training_console_if();
	/* restore customer reg */
	ddrtrn_hal_boot_cmd_restore(&reg);
	return result;
}
#endif /* DDR_SW_TRAINING_FUNC_PUBLIC */

#ifdef DDR_HW_TRAINING_CONFIG
int ddrtrn_hw_training_func(void)
{
	return ddrtrn_hw_training();
}
#else
int ddrtrn_hw_training_func(void)
{
	ddrtrn_warning("Not support DDR HW training");
	return 0;
}
#endif /* DDR_HW_TRAINING_CONFIG */

int ddrtrn_sw_training_if(unsigned int sw_item_mask)
{
	ddrtrn_hal_cfg_init();
	ddrtrn_hal_sw_item_cfg(sw_item_mask);
	return DDR_SW_TRAINING_FUNC();
}

int ddrtrn_hw_training_if(void)
{
	return DDR_HW_TRAINING_FUNC();
}

/* trace function */
void ddrtrn_trace(const char *func_name, const unsigned int *val)
{
#ifdef DDR_TRAINING_TRACE_USUAL_PRINT
	printf("func_name : %s\n", func_name);
#endif
#ifdef DDR_TRAINING_TRACE_UART_EARLY_PRINT
	DDR_PUTS("func_name : ");
	DDR_PUTS(func_name);
	DDR_PUTS("\n\r");
#endif
#ifdef DDR_TRAINING_TRACE_SERIAL_PRINT
	serial_puts("func_name : ");
	serial_puts(func_name);
	serial_puts("\n");
#endif
#ifdef DDR_TRAINING_TRACE_REG_RECORD
	ddrtrn_reg_write(val, addr);
#endif
}
