/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "isp_ioctl.h"

#include "isp.h"
#include "isp_drv.h"
#include "isp_drv_define.h"
#include "isp_drv_vreg.h"
#include "mm_ext.h"
#include "osal_ioctl.h"
#include "mkp_isp.h"
#include "isp_mem_share.h"
#include "isp_dfx.h"

static isp_version g_isp_lib_info = { { 0 } };
static td_u16  g_res_max_width[OT_ISP_MAX_PIPE_NUM] = { OT_ISP_RES_WIDTH_MAX };
static td_u16  g_res_max_height[OT_ISP_MAX_PIPE_NUM] = { OT_ISP_RES_HEIGHT_MAX };
td_u32 g_isp_exit_timeout = 2000;     /* The time(unit:ms) of exit be buffer timeout */

#define ISP_ALG_MOD_NAME_LENGTH  20
static td_char g_isp_alg_mod_name[OT_ISP_ALG_MOD_BUTT][ISP_ALG_MOD_NAME_LENGTH] = {
    "ae", "af", "awb", "blc", "dpc", "pregamma", "drc", "demosaic", "antifalsecolor",
    "gamma", "crosstalk", "sharpen", "fswdr", "fpn", "dehaze", "local cac", "cac", "bayershp", "csc",
    "expander", "mcds", "auto color shading", "mesh shading", "rc", "rgbir", "hrs", "dgain", "bayer_nr", "flicker",
    "ldci", "ca", "clut", "ccm", "pq_ai", "hnr", "lblc"
};

td_s32 isp_drv_wait_condition_callback(const td_void *param)
{
    td_bool condition;

    condition = *(td_bool *)param;

    return (condition == TD_TRUE);
}

td_s32 isp_drv_wait_exit_callback(const td_void *param)
{
    td_s32 condition;

    condition = *(td_s32 *)param;

    return (condition == 0);
}

td_s32 isp_get_frame_edge(ot_vi_pipe vi_pipe, td_u32 *status)
{
    unsigned long flags = 0;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(status);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    ret = osal_wait_timeout_interruptible(&drv_ctx->isp_wait, isp_drv_wait_condition_callback, &drv_ctx->edge,
        isp_drv_get_int_timeout(vi_pipe));
    if (ret <= 0) {
        *status = 0;
        isp_warn_trace("Get Interrupt timeout failed!\n");
        return TD_FAILURE;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->edge = TD_FALSE;
    *status = drv_ctx->status;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static int isp_get_vd_start_time_out(ot_vi_pipe vi_pipe, td_u32 milli_sec, td_u32 *pu32status)
{
    unsigned long flags = 0;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem)) {
        return -ERESTARTSYS;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->vd_start = TD_FALSE;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (milli_sec != 0) {
        ret = osal_wait_timeout_interruptible(&drv_ctx->isp_wait_vd_start, isp_drv_wait_condition_callback,
            &drv_ctx->vd_start, (milli_sec));
        if (ret <= 0) {
            osal_sem_up(&drv_ctx->isp_sem);
            return TD_FAILURE;
        }
    } else {
        ret = osal_wait_interruptible(&drv_ctx->isp_wait_vd_start, isp_drv_wait_condition_callback,
            &drv_ctx->vd_start);
        if (ret) {
            osal_sem_up(&drv_ctx->isp_sem);
            return TD_FAILURE;
        }
    }

    *pu32status = drv_ctx->status;

    osal_sem_up(&drv_ctx->isp_sem);

    return TD_SUCCESS;
}

static int isp_get_vd_end_time_out(ot_vi_pipe vi_pipe, td_u32 milli_sec, td_u32 *pu32status)
{
    unsigned long flags = 0;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_vd)) {
        return -ERESTARTSYS;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->vd_end = TD_FALSE;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (milli_sec != 0) {
        ret = osal_wait_timeout_interruptible(&drv_ctx->isp_wait_vd_end, isp_drv_wait_condition_callback,
            &drv_ctx->vd_end, (milli_sec));
        if (ret <= 0) {
            osal_sem_up(&drv_ctx->isp_sem_vd);
            return TD_FAILURE;
        }
    } else {
        ret = osal_wait_interruptible(&drv_ctx->isp_wait_vd_end, isp_drv_wait_condition_callback, &drv_ctx->vd_end);
        if (ret) {
            osal_sem_up(&drv_ctx->isp_sem_vd);
            return TD_FAILURE;
        }
    }

    *pu32status = drv_ctx->status;

    osal_sem_up(&drv_ctx->isp_sem_vd);

    return TD_SUCCESS;
}

static int isp_get_vd_be_end_time_out(ot_vi_pipe vi_pipe, td_u32 milli_sec, td_u32 *pu32status)
{
    unsigned long flags = 0;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_be_vd)) {
        return -ERESTARTSYS;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    if (drv_ctx->run_once_flag != TD_TRUE) {
        drv_ctx->vd_be_end = TD_FALSE;
    }
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (milli_sec != 0) {
        ret = osal_wait_timeout_interruptible(&drv_ctx->isp_wait_vd_be_end, isp_drv_wait_condition_callback,
            &drv_ctx->vd_be_end, (milli_sec));
        if (ret <= 0) {
            osal_sem_up(&drv_ctx->isp_sem_be_vd);
            return TD_FAILURE;
        }
    } else {
        ret = osal_wait_interruptible(&drv_ctx->isp_wait_vd_be_end, isp_drv_wait_condition_callback,
            &drv_ctx->vd_be_end);
        if (ret) {
            osal_sem_up(&drv_ctx->isp_sem_be_vd);
            return TD_FAILURE;
        }
    }

    *pu32status = 1;

    osal_sem_up(&drv_ctx->isp_sem_be_vd);

    return TD_SUCCESS;
}


