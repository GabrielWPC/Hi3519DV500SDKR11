/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_buf.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "motionsensor_buf.h"
#include <linux/kernel.h>
#include "mm_ext.h"
#include "motionsensor_exe.h"
#include "securec.h"


static td_bool g_forward = TD_TRUE;
static td_bool g_buf_init = TD_FALSE;
msensor_buf_info g_buf_info[MSENSOR_DATA_BUTT][DATA_BUTT];
static td_s64 g_offset;
td_bool g_already_released[MAX_USER_NUM];
static unsigned long g_msensor_buf_len = 0;

static msensor_buf_user_mng g_user_mng = {
    .mng_mutex.mutex = TD_NULL
};

#define DATA_RESERVE_NUM 50
#define ot_align(x, a) ((a) * (((x) + (a)-1) / (a)))

#define x_start_addr(data_type)     g_buf_info[data_type][DATA_X].start_addr
#define y_start_addr(data_type)     g_buf_info[data_type][DATA_Y].start_addr
#define z_start_addr(data_type)     g_buf_info[data_type][DATA_Z].start_addr
#define temp_start_addr(data_type)  g_buf_info[data_type][DATA_TEMP].start_addr
#define pts_start_addr(data_type)   g_buf_info[data_type][DATA_PTS].start_addr

#define x_write_ptr(data_type)      g_buf_info[data_type][DATA_X].write_ptr
#define y_write_ptr(data_type)      g_buf_info[data_type][DATA_Y].write_ptr
#define z_write_ptr(data_type)      g_buf_info[data_type][DATA_Z].write_ptr
#define temp_write_ptr(data_type)   g_buf_info[data_type][DATA_TEMP].write_ptr
#define pts_write_ptr(data_type)    g_buf_info[data_type][DATA_PTS].write_ptr

#define x_read_ptr(i, data_type)     g_user_mng.user_ctx[i]->read_ptr[data_type][DATA_X]
#define y_read_ptr(i, data_type)     g_user_mng.user_ctx[i]->read_ptr[data_type][DATA_Y]
#define z_read_ptr(i, data_type)     g_user_mng.user_ctx[i]->read_ptr[data_type][DATA_Z]
#define temp_read_ptr(i, data_type)  g_user_mng.user_ctx[i]->read_ptr[data_type][DATA_TEMP]
#define pts_read_ptr(i, data_type)   g_user_mng.user_ctx[i]->read_ptr[data_type][DATA_PTS]

msensor_buf_info **msensor_buf_get_info(td_void)
{
    return (msensor_buf_info**)g_buf_info;
}

osal_spinlock *msensor_buf_get_lock(td_void)
{
    return &g_user_mng.mng_lock;
}

static td_s32 msensor_buf_get_user_id(td_s32 *id)
{
    td_s32 i;

    for (i = 0; i < MAX_USER_NUM; i++) {
        if (g_user_mng.user_ctx[i] == TD_NULL) {
            *id = i;
            return TD_SUCCESS;
        }
    }

    msensor_err_trace("no free user for use.\n");
    return TD_FAILURE;
}

static td_s32 msensor_buf_add_user_create_ctx(const td_s32 *id)
{
    td_s32 i;
    td_s32 j;
    td_ulong flags;
    msensor_buf_user_context *user_ctx = TD_NULL;
    msensor_mng_proc_info *proc_info = TD_NULL;
    proc_info = msensor_mng_get_proc_info();

    user_ctx = (msensor_buf_user_context *)osal_vmalloc(sizeof(msensor_buf_user_context));
    if (user_ctx == TD_NULL) {
        msensor_err_trace("vmalloc failed.\n");
        return TD_FAILURE;
    }

    (td_void)memset_s(user_ctx, sizeof(msensor_buf_user_context), 0, sizeof(msensor_buf_user_context));

    osal_spin_lock_irqsave(&g_user_mng.read_lock[*id], &flags);

    for (i = 0; i < DATA_BUTT; i++) {
        if (i == DATA_PTS) {
            for (j = 0; j < MSENSOR_DATA_BUTT; j++) {
                user_ctx->read_ptr[j][i] =
                    (proc_info->buf_size[j] > DATA_RESERVE_NUM * BUF_BLOCK_NUM) ?
                    (td_u64 *)g_buf_info[j][i].write_ptr : g_buf_info[j][i].write_ptr;
            }
        } else {
            for (j = 0; j < MSENSOR_DATA_BUTT; j++) {
                user_ctx->read_ptr[j][i] = (int *)g_buf_info[j][i].write_ptr;
            }
        }
    }

    g_forward = TD_TRUE;
    g_user_mng.user_ctx[*id] = user_ctx;
    g_user_mng.user_cnt++;

    osal_spin_unlock_irqrestore(&g_user_mng.read_lock[*id], &flags);
    return TD_SUCCESS;
}

