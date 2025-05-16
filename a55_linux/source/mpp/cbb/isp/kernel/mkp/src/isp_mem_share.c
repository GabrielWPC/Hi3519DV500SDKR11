/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_mem_share.h"
#include "isp.h"

#define isp_check_mem_process_isolation_return() \
    do { \
        if (ot_mmz_get_mem_process_isolation() == 0) { \
            return TD_SUCCESS; \
        } \
    } while (0)


td_s32 isp_drv_mem_share_owner(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    if (drv_ctx->share_pid[0] != ISP_INVALID_PID) {
        isp_err_trace("share owner error, pid[0] = %d\n", drv_ctx->share_pid[0]);
        osal_sem_up(&drv_ctx->share_sem);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    drv_ctx->share_pid[0] = osal_get_current_tgid();
    osal_sem_up(&drv_ctx->share_sem);
    return TD_SUCCESS;
}

static td_s32 isp_drv_check_share(ot_vi_pipe vi_pipe, td_s32 pid, isp_drv_ctx *drv_ctx)
{
    if (osal_get_current_tgid() != drv_ctx->share_pid[0]) {
        isp_err_trace("pipe %d only process %d can share isp mem, current process is %d \n",
            vi_pipe, drv_ctx->share_pid[0], osal_get_current_tgid());
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (pid == drv_ctx->share_pid[0]) {
        isp_err_trace("pipe %d process %d is isp main process, no need share \n",
            vi_pipe, drv_ctx->share_pid[0]);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (drv_ctx->share_all_en == TD_TRUE) {
        isp_err_trace("pipe %d already share all\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_mem_share_exit(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_u32 i;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    if (drv_ctx->share_pid[0] != ISP_INVALID_PID && drv_ctx->share_pid[0] != osal_get_current_tgid()) {
        isp_err_trace("only the main process can exit pid[0]:%d, cur_pid:%d\n",
            drv_ctx->share_pid[0], osal_get_current_tgid());
        osal_sem_up(&drv_ctx->share_sem);
        return TD_FAILURE;
    }

    drv_ctx->share_all_en = TD_FALSE;

    for (i = 0; i < ISP_SHARE_ID_LEN; ++i) {
        drv_ctx->share_pid[i] = ISP_INVALID_PID;
    }
    osal_sem_up(&drv_ctx->share_sem);
    return TD_SUCCESS;
}

td_s32 isp_drv_mem_share_pid(ot_vi_pipe vi_pipe, td_s32 pid)
{
    td_u32 i;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    ret = isp_drv_check_share(vi_pipe, pid, drv_ctx);
    if (ret != TD_SUCCESS) {
        osal_sem_up(&drv_ctx->share_sem);
        return ret;
    }

    for (i = 0; i < ISP_SHARE_ID_LEN; ++i) {
        if (drv_ctx->share_pid[i] == ISP_INVALID_PID) {
            break;
        }
        if (drv_ctx->share_pid[i] == pid) {
            osal_sem_up(&drv_ctx->share_sem);
            isp_info_trace("pipe %d already share to process %d \n", vi_pipe, pid);
            return TD_SUCCESS;
        }
    }
    if (i >= ISP_SHARE_ID_LEN) {
        isp_err_trace("no more empty slot \n");
        osal_sem_up(&drv_ctx->share_sem);
        return OT_ERR_ISP_NOMEM;
    }
    drv_ctx->share_pid[i] = pid;
    osal_sem_up(&drv_ctx->share_sem);

    return TD_SUCCESS;
}

static td_s32 isp_drv_check_unshare(ot_vi_pipe vi_pipe, td_s32 pid, isp_drv_ctx *drv_ctx)
{
    if (osal_get_current_tgid() != drv_ctx->share_pid[0] && osal_get_current_tgid() != pid) {
        isp_err_trace("pipe %d only main process %d or own process %d can unshare, current process is %d \n",
            vi_pipe, drv_ctx->share_pid[0], pid, osal_get_current_tgid());
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (pid == drv_ctx->share_pid[0]) {
        isp_err_trace("pipe %d main process %d can not unshare \n",
            vi_pipe, drv_ctx->share_pid[0]);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (drv_ctx->share_all_en == TD_TRUE) {
        isp_err_trace("pipe %d already share all\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_mem_unshare_pid(ot_vi_pipe vi_pipe, td_s32 pid)
{
    td_u32 i;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    ret = isp_drv_check_unshare(vi_pipe, pid, drv_ctx);
    if (ret != TD_SUCCESS) {
        osal_sem_up(&drv_ctx->share_sem);
        return ret;
    }

    for (i = 1; i < ISP_SHARE_ID_LEN; ++i) {
        if (drv_ctx->share_pid[i] == pid) {
            break;
        }
    }

    if (i >= ISP_SHARE_ID_LEN) {
        isp_err_trace("pipe %d pid%d is not shared\n", vi_pipe, pid);
        osal_sem_up(&drv_ctx->share_sem);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    drv_ctx->share_pid[i] = ISP_INVALID_PID;
    osal_sem_up(&drv_ctx->share_sem);

    return TD_SUCCESS;
}

td_s32 isp_drv_mem_share_all(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    if (osal_get_current_tgid() != drv_ctx->share_pid[0]) {
        isp_err_trace("pipe %d only process %d can share all isp mem, current process is %d \n",
            vi_pipe, drv_ctx->share_pid[0], osal_get_current_tgid());
        osal_sem_up(&drv_ctx->share_sem);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    drv_ctx->share_all_en = TD_TRUE;

    osal_sem_up(&drv_ctx->share_sem);
    return TD_SUCCESS;
}

td_s32 isp_drv_mem_unshare_all(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    if (osal_get_current_tgid() != drv_ctx->share_pid[0]) {
        isp_err_trace("pipe %d only process %d can unshare all isp mem, current process is %d \n",
            vi_pipe, drv_ctx->share_pid[0], osal_get_current_tgid());
        osal_sem_up(&drv_ctx->share_sem);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    drv_ctx->share_all_en = TD_FALSE;
    osal_sem_up(&drv_ctx->share_sem);

    return TD_SUCCESS;
}

td_s32 isp_drv_mem_verify_pid(ot_vi_pipe vi_pipe, td_s32 pid)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_u32 i;

    isp_check_mem_process_isolation_return();
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->share_sem)) {
        return -ERESTARTSYS;
    }

    if (drv_ctx->share_all_en == TD_TRUE) {
        osal_sem_up(&drv_ctx->share_sem);
        return TD_SUCCESS;
    }

    for (i = 0; i < ISP_SHARE_ID_LEN; ++i) {
        if (drv_ctx->share_pid[i] == pid) {
            osal_sem_up(&drv_ctx->share_sem);
            return TD_SUCCESS;
        }
    }

    osal_sem_up(&drv_ctx->share_sem);
    isp_err_trace("pipe %d pid%d verify pid fail \n", vi_pipe, pid);

    return TD_FAILURE;
}