static td_s32 isp_drv_set_isp_run_state(ot_vi_pipe vi_pipe, td_u64 *hand_signal)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(hand_signal);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    if (*hand_signal == ISP_INIT_HAND_SIGNAL) {
        drv_ctx->isp_run_flag = TD_TRUE;
    } else if (*hand_signal == ISP_EXIT_HAND_SIGNAL) {
        drv_ctx->isp_run_flag = TD_FALSE;
        isp_drv_reset_fe_cfg(vi_pipe);
    } else {
        isp_err_trace("ISP[%d] set isp run state failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_wdr_attr(ot_vi_pipe vi_pipe, vi_pipe_wdr_attr *wdr_attr)
{
    td_u32 i;
    td_u32 num;
    td_s32 ret;
    vi_pipe_wdr_attr wdr_attr_str = { 0 };
    vi_pipe_wdr_attr wdr_attr_str_tmp = { 0 };
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(wdr_attr);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (!ckfn_vi_get_pipe_wdr_attr()) {
        isp_err_trace("pipe[%d] ckfn_vi_get_pipe_wdr_attr is null\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = call_vi_get_pipe_wdr_attr(vi_pipe, &wdr_attr_str);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    /* Not WDR mode,BindPipe attr update */
    if (!is_fs_wdr_mode(wdr_attr_str.wdr_mode) && (wdr_attr_str.pipe_num != 1)) {
        wdr_attr_str.pipe_num = 1;
        wdr_attr_str.pipe_id[0] = vi_pipe;
    }

    num = wdr_attr_str.pipe_num;
    if ((num < 1) || (num > ISP_WDR_CHN_MAX)) {
        isp_err_trace("pipe[%d] Err wdr bind num %d!\n", vi_pipe, num);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    for (i = 0; i < num; i++) {
        isp_check_pipe_return(wdr_attr_str.pipe_id[i]);
    }
    if ((wdr_attr_str.wdr_mode > OT_WDR_MODE_BUTT)) {
        isp_err_trace("pipe[%d] Err wdr mode %d!\n", vi_pipe, wdr_attr_str.wdr_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((is_half_wdr_mode(wdr_attr_str.wdr_mode) == TD_TRUE) &&
        (isp_drv_get_run_wakeup_sel(vi_pipe) == OT_ISP_RUN_WAKEUP_BE_END)) {
        isp_err_trace("cur pipe%d run wakeup select = be_end, not support frame wdr!\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    (td_void)memcpy_s(&wdr_attr_str_tmp, sizeof(vi_pipe_wdr_attr), &wdr_attr_str, sizeof(vi_pipe_wdr_attr));

    for (i = 0; i < num; i++) {
        wdr_attr_str_tmp.pipe_id[i] = wdr_attr_str.pipe_id[num - 1 - i];
    }
    (td_void)memcpy_s(&drv_ctx->wdr_attr, sizeof(vi_pipe_wdr_attr), &wdr_attr_str_tmp, sizeof(vi_pipe_wdr_attr));
    (td_void)memcpy_s(wdr_attr, sizeof(vi_pipe_wdr_attr), &wdr_attr_str_tmp, sizeof(vi_pipe_wdr_attr));

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_pipe_size(ot_vi_pipe vi_pipe, ot_size *pipe_size)
{
    td_s32 ret;
    ot_size pipe_size_str = { 0 };

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pipe_size);
    isp_check_vir_pipe_return(vi_pipe);

    if (!ckfn_vi_get_pipe_in_size()) {
        isp_err_trace("pipe[%d] ckfn_vi_get_pipe_bind_dev_size is null\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = call_vi_get_pipe_in_size(vi_pipe, &pipe_size_str);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe[%d] call_vi_get_pipe_bind_dev_size failed 0x%x!\n", vi_pipe, ret);
        return ret;
    }

    if ((pipe_size_str.width < OT_ISP_WIDTH_MIN) || (pipe_size_str.width > OT_ISP_SENSOR_WIDTH_MAX) ||
        (pipe_size_str.height < OT_ISP_HEIGHT_MIN) || (pipe_size_str.height > OT_ISP_SENSOR_HEIGHT_MAX)) {
        isp_err_trace("pipe[%d]: Image Width should between [%d, %d], Height should between[%d, %d]\n", vi_pipe,
            OT_ISP_WIDTH_MIN, OT_ISP_SENSOR_WIDTH_MAX, OT_ISP_HEIGHT_MIN, OT_ISP_SENSOR_HEIGHT_MAX);
        return TD_FAILURE;
    }

    (td_void)memcpy_s(pipe_size, sizeof(ot_size), &pipe_size_str, sizeof(ot_size));

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_hdr_attr(ot_vi_pipe vi_pipe, vi_pipe_hdr_attr *hdr_attr)
{
    td_s32 ret;
    vi_pipe_hdr_attr hdr_attr_str = { 0 };

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(hdr_attr);

    if (!ckfn_vi_get_pipe_hdr_attr()) {
        isp_err_trace("pipe[%d] ckfn_vi_get_pipe_hdr_attr is null\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = call_vi_get_pipe_hdr_attr(vi_pipe, &hdr_attr_str);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe[%d] call_vi_get_pipe_hdr_attr failed 0x%x!\n", vi_pipe, ret);
        return ret;
    }

    (td_void)memcpy_s(hdr_attr, sizeof(vi_pipe_hdr_attr), &hdr_attr_str, sizeof(vi_pipe_hdr_attr));

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_STITCH_GRP
td_s32 isp_drv_get_stitch_attr(ot_vi_pipe vi_pipe, vi_stitch_attr *stitch_attr)
{
    td_u8 i;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    vi_stitch_attr stitch_attr_str = { 0 };
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stitch_attr);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (!ckfn_vi_get_pipe_stitch_attr()) {
        isp_err_trace("pipe[%d] ckfn_vi_get_pipe_stitch_attr is null\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = call_vi_get_pipe_stitch_attr(vi_pipe, &stitch_attr_str);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe[%d] call_vi_get_pipe_stitch_attr failed 0x%x!\n", vi_pipe, ret);
        return ret;
    }

    if (stitch_attr_str.stitch_enable) {
        if ((stitch_attr_str.stitch_pipe_num < 1) || (stitch_attr_str.stitch_pipe_num > OT_ISP_MAX_STITCH_NUM)) {
            isp_err_trace("pipe[%d] err stitch num %d\n", vi_pipe, stitch_attr_str.stitch_pipe_num);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
        for (i = 0; i < stitch_attr_str.stitch_pipe_num; i++) {
            isp_check_pipe_return(stitch_attr_str.stitch_bind_id[i]);
        }
    }
    (td_void)memcpy_s(&drv_ctx->stitch_attr, sizeof(vi_stitch_attr), &stitch_attr_str, sizeof(vi_stitch_attr));
    (td_void)memcpy_s(stitch_attr, sizeof(vi_stitch_attr), &stitch_attr_str, sizeof(vi_stitch_attr));
    return TD_SUCCESS;
}
#else
td_s32 isp_drv_get_stitch_attr(ot_vi_pipe vi_pipe, vi_stitch_attr *stitch_attr)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stitch_attr);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    (td_void)memset_s(&drv_ctx->stitch_attr, sizeof(vi_stitch_attr), 0, sizeof(vi_stitch_attr));
    (td_void)memset_s(stitch_attr, sizeof(vi_stitch_attr), 0, sizeof(vi_stitch_attr));
    return TD_SUCCESS;
}
#endif
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_s32 isp_drv_check_detail_stats_param(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    if (drv_ctx->isp_init != TD_TRUE) {
        isp_err_trace("pipe:%d, isp is not inited:%d\n", vi_pipe, drv_ctx->isp_init);
        return OT_ERR_ISP_NOT_INIT;
    }
    if (drv_ctx->detail_stats_cfg.enable != TD_TRUE) {
        isp_err_trace("pipe:%d, detail_stats is not enable, not support get detail_stats\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }
    if (drv_ctx->detail_stats_buf.init != TD_TRUE) {
        isp_err_trace("pipe:%d, detail_stats buf is not inited:%d\n", vi_pipe, drv_ctx->detail_stats_buf.init);
        return OT_ERR_ISP_NOT_INIT;
    }

    return TD_SUCCESS;
}
#endif
static td_s32 isp_drv_get_version(isp_version *version)
{
    isp_check_pointer_return(version);

    (td_void)memcpy_s(&g_isp_lib_info, sizeof(isp_version), version, sizeof(isp_version));

    g_isp_lib_info.magic = VERSION_MAGIC + ISP_MAGIC_OFFSET;
    (td_void)memcpy_s(version, sizeof(isp_version), &g_isp_lib_info, sizeof(isp_version));

    return TD_SUCCESS;
}

static td_s32 isp_drv_proc_init(ot_vi_pipe vi_pipe, isp_proc_mem *proc_mem)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(proc_mem);

    if (isp_drv_get_proc_param(vi_pipe) == 0) {
        return TD_SUCCESS;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->porc_mem.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].proc", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    malloc_param.buf_name = ac_name;
    malloc_param.size = ISP_PROC_SIZE;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_cached(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("alloc proc buf err\n");
        return OT_ERR_ISP_NOMEM;
    }

    ((td_char *)vir_addr)[0] = '\0';
    ((td_char *)vir_addr)[ISP_PROC_SIZE - 1] = '\0';

    if (osal_sem_down(&drv_ctx->proc_sem)) {
        if (phy_addr != 0) {
            cmpi_mmz_free(phy_addr, vir_addr);
        }
        return -ERESTARTSYS;
    }

    drv_ctx->porc_mem.init = TD_TRUE;
    drv_ctx->porc_mem.phy_addr = phy_addr;
    drv_ctx->porc_mem.size = ISP_PROC_SIZE;
    drv_ctx->porc_mem.virt_addr = (td_void *)vir_addr;

    (td_void)memcpy_s(proc_mem, sizeof(isp_proc_mem), &drv_ctx->porc_mem, sizeof(isp_proc_mem));

    osal_sem_up(&drv_ctx->proc_sem);
#endif
    return TD_SUCCESS;
}

static td_s32 isp_drv_proc_exit(ot_vi_pipe vi_pipe)
{
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    if (isp_drv_get_proc_param(vi_pipe) == 0) {
        return TD_SUCCESS;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    isp_check_buf_exit_return(vi_pipe, drv_ctx->porc_mem.init);

    phy_addr = drv_ctx->porc_mem.phy_addr;
    vir_addr = (td_u8 *)drv_ctx->porc_mem.virt_addr;
    // it is necessary to up semphore firstly when user process was killed(maybe semphore is down)
    if (drv_ctx->proc_sem_flag == TD_TRUE) {
        osal_sem_up(&drv_ctx->proc_sem);
        drv_ctx->proc_sem_flag = TD_FALSE;
    }

    if (osal_sem_down(&drv_ctx->proc_sem)) {
        return -ERESTARTSYS;
    }

    drv_ctx->porc_mem.init = TD_FALSE;
    drv_ctx->porc_mem.phy_addr = 0;
    drv_ctx->porc_mem.size = 0;
    drv_ctx->porc_mem.virt_addr = TD_NULL;
    osal_sem_up(&drv_ctx->proc_sem);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }
#endif
    return TD_SUCCESS;
}

static td_void isp_drv_trans_info_addr_init(isp_drv_ctx *drv_ctx, td_phys_addr_t phy_addr, td_u8 *vir_addr)
{
    td_u32 size_dng_info, size_frame_info, size_attach_info, size_color_gammut;
    size_dng_info = sizeof(ot_isp_dng_image_static_info);
    size_attach_info = sizeof(ot_isp_attach_info);
    size_color_gammut = sizeof(ot_isp_colorgammut_info);
    size_frame_info = sizeof(ot_isp_frame_info) * ISP_MAX_FRAMEINFO_BUF_NUM;

    drv_ctx->trans_info.init = TD_TRUE;
    drv_ctx->trans_info.dng_info.phy_addr = phy_addr;
    drv_ctx->trans_info.dng_info.vir_addr = (td_void *)vir_addr;

    drv_ctx->trans_info.atta_info.phy_addr = drv_ctx->trans_info.dng_info.phy_addr + size_dng_info;
    drv_ctx->trans_info.atta_info.vir_addr =
        (td_void *)((td_u8 *)drv_ctx->trans_info.dng_info.vir_addr + size_dng_info);

    drv_ctx->trans_info.color_gammut_info.phy_addr = drv_ctx->trans_info.atta_info.phy_addr + size_attach_info;
    drv_ctx->trans_info.color_gammut_info.vir_addr =
        (td_void *)((td_u8 *)drv_ctx->trans_info.atta_info.vir_addr + size_attach_info);

    drv_ctx->trans_info.frame_info.phy_addr = drv_ctx->trans_info.color_gammut_info.phy_addr + size_color_gammut;
    drv_ctx->trans_info.frame_info.vir_addr =
        (td_void *)((td_u8 *)drv_ctx->trans_info.color_gammut_info.vir_addr + size_color_gammut);

    drv_ctx->trans_info.update_info.phy_addr = drv_ctx->trans_info.frame_info.phy_addr + size_frame_info;
    drv_ctx->trans_info.update_info.vir_addr =
        (td_void *)((td_u8 *)drv_ctx->trans_info.frame_info.vir_addr + size_frame_info);
}

static td_s32 isp_drv_trans_info_buf_init(ot_vi_pipe vi_pipe, isp_trans_info_buf *trans_info)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    td_u32 size_dng_info, size_update_info, size_frame_info, size_attach_info, size_color_gammut;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(trans_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->trans_info.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].trans", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    size_dng_info = sizeof(ot_isp_dng_image_static_info);
    size_attach_info = sizeof(ot_isp_attach_info);
    size_color_gammut = sizeof(ot_isp_colorgammut_info);
    size_frame_info = sizeof(ot_isp_frame_info) * ISP_MAX_FRAMEINFO_BUF_NUM;
    size_update_info = sizeof(ot_isp_dcf_update_info) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ot_isp_dcf_const_info);

    malloc_param.buf_name = ac_name;
    malloc_param.size = size_dng_info + size_attach_info + size_color_gammut + size_frame_info + size_update_info;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_nocache(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] alloc ISP Trans info buf err\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    isp_drv_trans_info_addr_init(drv_ctx, phy_addr, vir_addr);
    (td_void)memcpy_s(trans_info, sizeof(isp_trans_info_buf), &drv_ctx->trans_info, sizeof(isp_trans_info_buf));

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_trans_info_buf_exit(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_u64 phy_addr;
    td_u8 *vir_addr = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    isp_check_buf_exit_return(vi_pipe, drv_ctx->trans_info.init);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    phy_addr = drv_ctx->trans_info.dng_info.phy_addr;
    vir_addr = (td_u8 *)drv_ctx->trans_info.dng_info.vir_addr;

    drv_ctx->trans_info.init = TD_FALSE;

    drv_ctx->trans_info.dng_info.phy_addr = 0;
    drv_ctx->trans_info.dng_info.vir_addr = TD_NULL;

    drv_ctx->trans_info.atta_info.phy_addr = 0;
    drv_ctx->trans_info.atta_info.vir_addr = TD_NULL;

    drv_ctx->trans_info.color_gammut_info.phy_addr = 0;
    drv_ctx->trans_info.color_gammut_info.vir_addr = TD_NULL;

    drv_ctx->trans_info.frame_info.phy_addr = 0;
    drv_ctx->trans_info.frame_info.vir_addr = TD_NULL;

    drv_ctx->trans_info.update_info.phy_addr = 0;
    drv_ctx->trans_info.update_info.vir_addr = TD_NULL;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_fe_stat_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret, i;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = {0};
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->fe_statistics_buf.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].fe_stat", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    malloc_param.buf_name = ac_name;
    malloc_param.size = sizeof(isp_fe_stat) * MAX_ISP_FE_STAT_BUF_NUM;
    malloc_param.kernel_only = TD_TRUE;
    ret = cmpi_mmz_malloc_cached(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("alloc ISP statistics buf err\n");
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->fe_statistics_buf.phy_addr = phy_addr;
    drv_ctx->fe_statistics_buf.vir_addr = (td_void *)vir_addr;
    drv_ctx->fe_statistics_buf.size = malloc_param.size;

    OSAL_INIT_LIST_HEAD(&drv_ctx->fe_statistics_buf.free_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->fe_statistics_buf.busy_list);

    for (i = 0; i < MAX_ISP_FE_STAT_BUF_NUM; i++) {
        drv_ctx->fe_statistics_buf.node[i].stat_info.phy_addr = phy_addr + i * sizeof(isp_fe_stat);
        drv_ctx->fe_statistics_buf.node[i].stat_info.virt_addr = (td_void *)(vir_addr + i * sizeof(isp_fe_stat));

        drv_ctx->fe_statistics_buf.node[i].stat_info.stat_key.key = ISP_STATISTICS_KEY;

        osal_list_add_tail(&drv_ctx->fe_statistics_buf.node[i].list, &drv_ctx->fe_statistics_buf.free_list);
    }

    drv_ctx->fe_statistics_buf.init = TD_TRUE;
    drv_ctx->fe_statistics_buf.busy_num = 0;
    drv_ctx->fe_statistics_buf.free_num = MAX_ISP_FE_STAT_BUF_NUM;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

td_s32 isp_drv_stat_buf_exit(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;

    td_phys_addr_t fe_phy_addr;
    td_u8 *fe_vir_addr = TD_NULL;

    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    if ((drv_ctx->statistics_buf.init == TD_FALSE) && (drv_ctx->fe_statistics_buf.init == TD_FALSE)) {
        return TD_SUCCESS;
    }

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    phy_addr = drv_ctx->statistics_buf.phy_addr;
    vir_addr = (td_u8 *)drv_ctx->statistics_buf.vir_addr;

    fe_phy_addr = drv_ctx->fe_statistics_buf.phy_addr;
    fe_vir_addr = (td_u8 *)drv_ctx->fe_statistics_buf.vir_addr;

    drv_ctx->statistics_buf.vir_addr = TD_NULL;
    drv_ctx->statistics_buf.node[0].stat_info.virt_addr = TD_NULL;
    drv_ctx->statistics_buf.node[1].stat_info.virt_addr = TD_NULL;
    drv_ctx->statistics_buf.phy_addr = 0;
    drv_ctx->statistics_buf.node[0].stat_info.phy_addr = 0;
    drv_ctx->statistics_buf.node[1].stat_info.phy_addr = 0;
    drv_ctx->statistics_buf.init = TD_FALSE;
    drv_ctx->statistics_buf.act_stat = TD_NULL;

    drv_ctx->fe_statistics_buf.vir_addr = TD_NULL;
    drv_ctx->fe_statistics_buf.node[0].stat_info.virt_addr = TD_NULL;
    drv_ctx->fe_statistics_buf.node[1].stat_info.virt_addr = TD_NULL;
    drv_ctx->fe_statistics_buf.phy_addr = 0;
    drv_ctx->fe_statistics_buf.node[0].stat_info.phy_addr = 0;
    drv_ctx->fe_statistics_buf.node[1].stat_info.phy_addr = 0;
    drv_ctx->fe_statistics_buf.init = TD_FALSE;
    drv_ctx->fe_statistics_buf.act_stat = TD_NULL;

    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.free_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.busy_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.user_list);

    OSAL_INIT_LIST_HEAD(&drv_ctx->fe_statistics_buf.free_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->fe_statistics_buf.busy_list);

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    if (fe_phy_addr != 0) {
        cmpi_mmz_free(fe_phy_addr, fe_vir_addr);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_stat_buf_init(ot_vi_pipe vi_pipe, td_phys_addr_t *point_phy_addr)
{
    td_s32 ret, i;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = {0};
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(point_phy_addr);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->statistics_buf.init);

    if (snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].stat", vi_pipe) < 0) {
        return TD_FAILURE;
    }

    malloc_param.buf_name = ac_name;
    malloc_param.size = sizeof(isp_stat) * MAX_ISP_STAT_BUF_NUM;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_cached(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("alloc ISP statistics buf err\n");
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->statistics_buf.phy_addr = phy_addr;
    drv_ctx->statistics_buf.vir_addr = (td_void *)vir_addr;
    drv_ctx->statistics_buf.size = malloc_param.size;

    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.free_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.busy_list);
    OSAL_INIT_LIST_HEAD(&drv_ctx->statistics_buf.user_list);

    for (i = 0; i < MAX_ISP_STAT_BUF_NUM; i++) {
        drv_ctx->statistics_buf.node[i].stat_info.phy_addr = phy_addr + i * sizeof(isp_stat);
        drv_ctx->statistics_buf.node[i].stat_info.virt_addr = (td_void *)(vir_addr + i * sizeof(isp_stat));

        drv_ctx->statistics_buf.node[i].stat_info.stat_key.key = ISP_STATISTICS_KEY;

        osal_list_add_tail(&drv_ctx->statistics_buf.node[i].list, &drv_ctx->statistics_buf.free_list);
    }

    drv_ctx->statistics_buf.init = TD_TRUE;
    drv_ctx->statistics_buf.busy_num = 0;
    drv_ctx->statistics_buf.user_num = 0;
    drv_ctx->statistics_buf.free_num = MAX_ISP_STAT_BUF_NUM;

    *point_phy_addr = drv_ctx->statistics_buf.phy_addr;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (isp_drv_fe_stat_buf_init(vi_pipe) != TD_SUCCESS) {
        isp_err_trace("alloc ISP statistics buf err\n");
        isp_drv_stat_buf_exit(vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_stat_buf_user_get(ot_vi_pipe vi_pipe, isp_stat_info **stat_info)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    struct osal_list_head *plist = TD_NULL;
    isp_stat_node *node = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_stabuf_init_return(vi_pipe, drv_ctx->statistics_buf.init);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if (osal_list_empty(&drv_ctx->statistics_buf.busy_list)) {
        isp_warn_trace("busy list empty\n");
        *stat_info = TD_NULL;
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }

    /* get busy */
    plist = drv_ctx->statistics_buf.busy_list.next;
    if (plist == TD_NULL) {
        isp_warn_trace("busy list empty\n");
        *stat_info = TD_NULL;
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }
    osal_list_del(plist);
    drv_ctx->statistics_buf.busy_num--;

    /* return info */
    node = osal_list_entry(plist, isp_stat_node, list);
    *stat_info = &node->stat_info;

    /* put user */
    osal_list_add_tail(plist, &drv_ctx->statistics_buf.user_list);
    drv_ctx->statistics_buf.user_num++;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_stat_buf_user_put(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    struct osal_list_head *plist = TD_NULL;
    isp_stat_node *node = TD_NULL;
    td_bool valid = TD_FALSE;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_stabuf_init_return(vi_pipe, drv_ctx->statistics_buf.init);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    osal_list_for_each(plist, &drv_ctx->statistics_buf.user_list)
    {
        node = osal_list_entry(plist, isp_stat_node, list);
        if (node == TD_NULL) {
            isp_err_trace("node  null pointer\n");
            break;
        }

        if (node->stat_info.phy_addr == stat_info->phy_addr) {
            valid = TD_TRUE;
            node->stat_info.stat_key.key = stat_info->stat_key.key;
            break;
        }
    }

    if (!valid) {
        isp_err_trace("invalid stat info, please check it\n");
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }

    /* get user */
    if (plist == TD_NULL) {
        isp_err_trace("user list empty\n");
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }
    osal_list_del(plist);
    drv_ctx->statistics_buf.user_num--;

    /* put free */
    osal_list_add_tail(plist, &drv_ctx->statistics_buf.free_list);
    drv_ctx->statistics_buf.free_num++;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_user_stat_buf(ot_vi_pipe vi_pipe, isp_stat_info *stat)
{
    td_s32 ret;
    isp_stat_info *stat_info = TD_NULL;

    isp_check_pointer_return(stat);

    ret = isp_drv_stat_buf_user_get(vi_pipe, &stat_info);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (stat_info == TD_NULL) {
        return TD_FAILURE;
    }

    ret = ot_mmz_check_phys_addr(stat_info->phy_addr, sizeof(isp_stat));
    if (ret != TD_SUCCESS) {
        isp_err_trace("stat addr check error\n");
        return TD_FAILURE;
    }

    (td_void)memcpy_s(stat, sizeof(isp_stat_info), stat_info, sizeof(isp_stat_info));

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_stat_info_active(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_stat_info act_stat_info;
    unsigned long flags = 0;

    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    if (drv_ctx->statistics_buf.act_stat == TD_NULL) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_warn_trace("Pipe[%d] get statistic active buffer err, stat not ready!\n", vi_pipe);
        return TD_FAILURE;
    }

    (td_void)memcpy_s(&act_stat_info, sizeof(isp_stat_info), drv_ctx->statistics_buf.act_stat, sizeof(isp_stat_info));
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    ret = ot_mmz_check_phys_addr(act_stat_info.phy_addr, sizeof(isp_stat));
    if (ret != TD_SUCCESS) {
        isp_err_trace("stat addr check error\n");
        return TD_FAILURE;
    }
    (td_void)memcpy_s(stat_info, sizeof(isp_stat_info), &act_stat_info, sizeof(isp_stat_info));

    return TD_SUCCESS;
}

static td_s32 isp_drv_update_ldci_normal_online_attr(ot_vi_pipe vi_pipe)
{
    td_u8 write_buf_idx;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    write_buf_idx = drv_ctx->ldci_write_buf_attr.buf_idx;

    drv_ctx->ldci_read_buf_attr.buf_idx = write_buf_idx;
    drv_ctx->ldci_write_buf_attr.buf_idx = (write_buf_idx + 1) % div_0_to_1(drv_ctx->ldci_write_buf_attr.buf_num);

    return TD_SUCCESS;
}

static td_s32 isp_drv_update_ldci_tpr_online_attr(ot_vi_pipe vi_pipe, isp_stat *stat)
{
    td_u8 cur_read_buf_idx;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_ldci_stat *ldci_stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    cur_read_buf_idx = drv_ctx->ldci_write_buf_attr.buf_idx;

    /* Get LDCI Statistics from WriteBuffer(copy statistics to stat), then update WriteSttAddr */
    ldci_stat = (isp_ldci_stat *)drv_ctx->ldci_write_buf_attr.ldci_buf[cur_read_buf_idx].vir_addr;

    if (ldci_stat != TD_NULL) {
        (td_void)memcpy_s(&stat->ldci_stat, sizeof(isp_ldci_stat), ldci_stat, sizeof(isp_ldci_stat));
    }
    drv_ctx->ldci_write_buf_attr.buf_idx = (cur_read_buf_idx + 1) % div_0_to_1(drv_ctx->ldci_write_buf_attr.buf_num);

    return TD_SUCCESS;
}

td_s32 isp_drv_ldci_online_attr_update(ot_vi_pipe vi_pipe, isp_stat *stat)
{
    isp_check_pipe_return(vi_pipe);

    if (isp_drv_get_ldci_tpr_flt_en(vi_pipe) == TD_TRUE) {
        /* Copy LDCI statistics information to stat, then update LDCI WriteSttAddr */
        isp_drv_update_ldci_tpr_online_attr(vi_pipe, stat);
    } else {
        /* Only update LDCI ReadSttAddr and WriteSttAddr */
        isp_drv_update_ldci_normal_online_attr(vi_pipe);
    }
    return TD_SUCCESS;
}

td_s32 isp_drv_switch_be_offline_stt_info(ot_vi_pipe vi_pipe, td_u32 viproc_id,
    td_u32 stat_valid)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_viproc_id_return(drv_ctx->work_mode.running_mode, viproc_id);
    if (drv_ctx->isp_init != TD_TRUE) {
        return TD_FAILURE;
    }
    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        return TD_SUCCESS;
    }

    isp_drv_switch_be_offline_stt_buf_index(vi_pipe, viproc_id, drv_ctx, stat_valid);

#ifdef CONFIG_OT_VI_STITCH_GRP
    return isp_drv_stitch_sync_be_stt_info(vi_pipe, viproc_id, drv_ctx);
#else
    return TD_SUCCESS;
#endif
}

td_s32 isp_drv_get_be_offline_stt_addr(ot_vi_pipe vi_pipe, td_u32 viproc_id, td_u64 pts,
                                       isp_be_offline_stt_buf *be_offline_stt_addr)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_offline_stt_addr);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_viproc_id_return(drv_ctx->work_mode.running_mode, viproc_id);
    if (drv_ctx->isp_init != TD_TRUE) {
        return TD_FAILURE;
    }
    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        return TD_FAILURE;
    }

    isp_drv_be_offline_addr_info_update(vi_pipe, viproc_id, pts, be_offline_stt_addr);

    return TD_SUCCESS;
}

static td_s32 isp_drv_fswdr_buf_init(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    td_s32 ret;
    td_u64 size;
    unsigned long flags = 0;
    td_phys_addr_t phys_addr;
    td_u8 *virt_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_buf_init_return(vi_pipe, drv_ctx->fswdr_buf_attr.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].wdr", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    size = WDR_STAT_HOR_NUM * WDR_STAT_VER_NUM * sizeof(td_u32);

    malloc_param.buf_name = ac_name;
    malloc_param.size = size * PING_PONG_NUM;
    malloc_param.kernel_only = TD_TRUE;
    ret = cmpi_mmz_malloc_nocache(&malloc_param, &phys_addr, (td_void **)&virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] MmzMalloc wdr buffer Failure!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(virt_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->fswdr_buf_attr.init = TD_TRUE;
    drv_ctx->fswdr_buf_attr.cur_write_idx = 0;
    for (i = 0; i < PING_PONG_NUM; i++) {
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].size = size;
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].phy_addr = phys_addr + i * size;
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].vir_addr = (td_void *)(virt_addr + i * size);
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_fswdr_buf_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    td_phys_addr_t phys_addr;
    unsigned long flags = 0;
    td_void *virt_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);
    isp_check_buf_exit_return(vi_pipe, drv_ctx->fswdr_buf_attr.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phys_addr = drv_ctx->fswdr_buf_attr.fswdr_buf[0].phy_addr;
    virt_addr = drv_ctx->fswdr_buf_attr.fswdr_buf[0].vir_addr;
    drv_ctx->fswdr_buf_attr.init = TD_FALSE;
    drv_ctx->fswdr_buf_attr.cur_write_idx = 0;
    for (i = 0; i < PING_PONG_NUM; i++) {
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].size = 0;
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].phy_addr = 0;
        drv_ctx->fswdr_buf_attr.fswdr_buf[i].vir_addr = TD_NULL;
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phys_addr != 0) {
        cmpi_mmz_free(phys_addr, virt_addr);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_drc_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_buf_init_return(vi_pipe, drv_ctx->drc_buf_attr.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].drc", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    malloc_param.buf_name = ac_name;
    malloc_param.size = ISP_DRC_BUF_SIZE;
    malloc_param.kernel_only = TD_TRUE;
    ret = cmpi_mmz_malloc_nocache(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] MmzMalloc drc buffer Failure!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);

    drv_ctx->drc_buf_attr.init = TD_TRUE;
    drv_ctx->drc_buf_attr.drc_buf.size = malloc_param.size;
    drv_ctx->drc_buf_attr.drc_buf.phy_addr = phy_addr;
    drv_ctx->drc_buf_attr.drc_buf.vir_addr = (td_void *)vir_addr;

    return TD_SUCCESS;
}

static td_s32 isp_drv_drc_buf_exit(ot_vi_pipe vi_pipe)
{
    td_phys_addr_t phy_addr;
    td_void *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);
    isp_check_buf_exit_return(vi_pipe, drv_ctx->drc_buf_attr.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phy_addr = drv_ctx->drc_buf_attr.drc_buf.phy_addr;
    vir_addr = drv_ctx->drc_buf_attr.drc_buf.vir_addr;

    drv_ctx->drc_buf_attr.init = TD_FALSE;
    drv_ctx->drc_buf_attr.drc_buf.size = 0;
    drv_ctx->drc_buf_attr.drc_buf.phy_addr = 0;
    drv_ctx->drc_buf_attr.drc_buf.vir_addr = TD_NULL;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_ldci_buf_attr_reset(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 i, be_buf_num;

    drv_ctx->ldci_read_buf_attr.init = TD_FALSE;
    drv_ctx->ldci_write_buf_attr.init = TD_FALSE;
    drv_ctx->ldci_read_buf_attr.buf_num = 0;
    drv_ctx->ldci_write_buf_attr.buf_num = 0;
    be_buf_num = isp_drv_get_be_buf_num(vi_pipe);
    for (i = 0; i < be_buf_num; i++) {
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].phy_addr = 0;
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].vir_addr = TD_NULL;
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].size = 0;
    }

    for (i = 0; i < be_buf_num; i++) {
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].phy_addr = 0;
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].vir_addr = TD_NULL;
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].size = 0;
    }
}