td_s32 msensor_buf_add_user(td_s32 *id)
{
    td_s32 ret;

    if (id == NULL) {
        msensor_err_trace("motionsensor id is NULL.\n");
        return TD_FAILURE;
    }

    if (g_buf_init == TD_FALSE) {
        msensor_err_trace("buf not init,g_buf_init:%d\n", g_buf_init);
        return TD_FAILURE;
    }

    osal_mutex_lock(&g_user_mng.mng_mutex);

    /* step 1: get available ID */
    if (g_user_mng.user_cnt > MAX_USER_NUM) {
        msensor_err_trace("motionsensor ID has reached toplimit.\n");
        goto err0;
    }

    /* get user id for use */
    ret = msensor_buf_get_user_id(id);
    if (ret != TD_SUCCESS) {
        *id = -1;
        msensor_err_trace("get_user_id failed.\n");
        goto err0;
    }

    if ((*id < 0) || (*id >= MAX_USER_NUM)) {
        msensor_err_trace("*id(%d) out of range[0,%d].\n", *id, MAX_USER_NUM);
        goto err0;
    }

    /* step 2: create context for new ID */
    ret = msensor_buf_add_user_create_ctx(id);
    if (ret != TD_SUCCESS) {
        goto err0;
    }

    osal_mutex_unlock(&g_user_mng.mng_mutex);
    osal_msleep(50); /* 50ms */
    return TD_SUCCESS;
err0:
    osal_mutex_unlock(&g_user_mng.mng_mutex);
    return TD_FAILURE;
}

td_s32 msensor_buf_delete_user(const td_s32 *id)
{
    msensor_buf_user_context *msenser_buf_user_contex_temp = TD_NULL;
    td_ulong flags;

    if (id == NULL) {
        msensor_err_trace("motionsensor id is NULL.\n");
        return TD_FAILURE;
    }

    if ((*id < 0) || (*id >= MAX_USER_NUM)) {
        msensor_err_trace("*id(%d) out of range[0,%d].\n", *id, MAX_USER_NUM);
        return TD_FAILURE;
    }

    if (g_buf_init == TD_FALSE) {
        msensor_err_trace("when delete user,motionsensor buffer hasn't been initialised.\n");
        return TD_FAILURE;
    }

    if (g_user_mng.user_ctx[*id] == TD_NULL) {
        msensor_debug_trace("id is NULL.\n");
        return TD_SUCCESS;
    }

    osal_mutex_lock(&g_user_mng.mng_mutex);
    osal_spin_lock_irqsave(&g_user_mng.mng_lock, &flags);

    /* release context */
    msenser_buf_user_contex_temp = g_user_mng.user_ctx[*id];

    g_user_mng.user_ctx[*id] = TD_NULL;
    g_user_mng.user_cnt--;

    osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
    osal_mutex_unlock(&g_user_mng.mng_mutex);

    if (msenser_buf_user_contex_temp != TD_NULL) {
        osal_vfree(msenser_buf_user_contex_temp);
        msenser_buf_user_contex_temp = TD_NULL;
    }
    return TD_SUCCESS;
}

static td_void msensor_buf_init_proc(const td_u32 *gyro_block_size,
    const td_u32 *acc_block_size, const td_u32 *mag_block_size)
{
    msensor_mng_proc_info *proc_info = TD_NULL;
    proc_info = msensor_mng_get_proc_info();

    proc_info->buf_addr[MSENSOR_DATA_GYRO] = (td_u64)(td_uintptr_t)g_buf_info[MSENSOR_DATA_GYRO][DATA_X].start_addr;
    proc_info->buf_addr[MSENSOR_DATA_ACC] =  (td_u64)(td_uintptr_t)g_buf_info[MSENSOR_DATA_ACC][DATA_X].start_addr;
    proc_info->buf_addr[MSENSOR_DATA_MAGN] = (td_u64)(td_uintptr_t)g_buf_info[MSENSOR_DATA_MAGN][DATA_X].start_addr;

    proc_info->buf_size[MSENSOR_DATA_GYRO] = *gyro_block_size * BUF_BLOCK_NUM;
    proc_info->buf_size[MSENSOR_DATA_ACC] =  *acc_block_size * BUF_BLOCK_NUM;
    proc_info->buf_size[MSENSOR_DATA_MAGN] = *mag_block_size * BUF_BLOCK_NUM;

    msensor_debug_trace("#####GYRO-au64_buf_addr:%llu ACC-au64_buf_addr:%llu MAGN-au64_buf_addr:%llu\n",
                        proc_info->buf_addr[MSENSOR_DATA_GYRO],
                        proc_info->buf_addr[MSENSOR_DATA_ACC],
                        proc_info->buf_addr[MSENSOR_DATA_MAGN]);
    return;
}

