/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_main.h"
#include "isp_ext_config.h"
#include "isp_defaults.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_statistics.h"
#include "isp_regcfg.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_intf.h"
#include "isp_ext_reg_access.h"
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
#include "isp_dump_dbg.h"
#endif
#include "ot_mpi_sys_mem.h"

#define ISP_LIB_VERSION   "0"    /* [0~F] */

static isp_version g_version = {
    .mpp_version = ISP_LIB_VERSION,
    .magic = 0,
};

td_s32 isp_share_mem(td_phys_addr_t phys_addr, td_s32 pid)
{
    ot_sys_mem_info mem_info = {0};
    td_s32 ret;

    ret = ot_mpi_sys_get_mem_info_by_phys(phys_addr, &mem_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get mem info failed, should called by isp main process\n");
        return OT_ERR_ISP_INVALID_ADDR;
    }

    ret = ot_mpi_sys_mem_share(mem_info.mem_handle, pid);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp mem share failed, pid%d \n", pid);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 isp_unshare_mem(td_phys_addr_t phys_addr, td_s32 pid)
{
    ot_sys_mem_info mem_info = {0};
    td_s32 ret;

    ret = ot_mpi_sys_get_mem_info_by_phys(phys_addr, &mem_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get mem info failed, should called by isp main process\n");
        return OT_ERR_ISP_INVALID_ADDR;
    }

    ret = ot_mpi_sys_mem_unshare(mem_info.mem_handle, pid);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp mem unshare failed, pid %d \n", pid);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 isp_share_mem_all(td_phys_addr_t phys_addr)
{
    ot_sys_mem_info mem_info = {0};
    td_s32 ret;

    ret = ot_mpi_sys_get_mem_info_by_phys(phys_addr, &mem_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get mem info failed, should called by isp main process\n");
        return OT_ERR_ISP_INVALID_ADDR;
    }

    ret = ot_mpi_sys_mem_share_all(mem_info.mem_handle);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp mem share all failed\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 isp_unshare_mem_all(td_phys_addr_t phys_addr)
{
    ot_sys_mem_info mem_info = {0};
    td_s32 ret;

    ret = ot_mpi_sys_get_mem_info_by_phys(phys_addr, &mem_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get mem info failed, should called by isp main process\n");
        return OT_ERR_ISP_INVALID_ADDR;
    }

    ret = ot_mpi_sys_mem_unshare_all(mem_info.mem_handle);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp mem unshare all failed\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_bool isp_get_version(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_VERSION, &g_version);
    if (ret) {
        isp_err_trace("register ISP[%d] lib info ec %x!\n", vi_pipe, ret);
        return TD_FALSE;
    }

    return TD_TRUE;
}

static td_s32 pq_ai_attr_get(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    isp_ctx->ai_info.ai_pipe_id   = -1;
    isp_ctx->ai_info.base_pipe_id = -1;
    isp_ctx->ai_info.pq_ai_en     = TD_FALSE;
#ifdef CONFIG_OT_ISP_PQ_FOR_AI_SUPPORT
    td_s32 ret;
    pq_ai_attr ai_attr;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_PQ_AI_GROUP_ATTR_GET, &ai_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (ai_attr.pq_ai_en == TD_TRUE) {
        if ((ai_attr.ai_pipe_id < 0) || (ai_attr.ai_pipe_id >= OT_ISP_MAX_PIPE_NUM) ||
            (ai_attr.base_pipe_id < 0) || (ai_attr.base_pipe_id >= OT_ISP_MAX_PIPE_NUM)) {
            return TD_FAILURE;
        }

        if (ai_attr.ai_pipe_id == ai_attr.base_pipe_id) {
            return TD_FAILURE;
        }

        if ((vi_pipe != ai_attr.ai_pipe_id) && (vi_pipe != ai_attr.base_pipe_id)) {
            return TD_FAILURE;
        }
    }

    isp_ctx->ai_info.pq_ai_en     = ai_attr.pq_ai_en;
    isp_ctx->ai_info.ai_pipe_id   = ai_attr.ai_pipe_id;
    isp_ctx->ai_info.base_pipe_id = ai_attr.base_pipe_id;

    return TD_SUCCESS;
#else
    ot_unused(vi_pipe);
    return TD_SUCCESS;
#endif
}

static td_s32 isp_yuv_mode_get(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    isp_working_mode isp_work_mode;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] work mode get failed!\n", vi_pipe);
        return ret;
    }

    isp_ctx->is_ia_nr_enable = isp_work_mode.is_ia_nr_enable;

    if (isp_work_mode.data_input_mode == ISP_MODE_BT1120_YUV) {
        isp_ctx->isp_yuv_mode = TD_TRUE;
        ret = isp_sensor_ctx_init(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else if (isp_work_mode.data_input_mode == ISP_MODE_RAW) {
        isp_ctx->isp_yuv_mode = TD_FALSE;
    }

    return TD_SUCCESS;
}

td_s32 isp_mem_init_update_ctx(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    /* get pipe size */
    td_s32 ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_PIPE_SIZE, &isp_ctx->pipe_size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get pipe size failed\n", vi_pipe);
        return ret;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAGGER_ATTR_GET, &isp_ctx->stagger_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get stagger failed\n", vi_pipe);
        return ret;
    }

    /* HDR attribute */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_HDR_ATTR, &isp_ctx->hdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get HDR attr failed\n", vi_pipe);
        return ret;
    }

    /* stitch attribute */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_STITCH_ATTR, &isp_ctx->stitch_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get stitch attr failed\n", vi_pipe);
        return ret;
    }

    /* p2en info */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_P2EN_INFO_GET, &isp_ctx->isp0_p2_en);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get p2en info failed\n", vi_pipe);
        return ret;
    }

    /* be buf num */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_BUF_NUM_GET, &isp_ctx->be_buf_num);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get be buf num failed\n", vi_pipe);
        return ret;
    }

    if (isp_get_version(vi_pipe) != TD_TRUE) {
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    /* init block attribute(inclde write ext registers, should after vreg_init) */
    ret = isp_block_init(vi_pipe, &isp_ctx->block_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] init block failed!\n", vi_pipe);
        return ret;
    }

    ret = pq_ai_attr_get(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get pq ai attr failed!\n", vi_pipe);
        return ret;
    }

    ret = isp_yuv_mode_get(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_check_sns_register(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    if ((isp_ctx->sns_reg != TD_TRUE) && (isp_ctx->isp_yuv_mode != TD_TRUE)) {
        isp_err_trace("Sensor doesn't register to ISP[%d]!\n", vi_pipe);
        return OT_ERR_ISP_SNS_UNREGISTER;
    }

    return TD_SUCCESS;
}

td_s32 isp_mem_info_set(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;

    (td_void)memset_s(&isp_ctx->para_rec, sizeof(isp_para_rec), 0, sizeof(isp_para_rec));
    ot_ext_top_wdr_cfg_write(vi_pipe, isp_ctx->para_rec.wdr_cfg);
    ot_ext_top_pub_attr_cfg_write(vi_pipe, isp_ctx->para_rec.pub_cfg);

    ot_ext_top_wdr_switch_write(vi_pipe, TD_FALSE);
    ot_ext_top_res_switch_write(vi_pipe, TD_FALSE);

    isp_ctx->mem_init = TD_TRUE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_MEM_INFO_SET, &isp_ctx->mem_init);
    if (ret != TD_SUCCESS) {
        isp_ctx->mem_init = TD_FALSE;
        isp_err_trace("ISP[%d] set mem info failed!\n", vi_pipe);
        return OT_ERR_ISP_MEM_NOT_INIT;
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_info(ot_vi_pipe vi_pipe)
{
    td_s32 i, ret;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_check_open_return(vi_pipe);

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (isp_ctx->algs[i].used) {
            if (isp_ctx->algs[i].alg_func.pfn_alg_ctrl != TD_NULL) {
                isp_ctx->algs[i].alg_func.pfn_alg_ctrl(vi_pipe, OT_ISP_UPDATE_INFO_GET, &isp_ctx->dcf_update_info);
            }
        }
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_UPDATE_INFO_SET, &isp_ctx->dcf_update_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set dcf updateinfo failed\n", vi_pipe);
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_frame_info(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_open_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_FRAME_INFO_SET, &isp_ctx->frame_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set frameinfo failed\n", vi_pipe);
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_attach_info(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->attach_info_ctrl.attach_info == TD_NULL) {
        isp_err_trace("pipe:%d the isp attach info hasn't init!\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (isp_ctx->algs[i].used) {
            if (isp_ctx->algs[i].alg_func.pfn_alg_ctrl != TD_NULL) {
                isp_ctx->algs[i].alg_func.pfn_alg_ctrl(vi_pipe, OT_ISP_ATTACHINFO_GET,
                                                       isp_ctx->attach_info_ctrl.attach_info);
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_color_gamut_info(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->gamut_info_ctrl.color_gamut_info == TD_NULL) {
        isp_err_trace("pipe:%d the isp color gamut info hasn't init!\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < ISP_MAX_ALGS_NUM; i++) {
        if (isp_ctx->algs[i].used) {
            if (isp_ctx->algs[i].alg_func.pfn_alg_ctrl != TD_NULL) {
                isp_ctx->algs[i].alg_func.pfn_alg_ctrl(vi_pipe, OT_ISP_COLORGAMUTINFO_GET,
                                                       isp_ctx->gamut_info_ctrl.color_gamut_info);
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_read_debug_ext_regs(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_bool debug_update;
    ot_isp_debug_info isp_debug_info;

    debug_update = ot_ext_system_sys_debug_update_read(vi_pipe);
    ot_ext_system_sys_debug_update_write(vi_pipe, TD_FALSE);

    if (debug_update == TD_TRUE) {
        isp_debug_info.debug_type = ot_ext_system_sys_debug_type_read(vi_pipe);
        ret = isp_dbg_reg_get(vi_pipe, &isp_debug_info);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp_dbg_reg_get failed\n");
            return TD_FAILURE;
        }

        switch (isp_debug_info.debug_type) {
            case OT_ISP_DEBUG_BLC:
                return isp_blc_dbg_set(vi_pipe, &isp_debug_info);
            case OT_ISP_DEBUG_AE:
                return isp_ae_dbg_set(vi_pipe, &isp_debug_info);
            case OT_ISP_DEBUG_AWB:
                return isp_awb_dbg_set(vi_pipe, &isp_debug_info);
            case OT_ISP_DEBUG_DEHAZE:
                return isp_dehaze_dbg_set(vi_pipe, &isp_debug_info);
            default:
                isp_err_trace("invalid debug_type.\n");
                return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_blc_read_ext_regs(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_u64 phy_addr_high;
    td_u64 phy_addr_temp;

    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr_high = (td_u64)ot_ext_system_sys_debug_high_addr_read(vi_pipe);
    phy_addr_temp = (td_u64)ot_ext_system_sys_debug_low_addr_read(vi_pipe);
    phy_addr_temp |= (phy_addr_high << 32); /* left shift 32bits */

    isp_ctx->freeze_fw = ot_ext_system_freeze_firmware_read(vi_pipe);

    isp_ctx->isp_dbg.debug_en = ot_ext_system_sys_debug_enable_read(vi_pipe);
    isp_ctx->isp_dbg.phy_addr = phy_addr_temp;

    isp_ctx->isp_dbg.depth = ot_ext_system_sys_debug_depth_read(vi_pipe);
    isp_ctx->isp_dbg.size  = ot_ext_system_sys_debug_size_read(vi_pipe);
    isp_ctx->be_frame_attr.frame_pos = ot_ext_system_isp_dump_frame_pos_read(vi_pipe);
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
    isp_dump_dbg_ext_regs_read(vi_pipe, &isp_ctx->be_pos_attr);
#endif
}

td_s32 isp_wdr_cfg_set(ot_vi_pipe vi_pipe)
{
    td_u8  i;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_cmos_default *sns_dft = TD_NULL;
    isp_reg_cfg_attr    *reg_cfg = TD_NULL;
    isp_wdr_cfg wdr_cfg;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_sensor_get_default(vi_pipe, &sns_dft);
    (td_void)memset_s(&wdr_cfg, sizeof(isp_wdr_cfg), 0, sizeof(isp_wdr_cfg));

    wdr_cfg.wdr_mode = isp_ctx->sns_wdr_mode;

    for (i = 0; i < OT_ISP_EXP_RATIO_NUM; i++) {
        wdr_cfg.exp_ratio[i] = clip3(sns_dft->wdr_switch_attr.exp_ratio[i], 0x40, 0xFFF);
    }

    (td_void)memcpy_s(&wdr_cfg.wdr_reg_cfg, sizeof(isp_fswdr_sync_cfg),
                      &reg_cfg->reg_cfg.alg_reg_cfg[0].wdr_reg_cfg.sync_reg_cfg, sizeof(isp_fswdr_sync_cfg));

    ret = ioctl(isp_get_fd(vi_pipe), ISP_WDR_CFG_SET, &wdr_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set WDR mode to kernel failed ec %#x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_init_set_sensor_mode(ot_vi_pipe vi_pipe)
{
    td_s32  ret;
    td_u8   wdr_mode;
    td_u32  fps_value;
    td_void *value = TD_NULL;
    ot_isp_cmos_sns_image_mode sns_image_mode;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        return TD_SUCCESS;
    }

    /* set sensor wdr mode */
    wdr_mode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    ret      = isp_sensor_set_wdr_mode(vi_pipe, wdr_mode);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set sensor WDR mode failed!\n", vi_pipe);
        return OT_ERR_ISP_NOT_INIT;
    }

    sns_image_mode.width  = ot_ext_top_sensor_width_read(vi_pipe);
    sns_image_mode.height = ot_ext_top_sensor_height_read(vi_pipe);
    fps_value = ot_ext_system_fps_base_read(vi_pipe);
    value     = (td_void *)&fps_value;

    sns_image_mode.fps      = *(td_float *)value;
    sns_image_mode.sns_mode = ot_ext_system_sensor_mode_read(vi_pipe);

    ret = isp_sensor_set_image_mode(vi_pipe, &sns_image_mode);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set sensor image mode failed!\n", vi_pipe);
        return OT_ERR_ISP_NOT_INIT;
    }

    return TD_SUCCESS;
}

static td_s32 isp_init_sensor_update(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    if (isp_ctx->isp_yuv_mode == TD_FALSE) {
        ret = isp_sensor_update_all(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] update sensor all failed!\n", vi_pipe);
            return OT_ERR_ISP_NOT_INIT;
        }
    } else {
        ret = isp_sensor_update_all_yuv(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] update sensor all failed!\n", vi_pipe);
            return OT_ERR_ISP_NOT_INIT;
        }
    }

    isp_sensor_get_default(vi_pipe, &sns_dft);
    isp_ctx->frame_info_ctrl.isp_frame->sensor_id   = sns_dft->sns_mode.sns_id;
    isp_ctx->frame_info_ctrl.isp_frame->sensor_mode = sns_dft->sns_mode.sns_mode;
    if (isp_ctx->isp_yuv_mode == TD_FALSE) {
        isp_ctx->frame_info.sensor_id               = sns_dft->sns_mode.sns_id;
        isp_ctx->frame_info.sensor_mode             = sns_dft->sns_mode.sns_mode;
    }
    /* get dng parameters from CMOS.c */
    isp_dng_ext_regs_initialize(vi_pipe, &sns_dft->dng_color_param);
    ot_ext_system_dng_static_info_valid_write(vi_pipe, sns_dft->sns_mode.valid_dng_raw_format);

    if (sns_dft->sns_mode.valid_dng_raw_format == TD_TRUE) {
        (td_void)memcpy_s(&isp_ctx->dng_info_ctrl.isp_dng->dng_raw_format, sizeof(ot_isp_dng_raw_format),
                          &sns_dft->sns_mode.dng_raw_format, sizeof(ot_isp_dng_raw_format));
    } else {
        isp_err_trace("ISP[%d] dng_info not initialized in cmos.c!\n", vi_pipe);
    }

    return TD_SUCCESS;
}

static td_s32 isp_init_sensor_pre_process(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    /* set sensor mode */
    ret = isp_init_set_sensor_mode(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* isp update senor info */
    ret = isp_init_sensor_update(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_stitch_sync_init_set(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;

    isp_ctx->para_rec.stitch_sync = TD_TRUE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_SYNC_INIT_SET, &isp_ctx->para_rec.stitch_sync);
    if (ret != TD_SUCCESS) {
        isp_ctx->para_rec.stitch_sync = TD_FALSE;
        isp_err_trace("ISP[%d] set isp stitch sync failed!\n", vi_pipe);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_init_global_info(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    const td_u64 hand_signal = ISP_INIT_HAND_SIGNAL;

    isp_ctx->para_rec.init = TD_TRUE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_INIT_INFO_SET, &isp_ctx->para_rec.init);
    if (ret != TD_SUCCESS) {
        isp_ctx->para_rec.init = TD_FALSE;
        isp_err_trace("ISP[%d] set isp init info failed!\n", vi_pipe);
        goto fail;
    }
    isp_ctx->run_thread_status = ISP_RUN_THREAD_STATUS_INIT;

    /* set handshake signal */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_RUN_STATE_SET, &hand_signal);
    if (ret != TD_SUCCESS) {
        goto fail;
    }
    /* share to isp main process */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_SHARE_OWNER);
    if (ret != TD_SUCCESS) {
        goto fail;
    }

    return TD_SUCCESS;

fail:
    return ret;
}

static td_void isp_algs_register_pq_ai(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
#ifdef CONFIG_OT_ISP_PQ_FOR_AI_SUPPORT
    if ((isp_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == isp_ctx->ai_info.ai_pipe_id)) {
        if (isp_ctx->isp_alg_exp.pfn_alg_register != TD_NULL) {
            isp_ctx->isp_alg_exp.pfn_alg_register(vi_pipe);
        }
    }
#else
    ot_unused(vi_pipe);
    ot_unused(isp_ctx);
#endif
}

static td_void isp_register_algs(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    ot_isp_ctrl_param isp_ctrl_param = { 0 };
    isp_ctx->alg_run_select = OT_ISP_ALG_RUN_NORM;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_CTRL_PARAM, &isp_ctrl_param);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get ctrlparam failed\n", vi_pipe);
        return;
    }

    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        isp_yuv_algs_register(vi_pipe);
    } else if ((isp_ctrl_param.alg_run_select == OT_ISP_ALG_RUN_FE_ONLY) && (is_no_fe_pipe(vi_pipe) == TD_FALSE)) {
        isp_fe_algs_register(vi_pipe);
        isp_ctx->alg_run_select = isp_ctrl_param.alg_run_select;
    } else {
        isp_algs_register(vi_pipe);
        isp_algs_register_pq_ai(vi_pipe, isp_ctx);
    }
}

static td_void isp_apb_reg_pre_cfg(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    if (isp_ctx->isp_yuv_mode != TD_TRUE) {
        ret = isp_lut2stt_apb_reg(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] init lut2stt reg failed!\n", vi_pipe);
        }
    }
}

static td_s32 isp_should_init_sensor(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    ot_isp_sns_regs_info *sns_regs_info = TD_NULL;

    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        isp_ctx->linkage.cfg2valid_delay_max = 2; /* 2: default delay max */
    } else {
        ret = isp_sensor_init(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] init sensor failed!\n", vi_pipe);
            return ret;
        }
        isp_sensor_get_sns_reg(vi_pipe, &sns_regs_info);
        isp_ctx->linkage.cfg2valid_delay_max = sns_regs_info->cfg2_valid_delay_max;
    }

    return TD_SUCCESS;
}

static td_void isp_comm_reg_init(ot_vi_pipe vi_pipe)
{
    isp_ext_regs_default(vi_pipe);
    isp_regs_default(vi_pipe);
    isp_ext_regs_initialize(vi_pipe);
    isp_regs_initialize(vi_pipe);
}

static td_void isp_comm_initialize(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    /* init the common part of extern registers and real registers */
    isp_comm_reg_init(vi_pipe);

    /* isp algs global variable initialize */
    isp_global_initialize(vi_pipe, isp_ctx);
}

static td_void isp_register_and_init_algs(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_void *reg_cfg = TD_NULL;
    /* register all algorithm to isp, and init them. */
    isp_register_algs(vi_pipe, isp_ctx);

    /* get regcfg */
    isp_get_reg_cfg_ctx(vi_pipe, &reg_cfg);

    isp_algs_init(isp_ctx->algs, vi_pipe, reg_cfg);

    isp_apb_reg_pre_cfg(vi_pipe, isp_ctx); /* config before sensor init */
}

static td_void isp_sys_rect_init(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    isp_ctx->sys_rect.x      = ot_ext_system_corp_pos_x_read(vi_pipe);
    isp_ctx->sys_rect.y      = ot_ext_system_corp_pos_y_read(vi_pipe);
    isp_ctx->sys_rect.width  = ot_ext_sync_total_width_read(vi_pipe);
    isp_ctx->sys_rect.height = ot_ext_sync_total_height_read(vi_pipe);
}

td_s32 isp_init(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;

    isp_sys_rect_init(vi_pipe, isp_ctx);

    /* isp alg buf init */
    ret = isp_alg_buf_init(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return OT_ERR_ISP_NOT_INIT;
    }

    /* regcfg init */
    ret = isp_reg_cfg_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail1;
    }

    ret = isp_init_sensor_pre_process(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        goto fail2;
    }

    isp_comm_initialize(vi_pipe, isp_ctx);

    isp_register_and_init_algs(vi_pipe, isp_ctx);

    /* set WDR mode to kernel. */
    ret = isp_wdr_cfg_set(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail3;
    }

    /* sensor init */
    ret = isp_should_init_sensor(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        goto fail3;
    }

    /* regcfg info set */
    ret = isp_reg_cfg_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail4;
    }

    ret = isp_stitch_sync_init_set(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        goto fail4;
    }

    /* init isp be cfgs all buffer */
    ret = isp_all_cfgs_be_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail4;
    }

    /* init isp global variables */
    ret = isp_init_global_info(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        goto fail4;
    }

    return TD_SUCCESS;

fail4:
    if (isp_ctx->isp_yuv_mode != TD_TRUE) {
        isp_sensor_exit(vi_pipe);
    }
fail3:
    isp_algs_exit(vi_pipe, isp_ctx->algs);
    isp_algs_unregister(vi_pipe);
fail2:
    isp_reg_cfg_exit(vi_pipe);
fail1:
    isp_alg_buf_exit(vi_pipe);
    return OT_ERR_ISP_NOT_INIT;
}

static td_s32 isp_switch_mode_update_ctx(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32  ret;

    /* WDR attribute */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &isp_ctx->wdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] update WDR attr failed\n", vi_pipe);
        return ret;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_PIPE_SIZE, &isp_ctx->pipe_size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] update pipe size failed\n", vi_pipe);
        return ret;
    }

    /* p2en info */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_P2EN_INFO_GET, &isp_ctx->isp0_p2_en);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] update p2en info failed\n", vi_pipe);
        return ret;
    }

    ret = isp_stt_addr_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] init stt address failed!\n", vi_pipe);
        return ret;
    }

    ret = isp_block_update(vi_pipe, &isp_ctx->block_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] update ISP block attr failed !\n", vi_pipe);
        return ret;
    }