static td_void isp_drv_ldci_tpr_buf_attr_init(isp_drv_ctx *drv_ctx, td_u8 write_buf_num, td_u8 read_buf_num,
    td_phys_addr_t phy_addr, td_u8 *vir_addr)
{
    td_u8 i, j;
    td_u64 size = sizeof(isp_ldci_stat);

    drv_ctx->ldci_write_buf_attr.init    = TD_TRUE;
    drv_ctx->ldci_write_buf_attr.buf_num = write_buf_num;
    drv_ctx->ldci_write_buf_attr.buf_idx = 0;

    drv_ctx->ldci_read_buf_attr.init = TD_TRUE;
    drv_ctx->ldci_read_buf_attr.buf_num = read_buf_num;
    drv_ctx->ldci_read_buf_attr.buf_idx = 0;

    for (i = 0; i < write_buf_num; i++) {
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].phy_addr = phy_addr + i * size;
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].vir_addr = (td_void *)(vir_addr + i * size);
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].size = size;
    }

    for (i = 0; i < read_buf_num; i++) {
        j = i + write_buf_num;
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].phy_addr = phy_addr + j * size;
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].vir_addr = (td_void *)(vir_addr + j * size);
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].size = size;
    }
}

static td_s32 isp_drv_ldci_tpr_buf_malloc(ot_vi_pipe vi_pipe, td_phys_addr_t *ldci_phy_addr, td_u8 **ldci_vir_addr)
{
    td_u8 write_buf_num, read_buf_num, buf_num_all;
    td_u8 *vir_addr = TD_NULL;
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_u64 size;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].ldci", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_ldci_stat);

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        write_buf_num = ISP_ONLINE_LDCI_TPR_BUF_NUM;
        read_buf_num = ISP_ONLINE_LDCI_TPR_BUF_NUM;
    } else {
        write_buf_num = isp_drv_get_be_buf_num(vi_pipe);
        read_buf_num = write_buf_num;
    }

    buf_num_all = write_buf_num + read_buf_num;

    malloc_param.buf_name = ac_name;
    malloc_param.size = size * buf_num_all;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_nocache(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] MmzMalloc Ldci buffer Failure!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    isp_drv_ldci_tpr_buf_attr_init(drv_ctx, write_buf_num, read_buf_num, phy_addr, vir_addr);
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    *ldci_phy_addr = phy_addr;
    *ldci_vir_addr = vir_addr;

    return TD_SUCCESS;
}

static td_s32 isp_drv_ldci_tpr_buf_init(ot_vi_pipe vi_pipe)
{
    td_u8 *vir_addr = TD_NULL;
    td_s32 ret;
    td_phys_addr_t phy_addr;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_buf_init_return(vi_pipe, drv_ctx->ldci_read_buf_attr.init);
    isp_check_buf_init_return(vi_pipe, drv_ctx->ldci_write_buf_attr.init);

    ret = isp_drv_ldci_tpr_buf_malloc(vi_pipe, &phy_addr, &vir_addr);
    isp_check_return(vi_pipe, ret, "isp_drv_ldci_tpr_buf_malloc");

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_set_ldci_stt_addr(vi_pipe, drv_ctx->ldci_read_buf_attr.ldci_buf[0].phy_addr,
            drv_ctx->ldci_write_buf_attr.ldci_buf[0].phy_addr);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] Set Ldci Param/OutParam addr Err!\n", vi_pipe);
            goto fail0;
        }

        /* update Write Index */
        drv_ctx->ldci_write_buf_attr.buf_idx =
            (drv_ctx->ldci_write_buf_attr.buf_idx + 1) % div_0_to_1(drv_ctx->ldci_write_buf_attr.buf_num);
    }

    return TD_SUCCESS;

fail0:
    isp_drv_ldci_buf_attr_reset(vi_pipe, drv_ctx);
    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, (td_void *)vir_addr);
    }
    return TD_FAILURE;
}

static td_s32 isp_drv_ldci_tpr_buf_exit(ot_vi_pipe vi_pipe)
{
    td_phys_addr_t phy_addr;
    td_void *vir_addr = TD_NULL;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_exit_return(vi_pipe, drv_ctx->ldci_read_buf_attr.init);
    isp_check_buf_exit_return(vi_pipe, drv_ctx->ldci_write_buf_attr.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phy_addr = drv_ctx->ldci_write_buf_attr.ldci_buf[0].phy_addr;
    vir_addr = drv_ctx->ldci_write_buf_attr.ldci_buf[0].vir_addr;
    isp_drv_ldci_buf_attr_reset(vi_pipe, drv_ctx);
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_ldci_normal_buf_attr_init(isp_drv_ctx *drv_ctx, td_u8 buf_num,
    td_phys_addr_t phy_addr, td_u64 size)
{
    td_u8 i;
    drv_ctx->ldci_read_buf_attr.init = TD_TRUE;
    drv_ctx->ldci_read_buf_attr.buf_num = buf_num;
    drv_ctx->ldci_read_buf_attr.buf_idx = 0;

    drv_ctx->ldci_write_buf_attr.init = TD_TRUE;
    drv_ctx->ldci_write_buf_attr.buf_num = buf_num;
    drv_ctx->ldci_write_buf_attr.buf_idx = MIN2(buf_num - 1, drv_ctx->ldci_read_buf_attr.buf_idx + 1);

    for (i = 0; i < buf_num; i++) {
        drv_ctx->ldci_read_buf_attr.ldci_buf[i].phy_addr = phy_addr + i * size;
        drv_ctx->ldci_write_buf_attr.ldci_buf[i].phy_addr = phy_addr + i * size;
    }
}

static td_s32 isp_drv_ldci_normal_buf_init(ot_vi_pipe vi_pipe)
{
    td_u8 buf_num;
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_u64 size;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->ldci_read_buf_attr.init);
    isp_check_buf_init_return(vi_pipe, drv_ctx->ldci_write_buf_attr.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].ldci", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }
    size = sizeof(isp_ldci_stat);

    buf_num = is_striping_mode(drv_ctx->work_mode.running_mode) ? ISP_STRIPING_LDCI_NORMAL_BUF_NUM : 1;

    malloc_param.buf_name = ac_name;
    malloc_param.size = size * buf_num;
    malloc_param.kernel_only = TD_FALSE;
    phy_addr = cmpi_mmz_malloc(&malloc_param);
    if (phy_addr == 0) {
        isp_err_trace("alloc ISP[%d] Ldci buf err\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    isp_drv_ldci_normal_buf_attr_init(drv_ctx, buf_num, phy_addr, size);
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_set_ldci_stt_addr(vi_pipe, phy_addr, phy_addr);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] Set Ldci stt addr Err!\n", vi_pipe);
            goto fail0;
        }

        /* update Write & Read Index */
        drv_ctx->ldci_read_buf_attr.buf_idx = drv_ctx->ldci_write_buf_attr.buf_idx;
        drv_ctx->ldci_write_buf_attr.buf_idx =
            (drv_ctx->ldci_write_buf_attr.buf_idx + 1) % div_0_to_1(drv_ctx->ldci_write_buf_attr.buf_num);
    }

    return TD_SUCCESS;

fail0:
    isp_drv_ldci_buf_attr_reset(vi_pipe, drv_ctx);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, TD_NULL);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_ldci_normal_buf_exit(ot_vi_pipe vi_pipe)
{
    td_phys_addr_t phy_addr;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags = 0;

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_buf_exit_return(vi_pipe, drv_ctx->ldci_read_buf_attr.init);
    isp_check_buf_exit_return(vi_pipe, drv_ctx->ldci_write_buf_attr.init);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phy_addr = drv_ctx->ldci_write_buf_attr.ldci_buf[0].phy_addr;
    isp_drv_ldci_buf_attr_reset(vi_pipe, drv_ctx);
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, TD_NULL);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_ldci_buf_init(ot_vi_pipe vi_pipe)
{
    isp_check_pipe_return(vi_pipe);

    if (isp_drv_get_ldci_tpr_flt_en(vi_pipe) == TD_TRUE) {
        return isp_drv_ldci_tpr_buf_init(vi_pipe);
    } else {
        return isp_drv_ldci_normal_buf_init(vi_pipe);
    }
}

static td_s32 isp_drv_ldci_buf_exit(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    if (isp_drv_get_ldci_tpr_flt_en(vi_pipe) == TD_TRUE) {
        return isp_drv_ldci_tpr_buf_exit(vi_pipe);
    } else {
        return isp_drv_ldci_normal_buf_exit(vi_pipe);
    }

    return TD_FAILURE;
}

td_s32 isp_drv_ldci_read_stt_buf_get(ot_vi_pipe vi_pipe, isp_ldci_read_stt_buf *ldci_read_stt_buf)
{
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ldci_read_stt_buf);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    ldci_read_stt_buf->buf_num = drv_ctx->ldci_read_buf_attr.buf_num;
    ldci_read_stt_buf->buf_idx = 0;
    ldci_read_stt_buf->head_phy_addr = drv_ctx->ldci_read_buf_attr.ldci_buf[0].phy_addr;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

td_s32 isp_drv_clut_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = { 0 };
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_check_buf_init_return(vi_pipe, drv_ctx->clut_buf_attr.init);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].clut", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    malloc_param.buf_name = ac_name;
    malloc_param.size = OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32);
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_nocache(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] MmzMalloc Clut buffer Failure!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);

    drv_ctx->clut_buf_attr.init = TD_TRUE;
    drv_ctx->clut_buf_attr.clut_buf.size = malloc_param.size;
    drv_ctx->clut_buf_attr.clut_buf.phy_addr = phy_addr;
    drv_ctx->clut_buf_attr.clut_buf.vir_addr = (td_void *)vir_addr;

    return TD_SUCCESS;
}