static td_void msensor_buf_allocation_init(td_u32 buflen, td_u32 gyro_odr, td_u32 accel_odr,
    td_u32 mag_odr, const td_void *vir_addr)
{
    td_s32 i;
    td_u32 gyro_block_size;
    td_u32 acc_block_size;
    td_u32 mag_block_size;
    td_u32 gyro_value;
    td_u32 accel_value;
    td_u32 mag_value;
    gyro_value = gyro_odr / OT_MSENSOR_GRADIENT;
    accel_value = accel_odr / OT_MSENSOR_GRADIENT;
    mag_value = mag_odr / OT_MSENSOR_GRADIENT;
    gyro_block_size = buflen * gyro_value /
        (gyro_value + accel_value + mag_value) / BUF_BLOCK_NUM / 32 * 32; /* align by 32 */
    acc_block_size = buflen * accel_value /
        (gyro_value + accel_value + mag_value) / BUF_BLOCK_NUM / 32 * 32; /* align by 32 */
    mag_block_size = (buflen / BUF_BLOCK_NUM - gyro_block_size - acc_block_size) / 32 * 32; /* align by 32 */

    for (i = 0; i < DATA_BUTT; i++) {
        g_buf_info[MSENSOR_DATA_GYRO][i].start_addr = (td_u8 *)vir_addr + gyro_block_size * i;
        g_buf_info[MSENSOR_DATA_GYRO][i].write_ptr = g_buf_info[MSENSOR_DATA_GYRO][i].start_addr;

        g_buf_info[MSENSOR_DATA_ACC][i].start_addr =
            (td_u8 *)vir_addr + gyro_block_size * BUF_BLOCK_NUM + acc_block_size * i;
        g_buf_info[MSENSOR_DATA_ACC][i].write_ptr = g_buf_info[MSENSOR_DATA_ACC][i].start_addr;

        g_buf_info[MSENSOR_DATA_MAGN][i].start_addr =
            (td_u8 *)vir_addr + gyro_block_size * BUF_BLOCK_NUM + acc_block_size * BUF_BLOCK_NUM + mag_block_size * i;
        g_buf_info[MSENSOR_DATA_MAGN][i].write_ptr = g_buf_info[MSENSOR_DATA_MAGN][i].start_addr;
    }

    msensor_debug_trace("**PTS-write_pointer:%p\n", g_buf_info[MSENSOR_DATA_GYRO][DATA_PTS].write_ptr);

    msensor_buf_init_proc(&gyro_block_size, &acc_block_size, &mag_block_size);
    return;
}

td_s32 msensor_buf_init(const ot_msensor_buf_attr *buf_attr, td_u32 gyro_odr, td_u32 accel_odr, td_u32 mag_odr)
{
    td_void *vir_addr = TD_NULL;
    td_s32 i;

    if (g_buf_init == TD_TRUE) {
        msensor_err_trace("buf already inited\n");
        return TD_SUCCESS;
    }

    if ((buf_attr->phys_addr == 0) || (buf_attr->buf_len == 0)) {
        msensor_err_trace("buf addr can not be null and buf size must lager than 0\n");
        return TD_FAILURE;
    }

    if ((gyro_odr == 0) && (accel_odr == 0) && (mag_odr == 0)) {
        msensor_err_trace("can't all frequency be 0\n");
        return TD_FAILURE;
    }

    vir_addr = cmpi_remap_cached((td_u64)buf_attr->phys_addr, ot_align(buf_attr->buf_len, 4));
    if (vir_addr == TD_NULL) {
        msensor_err_trace("ioremap err\n");
        return TD_FAILURE;
    }

    g_msensor_buf_len = ot_align(buf_attr->buf_len, 4);
    (td_void)memset_s(vir_addr, buf_attr->buf_len, 0, buf_attr->buf_len);

    msensor_debug_trace("phy_addr:%llx vir_addr:%p buflen:%d\n", buf_attr->phys_addr, vir_addr, buf_attr->buf_len);

    g_offset = buf_attr->phys_addr - (td_u64)(td_uintptr_t)vir_addr;

    msensor_buf_allocation_init(buf_attr->buf_len, gyro_odr, accel_odr, mag_odr, vir_addr);

    for (i = 0; i < MAX_USER_NUM; i++) {
        g_already_released[i] = TD_TRUE;
    }
    g_buf_init = TD_TRUE;
    return TD_SUCCESS;
}