#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_DETAIL_BLK_SIZE, &isp_ctx->detail_stats_info.detail_size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] update detail_info size failed\n", vi_pipe);
        return ret;
    }
#endif

    isp_ctx->para_rec.stitch_sync = TD_FALSE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_SYNC_INIT_SET, &isp_ctx->para_rec.stitch_sync);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set stitch sync failed!\n", vi_pipe);
    }

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_SYNC_STITCH_PARAM_INIT);
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_RES_SWITCH_SET);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] config res switch failed with!\n", vi_pipe);
        return ret;
    }

    return TD_SUCCESS;
}

static td_void isp_switch_mode_update_finish_flag(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    isp_ctx->pre_sns_wdr_mode = isp_ctx->sns_wdr_mode;

    if (isp_ctx->change_wdr_mode == TD_TRUE) {
        ot_ext_top_wdr_switch_write(vi_pipe, TD_TRUE);
    }

    if ((isp_ctx->change_isp_res == TD_TRUE) || (isp_ctx->change_image_mode == TD_TRUE)) {
        ot_ext_top_res_switch_write(vi_pipe, TD_TRUE);
    }
}

static td_void isp_algs_ctrl_process(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, ot_isp_cmos_black_level *sns_black_level)
{
    ot_isp_cmos_sns_image_mode  isp_image_mode;
    isp_image_mode.width  = ot_ext_sync_total_width_read(vi_pipe);
    isp_image_mode.height = ot_ext_sync_total_height_read(vi_pipe);
    isp_image_mode.fps    = isp_ctx->sns_image_mode.fps;
    isp_image_mode.sns_mode = isp_ctx->sns_image_mode.sns_mode;

    isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_AE_BLC_SET, (td_void *)&sns_black_level->auto_attr.black_level[0][1]);

    /* init the common part of extern registers and real registers */
    isp_ext_regs_initialize(vi_pipe);

    if (isp_ctx->change_wdr_mode == TD_TRUE) {
        isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_WDR_MODE_SET, (td_void *)&isp_ctx->sns_wdr_mode);
    }

    if ((isp_ctx->change_isp_res == TD_TRUE) || (isp_ctx->change_image_mode == TD_TRUE)) {
        isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_CHANGE_IMAGE_MODE_SET, (td_void *)&isp_image_mode);
    }
    isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_AE_FPS_BASE_SET, (td_void *)&isp_ctx->sns_image_mode.fps);
}