td_s32 isp_drv_clut_buf_exit(ot_vi_pipe vi_pipe)
{
    td_phys_addr_t phy_addr;
    td_void *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    isp_check_buf_exit_return(vi_pipe, drv_ctx->clut_buf_attr.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phy_addr = drv_ctx->clut_buf_attr.clut_buf.phy_addr;
    vir_addr = drv_ctx->clut_buf_attr.clut_buf.vir_addr;

    drv_ctx->clut_buf_attr.init = TD_FALSE;
    drv_ctx->clut_buf_attr.clut_buf.size = 0;
    drv_ctx->clut_buf_attr.clut_buf.phy_addr = 0;
    drv_ctx->clut_buf_attr.clut_buf.vir_addr = TD_NULL;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_fe_lut_stt_buf_get(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_unused(cmd);
    // need to check pipe is valid or not first, then to check it is no fe pipe or not
    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->fe_lut2stt_attr.init != TD_TRUE) {
        return OT_ERR_ISP_NOMEM;
    }

    *(td_phys_addr_t *)arg = drv_ctx->fe_lut2stt_attr.lut_stt_buf[0].phy_addr;
    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_u64 isp_drv_get_extend_cfg_size(isp_drv_ctx *drv_ctx)
{
    td_u8 blk_num;
    td_u64 extend_cfg_size = 0;

    blk_num = drv_ctx->detail_stats_cfg.col * drv_ctx->detail_stats_cfg.row;
    if (drv_ctx->detail_stats_cfg.ctrl.bit1_ae) {
        extend_cfg_size += ISP_DETAIL_STATS_AE_CFG_SIZE;
    }
    if (drv_ctx->detail_stats_cfg.ctrl.bit1_awb) {
        extend_cfg_size += ISP_DETAIL_STATS_AWB_CFG_SIZE;
    }
    extend_cfg_size = extend_cfg_size * blk_num;
    return extend_cfg_size;
}
#endif
static td_s32 isp_drv_be_buf_malloc(ot_vi_pipe vi_pipe, isp_mmz_buf_ex *be_buf_temp, td_u64 *extend_size)
{
    td_u8 be_buf_num;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u64 size, extend_cfg_size;
    td_u8 *vir_addr = TD_NULL;
    td_char ac_name[MAX_MMZ_NAME_LEN] = {0};
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    mm_malloc_param malloc_param = {0};

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    ret = snprintf_s(ac_name, sizeof(ac_name), sizeof(ac_name) - 1, "isp[%d].be_cfg", vi_pipe);
    if (ret < 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_wo_reg_cfg);

    extend_cfg_size = 0;
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    if (drv_ctx->detail_stats_cfg.enable) {
        extend_cfg_size = isp_drv_get_extend_cfg_size(drv_ctx);
    }
    size += extend_cfg_size;
#endif
    be_buf_num = isp_drv_get_be_buf_num(vi_pipe);

    malloc_param.buf_name = ac_name;
    malloc_param.size = size * be_buf_num;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_cached(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe[%d] alloc ISP BeCfgBuf err!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    (td_void)memset_s(vir_addr, malloc_param.size, 0, malloc_param.size);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    drv_ctx->be_buf_info.init = TD_TRUE;
    drv_ctx->be_buf_info.be_buf_haddr.phy_addr = phy_addr;
    drv_ctx->be_buf_info.be_buf_haddr.vir_addr = (td_void *)vir_addr;
    drv_ctx->be_buf_info.be_buf_haddr.size = malloc_param.size; // all_size : size * be_buf_num

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    be_buf_temp->phy_addr = phy_addr;
    be_buf_temp->vir_addr = (td_void *)vir_addr;
    be_buf_temp->size = size;
    *extend_size = extend_cfg_size;
    return TD_SUCCESS;
}

static td_void isp_drv_be_buf_info_reset(isp_drv_ctx *drv_ctx)
{
    drv_ctx->be_buf_info.init = TD_FALSE;
    drv_ctx->be_buf_info.be_buf_haddr.phy_addr = 0;
    drv_ctx->be_buf_info.be_buf_haddr.vir_addr = TD_NULL;
    drv_ctx->be_buf_info.be_buf_haddr.size = 0;
    return;
}

static td_s32 isp_drv_be_buf_init(ot_vi_pipe vi_pipe, isp_mmz_buf_ex *be_cfg_buf_info)
{
    td_u8 be_buf_num;
    td_s32 ret, i;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_buf_node *node = TD_NULL;
    td_u64 extend_size;
    isp_mmz_buf_ex be_buf_temp = {0};
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_cfg_buf_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);

    isp_check_buf_init_return(vi_pipe, drv_ctx->be_buf_info.init);

    ret = isp_drv_be_buf_malloc(vi_pipe, &be_buf_temp, &extend_size);
    isp_check_return(vi_pipe, ret, "isp_drv_be_buf_malloc");
    be_buf_num = isp_drv_get_be_buf_num(vi_pipe);
    ret = isp_creat_be_buf_queue(&drv_ctx->be_buf_queue, be_buf_num);
    isp_check_ret_goto(ret, ret, fail0, "vi_pipe[%d] creat be buf queue fail!\n", vi_pipe);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    for (i = 0; i < be_buf_num; i++) {
        node = isp_queue_get_free_be_buf(&drv_ctx->be_buf_queue);
        if (node == TD_NULL) {
            isp_err_trace("vi_pipe[%d] queue get free be buf fail!\r\n", vi_pipe);
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
            goto fail1;
        }
        isp_drv_be_cfg_buf_addr_init(vi_pipe, node, i, &be_buf_temp, extend_size);
        isp_queue_put_free_be_buf(&drv_ctx->be_buf_queue, node);
    }

    drv_ctx->use_node = TD_NULL;
    drv_ctx->running_state = ISP_BE_BUF_STATE_INIT;
    drv_ctx->exit_state = ISP_BE_BUF_READY;

    be_cfg_buf_info->phy_addr = drv_ctx->be_buf_info.be_buf_haddr.phy_addr;
    be_cfg_buf_info->size = drv_ctx->be_buf_info.be_buf_haddr.size; // all_size : size * be_buf_num
    be_cfg_buf_info->vir_addr = TD_NULL;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;

fail1:
    isp_destroy_be_buf_queue(&drv_ctx->be_buf_queue);

fail0:
    isp_drv_be_buf_info_reset(drv_ctx);

    if (be_buf_temp.phy_addr != 0) {
        cmpi_mmz_free(be_buf_temp.phy_addr, (td_void *)be_buf_temp.vir_addr);
    }

    return TD_FAILURE;
}

td_s32 isp_drv_be_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_phys_addr_t phy_addr;
    td_void *vir_addr = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    isp_check_buf_exit_return(vi_pipe, drv_ctx->be_buf_info.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->exit_state = ISP_BE_BUF_WAITING;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    if (check_func_entry(OT_ID_VI) && ckfn_vi_update_vi_vpss_mode()) {
        /* Note: this function cannot be placed in the ISP lock, otherwise it will be deadlocked. */
        call_vi_isp_clear_input_queue(vi_pipe);
    }

    ret = osal_wait_timeout_uninterruptible(&drv_ctx->isp_exit_wait, isp_drv_wait_exit_callback,
        &drv_ctx->be_buf_info.use_cnt, g_isp_exit_timeout);
    if (ret <= 0) {
        isp_err_trace("Pipe:%d isp exit wait failed:ret:%d!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    phy_addr = drv_ctx->be_buf_info.be_buf_haddr.phy_addr;
    vir_addr = drv_ctx->be_buf_info.be_buf_haddr.vir_addr;
    isp_destroy_be_buf_queue(&drv_ctx->be_buf_queue);
    isp_drv_be_buf_info_reset(drv_ctx);
    drv_ctx->exit_state = ISP_BE_BUF_EXIT;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, vir_addr);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_write_all_ldci_stt_addr(ot_vi_pipe vi_pipe)
{
    td_u8 i, k, write_buf_idx, blk_num, free_num, write_buf_num;
    td_u32 size;
    td_phys_addr_t write_stt_head_addr, write_stt_addr;
    isp_be_wo_reg_cfg *be_reg_cfg = TD_NULL;
    isp_be_buf_node *node = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    blk_num = drv_ctx->work_mode.block_num;
    size = drv_ctx->ldci_write_buf_attr.ldci_buf[0].size;
    write_buf_num = drv_ctx->ldci_write_buf_attr.buf_num;
    write_buf_idx = drv_ctx->ldci_write_buf_attr.buf_idx;
    write_stt_head_addr = drv_ctx->ldci_write_buf_attr.ldci_buf[write_buf_idx].phy_addr;

    be_reg_cfg = (isp_be_wo_reg_cfg *)drv_ctx->use_node->be_cfg_buf.vir_addr;

    for (i = 0; i < blk_num; i++) {
        write_stt_addr = write_stt_head_addr + i * (size / div_0_to_1(blk_num));
        isp_drv_set_ldci_write_stt_addr_offline(&be_reg_cfg->be_reg_cfg[i], write_stt_addr);
    }

    drv_ctx->ldci_write_buf_attr.buf_idx = (write_buf_idx + 1) % div_0_to_1(write_buf_num);

    free_num = isp_queue_get_free_num(&drv_ctx->be_buf_queue);

    for (k = 0; k < free_num; k++) {
        node = isp_queue_get_free_be_buf(&drv_ctx->be_buf_queue);
        if (node == TD_NULL) {
            isp_err_trace("ISP[%d] Get QueueGetFreeBeBuf fail!\r\n", vi_pipe);
            return TD_FAILURE;
        }

        be_reg_cfg = (isp_be_wo_reg_cfg *)node->be_cfg_buf.vir_addr;
        write_buf_idx = drv_ctx->ldci_write_buf_attr.buf_idx;
        write_stt_head_addr = drv_ctx->ldci_write_buf_attr.ldci_buf[write_buf_idx].phy_addr;

        for (i = 0; i < blk_num; i++) {
            write_stt_addr = write_stt_head_addr + i * (size / div_0_to_1(blk_num));
            isp_drv_set_ldci_write_stt_addr_offline(&be_reg_cfg->be_reg_cfg[i], write_stt_addr);
        }

        drv_ctx->ldci_write_buf_attr.buf_idx = (write_buf_idx + 1) % div_0_to_1(write_buf_num);
        isp_queue_put_free_be_buf(&drv_ctx->be_buf_queue, node);
    }

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_void isp_drv_copy_extend_to_free_buf(isp_drv_ctx *drv_ctx, isp_be_buf_node *node)
{
    td_s32 ret;
    if (drv_ctx->detail_stats_cfg.enable) {
        ret = memcpy_s((td_u8 *)node->be_cfg_buf.extend_vir_addr, node->be_cfg_buf.extend_size,
            (td_u8 *)drv_ctx->use_node->be_cfg_buf.extend_vir_addr, drv_ctx->use_node->be_cfg_buf.extend_size);
        isp_check_eok_void(ret);
    }
}
#endif
static td_s32 isp_drv_write_be_free_buf(ot_vi_pipe vi_pipe)
{
    td_s32 i, free_num, ret;
    isp_running_mode running_mode;
    isp_be_buf_node *node = TD_NULL;
    isp_be_wo_reg_cfg *be_reg_cfg_src = TD_NULL;
    isp_be_wo_reg_cfg *be_reg_cfg_dst = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    td_u64 size;
    td_void *vir_addr = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_bebuf_init_return(vi_pipe, drv_ctx->be_buf_info.init);

    if (drv_ctx->use_node == TD_NULL) {
        isp_err_trace("Pipe[%d] pstCurNode is null for init!\r\n", vi_pipe);
        return TD_FAILURE;
    }

    be_reg_cfg_src = drv_ctx->use_node->be_cfg_buf.vir_addr;
    running_mode = drv_ctx->work_mode.running_mode;

    free_num = isp_queue_get_free_num(&drv_ctx->be_buf_queue);

    for (i = 0; i < free_num; i++) {
        node = isp_queue_get_free_be_buf(&drv_ctx->be_buf_queue);
        if (node == TD_NULL) {
            isp_err_trace("Pipe[%d] Get QueueGetFreeBeBuf fail!\r\n", vi_pipe);
            return TD_FAILURE;
        }
        be_reg_cfg_dst = (isp_be_wo_reg_cfg *)node->be_cfg_buf.vir_addr;

        if ((running_mode == ISP_MODE_RUNNING_SIDEBYSIDE) || (running_mode == ISP_MODE_RUNNING_STRIPING)) {
            (td_void)memcpy_s(be_reg_cfg_dst, sizeof(isp_be_wo_reg_cfg), be_reg_cfg_src, sizeof(isp_be_wo_reg_cfg));
        } else {
            (td_void)memcpy_s(&be_reg_cfg_dst->be_reg_cfg[0], sizeof(isp_be_all_reg_type),
                &be_reg_cfg_src->be_reg_cfg[0], sizeof(isp_be_all_reg_type));
        }
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
        isp_drv_copy_extend_to_free_buf(drv_ctx, node);
#endif
        phy_addr = drv_ctx->use_node->be_cfg_buf.phy_addr;
        vir_addr = drv_ctx->use_node->be_cfg_buf.vir_addr;
        size = drv_ctx->use_node->be_cfg_buf.size;

        cmpi_dcache_region_wb(vir_addr, phy_addr, size);

        isp_queue_put_free_be_buf(&drv_ctx->be_buf_queue, node);
    }

    ret = isp_drv_write_all_ldci_stt_addr(vi_pipe);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_STITCH_GRP
static td_s32 isp_drv_stitch_sync_ex(ot_vi_pipe vi_pipe)
{
    td_u8 k;
    ot_vi_pipe vi_pipe_id;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_drv_ctx *drv_ctx_s = TD_NULL;

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        vi_pipe_id = drv_ctx->stitch_attr.stitch_bind_id[k];
        drv_ctx_s = isp_drv_get_ctx(vi_pipe_id);
        if (drv_ctx_s->stitch_sync != TD_TRUE) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_stitch_write_be_buf_all(ot_vi_pipe vi_pipe)
{
    td_s32 i, ret;
    ot_vi_pipe vi_pipes, main_pipe;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_drv_ctx *drv_ctx_s = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_sync_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    main_pipe = drv_ctx->stitch_attr.stitch_bind_id[0];

    ret = isp_drv_write_be_free_buf(vi_pipe);
    isp_check_return(vi_pipe, ret, "isp_drv_write_be_free_buf");
    isp_sync_lock = isp_drv_get_sync_lock(main_pipe);
    osal_spin_lock_irqsave(isp_sync_lock, &flags);

    ret = isp_drv_stitch_sync_ex(vi_pipe);
    if (ret != TD_SUCCESS) {
        osal_spin_unlock_irqrestore(isp_sync_lock, &flags);

        return TD_SUCCESS;
    }

    if (drv_ctx->running_state == ISP_BE_BUF_STATE_SWITCH_START) {
        drv_ctx->running_state = ISP_BE_BUF_STATE_SWITCH;
        for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
            vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
            drv_ctx_s = isp_drv_get_ctx(vi_pipes);
            if (drv_ctx_s->running_state != ISP_BE_BUF_STATE_SWITCH) {
                osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
                drv_ctx->running_state = ISP_BE_BUF_STATE_SWITCH_START;
                return TD_SUCCESS;
            }
        }
        drv_ctx->running_state = ISP_BE_BUF_STATE_SWITCH_START;
    }

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
        drv_ctx_s = isp_drv_get_ctx(vi_pipes);
        if ((drv_ctx_s->be_buf_info.init != TD_TRUE) || (drv_ctx_s->use_node == TD_NULL)) {
            osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
            isp_err_trace("Pipe[%d] BeBuf (bInit != TRUE) or use_node is TD_NULL!\n", vi_pipe);
            return TD_FAILURE;
        }

        isp_queue_put_busy_be_buf(&drv_ctx_s->be_buf_queue, drv_ctx_s->use_node);
        drv_ctx_s->use_node = TD_NULL;
        drv_ctx_s->running_state = ISP_BE_BUF_STATE_INIT;
    }

    osal_spin_unlock_irqrestore(isp_sync_lock, &flags);

    return TD_SUCCESS;
}
#endif

static td_s32 isp_drv_all_be_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);

    if (drv_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        if (drv_ctx->be_buf_info.init != TD_TRUE) {
            isp_err_trace("Pipe[%d] BeBuf (bInit != TRUE) !\n", vi_pipe);
            return TD_FAILURE;
        }
        isp_spin_lock = isp_drv_get_lock(vi_pipe);
        osal_spin_lock_irqsave(isp_spin_lock, &flags);

        ret = isp_drv_write_be_free_buf(vi_pipe);
        if (ret != TD_SUCCESS) {
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

            isp_err_trace("Pipe[%d] ISP_DRV_WriteBeFreeBuf fail!\n", vi_pipe);
            return ret;
        }

        isp_queue_put_busy_be_buf(&drv_ctx->be_buf_queue, drv_ctx->use_node);
        drv_ctx->use_node = TD_NULL;

        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    } else {
#ifdef CONFIG_OT_VI_STITCH_GRP
        ret = isp_drv_stitch_write_be_buf_all(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Pipe[%d] ISP_DRV_StitchWriteBeBufAll fail!\n", vi_pipe);
            return ret;
        }
#endif
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_be_buf_first(ot_vi_pipe vi_pipe, td_phys_addr_t *point_phy_addr)
{
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_buf_node *node = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(point_phy_addr);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_bebuf_init_return(vi_pipe, drv_ctx->be_buf_info.init);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    node = isp_queue_get_free_be_buf(&drv_ctx->be_buf_queue);
    if (node == TD_NULL) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

        isp_err_trace("Pipe[%d] Get FreeBeBuf to user fail!\r\n", vi_pipe);
        return TD_FAILURE;
    }

    drv_ctx->use_node = node;

    *point_phy_addr = drv_ctx->use_node->be_cfg_buf.phy_addr;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_be_free_buf(ot_vi_pipe vi_pipe, isp_be_wo_cfg_buf *be_wo_cfg_buf)
{
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_wo_cfg_buf *cur_node_buf = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_wo_cfg_buf);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_bebuf_init_return(vi_pipe, drv_ctx->be_buf_info.init);

    isp_spin_lock = isp_drv_get_spin_lock(vi_pipe);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if (drv_ctx->use_node == TD_NULL) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }

    cur_node_buf = &drv_ctx->use_node->be_cfg_buf;
    (td_void)memcpy_s(be_wo_cfg_buf, sizeof(isp_be_wo_cfg_buf), cur_node_buf, sizeof(isp_be_wo_cfg_buf));

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}
static td_s32 isp_drv_get_be_last_buf(ot_vi_pipe vi_pipe, td_phys_addr_t *point_phy_addr)
{
    td_u8 i;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_buf_node *node = TD_NULL;
    isp_be_wo_reg_cfg *be_reg_cfg_dst = TD_NULL;
    struct osal_list_head *list_tmp = TD_NULL;
    struct osal_list_head *list_node = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(point_phy_addr);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_bebuf_init_return(vi_pipe, drv_ctx->be_buf_info.init);

    isp_spin_lock = isp_drv_get_spin_lock(vi_pipe);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    osal_list_for_each_safe(list_node, list_tmp, &drv_ctx->be_buf_queue.busy_list)
    {
        node = osal_list_entry(list_node, isp_be_buf_node, list);

        node->hold_cnt = 0;

        isp_queue_del_busy_be_buf(&drv_ctx->be_buf_queue, node);
        isp_queue_put_free_be_buf(&drv_ctx->be_buf_queue, node);
    }

    if (drv_ctx->use_node == TD_NULL) {
        node = isp_queue_get_free_be_buf_tail(&drv_ctx->be_buf_queue);
        if (node == TD_NULL) {
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
            isp_err_trace("Pipe[%d] Get LastBeBuf fail!\r\n", vi_pipe);
            return TD_FAILURE;
        }

        drv_ctx->use_node = node;
    }

    be_reg_cfg_dst = (isp_be_wo_reg_cfg *)drv_ctx->use_node->be_cfg_buf.vir_addr;

    for (i = drv_ctx->work_mode.pre_block_num; i < drv_ctx->work_mode.block_num; i++) {
        (td_void)memcpy_s(&be_reg_cfg_dst->be_reg_cfg[i], sizeof(isp_be_all_reg_type), &be_reg_cfg_dst->be_reg_cfg[0],
            sizeof(isp_be_all_reg_type));
    }

    *point_phy_addr = drv_ctx->use_node->be_cfg_buf.phy_addr;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_set_reg_kernel_cfgs(ot_vi_pipe vi_pipe, isp_kernel_reg_cfg *reg_kernel_cfg)
{
    td_u32 flag;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(reg_kernel_cfg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if ((drv_ctx->reg_cfg_info_flag != 0) && (drv_ctx->reg_cfg_info_flag != 1)) {
        isp_err_trace("Pipe[%d] Err reg_cfg_info_flag != 0/1 !!!\n", vi_pipe);
    }

    flag = 1 - drv_ctx->reg_cfg_info_flag;
    (td_void)memcpy_s(&drv_ctx->kernel_cfg[flag], sizeof(isp_kernel_reg_cfg), reg_kernel_cfg,
        sizeof(isp_kernel_reg_cfg));
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->reg_cfg_info_flag = flag;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_chn_select_write(ot_vi_pipe vi_pipe, td_u32 channel_sel)
{
    td_u8 i;
    td_u32 chn_switch[ISP_CHN_SWITCH_NUM] = { 0 };
    isp_chn_sel chn_sel;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    chn_switch[4] = drv_ctx->yuv_mode ? 1 : 0; /* register[4] value 1 means yuv mode */

    for (i = 0; i < ISP_CHN_SWITCH_NUM - 1; i++) {
        chn_switch[i] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[i];
    }

    chn_sel = (isp_chn_sel)(channel_sel & 0x3);
    switch (chn_sel) {
        case ISP_CHN_SWITCH_NORMAL:
            break;

        case ISP_CHN_SWITCH_2LANE:
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0];
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1];
            break;

        case ISP_CHN_SWITCH_3LANE:
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 2 */
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1];
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 2 */
            break;

        case ISP_CHN_SWITCH_4LANE:
            chn_switch[3] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 3 */
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1]; /* array index 2 */
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 2 */
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[3]; /* array index 3 */
            break;

        default:
            break;
    }

    if (is_fs_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        isp_drv_set_input_sel(vi_pipe, &chn_switch[0], sizeof(chn_switch));
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_chn_select_cfg(ot_vi_pipe vi_pipe, td_u32 chn_sel)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
        drv_ctx->chn_sel_attr[i].channel_sel = chn_sel;
    }

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_chn_select_write(vi_pipe, chn_sel);
        if (ret != TD_SUCCESS) {
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
            isp_err_trace("isp[%d] ChnSelect Write err!\n", vi_pipe);
            return ret;
        }
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return ret;
}

static td_s32 isp_drv_get_nr_mode(ot_vi_pipe vi_pipe, td_bool *nr_en)
{
    if (!ckfn_vi_is_ia_nr_en()) {
        return TD_FAILURE;
    }

    *nr_en = call_vi_is_ia_nr_en(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_drv_bnr_temporal_filt_set(ot_vi_pipe vi_pipe, isp_bnr_temporal_filt *tpr_filt)
{
    td_s32 ret;
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(tpr_filt);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->bnr_tpr_filt.cur.nr_en = tpr_filt->nr_en;
    drv_ctx->bnr_tpr_filt.cur.tnr_en = tpr_filt->tnr_en;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (ckfn_vi_set_pipe_bnr_en() != TD_NULL) {
        ret = call_vi_set_pipe_bnr_en(vi_pipe, tpr_filt->nr_en && tpr_filt->tnr_en);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] call_vi_set_pipe_bnr_en Failed!\n", vi_pipe);
            return ret;
        }
    } else {
        isp_err_trace("ISP[%d] vi_set_pipe_bnr_en is TD_NULL\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

#ifdef IQ_DEBUG
static td_s32 isp_drv_bnr_mot_bitw_set(ot_vi_pipe vi_pipe, td_u8 *bitw)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    if (ckfn_vi_set_pipe_bnr_mot_bitw() != TD_NULL) {
        ret = call_vi_set_pipe_bnr_mot_bitw(vi_pipe, *bitw);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] call_vi_set_pipe_bnr_mot_bitw Failed!\n", vi_pipe);
            return ret;
        }
    } else {
        isp_err_trace("ISP[%d] ckfn_vi_set_pipe_bnr_mot_bitw is TD_NULL\n", vi_pipe);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}
#endif

static td_s32 isp_drv_sharpen_mot_set(ot_vi_pipe vi_pipe, td_bool *mot_en)
{
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(mot_en);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->bnr_tpr_filt.sharpen_mot_en = *mot_en;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_sync_cfg_set(ot_vi_pipe vi_pipe, isp_sync_cfg_buf_node *sync_cfg_buf_node)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_sync_cfg_buf *sync_cfg_buf = TD_NULL;
    isp_sync_cfg_buf_node *cur_node = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(sync_cfg_buf_node);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    sync_cfg_buf = &drv_ctx->sync_cfg.sync_cfg_buf;

    if (isp_sync_buf_is_full(sync_cfg_buf)) {
        isp_err_trace("Pipe[%d] isp sync buffer is full\n", vi_pipe);
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return TD_FAILURE;
    }

    if ((sync_cfg_buf_node->sns_regs_info.sns_type >= OT_ISP_SNS_TYPE_BUTT) ||
        (sync_cfg_buf_node->ae_reg_cfg.fs_wdr_mode >= OT_ISP_FSWDR_MODE_BUTT) ||
        (sync_cfg_buf_node->sns_regs_info.cfg2_valid_delay_max > CFG2VLD_DLY_LIMIT) ||
        (sync_cfg_buf_node->sns_regs_info.cfg2_valid_delay_max < 1) ||
        (sync_cfg_buf_node->sns_regs_info.slv_sync.delay_frame_num > CFG2VLD_DLY_LIMIT) ||
        (sync_cfg_buf_node->sns_regs_info.reg_num > OT_ISP_MAX_SNS_REGS)) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_err_trace("Pipe[%d] Invalid sns_regs_info!(sns_type:%d, wdr:%d, delay_max:%u, slv_delay:%u, reg_num:%u)\n",
            vi_pipe, sync_cfg_buf_node->sns_regs_info.sns_type,
            sync_cfg_buf_node->ae_reg_cfg.fs_wdr_mode,
            sync_cfg_buf_node->sns_regs_info.cfg2_valid_delay_max,
            sync_cfg_buf_node->sns_regs_info.slv_sync.delay_frame_num,
            sync_cfg_buf_node->sns_regs_info.reg_num);
        return TD_FAILURE;
    }

    cur_node = &sync_cfg_buf->sync_cfg_buf_node[sync_cfg_buf->buf_wr_flag];
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    (td_void)memcpy_s(cur_node, sizeof(isp_sync_cfg_buf_node), sync_cfg_buf_node, sizeof(isp_sync_cfg_buf_node));

    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    sync_cfg_buf->buf_wr_flag = (sync_cfg_buf->buf_wr_flag + 1) % ISP_SYNC_BUF_NODE_NUM;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_STITCH_GRP
static td_void isp_drv_be_stitch_sync_param_init(ot_vi_pipe vi_pipe, const isp_be_sync_para *be_sync_param,
    isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    ot_vi_pipe stitch_pipe;
    isp_drv_ctx *drv_ctx_stitch_pipe = TD_NULL;
    if (drv_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        return;
    }
    if (drv_ctx->stitch_attr.main_pipe != TD_TRUE) {
        return;
    }
    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        (td_void)memcpy_s(&drv_ctx->be_sync_para_stitch[i], sizeof(isp_be_sync_para), be_sync_param,
            sizeof(isp_be_sync_para));
        (td_void)memcpy_s(&drv_ctx->be_pre_sync_para_stitch[i], sizeof(isp_be_sync_para), be_sync_param,
            sizeof(isp_be_sync_para));
    }

    for (i = 1; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = drv_ctx->stitch_attr.stitch_bind_id[i];
        drv_ctx_stitch_pipe = isp_drv_get_ctx(stitch_pipe);
        (td_void)memcpy_s(&drv_ctx_stitch_pipe->be_sync_para_stitch[0], sizeof(isp_be_sync_para) * OT_ISP_MAX_PIPE_NUM,
            &drv_ctx->be_sync_para_stitch[0], sizeof(isp_be_sync_para) * OT_ISP_MAX_PIPE_NUM);
        (td_void)memcpy_s(&drv_ctx_stitch_pipe->be_pre_sync_para_stitch[0],
            sizeof(isp_be_sync_para) * OT_ISP_MAX_PIPE_NUM, &drv_ctx->be_pre_sync_para_stitch[0],
            sizeof(isp_be_sync_para) * OT_ISP_MAX_PIPE_NUM);
    }
}
#endif
static td_s32 isp_drv_be_sync_param_init(ot_vi_pipe vi_pipe, const isp_be_sync_para *be_sync_param)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_sync_param);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    (td_void)memcpy_s(&drv_ctx->be_sync_para, sizeof(isp_be_sync_para), be_sync_param, sizeof(isp_be_sync_para));
    (td_void)memcpy_s(&drv_ctx->be_pre_sync_para, sizeof(isp_be_sync_para), be_sync_param, sizeof(isp_be_sync_para));
    drv_ctx->dyna_blc_info.pre_black_level_mode = OT_ISP_BLACK_LEVEL_MODE_BUTT;
#ifdef CONFIG_OT_VI_STITCH_GRP
    isp_drv_be_stitch_sync_param_init(vi_pipe, be_sync_param, drv_ctx);
#endif
    return TD_SUCCESS;
}

