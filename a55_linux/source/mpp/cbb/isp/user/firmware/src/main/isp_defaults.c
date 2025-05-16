/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_defaults.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_sensor.h"
#include "isp_ext_reg_access.h"
#include "isp_math_utils.h"
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
#include "isp_dump_dbg.h"
#endif

td_void isp_regs_default(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);
    return;
}

td_void isp_regs_initialize(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);
    return;
}

/* the ext regs is a block of memory, not real register, need a default value. */
td_void isp_ext_regs_default(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    ot_ext_system_freeze_firmware_write(vi_pipe, OT_EXT_SYSTEM_FREEZE_FIRMWARE_DEFAULT);

    ot_ext_system_sys_debug_enable_write(vi_pipe, OT_EXT_SYSTEM_SYS_DEBUG_ENABLE_DEFAULT);
    ot_ext_system_sys_debug_high_addr_write(vi_pipe, OT_EXT_SYSTEM_SYS_DEBUG_HIGH_ADDR_DEFAULT);
    ot_ext_system_sys_debug_low_addr_write(vi_pipe, OT_EXT_SYSTEM_SYS_DEBUG_LOW_ADDR_DEFAULT);
    ot_ext_system_sys_debug_size_write(vi_pipe, OT_EXT_SYSTEM_SYS_DEBUG_SIZE_DEFAULT);
    ot_ext_system_sys_debug_depth_write(vi_pipe, OT_EXT_SYSTEM_DEBUG_DEPTH_DEFAULT);
    ot_ext_system_statistics_ctrl_lowbit_write(vi_pipe, OT_EXT_SYSTEM_STATISTICS_LOWBIT_DEFAULT);
    ot_ext_system_statistics_ctrl_highbit_write(vi_pipe, OT_EXT_SYSTEM_STATISTICS_HIGHBIT_DEFAULT);
    ot_ext_system_be_free_buffer_high_addr_write(vi_pipe, OT_EXT_SYSTEM_BE_FREE_BUFFER_HIGH_ADDR_DEFAULT);
    ot_ext_system_be_free_buffer_low_addr_write(vi_pipe, OT_EXT_SYSTEM_BE_FREE_BUFFER_LOW_ADDR_DEFAULT);
    ot_ext_system_be_lut_stt_buffer_high_addr_write(vi_pipe, OT_EXT_SYSTEM_BE_LUT_STT_BUFFER_HIGH_ADDR_DEFAULT);
    ot_ext_system_be_lut_stt_buffer_low_addr_write(vi_pipe, OT_EXT_SYSTEM_BE_LUT_STT_BUFFER_LOW_ADDR_DEFAULT);
    ot_ext_system_ldci_read_stt_buffer_low_addr_write(vi_pipe, OT_EXT_SYSTEM_LDCI_READ_STT_BUF_LOW_ADDR_DEFAULT);
    ot_ext_system_ldci_read_stt_buffer_high_addr_write(vi_pipe, OT_EXT_SYSTEM_LDCI_READ_STT_BUF_HIGH_ADDR_DEFAULT);
    ot_ext_system_top_channel_select_write(vi_pipe, OT_EXT_SYSTEM_TOP_CHANNEL_SELECT_WRITE_DEFAULT);
    ot_ext_system_top_channel_select_pre_write(vi_pipe, OT_EXT_SYSTEM_TOP_CHANNEL_SELECT_PRE_WRITE_DEFAULT);
    ot_ext_system_isp_dump_frame_pos_write(vi_pipe, OT_ISP_DUMP_FRAME_POS_NORMAL);
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
    isp_dump_dbg_ext_regs_defalut(vi_pipe);
#endif
    ot_ext_system_fe_roi_en_write(vi_pipe, TD_FALSE);
    ot_ext_system_fe_roi_x_write(vi_pipe, 0);
    ot_ext_system_fe_roi_y_write(vi_pipe, 0);
    ot_ext_system_fe_roi_width_write(vi_pipe, isp_ctx->sys_rect.width);
    ot_ext_system_fe_roi_height_write(vi_pipe, isp_ctx->sys_rect.height);
    isp_get_fe_roi_config(vi_pipe, &isp_ctx->fe_roi_cfg);

    /* isp pipe different configs for stitch mode */
    ot_ext_system_isp_pipe_diff_type_write(vi_pipe, OT_EXT_SYSTEM_ISP_PIPE_DIFF_TYPE_DEFAULT);

    /* isp pipe different configs for stitch mode */
    ot_ext_system_isp_pipe_diff_overlap_write(vi_pipe, OT_EXT_SYSTEM_ISP_PIPE_DIFF_OVERLAP_DEFAULT);

    /* isp pipe different configs for stitch mode */
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ot_ext_system_isp_pipe_diff_offset_write(vi_pipe, i, OT_EXT_SYSTEM_ISP_PIPE_DIFF_OFFSET_DEFAULT);
        ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, i, OT_EXT_SYSTEM_ISP_PIPE_DIFF_GAIN_DEFAULT);
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        if (i % 4 == 0) {   /* const value 4 */
            ot_ext_system_isp_pipe_diff_ccm_write(vi_pipe, i, OT_EXT_SYSTEM_ISP_PIPE_DIFF_CCM_DEFAULT);
        } else {
            ot_ext_system_isp_pipe_diff_ccm_write(vi_pipe, i, 0);
        }
    }

    return;
}

