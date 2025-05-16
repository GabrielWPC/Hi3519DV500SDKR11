/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: motionsensor_mng_proc.c
 * Author: ISP SW
 * Create: 2018-7-30
 * Version: Initial Draft
 */

#include "motionsensor_mng_proc.h"
#include <linux/kernel.h>
#include "ot_osal.h"
#include "securec.h"
#include "sys_ext.h"
#include "motionsensor_buf.h"
#include "motionsensor_exe.h"

#define MSENSOR_MNG_INFO "motionsensor_mng"
#define MSENSOR_MNG_VERSION_INFO "motionsensor_mng debug V0.0.0.1"

static td_void motionsensor_proc_show_gyro(osal_proc_entry *s, msensor_buf_info (*buf_info)[DATA_BUTT])
{
    msensor_mng_proc_info *proc_info = msensor_mng_get_proc_info();

    if (proc_info->gyro_name[0] == 0) {
        return;
    }

    call_sys_print_proc_title(s, "gyro sensor param");
    osal_seq_printf(s->seqfile, "%20s%16s%16s%16s%16s%16s\n",
                    "buf_addr", "buf_size", "overflow", "data_unmatch", "overflow_id", "data_unmatch_id");
    osal_seq_printf(s->seqfile, "%20llx", proc_info->buf_addr[MSENSOR_DATA_GYRO]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_size[MSENSOR_DATA_GYRO]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_overflow[MSENSOR_DATA_GYRO]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_data_unmatch[MSENSOR_DATA_GYRO]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_overflow_id[MSENSOR_DATA_GYRO]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_data_unmatch_id[MSENSOR_DATA_GYRO]);

    call_sys_print_proc_title(s, "gyro sensor addr");
    osal_seq_printf(s->seqfile, "\n%10s%20s%20s", "", "start_addr", "write_addr");
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "x",
        buf_info[MSENSOR_DATA_GYRO][DATA_X].start_addr,
        buf_info[MSENSOR_DATA_GYRO][DATA_X].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "y",
        buf_info[MSENSOR_DATA_GYRO][DATA_Y].start_addr,
        buf_info[MSENSOR_DATA_GYRO][DATA_Y].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "z",
        buf_info[MSENSOR_DATA_GYRO][DATA_Z].start_addr,
        buf_info[MSENSOR_DATA_GYRO][DATA_Z].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "temp",
        buf_info[MSENSOR_DATA_GYRO][DATA_TEMP].start_addr,
        buf_info[MSENSOR_DATA_GYRO][DATA_TEMP].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "pts",
        buf_info[MSENSOR_DATA_GYRO][DATA_PTS].start_addr,
        buf_info[MSENSOR_DATA_GYRO][DATA_PTS].write_ptr);

    return;
}

static td_void motionsensor_proc_show_acc(osal_proc_entry *s, msensor_buf_info (*buf_info)[DATA_BUTT])
{
    msensor_mng_proc_info *proc_info = msensor_mng_get_proc_info();

    if (proc_info->accel_name[0] == 0) {
        return;
    }

    call_sys_print_proc_title(s, "acc sensor param");
    osal_seq_printf(s->seqfile, "%20s%16s%16s%16s%16s%16s\n",
                    "buf_addr", "buf_size", "overflow", "data_unmatch", "overflow_id", "data_unmatch_id");
    osal_seq_printf(s->seqfile, "%20llx", proc_info->buf_addr[MSENSOR_DATA_ACC]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_size[MSENSOR_DATA_ACC]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_overflow[MSENSOR_DATA_ACC]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_data_unmatch[MSENSOR_DATA_ACC]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_overflow_id[MSENSOR_DATA_ACC]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_data_unmatch_id[MSENSOR_DATA_ACC]);

    call_sys_print_proc_title(s, "acc sensor addr");
    osal_seq_printf(s->seqfile, "\n%10s%20s%20s", "", "start_addr", "write_addr");
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "x",
        buf_info[MSENSOR_DATA_ACC][DATA_X].start_addr,
        buf_info[MSENSOR_DATA_ACC][DATA_X].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "y",
        buf_info[MSENSOR_DATA_ACC][DATA_Y].start_addr,
        buf_info[MSENSOR_DATA_ACC][DATA_Y].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "z",
        buf_info[MSENSOR_DATA_ACC][DATA_Z].start_addr,
        buf_info[MSENSOR_DATA_ACC][DATA_Z].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "temp",
        buf_info[MSENSOR_DATA_ACC][DATA_TEMP].start_addr,
        buf_info[MSENSOR_DATA_ACC][DATA_TEMP].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px\n", "pts",
        buf_info[MSENSOR_DATA_ACC][DATA_PTS].start_addr,
        buf_info[MSENSOR_DATA_ACC][DATA_PTS].write_ptr);

    return;
}

