/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_frameinfo.h"
#include <sys/ioctl.h>
#include "ot_mpi_sys_mem.h"
#include "isp_main.h"

static td_s32 isp_trans_info_buf_init(ot_vi_pipe vi_pipe, isp_trans_info_buf *trans_info)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(trans_info);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_TRANS_BUF_INIT, trans_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp[%d] init trans info bufs failed %x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_void isp_trans_info_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_TRANS_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp[%d] exit trans info buf failed %x!\n", vi_pipe, ret);
        return;
    }
}

static td_s32 isp_frame_info_init(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_frame_info *vir = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr = isp_ctx->isp_trans_info.frame_info.phy_addr;

    isp_ctx->frame_info_ctrl.isp_frame = ot_mpi_sys_mmap(phy_addr,
                                                         sizeof(ot_isp_frame_info) * ISP_MAX_FRAMEINFO_BUF_NUM);

    if (isp_ctx->frame_info_ctrl.isp_frame == TD_NULL) {
        isp_err_trace("isp[%d] mmap frame info buf failed!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    for (i = 0; i < ISP_MAX_FRAMEINFO_BUF_NUM; i++) {
        vir = (ot_isp_frame_info *)((td_u8 *)isp_ctx->frame_info_ctrl.isp_frame + sizeof(ot_isp_frame_info) * i);
        vir->iso = 100; // set default iso to 100
    }

    return TD_SUCCESS;
}

static td_void isp_frame_info_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->frame_info_ctrl.isp_frame != TD_NULL) {
        ot_mpi_sys_munmap(isp_ctx->frame_info_ctrl.isp_frame, sizeof(ot_isp_frame_info) * ISP_MAX_FRAMEINFO_BUF_NUM);
        isp_ctx->frame_info_ctrl.isp_frame = TD_NULL;
    }
}

static td_s32 isp_attach_info_init(ot_vi_pipe vi_pipe)
{
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr = isp_ctx->isp_trans_info.atta_info.phy_addr;

    isp_ctx->attach_info_ctrl.attach_info = ot_mpi_sys_mmap(phy_addr, sizeof(ot_isp_attach_info));

    if (isp_ctx->attach_info_ctrl.attach_info == TD_NULL) {
        isp_err_trace("isp[%d] mmap attach info buf failed!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    return TD_SUCCESS;
}

static td_void isp_attach_info_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->attach_info_ctrl.attach_info != TD_NULL) {
        ot_mpi_sys_munmap(isp_ctx->attach_info_ctrl.attach_info, sizeof(ot_isp_attach_info));
        isp_ctx->attach_info_ctrl.attach_info = TD_NULL;
    }
}

static td_s32 isp_color_gamut_info_init(ot_vi_pipe vi_pipe)
{
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr = isp_ctx->isp_trans_info.color_gammut_info.phy_addr;

    isp_ctx->gamut_info_ctrl.color_gamut_info = ot_mpi_sys_mmap(phy_addr, sizeof(ot_isp_colorgammut_info));

    if (isp_ctx->gamut_info_ctrl.color_gamut_info == TD_NULL) {
        isp_err_trace("isp[%d] mmap color gamut info buf failed!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    isp_ctx->gamut_info_ctrl.color_gamut_info->color_gamut = OT_COLOR_GAMUT_BT709;

    return TD_SUCCESS;
}

static td_void isp_color_gamut_info_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->gamut_info_ctrl.color_gamut_info != TD_NULL) {
        ot_mpi_sys_munmap(isp_ctx->gamut_info_ctrl.color_gamut_info, sizeof(ot_isp_colorgammut_info));
        isp_ctx->gamut_info_ctrl.color_gamut_info = TD_NULL;
    }
}

td_s32 isp_trans_info_init(ot_vi_pipe vi_pipe, isp_trans_info_buf *trans_info)
{
    td_s32 ret;
    ret = isp_trans_info_buf_init(vi_pipe, trans_info);
    if (ret != TD_SUCCESS) {
        goto fail00;
    }

    ret = isp_update_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail01;
    }

    ret = isp_frame_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail02;
    }

    ret = isp_attach_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail03;
    }

    ret = isp_color_gamut_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail04;
    }

    ret = isp_dng_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail05;
    }
    return TD_SUCCESS;

fail05:
    isp_color_gamut_info_exit(vi_pipe);
fail04:
    isp_attach_info_exit(vi_pipe);
fail03:
    isp_frame_info_exit(vi_pipe);
fail02:
    isp_update_info_exit(vi_pipe);
fail01:
    isp_trans_info_buf_exit(vi_pipe);
fail00:
    return OT_ERR_ISP_NOT_INIT;
}

td_void isp_trans_info_exit(ot_vi_pipe vi_pipe)
{
    isp_dng_info_exit(vi_pipe);
    isp_color_gamut_info_exit(vi_pipe);
    isp_attach_info_exit(vi_pipe);
    isp_frame_info_exit(vi_pipe);
    isp_update_info_exit(vi_pipe);
    isp_trans_info_buf_exit(vi_pipe);
}

td_s32 isp_pro_info_init(ot_vi_pipe vi_pipe, const isp_pro_info_buf *pro_info)
{
    ot_unused(vi_pipe);
    ot_unused(pro_info);

    return TD_SUCCESS;
}

td_void isp_pro_info_exit(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);
}