td_void isp_ext_regs_initialize(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_u32 isp_bind_attr;
    ot_isp_cmos_sns_max_resolution *sns_max_resolution = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_sensor_get_max_resolution(vi_pipe, &sns_max_resolution);
    ot_ext_system_sensor_max_resolution_width_write(vi_pipe, sns_max_resolution->max_width);
    ot_ext_system_sensor_max_resolution_height_write(vi_pipe, sns_max_resolution->max_height);

    /* update isp bind attr to ext regs */
    isp_bind_attr = signed_left_shift(isp_ctx->bind_attr.ae_lib.id, 8) + isp_ctx->bind_attr.awb_lib.id; /* bit8~15:ae */

    ot_ext_system_bind_attr_write(vi_pipe, isp_bind_attr);

    ot_ext_system_block_num_write(vi_pipe, isp_ctx->block_attr.block_num);

    return;
}

static td_void isp_alg_key_init(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_ctx->alg_key.key = 0xFFFFFFFFFFFFFFFF;
}

static td_void isp_snap_param_initialize(isp_usr_ctx *isp_ctx)
{
#ifdef CONFIG_OT_SNAP_SUPPORT
    td_u8 i;

    isp_check_pointer_void_return(isp_ctx);
    isp_ctx->linkage.snap_pipe_mode       = ISP_SNAP_NONE;
    isp_ctx->linkage.preview_pipe_id      = -1;
    isp_ctx->linkage.picture_pipe_id      = -1;
    isp_ctx->linkage.preview_running_mode = ISP_MODE_RUNNING_OFFLINE;
    isp_ctx->linkage.picture_running_mode = ISP_MODE_RUNNING_OFFLINE;
    isp_ctx->linkage.snap_type            = OT_SNAP_TYPE_NORM;

    isp_ctx->linkage.snap_state       = TD_FALSE;
    isp_ctx->linkage.pro_trigger      = TD_FALSE;
    isp_ctx->pro_frm_cnt              = 0;
    isp_ctx->linkage.iso_done_frm_cnt = 0;
    isp_ctx->linkage.pro_index        = 0;

    for (i = 0; i < ISP_SYNC_ISO_BUF_MAX; i++) {
        isp_ctx->linkage.sync_iso_buf[i] = 100; /* 100: default iso */
        isp_ctx->linkage.pro_index_buf[i] = 0;
    }
#else
    ot_unused(isp_ctx);
#endif
}

td_void isp_global_initialize(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    td_u8   i;
    td_u32  value;
    td_void *value_tmp = TD_NULL;

    isp_ctx->sns_image_mode.width  = ot_ext_top_sensor_width_read(vi_pipe);
    isp_ctx->sns_image_mode.height = ot_ext_top_sensor_height_read(vi_pipe);
    value = ot_ext_system_fps_base_read(vi_pipe);
    value_tmp = (td_void *)&value;
    isp_ctx->sns_image_mode.fps = *(td_float *)value_tmp;

    isp_ctx->pre_sns_image_mode.width  = isp_ctx->sns_image_mode.width;
    isp_ctx->pre_sns_image_mode.height = isp_ctx->sns_image_mode.height;
    isp_ctx->pre_sns_image_mode.fps    = isp_ctx->sns_image_mode.fps;

    isp_ctx->change_wdr_mode   = TD_FALSE;
    isp_ctx->change_image_mode = TD_FALSE;
    isp_ctx->change_isp_res    = TD_FALSE;
    isp_ctx->sns_wdr_mode      = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    isp_ctx->pre_sns_wdr_mode  = isp_ctx->sns_wdr_mode;

    isp_ctx->be_frame_attr.frame_pos = OT_ISP_DUMP_FRAME_POS_NORMAL;
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
    isp_dump_dbg_init(&isp_ctx->be_pos_attr);
#endif
    isp_ctx->frame_cnt           = 0;
    isp_ctx->isp_image_mode_flag = 0;

    isp_ctx->linkage.defect_pixel = TD_FALSE;
    isp_ctx->freeze_fw            = TD_FALSE;

    isp_snap_param_initialize(isp_ctx);

    isp_ctx->linkage.stat_ready  = TD_FALSE;
    isp_ctx->linkage.run_once    = TD_FALSE;
    isp_ctx->linkage.cfg2valid_delay_max = 2;  /* 2:default delay max */
    isp_ctx->linkage.fswdr_mode          = OT_ISP_FSWDR_NORMAL_MODE;
    isp_ctx->linkage.pre_fswdr_mode      = OT_ISP_FSWDR_NORMAL_MODE;
    isp_ctx->linkage.blc_fix.enable = TD_FALSE;
    isp_ctx->linkage.blc_fix.blc = 1024; // default 1024

    for (i = 0; i < ISP_MAX_BE_NUM; i++) {
        isp_ctx->special_opt.be_on_stt_update[i] = TD_TRUE;
    }
    isp_ctx->special_opt.fe_stt_update = TD_TRUE;
    (td_void)memset_s(&isp_ctx->pre_dng_ccm, sizeof(isp_dng_ccm), 0, sizeof(isp_dng_ccm));
    (td_void)memset_s(&isp_ctx->pre_dng_color_param, sizeof(ot_isp_dng_color_param), 0, sizeof(ot_isp_dng_color_param));
    (td_void)memset_s(&isp_ctx->dcf_update_info, sizeof(ot_isp_dcf_update_info), 0, sizeof(ot_isp_dcf_update_info));
    (td_void)memset_s(&isp_ctx->frame_info, sizeof(ot_isp_frame_info), 0, sizeof(ot_isp_frame_info));

    isp_alg_key_init(vi_pipe);
}