static td_void motionsensor_proc_show_magn(osal_proc_entry *s, msensor_buf_info (*buf_info)[DATA_BUTT])
{
    msensor_mng_proc_info *proc_info = msensor_mng_get_proc_info();

    if (proc_info->mag_name[0] == 0) {
        return;
    }
#ifdef MSENSOR_DEBUG
    call_sys_print_proc_title(s, "magn sensor param");
    osal_seq_printf(s->seqfile, "%20s%16s%16s%16s%16s%16s\n",
                    "buf_addr", "buf_size", "overflow", "data_unmatch", "overflow_id", "data_unmatch_id");
    osal_seq_printf(s->seqfile, "%20llx", proc_info->buf_addr[MSENSOR_DATA_MAGN]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_size[MSENSOR_DATA_MAGN]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_overflow[MSENSOR_DATA_MAGN]);
    osal_seq_printf(s->seqfile, "%16u", proc_info->buf_data_unmatch[MSENSOR_DATA_MAGN]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_overflow_id[MSENSOR_DATA_MAGN]);
    osal_seq_printf(s->seqfile, "%16d", proc_info->buf_data_unmatch_id[MSENSOR_DATA_MAGN]);

    call_sys_print_proc_title(s, "magn sensor addr");
    osal_seq_printf(s->seqfile, "\n%10s%20s%20s", "", "start_addr", "write_addr");
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "x",
        buf_info[MSENSOR_DATA_MAGN][DATA_X].start_addr,
        buf_info[MSENSOR_DATA_MAGN][DATA_X].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "y",
        buf_info[MSENSOR_DATA_MAGN][DATA_Y].start_addr,
        buf_info[MSENSOR_DATA_MAGN][DATA_Y].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "z",
        buf_info[MSENSOR_DATA_MAGN][DATA_Z].start_addr,
        buf_info[MSENSOR_DATA_MAGN][DATA_Z].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "temp",
        buf_info[MSENSOR_DATA_MAGN][DATA_TEMP].start_addr,
        buf_info[MSENSOR_DATA_MAGN][DATA_TEMP].write_ptr);
    osal_seq_printf(s->seqfile, "\n%10s%20px%20px", "pts",
        buf_info[MSENSOR_DATA_MAGN][DATA_PTS].start_addr,
        buf_info[MSENSOR_DATA_MAGN][DATA_PTS].write_ptr);
#endif
    return;
}

static td_s32 motionsensor_proc_show(osal_proc_entry *s)
{
    td_s32 ret;
    msensor_buf_info (*buf_info)[DATA_BUTT] = TD_NULL;

    buf_info = (msensor_buf_info(*)[DATA_BUTT])msensor_buf_get_info();
    ret = msensor_mng_proc_info_init();
    if (ret != TD_SUCCESS) {
        printk("motionsensor mng proc info init failed!\n");
        return -1;
    }

    osal_seq_printf(s->seqfile,
        "[motionsensor] version:[" MSENSOR_MNG_VERSION_INFO "], build time["__DATE__", "__TIME__"]\n");

    motionsensor_proc_show_gyro(s, buf_info);
    motionsensor_proc_show_acc(s, buf_info);
    motionsensor_proc_show_magn(s, buf_info);

    return 0;
}

td_s32 motionsensor_proc_init(td_void)
{
    osal_proc_entry *motionsensor_entry = TD_NULL;

    motionsensor_entry = osal_create_proc_entry(MSENSOR_MNG_INFO, TD_NULL);
    if (motionsensor_entry == TD_NULL) {
        printk("osal_create_proc_entry failed!\n");
        return -1;
    }

    motionsensor_entry->read = motionsensor_proc_show;
    motionsensor_entry->write = TD_NULL;
    return 0;
}

void motionsensor_proc_exit(td_void)
{
    osal_remove_proc_entry(MSENSOR_MNG_INFO, TD_NULL);
    return;
}