static td_s32 isp_drv_be_buf_run_state(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);
    isp_check_bebuf_init_return(vi_pipe, drv_ctx->be_buf_info.init);

    isp_spin_lock = isp_drv_get_spin_lock(vi_pipe);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        if (drv_ctx->running_state != ISP_BE_BUF_STATE_INIT) {
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

            isp_warn_trace("Pipe[%d] isp isn't init state!\n", vi_pipe);
            return TD_FAILURE;
        }

        drv_ctx->running_state = ISP_BE_BUF_STATE_RUNNING;
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_be_buf_switch_state(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_spin_lock = isp_drv_get_spin_lock(vi_pipe);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        drv_ctx->running_state = ISP_BE_BUF_STATE_SWITCH_START;
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_be_buf_switch_finish_state(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    ot_vi_pipe vi_pipes;
    isp_drv_ctx *drv_ctx_s = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_spin_lock = isp_drv_get_spin_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        if (drv_ctx->running_state != ISP_BE_BUF_STATE_SWITCH_START) {
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
            isp_warn_trace("Pipe[%d] isp isn't init state!\n", vi_pipe);
            return TD_FAILURE;
        }

        drv_ctx->running_state = ISP_BE_BUF_STATE_SWITCH;

        for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
            vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
            drv_ctx_s = isp_drv_get_ctx(vi_pipes);
            if (drv_ctx_s->running_state != ISP_BE_BUF_STATE_SWITCH) {
                osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
                isp_warn_trace("Pipe[%d] isp isn't  finish state!\n", vi_pipe);
                return TD_FAILURE;
            }
        }

        for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
            vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
            drv_ctx_s = isp_drv_get_ctx(vi_pipes);
            drv_ctx_s->running_state = ISP_BE_BUF_STATE_INIT;
        }
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_STITCH_GRP
static td_void isp_drv_stitch_be_buf_ctl(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    td_s32 ret;
    ot_vi_pipe vi_pipes;
    ot_vi_pipe main_pipe;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_drv_ctx *drv_ctx_s = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_sync_lock = TD_NULL;

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    main_pipe = drv_ctx->stitch_attr.stitch_bind_id[0];
    isp_sync_lock = isp_drv_get_sync_lock(main_pipe);
    osal_spin_lock_irqsave(isp_sync_lock, &flags);

    if (drv_ctx->running_state != ISP_BE_BUF_STATE_RUNNING) {
        osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
        return;
    }

    drv_ctx->running_state = ISP_BE_BUF_STATE_FINISH;

    ret = isp_drv_stitch_sync(vi_pipe);
    if (ret != TD_SUCCESS) {
        osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
        return;
    }

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
        drv_ctx_s = isp_drv_get_ctx(vi_pipes);
        if (drv_ctx_s->running_state != ISP_BE_BUF_STATE_FINISH) {
            osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
            return;
        }
    }

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        vi_pipes = drv_ctx->stitch_attr.stitch_bind_id[i];
        drv_ctx_s = isp_drv_get_ctx(vi_pipes);
        if (drv_ctx_s->be_buf_info.init != TD_TRUE) {
            isp_err_trace("Pipe[%d] BeBuf (bInit != TRUE) !\n", vi_pipe);
            osal_spin_unlock_irqrestore(isp_sync_lock, &flags);
            return;
        }

        if (drv_ctx_s->run_once_flag != TD_TRUE) {
            isp_drv_be_buf_queue_put_busy(vi_pipes);
        }
        drv_ctx_s->running_state = ISP_BE_BUF_STATE_INIT;
    }

    osal_spin_unlock_irqrestore(isp_sync_lock, &flags);

    return;
}
#endif

static td_s32 isp_drv_be_buf_ctl(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_online_mode_return(vi_pipe, drv_ctx->work_mode.running_mode);

    if (drv_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        if (drv_ctx->be_buf_info.init != TD_TRUE) {
            isp_err_trace("Pipe[%d] BeBuf (bInit != TRUE) !\n", vi_pipe);
            return TD_FAILURE;
        }
        isp_spin_lock = isp_drv_get_lock(vi_pipe);
        osal_spin_lock_irqsave(isp_spin_lock, &flags);
        if (drv_ctx->run_once_flag != TD_TRUE) {
            isp_drv_be_buf_queue_put_busy(vi_pipe);
        }
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    } else {
#ifdef CONFIG_OT_VI_STITCH_GRP
        isp_drv_stitch_be_buf_ctl(vi_pipe);
#endif
    }

    return TD_SUCCESS;
}


static td_void isp_drv_switch_mode_cfg_vc(isp_sync_cfg *sync_cfg)
{
    sync_cfg->vc_num = 0;
    sync_cfg->vc_cfg_num = 0;
    sync_cfg->cfg2_vld_dly_max = 1;

    /* get N (N to 1 WDR) */
    switch (sync_cfg->wdr_mode) {
        default:
            sync_cfg->vc_num_max = 0;
            break;

        case OT_WDR_MODE_2To1_FRAME:
            sync_cfg->vc_num_max = 1;
            break;

        case OT_WDR_MODE_3To1_FRAME:
            sync_cfg->vc_num_max = 2; /* reg config is 2 */
            break;

        case OT_WDR_MODE_4To1_FRAME:
            sync_cfg->vc_num_max = 3; /* reg config is 3 */
            break;
    }
}

static td_void isp_drv_switch_mode_cfg_chn_switch(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    td_u32 chn_switch[ISP_CHN_SWITCH_NUM] = { 0 };

    for (i = 0; i < ISP_CHN_SWITCH_NUM - 1; i++) {
        chn_switch[i] = i;
    }

    if (is_half_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        chn_switch[0] = 1 % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
        chn_switch[1] = (chn_switch[0] + 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
        chn_switch[2] = (chn_switch[0] + 2) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* array index 2 */
        chn_switch[3] = (chn_switch[0] + 3) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* array index 3 */
    }

    chn_switch[4] = drv_ctx->yuv_mode ? 1 : 0; /* array index 4 */

    isp_drv_set_input_sel(vi_pipe, &chn_switch[0], sizeof(chn_switch));
    for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
        (td_void)memcpy_s(drv_ctx->chn_sel_attr[i].wdr_chn_sel, sizeof(chn_switch), chn_switch, sizeof(chn_switch));
    }
}

static td_s32 isp_drv_switch_mode(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    td_s32 vi_dev;

    isp_sync_cfg *sync_cfg = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(drv_ctx);
    vi_dev = drv_ctx->wdr_attr.vi_dev;

    sync_cfg = &drv_ctx->sync_cfg;
    sync_cfg->wdr_mode = drv_ctx->wdr_cfg.wdr_mode;

    for (j = 0; j < OT_ISP_EXP_RATIO_NUM; j++) {
        for (i = 0; i < CFG2VLD_DLY_LIMIT; i++) {
            sync_cfg->exp_ratio[j][i] = drv_ctx->wdr_cfg.exp_ratio[j];
        }
    }

    /* init cfg when modes change */
    (td_void)memset_s(&drv_ctx->sync_cfg.sync_cfg_buf, sizeof(isp_sync_cfg_buf), 0, sizeof(isp_sync_cfg_buf));
    (td_void)memset_s(&drv_ctx->sync_cfg.node, sizeof(drv_ctx->sync_cfg.node), 0, sizeof(drv_ctx->sync_cfg.node));
    (td_void)memset_s(&drv_ctx->isp_int_info, sizeof(isp_interrupt_info), 0, sizeof(isp_interrupt_info));

    sync_cfg->cur_sync_id = -1;

    isp_drv_switch_mode_cfg_vc(sync_cfg);

    /* Channel Switch config */
    isp_drv_switch_mode_cfg_chn_switch(vi_pipe, drv_ctx);

    /* pt_int_mask */
    if ((is_full_wdr_mode(sync_cfg->wdr_mode)) || (is_half_wdr_mode(sync_cfg->wdr_mode))) {
        io_rw_pt_address(vi_pt_base(vi_dev) + VI_PT_INT_MASK) |= VI_PT_INT_FSTART;
    } else {
        io_rw_pt_address(vi_pt_base(vi_dev) + VI_PT_INT_MASK) &= ~(VI_PT_INT_FSTART);
    }

    sync_cfg->pre_wdr_mode = sync_cfg->wdr_mode;

    return TD_SUCCESS;
}


