/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_exe.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "motionsensor_exe.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "sys_ext.h"
#include "ot_osal.h"
#include "ot_common.h"
#include "ot_motionsensor_mng_cmd.h"
#include "motionsensor_ext.h"
#include "motionsensor_buf.h"
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
#include "motionsensor_mng_proc.h"
#endif
#include "securec.h"

static osal_dev *g_msensor_mng_dev = TD_NULL;
static msensor_mng_callback g_motionsensor_mng_callback = { TD_NULL };
msensor_mng_proc_info g_mng_proc_info;

#define USER_SEND_DATA 1

#define MOTIONSENSOR_DEV_NAME "motionsensor_mng"

msensor_mng_proc_info *msensor_mng_get_proc_info(td_void)
{
    return &g_mng_proc_info;
}

td_s32 msensor_mng_proc_info_init(td_void)
{
    errno_t ret;
    msensor_mng_proc_info *proc_info = TD_NULL;
    proc_info = msensor_mng_get_proc_info();

    ret = memcpy_s(proc_info->gyro_name, sizeof(proc_info->gyro_name), "ICM20690", sizeof("ICM20690"));
    if (ret != EOK) {
        return TD_FAILURE;
    }

    ret = memcpy_s(proc_info->accel_name, sizeof(proc_info->accel_name), "ICM20690", sizeof("ICM20690"));
    if (ret != EOK) {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 msensor_mng_write_data_to_buf(ot_msensor_data *msensor_data)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    osal_spinlock *mng_lock = msensor_buf_get_lock();
    td_ulong flags;

    if (msensor_data == TD_NULL) {
        msensor_err_trace("motionsensor data is NULL\n");
        return TD_FAILURE;
    }

    if ((msensor_data->gyro_buf.data_num > OT_MSENSOR_MAX_DATA_NUM) ||
        (msensor_data->acc_buf.data_num > OT_MSENSOR_MAX_DATA_NUM) ||
        (msensor_data->magn_buf.data_num > OT_MSENSOR_MAX_DATA_NUM)) {
        return TD_FAILURE;
    }

    osal_spin_lock_irqsave(mng_lock, &flags);

    if ((msensor_data->attr.device_mask & OT_MSENSOR_DEVICE_GYRO) == OT_MSENSOR_DEVICE_GYRO) {
        for (i = 0; i < msensor_data->gyro_buf.data_num; i++) {
            if ((msensor_data->attr.temperature_mask & OT_MSENSOR_TEMP_GYRO) != OT_MSENSOR_TEMP_GYRO) {
                msensor_data->gyro_buf.gyro_data[i].temperature = 0xffffffff;
            }

            msensor_info_trace("gyro:x:%8d y:%8d z:%8d temperature:%8d pts:%10lld!\n",
                msensor_data->gyro_buf.gyro_data[i].x, msensor_data->gyro_buf.gyro_data[i].y,
                msensor_data->gyro_buf.gyro_data[i].z, msensor_data->gyro_buf.gyro_data[i].temperature,
                msensor_data->gyro_buf.gyro_data[i].pts);

            ret = msensor_buf_write_data(MSENSOR_DATA_GYRO, &(msensor_data->gyro_buf.gyro_data[i]));
        }
    }

    if ((msensor_data->attr.device_mask & OT_MSENSOR_DEVICE_ACC) == OT_MSENSOR_DEVICE_ACC) {
        for (i = 0; i < msensor_data->acc_buf.data_num; i++) {
            if ((msensor_data->attr.temperature_mask & OT_MSENSOR_TEMP_ACC) != OT_MSENSOR_TEMP_ACC) {
                msensor_data->acc_buf.acc_data[i].temperature = 0xffffffff;
            }
            ret = msensor_buf_write_data(MSENSOR_DATA_ACC, &(msensor_data->acc_buf.acc_data[i]));
        }
    }

    if ((msensor_data->attr.device_mask & OT_MSENSOR_DEVICE_MAGN) == OT_MSENSOR_DEVICE_MAGN) {
#ifdef MSENSOR_SUPPORT_MAGN
        for (i = 0; i < msensor_data->magn_buf.data_num; i++) {
            if ((msensor_data->attr.temperature_mask & OT_MSENSOR_TEMP_MAGN) != OT_MSENSOR_TEMP_MAGN) {
                msensor_data->magn_buf.magn_data[i].temperature = 0xffffffff;
            }
            ret = msensor_buf_write_data(MSENSOR_DATA_MAGN, &(msensor_data->magn_buf.magn_data[i]));
        }
#else
        msensor_err_trace("msensor device_mask not support magn now!\n");
#endif
    }

    osal_spin_unlock_irqrestore(mng_lock, &flags);
    return ret;
}

static td_s32 msensor_buf_check_phys_addr(ot_msensor_data_info *data_info)
{
    td_s32 ret = TD_SUCCESS;
    if (data_info->data[0].num > 0) {
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[0].x_phys_addr,
            sizeof(td_s32) * data_info->data[0].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[0].y_phys_addr,
            sizeof(td_s32) * data_info->data[0].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[0].z_phys_addr,
            sizeof(td_s32) * data_info->data[0].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[0].temperature_phys_addr,
            sizeof(td_s32) * data_info->data[0].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[0].pts_phys_addr,
            sizeof(td_u64) * data_info->data[0].num);
    }

    if (data_info->data[1].num > 0) {
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[1].x_phys_addr,
            sizeof(td_s32) * data_info->data[1].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[1].y_phys_addr,
            sizeof(td_s32) * data_info->data[1].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[1].z_phys_addr,
            sizeof(td_s32) * data_info->data[1].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[1].temperature_phys_addr,
            sizeof(td_s32) * data_info->data[1].num);
        ret |= ot_mmz_check_phys_addr((td_u64)(td_uintptr_t)data_info->data[1].pts_phys_addr,
            sizeof(td_u64) * data_info->data[1].num);
    }
    return ret;
}


