/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_mpi_isp_inner.h"
#include <sys/ioctl.h>

#include "ot_mpi_sys.h"
#include "isp_ext_config.h"
#include "isp_debug.h"
#include "isp_main.h"
#include "isp_math_utils.h"
#include "isp_vreg.h"
#include "ot_math.h"
#include "isp_alg.h"
#include "isp_aiisp_ext_config.h"
#include "isp_ext_reg_access.h"
#include "ot_mpi_sys_mem.h"
#include "isp_intf.h"

#include "isp_param_check.h"
#ifdef CONFIG_OT_SNAP_SUPPORT
static td_s32 isp_snap_attr_check(ot_vi_pipe vi_pipe, const ot_snap_attr *snap_attr,
    const isp_snap_pipe *snap_pipe)
{
    if (snap_attr->snap_type >= OT_SNAP_TYPE_BUTT) {
        isp_err_trace("Invalid op mode %d!\n", snap_attr->snap_type);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (snap_pipe->video_pipe == -1) {
        isp_err_trace("Invalid video pipe %d!\n", snap_pipe->video_pipe);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (snap_pipe->pic_pipe == -1) {
        isp_err_trace("Invalid pic pipe %d!\n", snap_pipe->pic_pipe);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_set_snap_attr(ot_vi_pipe vi_pipe, const ot_snap_attr *snap_attr,
    const isp_snap_pipe *snap_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx_s = TD_NULL;
    isp_snap_attr mkp_snap_attr;

    isp_get_ctx(vi_pipe, isp_ctx_s);
    isp_check_mem_init_return(vi_pipe);
    isp_ctx_s->linkage.snap_type = snap_attr->snap_type;
    isp_ctx_s->linkage.preview_pipe_id = snap_pipe->video_pipe;
    isp_ctx_s->linkage.picture_pipe_id = snap_pipe->pic_pipe;
    isp_ctx_s->linkage.load_ccm = snap_attr->load_ccm_en;
    if (vi_pipe == snap_pipe->video_pipe) {
        isp_ctx_s->linkage.snap_pipe_mode = ISP_SNAP_PREVIEW;
    } else {
        isp_ctx_s->linkage.snap_pipe_mode = ISP_SNAP_PICTURE;
    }

    if (isp_ctx_s->linkage.snap_type == OT_SNAP_TYPE_PRO) {
        isp_ctx_s->pro_param.operation_mode = snap_attr->pro_attr.pro_param.op_mode;
        if (isp_ctx_s->pro_param.operation_mode == OT_OP_MODE_AUTO) {
            (td_void)memcpy_s(&(isp_ctx_s->pro_param.auto_param), sizeof(ot_snap_pro_auto_param),
                &snap_attr->pro_attr.pro_param.auto_param, sizeof(ot_snap_pro_auto_param));
        } else if (isp_ctx_s->pro_param.operation_mode == OT_OP_MODE_MANUAL) {
            (td_void)memcpy_s(&(isp_ctx_s->pro_param.manual_param), sizeof(ot_snap_pro_manual_param),
                &snap_attr->pro_attr.pro_param.manual_param, sizeof(ot_snap_pro_manual_param));
        }
    }

    (td_void)memset_s(&mkp_snap_attr, sizeof(isp_snap_attr), 0, sizeof(isp_snap_attr));
    mkp_snap_attr.load_ccm = snap_attr->load_ccm_en;
    mkp_snap_attr.snap_type = snap_attr->snap_type;
    mkp_snap_attr.preview_pipe_id = snap_pipe->video_pipe;
    mkp_snap_attr.picture_pipe_id = snap_pipe->pic_pipe;
    mkp_snap_attr.pro_param.pro_frame_num = snap_attr->pro_attr.frame_cnt;
    mkp_snap_attr.pro_param.operation_mode = snap_attr->pro_attr.pro_param.op_mode;
    if (isp_ctx_s->linkage.snap_type == OT_SNAP_TYPE_PRO) {
        if (isp_ctx_s->pro_param.operation_mode == OT_OP_MODE_AUTO) {
            (td_void)memcpy_s(&(mkp_snap_attr.pro_param.auto_param), sizeof(ot_snap_pro_auto_param),
                &snap_attr->pro_attr.pro_param.auto_param, sizeof(ot_snap_pro_auto_param));
        } else if (isp_ctx_s->pro_param.operation_mode == OT_OP_MODE_MANUAL) {
            (td_void)memcpy_s(&(mkp_snap_attr.pro_param.manual_param), sizeof(ot_snap_pro_manual_param),
                &snap_attr->pro_attr.pro_param.manual_param, sizeof(ot_snap_pro_manual_param));
        }
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_SNAP_ATTR_SET, &mkp_snap_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set snap attr to kernel failed ec %#x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_snap_attr(ot_vi_pipe vi_pipe, const ot_snap_attr *snap_attr, const isp_snap_pipe *snap_pipe)
{
    td_s32 ret;
    td_s32 video_pipe;
    td_s32 pic_pipe;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(snap_attr);
    isp_check_pointer_return(snap_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);

    ret = isp_snap_attr_check(vi_pipe, snap_attr, snap_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    video_pipe = snap_pipe->video_pipe;
    pic_pipe = snap_pipe->pic_pipe;

    ret = isp_set_snap_attr(video_pipe, snap_attr, snap_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Set pipe snap attr %d fail!\n", video_pipe);
        return ret;
    }

    ret = isp_set_snap_attr(pic_pipe, snap_attr, snap_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Set pipe snap attr %d fail!\n", pic_pipe);
        return ret;
    }

    return TD_SUCCESS;
}
#endif

/* find or search the empty pos of algs */
static isp_alg_node *isp_find_register_alg(isp_alg_node *algs, td_u32 alg_id)
{
    td_s32 i;

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (algs[i].alg_type == alg_id) {
            return &algs[i];
        }
    }

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (!algs[i].used) {
            return &algs[i];
        }
    }

    return TD_NULL;
}

td_s32 mpi_isp_register_alg(ot_vi_pipe vi_pipe, td_u32 alg_id, const isp_register_alg_func *funcs)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    isp_check_pointer_return(funcs);
    algs = isp_find_register_alg(isp_ctx->algs, alg_id);
    isp_check_pointer_return(algs);
    algs->alg_type = alg_id;
    algs->alg_func.pfn_alg_init = funcs->isp_register_alg_init;
    algs->alg_func.pfn_alg_run  = funcs->isp_register_alg_run;
    algs->alg_func.pfn_alg_ctrl = funcs->isp_register_alg_ctrl;
    algs->alg_func.pfn_alg_exit = funcs->isp_register_alg_exit;
    algs->used = TD_TRUE;
    return TD_SUCCESS;
}

td_s32 mpi_isp_get_master_pipe(ot_vi_pipe vi_pipe, ot_vi_pipe *master_pipe)
{
    td_s32 ret;
    vi_pipe_wdr_attr wdr_attr = { 0 };
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(master_pipe);
    isp_check_open_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &wdr_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    *master_pipe = wdr_attr.pipe_id[0];

    return TD_SUCCESS;
}

td_u32 mpi_isp_get_pipe_index(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    td_s32 ret;
    vi_pipe_wdr_attr wdr_attr = { 0 };
    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &wdr_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < wdr_attr.pipe_num; i++) {
        if (wdr_attr.pipe_id[i] == vi_pipe) {
            return i;
        }
    }

    return 0;
}

td_s32 mpi_isp_set_ai_mode_attr(ot_vi_pipe vi_pipe, const ot_isp_ai_mode ai_mode,
    const td_bool blend_en)
{
    isp_check_pipe_return(vi_pipe);

    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (ai_mode == OT_ISP_AI_BNR) {
        ot_ext_system_aibnr_en_write(vi_pipe, TD_TRUE);
        ot_ext_system_aibnr_normal_blend_status_write(vi_pipe, blend_en);
        isp_err_trace("set hnr en!!!, set fe dgain enable, blend_en:%d\n", blend_en);
    } else {
        ot_ext_system_aibnr_en_write(vi_pipe, TD_FALSE);
        ot_ext_system_aibnr_normal_blend_status_write(vi_pipe, TD_FALSE);
    }

    return TD_SUCCESS;
}

td_s32 mpi_isp_get_ai_mode_attr(ot_vi_pipe vi_pipe, ot_isp_ai_mode *ai_mode,
    td_bool *blend_en)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ai_mode);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    *ai_mode = (ot_ext_system_aibnr_en_read(vi_pipe) == TD_TRUE) ? OT_ISP_AI_BNR : OT_ISP_AI_NONE;
    *blend_en = ot_ext_system_aibnr_normal_blend_status_read(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 mpi_isp_enable_ai3dnr(ot_vi_pipe vi_pipe)
{
    td_bool bnr_enable = TD_FALSE;
    td_bool tnr_enable = TD_FALSE;

    bnr_enable = ot_ext_system_bayernr_enable_read(vi_pipe);
    tnr_enable = ot_ext_system_bayernr_tnr_enable_read(vi_pipe);
    if (bnr_enable != TD_TRUE || tnr_enable != TD_TRUE) {
        isp_err_trace("ai3dnr need bnr and tnr is enable, current cfg is bnr_enable:%d tnr_enable:%d\n",
            bnr_enable, tnr_enable);
    }

    ot_ext_system_ai3dnr_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 mpi_isp_enable_aiisp(ot_vi_pipe vi_pipe, ot_aiisp_type aiisp_type)
{
    td_s32 ret = TD_SUCCESS;
    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    switch (aiisp_type) {
        case OT_AIISP_TYPE_AIBNR:
            ot_ext_system_aibnr_en_write(vi_pipe, TD_TRUE);
            break;
        case OT_AIISP_TYPE_AIDRC:
            ot_ext_system_aidrc_en_write(vi_pipe, TD_TRUE);
            break;
        case OT_AIISP_TYPE_AIDM:
            ot_ext_system_aidm_en_write(vi_pipe, TD_TRUE);
            break;
        case OT_AIISP_TYPE_AI3DNR:
            ret = mpi_isp_enable_ai3dnr(vi_pipe);
            break;
        default:
            ret = TD_FAILURE;
            isp_err_trace("unsupport type:%d\n", aiisp_type);
            break;
    }

    return ret;
}

td_s32 mpi_isp_disable_aiisp(ot_vi_pipe vi_pipe, ot_aiisp_type aiisp_type)
{
    td_s32 ret = TD_SUCCESS;
    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    switch (aiisp_type) {
        case OT_AIISP_TYPE_AIBNR:
            ot_ext_system_aibnr_en_write(vi_pipe, TD_FALSE);
            break;
        case OT_AIISP_TYPE_AIDRC:
            ot_ext_system_aidrc_en_write(vi_pipe, TD_FALSE);
            break;
        case OT_AIISP_TYPE_AIDM:
            ot_ext_system_aidm_en_write(vi_pipe, TD_FALSE);
            break;
        case OT_AIISP_TYPE_AI3DNR:
            ot_ext_system_ai3dnr_en_write(vi_pipe, TD_FALSE);
            break;
        default:
            ret = TD_FAILURE;
            isp_err_trace("unsupport type:%d\n", aiisp_type);
            break;
    }

    return ret;
}

td_s32 mpi_isp_set_aidrc_mode(ot_vi_pipe vi_pipe, td_bool is_advanced)
{
    isp_check_pipe_return(vi_pipe);

    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ot_ext_system_aidrc_mode_write(vi_pipe, is_advanced);

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_aidrc_dataflow_status(ot_vi_pipe vi_pipe, td_bool dataflow_en)
{
    isp_check_pipe_return(vi_pipe);

    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ot_ext_system_aidrc_dataflow_status_write(vi_pipe, dataflow_en);

    return TD_SUCCESS;
}

td_s32 isp_drc_blend_check(const char *src, const isp_drc_blend *blend)
{
    td_u32 i;

    if (blend->tone_mapping_wgt_x > 0x80) {
        isp_err_trace("Err %s tone_mapping_wgt_x: %u!\n", src, blend->tone_mapping_wgt_x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->detail_adjust_coef_x > 0xF) {
        isp_err_trace("Err %s detail_adjust_coef_x: %u!\n", src, blend->detail_adjust_coef_x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->blend_luma_max < blend->blend_luma_bright_min) {
        isp_err_trace("Err %s: blend_luma_max (%u) must be greater than blend_luma_bright_min (%u)!\n",
            src, blend->blend_luma_max, blend->blend_luma_bright_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->blend_luma_max < blend->blend_luma_dark_min) {
        isp_err_trace("Err %s: blend_luma_max (%u) must be greater than blend_luma_dark_min (%u)!\n",
            src, blend->blend_luma_max, blend->blend_luma_dark_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->blend_detail_max < blend->blend_detail_bright_min) {
        isp_err_trace("Err %s: blend_detail_max (%u) must be greater than blend_detail_bright_min (%u)!\n",
            src, blend->blend_detail_max, blend->blend_detail_bright_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->blend_detail_max < blend->blend_detail_dark_min) {
        isp_err_trace("Err %s: blend_detail_max (%u) must be greater than blend_detail_dark_min (%u)!\n",
            src, blend->blend_detail_max, blend->blend_detail_dark_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (blend->detail_adjust_coef_blend > 0xF) {
        isp_err_trace("Err %s detail_adjust_coef_blend: %u!\n", src, blend->detail_adjust_coef_blend);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        if (blend->local_mixing_bright_x[i] > 0x80) {
            isp_err_trace("Err %s local_mixing_bright_x[%u]: %u!\n", src, i, blend->local_mixing_bright_x[i]);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        if (blend->local_mixing_dark_x[i] > 0x80) {
            isp_err_trace("Err %s local_mixing_dark_x[%u]: %u!\n", src, i, blend->local_mixing_dark_x[i]);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_drc_blend(ot_vi_pipe vi_pipe, const isp_drc_blend *blend)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(blend);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_drc_init_return(vi_pipe);

    ret = isp_drc_blend_check("mpi", blend);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_drc_blend_write(vi_pipe, blend);
    ot_ext_system_drc_param_updated_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 mpi_isp_get_drc_blend(ot_vi_pipe vi_pipe, isp_drc_blend *blend)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(blend);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_drc_init_return(vi_pipe);

    isp_drc_blend_read(vi_pipe, blend);
    return TD_SUCCESS;
}

td_s32 mpi_isp_set_ynet_cfg(ot_vi_pipe vi_pipe, const ot_isp_nr_ynet_attr *ynet_cfg)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ynet_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    isp_nr_ynet_attr_write(vi_pipe, ynet_cfg);

    return TD_SUCCESS;
}

td_s32 mpi_isp_get_ynet_cfg(ot_vi_pipe vi_pipe, ot_isp_nr_ynet_attr *ynet_cfg)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ynet_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    isp_nr_ynet_attr_read(vi_pipe, ynet_cfg);

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_binary_enable(ot_vi_pipe vi_pipe, td_bool binary_en)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_bool_return(binary_en);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ot_ext_system_sys_binary_en_write(vi_pipe, binary_en);

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_check_sum_enable(ot_vi_pipe vi_pipe, td_bool sum_en)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_bool_return(sum_en);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ot_ext_system_sys_check_sum_en_write(vi_pipe, sum_en);

    return TD_SUCCESS;
}

td_s32 mpi_isp_get_check_sum_stats(ot_vi_pipe vi_pipe, isp_check_sum_stat *sum_stat)
{
    td_s32 ret;
    td_bool sum_en;

    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_pointer_return(sum_stat);
    isp_check_vreg_permission_return(vi_pipe);

    sum_en = ot_ext_system_sys_check_sum_en_read(vi_pipe);
    if (sum_en == TD_FALSE) {
        isp_err_trace("ISP[%d] not set check_sum_en:%d\n", vi_pipe, sum_en);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    ret = isp_get_check_sum_stats(vi_pipe, sum_stat);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 mpi_isp_set_mesh_shading_gain_lut_attr(ot_vi_pipe vi_pipe,
    const ot_isp_shading_lut_attr *shading_lut_attr, td_bool inner_update)
{
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_lut_attr);
    isp_mutex_lock(vi_pipe);
    isp_check_open_goto(vi_pipe, ret, exit);
    isp_check_mem_init_goto(vi_pipe, ret, exit);
    isp_check_vreg_permission_goto(vi_pipe, ret, exit);
    isp_check_mesh_shading_init_goto(vi_pipe, ret, exit);
    ret = isp_mesh_shading_gain_lut_attr_check("inner", vi_pipe, shading_lut_attr);
    if (ret != TD_SUCCESS) {
        goto exit;
    }
    ot_ext_system_isp_mesh_shading_inner_update_write(vi_pipe, inner_update);
    isp_set_mesh_shading_gain_lut_attr(vi_pipe, shading_lut_attr);
exit:
    isp_mutex_unlock(vi_pipe);
    return ret;
}

td_s32 mpi_isp_set_mesh_shading_attr(ot_vi_pipe vi_pipe,
    const ot_isp_shading_attr *shading_attr, td_bool inner_update)
{
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_attr);
    isp_mutex_lock(vi_pipe);
    isp_check_open_goto(vi_pipe, ret, exit);
    isp_check_mem_init_goto(vi_pipe, ret, exit);
    isp_check_vreg_permission_goto(vi_pipe, ret, exit);
    isp_check_mesh_shading_init_goto(vi_pipe, ret, exit);
    ret = isp_mesh_shading_attr_check("inner", vi_pipe, shading_attr);
    if (ret != TD_SUCCESS) {
        goto exit;
    }
    ot_ext_system_isp_mesh_shading_attr_inner_update_write(vi_pipe, inner_update);
    isp_set_mesh_shading_attr(vi_pipe, shading_attr);
exit:
    isp_mutex_unlock(vi_pipe);
    return ret;
}