td_s32 msensor_buf_deinit(td_void)
{
    td_s32 i;
    td_s32 ret;

    if (g_buf_init == TD_FALSE) {
        msensor_err_trace("buf already deinited\n");
        return TD_SUCCESS;
    }

    for (i = 0; i < MAX_USER_NUM; i++) {
        ret = msensor_buf_delete_user(&i);
        if (ret != TD_SUCCESS) {
            msensor_err_trace("motionsensor delete user failed\n");
        }
    }

    if (g_buf_info[MSENSOR_DATA_GYRO][DATA_X].start_addr != TD_NULL) {
        cmpi_unmap(g_buf_info[MSENSOR_DATA_GYRO][DATA_X].start_addr);
    }

    g_msensor_buf_len = 0;

    (td_void)memset_s(&g_buf_info, sizeof(g_buf_info), 0, sizeof(g_buf_info));
    g_buf_init = TD_FALSE;
    return ret;
}

static td_void msensor_buf_write(ot_msensor_data_type data_type, const ot_msensor_sample_data *sample_data)
{
    td_void *next_write_pointer = TD_NULL;

    /* step 2:write data */
    *(td_s32 *)x_write_ptr(data_type) = sample_data->x;
    *(td_s32 *)y_write_ptr(data_type) = sample_data->y;
    *(td_s32 *)z_write_ptr(data_type) = sample_data->z;
    *(td_s32 *)temp_write_ptr(data_type) = sample_data->temperature;
    *(td_u64 *)pts_write_ptr(data_type) = sample_data->pts;

    /* step 3:calculate next write pointer */
    next_write_pointer = (td_u8 *)x_write_ptr(data_type) + sizeof(sample_data->x);

    if (next_write_pointer >= y_start_addr(data_type)) {
        x_write_ptr(data_type) = x_start_addr(data_type);
        y_write_ptr(data_type) = y_start_addr(data_type);
        z_write_ptr(data_type) = z_start_addr(data_type);
        temp_write_ptr(data_type) = temp_start_addr(data_type);
        pts_write_ptr(data_type) = pts_start_addr(data_type);
    } else {
        x_write_ptr(data_type) = (td_u8 *)x_write_ptr(data_type) + sizeof(sample_data->x);
        y_write_ptr(data_type) = (td_u8 *)y_write_ptr(data_type) + sizeof(sample_data->y);
        z_write_ptr(data_type) = (td_u8 *)z_write_ptr(data_type) + sizeof(sample_data->z);
        temp_write_ptr(data_type) = (td_u8 *)temp_write_ptr(data_type) + sizeof(sample_data->temperature);
        pts_write_ptr(data_type) = (td_u8 *)pts_write_ptr(data_type) + sizeof(sample_data->pts);
    }
    return;
}

td_s32 msensor_buf_write_data(ot_msensor_data_type data_type, const ot_msensor_sample_data *sample_data)
{
    td_s32 i;
    td_void *tmp_ptr = TD_NULL;
    td_bool condition;
    msensor_mng_proc_info *proc_info = TD_NULL;
    td_ulong flags;

    proc_info = msensor_mng_get_proc_info();

    if (g_buf_init == TD_FALSE) {
        msensor_err_trace("when write_data(pts:%lld),motionsensor buf not be inited yet\n", sample_data->pts);
        return TD_FAILURE;
    }

    /* step 1:judge if any user overflow */
    if ((td_u8 *)y_start_addr(data_type) - (td_u8 *)x_write_ptr(data_type) >
        (td_s32)(sizeof(sample_data->x) * WR_GAP)) {
        tmp_ptr = (td_u8 *)x_write_ptr(data_type) + sizeof(sample_data->x) * WR_GAP;
    } else {
        tmp_ptr = (td_u8 *)x_start_addr(data_type) + sizeof(sample_data->x) * WR_GAP -
            ((td_u8 *)y_start_addr(data_type) - (td_u8 *)x_write_ptr(data_type));
    }

    for (i = 0; i < MAX_USER_NUM; i++) {
        if (g_user_mng.user_ctx[i] != TD_NULL) {
            if (((tmp_ptr == y_start_addr(data_type)) && (x_read_ptr(i, data_type) == x_start_addr(data_type))) ||
                (tmp_ptr == x_read_ptr(i, data_type))) {
                proc_info->buf_overflow[data_type]++;
                proc_info->buf_overflow_id[data_type] = i;

                osal_spin_lock_irqsave(&g_user_mng.read_lock[i], &flags);
                condition = ((td_u32 *)x_read_ptr(i, data_type) + 1) >= (td_u32 *)y_start_addr(data_type);
                x_read_ptr(i, data_type) =
                    condition ? x_start_addr(data_type) : (td_u32 *)x_read_ptr(i, data_type) + 1;
                y_read_ptr(i, data_type) =
                    condition ? y_start_addr(data_type) : (td_u32 *)y_read_ptr(i, data_type) + 1;
                z_read_ptr(i, data_type) =
                    condition ? z_start_addr(data_type) : (td_u32 *)z_read_ptr(i, data_type) + 1;
                temp_read_ptr(i, data_type) =
                    condition ? temp_start_addr(data_type) : (td_u32 *)temp_read_ptr(i, data_type) + 1;
                pts_read_ptr(i, data_type) =
                    condition ? pts_start_addr(data_type) : (td_u64 *)pts_read_ptr(i, data_type) + 1;
                osal_spin_unlock_irqrestore(&g_user_mng.read_lock[i], &flags);
            }
        }
    }

    /* write data */
    msensor_buf_write(data_type, sample_data);
    return TD_SUCCESS;
}