/* motionsensor mng ioctl functions */
static td_s32 msensor_mng_drv_user_get_data(td_uintptr_t arg)
{
    td_s32 ret;

    ret = msensor_buf_get_data((ot_msensor_data_info *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("ot_motionsensor_buf_read_data failed! ret=%x\n", ret);
        return TD_FAILURE;
    }

    ret = msensor_buf_check_phys_addr((ot_msensor_data_info *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("check msensor_data_addr share failed\n");
        return TD_FAILURE;
    }
    return ret;
}

static td_s32 msensor_mng_drv_user_release_buf(td_uintptr_t arg)
{
    td_s32 ret;

    ret = msensor_buf_check_phys_addr((ot_msensor_data_info *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("check msensor_data_addr share failed\n");
        return TD_FAILURE;
    }

    ret = msensor_buf_release_data((ot_msensor_data_info *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("ot_motionsensor_buf_release failed! ret=%x\n", ret);
        return TD_FAILURE;
    }
    return ret;
}

static td_s32 msensor_mng_drv_user_add_user(td_uintptr_t arg)
{
    td_s32 ret;

    ret = msensor_buf_add_user((td_s32 *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("motionsensor buf add_user failed! ret=%x\n", ret);
        return TD_FAILURE;
    }
    return ret;
}

static td_s32 msensor_mng_drv_user_delete_user(td_uintptr_t arg)
{
    td_s32 ret;

    ret = msensor_buf_delete_user((td_s32 *)arg);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("motionsensor buf delete user failed! ret=%x\n", ret);
        return TD_FAILURE;
    }
    return ret;
}

static td_s32 msensor_mng_drv_user_send_data(td_uintptr_t arg)
{
#ifdef USER_SEND_DATA
    td_s32 ret;
    ot_msensor_data *msensor_data;

    msensor_data = (ot_msensor_data *)arg;

    ret = msensor_mng_write_data_to_buf(msensor_data);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("motionsensor buf read data failed! ret=%x\n", ret);
        return TD_FAILURE;
    }
    return ret;
#else
    msensor_err_trace("motionsensor send data not support\n");
    return TD_FAILURE;
#endif
}

static td_s32 msensor_mng_drv_ioctl_get_data(td_u32 cmd, td_void *arg, td_void *private_data)
{
    ot_unused(cmd);
    ot_unused(private_data);
    return msensor_mng_drv_user_get_data((td_uintptr_t)arg);
}

static td_s32 msensor_mng_drv_ioctl_release_buf(td_u32 cmd, td_void *arg, td_void *private_data)
{
    ot_unused(cmd);
    ot_unused(private_data);
    return msensor_mng_drv_user_release_buf((td_uintptr_t)arg);
}

static td_s32 msensor_mng_drv_ioctl_add_user(td_u32 cmd, td_void *arg, td_void *private_data)
{
    ot_unused(cmd);
    ot_unused(private_data);
    return msensor_mng_drv_user_add_user((td_uintptr_t)arg);
}

static td_s32 msensor_mng_drv_ioctl_delete_user(td_u32 cmd, td_void *arg, td_void *private_data)
{
    ot_unused(cmd);
    ot_unused(private_data);
    return msensor_mng_drv_user_delete_user((td_uintptr_t)arg);
}

static td_s32 msensor_mng_drv_ioctl_send_data(td_u32 cmd, td_void *arg, td_void *private_data)
{
    ot_unused(cmd);
    ot_unused(private_data);
    return msensor_mng_drv_user_send_data((td_uintptr_t)arg);
}

static osal_ioctl_cmd g_msensor_mng_ioctl_cmd_list[] = {
    { MSENSOR_CMD_GET_DATA,    msensor_mng_drv_ioctl_get_data },
    { MSENSOR_CMD_RELEASE_BUF, msensor_mng_drv_ioctl_release_buf },
    { MSENSOR_CMD_ADD_USER,    msensor_mng_drv_ioctl_add_user },
    { MSENSOR_CMD_DELETE_USER, msensor_mng_drv_ioctl_delete_user },
    { MSENSOR_CMD_SEND_DATA,   msensor_mng_drv_ioctl_send_data },
};

td_void msensor_mng_set_ioctl_cmd_list(osal_fileops *msensor_fops)
{
    msensor_fops->cmd_list = g_msensor_mng_ioctl_cmd_list;
    msensor_fops->cmd_cnt = sizeof(g_msensor_mng_ioctl_cmd_list) / sizeof(g_msensor_mng_ioctl_cmd_list[0]);
}

static td_s32 msensor_mng_open(td_void *private_data)
{
    ot_unused(private_data);
    return TD_SUCCESS;
}

static td_s32 msensor_mng_release(td_void *private_data)
{
    ot_unused(private_data);
    return TD_SUCCESS;
}

static td_s32 msensor_mng_fn_init(td_void *args)
{
    ot_unused(args);
    return TD_SUCCESS;
}
static td_void msensor_mng_fn_exit(td_void)
{
    return;
}

static td_void msensor_mng_fn_query_state(mod_state *state)
{
    *state = MOD_STATE_FREE;
    return;
}

static td_void msensor_mng_fn_notify(mod_notice_id notice)
{
    ot_unused(notice);
    return;
}

static td_u32 msensor_mng_fn_get_ver_magic(td_void)
{
    return VERSION_MAGIC;
}

static td_s32 msensor_mng_init_buf(const ot_msensor_attr *motion_attr,
    const ot_msensor_buf_attr *msensor_buf_attr, const ot_msensor_config *msensor_config)
{
    td_u32 gyro_odr;
    td_u32 acc_odr;
    td_u32 magn_odr;
    td_s32 ret;

    if (motion_attr == TD_NULL || msensor_buf_attr == TD_NULL || msensor_config == TD_NULL) {
        msensor_err_trace("input NULL\n");
        return TD_FAILURE;
    }

    switch (motion_attr->device_mask) {
        case OT_MSENSOR_DEVICE_GYRO | OT_MSENSOR_DEVICE_ACC: {
            /* only for american present */
            gyro_odr = msensor_config->gyro_config.odr;
            acc_odr = msensor_config->acc_config.odr;
            magn_odr = 0;

            msensor_debug_trace("ODR:gyro_odr:%d acc_odr:%d magn_odr:%d\n",
                                gyro_odr, acc_odr, magn_odr);

            ret = msensor_buf_init(msensor_buf_attr, gyro_odr, acc_odr, magn_odr);
            break;
        }

        case OT_MSENSOR_DEVICE_GYRO: {
            /* only for american present */
            gyro_odr = msensor_config->gyro_config.odr;
            acc_odr = 0;
            magn_odr = 0;

            msensor_debug_trace("gyro_odr:%d acc_odr:%d magn_odr:%d\n",
                                gyro_odr, acc_odr, magn_odr);

            ret = msensor_buf_init(msensor_buf_attr, gyro_odr, acc_odr, magn_odr);
            break;
        }
        case OT_MSENSOR_DEVICE_ALL: {
            /* only for american present */
            gyro_odr = msensor_config->gyro_config.odr;
            acc_odr = msensor_config->acc_config.odr;
            magn_odr = msensor_config->acc_config.odr;

            msensor_debug_trace("gyro_odr:%d acc_odr:%d magn_odr:%d\n",
                                gyro_odr, acc_odr, magn_odr);

            ret = msensor_buf_init(msensor_buf_attr, gyro_odr, acc_odr, magn_odr);
            break;
        }
        default: {
            gyro_odr = 0;
            acc_odr = 0;
            magn_odr = 0;
            msensor_err_trace("motionsensor mng init buf(gyro_odr:%d acc_odr:%d magn_odr:%d) err!\n",
                              gyro_odr, acc_odr, magn_odr);
            ret = TD_FAILURE;
            break;
        }
    }
    return ret;
}

static td_s32 msensor_mng_deinit_buf(td_void)
{
    return msensor_buf_deinit();
}

static td_s32 msensor_mng_get_chip_cfg(ot_msensor_param *msensor_param)
{
    if (g_motionsensor_mng_callback.pfn_get_config_from_chip == TD_NULL) {
        msensor_err_trace("pfn_get_config_from_chip is NULL!\n");
        return TD_FAILURE;
    }
    return g_motionsensor_mng_callback.pfn_get_config_from_chip(msensor_param);
}

td_s32 msensor_mng_write_data_2_buf(td_void)
{
    if (g_motionsensor_mng_callback.pfn_write_data_to_buf == TD_NULL) {
        msensor_err_trace("pfn_write_data_to_buf is NULL!\n");
        return TD_FAILURE;
    }
    return g_motionsensor_mng_callback.pfn_write_data_to_buf();
}

static td_s32 msensor_mng_register_call_back(const msensor_mng_callback *callback)
{
    msensor_return_if_null_ptr(callback);

    g_motionsensor_mng_callback.pfn_get_config_from_chip = callback->pfn_get_config_from_chip;
    g_motionsensor_mng_callback.pfn_write_data_to_buf = callback->pfn_write_data_to_buf;
    return TD_SUCCESS;
}

static td_void msensor_mng_unregister_call_back(td_void)
{
    g_motionsensor_mng_callback.pfn_get_config_from_chip = TD_NULL;
    g_motionsensor_mng_callback.pfn_write_data_to_buf = TD_NULL;
    return;
}

static msensor_mng_export_func g_msensor_mng_export_funcs = {
    .pfn_add_msensor_user = msensor_buf_add_user,
    .pfn_delete_msensor_user = msensor_buf_delete_user,
    .pfn_get_data = msensor_buf_get_data,
    .pfn_release_data = msensor_buf_release_data,
    .pfn_get_msensor_config = msensor_mng_get_chip_cfg,

    /* call_back2_chip */
    .pfn_chip_write_data_to_mng_buff = msensor_mng_write_data_to_buf,
    .pfn_init = msensor_mng_init_buf,
    .pfn_deinit = msensor_mng_deinit_buf,

    /* call_back_to_chip */
    .pfn_register_call_back = msensor_mng_register_call_back,
    .pfn_unregister_call_back = msensor_mng_unregister_call_back,
};

static umap_module g_msensor_mng_module = {
    .mod_id = OT_ID_MOTIONSENSOR,
    .mod_name = "motionsensor",

    .pfn_init = msensor_mng_fn_init,
    .pfn_exit = msensor_mng_fn_exit,
    .pfn_query_state = msensor_mng_fn_query_state,
    .pfn_notify = msensor_mng_fn_notify,
    .pfn_ver_checker = msensor_mng_fn_get_ver_magic,

    .export_funcs = &g_msensor_mng_export_funcs,
    .data = TD_NULL,
};

static osal_fileops g_msensor_mng_fops = {
    .open = msensor_mng_open,
    .release = msensor_mng_release,
};

td_s32 motionsensor_mng_module_init(td_void)
{
    td_s32 ret;

    ret = cmpi_register_module(&g_msensor_mng_module);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("register_module failed!\n");
        return TD_FAILURE;
    }

    (td_void)memset_s(&g_mng_proc_info, sizeof(g_mng_proc_info), 0, sizeof(g_mng_proc_info));

#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    ret = motionsensor_proc_init();
    if (ret != TD_SUCCESS) {
        msensor_err_trace("motionsensor proc init failed\n");
        goto unregister_module;
    }
#endif

    g_msensor_mng_dev = osal_dev_create(MOTIONSENSOR_DEV_NAME);
    if (g_msensor_mng_dev == TD_NULL) {
        msensor_err_trace("motionsensor: create device failed\n");
        goto proc_exit;
    }

    msensor_mng_set_ioctl_cmd_list(&g_msensor_mng_fops);
    g_msensor_mng_dev->fops = &g_msensor_mng_fops;
    ret = osal_dev_register(g_msensor_mng_dev);
    if (ret != TD_SUCCESS) {
        msensor_err_trace("register motionsensor device failed!\n");
        goto destroydev;
    }

    ret = msensor_buf_lock_init();
    if (ret != TD_SUCCESS) {
        msensor_err_trace("register motionsensor buf lock init failed!\n");
        goto deregisterdevice;
    }

    osal_printk("load motionsensor_mng.ko ....OK!\n");
    return TD_SUCCESS;

deregisterdevice:
    osal_dev_unregister(g_msensor_mng_dev);
destroydev:
    osal_dev_destroy(g_msensor_mng_dev);
proc_exit:
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    motionsensor_proc_exit();
unregister_module:
#endif
    cmpi_unregister_module(OT_ID_MOTIONSENSOR);

    return TD_FAILURE;
}

/* if static, liteos warning */
td_void motionsensor_mng_module_exit(td_void)
{
    msensor_buf_lock_deinit();
    osal_dev_unregister(g_msensor_mng_dev);
    osal_dev_destroy(g_msensor_mng_dev);
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    motionsensor_proc_exit();
#endif
    cmpi_unregister_module(OT_ID_MOTIONSENSOR);
    osal_printk("unload motionsensor_mng.ko ....OK!\n");
}

module_init(motionsensor_mng_module_init);
module_exit(motionsensor_mng_module_exit);

MODULE_DESCRIPTION("motionsensor driver");
MODULE_LICENSE("GPL");