static td_s32 isp_drv_set_wdr_cfg(ot_vi_pipe vi_pipe, isp_wdr_cfg *wdr_cfg)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(wdr_cfg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    if (wdr_cfg->wdr_mode >= OT_WDR_MODE_BUTT) {
        isp_err_trace("Pipe[%d] Invalid WDR mode %d!\n", vi_pipe, wdr_cfg->wdr_mode);
        return TD_FAILURE;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    (td_void)memcpy_s(&drv_ctx->wdr_cfg, sizeof(isp_wdr_cfg), wdr_cfg, sizeof(isp_wdr_cfg));
    ret = isp_drv_switch_mode(vi_pipe, drv_ctx);
    if (ret != TD_SUCCESS) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_err_trace("Pipe[%d] isp_drv_switch_mode err 0x%x!\n", vi_pipe, ret);
        return ret;
    }
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_alg_init_err_info_print(ot_vi_pipe vi_pipe, ot_isp_alg_mod alg_mod)
{
    isp_check_pipe_return(vi_pipe);
    if (alg_mod >= OT_ISP_ALG_MOD_BUTT) {
        isp_err_trace("Not Support this alg module!!\n");
        return TD_FAILURE;
    }

    isp_err_trace("ISP[%d] %s NOT init. Please check cmos parameters or mem malloc!\n",
                  vi_pipe, g_isp_alg_mod_name[alg_mod]);

    return TD_SUCCESS;
}

td_s32 isp_set_pub_attr_info(ot_vi_pipe vi_pipe, ot_isp_pub_attr *pub_attr)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pub_attr);

    if ((pub_attr->wnd_rect.width < OT_ISP_WIDTH_MIN) || (pub_attr->wnd_rect.width > g_res_max_width[vi_pipe]) ||
        (pub_attr->wnd_rect.width % OT_ISP_ALIGN_WIDTH != 0)) {
        isp_err_trace("Invalid Image Width:%d!\n", pub_attr->wnd_rect.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.height < OT_ISP_HEIGHT_MIN) || (pub_attr->wnd_rect.height > g_res_max_height[vi_pipe]) ||
        (pub_attr->wnd_rect.height % OT_ISP_ALIGN_HEIGHT != 0)) {
        isp_err_trace("Invalid Image Height:%d!\n", pub_attr->wnd_rect.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->sns_size.width < OT_ISP_WIDTH_MIN) || (pub_attr->sns_size.width > OT_ISP_SENSOR_WIDTH_MAX)) {
        isp_err_trace("Invalid Sensor Image Width:%d!\n", pub_attr->sns_size.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->sns_size.height < OT_ISP_HEIGHT_MIN) || (pub_attr->sns_size.height > OT_ISP_SENSOR_HEIGHT_MAX)) {
        isp_err_trace("Invalid Sensor Image Height:%d!\n", pub_attr->sns_size.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.x < 0) || (pub_attr->wnd_rect.x > g_res_max_width[vi_pipe] - OT_ISP_WIDTH_MIN)) {
        isp_err_trace("Invalid Image X:%d!\n", pub_attr->wnd_rect.x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.y < 0) || (pub_attr->wnd_rect.y > g_res_max_height[vi_pipe] - OT_ISP_HEIGHT_MIN)) {
        isp_err_trace("Invalid Image Y:%d!\n", pub_attr->wnd_rect.y);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pub_attr->bayer_format >= OT_ISP_BAYER_BUTT) {
        isp_err_trace("Invalid Bayer Pattern:%d!\n", pub_attr->bayer_format);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pub_attr->wdr_mode >= OT_WDR_MODE_BUTT) {
        isp_err_trace("Invalid WDR mode %d!\n", pub_attr->wdr_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    (td_void)memcpy_s(&drv_ctx->proc_pub_info, sizeof(ot_isp_pub_attr), pub_attr, sizeof(ot_isp_pub_attr));
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    drv_ctx->pub_attr_ok = TD_TRUE;

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_dynamic_actual_info(ot_vi_pipe vi_pipe, isp_blc_actual_info *actual_info)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(actual_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    (td_void)memcpy_s(actual_info, sizeof(isp_blc_actual_info),
                      &drv_ctx->dyna_blc_info.actual_info, sizeof(isp_blc_actual_info));
    return TD_SUCCESS;
}

static td_s32 isp_drv_set_fpn_work_mode_set(ot_vi_pipe vi_pipe, const td_u8 *work_mode)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    if ((*work_mode != FPN_MODE_CORRECTION) && (*work_mode != FPN_MODE_CALIBRATE) &&
        (*work_mode != FPN_MODE_NONE)) {
        return TD_FAILURE;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->fpn_work_mode = *work_mode;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_get_bas_crop_attr(ot_vi_pipe vi_pipe, vi_blc_crop_info *bas_crop_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(bas_crop_attr);

    if (!ckfn_vi_get_blc_crop_info()) {
        return TD_FAILURE;
    }

    return call_vi_get_blc_crop_info(vi_pipe, bas_crop_attr);
}

static td_s32 isp_get_stagger_attr(ot_vi_pipe vi_pipe, isp_stagger_attr *stagger_attr)
{
    td_s32 ret;
    vi_out_mode_info out_mode_info;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stagger_attr);
    if (is_no_fe_pipe(vi_pipe)) {
        stagger_attr->stagger_en       = TD_FALSE;
        stagger_attr->merge_frame_num  = 1;
        stagger_attr->crop_info.enable = TD_FALSE;
        return TD_SUCCESS;
    }

    if (!ckfn_vi_get_out_mode()) {
        isp_err_trace("pipe[%d] ckfn_vi_get_out_mode is null\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = call_vi_get_out_mode(vi_pipe, &out_mode_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe[%d] call_vi_get_out_mode failed 0x%x!\n", vi_pipe, ret);
        return ret;
    }

    if (out_mode_info.out_mode == OT_VI_OUT_MODE_2F1_STAGGER) {
        stagger_attr->stagger_en      = TD_TRUE;
        stagger_attr->merge_frame_num = 2; /* 2F1_STAGGER */
    } else if (out_mode_info.out_mode == OT_VI_OUT_MODE_3F1_STAGGER) {
        stagger_attr->stagger_en      = TD_TRUE;
        stagger_attr->merge_frame_num = 3; /* 3F1_STAGGER */
    } else if (out_mode_info.out_mode == OT_VI_OUT_MODE_4F1_STAGGER) {
        stagger_attr->stagger_en      = TD_TRUE;
        stagger_attr->merge_frame_num = 4; /* 4F1_STAGGER */
    } else {
        stagger_attr->stagger_en      = TD_FALSE;
        stagger_attr->merge_frame_num = 1;
    }
    (td_void)memcpy_s(&stagger_attr->crop_info, sizeof(ot_crop_info),
                      &out_mode_info.satgger_crop_info, sizeof(ot_crop_info));

    return TD_SUCCESS;
}

static td_s32 isp_drv_get_split_attr(ot_vi_pipe vi_pipe, vi_pipe_split_attr *pipe_split_attr)
{
    if (!ckfn_vi_get_split_attr()) {
        return TD_FAILURE;
    }

    return call_vi_get_split_attr(vi_pipe, pipe_split_attr);
}

static td_s32 isp_drv_get_running_mode(const vi_pipe_split_attr *pipe_split_attr, isp_running_mode *running_mode)
{
    isp_check_pointer_return(running_mode);
    isp_check_pointer_return(pipe_split_attr);
    switch (pipe_split_attr->vi_vpss_mode) {
        case OT_VI_ONLINE_VPSS_OFFLINE:
        case OT_VI_ONLINE_VPSS_ONLINE:
            if (pipe_split_attr->aiisp_mode == OT_VI_AIISP_MODE_DEFAULT) {
                *running_mode = ISP_MODE_RUNNING_ONLINE;
            } else {
#ifdef CONFIG_SD3403V100
                *running_mode = ISP_MODE_RUNNING_PRE_ONLINE_POST_OFFLINE;
#else
                *running_mode = ISP_MODE_RUNNING_ONLINE;
#endif
            }
            break;
        case OT_VI_OFFLINE_VPSS_OFFLINE:
        case OT_VI_OFFLINE_VPSS_ONLINE:
            if (pipe_split_attr->split_num == 1) {
                *running_mode = ISP_MODE_RUNNING_OFFLINE;
            } else {
                *running_mode = ISP_MODE_RUNNING_STRIPING;
            }
            break;
        default:
            *running_mode = ISP_MODE_RUNNING_BUTT;
            return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void isp_drv_get_yuv_mode(const vi_pipe_split_attr *pipe_split_attr, td_bool *yuv_mode,
                                    isp_data_input_mode *data_input_mode)
{
    if ((pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YUV_SEMIPLANAR_444) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_444) ||
        (pipe_split_attr->pixel_format == OT_PIXEL_FORMAT_YUV_400)) {
        *yuv_mode = TD_TRUE;
        *data_input_mode = ISP_MODE_BT1120_YUV;
    } else {
        *yuv_mode = TD_FALSE;
        *data_input_mode = ISP_MODE_RAW;
    }

    return;
}

static td_s32 isp_drv_work_mode_init(ot_vi_pipe vi_pipe, isp_block_attr *blk_attr)
{
    td_s32 ret;
    td_bool nr_en = TD_FALSE;
    unsigned long flags = 0;
    isp_running_mode running_mode = ISP_MODE_RUNNING_OFFLINE;
    vi_pipe_split_attr pipe_split_attr = { 0 };
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_frame_interrupt_attr frame_int_attr = { 0 };
    td_bool yuv_mode = TD_FALSE;
    isp_data_input_mode data_input_mode = ISP_MODE_RAW;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(blk_attr);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->work_mode.pre_block_num = drv_ctx->work_mode.block_num;

    ret = isp_drv_get_split_attr(vi_pipe, &pipe_split_attr);
    isp_check_return(vi_pipe, ret, "isp_drv_get_split_attr");

    ret = isp_drv_get_nr_mode(vi_pipe, &nr_en);
    isp_check_return(vi_pipe, ret, "isp_drv_get_nr_mode");

    drv_ctx->work_mode.is_ia_nr_enable = nr_en;

    isp_check_block_num_return(pipe_split_attr.split_num);

    ret = isp_drv_get_running_mode(&pipe_split_attr, &running_mode);
    isp_check_return(vi_pipe, ret, "isp_drv_get_running_mode");

    isp_drv_get_yuv_mode(&pipe_split_attr, &yuv_mode, &data_input_mode);

    ret = isp_drv_get_frame_interrupt_attr(vi_pipe, &frame_int_attr);
    isp_check_return(vi_pipe, ret, "isp_drv_get_frame_interrupt_attr");

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    drv_ctx->yuv_mode = yuv_mode;
    drv_ctx->work_mode.data_input_mode = data_input_mode;

    drv_ctx->work_mode.block_num = pipe_split_attr.split_num;
    drv_ctx->work_mode.block_dev = (td_u8)isp_drv_get_block_id(vi_pipe, running_mode);
    drv_ctx->work_mode.running_mode = running_mode;
    drv_ctx->work_mode.aiisp_mode = pipe_split_attr.aiisp_mode;
    drv_ctx->work_mode.over_lap = pipe_split_attr.overlap;

    blk_attr->block_num = pipe_split_attr.split_num;
    blk_attr->over_lap = pipe_split_attr.overlap;
    blk_attr->running_mode = running_mode;

    drv_ctx->work_mode.frame_rect.width = pipe_split_attr.wch_out_rect.width;
    drv_ctx->work_mode.frame_rect.height = pipe_split_attr.wch_out_rect.height;

    (td_void)memcpy_s(&blk_attr->frame_rect, sizeof(ot_size), &drv_ctx->work_mode.frame_rect, sizeof(ot_size));
    (td_void)memcpy_s(drv_ctx->work_mode.block_rect, sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM, pipe_split_attr.rect,
        sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM);
    (td_void)memcpy_s(blk_attr->block_rect, sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM, pipe_split_attr.rect,
        sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM);
    drv_ctx->frame_int_attr = frame_int_attr;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_work_mode_exit(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_exit_state_return(vi_pipe, drv_ctx->isp_run_flag);

    drv_ctx->work_mode.running_mode = ISP_MODE_RUNNING_OFFLINE;

    return TD_SUCCESS;
}
static td_s32 isp_drv_get_dng_info(ot_vi_pipe vi_pipe, ot_isp_dng_image_static_info *dng_info)
{
    unsigned long flags = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dng_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_check_tranbuf_init_return(vi_pipe, drv_ctx->trans_info.init);

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    if ((dng_info != TD_NULL) && (drv_ctx->trans_info.dng_info.vir_addr != TD_NULL)) {
        (td_void)memcpy_s(dng_info, sizeof(ot_isp_dng_image_static_info),
            (ot_isp_dng_image_static_info *)drv_ctx->trans_info.dng_info.vir_addr,
            sizeof(ot_isp_dng_image_static_info));
    }

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

static td_s32 isp_drv_set_dng_info(ot_vi_pipe vi_pipe, ot_dng_image_dynamic_info *dng_img_dyn_info)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dng_img_dyn_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    (td_void)memcpy_s(&drv_ctx->dng_image_dynamic_info[1], sizeof(ot_dng_image_dynamic_info),
        &drv_ctx->dng_image_dynamic_info[0], sizeof(ot_dng_image_dynamic_info));
    (td_void)memcpy_s(&drv_ctx->dng_image_dynamic_info[0], sizeof(ot_dng_image_dynamic_info), dng_img_dyn_info,
        sizeof(ot_dng_image_dynamic_info));

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_SNAP_SUPPORT
/* vi send Proenable */
static td_s32 isp_set_pro_enable(ot_vi_pipe vi_pipe)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    drv_ctx->pro_enable = TD_TRUE;
    drv_ctx->pro_start = TD_FALSE;
    drv_ctx->pro_frm_num = 0;
    drv_ctx->start_snap_num = 0;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}

td_s32 isp_set_snap_attr(ot_vi_pipe vi_pipe, const isp_snap_attr *snap_attr)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    unsigned long flags = 0;
    osal_spinlock *isp_spin_lock = TD_NULL;
    td_u8 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(snap_attr);

    for (i = 0; i < OT_ISP_MAX_PIPE_NUM; i++) {
        drv_ctx = isp_drv_get_ctx(i);
        if (!drv_ctx->mem_init) {
            continue;
        }

        if ((i == snap_attr->picture_pipe_id) || (i == snap_attr->preview_pipe_id)) {
            isp_spin_lock = isp_drv_get_lock(i);
            osal_spin_lock_irqsave(isp_spin_lock, &flags);
            (td_void)memcpy_s(&drv_ctx->snap_attr, sizeof(isp_snap_attr), snap_attr, sizeof(isp_snap_attr));
            osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
            isp_set_pro_enable(i);
        }
    }
    return TD_SUCCESS;
}
#endif

#ifdef CONFIG_OT_SNAP_SUPPORT
static td_s32 isp_drv_set_config_info(ot_vi_pipe vi_pipe, ot_isp_config_info *isp_config_info)
{
    td_u32 i;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(isp_config_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    for (i = ISP_SAVEINFO_MAX - 1; i >= 1; i--) {
        (td_void)memcpy_s(&drv_ctx->snap_info_save[i], sizeof(ot_isp_config_info), &drv_ctx->snap_info_save[i - 1],
            sizeof(ot_isp_config_info));
    }

    (td_void)memcpy_s(&drv_ctx->snap_info_save[0], sizeof(ot_isp_config_info), isp_config_info,
        sizeof(ot_isp_config_info));

    return TD_SUCCESS;
}
#endif

static td_s32 isp_ioctl_set_int_enable(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_bool en = *(td_bool *)arg;
    isp_check_pipe_return(vi_pipe);

    ot_unused(cmd);
    return isp_drv_set_int_enable(vi_pipe, en);
}

static td_s32 isp_ioctl_set_mem_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_bool_return(*(td_bool *)arg);

    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->mem_init = TD_TRUE;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_mem_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    *(td_bool *)arg = drv_ctx->mem_init;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_init_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    isp_check_bool_return(*(td_bool *)arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->isp_init = *(td_bool *)arg;
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }

    isp_dfx_ctx_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_init_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    *(td_bool *)arg = drv_ctx->isp_init;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_run_state(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_u64 *hand_signal = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    hand_signal = (td_u64 *)arg;
    return isp_drv_set_isp_run_state(vi_pipe, hand_signal);
}

static td_s32 isp_ioctl_get_wdr_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    vi_pipe_wdr_attr *wdr_attr = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    wdr_attr = (vi_pipe_wdr_attr *)arg;
    return isp_drv_get_wdr_attr(vi_pipe, wdr_attr);
}

static td_s32 isp_ioctl_update_pre_blk_num(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->work_mode.pre_block_num = *(td_u8 *)arg;
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_pipe_size(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    ot_size *pipe_size = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    pipe_size = (ot_size *)arg;
    return isp_drv_get_pipe_size(vi_pipe, pipe_size);
}

static td_s32 isp_ioctl_get_hdr_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    vi_pipe_hdr_attr *hdr_attr = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    hdr_attr = (vi_pipe_hdr_attr *)arg;
    return isp_drv_get_hdr_attr(vi_pipe, hdr_attr);
}

static td_s32 isp_ioctl_get_p2_en_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_bool *p2_en = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    p2_en = (td_bool *)arg;
    return isp_drv_get_p2_en_info(vi_pipe, p2_en);
}

static td_s32 isp_ioctl_get_stitch_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    vi_stitch_attr *stitch_attr = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    stitch_attr = (vi_stitch_attr *)arg;
    return isp_drv_get_stitch_attr(vi_pipe, stitch_attr);
}

static td_s32 isp_ioctl_get_version(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_version *version = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    version = (isp_version *)arg;
    return isp_drv_get_version(version);
}

static td_s32 isp_ioctl_init_trans_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_trans_info_buf *trans_info = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    trans_info = (isp_trans_info_buf *)arg;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_trans_info_buf_init(vi_pipe, trans_info);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_stat_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t *stat_phy_addr = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    stat_phy_addr = (td_phys_addr_t *)arg;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_stat_buf_init(vi_pipe, stat_phy_addr);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_be_cfg_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_mmz_buf_ex *be_cfg_buf = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    be_cfg_buf = (isp_mmz_buf_ex *)arg;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_be_buf_init(vi_pipe, be_cfg_buf);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_be_all_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    ot_unused(cmd);
    ot_unused(arg);

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_all_be_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_clut_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    ot_unused(arg);
    ot_unused(cmd);
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_clut_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_fswdr_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_fswdr_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_drc_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_drc_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_init_ldci_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_ldci_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    ot_unused(cmd);
    ot_unused(arg);
    return ret;
}