static td_void msensor_buf_release(ot_msensor_data_info *data_info)
{
    td_s32 id = data_info->id;
    ot_msensor_data_type data_type = data_info->data_type;
    td_ulong flags;

    osal_spin_lock_irqsave(&g_user_mng.read_lock[id], &flags);
    if (data_info->data[1].num == 0) {
        if (data_info->data[0].num > 0) {
            if (((td_u64)(td_uintptr_t)data_info->data[0].x_phys_addr +
                 data_info->data[0].num * sizeof(td_s32) - g_offset) >=
                 ((td_u64)(td_uintptr_t)y_start_addr(data_type))) {
                x_read_ptr(id, data_type) = x_start_addr(data_type);
                y_read_ptr(id, data_type) = y_start_addr(data_type);
                z_read_ptr(id, data_type) = z_start_addr(data_type);
                temp_read_ptr(id, data_type) = temp_start_addr(data_type);
                pts_read_ptr(id, data_type) = pts_start_addr(data_type);
            } else {
                x_read_ptr(id, data_type) =
                    (td_u8 *)data_info->data[0].x_phys_addr + data_info->data[0].num * sizeof(td_s32) - g_offset;
                y_read_ptr(id, data_type) =
                    (td_u8 *)data_info->data[0].y_phys_addr + data_info->data[0].num * sizeof(td_s32) - g_offset;
                z_read_ptr(id, data_type) =
                    (td_u8 *)data_info->data[0].z_phys_addr + data_info->data[0].num * sizeof(td_s32) - g_offset;
                temp_read_ptr(id, data_type) =
                    (td_u8 *)data_info->data[0].temperature_phys_addr + data_info->data[0].num * sizeof(td_s32) -
                    g_offset;
                pts_read_ptr(id, data_type) =
                    (td_u8 *)data_info->data[0].pts_phys_addr + data_info->data[0].num * sizeof(td_u64) - g_offset;
            }
        }
    } else {
        x_read_ptr(id, data_type) =
            (td_u8 *)data_info->data[1].x_phys_addr + data_info->data[1].num * sizeof(td_s32) - g_offset;
        y_read_ptr(id, data_type) =
            (td_u8 *)data_info->data[1].y_phys_addr + data_info->data[1].num * sizeof(td_s32) - g_offset;
        z_read_ptr(id, data_type) =
            (td_u8 *)data_info->data[1].z_phys_addr + data_info->data[1].num * sizeof(td_s32) - g_offset;
        temp_read_ptr(id, data_type) =
            (td_u8 *)data_info->data[1].temperature_phys_addr + data_info->data[1].num * sizeof(td_s32) - g_offset;
        pts_read_ptr(id, data_type) =
            (td_u8 *)data_info->data[1].pts_phys_addr + data_info->data[1].num * sizeof(td_u64) - g_offset;
    }
    data_info->data[0].num = 0;
    data_info->data[1].num = 0;
    g_already_released[id] = TD_TRUE;

    osal_spin_unlock_irqrestore(&g_user_mng.read_lock[id], &flags);
    return;
}

td_s32 msensor_buf_release_data(ot_msensor_data_info *data_info)
{
    td_ulong flags;

    if (data_info == TD_NULL) {
        msensor_err_trace("data info is NULL\n");
        return TD_FAILURE;
    }

    osal_spin_lock_irqsave(&g_user_mng.mng_lock, &flags);

    if ((data_info->id < 0) || (data_info->id >= MAX_USER_NUM)) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        msensor_err_trace("*id(%d) is out of range[0,%d].\n", data_info->id, MAX_USER_NUM);
        return TD_FAILURE;
    }

    if (g_buf_init == TD_FALSE) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        msensor_err_trace("when release_data, motionsensor buf not be inited yet\n");
        return TD_FAILURE;
    }

    if (g_user_mng.user_ctx[data_info->id] == TD_NULL) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        msensor_err_trace("motionsensor buf release: id is NULL.\n");
        return TD_FAILURE;
    }

    if (data_info->data_type >= MSENSOR_DATA_BUTT) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        msensor_err_trace("data_type:%d is out of range.\n", data_info->data_type);
        return TD_FAILURE;
    }
