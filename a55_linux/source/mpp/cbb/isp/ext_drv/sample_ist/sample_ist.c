/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __LITEOS__
#include "sample_ist.h"
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include "ot_debug.h"
#include "ot_common.h"
#include "isp_ext.h"
#include "securec.h"

#define MAX_TEST_NODES 4
#define MAX_DATA_STR_LEN 150
#define sync_task_info_trace(fmt, ...) \
    OT_INFO_TRACE(OT_ID_ISP, fmt, ##__VA_ARGS__)

td_s32 sync_callback(td_u64 data);
td_s32 sync_af_calc(td_u64 data);
td_s32 sync_ae_calc(td_u64 data);

ot_vi_pipe g_vi_pipe = 0;
ot_isp_drv_af_statistics g_focus_stat;
ot_isp_ae_stats g_ae_stats;

ot_isp_sync_task_node g_sync_node[MAX_TEST_NODES] = {
    {
        .method = ISP_SYNC_TSK_METHOD_HW_IRQ,
        .isp_sync_tsk_callback = sync_af_calc,
        .data = 0,
        .focus_stat = &g_focus_stat, /* if don't need af stats, can delete this task or */
                                     /* set focus_stat = TD_NULL and let isp_sync_tsk_callback do nothing */
        .ae_stat = TD_NULL,
        .sz_id = "hw_0"
    },
    {
        .method = ISP_SYNC_TSK_METHOD_HW_IRQ,
        .isp_sync_tsk_callback = sync_ae_calc,
        .data = 1,
        .focus_stat = TD_NULL,
        .ae_stat = &g_ae_stats, /* if don't need ae stats, can delete this task or */
                                /* set ae_stat = TD_NULL and let isp_sync_tsk_callback do nothing */
        .sz_id = "hw_1"
    },
    {
        .method = ISP_SYNC_TSK_METHOD_WORKQUE,
        .isp_sync_tsk_callback = sync_callback,
        .data = 3, /* 3 */
        .focus_stat = TD_NULL,
        .ae_stat = TD_NULL,
        .sz_id = "wq_0"
    },
    {
        .method = ISP_SYNC_TSK_METHOD_WORKQUE,
        .isp_sync_tsk_callback = sync_callback,
        .data = 4, /* 4 */
        .focus_stat = TD_NULL,
        .ae_stat = TD_NULL,
        .sz_id = "wq_1"
    }
};

td_s32 sync_af_calc(td_u64 data)
{
    td_s32 i, j;
    static td_s32 cnt = 0;
    td_u16 stat_data;
    td_char stat_data_str[MAX_DATA_STR_LEN] = {0};
    td_char *temp_str_ptr = TD_NULL;

    ot_isp_drv_be_focus_statistics *isp_focus_st = TD_NULL;
    isp_focus_st = &g_focus_stat.be_af_stat;

    ot_unused(data);

    /* get af statistics */
    if (cnt++ % 30 == 0) { /* calculate every 30 frames */
        sync_task_info_trace("h1:\n");
        for (i = 0; i < 15; i++) { /* this is a 15 * 17 matrix */
            memset_s(stat_data_str, sizeof(td_char) * MAX_DATA_STR_LEN, 0, sizeof(td_char) * MAX_DATA_STR_LEN);
            temp_str_ptr = stat_data_str;
            for (j = 0; j < 17; j++) { /* this is a 15 * 17 matrix */
                stat_data = isp_focus_st->zone_metrics[i][j].h1;
                sprintf_s(temp_str_ptr, sizeof(td_char) * 8, "%6d ", stat_data); /* sizeof "%6d " is 8 */
                temp_str_ptr += 7; /* length of "%6d " is 7 */
            }
            sync_task_info_trace("%s\n", stat_data_str);
        }
    }

    /* af algorithm */
    return 0;
}

td_s32 sync_ae_calc(td_u64 data)
{
    td_s32 i, j, wdr_idx;
    static td_s32 cnt = 0;
    td_u16 stat_data;
    td_char stat_data_str[MAX_DATA_STR_LEN] = {0};
    td_char *temp_str_ptr = TD_NULL;

    ot_isp_ae_stats *isp_ae_stat = TD_NULL;
    isp_ae_stat = &g_ae_stats;
    wdr_idx = 0; /* for example, print fe_zone_avg[wdr_idx][i][j][0] stats. if need, can print all ot_isp_ae_stats */

    ot_unused(data);

    /* get af statistics */
    if (cnt++ % 30 == 0) { /* calculate every 30 frames */
        sync_task_info_trace("fe0_ae_zone_stats:\n");
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) { /* this is a 15 * 17 matrix */
            memset_s(stat_data_str, sizeof(td_char) * MAX_DATA_STR_LEN, 0, sizeof(td_char) * MAX_DATA_STR_LEN);
            temp_str_ptr = stat_data_str;
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) { /* this is a 15 * 17 matrix */
                stat_data = isp_ae_stat->fe_zone_avg[wdr_idx][i][j][0];
                sprintf_s(temp_str_ptr, sizeof(td_char) * 8, "%6d ", stat_data); /* sizeof "%6d " is 8 */
                temp_str_ptr += 7; /* length of "%6d " is 7 */
            }
            sync_task_info_trace("%s\n", stat_data_str);
        }
    }

    /* af algorithm */
    return 0;
}

