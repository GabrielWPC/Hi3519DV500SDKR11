/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_statistics.h"
#include <sys/ioctl.h>
#include "ot_mpi_sys_mem.h"
#include "isp_ext_config.h"
#include "isp_main.h"

typedef struct {
    td_u64  stat_phyaddr;
    td_void *stat_virt_addr;

    td_bool read;
    isp_stat_info stat_info;
} isp_sta_ctx;

isp_sta_ctx g_stat_ctx[OT_ISP_MAX_PIPE_NUM] = { { 0 } };
#define statistics_get_ctx(pipe, ctx) ctx = &g_stat_ctx[pipe]

td_s32 isp_statistics_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_sta_ctx *stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    statistics_get_ctx(vi_pipe, stat);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_BUF_INIT, &stat->stat_phyaddr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] init statistics bufs failed!\n", vi_pipe);
        return ret;
    }

    stat->read = TD_FALSE;
    stat->stat_virt_addr = ot_mpi_sys_mmap_cached(stat->stat_phyaddr,
                                                  sizeof(isp_stat) * MAX_ISP_STAT_BUF_NUM);

    if (stat->stat_virt_addr == TD_NULL) {
        isp_err_trace("ISP[%d] mmap statistics bufs failed!\n", vi_pipe);

        ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_BUF_EXIT);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] exit statistics bufs failed!\n", vi_pipe);
            return ret;
        }

        return OT_ERR_ISP_NOMEM;
    }

    return TD_SUCCESS;
}

td_void isp_statistics_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_sta_ctx *stat = TD_NULL;

    statistics_get_ctx(vi_pipe, stat);

    if (stat->stat_virt_addr != TD_NULL) {
        ot_mpi_sys_munmap(stat->stat_virt_addr, sizeof(isp_stat) * MAX_ISP_STAT_BUF_NUM);
        stat->stat_virt_addr = TD_NULL;
        stat->stat_info.phy_addr  = 0;
        stat->stat_info.virt_addr = TD_NULL;
        stat->read                = TD_FALSE;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] exit statistics bufs failed!\n", vi_pipe);
        return;
    }
}

td_s32 isp_statistics_get_buf(ot_vi_pipe vi_pipe, td_void **stat)
{
    td_s32 ret;
    isp_sta_ctx *stat_ctx = TD_NULL;
    td_bool use_active_buffer = TD_FALSE;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat);

    statistics_get_ctx(vi_pipe, stat_ctx);

    if (!stat_ctx->read) {
        ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_BUF_GET, &stat_ctx->stat_info);
        if (ret) {              /* if do not have busy statistics buf, use last buf */
            ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &stat_ctx->stat_info);
            if (ret) {
                return ret;
            }
            use_active_buffer = TD_TRUE;
        }

        if (stat_ctx->stat_virt_addr != TD_NULL) {
            stat_ctx->stat_info.virt_addr = (td_void *)((td_u8 *)stat_ctx->stat_virt_addr +
                                                        (stat_ctx->stat_info.phy_addr - stat_ctx->stat_phyaddr));
        } else {
            stat_ctx->stat_info.virt_addr = TD_NULL;
        }
        if (use_active_buffer == TD_FALSE) {
            stat_ctx->read = TD_TRUE;
        } else {
            stat_ctx->read = TD_FALSE;
        }
    }

    if (stat_ctx->stat_info.virt_addr == TD_NULL) {
        return TD_FAILURE;
    }

    *stat = stat_ctx->stat_info.virt_addr;

    return TD_SUCCESS;
}

static td_void isp_statistic_update_checksum_en(ot_vi_pipe vi_pipe, td_u32 *update)
{
    td_u32 isr_access;
    td_bool check_sum_en = ot_ext_system_sys_check_sum_en_read(vi_pipe);
    isr_access = *update;

    if (check_sum_en == TD_TRUE) {
        isr_access |= (1 << CHECK_SUM_KEY_BIT);
    } else {
        isr_access &= (~(1 << CHECK_SUM_KEY_BIT));
    }
    *update = isr_access;
}

td_s32 isp_statistics_put_buf(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u32 update;
    isp_sta_ctx *stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    statistics_get_ctx(vi_pipe, stat);

    update = ot_ext_system_statistics_ctrl_highbit_read(vi_pipe);
    isp_statistic_update_checksum_en(vi_pipe, &update);
    stat->stat_info.stat_key.key = update;

    if (stat->read) {
        ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_BUF_PUT, &stat->stat_info);
        if (ret) {
            isp_err_trace("release ISP[%d] stat buf failed with ec %#x!\n", vi_pipe, ret);
        }
        stat->stat_info.virt_addr = TD_NULL;
        stat->read = TD_FALSE;
    }

    return TD_SUCCESS;
}