static td_s32 isp_switch_mode_init_sensor(ot_vi_pipe vi_pipe)
{
    td_s32  ret;
    isp_usr_ctx              *isp_ctx        = TD_NULL;
    ot_isp_cmos_black_level  *sns_black_level = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);

    ret = isp_reset_fe_stt_en(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] reset fe_stt enable failed\n", vi_pipe);
        return ret;
    }

    /* 2.update info: WDR, HDR, stitch, blockAttr. etc */
    ret = isp_switch_mode_update_ctx(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_reg_cfg_ctrl(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_get_be_last_buf(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get be last bufs failed %x!\n", vi_pipe, ret);
        return ret;
    }

    isp_sensor_update_default(vi_pipe);
    isp_sensor_update_blc(vi_pipe);
    isp_sensor_get_blc(vi_pipe, &sns_black_level);

    isp_algs_ctrl_process(vi_pipe, isp_ctx, sns_black_level);

    ret = isp_wdr_cfg_set(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] isp_wdr_cfg_set error \n", vi_pipe);
        return ret;
    }

    ret = isp_switch_reg_set(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set reg config failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    isp_sensor_update_sns_reg(vi_pipe);
    ret = isp_sensor_switch(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] init sensor failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    isp_switch_mode_update_finish_flag(vi_pipe, isp_ctx);

    return TD_SUCCESS;
}

static td_s32 isp_switch_res(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    ot_isp_cmos_sns_image_mode isp_image_mode;
    td_u8  wdr_mode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    td_u32 fps_value = ot_ext_system_fps_base_read(vi_pipe);
    td_void *value = (td_void *)(&fps_value);

    isp_image_mode.width  = ot_ext_sync_total_width_read(vi_pipe);
    isp_image_mode.height = ot_ext_sync_total_height_read(vi_pipe);
    isp_image_mode.fps    = *(td_float *)value;
    isp_image_mode.sns_mode = ot_ext_system_sensor_mode_read(vi_pipe);

    if (isp_reset_fe_stt_en(vi_pipe) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] reset fe_stt enable failed\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = isp_switch_mode_update_ctx(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_reg_cfg_ctrl(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_get_be_last_buf(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_sensor_update_default(vi_pipe);
    isp_sensor_update_blc(vi_pipe);

    isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_CHANGE_IMAGE_MODE_SET, (td_void *)&isp_image_mode);
    isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_AE_FPS_BASE_SET, (td_void *)&isp_ctx->sns_image_mode.fps);

    if (is_fs_wdr_mode(wdr_mode)) {
        ret = isp_wdr_cfg_set(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    ret = isp_switch_reg_set(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set reg config failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    isp_sensor_update_sns_reg(vi_pipe);
    ret = isp_sensor_switch(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe:%d init sensor failed!\n", vi_pipe);
        return ret;
    }

    ot_ext_top_res_switch_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

static td_void isp_switch_mode_update_sns_info(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    ot_isp_cmos_default  *sns_dft = TD_NULL;
    ot_isp_sns_regs_info *sns_regs_info = TD_NULL;
    isp_sensor_get_default(vi_pipe, &sns_dft);

    isp_ctx->frame_info_ctrl.isp_frame->sensor_id   = sns_dft->sns_mode.sns_id;
    isp_ctx->frame_info_ctrl.isp_frame->sensor_mode = sns_dft->sns_mode.sns_mode;

    isp_sensor_get_sns_reg(vi_pipe, &sns_regs_info);
    isp_switch_state_set(vi_pipe);

    (td_void)memcpy_s(&isp_ctx->dng_info_ctrl.isp_dng->dng_raw_format, sizeof(ot_isp_dng_raw_format),
                      &sns_dft->sns_mode.dng_raw_format, sizeof(ot_isp_dng_raw_format));

    if ((sns_regs_info->cfg2_valid_delay_max > CFG2VLD_DLY_LIMIT) || (sns_regs_info->cfg2_valid_delay_max < 1)) {
        isp_err_trace("ISP[%d] delay of config to invalid is:0x%x\n", vi_pipe, sns_regs_info->cfg2_valid_delay_max);
        sns_regs_info->cfg2_valid_delay_max = 1;
    }

    isp_ctx->linkage.cfg2valid_delay_max = sns_regs_info->cfg2_valid_delay_max;
}

static td_s32 isp_switch_mode_process(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, td_u8 wdr_mode)
{
    td_s32  ret;
    td_u8 hdr_enable;
    td_u8   hdr_mode;
    ot_isp_cmos_sns_image_mode sns_image_mode;

    /* HDR attribute */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_HDR_ATTR, &isp_ctx->hdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get HDR attr failed\n", vi_pipe);
        return ret;
    }

    if (isp_ctx->change_wdr_mode == TD_TRUE) {
        hdr_enable = (isp_ctx->hdr_attr.dynamic_range == OT_DYNAMIC_RANGE_XDR) ? 1 : 0;
        hdr_mode  = (((hdr_enable) & 0x1) << 0x6);
        hdr_mode  = hdr_mode | ot_ext_system_sensor_wdr_mode_read(vi_pipe);

        ret = isp_sensor_set_wdr_mode(vi_pipe, hdr_mode);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] set sensor wdr mode failed\n", vi_pipe);
            return ret;
        }
    }

    sns_image_mode.width  = isp_ctx->sns_image_mode.width;
    sns_image_mode.height = isp_ctx->sns_image_mode.height;
    sns_image_mode.fps    = isp_ctx->sns_image_mode.fps;
    sns_image_mode.sns_mode = isp_ctx->sns_image_mode.sns_mode;

    (td_void)memcpy_s(&isp_ctx->pre_sns_image_mode, sizeof(isp_sensor_image_mode),
                      &isp_ctx->sns_image_mode, sizeof(isp_sensor_image_mode));

    ret = isp_sensor_set_image_mode(vi_pipe, &sns_image_mode);
    if (ret == TD_SUCCESS) {  /* need to init sensor */
        isp_ctx->sns_wdr_mode = wdr_mode;
        isp_ctx->special_opt.fe_stt_update = TD_TRUE; /* used for fe statistics */

        ret = isp_switch_mode_init_sensor(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] switch mode failed!\n", vi_pipe);
            return ret;
        }
    } else {
        if (isp_ctx->change_isp_res == TD_TRUE) {
            isp_ctx->special_opt.fe_stt_update = TD_TRUE; /* used for fe statistics */
            isp_switch_res(vi_pipe, isp_ctx);
        }
        isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_AE_FPS_BASE_SET, (td_void *)&isp_ctx->sns_image_mode.fps);
        ot_ext_top_res_switch_write(vi_pipe, TD_TRUE);
    }

    isp_switch_mode_update_sns_info(vi_pipe, isp_ctx);

    return TD_SUCCESS;
}

static td_bool isp_stitch_main_pipe_switch_finished(isp_usr_ctx *isp_ctx)
{
    td_s8 stitch_main_pipe;

    if ((isp_ctx->stitch_attr.stitch_enable == TD_TRUE) && (isp_ctx->stitch_attr.main_pipe == TD_FALSE)) {
        stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];

        if ((isp_ctx->change_wdr_mode == TD_TRUE) && ot_ext_top_wdr_switch_read(stitch_main_pipe) != TD_TRUE) {
            return TD_FALSE;
        }

        if (((isp_ctx->change_image_mode == TD_TRUE) || (isp_ctx->change_isp_res == TD_TRUE)) &&
            ot_ext_top_res_switch_read(stitch_main_pipe) != TD_TRUE) {
            return TD_FALSE;
        }
    }

    return TD_TRUE;
}