static td_s32 isp_ioctl_init_stt_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }
    ret = isp_drv_stt_buf_init(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    ot_unused(cmd);
    ot_unused(arg);

    return ret;
}

static td_s32 isp_ioctl_get_ldci_read_stt_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_ldci_read_stt_buf_get(vi_pipe, (isp_ldci_read_stt_buf *)arg);
}

static td_s32 isp_ioctl_init_stt_addr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->stitch_attr.stitch_enable == TD_FALSE) {
        return isp_drv_fe_stt_addr_init(vi_pipe);
    } else {
#ifdef CONFIG_OT_VI_STITCH_GRP
        return isp_drv_fe_stitch_stt_addr_init(vi_pipe);
#else
        return TD_SUCCESS;
#endif
    }
}

static td_s32 isp_ioctl_get_be_lut_stt_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_phys_addr_t phy_addr;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->be_lut2stt_attr.init == TD_FALSE) {
        return OT_ERR_ISP_NOMEM;
    }
    phy_addr = drv_ctx->be_lut2stt_attr.be_lut_stt_buf[0].lut_stt_buf[0].phy_addr;
    *(td_phys_addr_t *)arg = phy_addr;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_be_pre_lut_stt_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_phys_addr_t phy_addr;
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_unused(cmd);
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->be_pre_on_post_off_lut2stt_attr.init == TD_FALSE) {
        return OT_ERR_ISP_NOMEM;
    }
    phy_addr = drv_ctx->be_pre_on_post_off_lut2stt_attr.be_lut_stt_buf[0].lut_stt_buf[0].phy_addr;
    *(td_phys_addr_t *)arg = phy_addr;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_dev_fd(unsigned int cmd, void *arg, void *private_data)
{
    ot_unused(cmd);
    isp_check_pointer_return(arg);
    isp_check_pointer_return(private_data);

    *((td_u32 *)private_data) = *(td_u32 *)arg;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_pwm_num(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    *(td_u32 *)arg = isp_drv_get_pwm_number(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_opt_run_once_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_opt_run_once_info(vi_pipe, (td_bool *)arg);
}

static td_s32 isp_ioctl_yuv_run_once_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_yuv_run_once_info(vi_pipe, (td_bool *)arg);
}

static td_s32 isp_ioctl_run_once_process(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_run_once_process(vi_pipe);
}

static td_s32 isp_ioctl_yuv_run_once_process(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_yuv_run_once_process(vi_pipe);
}

static td_s32 isp_ioctl_run_trigger_process(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_run_trigger_process(vi_pipe);
}

static td_s32 isp_ioctl_set_reg_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_kernel_reg_cfg *reg_kernel_cfg = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    reg_kernel_cfg = (isp_kernel_reg_cfg *)arg;

    return isp_drv_set_reg_kernel_cfgs(vi_pipe, reg_kernel_cfg);
}

static td_s32 isp_ioctl_chn_select_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_u32 channel_sel;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    channel_sel = *(td_u32 *)arg;

    return isp_drv_chn_select_cfg(vi_pipe, channel_sel);
}

static td_s32 isp_ioctl_get_mcf_en(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_bool *nr_en = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    nr_en = (td_bool *)arg;
    isp_drv_get_nr_mode(vi_pipe, nr_en);
    drv_ctx->work_mode.is_ia_nr_enable = *nr_en;
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_bnr_temporal_filt(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_bnr_temporal_filt_set(vi_pipe, (isp_bnr_temporal_filt *)arg);
}

#ifdef IQ_DEBUG
static td_s32 isp_ioctl_set_bnr_mot_bitw(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_bnr_mot_bitw_set(vi_pipe, (td_u8 *)arg);
}
#endif

static td_s32 isp_ioctl_disable_bnr_wmot(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_disable_bnr_wmot(vi_pipe);
}

static td_s32 isp_ioctl_set_mot_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_sharpen_mot_set(vi_pipe, (td_bool *)arg);
}

static td_s32 isp_ioctl_get_clut_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->clut_buf_attr.init == TD_FALSE) {
        return OT_ERR_ISP_NOMEM;
    }

    if (ot_mmz_check_phys_addr(drv_ctx->clut_buf_attr.clut_buf.phy_addr,
        drv_ctx->clut_buf_attr.clut_buf.size) != TD_SUCCESS) {
        isp_err_trace("check clut buf addr error\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }
    *(td_phys_addr_t *)arg = drv_ctx->clut_buf_attr.clut_buf.phy_addr;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_procalcdone(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->pro_start = TD_TRUE;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_sync_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_sync_cfg_buf_node *sync_cfg_buf = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    sync_cfg_buf = (isp_sync_cfg_buf_node *)arg;

    return isp_drv_sync_cfg_set(vi_pipe, sync_cfg_buf);
}

static td_s32 isp_ioctl_init_be_sync_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_be_sync_param_init(vi_pipe, (isp_be_sync_para *)arg);
}

static td_s32 isp_ioctl_init_stt_sync(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_bool_return(*(td_bool *)arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    drv_ctx->stitch_sync = TD_FALSE;
    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        drv_ctx->stitch_sync = *(td_bool *)arg;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_res_switch(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_be_buf_switch_state(vi_pipe);
}

static td_s32 isp_ioctl_set_be_buf_switch_finish(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_be_buf_switch_finish_state(vi_pipe);
}

static td_s32 isp_ioctl_set_wdr_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_set_wdr_cfg(vi_pipe, (isp_wdr_cfg *)arg);
}

static td_s32 isp_ioctl_init_stitch_sync_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

#ifdef CONFIG_OT_VI_STITCH_GRP
    return isp_drv_stitch_sync_ctrl_init(vi_pipe);
#else
    return TD_FAILURE;
#endif
}

static td_s32 isp_ioctl_set_mod_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_mod_param((ot_isp_mod_param *)arg);
}

static td_s32 isp_ioctl_get_mod_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_mod_param((ot_isp_mod_param *)arg);
}

static td_s32 isp_ioctl_set_ctrl_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_ctrl_param(vi_pipe, (ot_isp_ctrl_param *)arg);
}

static td_s32 isp_ioctl_get_ctrl_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_ctrl_param(vi_pipe, (ot_isp_ctrl_param *)arg);
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_s32 isp_ioctl_set_detail_stats_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_fe_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_detail_stats_cfg(vi_pipe, (ot_isp_detail_stats_cfg *)arg);
}

static td_s32 isp_ioctl_get_detail_stats_cfg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_fe_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_detail_stats_cfg(vi_pipe, (ot_isp_detail_stats_cfg *)arg);
}

static td_s32 isp_ioctl_get_detail_blk_size(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_detail_size *detail_size = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    detail_size = (isp_detail_size *)arg;
    return isp_drv_get_detail_blk_size(vi_pipe, detail_size);
}

static td_s32 isp_ioctl_init_detail_stats_buf(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret, ret2;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_detail_stats_info *detail_stats_info = TD_NULL;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    detail_stats_info = (isp_detail_stats_info *)arg;

    ot_unused(cmd);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }

    if (drv_ctx->detail_stats_cfg.enable == TD_FALSE) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        return TD_SUCCESS;
    }
    isp_check_fe_pipe_return(vi_pipe);
    if (is_offline_mode(drv_ctx->work_mode.running_mode) == TD_FALSE) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        isp_err_trace("pipe:%d only support offline mode:%d\n", vi_pipe, (drv_ctx->work_mode.running_mode));
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    ret = isp_drv_detail_stats_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        isp_err_trace("pipe:%d, init detail stats error:%x\n", vi_pipe, ret);
        return ret;
    }

    ret = isp_drv_get_detail_blk_size(vi_pipe, &detail_stats_info->detail_size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe:%d, get_detail_blk_size error:%x\n", vi_pipe, ret);
        goto fail_ret;
    }

    ret = memcpy_s(&detail_stats_info->stats_cfg, sizeof(ot_isp_detail_stats_cfg),
        &drv_ctx->detail_stats_cfg, sizeof(ot_isp_detail_stats_cfg));
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe:%d, memcpy_s error ret:%x\n", vi_pipe, ret);
        goto fail_ret;
    }

    osal_sem_up(&drv_ctx->isp_sem_buf);
    return TD_SUCCESS;
fail_ret:
    ret2 = isp_drv_detail_stats_buf_exit(vi_pipe);
    if (ret2 != TD_SUCCESS) {
        isp_err_trace("pipe:%d, detail_stats_buf_exit error:%x\n", vi_pipe, ret2);
    }
    osal_sem_up(&drv_ctx->isp_sem_buf);

    return ret;
}

static td_s32 isp_ioctl_detail_stats_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    ot_unused(cmd);
    ot_unused(arg);
    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }

    ret = isp_drv_detail_stats_buf_exit(vi_pipe);
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return ret;
}

static td_s32 isp_ioctl_get_detail_stats(unsigned int cmd, void *arg, void *private_data)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe;
    isp_detail_stats_user *detail_stats_to_user = TD_NULL;
    ot_isp_detail_stats *detail_stats = TD_NULL;
    ot_isp_detail_stats *detail_stats_dest = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_void *vir_addr = TD_NULL;

    vi_pipe = isp_get_pipe(private_data);
    isp_check_fe_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    ot_unused(cmd);

    detail_stats_to_user = (isp_detail_stats_user *)arg;
    isp_check_pointer_return(detail_stats_to_user);
    detail_stats_dest = (ot_isp_detail_stats *)detail_stats_to_user->detail_stats;
    isp_check_pointer_return(detail_stats_dest);

    if (osal_sem_down(&drv_ctx->isp_sem_buf)) {
        return -ERESTARTSYS;
    }

    ret = isp_drv_check_detail_stats_param(vi_pipe, drv_ctx);
    if (ret != TD_SUCCESS) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        return ret;
    }

    vir_addr = drv_ctx->detail_stats_buf.detail_buf.vir_addr;
    detail_stats = (ot_isp_detail_stats *)vir_addr;
    if (detail_stats == TD_NULL) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        isp_err_trace("isp%d get detail stats vir addr is null\n", vi_pipe);
        return OT_ERR_ISP_NULL_PTR;
    }

    ret = isp_drv_get_detail_stats(vi_pipe, detail_stats, detail_stats_to_user);
    if (ret != TD_SUCCESS) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        isp_err_trace("pipe:%d, isp get detail stats failed! ret:0x%x\n", vi_pipe, ret);
        return ret;
    }
    ret = osal_copy_to_user(detail_stats_dest, detail_stats, sizeof(ot_isp_detail_stats));
    if (ret != 0) {
        osal_sem_up(&drv_ctx->isp_sem_buf);
        isp_err_trace("pipe:%d, isp copy detail stats to user failed, ret:0x%x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }
    osal_sem_up(&drv_ctx->isp_sem_buf);
    return TD_SUCCESS;
}
#endif
static td_s32 isp_ioctl_print_alg_err_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_alg_init_err_info_print(vi_pipe, *(ot_isp_alg_mod *)arg);
}

static td_s32 isp_ioctl_get_be_buf_num(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    *(td_u8 *)arg = isp_drv_get_be_buf_num(vi_pipe);
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_be_buf_first(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_get_be_buf_first(vi_pipe, (td_phys_addr_t *)arg);
}

static td_s32 isp_ioctl_get_be_free_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_get_be_free_buf(vi_pipe, (isp_be_wo_cfg_buf *)arg);
}

static td_s32 isp_ioctl_get_be_last_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_get_be_last_buf(vi_pipe, (td_phys_addr_t *)arg);
}

static td_s32 isp_ioctl_be_buf_run_state(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_be_buf_run_state(vi_pipe);
}

static td_s32 isp_ioctl_be_buf_ctl(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_be_buf_ctl(vi_pipe);
}

static td_s32 isp_ioctl_trans_info_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_trans_info_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_stat_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_stat_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_be_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_be_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_clut_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_clut_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_fswdr_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_fswdr_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_drc_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_drc_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_ldci_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_ldci_buf_exit(vi_pipe);
}

static td_s32 isp_ioctl_stt_buf_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_stt_buf_exit(vi_pipe);
}

#ifdef CONFIG_OT_SNAP_SUPPORT
static td_s32 isp_ioctl_get_pro_trigger(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    *(td_bool *)arg = drv_ctx->pro_enable;

    if (drv_ctx->pro_enable == TD_TRUE) {
        drv_ctx->pro_enable = TD_FALSE;
        drv_ctx->pro_trig_flag = 1;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_snap_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_snap_attr *snap_attr = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    snap_attr = (isp_snap_attr *)arg;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    (td_void)memcpy_s(snap_attr, sizeof(isp_snap_attr), &drv_ctx->snap_attr, sizeof(isp_snap_attr));

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_config_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    ot_isp_config_info *isp_config_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    isp_config_info = (ot_isp_config_info *)arg;

    return isp_drv_set_config_info(vi_pipe, isp_config_info);
}

static td_s32 isp_ioctl_set_snap_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_snap_attr(vi_pipe, (isp_snap_attr*)arg);
}
#endif

#ifdef CONFIG_OT_ISP_PQ_FOR_AI_SUPPORT
static td_s32 isp_ioctl_set_pq_ai_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_pq_ai_attr((pq_ai_attr *)arg);
}

static td_s32 isp_ioctl_get_pq_ai_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_pq_ai_attr(vi_pipe, (pq_ai_attr *)arg);
}

static td_s32 isp_ioctl_set_pq_ai_post_nr_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_pq_ai_post_nr_attr(vi_pipe, (ot_pq_ai_noiseness_post_attr *)arg);
}

static td_s32 isp_ioctl_get_pq_ai_post_nr_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_pq_ai_post_nr_attr(vi_pipe, (ot_pq_ai_noiseness_post_attr *)arg);
}
#endif

static td_s32 isp_ioctl_set_rgbir_format(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    drv_ctx->isp_rgbir_format = *(td_u32 *)arg;
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_update_pos(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    *(td_u32 *)arg = isp_drv_get_update_pos(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_frm_cnt(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    *(td_u32 *)arg = drv_ctx->frame_cnt;

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_pub_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_set_pub_attr_info(vi_pipe, (ot_isp_pub_attr *)arg);
}

static td_s32 isp_ioctl_reset_ctx(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_reset_ctx(vi_pipe);
}

static td_s32 isp_ioctl_get_dynamic_actual_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_get_dynamic_actual_info(vi_pipe, (isp_blc_actual_info *)arg);
}

static td_s32 isp_ioctl_set_fpn_work_mode(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    return isp_drv_set_fpn_work_mode_set(vi_pipe, (td_u8 *)arg);
}

static td_s32 isp_ioctl_get_bas_crop_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_bas_crop_attr(vi_pipe, (vi_blc_crop_info *)arg);
}

static td_s32 isp_ioctl_get_stagger_attr(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_stagger_attr(vi_pipe, (isp_stagger_attr *)arg);
}

static td_s32 isp_ioctl_init_work_mode(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_work_mode_init(vi_pipe, (isp_block_attr *)arg);
}

static td_s32 isp_ioctl_work_mode_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_work_mode_exit(vi_pipe);
}

static td_s32 isp_ioctl_get_work_mode(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_working_mode *isp_work_mode = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);
    isp_work_mode = (isp_working_mode *)arg;
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    (td_void)memcpy_s(isp_work_mode, sizeof(isp_working_mode), &drv_ctx->work_mode, sizeof(isp_working_mode));

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_stat_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    return isp_drv_get_user_stat_buf(vi_pipe, (isp_stat_info *)arg);
}

static td_s32 isp_ioctl_put_stat_buf(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    return isp_drv_stat_buf_user_put(vi_pipe, (isp_stat_info *)arg);
}

static td_s32 isp_ioctl_get_stat_act(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    return isp_drv_get_stat_info_active(vi_pipe, (isp_stat_info *)arg);
}

static td_s32 isp_ioctl_set_update_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_isp_dcf_update_info *isp_update_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_update_info = (ot_isp_dcf_update_info *)arg;
    (td_void)memcpy_s(&drv_ctx->update_info, sizeof(ot_isp_dcf_update_info),
                      isp_update_info, sizeof(ot_isp_dcf_update_info));
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_set_frame_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_isp_frame_info *isp_frame_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    isp_frame_info = (ot_isp_frame_info *)arg;
    (td_void)memcpy_s(&drv_ctx->frame_info, sizeof(ot_isp_frame_info),
                      isp_frame_info, sizeof(ot_isp_frame_info));
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_frame_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    ot_isp_frame_info *isp_frame_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    isp_frame_info = (ot_isp_frame_info *)arg;
    return isp_get_frame_info(vi_pipe, isp_frame_info);
}

