/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_exec_time.h"
#include "ddrtrn_training.h"

#ifdef DDR_TRAINING_EXEC_TIME
/*
 * Start timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_start(void)
{
	/* timer start */
	ddrtrn_reg_write(0, REG_BASE_TIMER01 + REG_TIMER_LOAD); /* REG_BASE_TIMER01 + REG_TIMER_RELOAD */
	/* TIMER_EN  | TIMER_MODE |TIMER_PRE | TIMER_SIZE, REG_TIMER_CONTROL */
	ddrtrn_reg_write(TIMER_EN_VAL | TIMER_MODE_VAL | TIMER_PRE_VAL | TIMER_SIZE_VAL,
					 REG_BASE_TIMER01 + REG_TIMER_CONTROL);
	ddrtrn_reg_write(INITIAL_COUNT_VAL, REG_BASE_TIMER01 + REG_TIMER_LOAD);
}

/*
 * Stop timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_stop(void)
{
	/* timer stop */
	ddrtrn_reg_write(0, REG_BASE_TIMER01 + REG_TIMER_CONTROL); /* REG_TIMER_CONTROL */
	/* REG_TIMER_VALUE, 24MHz */
	ddrtrn_warning("DDR training execute time: [%d]us\n",
				   (INITIAL_COUNT_VAL - ddrtrn_reg_read(REG_BASE_TIMER01 + REG_TIMER_VALUE)) / CLOCK_FREQ); /* 24MHz */
}
#else
void ddrtrn_hal_chip_timer_start(void)
{
}

/*
 * Stop timer for calculate DDR training execute time.
 * NOTE: Just only for debug.
 */
void ddrtrn_hal_chip_timer_stop(void)
{
}
#endif