#ifndef MSENSOR_SUPPORT_MAGN
    if (data_info->data_type == MSENSOR_DATA_MAGN) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        msensor_err_trace("data_type:%d not support now.\n", data_info->data_type);
        return TD_FAILURE;
    }
#endif
    if (g_already_released[data_info->id] == TD_TRUE) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        return TD_SUCCESS;
    }

    /* release data */
    msensor_buf_release(data_info);
    osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
    return TD_SUCCESS;
}

static td_s32 msensor_buf_arrive_back_start_addr(const ot_msensor_data_info *data_info)
{
    td_s32 length[2] = { 0 }; /* 2 : cycle buf */
    td_s32 buftotal_len;
    td_bool loop_flag = TD_FALSE;
    td_u32 interval;
    td_u64 *pts_read_ptr = TD_NULL;
    ot_msensor_data_type data_type = data_info->data_type;
    td_s32 id = data_info->id;

    buftotal_len = (td_u32 *)y_start_addr(data_type) - (td_u32 *)x_start_addr(data_type);

    pts_read_ptr = pts_read_ptr(id, data_type);

    while (1) {
        if ((*pts_read_ptr <= data_info->begin_pts) || (pts_read_ptr == pts_write_ptr(data_type))) {
            if (pts_read_ptr == pts_write_ptr(data_type)) {
                msensor_err_trace("arrive to write_addr!\n");
            }
            break;
        }

        if (pts_read_ptr == pts_start_addr(data_type)) {
            loop_flag = TD_TRUE;
            pts_read_ptr = (td_u64 *)pts_start_addr(data_type) + buftotal_len - 1;
        } else {
            pts_read_ptr--;
        }

        g_forward = TD_FALSE;

        /* msensor_debug_trace("#####pts_read_pointer:%p %lld\n", pts_read_ptr, *pts_read_ptr) */
        if (loop_flag == TD_TRUE) {
            length[1]++;
        } else {
            length[0]++;
        }
    }

    if (length[1] > 0) {
        interval = (td_u64 *)pts_start_addr(data_type) + buftotal_len - pts_read_ptr;
        x_read_ptr(id, data_type) = (td_u32 *)y_start_addr(data_type) - interval;
        y_read_ptr(id, data_type) = (td_u32 *)z_start_addr(data_type) - interval;
        z_read_ptr(id, data_type) = (td_u32 *)temp_start_addr(data_type) - interval;
        temp_read_ptr(id, data_type) = (td_u32 *)pts_start_addr(data_type) - interval;
        pts_read_ptr(id, data_type) = pts_read_ptr;
    } else {
        interval = (td_u64 *)pts_read_ptr(id, data_type) - pts_read_ptr;
        x_read_ptr(id, data_type) = (td_u32 *)x_read_ptr(id, data_type) - interval;
        y_read_ptr(id, data_type) = (td_u32 *)y_read_ptr(id, data_type) - interval;
        z_read_ptr(id, data_type) = (td_u32 *)z_read_ptr(id, data_type) - interval;
        temp_read_ptr(id, data_type) = (td_u32 *)temp_read_ptr(id, data_type) - interval;
        pts_read_ptr(id, data_type) = pts_read_ptr;
    }
    return TD_SUCCESS;
}

