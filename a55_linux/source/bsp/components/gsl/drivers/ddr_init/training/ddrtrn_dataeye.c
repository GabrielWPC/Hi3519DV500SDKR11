/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_DATAEYE_TRAINING_CONFIG
/* Check dataeye dq */
int ddrtrn_dataeye_check_dq(void)
{
	unsigned int dq_check_type;
	dq_check_type = ddrtrn_hal_get_dq_type();
	if (dq_check_type == DDR_CHECK_TYPE_DDRT) {
		return ddrtrn_ddrt_check();
	} else if (dq_check_type == DDR_CHECK_TYPE_MPR) {
#ifdef DDR_MPR_TRAINING_CONFIG
		return ddrtrn_mpr_check();
#else
		return 0;
#endif
	} else {
		ddrtrn_error("DDR dataeye dq check type not set");
	}
	return 0;
}

/* Check dq whether valid and set mask to reduce search time */
static int ddrtrn_dataeye_check_dir(unsigned int direction, unsigned int left,
									unsigned int right, unsigned int *mask)
{
	int result;
	result = ddrtrn_dataeye_check_dq();
	switch (direction) {
	case DDR_FIND_DQ_BOTH:
		*mask = DDR_FIND_DQ_LEFT | DDR_FIND_DQ_RIGHT;
		break;
	case DDR_FIND_DQ_LEFT:
		if (result) {
			/* ddr test error, search opposite side */
			*mask = DDR_FIND_DQ_RIGHT;
		} else { /* ddr test ok */
			ddrtrn_hal_phy_set_dq_bdl(left);
			if (ddrtrn_dataeye_check_dq() == 0)
				/* test ok, go on search this side */
				*mask = DDR_FIND_DQ_LEFT;
		}
		break;
	case DDR_FIND_DQ_RIGHT:
		if (result) { /* ddr test error, search opposite side */
			*mask = DDR_FIND_DQ_LEFT;
		} else { /* ddr test ok */
			ddrtrn_hal_phy_set_dq_bdl(right);
			if (ddrtrn_dataeye_check_dq() == 0)
				/* test OK, go on search this side */
				*mask = DDR_FIND_DQ_RIGHT;
		}
		break;
	default:
		break;
	}
	return result;
}

/* Binary search the valid dq bdl */
static void ddrtrn_dataeye_search_dq(unsigned int left, unsigned int right, int *target, unsigned int direction)
{
	unsigned int middle;
	unsigned int mask = 0;
	middle = left + ((right - left) >> 1);
	ddrtrn_hal_phy_set_dq_bdl(middle);
	if (ddrtrn_dataeye_check_dir(direction, left, right, &mask) == 0) { /* test ok */
		*target = (int)middle;
		return;
	}
	if (left == middle || middle == right) /* not found */
		return;
	/* find left side */
	if (DDR_FIND_DQ_LEFT & mask)
		ddrtrn_dataeye_search_dq(left, middle, target, direction);
	/* find right side */
	if (DDR_FIND_DQ_RIGHT & mask)
		ddrtrn_dataeye_search_dq(middle, right, target, direction);
	return;
}