static td_s32 isp_ioctl_set_dng_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_set_dng_info(vi_pipe, (ot_dng_image_dynamic_info *)arg);
}

static td_s32 isp_ioctl_get_dng_info(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_get_dng_info(vi_pipe, (ot_isp_dng_image_static_info *)arg);
}

static td_s32 isp_ioctl_get_frame_edge(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_get_frame_edge(vi_pipe, (td_u32 *)arg);
}

static td_s32 isp_ioctl_get_vd_timeout(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_vd_timeout isp_vd_time_out;
    isp_vd_timeout *vd_time_out = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    vd_time_out = (isp_vd_timeout *)arg;
    (td_void)memcpy_s(&isp_vd_time_out, sizeof(isp_vd_timeout), vd_time_out, sizeof(isp_vd_timeout));
    isp_get_vd_start_time_out(vi_pipe, isp_vd_time_out.milli_sec, &isp_vd_time_out.int_status);
    (td_void)memcpy_s(vd_time_out, sizeof(isp_vd_timeout), &isp_vd_time_out, sizeof(isp_vd_timeout));

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_vd_end_timeout(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_vd_timeout isp_vd_time_out;
    isp_vd_timeout *vd_time_out = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    vd_time_out = (isp_vd_timeout *)arg;
    (td_void)memcpy_s(&isp_vd_time_out, sizeof(isp_vd_timeout), vd_time_out, sizeof(isp_vd_timeout));
    isp_get_vd_end_time_out(vi_pipe, isp_vd_time_out.milli_sec, &isp_vd_time_out.int_status);
    (td_void)memcpy_s(vd_time_out, sizeof(isp_vd_timeout), &isp_vd_time_out, sizeof(isp_vd_timeout));

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_get_be_end_timeout(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_vd_timeout isp_vd_time_out;
    isp_vd_timeout *vd_time_out = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    vd_time_out = (isp_vd_timeout *)arg;
    (td_void)memcpy_s(&isp_vd_time_out, sizeof(isp_vd_timeout), vd_time_out, sizeof(isp_vd_timeout));
    isp_get_vd_be_end_time_out(vi_pipe, isp_vd_time_out.milli_sec, &isp_vd_time_out.int_status);
    (td_void)memcpy_s(vd_time_out, sizeof(isp_vd_timeout), &isp_vd_time_out, sizeof(isp_vd_timeout));

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_init_proc(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);

    return isp_drv_proc_init(vi_pipe, (isp_proc_mem *)arg);
}

static td_s32 isp_ioctl_writing_proc(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down_interruptible(&drv_ctx->proc_sem)) {
        return -ERESTARTSYS;
    }
    drv_ctx->proc_sem_flag = TD_TRUE;
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_write_proc_ok(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->proc_sem_flag == TD_TRUE) {
        osal_sem_up(&drv_ctx->proc_sem);
        drv_ctx->proc_sem_flag = TD_FALSE;
    }
    return TD_SUCCESS;
}

static td_s32 isp_ioctl_proc_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_proc_exit(vi_pipe);
}

static td_s32 isp_ioctl_get_proc_param(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(arg);
    ot_unused(cmd);

    *(td_u32 *)arg = isp_drv_get_proc_param(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_ioctl_mem_share_owner(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return isp_drv_mem_share_owner(vi_pipe);
}

static td_s32 isp_ioctl_mem_share_pid(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 pid;

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    pid = *(td_s32 *)arg;
    return isp_drv_mem_share_pid(vi_pipe, pid);
}

static td_s32 isp_ioctl_mem_unshare_pid(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 pid;

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    pid = *(td_s32 *)arg;
    return isp_drv_mem_unshare_pid(vi_pipe, pid);
}

static td_s32 isp_ioctl_mem_share_all(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);
    return isp_drv_mem_share_all(vi_pipe);
}

static td_s32 isp_ioctl_mem_unshare_all(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);
    return isp_drv_mem_unshare_all(vi_pipe);
}

static td_s32 isp_ioctl_mem_verify_pid(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    td_s32 pid;

    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    pid = *(td_s32 *)arg;
    return isp_drv_mem_verify_pid(vi_pipe, pid);
}

static osal_ioctl_cmd g_isp_ioctl_cmd_list[] = {
    { ISP_DEV_SET_FD, isp_ioctl_set_dev_fd },
    { VREG_DRV_FD, isp_ioctl_set_vreg_fd },
    { VREG_DRV_GETADDR, isp_ioctl_get_vreg_addr },
    { VREG_DRV_CHECK_PERMISSION, isp_ioctl_check_permission},
    { ISP_MEM_INFO_GET,  isp_ioctl_get_mem_info },
    { ISP_GET_FRAME_EDGE, isp_ioctl_get_frame_edge },
    { ISP_GET_VD_TIMEOUT, isp_ioctl_get_vd_timeout },
    { ISP_GET_VD_END_TIMEOUT, isp_ioctl_get_vd_end_timeout },
    { ISP_GET_VD_BEEND_TIMEOUT, isp_ioctl_get_be_end_timeout },
    { ISP_MCF_EN_GET, isp_ioctl_get_mcf_en },
    { ISP_ALG_INIT_ERR_INFO_PRINT, isp_ioctl_print_alg_err_info },
    { ISP_BE_FREE_BUF_GET, isp_ioctl_get_be_free_buf },
    { ISP_BE_CFG_BUF_CTL, isp_ioctl_be_buf_ctl },
    { ISP_BE_CFG_BUF_RUNNING, isp_ioctl_be_buf_run_state },
    { ISP_STAT_BUF_GET, isp_ioctl_get_stat_buf },
    { ISP_STAT_ACT_GET, isp_ioctl_get_stat_act },
    { ISP_STAT_BUF_PUT, isp_ioctl_put_stat_buf },
    { ISP_REG_CFG_SET, isp_ioctl_set_reg_cfg },
    { ISP_UPDATE_POS_GET, isp_ioctl_get_update_pos },
    { ISP_DYNAMIC_ACTUAL_INFO_GET, isp_ioctl_get_dynamic_actual_info },
    { ISP_BNR_TEMPORAL_FILT_CFG_SET, isp_ioctl_set_bnr_temporal_filt },
#ifdef IQ_DEBUG
    { ISP_BNR_MOT_BITW_SET, isp_ioctl_set_bnr_mot_bitw },
#endif
    { ISP_DISABLE_BNR_WMOT, isp_ioctl_disable_bnr_wmot},
    { ISP_DNG_INFO_SET, isp_ioctl_set_dng_info },
    { ISP_WDR_CFG_SET, isp_ioctl_set_wdr_cfg },
    { ISP_FRAME_CNT_GET, isp_ioctl_get_frm_cnt },
    { ISP_PROC_PARAM_GET, isp_ioctl_get_proc_param },
    { ISP_PROC_WRITE_ING, isp_ioctl_writing_proc },
    { ISP_PROC_WRITE_OK, isp_ioctl_write_proc_ok },
    { ISP_SYNC_CFG_SET, isp_ioctl_set_sync_cfg },
    { ISP_OPT_RUNONCE_INFO, isp_ioctl_opt_run_once_info },
    { ISP_YUV_RUNONCE_INFO, isp_ioctl_yuv_run_once_info },
    { ISP_KERNEL_RUNONCE, isp_ioctl_run_once_process },
    { ISP_KERNEL_YUV_RUNONCE, isp_ioctl_yuv_run_once_process },
    { ISP_KERNEL_RUN_TRIGGER, isp_ioctl_run_trigger_process },

    { ISP_WORK_MODE_GET, isp_ioctl_get_work_mode },
    { ISP_GET_WDR_ATTR, isp_ioctl_get_wdr_attr },
    { ISP_CLUT_BUF_GET, isp_ioctl_get_clut_buf },
    { ISP_GET_STITCH_ATTR, isp_ioctl_get_stitch_attr },
    { ISP_MOT_CFG_SET, isp_ioctl_set_mot_cfg },
    { ISP_PUB_ATTR_INFO, isp_ioctl_set_pub_attr },
    { ISP_CHN_SELECT_CFG, isp_ioctl_chn_select_cfg },
    { ISP_PWM_NUM_GET, isp_ioctl_get_pwm_num },

#ifdef CONFIG_OT_SNAP_SUPPORT
    { ISP_PRO_TRIGGER_GET, isp_ioctl_get_pro_trigger },
    { ISP_SNAP_ATTR_GET, isp_ioctl_get_snap_attr },
    { ISP_CONFIG_INFO_SET, isp_ioctl_set_config_info },
    { ISP_SNAP_ATTR_SET, isp_ioctl_set_snap_attr },
#endif
    { ISP_SET_PROCALCDONE, isp_ioctl_set_procalcdone },
    { ISP_SET_CTRL_PARAM, isp_ioctl_set_ctrl_param },
    { ISP_GET_CTRL_PARAM, isp_ioctl_get_ctrl_param },
    { ISP_SET_RGBIR_FORMAT, isp_ioctl_set_rgbir_format },
    { ISP_DNG_INFO_GET, isp_ioctl_get_dng_info },
    { ISP_UPDATE_INFO_SET, isp_ioctl_set_update_info },
    { ISP_FRAME_INFO_SET, isp_ioctl_set_frame_info },
    { ISP_FRAME_INFO_GET, isp_ioctl_get_frame_info },
    { ISP_FPN_WORK_MODE_SET, isp_ioctl_set_fpn_work_mode },
    { ISP_SET_MOD_PARAM, isp_ioctl_set_mod_param },
    { ISP_GET_MOD_PARAM, isp_ioctl_get_mod_param },
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    { ISP_SET_DETAIL_STATS_CFG, isp_ioctl_set_detail_stats_cfg },
    { ISP_GET_DETAIL_STATS_CFG, isp_ioctl_get_detail_stats_cfg },
    { ISP_GET_DETAIL_STATS, isp_ioctl_get_detail_stats },
    { ISP_GET_DETAIL_BLK_SIZE, isp_ioctl_get_detail_blk_size },
#endif
    /* init & switch */
    { ISP_GET_HDR_ATTR, isp_ioctl_get_hdr_attr },
    { ISP_GET_PIPE_SIZE, isp_ioctl_get_pipe_size },
    { ISP_P2EN_INFO_GET, isp_ioctl_get_p2_en_info },
    { ISP_STT_ADDR_INIT, isp_ioctl_init_stt_addr },
    { ISP_WORK_MODE_INIT, isp_ioctl_init_work_mode },
    { ISP_SYNC_INIT_SET, isp_ioctl_init_stt_sync },
    { ISP_SYNC_STITCH_PARAM_INIT, isp_ioctl_init_stitch_sync_param },
    { ISP_RES_SWITCH_SET, isp_ioctl_set_res_switch },
    { ISP_BE_LAST_BUF_GET, isp_ioctl_get_be_last_buf },
    { ISP_BE_SYNC_PARAM_INIT, isp_ioctl_init_be_sync_param },
    { ISP_BE_SWITCH_FINISH_STATE_SET, isp_ioctl_set_be_buf_switch_finish },
    { ISP_GET_BAS_CROP_ATTR, isp_ioctl_get_bas_crop_attr },

    /* mem init */
    { VREG_DRV_INIT, isp_ioctl_init_vreg },
    { ISP_PRE_BLK_NUM_UPDATE, isp_ioctl_update_pre_blk_num },
    { ISP_STAGGER_ATTR_GET, isp_ioctl_get_stagger_attr },
    { ISP_BE_BUF_NUM_GET, isp_ioctl_get_be_buf_num },
    { ISP_GET_VERSION, isp_ioctl_get_version },
    { ISP_MEM_INFO_SET,   isp_ioctl_set_mem_info },
    /* init */
    { ISP_LDCI_BUF_INIT, isp_ioctl_init_ldci_buf },
    { ISP_LDCI_READ_STT_BUF_GET, isp_ioctl_get_ldci_read_stt_buf },
    { ISP_DRC_BUF_INIT, isp_ioctl_init_drc_buf },
    { ISP_FSWDR_BUF_INIT, isp_ioctl_init_fswdr_buf },
    { ISP_BE_CFG_BUF_INIT, isp_ioctl_init_be_cfg_buf },
    { ISP_GET_BE_BUF_FIRST, isp_ioctl_get_be_buf_first },
    { ISP_STT_BUF_INIT, isp_ioctl_init_stt_buf },
    { ISP_BE_PRE_LUT_STT_BUF_GET, isp_ioctl_get_be_pre_lut_stt_buf },
    { ISP_BE_LUT_STT_BUF_GET, isp_ioctl_get_be_lut_stt_buf },
    { ISP_FE_LUT_STT_BUF_GET, isp_drv_fe_lut_stt_buf_get },
    { ISP_CLUT_BUF_INIT, isp_ioctl_init_clut_buf },
    { ISP_STAT_BUF_INIT, isp_ioctl_init_stat_buf },
    { ISP_PROC_INIT, isp_ioctl_init_proc },
    { ISP_TRANS_BUF_INIT, isp_ioctl_init_trans_info },
    { ISP_BE_ALL_BUF_INIT, isp_ioctl_init_be_all_buf },
    { ISP_INIT_INFO_SET, isp_ioctl_set_init_info },
    { ISP_INIT_INFO_GET, isp_ioctl_get_init_info },
    { ISP_RUN_STATE_SET, isp_ioctl_set_run_state },
    { ISP_SET_INT_ENABLE, isp_ioctl_set_int_enable },
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    { ISP_DETAIL_STATS_BUF_INIT, isp_ioctl_init_detail_stats_buf },
#endif
    /* exit */
    { ISP_TRANS_BUF_EXIT, isp_ioctl_trans_info_buf_exit },
    { ISP_PROC_EXIT, isp_ioctl_proc_exit },
    { ISP_STAT_BUF_EXIT, isp_ioctl_stat_buf_exit },
    { ISP_CLUT_BUF_EXIT, isp_ioctl_clut_buf_exit },
    { ISP_BE_CFG_BUF_EXIT, isp_ioctl_be_buf_exit },
    { ISP_STT_BUF_EXIT, isp_ioctl_stt_buf_exit },
    { ISP_FSWDR_BUF_EXIT, isp_ioctl_fswdr_buf_exit },
    { ISP_DRC_BUF_EXIT, isp_ioctl_drc_buf_exit },
    { ISP_LDCI_BUF_EXIT, isp_ioctl_ldci_buf_exit },
    { ISP_WORK_MODE_EXIT, isp_ioctl_work_mode_exit },
    { ISP_RESET_CTX, isp_ioctl_reset_ctx },
    { VREG_DRV_EXIT, isp_ioctl_vreg_exit },
    { VREG_DRV_RELEASE_ALL, isp_ioctl_release_vreg },
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    { ISP_DETAIL_STATS_BUF_EXIT, isp_ioctl_detail_stats_buf_exit },
#endif
    /* mem share */
    { ISP_SHARE_OWNER, isp_ioctl_mem_share_owner },
    { ISP_SHARE_PID, isp_ioctl_mem_share_pid },
    { ISP_UNSHARE_PID, isp_ioctl_mem_unshare_pid },
    { ISP_SHARE_ALL, isp_ioctl_mem_share_all },
    { ISP_UNSHARE_ALL, isp_ioctl_mem_unshare_all },
    { ISP_VERIFY_PID, isp_ioctl_mem_verify_pid },
#ifdef CONFIG_OT_ISP_PQ_FOR_AI_SUPPORT
    { ISP_PQ_AI_GROUP_ATTR_SET, isp_ioctl_set_pq_ai_attr },
    { ISP_PQ_AI_GROUP_ATTR_GET, isp_ioctl_get_pq_ai_attr },
    { ISP_PQ_AI_POST_NR_ATTR_SET, isp_ioctl_set_pq_ai_post_nr_attr },
    { ISP_PQ_AI_POST_NR_ATTR_GET, isp_ioctl_get_pq_ai_post_nr_attr },
#endif
};

td_void isp_set_ioctl_cmd_list(osal_fileops *isp_fops)
{
    isp_fops->cmd_list = g_isp_ioctl_cmd_list;
    isp_fops->cmd_cnt = sizeof(g_isp_ioctl_cmd_list) / sizeof(g_isp_ioctl_cmd_list[0]);
}

td_s32 isp_init_ioctl_info_check(td_void)
{
    td_s32 i;
    td_u32 cmd_nr;
    td_s32 cmd_num;

    cmd_num = sizeof(g_isp_ioctl_cmd_list) / sizeof(g_isp_ioctl_cmd_list[0]);
    if (cmd_num >= IOC_NR_ISP_BUTT) {
        isp_err_trace("Ioctl cmd num is too large!\n");
        return TD_FAILURE;
    }

    for (i = 0; i < cmd_num - 1; i++) {
        cmd_nr = _IOC_NR(g_isp_ioctl_cmd_list[i].cmd);
        if (cmd_nr >= IOC_NR_ISP_BUTT) {
            isp_err_trace("Ioctl cmd is out of range!\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}