static td_void msensor_buf_get_data_assign(ot_msensor_data_info *data_info)
{
    td_s32 id = data_info->id;
    ot_msensor_data_type data_type = data_info->data_type;
    td_u32 interval;
    msensor_mng_proc_info *proc_info = TD_NULL;
    proc_info = msensor_mng_get_proc_info();

    if (data_info->data[0].num > 0) {
        data_info->data[0].x_phys_addr = (td_void *)((td_u8 *)x_read_ptr(id, data_type) + g_offset);
        data_info->data[0].y_phys_addr = (td_void *)((td_u8 *)y_read_ptr(id, data_type) + g_offset);
        data_info->data[0].z_phys_addr = (td_void *)((td_u8 *)z_read_ptr(id, data_type) + g_offset);
        data_info->data[0].temperature_phys_addr = (td_void *)((td_u8 *)temp_read_ptr(id, data_type) + g_offset);
        data_info->data[0].pts_phys_addr = (td_void *)((td_u8 *)pts_read_ptr(id, data_type) + g_offset);
    }

    if (data_info->data[1].num > 0) {
        data_info->data[1].x_phys_addr = (td_void *)((td_u8 *)x_start_addr(data_type) + g_offset);
        data_info->data[1].y_phys_addr = (td_void *)((td_u8 *)y_start_addr(data_type) + g_offset);
        data_info->data[1].z_phys_addr = (td_void *)((td_u8 *)z_start_addr(data_type) + g_offset);
        data_info->data[1].temperature_phys_addr = (td_void *)((td_u8 *)temp_start_addr(data_type) + g_offset);
        data_info->data[1].pts_phys_addr = (td_void *)((td_u8 *)pts_start_addr(data_type) + g_offset);
    }

    if ((data_info->data[0].num == 0) && (data_info->data[1].num == 0)) {
        msensor_err_trace("no data! bg_pts:%12lld ed_pts:%12lld\n", data_info->begin_pts, data_info->end_pts);
        g_already_released[id] = TD_TRUE;
        proc_info->buf_data_unmatch[data_type]++;
        proc_info->buf_data_unmatch_id[data_type] = id;
        interval =
            (td_u32)(td_uintptr_t)((td_u8 *)pts_read_ptr(id, data_type) - (td_u8 *)pts_start_addr(data_type)) >> 1;
        x_read_ptr(id, data_type) = (td_u8 *)x_start_addr(data_type) + interval;
        y_read_ptr(id, data_type) = (td_u8 *)y_start_addr(data_type) + interval;
        z_read_ptr(id, data_type) = (td_u8 *)z_start_addr(data_type) + interval;
        temp_read_ptr(id, data_type) = (td_u8 *)temp_start_addr(data_type) + interval;
        pts_read_ptr(id, data_type) = pts_read_ptr(id, data_type);
    } else {
        g_already_released[id] = TD_FALSE;
    }
    data_info->addr_offset = g_offset;
    return;
}

static td_void msensor_buf_get_data_process(ot_msensor_data_info *data_info)
{
    td_bool loop_flag = TD_FALSE;
    td_bool first_found = TD_FALSE;
    td_u32 interval;
    td_u64 *pts_read_ptr = TD_NULL;
    td_s32 id = data_info->id;
    ot_msensor_data_type data_type = data_info->data_type;

    pts_read_ptr = pts_read_ptr(id, data_type);

    while (1) {
        if (((pts_read_ptr == pts_write_ptr(data_type)) && (g_forward == TD_TRUE)) ||
            (*pts_read_ptr > data_info->end_pts)) {
            break;
        }

        if ((*pts_read_ptr >= data_info->begin_pts) && (*pts_read_ptr <= data_info->end_pts)) {
            if (loop_flag == TD_TRUE) {
                data_info->data[1].num++;
            } else if (first_found == TD_FALSE) {
                data_info->data[0].num++;

                interval = (td_u32)(td_uintptr_t)((td_u8 *)pts_read_ptr - (td_u8 *)pts_start_addr(data_type)) >> 1;
                x_read_ptr(id, data_type) = (td_u8 *)x_start_addr(data_type) + interval;
                y_read_ptr(id, data_type) = (td_u8 *)y_start_addr(data_type) + interval;
                z_read_ptr(id, data_type) = (td_u8 *)z_start_addr(data_type) + interval;
                temp_read_ptr(id, data_type) = (td_u8 *)temp_start_addr(data_type) + interval;
                pts_read_ptr(id, data_type) = pts_read_ptr;
                first_found = TD_TRUE;
            } else {
                data_info->data[0].num++;
            }
        }

        pts_read_ptr++;
        g_forward = TD_TRUE;

        if (pts_read_ptr >= ((td_u64 *)pts_start_addr(data_type) + ((td_u8 *)y_start_addr(data_type) -
            (td_u8 *)x_start_addr(data_type)) / sizeof(td_s32))) {
            pts_read_ptr = pts_start_addr(data_type);

            if (first_found == TD_TRUE) {
                loop_flag = TD_TRUE;
            }
        }
    }
    return;
}