/* Find DQ valid range */
static void ddrtrn_dataeye_find_dq(struct ddrtrn_training_result_data *training)
{
	int cur_dq, left_dq, right_dq, def_dq;
	unsigned int dq_num;
	unsigned int win_num;
	dq_num = (ddrtrn_hal_get_cur_byte() << DDR_BYTE_DQ) + ddrtrn_hal_get_cur_dq();
	def_dq = (int)ddrtrn_hal_phy_get_dq_bdl();
	cur_dq = def_dq;
	/* check default dq */
	if (ddrtrn_dataeye_check_dq()) {
		/* test error */
		cur_dq = -1;
		ddrtrn_dataeye_search_dq(0, PHY_BDL_MASK, &cur_dq, DDR_FIND_DQ_BOTH);
		ddrtrn_debug("DQ[%x] def[%x] not ok, find new value[%x]", dq_num, def_dq, cur_dq);
		if (cur_dq == -1) { /* no valid dq */
			training->ddrtrn_bit_result[dq_num] = 0;
			training->ddrtrn_bit_best[dq_num] = 0;
			/* restore default value */
			ddrtrn_hal_phy_set_dq_bdl(def_dq);
			ddrtrn_warning("DQ[%x] not found dq. restore[%x]", dq_num, def_dq);
			return;
		}
	}
	/* find the left boundary */
	left_dq = cur_dq;
	ddrtrn_dataeye_search_dq(0, cur_dq, &left_dq, DDR_FIND_DQ_LEFT);
	while (left_dq > 0) {
		left_dq--;
		ddrtrn_hal_phy_set_dq_bdl(left_dq);
		if (ddrtrn_dataeye_check_dq()) {
			/* test error */
			left_dq++;
			break;
		}
	}
	/* find the right boundary */
	right_dq = cur_dq;
	ddrtrn_dataeye_search_dq(cur_dq, PHY_BDL_MASK, &right_dq, DDR_FIND_DQ_RIGHT);
	while (right_dq < PHY_BDL_MASK) {
		right_dq++;
		ddrtrn_hal_phy_set_dq_bdl(right_dq);
		if (ddrtrn_dataeye_check_dq()) {
			/* test error */
			right_dq--;
			break;
		}
	}
	/* reset dq */
	ddrtrn_hal_phy_set_dq_bdl(def_dq);
	/*
	 * 0 1 2 3 4 5 6 7 8 9
	 * x x - - - - - x x x
	 * |       |
	 * left_dq   right_dq
	 *
	 * so left_dq = 2, right_dq = 6
	 */
	/* set result */
	win_num = (unsigned int)right_dq - (unsigned int)left_dq + 1;
	training->ddrtrn_bit_result[dq_num] = ((unsigned int)left_dq << DDR_DATAEYE_RESULT_BIT) | (unsigned int)right_dq;
	training->ddrtrn_bit_best[dq_num] = (win_num << DDR_DATAEYE_RESULT_BIT) | ((win_num >> 1) + (unsigned int)left_dq);
	ddrtrn_info("DQ[%x] range: left[%x] right[%x] best[%x] mode[%x] rank[%x]",
				dq_num, left_dq, right_dq, training->ddrtrn_bit_best[dq_num],
				ddrtrn_hal_get_cur_mode(), ddrtrn_hal_get_rank_id());
}

static void ddrtrn_dataeye_deskew_process(struct ddrtrn_training_result_data *training,
	unsigned int *dq_sum, unsigned int *cur_dq_sum)
{
	int i;
	unsigned int dq_num;
	unsigned int win_num;
	unsigned int def_dq, best_dq;
	unsigned int loop_times = 0;
	unsigned int byte_index = ddrtrn_hal_get_cur_byte();
	for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
		ddrtrn_hal_set_cur_dq(i);
		dq_num = (byte_index << DDR_BYTE_DQ) + i;
		def_dq = ddrtrn_hal_phy_get_dq_bdl();
		*cur_dq_sum += def_dq;
		ddrtrn_dataeye_find_dq(training);
		win_num = training->ddrtrn_bit_best[dq_num] >> DDR_DATAEYE_RESULT_BIT;
		best_dq = training->ddrtrn_bit_best[dq_num] & DDR_DATAEYE_RESULT_MASK;
		/* check window number */
		if (win_num < DDR_DATAEYE_WIN_NUM) {
			if (loop_times < DDR_LOOP_TIMES_LMT) {
				loop_times++;
				i--;
				continue;
			} else if (win_num == 0) {
				ddrtrn_warning("Byte[%x] DQ[%x] no win", byte_index, dq_num);
				/* restore default value */
				ddrtrn_hal_phy_set_dq_bdl(def_dq);
				ddrtrn_hal_training_stat(DDR_ERR_DATAEYE, ddrtrn_hal_get_cur_phy(), byte_index, i);
				continue;
			}
		}
		loop_times = 0;
		ddrtrn_hal_phy_set_dq_bdl(best_dq);
		*dq_sum = *dq_sum + best_dq;
		training->ddrtrn_win_sum = training->ddrtrn_win_sum + win_num;
	}
}