td_s32 sync_callback(td_u64 data)
{
    td_s32 local_data = data;

    sync_task_info_trace("%d\n", local_data);
    return 0;
}

/* file operation */
td_s32 sample_ist_open(struct inode *inode, struct file *file)
{
    ot_unused(inode);
    ot_unused(file);

    return 0;
}

td_s32 sample_ist_close(struct inode *inode, struct file *file)
{
    ot_unused(inode);
    ot_unused(file);

    return 0;
}

static td_slong sample_ist_ioctl(struct file *file, td_u32 cmd, td_ulong arg)
{
    td_s32 __user *argp = (td_s32 __user *)(td_uintptr_t)arg;
    td_s32 node_index = *argp;
    ot_unused(file);

    if (node_index >= MAX_TEST_NODES) {
        return -1;
    }

    switch (cmd) {
        case SAMPLE_IST_ADD_NODE:
            if (ckfn_isp_register_sync_task()) {
                call_isp_register_sync_task(g_vi_pipe, &g_sync_node[node_index]);
            } else {
                sync_task_info_trace("register sample_ist failed!\n");
                return -1;
            }
            break;

        case SAMPLE_IST_DEL_NODE:
            if (ckfn_isp_unregister_sync_task()) {
                if (call_isp_unregister_sync_task(g_vi_pipe, &g_sync_node[node_index]) == TD_FAILURE) {
                    sync_task_info_trace("del node err %d\n", node_index);
                }
            } else {
                sync_task_info_trace("unregister sample_ist failed!\n");
                return -1;
            }
            break;

        default: {
            sync_task_info_trace("invalid ioctl command!\n");
            return -ENOIOCTLCMD;
        }
    }

    return 0;
}

static struct file_operations g_sample_ist_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl = sample_ist_ioctl,
    .open       = sample_ist_open,
    .release    = sample_ist_close,
};

static struct miscdevice g_sample_ist_dev = {
    .minor   = MISC_DYNAMIC_MINOR,
    .name    = "sample_ist",
    .fops    = &g_sample_ist_fops,
};

/* module init and exit */
td_s32 sample_ist_mod_init(td_void)
{
    td_s32  i, j, ret;

    ret = misc_register(&g_sample_ist_dev);
    if (ret != 0) {
        osal_printk("register sample_ist device failed with %#x!\n", ret);
        return -1;
    }

    j = 0;
    for (i = 0; i < MAX_TEST_NODES; i++) {
        if (ckfn_isp_register_sync_task()) {
            ret = call_isp_register_sync_task(g_vi_pipe, &g_sync_node[i]);
            if (ret != TD_SUCCESS) {
                osal_printk("register sync_task to isp[%d] failed!\n", i);
                j++;
            }
        } else {
            goto OUT1;
        }
    }

    if (j == MAX_TEST_NODES) {
        goto OUT1;
    }
    osal_printk("load sample_ist.ko ....OK!\n");

    return 0;

OUT1:
    misc_deregister(&g_sample_ist_dev);

    return -1;
}

td_void sample_ist_mod_exit(td_void)
{
    td_s32 i;

    misc_deregister(&g_sample_ist_dev);

    for (i = 0; i < MAX_TEST_NODES; i++) {
        if (ckfn_isp_unregister_sync_task()) {
            call_isp_unregister_sync_task(g_vi_pipe, &g_sync_node[i]);
        }
    }
    osal_printk("unload sample_ist.ko ....OK!\n");
}
#endif
