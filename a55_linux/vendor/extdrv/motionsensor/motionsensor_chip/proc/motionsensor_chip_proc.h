/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_chip_proc.h
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#ifndef MOTIONSENSOR_CHIP_PROC_H
#define MOTIONSENSOR_CHIP_PROC_H

#include "motionsensor_ext.h"
#include "ot_common_motionsensor.h"
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_LEN 32

td_s32 mpu_proc_init(td_void);
void mpu_proc_exit(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
