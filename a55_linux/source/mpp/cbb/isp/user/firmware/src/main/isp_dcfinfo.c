/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_dcfinfo.h"
#include "mkp_isp.h"
#include "ot_mpi_sys_mem.h"
#include "isp_ext_config.h"
#include "isp_main.h"

td_s32 isp_update_info_init(ot_vi_pipe vi_pipe)
{
    td_u64 phy_addr;
    size_t buf_size;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr = isp_ctx->isp_trans_info.update_info.phy_addr;

    ot_ext_system_update_info_high_phyaddr_write(vi_pipe, ((phy_addr >> 32) & 0xFFFFFFFF)); /* right shift 32bits */
    ot_ext_system_update_info_low_phyaddr_write(vi_pipe, (phy_addr & 0xFFFFFFFF));

    buf_size = sizeof(ot_isp_dcf_update_info) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ot_isp_dcf_const_info);

    isp_ctx->update_info_ctrl.isp_update_info = ot_mpi_sys_mmap(phy_addr, buf_size);

    if (isp_ctx->update_info_ctrl.isp_update_info == TD_NULL) {
        isp_err_trace("isp[%d] mmap update info buf failed!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    isp_ctx->update_info_ctrl.isp_dcf_const_info =
        (ot_isp_dcf_const_info *)(isp_ctx->update_info_ctrl.isp_update_info + ISP_MAX_UPDATEINFO_BUF_NUM);

    return TD_SUCCESS;
}

td_void isp_update_info_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    if (isp_ctx->update_info_ctrl.isp_update_info != TD_NULL) {
        size_t buf_size;
        buf_size = sizeof(ot_isp_dcf_update_info) * ISP_MAX_UPDATEINFO_BUF_NUM + sizeof(ot_isp_dcf_const_info);
        ot_mpi_sys_munmap(isp_ctx->update_info_ctrl.isp_update_info, buf_size);
        isp_ctx->update_info_ctrl.isp_update_info = TD_NULL;
        isp_ctx->update_info_ctrl.isp_dcf_const_info = TD_NULL;
    }
}