/* DDR dataeye training one byte */
int ddrtrn_dataeye_deskew(struct ddrtrn_training_result_data *training)
{
	unsigned int dq_sum;
	unsigned int cur_dq_sum = 0;
	unsigned int cur_dq_average;
	unsigned int cur_dm;
	cur_dm = ddrtrn_hal_dataeye_get_dm();
	dq_sum = 0;
	training->ddrtrn_win_sum = 0;
	ddrtrn_dataeye_deskew_process(training, &dq_sum, &cur_dq_sum);
	cur_dq_average = cur_dq_sum >> DDR_BYTE_DQ;
	dq_sum = (dq_sum >> DDR_BYTE_DQ) + cur_dm;
	if (dq_sum < cur_dq_average)
		dq_sum = 0;
	else
		dq_sum = dq_sum - cur_dq_average;
	if (dq_sum > PHY_BDL_MAX)
		dq_sum = PHY_BDL_MAX;
	/* only DDR_MODE_WRITE need to set */
	if (ddrtrn_hal_get_cur_mode() == DDR_MODE_WRITE)
		ddrtrn_hal_dataeye_set_dq_sum(dq_sum);
	ddrtrn_hal_phy_cfg_update(ddrtrn_hal_get_cur_phy());
	return 0;
}

/* DDR write or read dataeye training */
static int ddrtrn_dataeye_process(struct ddrtrn_training_result_data *training)
{
	int result = 0;
	unsigned int i;
	/* dataeye training */
	for (i = 0; i < ddrtrn_hal_get_byte_num(); i++) {
		ddrtrn_hal_set_cur_byte(i + (ddrtrn_hal_get_dmc_id() << 1)); /* byte index accord to phy */
		result += ddrtrn_dataeye_deskew(training);
	}
	if (result) {
		result = -1;
		ddrtrn_error("PHY[%x] mode[%x] dataeye training fail", ddrtrn_hal_get_cur_phy(),
					 ddrtrn_hal_get_cur_mode());
	} else {
		/* dataeye training result adjust */
		ddrtrn_adjust_dataeye(training);
	}
	/* save training result to printf */
	ddrtrn_result_data_save(training);
	return result;
}

/* DDR dataeye training */
int ddrtrn_dataeye_training(void)
{
	struct ddrtrn_training_result_data tmp_result;
	struct ddrtrn_training_result_data *training = &tmp_result;
	int result_read, result_write;
	ddrtrn_debug("DDR dataeye training PHY[%x][%x] DMC[%x][%x] Rank[%x]",
				 ddrtrn_hal_get_phy_id(), ddrtrn_hal_get_cur_phy(),
				 ddrtrn_hal_get_dmc_id(), ddrtrn_hal_get_cur_dmc(), ddrtrn_hal_get_rank_id());
	/* write dataeye training */
	ddrtrn_hal_set_cur_mode(DDR_MODE_WRITE);
	ddrtrn_set_data(training, 0, sizeof(struct ddrtrn_training_result_data));
	result_write = ddrtrn_dataeye_process(training);
	/* read dataeye training */
	ddrtrn_hal_set_cur_mode(DDR_MODE_READ);
	ddrtrn_set_data(training, 0, sizeof(struct ddrtrn_training_result_data));
	result_read = ddrtrn_dataeye_process(training);
	if (result_read != 0 || result_write != 0)
		return -1;
	else
		return 0;
}

int ddrtrn_dataeye_training_func(void)
{
	struct ddrtrn_hal_training_relate_reg relate_reg;
	int result;
	ddrtrn_hal_set_dq_type(DDR_CHECK_TYPE_DDRT);
	/* dataeye training disable */
	if (ddrtrn_hal_check_bypass(DDR_BYPASS_DATAEYE_MASK) != DDR_FALSE)
		return 0;
	ddrtrn_hal_save_reg(&relate_reg, DDR_BYPASS_DATAEYE_MASK);
	ddrtrn_hal_training_switch_axi();
	ddrtrn_ddrt_init(DDR_DDRT_MODE_DATAEYE);
	ddrtrn_hal_set_adjust(DDR_DATAEYE_NORMAL_ADJUST);
	result = ddrtrn_dataeye_training();
	ddrtrn_hal_restore_reg(&relate_reg);
	return result;
}
#else
int ddrtrn_dataeye_training_func(void)
{
	ddrtrn_warning("Not support DDR dataeye training");
	return 0;
}
#endif /* DDR_DATAEYE_TRAINING_CONFIG */
