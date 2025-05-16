/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_api.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"

int bsp_ddrtrn_resume(void)
{
	return ddrtrn_resume();
}

int bsp_ddrtrn_suspend(void)
{
	ddrtrn_suspend_store_para();
	return 0;
}