static td_void isp_check_dng_color_param(ot_isp_dng_color_param *dng_param)
{
    if (dng_param->wb_gain1.b_gain > 0xFFF) {
        isp_err_trace("wb_gain1.b_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain1.b_gain = 0xFFF;
    }
    if (dng_param->wb_gain1.g_gain > 0xFFF) {
        isp_err_trace("wb_gain1.g_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain1.g_gain = 0xFFF;
    }
    if (dng_param->wb_gain1.r_gain > 0xFFF) {
        isp_err_trace("wb_gain1.g_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain1.r_gain = 0xFFF;
    }
    if (dng_param->wb_gain2.b_gain > 0xFFF) {
        isp_err_trace("wb_gain2.b_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain2.b_gain = 0xFFF;
    }
    if (dng_param->wb_gain2.g_gain > 0xFFF) {
        isp_err_trace("wb_gain2.g_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain2.g_gain = 0xFFF;
    }
    if (dng_param->wb_gain2.r_gain > 0xFFF) {
        isp_err_trace("wb_gain2.r_gain can't bigger than 0xFFF in cmos!\n");
        dng_param->wb_gain2.r_gain = 0xFFF;
    }

    if (dng_param->ccm_tab1.color_temp < 2000 || dng_param->ccm_tab1.color_temp > 10000) { /* 2000, 10000 */
        isp_err_trace("ccm_tab1.color_temp should between [2000, 10000] in cmos!\n");
        dng_param->ccm_tab1.color_temp = clip3(dng_param->ccm_tab1.color_temp, 2000, 10000); /* 2000, 10000 */
    }

    if (dng_param->ccm_tab2.color_temp < 2000 || dng_param->ccm_tab2.color_temp > 10000) { /* 2000, 10000 */
        isp_err_trace("ccm_tab2.color_temp should between [2000, 10000] in cmos!\n");
        dng_param->ccm_tab2.color_temp = clip3(dng_param->ccm_tab2.color_temp, 2000, 10000); /* 2000, 10000 */
    }
}

td_void isp_dng_ext_regs_initialize(ot_vi_pipe vi_pipe, ot_isp_dng_color_param *dng_color_param)
{
    td_u32 i;
    isp_check_dng_color_param(dng_color_param);

    ot_ext_system_dng_low_wb_gain_r_write(vi_pipe, dng_color_param->wb_gain1.r_gain);
    ot_ext_system_dng_low_wb_gain_g_write(vi_pipe, dng_color_param->wb_gain1.g_gain);
    ot_ext_system_dng_low_wb_gain_b_write(vi_pipe, dng_color_param->wb_gain1.b_gain);
    ot_ext_system_dng_high_wb_gain_r_write(vi_pipe, dng_color_param->wb_gain2.r_gain);
    ot_ext_system_dng_high_wb_gain_g_write(vi_pipe, dng_color_param->wb_gain2.g_gain);
    ot_ext_system_dng_high_wb_gain_b_write(vi_pipe, dng_color_param->wb_gain2.b_gain);

    ot_ext_system_dng_high_temp_write(vi_pipe, dng_color_param->ccm_tab2.color_temp);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ot_ext_system_dng_high_ccm_write(vi_pipe, i, direct_to_complement_u16(dng_color_param->ccm_tab2.ccm[i]));
    }

    ot_ext_system_dng_low_temp_write(vi_pipe, dng_color_param->ccm_tab1.color_temp);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ot_ext_system_dng_low_ccm_write(vi_pipe, i, direct_to_complement_u16(dng_color_param->ccm_tab1.ccm[i]));
    }
}