td_s32 isp_switch_mode(ot_vi_pipe vi_pipe)
{
    td_u8   wdr_mode;
    td_u32  fps_value;
    td_void   *value    = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);
    wdr_mode  = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    fps_value = ot_ext_system_fps_base_read(vi_pipe);
    value     = (td_void *)&fps_value;
    isp_ctx->sns_image_mode.fps      = *(td_float *)value;
    isp_ctx->sns_image_mode.width    = ot_ext_top_sensor_width_read(vi_pipe);
    isp_ctx->sns_image_mode.height   = ot_ext_top_sensor_height_read(vi_pipe);
    isp_ctx->sns_image_mode.sns_mode = ot_ext_system_sensor_mode_read(vi_pipe);
    isp_ctx->sys_rect.x = ot_ext_system_corp_pos_x_read(vi_pipe);
    isp_ctx->sys_rect.y = ot_ext_system_corp_pos_y_read(vi_pipe);

    if (isp_ctx->isp_image_mode_flag == 0) {
        isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_AE_FPS_BASE_SET, (td_void *)&isp_ctx->sns_image_mode.fps);

        (td_void)memcpy_s(&isp_ctx->pre_sns_image_mode, sizeof(isp_sensor_image_mode),
                          &isp_ctx->sns_image_mode, sizeof(isp_sensor_image_mode));

        isp_ctx->isp_image_mode_flag = 1;
    } else {
        if (isp_ctx->sns_wdr_mode != wdr_mode) {
            isp_ctx->change_wdr_mode = TD_TRUE;
        }

        if ((isp_ctx->sns_image_mode.width  != isp_ctx->pre_sns_image_mode.width) ||
            (isp_ctx->sns_image_mode.height != isp_ctx->pre_sns_image_mode.height) ||
            (is_float_equal(isp_ctx->sns_image_mode.fps, isp_ctx->pre_sns_image_mode.fps) == TD_FALSE) ||
            (isp_ctx->sns_image_mode.sns_mode != isp_ctx->pre_sns_image_mode.sns_mode)) {
            isp_ctx->change_image_mode = TD_TRUE;
        }

        if (isp_ctx->sys_rect.width  != ot_ext_sync_total_width_read(vi_pipe) ||
            isp_ctx->sys_rect.height != ot_ext_sync_total_height_read(vi_pipe)) {
            isp_ctx->sys_rect.width  = ot_ext_sync_total_width_read(vi_pipe);
            isp_ctx->sys_rect.height = ot_ext_sync_total_height_read(vi_pipe);

            isp_ctx->change_isp_res = TD_TRUE;
        }

        if (isp_ctx->change_wdr_mode || isp_ctx->change_image_mode || isp_ctx->change_isp_res) {
            /* stitch mode, should switch main pipe first */
            if (isp_stitch_main_pipe_switch_finished(isp_ctx) != TD_TRUE) {
                return TD_SUCCESS;
            }

            isp_switch_mode_process(vi_pipe, isp_ctx, wdr_mode);
        }

        isp_ctx->change_wdr_mode   = TD_FALSE;
        isp_ctx->change_image_mode = TD_FALSE;
        isp_ctx->change_isp_res    = TD_FALSE;
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_mcf_enable_get(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 ret;
    td_bool mcf_enable = TD_FALSE;

    isp_get_ctx(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_MCF_EN_GET, &mcf_enable);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] work mode get failed!\n", vi_pipe);
        return ret;
    }

    isp_ctx->is_ia_nr_enable = mcf_enable;

    return TD_SUCCESS;
}