static td_s32 msensor_buf_data_check(const ot_msensor_data_info *msensor_data)
{
    if (g_buf_init == TD_FALSE) {
        msensor_err_trace("get data,but motionsensor buffer hasn't been initialised.\n");
        return TD_FAILURE;
    }

    if (msensor_data == TD_NULL) {
        msensor_err_trace("motionsensor_data(%p) is NULL!!!\n", msensor_data);
        return TD_FAILURE;
    }

    if ((msensor_data->id < 0) || (msensor_data->id >= MAX_USER_NUM)) {
        msensor_err_trace("id(%d) is out of range[0,%d].\n", msensor_data->id, MAX_USER_NUM);
        return TD_FAILURE;
    }

    if (msensor_data->data_type >= MSENSOR_DATA_BUTT) {
        msensor_err_trace("data_type:%d out of range.\n", msensor_data->data_type);
        return TD_FAILURE;
    }
#ifndef MSENSOR_SUPPORT_MAGN
    if (msensor_data->data_type == MSENSOR_DATA_MAGN) {
        msensor_err_trace("data_type:%d not support now.\n", msensor_data->data_type);
        return TD_FAILURE;
    }
#endif
    /* Check whether user is created. */
    if (g_user_mng.user_ctx[msensor_data->id] == TD_NULL) {
        msensor_err_trace("motionsensor_buf_read_data: id:%d is NULL.\n", msensor_data->id);
        return TD_FAILURE;
    }

    if (g_already_released[msensor_data->id] == TD_FALSE) {
        msensor_err_trace("please release last read\n");
        return TD_FAILURE;
    }

    if ((msensor_data->end_pts - msensor_data->begin_pts) > 5000000) { /* 5000000us */
        msensor_err_trace("end pts[%llu] - begin pts[%llu] = %llu, more than 5000000\n", msensor_data->end_pts,
            msensor_data->begin_pts, msensor_data->end_pts - msensor_data->begin_pts);
        return TD_FAILURE;
    }

    if (msensor_data->end_pts <= msensor_data->begin_pts) {
        msensor_err_trace("end_pts[%llu] <= begin_pts[%llu]\n", msensor_data->end_pts, msensor_data->begin_pts);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 msensor_buf_get_data(ot_msensor_data_info *msensor_data)
{
    td_s32 ret;
    td_ulong flags;
    td_ulong flags_read;
    ot_msensor_data_info data_info = { 0 };

    ret = msensor_buf_data_check(msensor_data);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* remain check user id to be done */
    ret = memcpy_s(&data_info, sizeof(ot_msensor_data_info), msensor_data, sizeof(ot_msensor_data_info));
    if (ret != EOK) {
        return ret;
    }

    if (msensor_data->data_type == MSENSOR_DATA_GYRO) {
        msensor_mng_write_data_2_buf();
    }

    osal_spin_lock_irqsave(&g_user_mng.mng_lock, &flags);

    ret = msensor_buf_data_check(msensor_data);
    if (ret != TD_SUCCESS) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        return ret;
    }

    ret = msensor_buf_arrive_back_start_addr(&data_info);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("data_type:%d has no data.\n", msensor_data->data_type);
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        return TD_FAILURE;
    }

    data_info.data[0].num = 0;
    data_info.data[1].num = 0;

    osal_spin_lock_irqsave(&g_user_mng.read_lock[msensor_data->id], &flags_read);
    msensor_buf_get_data_process(&data_info);
    msensor_buf_get_data_assign(&data_info);
    osal_spin_unlock_irqrestore(&g_user_mng.read_lock[msensor_data->id], &flags_read);

    ret = memcpy_s(msensor_data, sizeof(ot_msensor_data_info), &data_info, sizeof(ot_msensor_data_info));
    if (ret != EOK) {
        osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);
        return ret;
    }

    osal_spin_unlock_irqrestore(&g_user_mng.mng_lock, &flags);

    ret = msensor_buf_release_data(&data_info);

    return ret;
}

static td_s32 msensor_buf_sync_init(td_void)
{
    td_s32 ret;
    td_s32 i;

    for (i = 0; i < MAX_USER_NUM; i++) {
        ret = osal_spin_lock_init(&g_user_mng.read_lock[i]);
        if (ret != TD_SUCCESS) {
            msensor_err_trace("spin_lock_init failed!!!!\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_void msensor_buf_sync_deinit(td_void)
{
    td_s32 i;

    for (i = 0; i < MAX_USER_NUM; i++) {
        osal_spin_lock_destroy(&g_user_mng.read_lock[i]);
    }
    return;
}

td_s32 msensor_buf_lock_init(td_void)
{
    td_s32 ret;

    (td_void)memset_s(&g_user_mng, sizeof(msensor_buf_user_mng), 0, sizeof(msensor_buf_user_mng));

    ret = osal_spin_lock_init(&g_user_mng.mng_lock);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("spin_lock_init failed!!!!\n");
        return TD_FAILURE;
    }

    ret = msensor_buf_sync_init();
    if (ret != TD_SUCCESS) {
        msensor_err_trace("motionsensor buf sync init failed!!!!\n");
        return TD_FAILURE;
    }

    osal_mutex_init(&g_user_mng.mng_mutex);
    return TD_SUCCESS;
}

td_void msensor_buf_lock_deinit(td_void)
{
    osal_mutex_destroy(&g_user_mng.mng_mutex);

    msensor_buf_sync_deinit();

    osal_spin_lock_destroy(&g_user_mng.mng_lock);
    return;
}
