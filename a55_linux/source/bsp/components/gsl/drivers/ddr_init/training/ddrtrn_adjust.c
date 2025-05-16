/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_TRAINING_ADJUST_CONFIG
/*
 * @accel : Return a value to adjust quickly.
 * Check dataeye DQ window on left or right or middle.
 */
static unsigned int ddrtrn_adjust_trend_check(int *accel)
{
	unsigned int dq_bdl;
	unsigned int size;
	/* 32 BDL middle[13, 17]. 128 BDL middle[40, 56] */
	/* 1 Phase = (DDR_BDL_PHASE_TRANSFORM) BDL */
	size = DDR_BDL_PHASE_TRANSFORM >> 1;
	dq_bdl = ddrtrn_hal_adjust_get_average();
	/* increase adjust step to accelerate */
	if (accel != NULL) {
		if (dq_bdl > PHY_DQ_BDL_MIDDLE)
			*accel = dq_bdl - PHY_DQ_BDL_MIDDLE;
		else if (dq_bdl < PHY_DQ_BDL_MIDDLE)
			*accel = PHY_DQ_BDL_MIDDLE - dq_bdl;
		ddrtrn_info("byte[%x] bdl[%x] middle[%x] accel[%x] rdqs[%x]",
					ddrtrn_hal_get_cur_byte(), dq_bdl, PHY_DQ_BDL_MIDDLE, *accel,
					ddrtrn_hal_adjust_get_rdqs());
	}
	/* window on left */
	if (dq_bdl < (PHY_DQ_BDL_MIDDLE - size)) {
		return DDR_WIN_LEFT;
		/* on right */
	} else if (dq_bdl > (PHY_DQ_BDL_MIDDLE + size)) {
		return DDR_WIN_RIGHT;
	} else {
		return DDR_WIN_MIDDLE;
	}
}

/* Check adjust value whether valid */
static int ddrtrn_adjust_check_val(int val, unsigned int mode)
{
	if (mode == DDR_MODE_READ) {
		if (val < 0 || val > PHY_RDQS_BDL_MASK)
			return DDR_FALSE;
	} else {
		if (val < 0 || val > PHY_WDQ_PHASE_MASK)
			return DDR_FALSE;
	}
	return DDR_TRUE;
}

/* Add or delete value to adjust */
static void ddrtrn_adjust_change_val(unsigned int dir, int *val, int step, unsigned int mode)
{
	if (mode == DDR_MODE_READ) {
		if (dir == DDR_WIN_RIGHT)
			(*val) = (*val) + step;
		else
			(*val) = (*val) - step;
	} else {
		/* decrease wdq phase, window move to right */
		if (dir == DDR_WIN_RIGHT)
			(*val) = (*val) - step;
		else
			(*val) = (*val) + step;
	}
}

/*
 * @dir : move direction. DDR_TRUE move to right, DDR_FALSE move to left.
 * Move window to specified direction until the best DQ bdl beyond the midline.
 */
static void ddrtrn_adjust_move_win(struct ddrtrn_training_result_data *training, int step, unsigned int dir)
{
	int cur_val, def_val;
	int accel;
	unsigned int i;
	unsigned int trend;
	unsigned int max_value;
	max_value = (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE ? PHY_WDQ_PHASE_MASK : PHY_RDQS_BDL_MASK);
	def_val = ddrtrn_hal_adjust_get_val();
	cur_val = def_val;
	for (i = 0; i <= max_value; i++) {
		accel = step;
		/* write mode no need to accelerate */
		if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE)
			trend = ddrtrn_adjust_trend_check(0);
		else
			trend = ddrtrn_adjust_trend_check(&accel);
		if (trend == DDR_WIN_MIDDLE || dir == trend) {
			ddrtrn_debug("Move byte[%x] window to middle suc", ddrtrn_hal_get_cur_byte());
			break;
		}
		ddrtrn_adjust_change_val(dir, &cur_val, accel, ddrtrn_hal_get_cur_mode());
		if (ddrtrn_adjust_check_val(cur_val, ddrtrn_hal_get_cur_mode()) == DDR_FALSE) {
			ddrtrn_warning("Move byte[%x] to middle fail. value[%x]",
						   ddrtrn_hal_get_cur_byte(), cur_val);
			break;
		}
		ddrtrn_debug("Byte[%x] mode[%x] set value[%x]", ddrtrn_hal_get_cur_byte(),
					 ddrtrn_hal_get_cur_mode(), cur_val);
		ddrtrn_hal_adjust_set_val(cur_val);
		if (ddrtrn_dataeye_deskew(training)) {
			ddrtrn_hal_adjust_set_val(def_val);
			/* MUST deskew dataeye after restore rdqs */
			ddrtrn_dataeye_deskew(training);
			ddrtrn_error("Byte[%x] deskew fail, restore[%x]", ddrtrn_hal_get_cur_byte(), def_val);
			break;
		}
	}
}

/* Adjust specified byte winodw to middle */
static void ddrtrn_adjust_byte(struct ddrtrn_training_result_data *training)
{
	unsigned int trend;
	trend = ddrtrn_adjust_trend_check(0);
	/* window on left, move to right */
	if (trend == DDR_WIN_LEFT) {
		ddrtrn_adjust_move_win(training, DDR_DQS_ADJ_STEP, DDR_WIN_RIGHT);
		/* window on right, move to left */
	} else if (trend == DDR_WIN_RIGHT) {
		ddrtrn_adjust_move_win(training, DDR_DQS_ADJ_STEP, DDR_WIN_LEFT);
		/* window on middle, no need to move */
	} else {
		ddrtrn_debug("Byte[%x] mode[%x] win on middle", ddrtrn_hal_get_cur_byte(),
					 ddrtrn_hal_get_cur_mode());
	}
}

/*
 * Adjust PHY dataeye. On normal case,
 * read dateeye window on left after read dataeye hardware training,
 * write dataeye window on left after write leveling training.
 */
void ddrtrn_adjust_dataeye(struct ddrtrn_training_result_data *training)
{
	unsigned int i;
	unsigned int adjust;
	/* dataeye adjust disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_DATAEYE_ADJ_MASK) != DDR_FALSE)
		return;
	ddrtrn_debug("DDR dataeye adjust PHY[%x][%x] DMC[%x][%x] Rank[%x]",
				 ddrtrn_hal_get_phy_id(), ddrtrn_hal_get_cur_phy(),
				 ddrtrn_hal_get_dmc_id(), ddrtrn_hal_get_cur_dmc(), ddrtrn_hal_get_rank_id());
	adjust = ddrtrn_hal_get_adjust();
	if (adjust == DDR_FALSE)
		return;
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		ddrtrn_hal_set_cur_byte(i + (ddrtrn_hal_get_dmc_id() << 1)); /* byte index accord to phy */
		ddrtrn_adjust_byte(training);
	}
}
#else
#define ddrtrn_adjust_dataeye(training)
#endif /* DDR_TRAINING_ADJUST_CONFIG */