static td_s32 isp_update_pos_get(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 ret;

    isp_get_ctx(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_UPDATE_POS_GET, &isp_ctx->linkage.update_pos);
    if (ret) {
        isp_err_trace("pipe:%d get update pos %x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}

td_u32 isp_frame_cnt_get(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 ret;
    td_u32 frm_cnt = 0;
    isp_get_ctx(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_FRAME_CNT_GET, &frm_cnt);
    if (ret) {
        isp_err_trace("pipe:%d get update pos %x!\n", vi_pipe, ret);
        return isp_ctx->linkage.iso_done_frm_cnt;
    }

    return frm_cnt;
}
#ifdef CONFIG_OT_SNAP_SUPPORT
td_s32 isp_snap_attr_get(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    isp_snap_attr snap_attr;
    isp_working_mode isp_work_mode;

    isp_check_pointer_return(isp_ctx);
    ret = ioctl(isp_get_fd(vi_pipe), ISP_SNAP_ATTR_GET, &snap_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    (td_void)memcpy_s(&isp_ctx->pro_param, sizeof(isp_pro_param), &snap_attr.pro_param, sizeof(isp_pro_param));
    isp_ctx->linkage.snap_type = snap_attr.snap_type;
    if ((snap_attr.picture_pipe_id == snap_attr.preview_pipe_id) && (snap_attr.preview_pipe_id != -1)) {
        isp_ctx->linkage.snap_pipe_mode = ISP_SNAP_PREVIEW_PICTURE;
    } else if (snap_attr.picture_pipe_id == vi_pipe) {
        isp_ctx->linkage.snap_pipe_mode = ISP_SNAP_PICTURE;
    } else if (snap_attr.preview_pipe_id == vi_pipe) {
        isp_ctx->linkage.snap_pipe_mode = ISP_SNAP_PREVIEW;
    } else {
        isp_ctx->linkage.snap_pipe_mode = ISP_SNAP_NONE;
    }
    isp_ctx->linkage.load_ccm = snap_attr.load_ccm;
    isp_ctx->linkage.picture_pipe_id = snap_attr.picture_pipe_id;
    isp_ctx->linkage.preview_pipe_id = snap_attr.preview_pipe_id;

    if (snap_attr.picture_pipe_id != -1) {
        ret = ioctl(isp_get_fd(snap_attr.picture_pipe_id), ISP_WORK_MODE_GET, &isp_work_mode);
        if (ret) {
            isp_err_trace("get isp work mode failed!\n");
            return ret;
        }
        isp_ctx->linkage.picture_running_mode = isp_work_mode.running_mode;
    } else {
        isp_ctx->linkage.picture_running_mode = ISP_MODE_RUNNING_BUTT;
    }

    if (snap_attr.picture_pipe_id == snap_attr.preview_pipe_id) {
        isp_ctx->linkage.preview_running_mode = isp_ctx->linkage.picture_running_mode;
    } else {
        if (snap_attr.preview_pipe_id != -1) {
            ret = ioctl(isp_get_fd(snap_attr.preview_pipe_id), ISP_WORK_MODE_GET, &isp_work_mode);
            if (ret) {
                isp_err_trace("get isp work mode failed!\n");
                return ret;
            }
            isp_ctx->linkage.preview_running_mode = isp_work_mode.running_mode;
        } else {
            isp_ctx->linkage.preview_running_mode = ISP_MODE_RUNNING_BUTT;
        }
    }

    return TD_SUCCESS;
}
#endif
static td_u8 isp_iso_index_cal(ot_vi_pipe vi_pipe, td_u32 ae_done_frm_cnt)
{
    td_u8  index;
    td_u32 cnt_diff;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    index = isp_ctx->linkage.cfg2valid_delay_max;

    if (isp_ctx->linkage.iso_done_frm_cnt > ae_done_frm_cnt) {  /* preview pipe the last frame dont run complete. */
        cnt_diff = isp_ctx->linkage.iso_done_frm_cnt - ae_done_frm_cnt;
        index = index - cnt_diff;
    } else if (isp_ctx->linkage.iso_done_frm_cnt < ae_done_frm_cnt) { /* the preview pipe run first. */
        index = index + (ae_done_frm_cnt - isp_ctx->linkage.iso_done_frm_cnt - 1);
    } else if (isp_ctx->linkage.iso_done_frm_cnt == ae_done_frm_cnt) { /* the picture pipe run first. */
        index = index - 1;
    }

    if (isp_ctx->linkage.update_pos > 0) {
        index = index - 1;
    }

    index = MIN2(index, ISP_SYNC_ISO_BUF_MAX - 1);

    return index;
}

static td_u8 isp_ai_wb_index_cal(ot_vi_pipe vi_pipe, td_u32 ae_done_frm_cnt)
{
    td_u8  index;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->linkage.iso_done_frm_cnt < ae_done_frm_cnt) {
        index = ae_done_frm_cnt - isp_ctx->linkage.iso_done_frm_cnt - 1;
    } else  { /* the ai pipe run first. */
        index = 0;
    }

    index = MIN2(index, ISP_SYNC_ISO_BUF_MAX - 1);

    return index;
}

#ifdef CONFIG_OT_SNAP_SUPPORT
static td_void isp_snap_preview_process(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_u8 i;
    ot_snap_type snap_type = isp_ctx->linkage.snap_type;
    isp_snap_pipe_mode snap_pipe_mode = isp_ctx->linkage.snap_pipe_mode;

    if ((snap_pipe_mode == ISP_SNAP_PREVIEW) || (snap_pipe_mode == ISP_SNAP_PREVIEW_PICTURE)) {
        if (snap_type == OT_SNAP_TYPE_PRO) {
            isp_ctx->linkage.pro_trigger = isp_pro_trigger_get(vi_pipe);
            if (isp_ctx->linkage.pro_trigger == TD_TRUE) {
                isp_algs_ctrl(isp_ctx->algs, vi_pipe, OT_ISP_PROTRIGGER_SET, (td_void *)&isp_ctx->pro_param);
            }
        }

        for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
            isp_ctx->linkage.pro_index_buf[i] = isp_ctx->linkage.pro_index_buf[i - 1];
        }
        if ((isp_ctx->linkage.pro_trigger == TD_TRUE) ||
            ((isp_ctx->pro_frm_cnt > 0) && (isp_ctx->pro_frm_cnt < isp_ctx->pro_param.pro_frame_num))) {
            isp_ctx->linkage.pro_index_buf[0] = isp_ctx->pro_frm_cnt + 1;
        } else {
            isp_ctx->linkage.pro_index_buf[0] = 0;
        }
        isp_ctx->linkage.pro_index = isp_ctx->linkage.pro_index_buf[isp_ctx->linkage.cfg2valid_delay_max];
        if (isp_ctx->linkage.update_pos > 0) {
            isp_ctx->linkage.pro_index = isp_ctx->linkage.pro_index_buf[isp_ctx->linkage.cfg2valid_delay_max - 1];
        }
    }
}
#endif
static td_s32 isp_snap_pre_process(ot_vi_pipe vi_pipe)
{
#ifdef CONFIG_OT_SNAP_SUPPORT
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_usr_ctx *preview_isp_ctx = TD_NULL;
    isp_snap_pipe_mode snap_pipe_mode;
    td_u8 index;
    td_u32 pre_pipe_frm_cnt = 0;

    isp_get_ctx(vi_pipe, isp_ctx);

    ret = isp_snap_attr_get(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    snap_pipe_mode = isp_ctx->linkage.snap_pipe_mode;

    if (snap_pipe_mode == ISP_SNAP_NONE) {
        return TD_SUCCESS;
    }

    if ((isp_ctx->linkage.preview_pipe_id > -1) && (isp_ctx->linkage.preview_pipe_id < OT_ISP_MAX_PIPE_NUM)) {
        isp_get_ctx(isp_ctx->linkage.preview_pipe_id, preview_isp_ctx);
        pre_pipe_frm_cnt = preview_isp_ctx->linkage.iso_done_frm_cnt;
    }

    isp_snap_preview_process(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        if (snap_pipe_mode == ISP_SNAP_PICTURE) {
            index = isp_iso_index_cal(vi_pipe, pre_pipe_frm_cnt);
            if (preview_isp_ctx != TD_NULL) {
                isp_ctx->linkage.iso = preview_isp_ctx->linkage.sync_iso_buf[index];
                isp_ctx->linkage.pro_index = preview_isp_ctx->linkage.pro_index_buf[index];
            }
        }
    }
#else
    ot_unused(vi_pipe);
#endif
    return TD_SUCCESS;
}

static td_void isp_ai_process(ot_vi_pipe vi_pipe)
{
    td_u8 index, i;
    td_u32 base_pipe_frm_cnt;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_usr_ctx *isp_base_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if ((isp_ctx->ai_info.pq_ai_en == TD_FALSE) ||
        ((isp_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == isp_ctx->ai_info.base_pipe_id))) {
        return;
    }

    if (vi_pipe == isp_ctx->ai_info.ai_pipe_id) {
        isp_get_ctx(isp_ctx->ai_info.base_pipe_id, isp_base_ctx);
        base_pipe_frm_cnt = isp_base_ctx->linkage.iso_done_frm_cnt;
        index = isp_iso_index_cal(vi_pipe, base_pipe_frm_cnt);

        isp_ctx->linkage.iso        = isp_base_ctx->linkage.sync_iso_buf[index];
        isp_ctx->linkage.isp_dgain  = isp_base_ctx->linkage.sync_isp_dgain_buf[index];
        isp_ctx->linkage.sensor_iso = ((td_u64)isp_ctx->linkage.iso << 0x8) / div_0_to_1(isp_ctx->linkage.isp_dgain);

        index = isp_ai_wb_index_cal(vi_pipe, base_pipe_frm_cnt);
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            isp_ctx->linkage.white_balance_gain[i] = isp_base_ctx->linkage.sync_wb_gain[index][i];
        }

        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            isp_ctx->linkage.ccm[i] = isp_base_ctx->linkage.sync_ccm[index][i];
        }

        isp_ctx->linkage.color_temp = isp_base_ctx->linkage.sync_color_temp[index];
    }

    return;
}

static td_s32 isp_set_pro_calc_done(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_SET_PROCALCDONE);
    if (ret) {
        isp_err_trace("set isp pro calculate done failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_stitch_sync_exit(ot_vi_pipe vi_pipe)
{
    td_u32  k;
    ot_vi_pipe vi_pipe_s;
    isp_usr_ctx *isp_ctx = TD_NULL;

    /* 1. check status */
    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);

    if ((isp_ctx->stitch_attr.stitch_enable == TD_TRUE) && (isp_ctx->stitch_attr.main_pipe != TD_TRUE)) {
        for (k = 0; k < isp_ctx->stitch_attr.stitch_pipe_num; k++) {
            vi_pipe_s = isp_ctx->stitch_attr.stitch_bind_id[k];
            isp_get_ctx(vi_pipe_s, isp_ctx);

            while ((isp_ctx->stitch_attr.main_pipe == TD_TRUE) && (isp_ctx->mem_init != TD_FALSE)) {
                usleep(10); /* sleep 10us */
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_stitch_sync_run(ot_vi_pipe vi_pipe)
{
    td_u8 k;
    ot_vi_pipe vi_pipe_id;
    isp_usr_ctx *isp_ctx   = TD_NULL;
    isp_usr_ctx *isp_ctx_s = TD_NULL;

    /* 1. check status */
    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);

    if (isp_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        return TD_SUCCESS;
    }

    if (isp_ctx->stitch_attr.main_pipe == TD_TRUE) {
        for (k = 0; k < isp_ctx->stitch_attr.stitch_pipe_num; k++) {
            vi_pipe_id = isp_ctx->stitch_attr.stitch_bind_id[k];
            isp_get_ctx(vi_pipe_id, isp_ctx_s);

            if (isp_ctx_s->mem_init != TD_TRUE) {
                return TD_FAILURE;
            }

            if (isp_ctx_s->para_rec.init != TD_TRUE) {
                return TD_FAILURE;
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_stitch_iso_sync(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_usr_ctx *main_isp_ctx = TD_NULL;
    td_u8 index;
    td_u32 main_pipe_frm_cnt;
    ot_vi_pipe main_pipe;
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        if (isp_ctx->stitch_attr.main_pipe != TD_TRUE) {
            main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];
            isp_check_pipe_return(main_pipe);
            isp_get_ctx(vi_pipe, main_isp_ctx);

            main_pipe_frm_cnt = main_isp_ctx->linkage.iso_done_frm_cnt;
            index = isp_iso_index_cal(vi_pipe, main_pipe_frm_cnt);
            isp_ctx->linkage.iso = main_isp_ctx->linkage.sync_iso_buf[index];
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_statistics_get(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, td_void **stat)
{
    td_s32 ret;
    ret = isp_statistics_get_buf(vi_pipe, stat);
    if (ret != TD_SUCCESS) {
        if (isp_ctx->frame_cnt != 0) {
            return ret;
        }

        isp_ctx->linkage.stat_ready  = TD_FALSE;
    } else {
        isp_ctx->linkage.stat_ready  = TD_TRUE;
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_trans_info(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
#ifdef CONFIG_OT_SNAP_SUPPORT
    if (isp_ctx->linkage.snap_pipe_mode != ISP_SNAP_PICTURE) {
        ret = isp_update_info(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        ret = isp_update_frame_info(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }
#else
    ret = isp_update_info(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_update_frame_info(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_unused(isp_ctx);
#endif

    ret = isp_update_attach_info(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_update_color_gamut_info(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void isp_update_pro_frm_cnt(isp_usr_ctx *isp_ctx)
{
#ifdef CONFIG_OT_SNAP_SUPPORT
    if (((isp_ctx->pro_frm_cnt > 0) &&
         (isp_ctx->pro_frm_cnt < isp_ctx->pro_param.pro_frame_num + 4)) || /* const value 4 */
        (isp_ctx->linkage.pro_trigger == TD_TRUE)) {
        isp_ctx->pro_frm_cnt++;
    } else {
        isp_ctx->pro_frm_cnt = 0;
    }
#else
    isp_ctx->pro_frm_cnt = 0;
#endif
}

static td_s32 isp_sync_cfg_config(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    if ((isp_ctx->frame_cnt % div_0_to_1(isp_ctx->linkage.ae_run_interval) == 0) || (isp_ctx->pro_frm_cnt > 0)) {
        if (!isp_ctx->linkage.defect_pixel) {
            ret = isp_sync_cfg_set(vi_pipe);
            if (ret != TD_SUCCESS) {
                return TD_SUCCESS;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_alg_buf_init(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;

    ret = isp_alg_stats_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail00;
    }
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    ret = isp_detail_stats_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail01;
    }
#endif
    ret = isp_cfg_be_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail02;
    }

    ret = isp_get_be_buf_first(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail03;
    }

    ret = isp_be_vreg_addr_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail03;
    }

    ret = isp_stt_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail03;
    }

    ret = isp_stt_addr_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail04;
    }

    ret = isp_clut_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail04;
    }

    /* init statistics bufs. */
    ret = isp_statistics_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail05;
    }

    /* init proc bufs. */
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    ret = isp_proc_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail06;
    }
#endif
    /* init trans info bufs. */
    ret = isp_trans_info_init(vi_pipe, &isp_ctx->isp_trans_info);
    if (ret != TD_SUCCESS) {
        goto fail07;
    }

    ret = isp_pro_info_init(vi_pipe, &isp_ctx->isp_pro_info);
    if (ret != TD_SUCCESS) {
        goto fail08;
    }

    return TD_SUCCESS;

fail08:
    isp_trans_info_exit(vi_pipe);
fail07:
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    isp_proc_exit(vi_pipe);
fail06:
#endif
    isp_statistics_exit(vi_pipe);
fail05:
    isp_clut_buf_exit(vi_pipe);
fail04:
    isp_stt_buf_exit(vi_pipe);
fail03:
    isp_cfg_be_buf_exit(vi_pipe);
fail02:
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_detail_stats_buf_exit(vi_pipe);
fail01:
#endif
    isp_alg_stats_buf_exit(vi_pipe);
fail00:
    return OT_ERR_ISP_NOT_INIT;
}

td_void isp_alg_buf_exit(ot_vi_pipe vi_pipe)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_detail_stats_buf_exit(vi_pipe);
#endif
    isp_pro_info_exit(vi_pipe);
    isp_trans_info_exit(vi_pipe);
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    isp_proc_exit(vi_pipe);
#endif
    isp_statistics_exit(vi_pipe);
    isp_clut_buf_exit(vi_pipe);
    isp_cfg_be_buf_exit(vi_pipe);
    isp_stt_buf_exit(vi_pipe);
    isp_alg_stats_buf_exit(vi_pipe);
}

static td_void isp_get_actucal_black_level_dynamic_mode(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_s32 ret;
    ot_isp_black_level_mode black_level_mode = ot_ext_system_black_level_mode_read(vi_pipe);
    if (black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_DYNAMIC_ACTUAL_INFO_GET, &isp_ctx->actual_blc);
    if (ret != TD_SUCCESS) {
        return;
    }

    if (isp_ctx->actual_blc.is_ready == TD_TRUE) {
        black_level_actual_value_write(vi_pipe, &isp_ctx->actual_blc);
    }
}

static td_void isp_algs_run_process(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, td_void *stat)
{
    td_void *reg_cfg = TD_NULL;

    isp_get_actucal_black_level_dynamic_mode(vi_pipe, isp_ctx);
    isp_get_fe_roi_config(vi_pipe, &isp_ctx->fe_roi_cfg);

    /* get regcfg */
    isp_get_reg_cfg_ctx(vi_pipe, &reg_cfg);

    isp_algs_run(isp_ctx->algs, vi_pipe, stat, reg_cfg, 0);
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
    isp_proc_write(isp_ctx->algs, vi_pipe);
#endif
}

td_s32 isp_run(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_void *stat = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    /* wait all pipe is initial done when pipe is in stitch mode, do not need run every frame */
    ret = isp_stitch_sync_run(vi_pipe);
    if (ret != TD_SUCCESS) {
        return TD_SUCCESS;
    }
    /* update ctx info from vi */
    ret = isp_update_mcf_enable_get(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /*  get isp be_buf info. */
    ret = isp_get_be_free_buf(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /*  get statistics buf info. */
    ret = isp_statistics_get(vi_pipe, isp_ctx, &stat);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* only used stitch mode */
    ret = isp_set_cfg_be_buf_state(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_read_debug_ext_regs(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set debug reg failure\n");
    }
    isp_blc_read_ext_regs(vi_pipe);

    if (isp_ctx->freeze_fw) {
        isp_reg_cfg_info_set(vi_pipe);
        return TD_SUCCESS;
    }

    ret = isp_update_pos_get(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_ai_process(vi_pipe);

    ret = isp_snap_pre_process(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_stitch_iso_sync(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_ctx->frame_cnt++;

    isp_blc_dbg_run_bgn(&isp_ctx->isp_dbg, isp_ctx->frame_cnt);

    isp_algs_run_process(vi_pipe, isp_ctx, stat);

    /* update info */
    ret = isp_update_trans_info(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_blc_dbg_run_end(vi_pipe, &isp_ctx->isp_dbg, isp_ctx->frame_cnt);

    /* release statistics buf info. */
    if (isp_ctx->linkage.stat_ready  == TD_TRUE) {
        isp_statistics_put_buf(vi_pipe);
    }

    /* record the register config information to fhy and kernel,and be valid in next frame. */
    ret = isp_reg_cfg_info_set(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_update_pro_frm_cnt(isp_ctx);
    ret = isp_sync_cfg_config(vi_pipe, isp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_update_dng_image_dynamic_info(vi_pipe);

    /* pro snap mode, ae calculate done */
    if (isp_ctx->pro_frm_cnt == 1) {
        isp_set_pro_calc_done(vi_pipe);
    }

    return TD_SUCCESS;
}

static td_void isp_run_by_be_end(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    td_s32 ret;
    ot_vi_pipe vi_pipe_s;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->stitch_attr.stitch_enable) {
        if (vi_pipe != isp_ctx->stitch_attr.stitch_bind_id[0]) {
            return;
        }

        for (i = 0; i < isp_ctx->stitch_attr.stitch_pipe_num; i++) {
            vi_pipe_s = isp_ctx->stitch_attr.stitch_bind_id[i];
            isp_mutex_lock(vi_pipe_s);
            isp_run(vi_pipe_s);
            isp_mutex_unlock(vi_pipe_s);
        }

        isp_mutex_lock(vi_pipe);
        ret = ioctl(isp_get_fd(vi_pipe), ISP_KERNEL_RUN_TRIGGER);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP main pipe[%d] strigger failed!\n", vi_pipe);
        }
        isp_mutex_unlock(vi_pipe);
    } else {
        isp_mutex_lock(vi_pipe);
        isp_run(vi_pipe);
        ret = ioctl(isp_get_fd(vi_pipe), ISP_KERNEL_RUN_TRIGGER);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] strigger failed!\n", vi_pipe);
        }
        isp_mutex_unlock(vi_pipe);
    }
}

td_s32 isp_run_thread_proc(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u32 int_status = 0;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    isp_mutex_lock(vi_pipe);

    /* change image mode (WDR mode or resolution) */
    ret = isp_switch_mode(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] switch mode failed!\n", vi_pipe);
        isp_mutex_unlock(vi_pipe);
        return ret;
    }

    isp_mutex_unlock(vi_pipe);

    /* waked up by the interrupt */
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_FRAME_EDGE, &int_status);
    if (ret == TD_SUCCESS) {
        if (isp_ctx->run_thread_status != ISP_RUN_THREAD_STATUS_RUN) {
            return TD_SUCCESS;
        }
        /* isp firmware calculate, include AE/AWB, etc. */
        if (int_status & ISP_INT_BE_END) {
            isp_run_by_be_end(vi_pipe);
        } else {
            isp_mutex_lock(vi_pipe);
            isp_run(vi_pipe);
            isp_mutex_unlock(vi_pipe);
        }
    }

    return TD_SUCCESS;
}

static td_void isp_global_ctx_reset(isp_usr_ctx *isp_ctx)
{
    td_u8  i;
    isp_ctx->mem_init                     = TD_FALSE;
    isp_ctx->is_vreg_checked              = TD_FALSE;
    isp_ctx->para_rec.init                = TD_FALSE;
    isp_ctx->para_rec.stitch_sync         = TD_FALSE;
    isp_ctx->linkage.run_once             = TD_FALSE;
    isp_ctx->linkage.blc_fix.enable = TD_FALSE;
    isp_ctx->linkage.blc_fix.blc = 1024; // default 1024
#ifdef CONFIG_OT_SNAP_SUPPORT
    isp_ctx->linkage.snap_pipe_mode       = ISP_SNAP_NONE;
    isp_ctx->linkage.preview_pipe_id      = -1;
    isp_ctx->linkage.picture_pipe_id      = -1;
    isp_ctx->linkage.preview_running_mode = ISP_MODE_RUNNING_OFFLINE;
    isp_ctx->linkage.picture_running_mode = ISP_MODE_RUNNING_OFFLINE;
    isp_ctx->linkage.snap_type            = OT_SNAP_TYPE_NORM;
    isp_ctx->linkage.snap_state           = TD_FALSE;
#endif
    for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
        isp_ctx->isp_pre_be_virt_addr[i]  = TD_NULL;
        isp_ctx->isp_post_be_virt_addr[i] = TD_NULL;
        isp_ctx->pre_viproc_virt_addr[i]  = TD_NULL;
        isp_ctx->post_viproc_virt_addr[i] = TD_NULL;
    }
    isp_ctx->isp_be_ae_virt_addr = TD_NULL;
    isp_ctx->isp_be_awb_virt_addr = TD_NULL;
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    (td_void)memset_s(&isp_ctx->detail_stats_info, sizeof(isp_detail_stats_info), 0, sizeof(isp_detail_stats_info));
#endif
}

td_s32 isp_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    const td_u64 handsignal = ISP_EXIT_HAND_SIGNAL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);
    isp_check_open_return(vi_pipe);

    /* set handsignal */
    if (ioctl(isp_get_fd(vi_pipe), ISP_RUN_STATE_SET, &handsignal)) {
        isp_err_trace("ISP[%d] set run state failed!\n", vi_pipe);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    (td_void)memset_s(&isp_ctx->para_rec, sizeof(isp_para_rec), 0, sizeof(isp_para_rec));

    ot_ext_top_wdr_cfg_write(vi_pipe, isp_ctx->para_rec.wdr_cfg);
    ot_ext_top_pub_attr_cfg_write(vi_pipe, isp_ctx->para_rec.pub_cfg);

    if (isp_ctx->isp_yuv_mode == TD_FALSE) {
        isp_sensor_exit(vi_pipe);
    }

    isp_algs_exit(vi_pipe, isp_ctx->algs);

    isp_algs_unregister(vi_pipe);

    isp_reg_cfg_exit(vi_pipe);
    isp_alg_buf_exit(vi_pipe);
    isp_block_exit(vi_pipe);
    /* exit global variables */
    isp_global_ctx_reset(isp_ctx);

    if (ioctl(isp_get_fd(vi_pipe), ISP_RESET_CTX)) {
        isp_err_trace("ISP[%d] reset ctx failed!\n", vi_pipe);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    /* release vregs */
    vreg_exit(vi_pipe, isp_vir_reg_base(vi_pipe), ISP_VREG_SIZE);
    vreg_release_all(vi_pipe);

    return TD_SUCCESS;
}
