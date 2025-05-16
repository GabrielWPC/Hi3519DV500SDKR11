/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_buf.h
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#ifndef MOTIONSENSOR_BUF_H
#define MOTIONSENSOR_BUF_H

#include "motionsensor_ext.h"
#include "ot_osal.h"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#define BUF_BLOCK_NUM 6
#define MAX_USER_NUM  10

/*
 * min motionsensor gap between reader pointer and write pointer,
 * to prevent new data overlap with reading/processing data
 */
#define WR_GAP        100

typedef struct {
    td_void *start_addr; /* start address */
    td_void *write_ptr;  /* write pointer */
} msensor_buf_info;

typedef enum {
    DATA_X,
    DATA_Y,
    DATA_Z,
    DATA_TEMP,
    DATA_PTS,
    DATA_BUTT
} msensor_buf_data_type;

typedef struct {
    td_void *read_ptr[MSENSOR_DATA_BUTT][DATA_BUTT];
    td_s32 reverd3[4]; /* array 4 */
} msensor_buf_user_context;

typedef struct {
    td_u32 user_cnt;
    osal_spinlock mng_lock;
    osal_spinlock read_lock[MAX_USER_NUM];
    msensor_buf_user_context *user_ctx[MAX_USER_NUM];
    osal_mutex mng_mutex;
} msensor_buf_user_mng;

msensor_buf_info **msensor_buf_get_info(td_void);
osal_spinlock *msensor_buf_get_lock(td_void);

td_s32 msensor_buf_lock_init(td_void);
td_void msensor_buf_lock_deinit(td_void);
td_s32 msensor_buf_init(const ot_msensor_buf_attr *buf_attr, td_u32 gyro_freq, td_u32 accel_freq, td_u32 mag_freq);
td_s32 msensor_buf_deinit(td_void);
td_s32 msensor_buf_write_data(ot_msensor_data_type data_type, const ot_msensor_sample_data *sample_data);
td_s32 msensor_buf_get_data(ot_msensor_data_info *msensor_data);
td_s32 msensor_buf_release_data(ot_msensor_data_info *msensor_data_info);
td_s32 msensor_buf_add_user(td_s32 *id);
td_s32 msensor_buf_delete_user(const td_s32 *id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
