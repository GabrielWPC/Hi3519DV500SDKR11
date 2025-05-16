/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_exe.h
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#ifndef MOTIONSENSOR_EXE_H
#define MOTIONSENSOR_EXE_H

#include "ot_type.h"
#include "motionsensor_ext.h"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

/* name MAG INTERFACE */
#define C_BMI160_BYTE_COUNT                 2
#define BMI160_SLEEP_STATE                  0x00
#define BMI160_WAKEUP_INTR                  0x00
#define BMI160_SLEEP_TRIGGER                0x04
#define BMI160_WAKEUP_TRIGGER               0x02
#define BMI160_ENABLE_FIFO_WM               0x02
#define BMI160_MAG_INTERFACE_OFF_PRIMARY_ON 0x00
#define BMI160_MAG_INTERFACE_ON_PRIMARY_ON  0x02

#define BMI160_MODE_SWITCHING_DELAY         30

#define msensor_return_if_null_ptr(ptr) do { \
    if ((ptr) == TD_NULL) { \
        printk("input null ptr\n"); \
        return TD_FAILURE; \
    } \
} while (0)

typedef struct {
    td_u32 cmd;
    td_s32 (*func)(td_uintptr_t arg);
} msensor_mng_ioctl_info;

/* for motionsensor_mng proc */
#define MAX_LEN 10
typedef struct {
    td_char gyro_name[MAX_LEN];
    td_char accel_name[MAX_LEN];
    td_char mag_name[MAX_LEN];

    td_u64 buf_addr[MSENSOR_DATA_BUTT];
    td_u32 buf_size[MSENSOR_DATA_BUTT];
    td_u32 buf_overflow[MSENSOR_DATA_BUTT];
    td_u32 buf_data_unmatch[MSENSOR_DATA_BUTT];
    td_s32 buf_overflow_id[MSENSOR_DATA_BUTT];
    td_s32 buf_data_unmatch_id[MSENSOR_DATA_BUTT];
} msensor_mng_proc_info;
msensor_mng_proc_info *msensor_mng_get_proc_info(td_void);

td_s32 msensor_mng_proc_info_init(td_void);
td_s32 msensor_mng_write_data_2_buf(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
