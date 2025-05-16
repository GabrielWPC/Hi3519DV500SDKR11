/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "ot_mpi_sys_mem.h"
#include "ot_mpi_isp.h"
#include "mkp_isp.h"
#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "ot_common_ae.h"
#include "ot_common_awb.h"
#include "ot_common_vi.h"
#include "isp_intf.h"
#include "isp_ext_config.h"
#include "isp_debug.h"
#include "isp_main.h"
#include "isp_math_utils.h"
#include "isp_vreg.h"
#include "ot_math.h"
#include "ot_common_sns.h"
#include "isp_config.h"
#include "isp_param_check.h"
#include "isp_ext_reg_access.h"

static td_s32 isp_pub_wnd_rect_attr_check(ot_vi_pipe vi_pipe, const ot_isp_pub_attr *pub_attr,
    isp_usr_ctx *isp_ctx_info)
{
    td_s32  ret;
    td_u32  pipe_width, pipe_height;

    if ((pub_attr->wnd_rect.width < OT_ISP_WIDTH_MIN) || (pub_attr->wnd_rect.width > ot_isp_res_width_max(vi_pipe)) ||
        (pub_attr->wnd_rect.width % OT_ISP_ALIGN_WIDTH) != 0) {
        isp_err_trace("Invalid image width:%u!\n", pub_attr->wnd_rect.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.height < OT_ISP_HEIGHT_MIN) ||
        (pub_attr->wnd_rect.height > ot_isp_res_height_max(vi_pipe)) ||
        (pub_attr->wnd_rect.height % OT_ISP_ALIGN_HEIGHT) != 0) {
        isp_err_trace("Invalid image height:%u!\n", pub_attr->wnd_rect.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.x < 0) || (pub_attr->wnd_rect.x > ot_isp_res_width_max(vi_pipe) - OT_ISP_WIDTH_MIN) ||
        ((pub_attr->wnd_rect.x % 2) != 0)) { /* 2 align */
        isp_err_trace("Invalid image X:%d!\n", pub_attr->wnd_rect.x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->wnd_rect.y < 0) || (pub_attr->wnd_rect.y > ot_isp_res_height_max(vi_pipe) - OT_ISP_HEIGHT_MIN) ||
        ((pub_attr->wnd_rect.y % 2) != 0)) { /* 2 align */
        isp_err_trace("Invalid image Y:%d!\n", pub_attr->wnd_rect.y);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (vi_pipe < OT_ISP_MAX_PHY_PIPE_NUM) {
        /* Get pipe size */
        ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_PIPE_SIZE, &isp_ctx_info->pipe_size);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] get Pipe size failed\n", vi_pipe);
            return ret;
        }

        pipe_width  = isp_ctx_info->pipe_size.width;
        pipe_height = isp_ctx_info->pipe_size.height;

        if (pub_attr->wnd_rect.width > pipe_width) {
            isp_err_trace("Invalid image width:%u\n", pub_attr->wnd_rect.width);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if (pub_attr->wnd_rect.height > pipe_height) {
            isp_err_trace("Invalid image height:%u\n", pub_attr->wnd_rect.height);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_mipi_wnd_rect_attr_check(const ot_isp_pub_attr *pub_attr)
{
    isp_check_pointer_return(pub_attr);
    if ((pub_attr->mipi_crop_attr.mipi_crop_offset.width < OT_ISP_WIDTH_MIN) ||
        (pub_attr->mipi_crop_attr.mipi_crop_offset.width < pub_attr->wnd_rect.width) ||
        (pub_attr->mipi_crop_attr.mipi_crop_offset.width % OT_ISP_ALIGN_WIDTH) != 0) {
        isp_err_trace("Invalid mipi_image width:%u!\n", pub_attr->mipi_crop_attr.mipi_crop_offset.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->mipi_crop_attr.mipi_crop_offset.height < OT_ISP_HEIGHT_MIN) ||
        (pub_attr->mipi_crop_attr.mipi_crop_offset.height < pub_attr->wnd_rect.height) ||
        (pub_attr->mipi_crop_attr.mipi_crop_offset.height % OT_ISP_ALIGN_HEIGHT) != 0) {
        isp_err_trace("Invalid mipi_image height:%u!\n", pub_attr->mipi_crop_attr.mipi_crop_offset.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pub_attr->mipi_crop_attr.mipi_crop_offset.x < 0) {
        isp_err_trace("Invalid mipi_image X:%d!\n", pub_attr->mipi_crop_attr.mipi_crop_offset.x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_pub_attr_check(ot_vi_pipe vi_pipe, const ot_isp_pub_attr *pub_attr, isp_usr_ctx *isp_ctx_info)
{
    td_s32  ret;
    ret = isp_pub_wnd_rect_attr_check(vi_pipe, pub_attr, isp_ctx_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if ((pub_attr->sns_size.width < OT_ISP_WIDTH_MIN) || (pub_attr->sns_size.width > OT_ISP_SENSOR_WIDTH_MAX)) {
        isp_err_trace("Invalid sensor image width:%u!\n", pub_attr->sns_size.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->sns_size.height < OT_ISP_HEIGHT_MIN) || (pub_attr->sns_size.height > OT_ISP_SENSOR_HEIGHT_MAX)) {
        isp_err_trace("Invalid sensor image height:%u!\n", pub_attr->sns_size.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pub_attr->frame_rate <= 0.0) || (pub_attr->frame_rate > OT_ISP_FRAME_RATE_MAX)) {
        isp_err_trace("Invalid frame_rate:%f!\n", pub_attr->frame_rate);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pub_attr->bayer_format >= OT_ISP_BAYER_BUTT) {
        isp_err_trace("Invalid bayer pattern:%d!\n", pub_attr->bayer_format);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (pub_attr->wdr_mode >= OT_WDR_MODE_BUTT) {
        isp_err_trace("Invalid WDR mode %d!\n", pub_attr->wdr_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    isp_check_bool_return(pub_attr->sns_flip_en);
    isp_check_bool_return(pub_attr->sns_mirror_en);
    isp_check_bool_return(pub_attr->mipi_crop_attr.mipi_crop_en);

    if (pub_attr->mipi_crop_attr.mipi_crop_en == TD_TRUE) {
        ret = isp_mipi_wnd_rect_attr_check(pub_attr);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_pub_attr_write(ot_vi_pipe vi_pipe, const ot_isp_pub_attr *pub_attr, isp_usr_ctx *isp_ctx_info)
{
    td_void *value = TD_NULL;
    /* set WDR mode */
    ot_ext_top_wdr_switch_write(vi_pipe, TD_FALSE);

    isp_ctx_info->para_rec.wdr_cfg = TD_TRUE;
    ot_ext_top_wdr_cfg_write(vi_pipe, isp_ctx_info->para_rec.wdr_cfg);

    if ((td_u8)pub_attr->wdr_mode == ot_ext_system_sensor_wdr_mode_read(vi_pipe)) {
        ot_ext_top_wdr_switch_write(vi_pipe, TD_TRUE);
    } else {
        ot_ext_system_sensor_wdr_mode_write(vi_pipe, (td_u8)pub_attr->wdr_mode);
    }

    /* set othes cfgs */
    ot_ext_top_res_switch_write(vi_pipe, TD_FALSE);

    ot_ext_system_corp_pos_x_write(vi_pipe, pub_attr->wnd_rect.x);
    ot_ext_system_corp_pos_y_write(vi_pipe, pub_attr->wnd_rect.y);

    ot_ext_sync_total_width_write(vi_pipe, pub_attr->wnd_rect.width);
    ot_ext_sync_total_height_write(vi_pipe, pub_attr->wnd_rect.height);

    ot_ext_top_sensor_width_write(vi_pipe, pub_attr->sns_size.width);
    ot_ext_top_sensor_height_write(vi_pipe, pub_attr->sns_size.height);

    ot_ext_system_rggb_cfg_write(vi_pipe, (td_u8)pub_attr->bayer_format);

    value = (td_void *)(&pub_attr->frame_rate);
    ot_ext_system_fps_base_write(vi_pipe, *(td_u32 *)value);
    ot_ext_system_sensor_mode_write(vi_pipe, pub_attr->sns_mode);

    isp_ctx_info->para_rec.pub_cfg = TD_TRUE;
    ot_ext_top_pub_attr_cfg_write(vi_pipe, isp_ctx_info->para_rec.pub_cfg);
    ot_sensor_flip_enable_write(vi_pipe, pub_attr->sns_flip_en);
    ot_sensor_mirror_enable_write(vi_pipe, pub_attr->sns_mirror_en);
    ot_mipi_crop_enable_write(vi_pipe, pub_attr->mipi_crop_attr.mipi_crop_en);
    ot_mipi_crop_pos_x_write(vi_pipe, pub_attr->mipi_crop_attr.mipi_crop_offset.x);
    ot_mipi_crop_pos_y_write(vi_pipe, pub_attr->mipi_crop_attr.mipi_crop_offset.y);
    ot_mipi_crop_width_write(vi_pipe, pub_attr->mipi_crop_attr.mipi_crop_offset.width);
    ot_mipi_crop_height_write(vi_pipe, pub_attr->mipi_crop_attr.mipi_crop_offset.height);
}

td_s32 ot_mpi_isp_set_pub_attr(ot_vi_pipe vi_pipe, const ot_isp_pub_attr *pub_attr)
{
    td_s32  ret;
    isp_usr_ctx *isp_ctx_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx_info);
    isp_check_pointer_return(isp_ctx_info);
    isp_check_pointer_return(pub_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = isp_pub_attr_check(vi_pipe, pub_attr, isp_ctx_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_PUB_ATTR_INFO, pub_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set ISP PUB attr failed\n", vi_pipe);
        return ret;
    }

    isp_pub_attr_write(vi_pipe, pub_attr, isp_ctx_info);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_pub_attr(ot_vi_pipe vi_pipe, ot_isp_pub_attr *pub_attr)
{
    td_u8 bayer;
    td_u8 wd_rmode;
    td_u32 fps_value;
    td_void *value = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pub_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    bayer = ot_ext_system_rggb_cfg_read(vi_pipe);
    pub_attr->bayer_format = (bayer >= OT_ISP_BAYER_BUTT) ? OT_ISP_BAYER_BUTT : bayer;

    wd_rmode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    pub_attr->wdr_mode = (wd_rmode >= OT_WDR_MODE_BUTT) ? OT_WDR_MODE_BUTT : wd_rmode;

    fps_value = ot_ext_system_fps_base_read(vi_pipe);
    value   = (td_void *)&fps_value;
    pub_attr->frame_rate = *(td_float *)value;

    pub_attr->sns_mode = ot_ext_system_sensor_mode_read(vi_pipe);

    pub_attr->wnd_rect.x       = ot_ext_system_corp_pos_x_read(vi_pipe);
    pub_attr->wnd_rect.y       = ot_ext_system_corp_pos_y_read(vi_pipe);
    pub_attr->wnd_rect.width   = ot_ext_sync_total_width_read(vi_pipe);
    pub_attr->wnd_rect.height  = ot_ext_sync_total_height_read(vi_pipe);
    pub_attr->sns_size.width   = ot_ext_top_sensor_width_read(vi_pipe);
    pub_attr->sns_size.height  = ot_ext_top_sensor_height_read(vi_pipe);
    pub_attr->sns_flip_en   = ot_sensor_flip_enable_read(vi_pipe);
    pub_attr->sns_mirror_en = ot_sensor_mirror_enable_read(vi_pipe);
    pub_attr->mipi_crop_attr.mipi_crop_en = ot_mipi_crop_enable_read(vi_pipe);
    pub_attr->mipi_crop_attr.mipi_crop_offset.x = ot_mipi_crop_pos_x_read(vi_pipe);
    pub_attr->mipi_crop_attr.mipi_crop_offset.y = ot_mipi_crop_pos_y_read(vi_pipe);
    pub_attr->mipi_crop_attr.mipi_crop_offset.width  = ot_mipi_crop_width_read(vi_pipe);
    pub_attr->mipi_crop_attr.mipi_crop_offset.height = ot_mipi_crop_height_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_pipe_differ_attr(ot_vi_pipe vi_pipe, const ot_isp_pipe_diff_attr *pipe_differ)
{
    return isp_set_pipe_differ_attr(vi_pipe, pipe_differ);
}

td_s32 ot_mpi_isp_get_pipe_differ_attr(ot_vi_pipe vi_pipe, ot_isp_pipe_diff_attr *pipe_differ)
{
    return isp_get_pipe_differ_attr(vi_pipe, pipe_differ);
}

static td_s32 isp_set_chn_select(ot_vi_pipe vi_pipe, const td_u32 chn_sel_cur)
{
    td_s32 ret;
    td_u32 chn_sel_pre;
    chn_sel_pre = ot_ext_system_top_channel_select_pre_read(vi_pipe);
    if (chn_sel_pre != chn_sel_cur) {
        ret = ioctl(isp_get_fd(vi_pipe), ISP_CHN_SELECT_CFG, &chn_sel_cur);
        if (ret != TD_SUCCESS) {
            isp_err_trace("set isp[%d] chn select failed with ec %#x!\n", vi_pipe, ret);
            return ret;
        }

        ot_ext_system_top_channel_select_pre_write(vi_pipe, chn_sel_cur);
    }
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_module_ctrl(ot_vi_pipe vi_pipe, const ot_isp_module_ctrl *mod_ctrl)
{
    td_s32 ret;
    td_u64 info;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(mod_ctrl);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    info = mod_ctrl->key;

    ot_ext_system_isp_bit_bypass_write(vi_pipe, (td_u32)info);

    ot_ext_system_isp_dgain_enable_write(vi_pipe, !((info >> 0) & 0x1));    /* bit0: isp_dgain */
    ot_ext_system_afc_enable_write(vi_pipe, !((info >> 1) & 0x1));          /* bit1: anti_false_color */
    ot_ext_system_ge_enable_write(vi_pipe, !((info >> 2) & 0x1));           /* bit2: crosstalk_removal */
    ot_ext_system_dpc_dynamic_cor_enable_write(vi_pipe, !((info >> 3) & 0x1));  /* bit3: dpc */
    ot_ext_system_bayernr_enable_write(vi_pipe, !((info >> 4) & 0x1));          /* bit4: bayer_nr */
    ot_ext_system_dehaze_enable_write(vi_pipe, !((info >> 5) & 0x1));           /* bit5: dehaze */
    ot_ext_system_awb_gain_enable_write(vi_pipe, !((info >> 6) & 0x1));         /* bit6: wb_gain */
    ot_ext_system_isp_mesh_shading_enable_write(vi_pipe, !((info >> 7) & 0x1)); /* bit7: mesh_ahding */
    ot_ext_system_drc_enable_write(vi_pipe, !((info >> 8) & 0x1));              /* bit8: drc */
    ot_ext_system_demosaic_enable_write(vi_pipe, !((info >> 9) & 0x1));         /* bit9: demosaic */
    ot_ext_system_cc_enable_write(vi_pipe, !((info >> 10) & 0x1));              /* bit10: ccm */
    ot_ext_system_gamma_en_write(vi_pipe, !((info >> 11) & 0x1));               /* bit11: gamma */
    ot_ext_system_wdr_en_write(vi_pipe, !((info >> 12) & 0x1));                 /* bit12: wdr */
    ot_ext_system_ca_en_write(vi_pipe, !((info >> 13) & 0x1));                  /* bit13: ca */
    ot_ext_system_csc_enable_write(vi_pipe, !((info >> 14) & 0x1));             /* bit14: csc */
    ot_ext_system_rc_en_write(vi_pipe, !((info >> 15) & 0x1));                  /* bit15: radial crop */
    ot_ext_system_manual_sharpen_en_write(vi_pipe, !((info >> 16) & 0x1));      /* bit16: sharpen */
    ot_ext_system_bshp_enable_write(vi_pipe, !((info >> 17) & 0x1));   /* bit17: bayer sharpen */
    ot_ext_system_cac_enable_write(vi_pipe, !((info >> 18) & 0x1));             /* bit18: cac */
    ot_ext_system_top_channel_select_write(vi_pipe, (td_u8)(info >> 19) & 0x3);  /* bit19~20: chn sel */
    ot_ext_system_ldci_enable_write(vi_pipe, !((info >> 21) & 0x1));            /* bit21: ldci */
    ot_ext_system_pregamma_en_write(vi_pipe, !((info >> 22) & 0x1));            /* bit22: pregamma */
    ot_ext_system_ae_fe_en_write(vi_pipe, !((info >> 23) & 0x1));               /* bit23: fe ae */
    ot_ext_system_ae_be_en_write(vi_pipe, !((info >> 24) & 0x1));               /* bit24: be ae */
    ot_ext_system_mg_en_write(vi_pipe, !((info >> 25) & 0x1));                  /* bit25: mg */
    ot_ext_system_af_enable_write(vi_pipe, (td_u8)~((info >> 26) & 0x3));              /* bit26: fe af; bit27: be af */
    ot_ext_system_awb_sta_enable_write(vi_pipe, !((info >> 28) & 0x1));         /* bit28: awb stat */
    ot_ext_system_clut_en_write(vi_pipe, !((info >> 29) & 0x1));                /* bit29: clut */
    ot_ext_system_rgbir_enable_write(vi_pipe, !((info >> 30) & 0x1));           /* bit30: rgbir */
    ot_ext_system_isp_lblc_enable_write(vi_pipe, !((info >> 31) & 0x1));          /* bit31: lblc */

    ret = isp_set_chn_select(vi_pipe, (td_u32)mod_ctrl->bit2_chn_select);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_af_set_flag_write(vi_pipe, OT_EXT_AF_SET_FLAG_ENABLE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_module_ctrl(ot_vi_pipe vi_pipe, ot_isp_module_ctrl *mod_ctrl)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(mod_ctrl);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    mod_ctrl->bit_bypass_isp_d_gain        = !(ot_ext_system_isp_dgain_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_anti_false_color  = !(ot_ext_system_afc_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_crosstalk_removal = !(ot_ext_system_ge_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_dpc               = !(ot_ext_system_dpc_dynamic_cor_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_nr                = !(ot_ext_system_bayernr_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_dehaze            = !(ot_ext_system_dehaze_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_wb_gain           = !(ot_ext_system_awb_gain_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_mesh_shading      = !(ot_ext_system_isp_mesh_shading_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_drc               = !(ot_ext_system_drc_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_demosaic          = !(ot_ext_system_demosaic_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_color_matrix      = !(ot_ext_system_cc_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_gamma             = !(ot_ext_system_gamma_en_read(vi_pipe));
    mod_ctrl->bit_bypass_fswdr             = !(ot_ext_system_wdr_en_read(vi_pipe));
    mod_ctrl->bit_bypass_ca                = !(ot_ext_system_ca_en_read(vi_pipe));
    mod_ctrl->bit_bypass_csc               = !(ot_ext_system_csc_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_radial_crop       = !(ot_ext_system_rc_en_read(vi_pipe));
    mod_ctrl->bit_bypass_sharpen           = !(ot_ext_system_manual_sharpen_en_read(vi_pipe));
    mod_ctrl->bit_bypass_bayer_sharpen     = !(ot_ext_system_bshp_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_cac               = !(ot_ext_system_cac_enable_read(vi_pipe));
    mod_ctrl->bit2_chn_select              = ot_ext_system_top_channel_select_read(vi_pipe);
    mod_ctrl->bit_bypass_ldci              = !(ot_ext_system_ldci_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_pregamma          = !(ot_ext_system_pregamma_en_read(vi_pipe));
    mod_ctrl->bit_bypass_ae_stat_fe        = !(ot_ext_system_ae_fe_en_read(vi_pipe));
    mod_ctrl->bit_bypass_ae_stat_be        = !(ot_ext_system_ae_be_en_read(vi_pipe));
    mod_ctrl->bit_bypass_mg_stat           = !(ot_ext_system_mg_en_read(vi_pipe));
    mod_ctrl->bit_bypass_af_stat_fe        = !(ot_ext_system_af_enable_read(vi_pipe) & 0x1);
    mod_ctrl->bit_bypass_af_stat_be        = !((ot_ext_system_af_enable_read(vi_pipe) >> 1) & 0x1);
    mod_ctrl->bit_bypass_awb_stat          = !(ot_ext_system_awb_sta_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_clut              = !(ot_ext_system_clut_en_read(vi_pipe));
    mod_ctrl->bit_bypass_rgbir             = !(ot_ext_system_rgbir_enable_read(vi_pipe));
    mod_ctrl->bit_bypass_lblc              = !(ot_ext_system_isp_lblc_enable_read(vi_pipe));
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_fmw_state(ot_vi_pipe vi_pipe, const ot_isp_fmw_state state)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (state >= OT_ISP_FMW_STATE_BUTT) {
        isp_err_trace("Invalid firmware state %d!\n", state);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_freeze_firmware_write(vi_pipe, (td_u8)state);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_fmw_state(ot_vi_pipe vi_pipe, ot_isp_fmw_state *state)
{
    td_u8 fmw_state;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(state);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    fmw_state = ot_ext_system_freeze_firmware_read(vi_pipe);
    *state = (fmw_state >= OT_ISP_FMW_STATE_BUTT) ? OT_ISP_FMW_STATE_BUTT : fmw_state;

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_ldci_attr(ot_vi_pipe vi_pipe, const ot_isp_ldci_attr *ldci_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ldci_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_ldci_init_return(vi_pipe);

    ret = isp_ldci_attr_check("mpi", ldci_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_ldci_attr_write(vi_pipe, ldci_attr);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_ldci_attr(ot_vi_pipe vi_pipe, ot_isp_ldci_attr *ldci_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ldci_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_ldci_init_return(vi_pipe);

    isp_ldci_attr_read(vi_pipe, ldci_attr);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_drc_attr(ot_vi_pipe vi_pipe, const ot_isp_drc_attr *drc_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(drc_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_drc_init_return(vi_pipe);

    ret = isp_drc_attr_check("mpi", drc_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_drc_attr_write(vi_pipe, drc_attr);
    ot_ext_system_drc_param_updated_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_drc_attr(ot_vi_pipe vi_pipe, ot_isp_drc_attr *drc_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(drc_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_drc_init_return(vi_pipe);

    isp_drc_attr_read(vi_pipe, drc_attr);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_dehaze_attr(ot_vi_pipe vi_pipe, const ot_isp_dehaze_attr *dehaze_attr)
{
    td_s32 i, ret;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dehaze_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dehaze_init_return(vi_pipe);

    ret = isp_dehaze_attr_check("mpi", dehaze_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_user_dehaze_lut_enable_write(vi_pipe, dehaze_attr->user_lut_en);

    for (i = 0; i < OT_ISP_DEHAZE_LUT_SIZE; i++) {
        ot_ext_system_dehaze_lut_write(vi_pipe, i, dehaze_attr->dehaze_lut[i]);
    }

    if (dehaze_attr->user_lut_en) {
        /* 1:update the defog lut,FW will change it to 0 when the lut updating is finished. */
        ot_ext_system_user_dehaze_lut_update_write(vi_pipe, TD_TRUE);
    }

    ot_ext_system_dehaze_enable_write(vi_pipe, dehaze_attr->enable);
    ot_ext_system_dehaze_manu_mode_write(vi_pipe, dehaze_attr->op_type);
    ot_ext_system_manual_dehaze_strength_write(vi_pipe, dehaze_attr->manual_attr.strength);
    ot_ext_system_manual_dehaze_autostrength_write(vi_pipe, dehaze_attr->auto_attr.strength);
    ot_ext_system_dehaze_tfic_write(vi_pipe, dehaze_attr->tmprflt_incr_coef);
    ot_ext_system_dehaze_tfdc_write(vi_pipe, dehaze_attr->tmprflt_decr_coef);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_dehaze_attr(ot_vi_pipe vi_pipe, ot_isp_dehaze_attr *dehaze_attr)
{
    td_s32 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dehaze_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dehaze_init_return(vi_pipe);

    dehaze_attr->enable               = ot_ext_system_dehaze_enable_read(vi_pipe);
    dehaze_attr->op_type              = (ot_op_mode)ot_ext_system_dehaze_manu_mode_read(vi_pipe);
    dehaze_attr->manual_attr.strength = ot_ext_system_manual_dehaze_strength_read(vi_pipe);
    dehaze_attr->auto_attr.strength   = ot_ext_system_manual_dehaze_autostrength_read(vi_pipe);
    dehaze_attr->user_lut_en      = ot_ext_system_user_dehaze_lut_enable_read(vi_pipe);

    for (i = 0; i < OT_ISP_DEHAZE_LUT_SIZE; i++) {
        dehaze_attr->dehaze_lut[i] = ot_ext_system_dehaze_lut_read(vi_pipe, i);
    }

    dehaze_attr->tmprflt_incr_coef = ot_ext_system_dehaze_tfic_read(vi_pipe);
    dehaze_attr->tmprflt_decr_coef = ot_ext_system_dehaze_tfdc_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_fswdr_attr(ot_vi_pipe vi_pipe, const ot_isp_wdr_fs_attr *fswdr_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(fswdr_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_fswdr_init_return(vi_pipe);

    ret = isp_fswdr_attr_check("mpi", fswdr_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_fswdr_attr_write(vi_pipe, fswdr_attr);

    ot_ext_system_wdr_coef_update_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_fswdr_attr(ot_vi_pipe vi_pipe, ot_isp_wdr_fs_attr *fswdr_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(fswdr_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_fswdr_init_return(vi_pipe);

    isp_fswdr_attr_read(vi_pipe, fswdr_attr);

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_AF_SUPPORT
static td_void isp_get_focus_stats(ot_vi_pipe vi_pipe, ot_isp_af_stats *af_stat, isp_stat *isp_act_stat,
    vi_pipe_wdr_attr *wdr_attr)
{
    td_u8 col, row, wdr_chn;
    td_s32 i, j;
    ot_isp_stats_ctrl stat_key;
    td_u32 ctrl;

    ctrl  = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl;

    col = MIN2(ot_ext_af_window_hnum_read(vi_pipe), OT_ISP_AF_ZONE_COLUMN);
    row = MIN2(ot_ext_af_window_vnum_read(vi_pipe), OT_ISP_AF_ZONE_ROW);

    wdr_chn = MIN2(wdr_attr->pipe_num, OT_ISP_WDR_MAX_FRAME_NUM);

    /* AF FE stat */
    if (stat_key.bit1_fe_af_stat && wdr_attr->is_mast_pipe) {
        isp_get_fe_focus_stats(vi_pipe, &af_stat->fe_af_stat, isp_act_stat, wdr_chn);
    }

    /* BE */
    if (stat_key.bit1_be_af_stat) {
        for (i = 0; i < row; i++) {
            for (j = 0; j < col; j++) {
                af_stat->be_af_stat.zone_metrics[i][j].v1 = isp_act_stat->be_af_stat.zone_metrics[i][j].v1;
                af_stat->be_af_stat.zone_metrics[i][j].h1 = isp_act_stat->be_af_stat.zone_metrics[i][j].h1;
                af_stat->be_af_stat.zone_metrics[i][j].v2 = isp_act_stat->be_af_stat.zone_metrics[i][j].v2;
                af_stat->be_af_stat.zone_metrics[i][j].h2 = isp_act_stat->be_af_stat.zone_metrics[i][j].h2;
                af_stat->be_af_stat.zone_metrics[i][j].y  = isp_act_stat->be_af_stat.zone_metrics[i][j].y;
                af_stat->be_af_stat.zone_metrics[i][j].hl_cnt = isp_act_stat->be_af_stat.zone_metrics[i][j].hl_cnt;
            }
        }
    }
}
#endif
td_s32 ot_mpi_isp_get_focus_stats(ot_vi_pipe vi_pipe, ot_isp_af_stats *af_stat)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    vi_pipe_wdr_attr wdr_attr;

    isp_stat_info act_stat_info;
    isp_stat *isp_act_stat = TD_NULL;
    td_s32 ret;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(af_stat);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &act_stat_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get active stat buffer err\n");
        return OT_ERR_ISP_NO_INT;
    }

    act_stat_info.virt_addr = ot_mpi_sys_mmap_cached(act_stat_info.phy_addr, sizeof(isp_stat));
    if (act_stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    isp_act_stat = (isp_stat *)act_stat_info.virt_addr;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &wdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get WDR attr failed\n", vi_pipe);
        ot_mpi_sys_munmap(act_stat_info.virt_addr, sizeof(isp_stat));
        return ret;
    }

    isp_get_focus_stats(vi_pipe, af_stat, isp_act_stat, &wdr_attr);
    af_stat->pts = isp_act_stat->frame_pts;

    ot_mpi_sys_munmap((td_void *)act_stat_info.virt_addr, sizeof(isp_stat));

    isp_get_af_grid_info(vi_pipe, &af_stat->fe_af_grid_info, &af_stat->be_af_grid_info);

    return TD_SUCCESS;
#else
    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

static td_void isp_get_fe_ae_glo_statistics(td_u32 pipe_num, ot_isp_ae_stats *ae_stat, isp_stat *isp_act_stat)
{
    td_u32 k, i;

    for (k = 0; k < pipe_num; k++) {
        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            ae_stat->fe_hist1024_value[k][i] = isp_act_stat->fe_ae_stat1.histogram_mem_array[k][i];
        }

        ae_stat->fe_global_avg[k][OT_ISP_CHN_R]  = isp_act_stat->fe_ae_stat2.global_avg_r[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_GR] = isp_act_stat->fe_ae_stat2.global_avg_gr[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_GB] = isp_act_stat->fe_ae_stat2.global_avg_gb[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_B]  = isp_act_stat->fe_ae_stat2.global_avg_b[k];
    }

    for (; k < OT_ISP_WDR_MAX_FRAME_NUM; k++) {
        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            ae_stat->fe_hist1024_value[k][i] = 0;
        }
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            ae_stat->fe_global_avg[k][i] = 0;
        }
    }
}

static td_void isp_get_fe_ae_loc_statistics(td_u32 pipe_num, ot_isp_ae_stats *ae_stat, isp_stat *act_stat)
{
    td_u32 k, i, j;
    size_t size;

    size = OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_AE_ZONE_ROW *
        OT_ISP_AE_ZONE_COLUMN * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);
    (td_void)memset_s(ae_stat->fe_zone_avg, size, 0, size);

    for (k = 0; k < pipe_num; k++) {
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                ae_stat->fe_zone_avg[k][i][j][OT_ISP_CHN_R]  = act_stat->fe_ae_stat3.zone_avg[k][i][j][OT_ISP_CHN_R];
                ae_stat->fe_zone_avg[k][i][j][OT_ISP_CHN_GR] = act_stat->fe_ae_stat3.zone_avg[k][i][j][OT_ISP_CHN_GR];
                ae_stat->fe_zone_avg[k][i][j][OT_ISP_CHN_GB] = act_stat->fe_ae_stat3.zone_avg[k][i][j][OT_ISP_CHN_GB];
                ae_stat->fe_zone_avg[k][i][j][OT_ISP_CHN_B]  = act_stat->fe_ae_stat3.zone_avg[k][i][j][OT_ISP_CHN_B];
            }
        }
    }
}

static td_void isp_get_be_ae_glo_statistics(ot_isp_ae_stats *ae_stat, isp_stat *isp_act_stat)
{
    td_u16 i;

    for (i = 0; i < OT_ISP_HIST_NUM; i++) {
        ae_stat->be_hist1024_value[i] = isp_act_stat->be_ae_stat1.histogram_mem_array[i];
    }

    ae_stat->be_global_avg[OT_ISP_CHN_R]  = isp_act_stat->be_ae_stat2.global_avg_r;
    ae_stat->be_global_avg[OT_ISP_CHN_GR] = isp_act_stat->be_ae_stat2.global_avg_gr;
    ae_stat->be_global_avg[OT_ISP_CHN_GB] = isp_act_stat->be_ae_stat2.global_avg_gb;
    ae_stat->be_global_avg[OT_ISP_CHN_B]  = isp_act_stat->be_ae_stat2.global_avg_b;
}

static td_void isp_get_be_ae_loc_statistics(ot_isp_ae_stats *ae_stat, isp_stat *isp_act_stat)
{
    td_u16 i, j;

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ae_stat->be_zone_avg[i][j][OT_ISP_CHN_R]  = isp_act_stat->be_ae_stat3.zone_avg[i][j][OT_ISP_CHN_R];
            ae_stat->be_zone_avg[i][j][OT_ISP_CHN_GR] = isp_act_stat->be_ae_stat3.zone_avg[i][j][OT_ISP_CHN_GR];
            ae_stat->be_zone_avg[i][j][OT_ISP_CHN_GB] = isp_act_stat->be_ae_stat3.zone_avg[i][j][OT_ISP_CHN_GB];
            ae_stat->be_zone_avg[i][j][OT_ISP_CHN_B]  = isp_act_stat->be_ae_stat3.zone_avg[i][j][OT_ISP_CHN_B];
        }
    }
}

td_s32 ot_mpi_isp_get_ae_stats(ot_vi_pipe vi_pipe, ot_isp_ae_stats *ae_stat)
{
    td_s32 pipe_num, ret;

    vi_pipe_wdr_attr wdr_attr;
    ot_isp_stats_ctrl stat_key;
    isp_stat_info stat_info;
    isp_stat *isp_act_stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_pointer_return(ae_stat);
    isp_check_vreg_permission_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &stat_info) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get active stat buffer failed\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    stat_info.virt_addr = ot_mpi_sys_mmap_cached(stat_info.phy_addr, sizeof(isp_stat));
    if (stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }
    isp_act_stat = (isp_stat *)stat_info.virt_addr;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &wdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get WDR attr failed\n", vi_pipe);
        ot_mpi_sys_munmap(stat_info.virt_addr, sizeof(isp_stat));
        return ret;
    }

    stat_key.key = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);

    /* AE FE stat */
    pipe_num = (td_s32)MIN2(wdr_attr.pipe_num, OT_ISP_WDR_MAX_FRAME_NUM);
    if (stat_key.bit1_fe_ae_global_stat && wdr_attr.is_mast_pipe) {
        isp_get_fe_ae_glo_statistics((td_u32)pipe_num, ae_stat, isp_act_stat);
    }

    if (stat_key.bit1_fe_ae_local_stat && wdr_attr.is_mast_pipe) {
        isp_get_fe_ae_loc_statistics((td_u32)pipe_num, ae_stat, isp_act_stat);
    }

    /* AE BE stat */
    if ((td_u32)stat_key.bit1_be_ae_global_stat) {
        isp_get_be_ae_glo_statistics(ae_stat, isp_act_stat);
    }

    if ((td_u32)stat_key.bit1_extend_stats == 1) {
        isp_be_stats_estimate(vi_pipe, ae_stat->be_estimate_hist1024_value, &isp_act_stat->fe_ae_stat1,
            &isp_act_stat->be_stats_calc_info);
    }

    if (stat_key.bit1_be_ae_local_stat) {
        isp_get_be_ae_loc_statistics(ae_stat, isp_act_stat);
    }
    ae_stat->pts = isp_act_stat->frame_pts;

    ot_mpi_sys_munmap((td_void *)isp_act_stat, sizeof(isp_stat));
    isp_get_ae_grid_info(vi_pipe, &ae_stat->fe_grid_info, &ae_stat->be_grid_info);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_ae_stitch_stats(ot_vi_pipe vi_pipe, ot_isp_ae_stitch_stats *stitch_stat)
{
    return isp_get_ae_stitch_stats(vi_pipe, stitch_stat);
}

td_s32 ot_mpi_isp_get_mg_stats(ot_vi_pipe vi_pipe, ot_isp_mg_stats *mg_stat)
{
    return isp_get_mg_stats(vi_pipe, mg_stat);
}

td_s32 ot_mpi_isp_get_wb_stitch_stats(ot_vi_pipe vi_pipe, ot_isp_wb_stitch_stats *stitch_wb_stat)
{
    return isp_get_wb_stitch_stats(vi_pipe, stitch_wb_stat);
}

td_s32 ot_mpi_isp_get_wb_stats(ot_vi_pipe vi_pipe, ot_isp_wb_stats *wb_stat)
{
    td_s32 i;
    ot_isp_stats_ctrl stat_key;
    isp_stat_info act_stat_info;
    isp_stat *isp_act_stat = TD_NULL;
    td_s32 ret;
    td_u32 ctrl;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(wb_stat);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ctrl  = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &act_stat_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get active stat buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    act_stat_info.virt_addr = ot_mpi_sys_mmap_cached(act_stat_info.phy_addr, sizeof(isp_stat));

    if (act_stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    isp_act_stat = (isp_stat *)act_stat_info.virt_addr;

    if (stat_key.bit1_awb_stat1) {
        wb_stat->global_r  = isp_act_stat->awb_stat1.metering_awb_avg_r;
        wb_stat->global_g  = isp_act_stat->awb_stat1.metering_awb_avg_g;
        wb_stat->global_b  = isp_act_stat->awb_stat1.metering_awb_avg_b;
        wb_stat->count_all = isp_act_stat->awb_stat1.metering_awb_count_all;
    }

    if (stat_key.bit1_awb_stat2) {
        for (i = 0; i < OT_ISP_AWB_ZONE_NUM; i++) {
            wb_stat->zone_avg_r[i] = isp_act_stat->awb_stat2.metering_mem_array_avg_r[i];
            wb_stat->zone_avg_g[i] = isp_act_stat->awb_stat2.metering_mem_array_avg_g[i];
            wb_stat->zone_avg_b[i] = isp_act_stat->awb_stat2.metering_mem_array_avg_b[i];
            wb_stat->zone_count_all[i] = isp_act_stat->awb_stat2.metering_mem_array_count_all[i];
        }
    }
    wb_stat->pts = isp_act_stat->frame_pts;

    ot_mpi_sys_munmap(act_stat_info.virt_addr, sizeof(isp_stat));

    isp_get_wb_grid_info(vi_pipe, &wb_stat->grid_info);

    return TD_SUCCESS;
}

static td_s32 isp_fe_roi_param_check(ot_vi_pipe vi_pipe, const ot_isp_fe_roi_cfg *fe_roi_cfg)
{
    td_u16 max_width, max_height;
    isp_check_bool_return(fe_roi_cfg->enable);

    if (is_no_fe_pipe(vi_pipe) && fe_roi_cfg->enable == TD_TRUE) {
        isp_err_trace("ISP[%d]: Not Support fe roi when no fe pipe!\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    max_width = ot_ext_sync_total_width_read(vi_pipe);
    max_height = ot_ext_sync_total_height_read(vi_pipe);
    if ((fe_roi_cfg->roi_rect.width < OT_ISP_WIDTH_MIN) ||
        (fe_roi_cfg->roi_rect.width > max_width) ||
        ((fe_roi_cfg->roi_rect.width % 2) != 0)) { /* 2 align */
        isp_err_trace("Invalid image width:%u!\n", fe_roi_cfg->roi_rect.width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((fe_roi_cfg->roi_rect.height < OT_ISP_HEIGHT_MIN) ||
        (fe_roi_cfg->roi_rect.height > max_height) ||
        ((fe_roi_cfg->roi_rect.height % 2) != 0)) { /* 2 align */
        isp_err_trace("Invalid image height:%u!\n", fe_roi_cfg->roi_rect.height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((fe_roi_cfg->roi_rect.x < 0) || (fe_roi_cfg->roi_rect.x > max_width - OT_ISP_WIDTH_MIN) ||
        (fe_roi_cfg->roi_rect.x % 2 != 0)) { /* 2 align */
        isp_err_trace("Invalid image X:%d!\n", fe_roi_cfg->roi_rect.x);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((fe_roi_cfg->roi_rect.y < 0) || (fe_roi_cfg->roi_rect.y > max_height - OT_ISP_HEIGHT_MIN) ||
        (fe_roi_cfg->roi_rect.y % 2 != 0)) { /* 2 align */
        isp_err_trace("Invalid image Y:%d!\n", fe_roi_cfg->roi_rect.y);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_set_fe_roi_config(ot_vi_pipe vi_pipe, const ot_isp_fe_roi_cfg *fe_roi_cfg)
{
    td_s32 ret;

    ret = isp_fe_roi_param_check(vi_pipe, fe_roi_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_fe_roi_en_write(vi_pipe, fe_roi_cfg->enable);
    ot_ext_system_fe_roi_x_write(vi_pipe, fe_roi_cfg->roi_rect.x);
    ot_ext_system_fe_roi_y_write(vi_pipe, fe_roi_cfg->roi_rect.y);
    ot_ext_system_fe_roi_width_write(vi_pipe, fe_roi_cfg->roi_rect.width);
    ot_ext_system_fe_roi_height_write(vi_pipe, fe_roi_cfg->roi_rect.height);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_fe_roi_cfg(ot_vi_pipe vi_pipe, const ot_isp_fe_roi_cfg            *fe_roi_cfg)
{
    td_s32 ret;
    isp_working_mode isp_work_mode;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(fe_roi_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    if (isp_work_mode.data_input_mode == ISP_MODE_RAW) {
        /* fe roi */
        ret = isp_set_fe_roi_config(vi_pipe, fe_roi_cfg);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else {
        isp_err_trace("not raw mode, set fe roi cfg invalid!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_fe_roi_cfg(ot_vi_pipe vi_pipe, ot_isp_fe_roi_cfg *fe_roi_cfg)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(fe_roi_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    /* fe roi */
    isp_get_fe_roi_config(vi_pipe, fe_roi_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_set_wb_stats_config_check_para(ot_vi_pipe vi_pipe, const ot_isp_wb_stats_cfg *wb_cfg)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    td_s32 ret;
    ot_isp_detail_stats_cfg detail_cfg;
#else
    ot_unused(vi_pipe);
#endif
    if (wb_cfg->awb_switch >= OT_ISP_AWB_SWITCH_BUTT) {
        isp_err_trace("Invalid WB awb_switch %d !\n", wb_cfg->awb_switch);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->cr_max > 0xFFF) {
        isp_err_trace("max value WB cr_max is 0xFFF!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->cb_max > 0xFFF) {
        isp_err_trace("max value of WB cb_max is 0xFFF!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->black_level > wb_cfg->white_level) {
        isp_err_trace("WB black_level should not larger than white_level!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((wb_cfg->cr_min) > (wb_cfg->cr_max)) {
        isp_err_trace("WB cr_min should not larger than cr_max!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->cb_min > wb_cfg->cb_max) {
        isp_err_trace("WB cb_min should not larger than cb_max!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    if (vi_pipe >= 0 && vi_pipe < OT_ISP_MAX_FE_PIPE_NUM) {
        ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_DETAIL_STATS_CFG, &detail_cfg);
        if (ret != TD_SUCCESS) {
            isp_err_trace("pipe:%d get detail_stats_cfg failed!\n", vi_pipe);
            return ret;
        }
        if (detail_cfg.enable && (wb_cfg->awb_switch == OT_ISP_AWB_AFTER_DRC)) {
            isp_err_trace("isp[%d], detail_en is true, not support awb_switch %d !\n", vi_pipe, wb_cfg->awb_switch);
            return OT_ERR_ISP_NOT_SUPPORT;
        }
    }
#endif
    return TD_SUCCESS;
}

static td_s32 isp_set_wb_stats_config_check_res(ot_vi_pipe vi_pipe, const ot_isp_wb_stats_cfg *wb_cfg,
                                                isp_working_mode *isp_work_mode)
{
    td_u8  max_awb_col, max_awb_row, min_awb_col, min_awb_row;
    td_u16 width, height;

    width  = ot_ext_system_be_total_width_read(vi_pipe);
    height = ot_ext_system_be_total_height_read(vi_pipe);
    max_awb_col = MIN2(OT_ISP_AWB_ZONE_ORIG_COLUMN, width / OT_ISP_AWB_MIN_WIDTH);
    max_awb_row = MIN2(OT_ISP_AWB_ZONE_ORIG_ROW, height / OT_ISP_AWB_MIN_HEIGHT);
    min_awb_col = MAX2(isp_work_mode->block_num, (width + OT_ISP_AWB_ZONE_MAX_WIDTH - 1) / OT_ISP_AWB_ZONE_MAX_WIDTH);
    min_awb_row = MAX2(1, (height + OT_ISP_AWB_ZONE_MAX_HEIGHT - 1) / OT_ISP_AWB_ZONE_MAX_HEIGHT);
    if (wb_cfg->zone_row < min_awb_row) {
        isp_err_trace("zone_row should be larger than %u(height / %d)\n", min_awb_row, OT_ISP_AWB_ZONE_MAX_HEIGHT);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->zone_row > max_awb_row) {
        isp_err_trace("zone_row should be less than %u(MIN(%d, height / %d))\n", max_awb_row,
            OT_ISP_AWB_ZONE_ORIG_ROW, OT_ISP_AWB_MIN_HEIGHT);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->zone_col < min_awb_col) {
        isp_err_trace("zone_col should be larger than %u(MAX(block_num, width / %d))\n", min_awb_col,
            OT_ISP_AWB_ZONE_MAX_WIDTH);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->zone_col > max_awb_col) {
        isp_err_trace("zone_col should be less than %u(MIN(%d, width / %d))\n", max_awb_col,
            OT_ISP_AWB_ZONE_ORIG_COLUMN, OT_ISP_AWB_MIN_WIDTH);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    isp_check_bool_return(wb_cfg->crop.enable);

    if (wb_cfg->crop.width < wb_cfg->zone_col * OT_ISP_AWB_MIN_WIDTH) {
        isp_err_trace("w should NOT be less than %d(zone_col * %d)!\n", wb_cfg->zone_col * OT_ISP_AWB_MIN_WIDTH,
            OT_ISP_AWB_MIN_WIDTH);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (wb_cfg->crop.height < wb_cfg->zone_row * OT_ISP_AWB_MIN_HEIGHT) {
        isp_err_trace("h should NOT be less than %d(zone_row * %d)!\n", wb_cfg->zone_row * OT_ISP_AWB_MIN_HEIGHT,
            OT_ISP_AWB_MIN_HEIGHT);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((wb_cfg->crop.x + wb_cfg->crop.width) > width) {
        isp_err_trace("x + w should NOT be larger than %u!\n", width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((wb_cfg->crop.y + wb_cfg->crop.height) > height) {
        isp_err_trace("y + h should NOT be larger than %u!\n", height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_set_wb_stats_config(ot_vi_pipe vi_pipe, const ot_isp_wb_stats_cfg *wb_cfg,
                                      isp_working_mode *isp_work_mode)
{
    td_s32 ret;
    ot_isp_ctrl_param isp_ctrl_param;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_CTRL_PARAM, &isp_ctrl_param);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    if (isp_ctrl_param.alg_run_select == OT_ISP_ALG_RUN_FE_ONLY) {
        return TD_SUCCESS;
    }

    ret = isp_set_wb_stats_config_check_para(vi_pipe, wb_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_set_wb_stats_config_check_res(vi_pipe, wb_cfg, isp_work_mode);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (wb_cfg->awb_switch == OT_ISP_AWB_AFTER_DG) {
        ot_ext_system_awb_switch_write(vi_pipe, OT_EXT_SYSTEM_AWB_SWITCH_AFTER_DG);
    } else if (wb_cfg->awb_switch == OT_ISP_AWB_AFTER_DRC) {
        ot_ext_system_awb_switch_write(vi_pipe, OT_EXT_SYSTEM_AWB_SWITCH_AFTER_DRC);
    } else if (wb_cfg->awb_switch == OT_ISP_AWB_AFTER_EXPANDER) {
        ot_ext_system_awb_switch_write(vi_pipe, OT_EXT_SYSTEM_AWB_SWITCH_AFTER_EXPANDER);
    } else {
        isp_err_trace("not support!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_awb_white_level_write(vi_pipe, wb_cfg->white_level);
    ot_ext_system_awb_black_level_write(vi_pipe, wb_cfg->black_level);
    ot_ext_system_awb_cr_ref_max_write(vi_pipe, wb_cfg->cr_max);
    ot_ext_system_awb_cr_ref_min_write(vi_pipe, wb_cfg->cr_min);
    ot_ext_system_awb_cb_ref_max_write(vi_pipe, wb_cfg->cb_max);
    ot_ext_system_awb_cb_ref_min_write(vi_pipe, wb_cfg->cb_min);

    ot_ext_system_awb_vnum_write(vi_pipe, wb_cfg->zone_row);
    ot_ext_system_awb_hnum_write(vi_pipe, wb_cfg->zone_col);

    ot_ext_system_awb_crop_en_write(vi_pipe, wb_cfg->crop.enable);
    ot_ext_system_awb_crop_x_write(vi_pipe, wb_cfg->crop.x);
    ot_ext_system_awb_crop_y_write(vi_pipe, wb_cfg->crop.y);
    ot_ext_system_awb_crop_height_write(vi_pipe, wb_cfg->crop.height);
    ot_ext_system_awb_crop_width_write(vi_pipe, wb_cfg->crop.width);

    ot_ext_system_wb_statistics_mpi_update_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

static td_s32 isp_ae_stats_hist_config_check(ot_vi_pipe vi_pipe, const ot_isp_ae_stats_cfg *ae_cfg)
{
    if (ae_cfg->hist_config.hist_skip_x >= OT_ISP_AE_HIST_SKIP_BUTT) {
        isp_err_trace("hist_skip_x should not be larger than 6\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->hist_config.hist_skip_y >= OT_ISP_AE_HIST_SKIP_BUTT) {
        isp_err_trace("hist_skip_y should not be larger than 6\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->hist_config.hist_offset_x >= OT_ISP_AE_HIST_OFFSET_X_BUTT) {
        isp_err_trace("hist_offset_x should not be larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->hist_config.hist_offset_y >= OT_ISP_AE_HIST_OFFSET_Y_BUTT) {
        isp_err_trace("hist_offset_y should not larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ae_cfg->four_plane_mode >= OT_ISP_AE_FOUR_PLANE_MODE_BUTT) {
        isp_err_trace("four_plane_mode should not be larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((ae_cfg->four_plane_mode == OT_ISP_AE_FOUR_PLANE_MODE_DISABLE) &&
        (ae_cfg->hist_config.hist_skip_x == OT_ISP_AE_HIST_SKIP_EVERY_PIXEL)) {
        isp_err_trace("hist_skip_x should not be 0 when not in four_plane_mode\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->hist_mode >= OT_ISP_AE_STAT_MODE_BUTT) {
        isp_err_trace("hist_mode should not be larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->aver_mode >= OT_ISP_AE_STAT_MODE_BUTT) {
        isp_err_trace("aver_mode should not be larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ae_cfg->max_gain_mode >= OT_ISP_AE_STAT_MODE_BUTT) {
        isp_err_trace("max_gain_mode should not be larger than 1\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_unused(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_ae_crop_config_check(const ot_isp_ae_crop *crop, td_u16 width, td_u16 height)
{
    isp_check_bool_return(crop->enable);

    if (crop->width < OT_ISP_AE_MIN_WIDTH) {
        isp_err_trace("crop_width should be lager than 256!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (crop->height < OT_ISP_AE_MIN_HEIGHT) {
        isp_err_trace("crop_height should be lager than 120!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((crop->x + crop->width) > width) {
        isp_err_trace("x + w should not be lager than %u!\n", width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((crop->y + crop->height) > height) {
        isp_err_trace("y + h should not be lager than %u!\n", height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ae_stats_crop_config_check(ot_vi_pipe vi_pipe, const ot_isp_ae_stats_cfg *ae_cfg)
{
    td_s32 ret;
    td_u16 width, height;

    width  = ot_ext_system_be_total_width_read(vi_pipe);
    height = ot_ext_system_be_total_height_read(vi_pipe);
    ret = isp_ae_crop_config_check(&ae_cfg->crop, width, height);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set ae be_crop failed!\n");
        return ret;
    }

    width  = ot_ext_sync_total_width_read(vi_pipe);
    height = ot_ext_sync_total_height_read(vi_pipe);
    ret = isp_ae_crop_config_check(&ae_cfg->fe_crop, width, height);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set ae fe_crop failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ae_stats_config_check(ot_vi_pipe vi_pipe, const ot_isp_ae_stats_cfg *ae_cfg)
{
    td_s32 i, j, ret;
    if (ae_cfg->ae_switch >= OT_ISP_AE_SWITCH_BUTT) {
        isp_err_trace("Invalid AE switch %d!\n", ae_cfg->ae_switch);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ret = isp_ae_stats_hist_config_check(vi_pipe, ae_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            if (ae_cfg->weight[i][j] > 0xF) {
                isp_err_trace("weight[%d][%d] should not be larger than 0xF!\n", i, j);
                return OT_ERR_ISP_ILLEGAL_PARAM;
            }
        }
    }

    ret = isp_ae_stats_crop_config_check(vi_pipe, ae_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_set_ae_stats_config(ot_vi_pipe vi_pipe, const ot_isp_ae_stats_cfg *ae_cfg)
{
    td_s32 ret;
    td_s32 i, j;
    ret = isp_ae_stats_config_check(vi_pipe, ae_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_ae_fourplanemode_write(vi_pipe, ae_cfg->four_plane_mode);
    ot_ext_system_ae_hist_skip_x_write(vi_pipe, ae_cfg->hist_config.hist_skip_x);
    ot_ext_system_ae_hist_skip_y_write(vi_pipe, ae_cfg->hist_config.hist_skip_y);
    ot_ext_system_ae_hist_offset_x_write(vi_pipe, ae_cfg->hist_config.hist_offset_x);
    ot_ext_system_ae_hist_offset_y_write(vi_pipe, ae_cfg->hist_config.hist_offset_y);
    ot_ext_system_ae_histmode_write(vi_pipe, ae_cfg->hist_mode);
    ot_ext_system_ae_avermode_write(vi_pipe, ae_cfg->aver_mode);
    ot_ext_system_ae_maxgainmode_write(vi_pipe, ae_cfg->max_gain_mode);
    ot_ext_system_ae_be_sel_write(vi_pipe, ae_cfg->ae_switch);
    ot_ext_system_ae_crop_en_write(vi_pipe, ae_cfg->crop.enable);
    ot_ext_system_ae_crop_x_write(vi_pipe, ae_cfg->crop.x);
    ot_ext_system_ae_crop_y_write(vi_pipe, ae_cfg->crop.y);
    ot_ext_system_ae_crop_height_write(vi_pipe, ae_cfg->crop.height);
    ot_ext_system_ae_crop_width_write(vi_pipe, ae_cfg->crop.width);

    ot_ext_system_ae_fe_crop_en_write(vi_pipe, ae_cfg->fe_crop.enable);
    ot_ext_system_ae_fe_crop_x_write(vi_pipe, ae_cfg->fe_crop.x);
    ot_ext_system_ae_fe_crop_y_write(vi_pipe, ae_cfg->fe_crop.y);
    ot_ext_system_ae_fe_crop_height_write(vi_pipe, ae_cfg->fe_crop.height);
    ot_ext_system_ae_fe_crop_width_write(vi_pipe, ae_cfg->fe_crop.width);

    /* set 15*17 weight table */
    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ot_ext_system_ae_weight_table_write(vi_pipe, (i * OT_ISP_AE_ZONE_COLUMN + j), ae_cfg->weight[i][j]);
        }
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_AF_SUPPORT
static td_s32 isp_af_crop_config_check(const ot_isp_af_crop *crop, td_u16 width, td_u16 height)
{
    isp_check_bool_return(crop->enable);

    if (crop->width < OT_ISP_AF_MIN_WIDTH) {
        isp_err_trace("w should be larger than 256!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (crop->height < OT_ISP_AF_MIN_HEIGHT) {
        isp_err_trace("h should be larger than 120!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((crop->x + crop->width) > width) {
        isp_err_trace("x + w should NOT be larger than %u!\n", width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((crop->y + crop->height) > height) {
        isp_err_trace("y + h should NOT be larger than %u!\n", height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (crop->width % OT_ISP_AF_ALIGN_WIDTH != 0) {
        isp_err_trace("Invalid crop w:%u!\n", crop->width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (crop->height % OT_ISP_AF_ALIGN_HEIGHT != 0) {
        isp_err_trace("Invalid crop h:%u!\n", crop->height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_af_stats_crop_config_check(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    td_s32 ret;
    td_u16 width, height;

    width  = ot_ext_system_be_total_width_read(vi_pipe);
    height = ot_ext_system_be_total_height_read(vi_pipe);
    ret = isp_af_crop_config_check(&af_cfg->config.crop, width, height);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set af be_crop failed!\n");
        return ret;
    }

    width  = ot_ext_sync_total_width_read(vi_pipe);
    height = ot_ext_sync_total_height_read(vi_pipe);
    ret = isp_af_crop_config_check(&af_cfg->config.fe_crop, width, height);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set af fe_crop failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_af_stats_config_check(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg,
                                        isp_working_mode *isp_work_mode)
{
    td_s32 ret;
    if (af_cfg->config.zone_col < isp_work_mode->block_num) {
        isp_err_trace("the value of AF zone_col should be >= block_num:%u!\n", isp_work_mode->block_num);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    isp_max_param_check_return(af_cfg->config.af_en,       0x1,                 "Err af_en!\n");
    isp_param_check_return(af_cfg->config.zone_col,        0x1, OT_ISP_AF_ZONE_COLUMN, "Err zone_col!\n");
    isp_param_check_return(af_cfg->config.zone_row,        0x1, OT_ISP_AF_ZONE_ROW, "Err zone_row!\n");
    isp_max_param_check_return(af_cfg->config.peak_mode,   0x1,                "Err peak_mode!\n");
    isp_max_param_check_return(af_cfg->config.square_mode, 0x1,                "Err squ_mode!\n");
    isp_max_param_check_return(af_cfg->config.stats_pos, 0x2, "Err statistics_pos!\n");
    isp_max_param_check_return(af_cfg->config.raw_cfg.gamma_value, OT_ISP_AF_GAMMA_VALUE_MAX, "Err gamma_value!\n");
    isp_max_param_check_return(af_cfg->config.raw_cfg.gamma_gain_limit, 0x5, "Err raw_cfg.gamma_gain_limit!\n");
    isp_max_param_check_return(af_cfg->config.raw_cfg.bayer_format, 0x3, "Err raw_cfg.bayer_format!\n");
    isp_max_param_check_return(af_cfg->config.pre_flt_cfg.enable, 0x1, "Err pre_flt_cfg.enable!\n");
    isp_max_param_check_return(af_cfg->config.pre_flt_cfg.strength, 0xFFFF, "Err pre_flt_cfg.u16strength!\n");
    isp_max_param_check_return(af_cfg->config.high_luma_threshold, 0xFF, "Err high_luma_th!\n");

    ret = isp_af_stats_crop_config_check(vi_pipe, af_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_af_stats_h_param_check(const ot_isp_af_h_param *h_param, const char *name)
{
    td_s32 i;
    isp_max_param_check_return(h_param->narrow_band_en, 0x1, "Err %s.narrow_band!\n", name);
    for (i = 0; i < OT_ISP_IIR_EN_NUM; i++) {
        isp_max_param_check_return(h_param->iir_en[i], 0x1, "Err %s.iir_en[%d]!\n", name, i);
    }

    isp_max_param_check_return(h_param->iir_shift, 0x3F, "Err %s.iir_delay!\n", name);
    isp_param_check_return(h_param->iir_gain[0], 0, 0xFF, "Err %s.iir_gain[0]\n!", name);
    for (i = 1; i < OT_ISP_IIR_GAIN_NUM; i++) {
        isp_param_check_return(h_param->iir_gain[i], -511, 511, "Err %s.iir_gain[%d]\n!", name, i);  /* [-511, 511] */
    }
    for (i = 0; i < OT_ISP_IIR_SHIFT_NUM; i++) {
        isp_max_param_check_return(h_param->iir_shift_lut[i], 0x7, "Err %s.iir_shift_lut[%d]!\n", name, i);
    }

    isp_max_param_check_return(h_param->level_depend.enable, 0x1, "Err %s.ld.ld_enable!\n", name);
    isp_max_param_check_return(h_param->level_depend.threshold_low, 0xFF, "Err %s.ld.th_low!\n", name);
    isp_max_param_check_return(h_param->level_depend.gain_low, 0xFF, "Err %s.ld.gain_low!\n", name);
    isp_max_param_check_return(h_param->level_depend.slope_low, 0xF, "Err %s.ld.slp_low!\n", name);
    isp_max_param_check_return(h_param->level_depend.threshold_high, 0xFF, "Err %s.ld.th_high\n", name);
    isp_max_param_check_return(h_param->level_depend.gain_high, 0xFF, "Err %s.ld.gain_high!\n", name);
    isp_max_param_check_return(h_param->level_depend.slope_high, 0xF, "Err %s.ld.slp_high!\n", name);
    isp_max_param_check_return(h_param->coring.threshold, 0x7FF, "Err %s.coring.threshold!\n", name);
    isp_max_param_check_return(h_param->coring.slope, 0xF, "Err %s.coring.slope!\n", name);
    isp_max_param_check_return(h_param->coring.limit, 0x7FF, "Err %s.coring.limit!\n", name);
    return TD_SUCCESS;
}

static td_s32 isp_af_stats_v_param_check(const ot_isp_af_v_param *v_param, const char *name)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_FIR_GAIN_NUM; i++) {
        isp_param_check_return(v_param->fir_gain[i], -31, 31, "Err %s.fir_gain[%u]\n!", name, i);  /* [-31, 31] */
    }
    isp_max_param_check_return(v_param->level_depend.enable, 0x1, "Err %s.level_depend.ld_enable!\n", name);
    isp_max_param_check_return(v_param->level_depend.threshold_low, 0xFF, "Err %s.level_depend.th_low!\n", name);
    isp_max_param_check_return(v_param->level_depend.gain_low, 0xFF, "Err %s.level_depend.gain_low!\n", name);
    isp_max_param_check_return(v_param->level_depend.slope_low, 0xF, "Err %s.level_depend.slope_low!\n", name);
    isp_max_param_check_return(v_param->level_depend.threshold_high, 0xFF, "Err %s.level_depend.th_high\n", name);
    isp_max_param_check_return(v_param->level_depend.gain_high, 0xFF, "Err %s.level_depend.gain_high!\n", name);
    isp_max_param_check_return(v_param->level_depend.slope_high, 0xF, "Err %s.level_depend.slope_high!\n", name);
    isp_max_param_check_return(v_param->coring.threshold, 0x7FF, "Err %s.coring.threshold!\n", name);
    isp_max_param_check_return(v_param->coring.slope, 0xF, "Err %s.coring.slope!\n", name);
    isp_max_param_check_return(v_param->coring.limit, 0x7FF, "Err %s.coring.limit!\n", name);

    return TD_SUCCESS;
}

static td_s32 isp_af_stats_fv_param_check(const ot_isp_af_fv_param *fv_param)
{
    td_u8 i;
    isp_max_param_check_return(fv_param->acc_shift_y, 0xF, "Err fv_param.acc_shift_y!\n");
    isp_max_param_check_return(fv_param->hl_cnt_shift, 0xF, "Err fv_param.hl_cnt_shift!\n");

    for (i = 0; i < OT_ISP_ACC_SHIFT_H_NUM; i++) {
        isp_max_param_check_return(fv_param->acc_shift_h[i], 0xF, "Err fv_param.acc_shift_h[%u]!\n", i);
    }

    for (i = 0; i < OT_ISP_ACC_SHIFT_V_NUM; i++) {
        isp_max_param_check_return(fv_param->acc_shift_v[i], 0xF, "Err fv_param.acc_shift_v[%u]!\n", i);
    }

    return TD_SUCCESS;
}

#endif
static td_s32 isp_set_af_stats_config(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg,
                                      isp_working_mode *isp_work_mode)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_s32 ret;

    ret = isp_af_stats_config_check(vi_pipe, af_cfg, isp_work_mode);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_af_stats_h_param_check(&af_cfg->h_param_iir0, "h_param_iir0");
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_af_stats_h_param_check(&af_cfg->h_param_iir1, "h_param_iir1");
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_af_stats_v_param_check(&af_cfg->v_param_fir0, "v_param_fir0");
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_af_stats_v_param_check(&af_cfg->v_param_fir1, "v_param_fir1");
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* FVPARAM */
    ret = isp_af_stats_fv_param_check(&af_cfg->fv_param);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_af_stats_config_write(vi_pipe, af_cfg);
    ot_ext_af_set_flag_write(vi_pipe, OT_EXT_AF_SET_FLAG_ENABLE);
#endif
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_stats_cfg(ot_vi_pipe vi_pipe, const ot_isp_stats_cfg *stat_cfg)
{
    td_s32 ret;
    td_u32 key_lowbit, key_highbit;
    isp_working_mode isp_work_mode;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    if (isp_work_mode.data_input_mode == ISP_MODE_RAW) {
        /* AWB */
        ret = isp_set_wb_stats_config(vi_pipe, &stat_cfg->wb_cfg, &isp_work_mode);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        /* AE */
        ret = isp_set_ae_stats_config(vi_pipe, &stat_cfg->ae_cfg);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    ret = isp_set_af_stats_config(vi_pipe, &stat_cfg->focus_cfg, &isp_work_mode);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    key_lowbit = (td_u32)(stat_cfg->ctrl.key);
    key_highbit = (td_u32)(stat_cfg->update.key);
    ot_ext_system_statistics_ctrl_lowbit_write(vi_pipe, key_lowbit);
    ot_ext_system_statistics_ctrl_highbit_write(vi_pipe, key_highbit);

    return TD_SUCCESS;
}

static td_void isp_get_wb_stats_config(ot_vi_pipe vi_pipe, ot_isp_wb_stats_cfg *wb_cfg)
{
    td_u8  tmp;
    tmp = ot_ext_system_awb_switch_read(vi_pipe);
    if (tmp == OT_EXT_SYSTEM_AWB_SWITCH_AFTER_DG) {
        wb_cfg->awb_switch = OT_ISP_AWB_AFTER_DG;
    } else if (tmp == OT_EXT_SYSTEM_AWB_SWITCH_AFTER_EXPANDER) {
        wb_cfg->awb_switch = OT_ISP_AWB_AFTER_EXPANDER;
    } else if (tmp == OT_EXT_SYSTEM_AWB_SWITCH_AFTER_DRC) {
        wb_cfg->awb_switch = OT_ISP_AWB_AFTER_DRC;
    } else {
        wb_cfg->awb_switch = OT_ISP_AWB_SWITCH_BUTT;
    }

    wb_cfg->zone_row = ot_ext_system_awb_vnum_read(vi_pipe);
    wb_cfg->zone_col = ot_ext_system_awb_hnum_read(vi_pipe);

    wb_cfg->white_level = ot_ext_system_awb_white_level_read(vi_pipe);
    wb_cfg->black_level = ot_ext_system_awb_black_level_read(vi_pipe);
    wb_cfg->cr_max = ot_ext_system_awb_cr_ref_max_read(vi_pipe);
    wb_cfg->cr_min = ot_ext_system_awb_cr_ref_min_read(vi_pipe);
    wb_cfg->cb_max = ot_ext_system_awb_cb_ref_max_read(vi_pipe);
    wb_cfg->cb_min = ot_ext_system_awb_cb_ref_min_read(vi_pipe);
    wb_cfg->crop.enable = ot_ext_system_awb_crop_en_read(vi_pipe);
    wb_cfg->crop.x = ot_ext_system_awb_crop_x_read(vi_pipe);
    wb_cfg->crop.y = ot_ext_system_awb_crop_y_read(vi_pipe);
    wb_cfg->crop.height = ot_ext_system_awb_crop_height_read(vi_pipe);
    wb_cfg->crop.width = ot_ext_system_awb_crop_width_read(vi_pipe);
}

static td_void isp_get_ae_stats_config(ot_vi_pipe vi_pipe, ot_isp_ae_stats_cfg *ae_cfg)
{
    td_u8  tmp;
    td_s32 i, j;
    ae_cfg->ae_switch = ot_ext_system_ae_be_sel_read(vi_pipe);
    tmp = ot_ext_system_ae_fourplanemode_read(vi_pipe);
    ae_cfg->four_plane_mode = (tmp >= OT_ISP_AE_FOUR_PLANE_MODE_BUTT) ? OT_ISP_AE_FOUR_PLANE_MODE_BUTT : tmp;

    ae_cfg->hist_config.hist_skip_x = ot_ext_system_ae_hist_skip_x_read(vi_pipe);
    ae_cfg->hist_config.hist_skip_y = ot_ext_system_ae_hist_skip_y_read(vi_pipe);
    ae_cfg->hist_config.hist_offset_x = ot_ext_system_ae_hist_offset_x_read(vi_pipe);
    ae_cfg->hist_config.hist_offset_y = ot_ext_system_ae_hist_offset_y_read(vi_pipe);
    ae_cfg->hist_mode = ot_ext_system_ae_histmode_read(vi_pipe);
    ae_cfg->aver_mode = ot_ext_system_ae_avermode_read(vi_pipe);
    ae_cfg->max_gain_mode = ot_ext_system_ae_maxgainmode_read(vi_pipe);

    ae_cfg->crop.enable = ot_ext_system_ae_crop_en_read(vi_pipe);
    ae_cfg->crop.x = ot_ext_system_ae_crop_x_read(vi_pipe);
    ae_cfg->crop.y = ot_ext_system_ae_crop_y_read(vi_pipe);
    ae_cfg->crop.height = ot_ext_system_ae_crop_height_read(vi_pipe);
    ae_cfg->crop.width = ot_ext_system_ae_crop_width_read(vi_pipe);

    ae_cfg->fe_crop.enable = ot_ext_system_ae_fe_crop_en_read(vi_pipe);
    ae_cfg->fe_crop.x = ot_ext_system_ae_fe_crop_x_read(vi_pipe);
    ae_cfg->fe_crop.y = ot_ext_system_ae_fe_crop_y_read(vi_pipe);
    ae_cfg->fe_crop.height = ot_ext_system_ae_fe_crop_height_read(vi_pipe);
    ae_cfg->fe_crop.width = ot_ext_system_ae_fe_crop_width_read(vi_pipe);

    /* set 15*17 weight table */
    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ae_cfg->weight[i][j] = ot_ext_system_ae_weight_table_read(vi_pipe, (i * OT_ISP_AE_ZONE_COLUMN + j));
        }
    }
}

#ifdef CONFIG_OT_ISP_AF_SUPPORT
static td_void isp_af_stats_config_read(ot_vi_pipe vi_pipe, ot_isp_af_cfg *config)
{
    config->af_en = (ot_ext_system_af_enable_read(vi_pipe) != 0x0) ? 1 : 0;
    config->peak_mode       = ot_ext_af_peakmode_read(vi_pipe);
    config->square_mode     = ot_ext_af_squmode_read(vi_pipe);
    config->zone_col        = ot_ext_af_window_hnum_read(vi_pipe);
    config->zone_row        = ot_ext_af_window_vnum_read(vi_pipe);
    /* AF crop */
    config->crop.enable = ot_ext_af_crop_enable_read(vi_pipe);
    config->crop.x = ot_ext_af_crop_pos_x_read(vi_pipe);
    config->crop.y = ot_ext_af_crop_pos_y_read(vi_pipe);
    config->crop.width = ot_ext_af_crop_h_size_read(vi_pipe);
    config->crop.height = ot_ext_af_crop_v_size_read(vi_pipe);

    config->fe_crop.enable = ot_ext_af_fe_crop_enable_read(vi_pipe);
    config->fe_crop.x = ot_ext_af_fe_crop_pos_x_read(vi_pipe);
    config->fe_crop.y = ot_ext_af_fe_crop_pos_y_read(vi_pipe);
    config->fe_crop.width = ot_ext_af_fe_crop_h_size_read(vi_pipe);
    config->fe_crop.height = ot_ext_af_fe_crop_v_size_read(vi_pipe);

    /* AF raw cfg */
    config->stats_pos = ot_ext_af_pos_sel_read(vi_pipe);
    config->raw_cfg.gamma_gain_limit = ot_ext_af_gain_limit_read(vi_pipe);
    config->raw_cfg.gamma_value = ot_ext_af_gamma_read(vi_pipe);
    config->raw_cfg.bayer_format = ot_ext_af_bayermode_read(vi_pipe);

    /* AF pre median filter */
    config->pre_flt_cfg.enable = ot_ext_af_mean_enable_read(vi_pipe);
    config->pre_flt_cfg.strength = ot_ext_af_mean_thres_read(vi_pipe);
    /* high luma counter */
    config->high_luma_threshold = ot_ext_af_highlight_thre_read(vi_pipe);
}

static td_void isp_af_stats_h_param_iir0_read(ot_vi_pipe vi_pipe, ot_isp_af_h_param *h_param_iir0)
{
    h_param_iir0->iir_en[0] = ot_ext_af_iir0_enable0_read(vi_pipe);  /* iir0_enable0 */
    h_param_iir0->iir_en[1] = ot_ext_af_iir0_enable1_read(vi_pipe);  /* iir0_enable1 */
    h_param_iir0->iir_en[2] = ot_ext_af_iir0_enable2_read(vi_pipe);  /* iir0_enable2 */
    h_param_iir0->iir_shift = ot_ext_af_iir0_shift_read(vi_pipe);
    h_param_iir0->iir_gain[0] = ot_ext_af_iir_gain0_group0_read(vi_pipe); /* iir_gain0_group0 */
    h_param_iir0->iir_gain[1] = ot_ext_af_iir_gain1_group0_read(vi_pipe); /* iir_gain1_group0 */
    h_param_iir0->iir_gain[2] = ot_ext_af_iir_gain2_group0_read(vi_pipe); /* iir_gain2_group0 */
    h_param_iir0->iir_gain[3] = ot_ext_af_iir_gain3_group0_read(vi_pipe); /* iir_gain3_group0 */
    h_param_iir0->iir_gain[4] = ot_ext_af_iir_gain4_group0_read(vi_pipe); /* iir_gain4_group0 */
    h_param_iir0->iir_gain[5] = ot_ext_af_iir_gain5_group0_read(vi_pipe); /* iir_gain5_group0 */
    h_param_iir0->iir_gain[6] = ot_ext_af_iir_gain6_group0_read(vi_pipe); /* iir_gain6_group0 */
    h_param_iir0->iir_shift_lut[0] = ot_ext_af_iir0_shift_group0_read(vi_pipe); /* iir0_shift_0 */
    h_param_iir0->iir_shift_lut[1] = ot_ext_af_iir1_shift_group0_read(vi_pipe); /* iir1_shift_0 */
    h_param_iir0->iir_shift_lut[2] = ot_ext_af_iir2_shift_group0_read(vi_pipe); /* iir2_shift_0 */
    h_param_iir0->iir_shift_lut[3] = ot_ext_af_iir3_shift_group0_read(vi_pipe); /* iir3_shift_0 */

    h_param_iir0->narrow_band_en  = ot_ext_af_iir0_ds_enable_read(vi_pipe);
    h_param_iir0->level_depend.enable = ot_ext_af_iir0_ldg_enable_read(vi_pipe);
    h_param_iir0->level_depend.threshold_low  = ot_ext_af_iir_thre0_low_read(vi_pipe);
    h_param_iir0->level_depend.threshold_high = ot_ext_af_iir_thre0_high_read(vi_pipe);
    h_param_iir0->level_depend.slope_low  = ot_ext_af_iir_slope0_low_read(vi_pipe);
    h_param_iir0->level_depend.slope_high = ot_ext_af_iir_slope0_high_read(vi_pipe);
    h_param_iir0->level_depend.gain_low   = ot_ext_af_iir_gain0_low_read(vi_pipe);
    h_param_iir0->level_depend.gain_high  = ot_ext_af_iir_gain0_high_read(vi_pipe);
    h_param_iir0->coring.threshold        = ot_ext_af_iir_thre0_coring_read(vi_pipe);
    h_param_iir0->coring.slope            = ot_ext_af_iir_slope0_coring_read(vi_pipe);
    h_param_iir0->coring.limit            = ot_ext_af_iir_peak0_coring_read(vi_pipe);
}

static td_void isp_af_stats_h_param_iir1_read(ot_vi_pipe vi_pipe, ot_isp_af_h_param *h_param_iir1)
{
    h_param_iir1->iir_en[0] = ot_ext_af_iir1_enable0_read(vi_pipe);  /* iir1_enable0 */
    h_param_iir1->iir_en[1] = ot_ext_af_iir1_enable1_read(vi_pipe);  /* iir1_enable1 */
    h_param_iir1->iir_en[2] = ot_ext_af_iir1_enable2_read(vi_pipe);  /* iir1_enable2 */

    h_param_iir1->iir_shift = ot_ext_af_iir1_shift_read(vi_pipe);
    h_param_iir1->iir_gain[0] = ot_ext_af_iir_gain0_group1_read(vi_pipe); /* iir_gain0_group1 */
    h_param_iir1->iir_gain[1] = ot_ext_af_iir_gain1_group1_read(vi_pipe); /* iir_gain1_group1 */
    h_param_iir1->iir_gain[2] = ot_ext_af_iir_gain2_group1_read(vi_pipe); /* iir_gain2_group1 */
    h_param_iir1->iir_gain[3] = ot_ext_af_iir_gain3_group1_read(vi_pipe); /* iir_gain3_group1 */
    h_param_iir1->iir_gain[4] = ot_ext_af_iir_gain4_group1_read(vi_pipe); /* iir_gain4_group1 */
    h_param_iir1->iir_gain[5] = ot_ext_af_iir_gain5_group1_read(vi_pipe); /* iir_gain5_group1 */
    h_param_iir1->iir_gain[6] = ot_ext_af_iir_gain6_group1_read(vi_pipe); /* iir_gain6_group1 */

    h_param_iir1->iir_shift_lut[0] = ot_ext_af_iir0_shift_group1_read(vi_pipe); /* iir0_shift_1 */
    h_param_iir1->iir_shift_lut[1] = ot_ext_af_iir1_shift_group1_read(vi_pipe); /* iir1_shift_1 */
    h_param_iir1->iir_shift_lut[2] = ot_ext_af_iir2_shift_group1_read(vi_pipe); /* iir2_shift_1 */
    h_param_iir1->iir_shift_lut[3] = ot_ext_af_iir3_shift_group1_read(vi_pipe); /* iir3_shift_1 */

    h_param_iir1->narrow_band_en = ot_ext_af_iir1_ds_enable_read(vi_pipe);
    h_param_iir1->level_depend.enable       = ot_ext_af_iir1_ldg_enable_read(vi_pipe);
    h_param_iir1->level_depend.threshold_low    = ot_ext_af_iir_thre1_low_read(vi_pipe);
    h_param_iir1->level_depend.threshold_high   = ot_ext_af_iir_thre1_high_read(vi_pipe);
    h_param_iir1->level_depend.slope_low   = ot_ext_af_iir_slope1_low_read(vi_pipe);
    h_param_iir1->level_depend.slope_high  = ot_ext_af_iir_slope1_high_read(vi_pipe);
    h_param_iir1->level_depend.gain_low  = ot_ext_af_iir_gain1_low_read(vi_pipe);
    h_param_iir1->level_depend.gain_high = ot_ext_af_iir_gain1_high_read(vi_pipe);
    h_param_iir1->coring.threshold  = ot_ext_af_iir_thre1_coring_read(vi_pipe);
    h_param_iir1->coring.slope = ot_ext_af_iir_slope1_coring_read(vi_pipe);
    h_param_iir1->coring.limit = ot_ext_af_iir_peak1_coring_read(vi_pipe);
}

static td_void isp_af_stats_v_param_fir0_read(ot_vi_pipe vi_pipe, ot_isp_af_v_param *v_param_fir0)
{
    v_param_fir0->fir_gain[0] = ot_ext_af_fir_h_gain0_group0_read(vi_pipe); /* fir_h_gain0_group0 */
    v_param_fir0->fir_gain[1] = ot_ext_af_fir_h_gain1_group0_read(vi_pipe); /* fir_h_gain1_group0 */
    v_param_fir0->fir_gain[2] = ot_ext_af_fir_h_gain2_group0_read(vi_pipe); /* fir_h_gain2_group0 */
    v_param_fir0->fir_gain[3] = ot_ext_af_fir_h_gain3_group0_read(vi_pipe); /* fir_h_gain3_group0 */
    v_param_fir0->fir_gain[4] = ot_ext_af_fir_h_gain4_group0_read(vi_pipe); /* fir_h_gain4_group0 */
    v_param_fir0->level_depend.enable       = ot_ext_af_fir0_ldg_enable_read(vi_pipe);
    v_param_fir0->level_depend.threshold_low    = ot_ext_af_fir_thre0_low_read(vi_pipe);
    v_param_fir0->level_depend.threshold_high   = ot_ext_af_fir_thre0_high_read(vi_pipe);
    v_param_fir0->level_depend.slope_low   = ot_ext_af_fir_slope0_low_read(vi_pipe);
    v_param_fir0->level_depend.slope_high  = ot_ext_af_fir_slope0_high_read(vi_pipe);
    v_param_fir0->level_depend.gain_low  = ot_ext_af_fir_gain0_low_read(vi_pipe);
    v_param_fir0->level_depend.gain_high = ot_ext_af_fir_gain0_high_read(vi_pipe);
    v_param_fir0->coring.threshold  = ot_ext_af_fir_thre0_coring_read(vi_pipe);
    v_param_fir0->coring.slope = ot_ext_af_fir_slope0_coring_read(vi_pipe);
    v_param_fir0->coring.limit = ot_ext_af_fir_peak0_coring_read(vi_pipe);
}

static td_void isp_af_stats_v_param_fir1_read(ot_vi_pipe vi_pipe, ot_isp_af_v_param *v_param_fir1)
{
    v_param_fir1->fir_gain[0] = ot_ext_af_fir_h_gain0_group1_read(vi_pipe); /* fir_h_gain0_group1 */
    v_param_fir1->fir_gain[1] = ot_ext_af_fir_h_gain1_group1_read(vi_pipe); /* fir_h_gain1_group1 */
    v_param_fir1->fir_gain[2] = ot_ext_af_fir_h_gain2_group1_read(vi_pipe); /* fir_h_gain2_group1 */
    v_param_fir1->fir_gain[3] = ot_ext_af_fir_h_gain3_group1_read(vi_pipe); /* fir_h_gain3_group1 */
    v_param_fir1->fir_gain[4] = ot_ext_af_fir_h_gain4_group1_read(vi_pipe); /* fir_h_gain4_group1 */
    v_param_fir1->level_depend.enable          = ot_ext_af_fir1_ldg_enable_read(vi_pipe);
    v_param_fir1->level_depend.threshold_low  = ot_ext_af_fir_thre1_low_read(vi_pipe);
    v_param_fir1->level_depend.threshold_high = ot_ext_af_fir_thre1_high_read(vi_pipe);
    v_param_fir1->level_depend.slope_low      = ot_ext_af_fir_slope1_low_read(vi_pipe);
    v_param_fir1->level_depend.slope_high     = ot_ext_af_fir_slope1_high_read(vi_pipe);
    v_param_fir1->level_depend.gain_low       = ot_ext_af_fir_gain1_low_read(vi_pipe);
    v_param_fir1->level_depend.gain_high      = ot_ext_af_fir_gain1_high_read(vi_pipe);
    v_param_fir1->coring.threshold = ot_ext_af_fir_thre1_coring_read(vi_pipe);
    v_param_fir1->coring.slope = ot_ext_af_fir_slope1_coring_read(vi_pipe);
    v_param_fir1->coring.limit = ot_ext_af_fir_peak1_coring_read(vi_pipe);
}

static td_void isp_af_stats_fv_param_read(ot_vi_pipe vi_pipe, ot_isp_af_fv_param *fv_param)
{
    fv_param->acc_shift_h[0] = ot_ext_af_acc_shift0_h_read(vi_pipe);
    fv_param->acc_shift_h[1] = ot_ext_af_acc_shift1_h_read(vi_pipe);
    fv_param->acc_shift_v[0] = ot_ext_af_acc_shift0_v_read(vi_pipe);
    fv_param->acc_shift_v[1] = ot_ext_af_acc_shift1_v_read(vi_pipe);
    fv_param->acc_shift_y    = ot_ext_af_acc_shift_y_read(vi_pipe);
    fv_param->hl_cnt_shift   = ot_ext_af_shift_count_y_read(vi_pipe);
}

static td_void isp_get_af_stats_config(ot_vi_pipe vi_pipe, ot_isp_focus_stats_cfg *focus_cfg)
{
    isp_af_stats_config_read(vi_pipe, &focus_cfg->config);
    isp_af_stats_h_param_iir0_read(vi_pipe, &focus_cfg->h_param_iir0);
    isp_af_stats_h_param_iir1_read(vi_pipe, &focus_cfg->h_param_iir1);
    isp_af_stats_v_param_fir0_read(vi_pipe, &focus_cfg->v_param_fir0);
    isp_af_stats_v_param_fir1_read(vi_pipe, &focus_cfg->v_param_fir1);
    isp_af_stats_fv_param_read(vi_pipe, &focus_cfg->fv_param);
}
#endif

td_s32 ot_mpi_isp_get_stats_cfg(ot_vi_pipe vi_pipe, ot_isp_stats_cfg *stat_cfg)
{
    td_u32 ctrl, update;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_cfg);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ctrl  = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    update = ot_ext_system_statistics_ctrl_highbit_read(vi_pipe);
    stat_cfg->ctrl.key = ctrl;
    stat_cfg->update.key = update;

    /* AWB STATISTICS CONIFG */
    isp_get_wb_stats_config(vi_pipe, &stat_cfg->wb_cfg);

    /* AE STATISTICS CONIFG */
    isp_get_ae_stats_config(vi_pipe, &stat_cfg->ae_cfg);

#ifdef CONFIG_OT_ISP_AF_SUPPORT
    /* AF STATISTICS CONIFG */
    isp_get_af_stats_config(vi_pipe, &stat_cfg->focus_cfg);
#endif

    return TD_SUCCESS;
}

static td_void isp_get_gamma_def_linear_lut(td_u16 *gamma_lut)
{
    td_u16 gamma_def[OT_ISP_GAMMA_NODE_NUM] = {
        0,   30,  60,  90,  120, 145, 170, 195, 220, 243, 265, 288, 310, 330, 350, 370, 390, 410, 430, 450, 470, 488,
        505, 523, 540, 558, 575, 593, 610, 625, 640, 655, 670, 685, 700, 715, 730, 744, 758, 772, 786, 800, 814, 828,
        842, 855, 868, 881, 894, 907, 919, 932, 944, 957, 969, 982, 994, 1008, 1022, 1036, 1050, 1062, 1073, 1085,
        1096, 1107, 1117, 1128, 1138, 1148, 1158, 1168, 1178, 1188, 1198, 1208, 1218, 1227, 1236, 1245, 1254, 1261,
        1267, 1274, 1280, 1289, 1297, 1306, 1314, 1322, 1330, 1338, 1346, 1354, 1362, 1370, 1378, 1386, 1393, 1401,
        1408, 1416, 1423, 1431, 1438, 1445, 1453, 1460, 1467, 1474, 1480, 1487, 1493, 1500, 1506, 1513, 1519, 1525,
        1531, 1537, 1543, 1549, 1556, 1562, 1568, 1574, 1580, 1586, 1592, 1598, 1604, 1609, 1615, 1621, 1627, 1632,
        1638, 1644, 1650, 1655, 1661, 1667, 1672, 1678, 1683, 1689, 1694, 1700, 1705, 1710, 1716, 1721, 1726, 1732,
        1737, 1743, 1748, 1753, 1759, 1764, 1769, 1774, 1779, 1784, 1789, 1794, 1800, 1805, 1810, 1815, 1820, 1825,
        1830, 1835, 1840, 1844, 1849, 1854, 1859, 1864, 1869, 1874, 1879, 1883, 1888, 1893, 1898, 1902, 1907, 1912,
        1917, 1921, 1926, 1931, 1936, 1940, 1945, 1950, 1954, 1959, 1963, 1968, 1972, 1977, 1981, 1986, 1990, 1995,
        1999, 2004, 2008, 2013, 2017, 2021, 2026, 2030, 2034, 2039, 2043, 2048, 2052, 2056, 2061, 2065, 2069, 2073,
        2078, 2082, 2086, 2090, 2094, 2098, 2102, 2106, 2111, 2115, 2119, 2123, 2128, 2132, 2136, 2140, 2144, 2148,
        2152, 2156, 2160, 2164, 2168, 2172, 2176, 2180, 2184, 2188, 2192, 2196, 2200, 2204, 2208, 2212, 2216, 2220,
        2224, 2227, 2231, 2235, 2239, 2243, 2247, 2251, 2255, 2258, 2262, 2266, 2270, 2273, 2277, 2281, 2285, 2288,
        2292, 2296, 2300, 2303, 2307, 2311, 2315, 2318, 2322, 2326, 2330, 2333, 2337, 2341, 2344, 2348, 2351, 2355,
        2359, 2362, 2366, 2370, 2373, 2377, 2380, 2384, 2387, 2391, 2394, 2398, 2401, 2405, 2408, 2412, 2415, 2419,
        2422, 2426, 2429, 2433, 2436, 2440, 2443, 2447, 2450, 2454, 2457, 2461, 2464, 2467, 2471, 2474, 2477, 2481,
        2484, 2488, 2491, 2494, 2498, 2501, 2504, 2508, 2511, 2515, 2518, 2521, 2525, 2528, 2531, 2534, 2538, 2541,
        2544, 2547, 2551, 2554, 2557, 2560, 2564, 2567, 2570, 2573, 2577, 2580, 2583, 2586, 2590, 2593, 2596, 2599,
        2603, 2606, 2609, 2612, 2615, 2618, 2621, 2624, 2628, 2631, 2634, 2637, 2640, 2643, 2646, 2649, 2653, 2656,
        2659, 2662, 2665, 2668, 2671, 2674, 2677, 2680, 2683, 2686, 2690, 2693, 2696, 2699, 2702, 2705, 2708, 2711,
        2714, 2717, 2720, 2723, 2726, 2729, 2732, 2735, 2738, 2741, 2744, 2747, 2750, 2753, 2756, 2759, 2762, 2764,
        2767, 2770, 2773, 2776, 2779, 2782, 2785, 2788, 2791, 2794, 2797, 2799, 2802, 2805, 2808, 2811, 2814, 2817,
        2820, 2822, 2825, 2828, 2831, 2834, 2837, 2840, 2843, 2845, 2848, 2851, 2854, 2856, 2859, 2862, 2865, 2868,
        2871, 2874, 2877, 2879, 2882, 2885, 2888, 2890, 2893, 2896, 2899, 2901, 2904, 2907, 2910, 2912, 2915, 2918,
        2921, 2923, 2926, 2929, 2932, 2934, 2937, 2940, 2943, 2945, 2948, 2951, 2954, 2956, 2959, 2962, 2964, 2967,
        2969, 2972, 2975, 2977, 2980, 2983, 2986, 2988, 2991, 2994, 2996, 2999, 3001, 3004, 3007, 3009, 3012, 3015,
        3018, 3020, 3023, 3026, 3028, 3031, 3033, 3036, 3038, 3041, 3043, 3046, 3049, 3051, 3054, 3057, 3059, 3062,
        3064, 3067, 3069, 3072, 3074, 3077, 3080, 3082, 3085, 3088, 3090, 3093, 3095, 3098, 3100, 3103, 3105, 3108,
        3110, 3113, 3115, 3118, 3120, 3123, 3125, 3128, 3130, 3133, 3135, 3138, 3140, 3143, 3145, 3148, 3150, 3153,
        3155, 3158, 3160, 3163, 3165, 3168, 3170, 3173, 3175, 3178, 3180, 3183, 3185, 3187, 3190, 3192, 3194, 3197,
        3199, 3202, 3204, 3207, 3209, 3212, 3214, 3217, 3219, 3222, 3224, 3226, 3229, 3231, 3233, 3236, 3238, 3241,
        3243, 3245, 3248, 3250, 3252, 3255, 3257, 3260, 3262, 3264, 3267, 3269, 3271, 3274, 3276, 3279, 3281, 3283,
        3286, 3288, 3290, 3293, 3295, 3298, 3300, 3302, 3305, 3307, 3309, 3311, 3314, 3316, 3318, 3320, 3323, 3325,
        3327, 3330, 3332, 3335, 3337, 3339, 3342, 3344, 3346, 3348, 3351, 3353, 3355, 3357, 3360, 3362, 3364, 3366,
        3369, 3371, 3373, 3375, 3378, 3380, 3382, 3384, 3387, 3389, 3391, 3393, 3396, 3398, 3400, 3402, 3405, 3407,
        3409, 3411, 3414, 3416, 3418, 3420, 3423, 3425, 3427, 3429, 3432, 3434, 3436, 3438, 3441, 3443, 3445, 3447,
        3450, 3452, 3454, 3456, 3459, 3461, 3463, 3465, 3467, 3469, 3471, 3473, 3476, 3478, 3480, 3482, 3485, 3487,
        3489, 3491, 3494, 3496, 3498, 3500, 3502, 3504, 3506, 3508, 3511, 3513, 3515, 3517, 3519, 3521, 3523, 3525,
        3528, 3530, 3532, 3534, 3536, 3538, 3540, 3542, 3545, 3547, 3549, 3551, 3553, 3555, 3557, 3559, 3562, 3564,
        3566, 3568, 3570, 3572, 3574, 3576, 3579, 3581, 3583, 3585, 3587, 3589, 3591, 3593, 3596, 3598, 3600, 3602,
        3604, 3606, 3608, 3610, 3612, 3614, 3616, 3618, 3620, 3622, 3624, 3626, 3629, 3631, 3633, 3635, 3637, 3639,
        3641, 3643, 3645, 3647, 3649, 3651, 3653, 3655, 3657, 3659, 3661, 3663, 3665, 3667, 3670, 3672, 3674, 3676,
        3678, 3680, 3682, 3684, 3686, 3688, 3690, 3692, 3694, 3696, 3698, 3700, 3702, 3704, 3706, 3708, 3710, 3712,
        3714, 3716, 3718, 3720, 3722, 3724, 3726, 3728, 3730, 3732, 3734, 3736, 3738, 3740, 3742, 3744, 3746, 3748,
        3750, 3752, 3754, 3756, 3758, 3760, 3762, 3764, 3766, 3767, 3769, 3771, 3773, 3775, 3777, 3779, 3781, 3783,
        3785, 3787, 3789, 3791, 3793, 3795, 3797, 3799, 3801, 3803, 3805, 3806, 3808, 3810, 3812, 3814, 3816, 3818,
        3820, 3822, 3824, 3826, 3828, 3830, 3832, 3834, 3836, 3837, 3839, 3841, 3843, 3845, 3847, 3849, 3851, 3853,
        3855, 3857, 3859, 3860, 3862, 3864, 3866, 3868, 3870, 3872, 3874, 3875, 3877, 3879, 3881, 3883, 3885, 3887,
        3889, 3890, 3892, 3894, 3896, 3898, 3900, 3902, 3904, 3905, 3907, 3909, 3911, 3913, 3915, 3917, 3919, 3920,
        3922, 3924, 3926, 3928, 3930, 3932, 3934, 3935, 3937, 3939, 3941, 3943, 3945, 3947, 3949, 3950, 3952, 3954,
        3956, 3957, 3959, 3961, 3963, 3965, 3967, 3969, 3971, 3972, 3974, 3976, 3978, 3979, 3981, 3983, 3985, 3987,
        3989, 3991, 3993, 3994, 3996, 3998, 4000, 4001, 4003, 4005, 4007, 4008, 4010, 4012, 4014, 4016, 4018, 4020,
        4022, 4023, 4025, 4027, 4029, 4030, 4032, 4034, 4036, 4037, 4039, 4041, 4043, 4044, 4046, 4048, 4050, 4052,
        4054, 4056, 4058, 4059, 4061, 4063, 4065, 4066, 4068, 4070, 4072, 4073, 4075, 4077, 4079, 4080, 4082, 4084,
        4086, 4087, 4089, 4091, 4092, 4094, 4095
    };
    td_s32 ret;
    ret = memcpy_s(gamma_lut, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16),
                   gamma_def, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16));
    isp_check_eok_void(ret);

    return;
}

static td_void isp_get_gamma_def_wdr_lut(td_u16 *gamma_lut)
{
    td_u16 gamma_def_wdr[OT_ISP_GAMMA_NODE_NUM] = {
        0, 3, 6, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 79, 82,
        85,  88,  91,  94,  97,  100, 103, 106, 109, 112, 114, 117, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147,
        149, 152, 155, 158, 161, 164, 167, 170, 172, 175, 178, 181, 184, 187, 190, 193, 195, 198, 201, 204, 207, 210,
        213, 216, 218, 221, 224, 227, 230, 233, 236, 239, 241, 244, 247, 250, 253, 256, 259, 262, 264, 267, 270, 273,
        276, 279, 282, 285, 287, 290, 293, 296, 299, 302, 305, 308, 310, 313, 316, 319, 322, 325, 328, 331, 333, 336,
        339, 342, 345, 348, 351, 354, 356, 359, 362, 365, 368, 371, 374, 377, 380, 383, 386, 389, 391, 394, 397, 400,
        403, 406, 409, 412, 415, 418, 421, 424, 426, 429, 432, 435, 438, 441, 444, 447, 450, 453, 456, 459, 462, 465,
        468, 471, 474, 477, 480, 483, 486, 489, 492, 495, 498, 501, 504, 507, 510, 513, 516, 519, 522, 525, 528, 531,
        534, 537, 540, 543, 546, 549, 552, 555, 558, 561, 564, 568, 571, 574, 577, 580, 583, 586, 589, 592, 595, 598,
        601, 605, 608, 611, 614, 617, 620, 623, 626, 630, 633, 636, 639, 643, 646, 649, 652, 655, 658, 661, 664, 668,
        671, 674, 677, 681, 684, 687, 690, 694, 697, 700, 703, 707, 710, 713, 716, 720, 723, 726, 730, 733, 737, 740,
        743, 747, 750, 753, 757, 760, 764, 767, 770, 774, 777, 780, 784, 787, 791, 794, 797, 801, 804, 807, 811, 814,
        818, 821, 825, 828, 832, 835, 838, 842, 845, 848, 852, 855, 859, 862, 866, 869, 873, 876, 880, 883, 887, 890,
        894, 897, 901, 904, 908, 911, 915, 918, 922, 925, 929, 932, 936, 939, 943, 946, 950, 954, 957, 961, 965, 968,
        972, 975, 979, 982, 986, 989, 993, 997, 1000, 1004, 1008, 1011, 1015, 1018, 1022, 1026, 1029, 1033, 1037, 1040,
        1044, 1047, 1051, 1055, 1058, 1062, 1066, 1069, 1073, 1076, 1080, 1084, 1087, 1091, 1095, 1099, 1102, 1106,
        1110, 1113, 1117, 1120, 1124, 1128, 1131, 1135, 1139, 1143, 1146, 1150, 1154, 1158, 1161, 1165, 1169, 1173,
        1176, 1180, 1184, 1188, 1191, 1195, 1199, 1203, 1206, 1210, 1214, 1218, 1221, 1225, 1229, 1233, 1236, 1240,
        1244, 1248, 1251, 1255, 1259, 1263, 1266, 1270, 1274, 1278, 1281, 1285, 1289, 1293, 1297, 1301, 1305, 1309,
        1312, 1316, 1320, 1324, 1327, 1331, 1335, 1339, 1343, 1347, 1351, 1355, 1358, 1362, 1366, 1370, 1373, 1377,
        1381, 1385, 1389, 1393, 1397, 1401, 1404, 1408, 1412, 1416, 1420, 1424, 1428, 1432, 1435, 1439, 1443, 1447,
        1451, 1455, 1459, 1463, 1467, 1471, 1475, 1479, 1482, 1486, 1490, 1494, 1498, 1502, 1506, 1510, 1514, 1518,
        1522, 1526, 1529, 1533, 1537, 1541, 1545, 1549, 1553, 1557, 1561, 1565, 1569, 1573, 1577, 1581, 1585, 1589,
        1593, 1597, 1601, 1605, 1609, 1613, 1617, 1621, 1624, 1628, 1632, 1636, 1640, 1644, 1648, 1652, 1656, 1660,
        1664, 1668, 1672, 1676, 1680, 1684, 1688, 1692, 1696, 1700, 1704, 1708, 1712, 1717, 1721, 1725, 1729, 1733,
        1737, 1741, 1745, 1749, 1753, 1757, 1761, 1765, 1769, 1773, 1777, 1781, 1785, 1789, 1793, 1797, 1801, 1805,
        1809, 1813, 1817, 1821, 1825, 1830, 1834, 1838, 1842, 1846, 1850, 1854, 1858, 1862, 1866, 1870, 1874, 1879,
        1883, 1887, 1891, 1895, 1899, 1903, 1907, 1911, 1915, 1919, 1923, 1928, 1932, 1936, 1940, 1944, 1948, 1952,
        1956, 1961, 1965, 1969, 1973, 1977, 1981, 1985, 1989, 1994, 1998, 2002, 2006, 2010, 2014, 2018, 2022, 2027,
        2031, 2035, 2039, 2044, 2048, 2052, 2056, 2060, 2064, 2068, 2072, 2077, 2081, 2085, 2089, 2094, 2098, 2102,
        2106, 2111, 2115, 2119, 2123, 2128, 2132, 2136, 2140, 2144, 2148, 2152, 2156, 2161, 2165, 2169, 2173, 2178,
        2182, 2186, 2190, 2195, 2199, 2203, 2207, 2212, 2216, 2220, 2224, 2229, 2233, 2237, 2241, 2246, 2250, 2254,
        2259, 2263, 2268, 2272, 2276, 2281, 2285, 2289, 2293, 2298, 2302, 2306, 2310, 2315, 2319, 2323, 2327, 2332,
        2336, 2340, 2345, 2349, 2354, 2358, 2362, 2367, 2371, 2375, 2380, 2384, 2389, 2393, 2397, 2402, 2406, 2410,
        2415, 2419, 2424, 2428, 2432, 2437, 2441, 2445, 2450, 2454, 2459, 2463, 2467, 2472, 2476, 2480, 2485, 2489,
        2494, 2498, 2503, 2507, 2512, 2516, 2520, 2525, 2529, 2533, 2538, 2542, 2547, 2551, 2556, 2560, 2565, 2569,
        2574, 2578, 2583, 2587, 2592, 2596, 2601, 2605, 2610, 2614, 2619, 2623, 2628, 2632, 2637, 2641, 2646, 2650,
        2655, 2659, 2664, 2668, 2673, 2677, 2682, 2686, 2691, 2695, 2700, 2704, 2709, 2713, 2718, 2723, 2727, 2732,
        2737, 2741, 2746, 2750, 2755, 2759, 2764, 2768, 2773, 2778, 2782, 2787, 2792, 2796, 2801, 2805, 2810, 2815,
        2819, 2824, 2829, 2833, 2838, 2842, 2847, 2852, 2856, 2861, 2866, 2870, 2875, 2879, 2884, 2889, 2893, 2898,
        2903, 2908, 2912, 2917, 2922, 2927, 2931, 2936, 2941, 2946, 2950, 2955, 2960, 2965, 2969, 2974, 2979, 2984,
        2988, 2993, 2998, 3003, 3008, 3013, 3018, 3023, 3027, 3032, 3037, 3042, 3046, 3051, 3056, 3061, 3066, 3071,
        3076, 3081, 3085, 3090, 3095, 3100, 3105, 3110, 3115, 3120, 3124, 3129, 3134, 3139, 3144, 3149, 3154, 3159,
        3163, 3168, 3173, 3178, 3183, 3188, 3193, 3198, 3203, 3208, 3213, 3218, 3223, 3228, 3233, 3238, 3243, 3248,
        3253, 3258, 3263, 3268, 3273, 3278, 3283, 3288, 3293, 3298, 3303, 3308, 3313, 3318, 3323, 3328, 3333, 3338,
        3343, 3348, 3353, 3358, 3363, 3368, 3373, 3378, 3383, 3388, 3393, 3398, 3403, 3408, 3413, 3418, 3423, 3428,
        3433, 3438, 3443, 3448, 3453, 3458, 3463, 3468, 3473, 3479, 3484, 3489, 3494, 3499, 3504, 3509, 3514, 3519,
        3524, 3529, 3534, 3539, 3544, 3549, 3554, 3560, 3565, 3570, 3575, 3580, 3585, 3590, 3595, 3600, 3605, 3610,
        3615, 3621, 3626, 3631, 3636, 3641, 3646, 3651, 3656, 3661, 3666, 3671, 3676, 3682, 3687, 3692, 3697, 3702,
        3707, 3712, 3717, 3722, 3727, 3732, 3737, 3742, 3747, 3752, 3757, 3763, 3768, 3773, 3778, 3783, 3788, 3793,
        3798, 3803, 3808, 3813, 3818, 3824, 3829, 3834, 3839, 3844, 3849, 3854, 3859, 3864, 3869, 3874, 3879, 3884,
        3889, 3894, 3899, 3904, 3909, 3914, 3919, 3924, 3929, 3934, 3939, 3945, 3950, 3955, 3960, 3965, 3970, 3975,
        3980, 3985, 3990, 3995, 4000, 4005, 4010, 4015, 4020, 4025, 4030, 4035, 4040, 4045, 4050, 4055, 4060, 4065,
        4070, 4075, 4080, 4085, 4090, 4095
    };
    td_s32 ret;
    ret = memcpy_s(gamma_lut, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16),
                   gamma_def_wdr, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16));
    isp_check_eok_void(ret);

    return;
}

static td_void isp_get_gamma_srgb_linear_lut(td_u16 *gamma_lut)
{
    td_u16 gamma_srgb[OT_ISP_GAMMA_NODE_NUM] = {
        0,   18,  36,  54,  72,  90,  108, 126, 144, 162, 180, 198, 216, 234, 252, 270, 288, 306, 324, 343, 360, 377,
        394, 410, 426, 441, 456, 471, 486, 500, 514, 527, 541, 554, 567, 580, 592, 605, 617, 629, 641, 652, 664, 675,
        686, 698, 709, 719, 730, 741, 751, 761, 772, 782, 792, 802, 812, 821, 831, 841, 850, 859, 869, 878, 887, 896,
        905, 914, 923, 931, 940, 949, 957, 966, 974, 983, 991, 999, 1007, 1015, 1024, 1032, 1039, 1047, 1055, 1063,
        1071, 1078, 1086, 1094, 1101, 1109, 1116, 1124, 1131, 1138, 1146, 1153, 1160, 1167, 1174, 1182, 1189, 1196,
        1203, 1210, 1216, 1223, 1230, 1237, 1244, 1250, 1257, 1264, 1270, 1277, 1284, 1290, 1297, 1303, 1310, 1316,
        1322, 1329, 1335, 1341, 1348, 1354, 1360, 1366, 1372, 1379, 1385, 1391, 1397, 1403, 1409, 1415, 1421, 1427,
        1433, 1439, 1444, 1450, 1456, 1462, 1468, 1474, 1479, 1485, 1491, 1496, 1502, 1508, 1513, 1519, 1524, 1530,
        1536, 1541, 1547, 1552, 1557, 1563, 1568, 1574, 1579, 1585, 1590, 1595, 1601, 1606, 1611, 1616, 1622, 1627,
        1632, 1637, 1642, 1648, 1653, 1658, 1663, 1668, 1673, 1678, 1683, 1688, 1693, 1698, 1703, 1708, 1713, 1718,
        1723, 1728, 1733, 1738, 1743, 1748, 1753, 1758, 1762, 1767, 1772, 1777, 1782, 1786, 1791, 1796, 1801, 1805,
        1810, 1815, 1819, 1824, 1829, 1833, 1838, 1843, 1847, 1852, 1857, 1861, 1866, 1870, 1875, 1879, 1884, 1888,
        1893, 1897, 1902, 1906, 1911, 1915, 1920, 1924, 1928, 1933, 1937, 1942, 1946, 1950, 1955, 1959, 1963, 1968,
        1972, 1976, 1981, 1985, 1989, 1994, 1998, 2002, 2006, 2011, 2015, 2019, 2023, 2027, 2032, 2036, 2040, 2044,
        2048, 2052, 2057, 2061, 2065, 2069, 2073, 2077, 2081, 2085, 2089, 2093, 2097, 2102, 2106, 2110, 2114, 2118,
        2122, 2126, 2130, 2134, 2138, 2142, 2146, 2149, 2153, 2157, 2161, 2165, 2169, 2173, 2177, 2181, 2185, 2189,
        2192, 2196, 2200, 2204, 2208, 2212, 2216, 2219, 2223, 2227, 2231, 2235, 2238, 2242, 2246, 2250, 2254, 2257,
        2261, 2265, 2269, 2272, 2276, 2280, 2283, 2287, 2291, 2295, 2298, 2302, 2306, 2309, 2313, 2317, 2320, 2324,
        2328, 2331, 2335, 2338, 2342, 2346, 2349, 2353, 2356, 2360, 2364, 2367, 2371, 2374, 2378, 2381, 2385, 2389,
        2392, 2396, 2399, 2403, 2406, 2410, 2413, 2417, 2420, 2424, 2427, 2431, 2434, 2438, 2441, 2445, 2448, 2451,
        2455, 2458, 2462, 2465, 2469, 2472, 2475, 2479, 2482, 2486, 2489, 2492, 2496, 2499, 2503, 2506, 2509, 2513,
        2516, 2519, 2523, 2526, 2529, 2533, 2536, 2539, 2543, 2546, 2549, 2553, 2556, 2559, 2563, 2566, 2569, 2572,
        2576, 2579, 2582, 2585, 2589, 2592, 2595, 2598, 2602, 2605, 2608, 2611, 2615, 2618, 2621, 2624, 2627, 2631,
        2634, 2637, 2640, 2643, 2647, 2650, 2653, 2656, 2659, 2662, 2666, 2669, 2672, 2675, 2678, 2681, 2684, 2688,
        2691, 2694, 2697, 2700, 2703, 2706, 2709, 2712, 2716, 2719, 2722, 2725, 2728, 2731, 2734, 2737, 2740, 2743,
        2746, 2749, 2752, 2755, 2759, 2762, 2765, 2768, 2771, 2774, 2777, 2780, 2783, 2786, 2789, 2792, 2795, 2798,
        2801, 2804, 2807, 2810, 2813, 2816, 2819, 2822, 2825, 2828, 2831, 2833, 2836, 2839, 2842, 2845, 2848, 2851,
        2854, 2857, 2860, 2863, 2866, 2869, 2872, 2875, 2877, 2880, 2883, 2886, 2889, 2892, 2895, 2898, 2901, 2904,
        2906, 2909, 2912, 2915, 2918, 2921, 2924, 2926, 2929, 2932, 2935, 2938, 2941, 2944, 2946, 2949, 2952, 2955,
        2958, 2961, 2963, 2966, 2969, 2972, 2975, 2977, 2980, 2983, 2986, 2989, 2991, 2994, 2997, 3000, 3003, 3005,
        3008, 3011, 3014, 3016, 3019, 3022, 3025, 3027, 3030, 3033, 3036, 3038, 3041, 3044, 3047, 3049, 3052, 3055,
        3058, 3060, 3063, 3066, 3068, 3071, 3074, 3077, 3079, 3082, 3085, 3087, 3090, 3093, 3095, 3098, 3101, 3103,
        3106, 3109, 3112, 3114, 3117, 3120, 3122, 3125, 3128, 3130, 3133, 3135, 3138, 3141, 3143, 3146, 3149, 3151,
        3154, 3157, 3159, 3162, 3164, 3167, 3170, 3172, 3175, 3178, 3180, 3183, 3185, 3188, 3191, 3193, 3196, 3198,
        3201, 3204, 3206, 3209, 3211, 3214, 3217, 3219, 3222, 3224, 3227, 3229, 3232, 3235, 3237, 3240, 3242, 3245,
        3247, 3250, 3252, 3255, 3257, 3260, 3263, 3265, 3268, 3270, 3273, 3275, 3278, 3280, 3283, 3285, 3288, 3290,
        3293, 3295, 3298, 3300, 3303, 3305, 3308, 3310, 3313, 3315, 3318, 3320, 3323, 3325, 3328, 3330, 3333, 3335,
        3338, 3340, 3343, 3345, 3348, 3350, 3353, 3355, 3358, 3360, 3362, 3365, 3367, 3370, 3372, 3375, 3377, 3380,
        3382, 3385, 3387, 3389, 3392, 3394, 3397, 3399, 3402, 3404, 3406, 3409, 3411, 3414, 3416, 3418, 3421, 3423,
        3426, 3428, 3431, 3433, 3435, 3438, 3440, 3443, 3445, 3447, 3450, 3452, 3454, 3457, 3459, 3462, 3464, 3466,
        3469, 3471, 3474, 3476, 3478, 3481, 3483, 3485, 3488, 3490, 3492, 3495, 3497, 3500, 3502, 3504, 3507, 3509,
        3511, 3514, 3516, 3518, 3521, 3523, 3525, 3528, 3530, 3532, 3535, 3537, 3539, 3542, 3544, 3546, 3549, 3551,
        3553, 3555, 3558, 3560, 3562, 3565, 3567, 3569, 3572, 3574, 3576, 3579, 3581, 3583, 3585, 3588, 3590, 3592,
        3595, 3597, 3599, 3601, 3604, 3606, 3608, 3610, 3613, 3615, 3617, 3620, 3622, 3624, 3626, 3629, 3631, 3633,
        3635, 3638, 3640, 3642, 3644, 3647, 3649, 3651, 3653, 3656, 3658, 3660, 3662, 3665, 3667, 3669, 3671, 3674,
        3676, 3678, 3680, 3682, 3685, 3687, 3689, 3691, 3694, 3696, 3698, 3700, 3702, 3705, 3707, 3709, 3711, 3713,
        3716, 3718, 3720, 3722, 3724, 3727, 3729, 3731, 3733, 3735, 3738, 3740, 3742, 3744, 3746, 3749, 3751, 3753,
        3755, 3757, 3759, 3762, 3764, 3766, 3768, 3770, 3772, 3775, 3777, 3779, 3781, 3783, 3785, 3788, 3790, 3792,
        3794, 3796, 3798, 3800, 3803, 3805, 3807, 3809, 3811, 3813, 3815, 3818, 3820, 3822, 3824, 3826, 3828, 3830,
        3833, 3835, 3837, 3839, 3841, 3843, 3845, 3847, 3850, 3852, 3854, 3856, 3858, 3860, 3862, 3864, 3866, 3869,
        3871, 3873, 3875, 3877, 3879, 3881, 3883, 3885, 3887, 3890, 3892, 3894, 3896, 3898, 3900, 3902, 3904, 3906,
        3908, 3910, 3912, 3915, 3917, 3919, 3921, 3923, 3925, 3927, 3929, 3931, 3933, 3935, 3937, 3939, 3942, 3944,
        3946, 3948, 3950, 3952, 3954, 3956, 3958, 3960, 3962, 3964, 3966, 3968, 3970, 3972, 3974, 3976, 3978, 3980,
        3983, 3985, 3987, 3989, 3991, 3993, 3995, 3997, 3999, 4001, 4003, 4005, 4007, 4009, 4011, 4013, 4015, 4017,
        4019, 4021, 4023, 4025, 4027, 4029, 4031, 4033, 4035, 4037, 4039, 4041, 4043, 4045, 4047, 4049, 4051, 4053,
        4055, 4057, 4059, 4061, 4063, 4065, 4067, 4069, 4071, 4073, 4075, 4077, 4079, 4081, 4083, 4085, 4087, 4089,
        4091, 4093, 4095
    };
    td_s32 ret;
    ret = memcpy_s(gamma_lut, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16),
                   gamma_srgb, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16));
    isp_check_eok_void(ret);

    return;
}

static td_void isp_get_gamma_srgb_wdr_lut(td_u16 *gamma_lut)
{
    td_u16 gamma033[OT_ISP_GAMMA_NODE_NUM] = {
        0, 21, 42, 64, 85, 106, 128, 149, 171, 192, 214, 235, 257, 279, 300, 322, 344, 366, 388, 409, 431, 453, 475,
        496, 518, 540, 562, 583, 605, 627, 648, 670, 691, 712, 734, 755, 776, 797, 818, 839, 860, 881, 902, 922, 943,
        963, 984, 1004, 1024, 1044, 1064, 1084, 1104, 1124, 1144, 1164, 1182, 1198, 1213, 1227, 1241, 1256, 1270, 1285,
        1299, 1313, 1328, 1342, 1356, 1370, 1384, 1398, 1412, 1426, 1440, 1453, 1467, 1481, 1494, 1508, 1521, 1535,
        1548, 1562, 1575, 1588, 1602, 1615, 1628, 1641, 1654, 1667, 1680, 1693, 1706, 1719, 1732, 1745, 1758, 1771,
        1784, 1797, 1810, 1823, 1836, 1849, 1862, 1875, 1888, 1901, 1914, 1926, 1939, 1952, 1965, 1978, 1991, 2004,
        2017, 2030, 2043, 2056, 2069, 2082, 2095, 2109, 2123, 2137, 2148, 2157, 2164, 2171, 2178, 2185, 2193, 2200,
        2208, 2216, 2223, 2231, 2239, 2247, 2255, 2262, 2270, 2278, 2286, 2293, 2301, 2309, 2316, 2324, 2332, 2340,
        2348, 2356, 2364, 2372, 2380, 2388, 2396, 2404, 2412, 2419, 2427, 2435, 2443, 2451, 2459, 2467, 2475, 2483,
        2491, 2499, 2507, 2515, 2523, 2531, 2539, 2546, 2554, 2562, 2570, 2578, 2586, 2594, 2602, 2609, 2617, 2625,
        2633, 2640, 2648, 2656, 2664, 2671, 2679, 2687, 2695, 2702, 2710, 2718, 2725, 2733, 2740, 2748, 2755, 2763,
        2770, 2778, 2785, 2793, 2800, 2807, 2815, 2822, 2829, 2836, 2844, 2851, 2858, 2865, 2872, 2879, 2886, 2893,
        2900, 2906, 2913, 2920, 2927, 2934, 2941, 2948, 2954, 2961, 2967, 2974, 2980, 2987, 2993, 2999, 3006, 3012,
        3018, 3024, 3030, 3036, 3042, 3048, 3054, 3059, 3065, 3071, 3077, 3082, 3088, 3094, 3099, 3105, 3110, 3115,
        3121, 3126, 3131, 3136, 3142, 3147, 3152, 3157, 3162, 3167, 3172, 3177, 3182, 3187, 3192, 3197, 3202, 3206,
        3211, 3216, 3220, 3225, 3229, 3234, 3238, 3243, 3247, 3252, 3256, 3261, 3265, 3269, 3274, 3278, 3282, 3286,
        3291, 3295, 3299, 3303, 3307, 3311, 3315, 3319, 3323, 3327, 3331, 3335, 3339, 3342, 3346, 3350, 3354, 3357,
        3361, 3365, 3368, 3371, 3375, 3379, 3383, 3386, 3390, 3394, 3397, 3401, 3404, 3407, 3411, 3414, 3417, 3420,
        3424, 3427, 3430, 3433, 3437, 3440, 3443, 3446, 3450, 3453, 3456, 3459, 3462, 3465, 3468, 3471, 3474, 3477,
        3480, 3483, 3486, 3489, 3492, 3495, 3498, 3501, 3504, 3507, 3510, 3512, 3515, 3518, 3521, 3523, 3526, 3529,
        3532, 3534, 3537, 3540, 3543, 3545, 3548, 3551, 3554, 3556, 3559, 3562, 3564, 3567, 3569, 3572, 3574, 3577,
        3579, 3582, 3584, 3587, 3589, 3591, 3594, 3596, 3598, 3600, 3603, 3605, 3607, 3609, 3612, 3614, 3616, 3618,
        3620, 3622, 3624, 3626, 3628, 3629, 3631, 3633, 3635, 3637, 3639, 3641, 3643, 3644, 3646, 3648, 3649, 3650,
        3652, 3654, 3656, 3657, 3659, 3661, 3662, 3664, 3665, 3667, 3668, 3670, 3671, 3672, 3674, 3675, 3676, 3677,
        3678, 3680, 3681, 3682, 3684, 3686, 3687, 3688, 3690, 3691, 3692, 3693, 3694, 3695, 3696, 3697, 3698, 3700,
        3701, 3702, 3704, 3705, 3706, 3707, 3708, 3709, 3710, 3711, 3713, 3714, 3715, 3716, 3717, 3718, 3719, 3720,
        3721, 3722, 3723, 3724, 3726, 3727, 3728, 3729, 3730, 3731, 3732, 3733, 3734, 3735, 3736, 3737, 3739, 3740,
        3741, 3742, 3743, 3744, 3745, 3746, 3748, 3749, 3750, 3751, 3752, 3753, 3754, 3755, 3756, 3758, 3759, 3760,
        3762, 3763, 3764, 3765, 3767, 3768, 3769, 3770, 3771, 3772, 3773, 3774, 3776, 3777, 3778, 3779, 3780, 3781,
        3782, 3783, 3785, 3786, 3787, 3788, 3789, 3790, 3791, 3792, 3793, 3794, 3795, 3796, 3797, 3798, 3799, 3800,
        3801, 3802, 3803, 3804, 3805, 3806, 3807, 3808, 3809, 3810, 3811, 3812, 3813, 3814, 3815, 3816, 3817, 3818,
        3819, 3820, 3821, 3822, 3823, 3824, 3825, 3825, 3826, 3827, 3828, 3829, 3830, 3831, 3832, 3833, 3834, 3835,
        3836, 3836, 3837, 3838, 3839, 3840, 3841, 3842, 3843, 3843, 3844, 3845, 3845, 3846, 3847, 3848, 3849, 3850,
        3851, 3852, 3853, 3853, 3854, 3855, 3855, 3856, 3857, 3858, 3859, 3860, 3861, 3862, 3863, 3863, 3864, 3865,
        3866, 3866, 3867, 3868, 3868, 3869, 3870, 3871, 3872, 3873, 3874, 3875, 3876, 3876, 3877, 3878, 3879, 3879,
        3880, 3881, 3882, 3882, 3883, 3884, 3885, 3885, 3886, 3887, 3888, 3888, 3889, 3890, 3891, 3891, 3892, 3893,
        3894, 3894, 3895, 3896, 3897, 3897, 3898, 3899, 3899, 3899, 3900, 3901, 3901, 3902, 3903, 3904, 3905, 3905,
        3906, 3907, 3907, 3907, 3908, 3909, 3910, 3910, 3911, 3912, 3912, 3912, 3913, 3914, 3915, 3915, 3916, 3917,
        3917, 3917, 3918, 3919, 3920, 3920, 3921, 3922, 3922, 3923, 3923, 3923, 3924, 3924, 3925, 3926, 3927, 3927,
        3928, 3929, 3929, 3930, 3930, 3930, 3931, 3931, 3932, 3933, 3934, 3934, 3935, 3936, 3936, 3937, 3937, 3937,
        3938, 3938, 3939, 3940, 3941, 3941, 3942, 3943, 3943, 3944, 3944, 3944, 3945, 3945, 3946, 3947, 3948, 3948,
        3949, 3950, 3950, 3950, 3951, 3952, 3953, 3953, 3954, 3955, 3955, 3955, 3956, 3957, 3958, 3958, 3959, 3960,
        3960, 3960, 3961, 3962, 3963, 3963, 3964, 3965, 3965, 3965, 3966, 3967, 3968, 3968, 3969, 3970, 3970, 3970,
        3971, 3972, 3973, 3973, 3974, 3975, 3975, 3975, 3976, 3977, 3977, 3978, 3979, 3980, 3981, 3981, 3982, 3983,
        3983, 3983, 3984, 3985, 3985, 3986, 3987, 3988, 3989, 3989, 3990, 3991, 3991, 3991, 3992, 3993, 3994, 3994,
        3995, 3996, 3996, 3996, 3997, 3998, 3998, 3999, 4000, 4001, 4002, 4002, 4003, 4004, 4004, 4004, 4005, 4006,
        4007, 4007, 4008, 4009, 4009, 4009, 4010, 4011, 4012, 4012, 4013, 4014, 4014, 4014, 4015, 4016, 4017, 4017,
        4018, 4019, 4019, 4019, 4020, 4021, 4022, 4022, 4023, 4024, 4024, 4024, 4025, 4026, 4027, 4027, 4028, 4029,
        4029, 4030, 4030, 4030, 4031, 4031, 4032, 4033, 4034, 4034, 4035, 4036, 4036, 4037, 4037, 4038, 4038, 4039,
        4039, 4040, 4040, 4041, 4041, 4042, 4042, 4043, 4043, 4044, 4044, 4045, 4045, 4046, 4046, 4047, 4047, 4048,
        4048, 4049, 4049, 4050, 4050, 4051, 4051, 4052, 4052, 4053, 4053, 4054, 4054, 4055, 4055, 4055, 4056, 4056,
        4056, 4056, 4057, 4057, 4058, 4059, 4059, 4060, 4060, 4060, 4061, 4061, 4061, 4061, 4062, 4062, 4063, 4064,
        4064, 4065, 4065, 4065, 4066, 4066, 4066, 4066, 4067, 4067, 4068, 4069, 4069, 4070, 4070, 4070, 4071, 4071,
        4071, 4071, 4072, 4073, 4073, 4073, 4074, 4074, 4074, 4074, 4075, 4076, 4076, 4076, 4077, 4077, 4077, 4077,
        4078, 4078, 4079, 4080, 4080, 4081, 4081, 4081, 4082, 4082, 4082, 4082, 4083, 4084, 4084, 4084, 4085, 4085,
        4085, 4085, 4086, 4087, 4087, 4087, 4088, 4088, 4088, 4088, 4089, 4089, 4090, 4091, 4091, 4092, 4092, 4092,
        4093, 4093, 4093, 4093, 4094, 4094, 4095
    };
    td_s32 ret;
    ret = memcpy_s(gamma_lut, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16),
                   gamma033, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16));
    isp_check_eok_void(ret);

    return;
}

static td_void isp_get_gamma_lut(ot_vi_pipe vi_pipe, td_u16 *gamma_lut, const ot_isp_gamma_attr *gamma_attr)
{
    td_u32 wdr_mode;
    td_s32 ret;
    wdr_mode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);

    if (gamma_attr->curve_type == OT_ISP_GAMMA_CURVE_DEFAULT) {
        if (wdr_mode == 0) {
            isp_get_gamma_def_linear_lut(gamma_lut);
        } else {
            isp_get_gamma_def_wdr_lut(gamma_lut);
        }
        return;
    }

    if (gamma_attr->curve_type == OT_ISP_GAMMA_CURVE_SRGB) {
        if (wdr_mode == 0) {
            isp_get_gamma_srgb_linear_lut(gamma_lut);
        } else {
            isp_get_gamma_srgb_wdr_lut(gamma_lut);
        }
        return;
    }

    if (gamma_attr->curve_type == OT_ISP_GAMMA_CURVE_USER_DEFINE) {
        ret = memcpy_s(gamma_lut, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16),
                       gamma_attr->table, OT_ISP_GAMMA_NODE_NUM * sizeof(td_u16));
        if (ret != EOK) {
            return;
        }
    }
    return;
}

td_s32 ot_mpi_isp_set_gamma_attr(ot_vi_pipe vi_pipe, const ot_isp_gamma_attr *gamma_attr)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 i;
    td_u16 *gamma_lut = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(gamma_attr);
    isp_check_bool_return(gamma_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_gamma_init_return(vi_pipe);

    if (gamma_attr->curve_type >= OT_ISP_GAMMA_CURVE_BUTT) {
        isp_err_trace("Invalid  gamma curve type %d!\n", gamma_attr->curve_type);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (gamma_attr->curve_type == OT_ISP_GAMMA_CURVE_HDR) {
        isp_err_trace("Not support gamma curve type %d!\n", gamma_attr->curve_type);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    gamma_lut = (td_u16 *)isp_malloc(sizeof(td_u16) * OT_ISP_GAMMA_NODE_NUM);
    if (gamma_lut == TD_NULL) {
        isp_err_trace("malloc gamma table memory failure!\n");
        return OT_ERR_ISP_NOMEM;
    }

    isp_get_gamma_lut(vi_pipe, gamma_lut, gamma_attr);

    for (i = 0; i < OT_ISP_GAMMA_NODE_NUM; i++) {
        if (gamma_lut[i] > 0xFFF) {
            isp_err_trace("Invalid gamma table[%u] %u!\n", i, gamma_lut[i]);
            ret = OT_ERR_ISP_ILLEGAL_PARAM;
            goto exit_set_gamma_attr;
        }

        ot_ext_system_gamma_lut_write(vi_pipe, i, gamma_lut[i]);
    }

    ot_ext_system_gamma_en_write(vi_pipe, gamma_attr->enable);
    ot_ext_system_gamma_curve_type_write(vi_pipe, gamma_attr->curve_type);
    ot_ext_system_gamma_lut_update_write(vi_pipe, TD_TRUE);

exit_set_gamma_attr:
    isp_free(gamma_lut);
    return ret;
}

td_s32 ot_mpi_isp_get_gamma_attr(ot_vi_pipe vi_pipe, ot_isp_gamma_attr *gamma_attr)
{
    td_u32 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(gamma_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_gamma_init_return(vi_pipe);

    gamma_attr->enable = ot_ext_system_gamma_en_read(vi_pipe);
    gamma_attr->curve_type = ot_ext_system_gamma_curve_type_read(vi_pipe);

    for (i = 0; i < OT_ISP_GAMMA_NODE_NUM; i++) {
        gamma_attr->table[i] = ot_ext_system_gamma_lut_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_pregamma_attr(ot_vi_pipe vi_pipe, const ot_isp_pregamma_attr *pregamma_attr)
{
#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    td_s32 ret;
    td_u32 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pregamma_attr);
    isp_check_bool_return(pregamma_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_pregamma_init_return(vi_pipe);

    ret = isp_pregamma_attr_check("mpi", pregamma_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < OT_ISP_PREGAMMA_NODE_NUM; i++) {
        ot_ext_system_pregamma_lut_write(vi_pipe, i, pregamma_attr->table[i]);
    }

    ot_ext_system_pregamma_en_write(vi_pipe, pregamma_attr->enable);
    ot_ext_system_pregamma_lut_update_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_pregamma_attr(ot_vi_pipe vi_pipe, ot_isp_pregamma_attr *pregamma_attr)
{
#ifdef CONFIG_OT_ISP_PREGAMMA_SUPPORT
    td_u32 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pregamma_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_pregamma_init_return(vi_pipe);

    pregamma_attr->enable = ot_ext_system_pregamma_en_read(vi_pipe);

    for (i = 0; i < OT_ISP_PREGAMMA_NODE_NUM; i++) {
        pregamma_attr->table[i] = ot_ext_system_pregamma_lut_read(vi_pipe, i);
    }

    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

static td_s32 isp_csc_attr_check(const ot_isp_csc_attr *csc_attr)
{
    td_u8 i;

    if (csc_attr->luma > 100) { /* range:[0, 100] */
        isp_err_trace("Invalid csc luma!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (csc_attr->contr > 100) { /* range:[0, 100] */
        isp_err_trace("Invalid csc contr!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (csc_attr->satu > 100) { /* range:[0, 100] */
        isp_err_trace("Invalid csc satu!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (csc_attr->hue > 100) { /* range:[0, 100] */
        isp_err_trace("Invalid csc hue!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (csc_attr->color_gamut >= OT_COLOR_GAMUT_BUTT) {
        isp_err_trace("Invalid color gamut!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (csc_attr->color_gamut == OT_COLOR_GAMUT_BT2020) {
        isp_err_trace("BT.2020 gamut is not support\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        if ((csc_attr->csc_magtrx.csc_in_dc[i] > 1023) ||   /* 1023: max value */
            (csc_attr->csc_magtrx.csc_in_dc[i] < -1024)) {  /* -1024:min */
            isp_err_trace("Invalid csc_in_dc[%u]!\n", i);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if ((csc_attr->csc_magtrx.csc_out_dc[i] > 1023) ||   /* 1023: max value */
            (csc_attr->csc_magtrx.csc_out_dc[i] < -1024)) {  /* -1024:min */
            isp_err_trace("Invalid csc_out_dc[%u]!\n", i);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
        if ((csc_attr->csc_magtrx.csc_coef[i] > 4095) ||    /* 4095: max value */
            (csc_attr->csc_magtrx.csc_coef[i] < -4096)) {   /* -4096:min */
            isp_err_trace("Invalid csc_coef[%u]!\n", i);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_csc_attr(ot_vi_pipe vi_pipe, const ot_isp_csc_attr *csc_attr)
{
    td_u8 i;
    td_s32 ret;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(csc_attr);
    isp_check_bool_return(csc_attr->enable);
    isp_check_bool_return(csc_attr->limited_range_en);
    isp_check_bool_return(csc_attr->ct_mode_en);
    isp_check_bool_return(csc_attr->ext_csc_en);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = isp_csc_attr_check(csc_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_csc_enable_write(vi_pipe, csc_attr->enable);
    ot_ext_system_csc_gamuttype_write(vi_pipe, csc_attr->color_gamut);
    ot_ext_system_csc_luma_write(vi_pipe, csc_attr->luma);
    ot_ext_system_csc_contrast_write(vi_pipe, csc_attr->contr);
    ot_ext_system_csc_hue_write(vi_pipe, csc_attr->hue);
    ot_ext_system_csc_sat_write(vi_pipe, csc_attr->satu);
    ot_ext_system_csc_limitrange_en_write(vi_pipe, csc_attr->limited_range_en);
    ot_ext_system_csc_ext_en_write(vi_pipe, csc_attr->ext_csc_en);
    ot_ext_system_csc_ctmode_en_write(vi_pipe, csc_attr->ct_mode_en);

    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        ot_ext_system_csc_dcin_write(vi_pipe, i,  csc_attr->csc_magtrx.csc_in_dc[i]);
        ot_ext_system_csc_dcout_write(vi_pipe, i, csc_attr->csc_magtrx.csc_out_dc[i]);
    }

    for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
        ot_ext_system_csc_coef_write(vi_pipe, i, csc_attr->csc_magtrx.csc_coef[i]);
    }

    ot_ext_system_csc_attr_update_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_csc_attr(ot_vi_pipe vi_pipe, ot_isp_csc_attr *csc_attr)
{
    td_u8 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(csc_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    csc_attr->enable        = ot_ext_system_csc_enable_read(vi_pipe);
    csc_attr->color_gamut   = ot_ext_system_csc_gamuttype_read(vi_pipe);
    csc_attr->hue           = ot_ext_system_csc_hue_read(vi_pipe);
    csc_attr->luma          = ot_ext_system_csc_luma_read(vi_pipe);
    csc_attr->contr         = ot_ext_system_csc_contrast_read(vi_pipe);
    csc_attr->satu          = ot_ext_system_csc_sat_read(vi_pipe);
    csc_attr->limited_range_en = ot_ext_system_csc_limitrange_en_read(vi_pipe);
    csc_attr->ext_csc_en       = ot_ext_system_csc_ext_en_read(vi_pipe);
    csc_attr->ct_mode_en       = ot_ext_system_csc_ctmode_en_read(vi_pipe);

    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        csc_attr->csc_magtrx.csc_in_dc[i]  = (td_s16)ot_ext_system_csc_dcin_read(vi_pipe, i);
        csc_attr->csc_magtrx.csc_out_dc[i] = (td_s16)ot_ext_system_csc_dcout_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
        csc_attr->csc_magtrx.csc_coef[i] = (td_s16)ot_ext_system_csc_coef_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

static td_s32 ot_mpi_isp_set_dp_calibrate_check_para(const ot_isp_dp_static_calibrate *dp_calibrate,
                                                     td_u16 static_count_max)
{
    if (dp_calibrate->count_max > static_count_max) {
        isp_err_trace("Invalid count_max!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (dp_calibrate->count_min >= dp_calibrate->count_max) {
        isp_err_trace("Invalid count_min!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (dp_calibrate->time_limit > 0x640) {
        isp_err_trace("Invalid time_limit!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (dp_calibrate->static_dp_type >= OT_ISP_STATIC_DP_BUTT) {
        isp_err_trace("Invalid static_dp_type!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (dp_calibrate->start_thresh == 0) {
        isp_err_trace("Invalid start_thresh!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_dp_calibrate(ot_vi_pipe vi_pipe, const ot_isp_dp_static_calibrate *dp_calibrate)
{
#ifdef CONFIG_OT_ISP_DPC_STATIC_TABLE_SUPPORT
    td_u16 static_count_max = OT_ISP_STATIC_DP_COUNT_NORMAL;
    vi_stitch_attr stitch_attr;

    td_s32 ret;
    isp_working_mode work_mode;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_calibrate);
    isp_check_bool_return(dp_calibrate->enable_detect);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_STITCH_ATTR, &stitch_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get stitch attr failed\n", vi_pipe);
        return ret;
    }

    if ((stitch_attr.stitch_enable == TD_TRUE) && (dp_calibrate->enable_detect == TD_TRUE)) {
        isp_err_trace("Not Support dpc calibration in stitch mode\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    if ((is_offline_mode(work_mode.running_mode) || is_striping_mode(work_mode.running_mode)) &&
        (dp_calibrate->enable_detect == TD_TRUE) && (OT_ISP_SUPPORT_OFFLINE_DPC_CALIBRATION) != 1) {
        isp_err_trace("only support dpc calibration under online mode!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    static_count_max = MIN2(static_count_max * work_mode.block_num, OT_ISP_STATIC_DP_COUNT_MAX);

    ret = ot_mpi_isp_set_dp_calibrate_check_para(dp_calibrate, static_count_max);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_dpc_static_calib_enable_write(vi_pipe, dp_calibrate->enable_detect);
    ot_ext_system_dpc_static_defect_type_write(vi_pipe, dp_calibrate->static_dp_type);
    ot_ext_system_dpc_start_thresh_write(vi_pipe, dp_calibrate->start_thresh);
    ot_ext_system_dpc_count_max_write(vi_pipe, dp_calibrate->count_max);
    ot_ext_system_dpc_count_min_write(vi_pipe, dp_calibrate->count_min);
    ot_ext_system_dpc_trigger_time_write(vi_pipe, dp_calibrate->time_limit);
    ot_ext_system_dpc_trigger_status_write(vi_pipe, OT_ISP_STATE_INIT);

    return TD_SUCCESS;
#else
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_dp_calibrate(ot_vi_pipe vi_pipe, ot_isp_dp_static_calibrate *dp_calibrate)
{
#ifdef CONFIG_OT_ISP_DPC_STATIC_TABLE_SUPPORT
    td_u16 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_calibrate);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    dp_calibrate->enable_detect   = ot_ext_system_dpc_static_calib_enable_read(vi_pipe);

    dp_calibrate->static_dp_type  = ot_ext_system_dpc_static_defect_type_read(vi_pipe);
    dp_calibrate->start_thresh   = ot_ext_system_dpc_start_thresh_read(vi_pipe);
    dp_calibrate->count_max     = ot_ext_system_dpc_count_max_read(vi_pipe);
    dp_calibrate->count_min     = ot_ext_system_dpc_count_min_read(vi_pipe);
    dp_calibrate->time_limit    = ot_ext_system_dpc_trigger_time_read(vi_pipe);

    dp_calibrate->status        = ot_ext_system_dpc_trigger_status_read(vi_pipe);
    dp_calibrate->finish_thresh  = ot_ext_system_dpc_finish_thresh_read(vi_pipe);
    dp_calibrate->count        = ot_ext_system_dpc_bpt_calib_number_read(vi_pipe);

    if (dp_calibrate->status == OT_ISP_STATE_INIT) {
        for (i = 0; i < OT_ISP_STATIC_DP_COUNT_MAX; i++) {
            dp_calibrate->table[i] = 0;
        }
    } else {
        for (i = 0; i < OT_ISP_STATIC_DP_COUNT_MAX; i++) {
            if (i < dp_calibrate->count) {
                dp_calibrate->table[i] = ot_ext_system_dpc_calib_bpt_read(vi_pipe, i);
            } else {
                dp_calibrate->table[i] = 0;
            }
        }
    }
    return TD_SUCCESS;
#else
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

#ifdef CONFIG_OT_ISP_DPC_STATIC_TABLE_SUPPORT
static td_s32 isp_dpc_merge_defect_table(td_u16 cnt_max, td_u32 *defect_pixel_table,
                                         const ot_isp_dp_static_attr *dp_static_attr, td_u16 *dpc_cnt)
{
    td_u16 i = 0;
    td_u16 j = 0;
    td_u16 m = 0;

    while ((i < dp_static_attr->bright_count) && (j < dp_static_attr->dark_count)) {
        if (m >= cnt_max) {
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
        if (dp_static_attr->bright_table[i] > dp_static_attr->dark_table[j]) {
            defect_pixel_table[m++] = dp_static_attr->dark_table[j++];
        } else if (dp_static_attr->bright_table[i] < dp_static_attr->dark_table[j]) {
            defect_pixel_table[m++] = dp_static_attr->bright_table[i++];
        } else {
            defect_pixel_table[m++] = dp_static_attr->bright_table[i];
            i++;
            j++;
        }
    }

    if (i >= dp_static_attr->bright_count) {
        while (j < dp_static_attr->dark_count) {
            if (m >= cnt_max) {
                return OT_ERR_ISP_ILLEGAL_PARAM;
            }
            defect_pixel_table[m++] = dp_static_attr->dark_table[j++];
        }
    }
    if (j >= dp_static_attr->dark_count) {
        while (i < dp_static_attr->bright_count) {
            if (m >= cnt_max) {
                return OT_ERR_ISP_ILLEGAL_PARAM;
            }
            defect_pixel_table[m++] = dp_static_attr->bright_table[i++];
        }
    }

    *dpc_cnt = m;

    return TD_SUCCESS;
}

static td_s32 isp_set_dp_static_defect_table(ot_vi_pipe vi_pipe, td_u16 cnt_max,
                                             const ot_isp_dp_static_attr *dp_static_attr)
{
    td_u16 i, count_in;
    td_u32 *defect_pixel_table = TD_NULL;
    td_s32 ret;

    defect_pixel_table = (td_u32 *)isp_malloc(sizeof(td_u32) * cnt_max);
    if (defect_pixel_table == TD_NULL) {
        isp_err_trace("malloc defect_pixel_table memory failure!\n");
        return OT_ERR_ISP_NOMEM;
    }

    ret = isp_dpc_merge_defect_table(cnt_max, defect_pixel_table, dp_static_attr, &count_in);
    if (ret != TD_SUCCESS) {
        isp_err_trace("the size of merging DP table(brighttable and darktable) is larger than %u!\n", cnt_max);
        isp_free(defect_pixel_table);
        return ret;
    }

    ot_ext_system_dpc_bpt_cor_number_write(vi_pipe, count_in);
    for (i = 0; i < OT_ISP_STATIC_DP_COUNT_MAX; i++) {
        if (i < count_in) {
            ot_ext_system_dpc_cor_bpt_write(vi_pipe, i, defect_pixel_table[i]);
        } else {
            ot_ext_system_dpc_cor_bpt_write(vi_pipe, i, 0);
        }
    }

    isp_free(defect_pixel_table);
    return TD_SUCCESS;
}

static td_s32 isp_set_dp_static_attr(ot_vi_pipe vi_pipe, const ot_isp_dp_static_attr *dp_static_attr)
{
    td_u16 i;
    td_s32 ret;
    td_u16 static_count_max = OT_ISP_STATIC_DP_COUNT_NORMAL;
    isp_working_mode isp_work_mode;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_static_attr);
    isp_check_bool_return(dp_static_attr->enable);
    isp_check_bool_return(dp_static_attr->show);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode)) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    if (isp_work_mode.running_mode == ISP_MODE_RUNNING_SIDEBYSIDE) {
        static_count_max = static_count_max * OT_ISP_SBS_BLOCK_NUM;
    } else if (isp_work_mode.running_mode == ISP_MODE_RUNNING_STRIPING) {
        if (OT_ISP_SUPPORT_OFFLINE_DPC_CALIBRATION == 1) {
            static_count_max = static_count_max * isp_work_mode.block_num;
        } else {
            static_count_max = static_count_max * OT_ISP_SBS_BLOCK_NUM;
        }
    }

    if (dp_static_attr->bright_count > static_count_max) {
        isp_err_trace("Invalid bright_count!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (dp_static_attr->dark_count > static_count_max) {
        isp_err_trace("Invalid dark_count!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    for (i = 0; i < static_count_max; i++) {
        if (dp_static_attr->bright_table[i] > 0x1FFF1FFF || dp_static_attr->dark_table[i] > 0x1FFF1FFF) {
            isp_err_trace("dark_table and bright_table should be less than 0x1FFF1FFF!\n");
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    /* merging dark table and bright table */
    ret = isp_set_dp_static_defect_table(vi_pipe, static_count_max, dp_static_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_dpc_static_cor_enable_write(vi_pipe, dp_static_attr->enable);
    ot_ext_system_dpc_static_dp_show_write(vi_pipe, dp_static_attr->show);
    ot_ext_system_dpc_static_attr_update_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}
#endif

td_s32 ot_mpi_isp_set_dp_static_attr(ot_vi_pipe vi_pipe, const ot_isp_dp_static_attr *dp_static_attr)
{
#ifdef CONFIG_OT_ISP_DPC_STATIC_TABLE_SUPPORT
    return isp_set_dp_static_attr(vi_pipe, dp_static_attr);
#else
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_dp_static_attr(ot_vi_pipe vi_pipe, ot_isp_dp_static_attr *dp_static_attr)
{
#ifdef CONFIG_OT_ISP_DPC_STATIC_TABLE_SUPPORT
    td_u32 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_static_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    dp_static_attr->enable        = ot_ext_system_dpc_static_cor_enable_read(vi_pipe);
    dp_static_attr->show          = ot_ext_system_dpc_static_dp_show_read(vi_pipe);
    dp_static_attr->bright_count = ot_ext_system_dpc_bpt_cor_number_read(vi_pipe);
    dp_static_attr->dark_count   = 0;

    for (i = 0; i < OT_ISP_STATIC_DP_COUNT_MAX; i++) {
        if (i < dp_static_attr->bright_count) {
            dp_static_attr->bright_table[i] = ot_ext_system_dpc_cor_bpt_read(vi_pipe, i);
        } else {
            dp_static_attr->bright_table[i] = 0;
        }
        dp_static_attr->dark_table[i] = 0;
    }
    return TD_SUCCESS;
#else
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_set_dp_dynamic_attr(ot_vi_pipe vi_pipe, const ot_isp_dp_dynamic_attr *dp_dynamic_attr)
{
    td_u8 i, j;
    td_s32 ret;
    isp_check_pointer_return(dp_dynamic_attr);

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_dynamic_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    ret = isp_dp_dynamic_attr_check("mpi", dp_dynamic_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_dpc_dynamic_cor_enable_write(vi_pipe, dp_dynamic_attr->enable);

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        const ot_isp_dp_frame_dynamic_attr *frame_dynamic_attr = &(dp_dynamic_attr->frame_dynamic[i]);
        const ot_isp_dp_dynamic_auto_attr *auto_attr = &(frame_dynamic_attr->auto_attr);

        for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
            td_u8 index = i * OT_ISP_AUTO_ISO_NUM + j;
            ot_ext_system_dpc_dynamic_strength_table_write(vi_pipe, index, auto_attr->strength[j]);
            ot_ext_system_dpc_dynamic_blend_ratio_table_write(vi_pipe, index, auto_attr->blend_ratio[j]);
        }

        ot_ext_system_dpc_dynamic_manual_enable_write(vi_pipe, i, frame_dynamic_attr->op_type);
        ot_ext_system_dpc_dynamic_strength_write(vi_pipe, i, frame_dynamic_attr->manual_attr.strength);
        ot_ext_system_dpc_dynamic_blend_ratio_write(vi_pipe, i, frame_dynamic_attr->manual_attr.blend_ratio);

        ot_ext_system_dpc_suppress_twinkle_enable_write(vi_pipe, i, frame_dynamic_attr->sup_twinkle_en);
        ot_ext_system_dpc_suppress_twinkle_thr_write(vi_pipe, i, frame_dynamic_attr->soft_thr);
        ot_ext_system_dpc_suppress_twinkle_slope_write(vi_pipe, i, frame_dynamic_attr->soft_slope);
        ot_ext_system_dpc_bright_strength_write(vi_pipe, i, frame_dynamic_attr->bright_strength);
        ot_ext_system_dpc_dark_strength_write(vi_pipe, i, frame_dynamic_attr->dark_strength);
    }
    ot_ext_system_dpc_dynamic_attr_update_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_dp_dynamic_attr(ot_vi_pipe vi_pipe, ot_isp_dp_dynamic_attr *dp_dynamic_attr)
{
    td_u8 i, j;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dp_dynamic_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_dpc_init_return(vi_pipe);

    dp_dynamic_attr->enable = ot_ext_system_dpc_dynamic_cor_enable_read(vi_pipe);

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        ot_isp_dp_frame_dynamic_attr *frame_dynamic_attr = &(dp_dynamic_attr->frame_dynamic[i]);
        ot_isp_dp_dynamic_auto_attr *auto_attr = &(frame_dynamic_attr->auto_attr);

        frame_dynamic_attr->op_type      = (ot_op_mode)ot_ext_system_dpc_dynamic_manual_enable_read(vi_pipe, i);
        frame_dynamic_attr->sup_twinkle_en = ot_ext_system_dpc_suppress_twinkle_enable_read(vi_pipe, i);
        frame_dynamic_attr->soft_thr     = ot_ext_system_dpc_suppress_twinkle_thr_read(vi_pipe, i);
        frame_dynamic_attr->soft_slope   = ot_ext_system_dpc_suppress_twinkle_slope_read(vi_pipe, i);

        for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
            td_u8 index = i * OT_ISP_AUTO_ISO_NUM + j;
            auto_attr->strength[j]    = ot_ext_system_dpc_dynamic_strength_table_read(vi_pipe, index);
            auto_attr->blend_ratio[j] = ot_ext_system_dpc_dynamic_blend_ratio_table_read(vi_pipe, index);
        }

        frame_dynamic_attr->manual_attr.strength    = ot_ext_system_dpc_dynamic_strength_read(vi_pipe, i);
        frame_dynamic_attr->manual_attr.blend_ratio = ot_ext_system_dpc_dynamic_blend_ratio_read(vi_pipe, i);
        frame_dynamic_attr->bright_strength = ot_ext_system_dpc_bright_strength_read(vi_pipe, i);
        frame_dynamic_attr->dark_strength = ot_ext_system_dpc_dark_strength_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_lblc_attr(ot_vi_pipe vi_pipe, const ot_isp_lblc_attr *lblc_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(lblc_attr);
    isp_check_bool_return(lblc_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_lblc_init_return(vi_pipe);

    ot_ext_system_isp_lblc_enable_write(vi_pipe, lblc_attr->enable);

    if (lblc_attr->strength > OT_ISP_LBLC_STRENGTH_MAX) {
        isp_err_trace("Invalid lblc strength!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    ot_ext_system_isp_lblc_strength_write(vi_pipe, lblc_attr->strength);

    ot_ext_system_isp_lblc_attr_updata_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_lblc_attr(ot_vi_pipe vi_pipe, ot_isp_lblc_attr *lblc_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(lblc_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_lblc_init_return(vi_pipe);

    lblc_attr->enable   = ot_ext_system_isp_lblc_enable_read(vi_pipe);
    lblc_attr->strength = ot_ext_system_isp_lblc_strength_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_lblc_lut_attr(ot_vi_pipe vi_pipe, const ot_isp_lblc_lut_attr *lblc_lut_attr)
{
    td_u16 i;
    td_s32 ret;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(lblc_lut_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_lblc_init_return(vi_pipe);

    ret = isp_lblc_lut_attr_check("mpi", vi_pipe, lblc_lut_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
        ot_ext_system_isp_lblc_mesh_blc_r_write(vi_pipe, i, lblc_lut_attr->mesh_blc_r[i]);
        ot_ext_system_isp_lblc_mesh_blc_gr_write(vi_pipe, i, lblc_lut_attr->mesh_blc_gr[i]);
        ot_ext_system_isp_lblc_mesh_blc_gb_write(vi_pipe, i, lblc_lut_attr->mesh_blc_gb[i]);
        ot_ext_system_isp_lblc_mesh_blc_b_write(vi_pipe, i, lblc_lut_attr->mesh_blc_b[i]);
    }
    ot_ext_system_isp_lblc_offset_r_write(vi_pipe, lblc_lut_attr->offset_r);
    ot_ext_system_isp_lblc_offset_gr_write(vi_pipe, lblc_lut_attr->offset_gr);
    ot_ext_system_isp_lblc_offset_gb_write(vi_pipe, lblc_lut_attr->offset_gb);
    ot_ext_system_isp_lblc_offset_b_write(vi_pipe, lblc_lut_attr->offset_b);

    ot_ext_system_isp_lblc_lut_attr_updata_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_lblc_lut_attr(ot_vi_pipe vi_pipe, ot_isp_lblc_lut_attr *lblc_lut_attr)
{
    td_u16 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(lblc_lut_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_lblc_init_return(vi_pipe);

    for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
        lblc_lut_attr->mesh_blc_r[i] = ot_ext_system_isp_lblc_mesh_blc_r_read(vi_pipe, i);
        lblc_lut_attr->mesh_blc_gr[i] = ot_ext_system_isp_lblc_mesh_blc_gr_read(vi_pipe, i);
        lblc_lut_attr->mesh_blc_gb[i] = ot_ext_system_isp_lblc_mesh_blc_gb_read(vi_pipe, i);
        lblc_lut_attr->mesh_blc_b[i] = ot_ext_system_isp_lblc_mesh_blc_b_read(vi_pipe, i);
    }
    lblc_lut_attr->offset_r = ot_ext_system_isp_lblc_offset_r_read(vi_pipe);
    lblc_lut_attr->offset_gr = ot_ext_system_isp_lblc_offset_gr_read(vi_pipe);
    lblc_lut_attr->offset_gb = ot_ext_system_isp_lblc_offset_gb_read(vi_pipe);
    lblc_lut_attr->offset_b = ot_ext_system_isp_lblc_offset_b_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_mesh_shading_attr(ot_vi_pipe vi_pipe, const ot_isp_shading_attr *shading_attr)
{
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_attr);
    isp_mutex_lock(vi_pipe);
    isp_check_open_goto(vi_pipe, ret, exit);
    isp_check_mem_init_goto(vi_pipe, ret, exit);
    isp_check_vreg_permission_goto(vi_pipe, ret, exit);
    isp_check_mesh_shading_init_goto(vi_pipe, ret, exit);
    ret = isp_mesh_shading_attr_check("mpi", vi_pipe, shading_attr);
    if (ret != TD_SUCCESS) {
        goto exit;
    }
    if (ot_ext_system_isp_mesh_shading_attr_inner_update_read(vi_pipe) == TD_TRUE) {
        isp_mutex_unlock(vi_pipe);
        isp_err_trace("[ISP%d]mesh shading attr is update by stitch vpss, can't be changed by mpi\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }
    isp_set_mesh_shading_attr(vi_pipe, shading_attr);
exit:
    isp_mutex_unlock(vi_pipe);
    return ret;
}

td_s32 ot_mpi_isp_get_mesh_shading_attr(ot_vi_pipe vi_pipe, ot_isp_shading_attr *shading_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_mesh_shading_init_return(vi_pipe);
    shading_attr->enable        = ot_ext_system_isp_mesh_shading_enable_read(vi_pipe);
    shading_attr->mesh_strength = ot_ext_system_isp_mesh_shading_mesh_strength_read(vi_pipe);
    shading_attr->blend_ratio   = ot_ext_system_isp_mesh_shading_blendratio_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_mesh_shading_gain_lut_attr(ot_vi_pipe vi_pipe, const ot_isp_shading_lut_attr *shading_lut_attr)
{
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_lut_attr);
    isp_mutex_lock(vi_pipe);
    isp_check_open_goto(vi_pipe, ret, exit);
    isp_check_mem_init_goto(vi_pipe, ret, exit);
    isp_check_vreg_permission_goto(vi_pipe, ret, exit);
    isp_check_mesh_shading_init_goto(vi_pipe, ret, exit);
    ret = isp_mesh_shading_gain_lut_attr_check("mpi", vi_pipe, shading_lut_attr);
    if (ret != TD_SUCCESS) {
        goto exit;
    }
    if (ot_ext_system_isp_mesh_shading_inner_update_read(vi_pipe) == TD_TRUE) {
        isp_mutex_unlock(vi_pipe);
        isp_err_trace("[ISP%d]mesh shading lut attr is update by stitch vpss, can't be changed by mpi\n", vi_pipe);
        return OT_ERR_ISP_NOT_SUPPORT;
    }
    isp_set_mesh_shading_gain_lut_attr(vi_pipe, shading_lut_attr);
exit:
    isp_mutex_unlock(vi_pipe);
    return ret;
}

td_s32 ot_mpi_isp_get_mesh_shading_gain_lut_attr(ot_vi_pipe vi_pipe, ot_isp_shading_lut_attr *shading_lut_attr)
{
    td_u16 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shading_lut_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_mesh_shading_init_return(vi_pipe);
    shading_lut_attr->mesh_scale = ot_ext_system_isp_mesh_shading_mesh_scale_read(vi_pipe);

    for (i = 0; i < OT_ISP_LSC_GRID_POINTS; i++) {
        shading_lut_attr->lsc_gain_lut[0].r_gain[i]  = ot_ext_system_isp_mesh_shading_r_gain0_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[0].gr_gain[i] = ot_ext_system_isp_mesh_shading_gr_gain0_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[0].gb_gain[i] = ot_ext_system_isp_mesh_shading_gb_gain0_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[0].b_gain[i]  = ot_ext_system_isp_mesh_shading_b_gain0_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[1].r_gain[i]  = ot_ext_system_isp_mesh_shading_r_gain1_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[1].gr_gain[i] = ot_ext_system_isp_mesh_shading_gr_gain1_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[1].gb_gain[i] = ot_ext_system_isp_mesh_shading_gb_gain1_read(vi_pipe, i);
        shading_lut_attr->lsc_gain_lut[1].b_gain[i]  = ot_ext_system_isp_mesh_shading_b_gain1_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_MLSC_X_HALF_GRID_NUM; i++) {
        shading_lut_attr->x_grid_width[i] = ot_ext_system_isp_mesh_shading_xgrid_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_MLSC_Y_HALF_GRID_NUM; i++) {
        shading_lut_attr->y_grid_width[i] = ot_ext_system_isp_mesh_shading_ygrid_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_auto_color_shading_attr(ot_vi_pipe vi_pipe, const ot_isp_acs_attr *acs_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(acs_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_auto_color_shading_init_return(vi_pipe);

    isp_check_bool_return(acs_attr->enable);
    isp_check_bool_return(acs_attr->lock_enable);
    ot_ext_system_isp_acs_enable_write(vi_pipe, acs_attr->enable);
    ot_ext_system_isp_acs_lock_enable_write(vi_pipe, acs_attr->lock_enable);

    if (acs_attr->run_interval == 0 || acs_attr->run_interval > 0xFF) { /* [1, 255] */
        isp_err_trace("Invalid run_interval:%u!\n", acs_attr->run_interval);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    ot_ext_system_isp_acs_run_interval_write(vi_pipe, acs_attr->run_interval);

    if (acs_attr->y_strength > 0x100) {
        isp_err_trace("Invalid y_strength:%u!\n", acs_attr->y_strength);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    ot_ext_system_isp_acs_y_strength_write(vi_pipe, acs_attr->y_strength);

    ot_ext_system_isp_acs_attr_updata_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_auto_color_shading_attr(ot_vi_pipe vi_pipe, ot_isp_acs_attr *acs_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(acs_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_auto_color_shading_init_return(vi_pipe);

    acs_attr->enable       = ot_ext_system_isp_acs_enable_read(vi_pipe);
    acs_attr->y_strength   = ot_ext_system_isp_acs_y_strength_read(vi_pipe);
    acs_attr->run_interval = ot_ext_system_isp_acs_run_interval_read(vi_pipe);
    acs_attr->lock_enable  = ot_ext_system_isp_acs_lock_enable_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_lightbox_gain(ot_vi_pipe vi_pipe, ot_isp_awb_calibration_gain *awb_calibration_gain)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(awb_calibration_gain);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    return isp_get_lightbox_gain(vi_pipe, awb_calibration_gain);
}

static td_s32 isp_mesh_shading_calibration_cfg_res_check(ot_isp_mlsc_calibration_cfg *cali_cfg)
{
    if (cali_cfg->img_width % OT_ISP_ALIGN_WIDTH != 0) {
        isp_err_trace("Err img_width:%u, width must be a muliple of 4!\n", cali_cfg->img_width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->img_height % OT_ISP_ALIGN_HEIGHT != 0) {
        isp_err_trace("Err img_height:%u, height must be a muliple of 4!\n", cali_cfg->img_height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->dst_img_width % OT_ISP_ALIGN_WIDTH != 0) {
        isp_err_trace("Err dst_img_width:%u, width must be a muliple of 4!\n", cali_cfg->dst_img_width);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->dst_img_height % OT_ISP_ALIGN_HEIGHT != 0) {
        isp_err_trace("Err dst_img_height:%u, height must be a muliple of 4!\n", cali_cfg->dst_img_height);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((cali_cfg->offset_x % 2) != 0) { /* offset must be a muliple of 2 */
        isp_err_trace("Err offset_x, bayer pattern of the cropped image does not match to the input image!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((cali_cfg->offset_y % 2) != 0) { /* offset must be a muliple of 2 */
        isp_err_trace("Err offset_y, bayer pattern of the cropped image does not match to the input image!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->dst_img_width + cali_cfg->offset_x > cali_cfg->img_width) {
        isp_err_trace("dst_img_width + offset_x should not be lager than img_width\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->dst_img_height + cali_cfg->offset_y > cali_cfg->img_height) {
        isp_err_trace("dst_img_height + offset_y should not be lager than img_height\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 isp_mesh_shading_calibration_cfg_check(ot_isp_mlsc_calibration_cfg *cali_cfg)
{
    td_s32 ret;

    ret = isp_mesh_shading_calibration_cfg_res_check(cali_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (cali_cfg->mesh_scale > 0x7) {
        isp_err_trace("Err mesh_scale:%u, mesh_scale must be smaller than 8!\n", cali_cfg->mesh_scale);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((cali_cfg->raw_bit >= OT_ISP_BAYER_RAW_BIT_BUTT) || (cali_cfg->raw_bit < OT_ISP_BAYER_RAW_BIT_8BIT) ||
        ((cali_cfg->raw_bit % 2) == 1)) {    /* raw_bit must be a muliple of 2 */
        isp_err_trace("Err raw_bit type:%d!\n", cali_cfg->raw_bit);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->bayer >= OT_ISP_BAYER_BUTT) {
        isp_err_trace("Err bayer type:%d!\n", cali_cfg->bayer);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->blc_offset_b > 0xFFF) {
        isp_err_trace("Err blc(chn B) offset:%u!\n", cali_cfg->blc_offset_b);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->blc_offset_gb > 0xFFF) {
        isp_err_trace("Err blc(chn Gb) offset:%u!\n", cali_cfg->blc_offset_gb);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->blc_offset_gr > 0xFFF) {
        isp_err_trace("Err blc(chn Gr) offset:%u!\n", cali_cfg->blc_offset_gr);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (cali_cfg->blc_offset_r > 0xFFF) {
        isp_err_trace("Err blc(chn R) offset:%u!\n", cali_cfg->blc_offset_r);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_mesh_shading_calibration(const ot_vi_pipe vi_pipe, const td_u16 *src_raw,
                                           ot_isp_mlsc_calibration_cfg *mlsc_cali_cfg,
                                           ot_isp_mesh_shading_table *mlsc_table)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(src_raw);
    isp_check_pointer_return(mlsc_cali_cfg);
    isp_check_pointer_return(mlsc_table);

    ret = isp_mesh_shading_calibration_cfg_check(mlsc_cali_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return isp_mesh_shading_calibration(src_raw, mlsc_cali_cfg, mlsc_table);
}

td_s32 ot_mpi_isp_set_cac_attr(ot_vi_pipe vi_pipe, const ot_isp_cac_attr *cac_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(cac_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_cac_init_return(vi_pipe);

    isp_check_bool_return(cac_attr->enable);
    ret = isp_cac_comm_attr_check("mpi", cac_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_cac_acac_attr_check("mpi", &cac_attr->acac_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_cac_lcac_attr_check("mpi", &cac_attr->lcac_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ot_ext_system_cac_enable_write(vi_pipe, cac_attr->enable);
    ot_ext_system_cac_op_type_write(vi_pipe, cac_attr->op_type);
    ot_ext_system_cac_det_mode_write(vi_pipe, cac_attr->detect_mode);
    ot_ext_system_cac_purple_upper_limit_write(vi_pipe, cac_attr->purple_upper_limit);
    ot_ext_system_cac_purple_lower_limit_write(vi_pipe, cac_attr->purple_lower_limit);

    isp_cac_acac_attr_write(vi_pipe, &cac_attr->acac_cfg);
    isp_cac_lcac_attr_write(vi_pipe, &cac_attr->lcac_cfg);

    ot_ext_system_cac_coef_update_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_cac_attr(ot_vi_pipe vi_pipe, ot_isp_cac_attr *cac_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(cac_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_cac_init_return(vi_pipe);

    cac_attr->enable = ot_ext_system_cac_enable_read(vi_pipe);
    cac_attr->op_type = ot_ext_system_cac_op_type_read(vi_pipe);
    cac_attr->detect_mode = ot_ext_system_cac_det_mode_read(vi_pipe);
    cac_attr->purple_upper_limit = ot_ext_system_cac_purple_upper_limit_read(vi_pipe);
    cac_attr->purple_lower_limit = ot_ext_system_cac_purple_lower_limit_read(vi_pipe);

    isp_cac_acac_attr_read(vi_pipe, &cac_attr->acac_cfg);
    isp_cac_lcac_attr_read(vi_pipe, &cac_attr->lcac_cfg);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_bayershp_attr(ot_vi_pipe vi_pipe, const ot_isp_bayershp_attr *bshp_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(bshp_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bayershp_init_return(vi_pipe);

    ret = isp_bshp_comm_attr_check("mpi", bshp_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ot_ext_system_bshp_enable_write(vi_pipe, bshp_attr->enable);
    ot_ext_system_bshp_manual_mode_write(vi_pipe, bshp_attr->op_type);
    isp_bayershp_common_attr_write(vi_pipe, bshp_attr);
    ret = isp_bshp_auto_attr_check("mpi", &bshp_attr->auto_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_bayershp_auto_attr_write(vi_pipe, &bshp_attr->auto_attr);

    ret = isp_bshp_manual_attr_check("mpi", &bshp_attr->manual_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_bayershp_manual_attr_write(vi_pipe, &bshp_attr->manual_attr);

    ot_ext_system_bshp_attr_update_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_bayershp_attr(ot_vi_pipe vi_pipe, ot_isp_bayershp_attr *bshp_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(bshp_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bayershp_init_return(vi_pipe);

    bshp_attr->enable  = ot_ext_system_bshp_enable_read(vi_pipe);
    bshp_attr->op_type = ot_ext_system_bshp_manual_mode_read(vi_pipe);

    isp_bayershp_auto_attr_read(vi_pipe, &bshp_attr->auto_attr);
    isp_bayershp_manual_attr_read(vi_pipe, &bshp_attr->manual_attr);
    isp_bayershp_common_attr_read(vi_pipe, bshp_attr);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_rc_attr(ot_vi_pipe vi_pipe, const ot_isp_rc_attr *rc_attr)
{
    return isp_set_rc_attr(vi_pipe, rc_attr);
}

td_s32 ot_mpi_isp_get_rc_attr(ot_vi_pipe vi_pipe, ot_isp_rc_attr *rc_attr)
{
    return isp_get_rc_attr(vi_pipe, rc_attr);
}

td_s32 ot_mpi_isp_set_nr_attr(ot_vi_pipe vi_pipe, const ot_isp_nr_attr *nr_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(nr_attr);

    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bayer_nr_init_return(vi_pipe);

    ret = isp_nr_comm_attr_check("mpi", nr_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_nr_comm_attr_write(vi_pipe, nr_attr);

    ret = isp_nr_snr_attr_check("mpi", &nr_attr->snr_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_nr_snr_attr_write(vi_pipe, &nr_attr->snr_cfg);

    ret = isp_nr_tnr_attr_check("mpi", &nr_attr->tnr_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_nr_tnr_attr_write(vi_pipe, &nr_attr->tnr_cfg);

    ret = isp_nr_wdr_attr_check("mpi", &nr_attr->wdr_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_nr_wdr_attr_write(vi_pipe, &nr_attr->wdr_cfg);

    ret = isp_nr_dering_attr_check("mpi", &nr_attr->dering_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_nr_dering_attr_write(vi_pipe, &nr_attr->dering_cfg);

    ot_ext_system_bayernr_attr_update_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_nr_attr(ot_vi_pipe vi_pipe, ot_isp_nr_attr *nr_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(nr_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bayer_nr_init_return(vi_pipe);

    isp_nr_comm_attr_read(vi_pipe, nr_attr);
    isp_nr_snr_attr_read(vi_pipe, &nr_attr->snr_cfg);
    isp_nr_tnr_attr_read(vi_pipe, &nr_attr->tnr_cfg);
    isp_nr_wdr_attr_read(vi_pipe, &nr_attr->wdr_cfg);
    isp_nr_dering_attr_read(vi_pipe, &nr_attr->dering_cfg);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_rgbir_attr(ot_vi_pipe vi_pipe, const ot_isp_rgbir_attr *rgbir_attr)
{
#ifdef CONFIG_OT_ISP_RGBIR_SUPPORT
    td_s32 ret;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(rgbir_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_rgbir_init_return(vi_pipe);

    ret = isp_rgbir_attr_check("mpi", vi_pipe, rgbir_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_rgbir_attr_write(vi_pipe, rgbir_attr);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_SET_RGBIR_FORMAT, &rgbir_attr->rgbir_cfg.in_rgbir_pattern);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set RGBIR FORMAT err!\n");
        return TD_FAILURE;
    }
    ot_ext_system_rgbir_attr_update_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_rgbir_attr(ot_vi_pipe vi_pipe, ot_isp_rgbir_attr *rgbir_attr)
{
#ifdef CONFIG_OT_ISP_RGBIR_SUPPORT
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(rgbir_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_rgbir_init_return(vi_pipe);

    isp_rgbir_attr_read(vi_pipe, rgbir_attr);

    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_set_color_tone_attr(ot_vi_pipe vi_pipe, const ot_isp_color_tone_attr *ct_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ct_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if ((ct_attr->red_cast_gain < 0x100) || (ct_attr->red_cast_gain > 0x180)) {
        isp_err_trace("Invalid red_cast_gain! should in range of [0x100, 0x180]\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((ct_attr->green_cast_gain < 0x100) || (ct_attr->green_cast_gain > 0x180)) {
        isp_err_trace("Invalid green_cast_gain! should in range of [0x100, 0x180]\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((ct_attr->blue_cast_gain < 0x100) || (ct_attr->blue_cast_gain > 0x180)) {
        isp_err_trace("Invalid blue_cast_gain! should in range of [0x100, 0x180]\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_cc_colortone_rgain_write(vi_pipe, ct_attr->red_cast_gain);
    ot_ext_system_cc_colortone_ggain_write(vi_pipe, ct_attr->green_cast_gain);
    ot_ext_system_cc_colortone_bgain_write(vi_pipe, ct_attr->blue_cast_gain);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_color_tone_attr(ot_vi_pipe vi_pipe, ot_isp_color_tone_attr *ct_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ct_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ct_attr->red_cast_gain   = ot_ext_system_cc_colortone_rgain_read(vi_pipe);
    ct_attr->green_cast_gain = ot_ext_system_cc_colortone_ggain_read(vi_pipe);
    ct_attr->blue_cast_gain  = ot_ext_system_cc_colortone_bgain_read(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_sharpen_mot_enable_set(ot_vi_pipe vi_pipe, td_bool motion_en)
{
    td_s32 ret;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_MOT_CFG_SET, &motion_en);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set mot cfg failed%x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_sharpen_attr(ot_vi_pipe vi_pipe, const ot_isp_sharpen_attr *shp_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shp_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_sharpen_init_return(vi_pipe);

    ret = isp_sharpen_comm_attr_check("mpi", shp_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_sharpen_comm_attr_write(vi_pipe, shp_attr);
    ret = isp_sharpen_mot_enable_set(vi_pipe, shp_attr->motion_en);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_sharpen_auto_attr_check("mpi", &shp_attr->auto_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_sharpen_auto_attr_write(vi_pipe, &shp_attr->auto_attr);

    ret = isp_sharpen_manual_attr_check("mpi", &shp_attr->manual_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_sharpen_manual_attr_write(vi_pipe, &shp_attr->manual_attr);

    ot_ext_system_sharpen_mpi_update_en_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_sharpen_attr(ot_vi_pipe vi_pipe, ot_isp_sharpen_attr *shp_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(shp_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_sharpen_init_return(vi_pipe);

    isp_sharpen_comm_attr_read(vi_pipe, shp_attr);
    isp_sharpen_auto_attr_read(vi_pipe, &shp_attr->auto_attr);
    isp_sharpen_manual_attr_read(vi_pipe, &shp_attr->manual_attr);

    return TD_SUCCESS;
}

/* Crosstalk Removal Strength */
td_s32 ot_mpi_isp_set_crosstalk_attr(ot_vi_pipe vi_pipe, const ot_isp_cr_attr *cr_attr)
{
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    td_u8 i;
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(cr_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_crosstalk_init_return(vi_pipe);

    ret = isp_crosstalk_attr_check("mpi", cr_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_ge_enable_write(vi_pipe, cr_attr->enable);
    ot_ext_system_ge_slope_write(vi_pipe, cr_attr->slope);
    ot_ext_system_ge_sensitivity_write(vi_pipe, cr_attr->sensi_slope);
    ot_ext_system_ge_sensithreshold_write(vi_pipe, cr_attr->sensi_threshold);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_ge_strength_write(vi_pipe, i, cr_attr->strength[i]);
        ot_ext_system_ge_npoffset_write(vi_pipe, i, cr_attr->np_offset[i]);
        ot_ext_system_ge_filter_mode_write(vi_pipe, i, cr_attr->filter_mode[i]);
        ot_ext_system_ge_threshold_write(vi_pipe, i, cr_attr->threshold[i]);
    }

    ot_ext_system_ge_coef_update_en_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
#else
    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_crosstalk_attr(ot_vi_pipe vi_pipe, ot_isp_cr_attr *cr_attr)
{
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    td_u8 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(cr_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_crosstalk_init_return(vi_pipe);

    cr_attr->enable      = ot_ext_system_ge_enable_read(vi_pipe);
    cr_attr->slope       = ot_ext_system_ge_slope_read(vi_pipe);
    cr_attr->sensi_slope = ot_ext_system_ge_sensitivity_read(vi_pipe);
    cr_attr->sensi_threshold = ot_ext_system_ge_sensithreshold_read(vi_pipe);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        cr_attr->strength[i]  = ot_ext_system_ge_strength_read(vi_pipe, i);
        cr_attr->np_offset[i] = ot_ext_system_ge_npoffset_read(vi_pipe, i);
        cr_attr->filter_mode[i] = ot_ext_system_ge_filter_mode_read(vi_pipe, i);
        cr_attr->threshold[i] = ot_ext_system_ge_threshold_read(vi_pipe, i);
    }

    return TD_SUCCESS;
#else
    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_set_anti_false_color_attr(ot_vi_pipe vi_pipe, const ot_isp_anti_false_color_attr *anti_false_color)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(anti_false_color);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_anti_false_color_init_return(vi_pipe);

    ret = isp_anti_false_color_attr_check("mpi", anti_false_color);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_anti_false_color_attr_write(vi_pipe, anti_false_color);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_anti_false_color_attr(ot_vi_pipe vi_pipe, ot_isp_anti_false_color_attr *anti_false_color)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(anti_false_color);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_anti_false_color_init_return(vi_pipe);

    isp_anti_false_color_attr_read(vi_pipe, anti_false_color);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_demosaic_attr(ot_vi_pipe vi_pipe, const ot_isp_demosaic_attr *demosaic_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(demosaic_attr);
    isp_check_bool_return(demosaic_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_demosaic_init_return(vi_pipe);

    ret = isp_demosaic_attr_check("mpi", demosaic_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_demosaic_attr_write(vi_pipe, demosaic_attr);

    ot_ext_system_demosaic_attr_update_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_demosaic_attr(ot_vi_pipe vi_pipe, ot_isp_demosaic_attr *demosaic_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(demosaic_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_demosaic_init_return(vi_pipe);

    isp_demosaic_attr_read(vi_pipe, demosaic_attr);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_ca_attr(ot_vi_pipe vi_pipe, const ot_isp_ca_attr *ca_attr)
{
#ifdef CONFIG_OT_ISP_CA_SUPPORT
    td_u16 i;
    td_u32 cp_lut_value;
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ca_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_ca_init_return(vi_pipe);
    ret = isp_ca_attr_check("mpi", ca_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ot_ext_system_ca_en_write(vi_pipe, ca_attr->enable);
    ot_ext_system_ca_cp_en_write(vi_pipe, ca_attr->ca_cp_en);
    for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
        cp_lut_value = (ca_attr->cp.cp_lut_y[i] << 0x10) + (ca_attr->cp.cp_lut_u[i] << 0x8) + ca_attr->cp.cp_lut_v[i];
        ot_ext_system_ca_cp_lut_write(vi_pipe, i, cp_lut_value);
        ot_ext_system_ca_y_ratio_lut_write(vi_pipe, i, ca_attr->ca.y_ratio_lut[i]);
        ot_ext_system_ca_y_sat_lut_write(vi_pipe, i, ca_attr->ca.y_sat_lut[i]);
    }
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_ca_iso_ratio_lut_write(vi_pipe, i, ca_attr->ca.iso_ratio[i]);
    }
    ot_ext_system_ca_coef_update_en_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_ca_attr(ot_vi_pipe vi_pipe, ot_isp_ca_attr *ca_attr)
{
#ifdef CONFIG_OT_ISP_CA_SUPPORT
    td_u16 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ca_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_ca_init_return(vi_pipe);

    ca_attr->enable   = ot_ext_system_ca_en_read(vi_pipe);
    ca_attr->ca_cp_en = ot_ext_system_ca_cp_en_read(vi_pipe);

    for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
        ca_attr->cp.cp_lut_y[i] = (ot_ext_system_ca_cp_lut_read(vi_pipe, i) >> 16) & 0xFF; /* bit16~23: cp_lut_y */
        ca_attr->cp.cp_lut_u[i] = (ot_ext_system_ca_cp_lut_read(vi_pipe, i) >> 8) & 0xFF;  /* bit8~15: cp_lut_u */
        ca_attr->cp.cp_lut_v[i] = ot_ext_system_ca_cp_lut_read(vi_pipe, i) & 0xFF;         /* bit0~7: cp_lut_v */
    }

    for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
        ca_attr->ca.y_ratio_lut[i] = ot_ext_system_ca_y_ratio_lut_read(vi_pipe, i);
        ca_attr->ca.y_sat_lut[i] = ot_ext_system_ca_y_sat_lut_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ca_attr->ca.iso_ratio[i] = (td_s32)ot_ext_system_ca_iso_ratio_lut_read(vi_pipe, i);
    }

    return TD_SUCCESS;
#else
    isp_err_trace("not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_set_clut_coeff(ot_vi_pipe vi_pipe, const ot_isp_clut_lut *clut_lut)
{
    return isp_set_clut_coeff(vi_pipe, clut_lut);
}

td_s32 ot_mpi_isp_get_clut_coeff(ot_vi_pipe vi_pipe, ot_isp_clut_lut *clut_lut)
{
    return isp_get_clut_coeff(vi_pipe, clut_lut);
}

td_s32 ot_mpi_isp_set_clut_attr(ot_vi_pipe vi_pipe, const ot_isp_clut_attr *clut_attr)
{
    return isp_set_clut_attr(vi_pipe, clut_attr);
}

td_s32 ot_mpi_isp_get_clut_attr(ot_vi_pipe vi_pipe, ot_isp_clut_attr *clut_attr)
{
    return isp_get_clut_attr(vi_pipe, clut_attr);
}

td_s32 ot_mpi_isp_set_black_level_attr(ot_vi_pipe vi_pipe, const ot_isp_black_level_attr *black_level)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(black_level);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_blc_init_return(vi_pipe);

    ret = isp_user_black_level_en_check(vi_pipe, black_level->user_black_level_en);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_black_level_value_check("mpi user", black_level->user_black_level);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ot_ext_system_isp_user_blc_en_write(vi_pipe, black_level->user_black_level_en);
    isp_user_black_level_write(vi_pipe, black_level->user_black_level);

    if (black_level->black_level_mode >= OT_ISP_BLACK_LEVEL_MODE_BUTT) {
        isp_err_trace("Invalid black_level_mode %d!\n", black_level->black_level_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_black_level_mode_write(vi_pipe, black_level->black_level_mode);
    ret = isp_black_level_value_check("mpi manual", black_level->manual_attr.black_level);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    isp_black_level_manual_attr_write(vi_pipe, &black_level->manual_attr);

    ret = isp_dynamic_blc_attr_check(vi_pipe, "mpi", &black_level->dynamic_attr, black_level->black_level_mode);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_black_level_dynamic_attr_write(vi_pipe, &black_level->dynamic_attr);

    ot_ext_system_black_level_change_write(vi_pipe, (td_u8)TD_TRUE);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_black_level_attr(ot_vi_pipe vi_pipe, ot_isp_black_level_attr *black_level)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(black_level);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_blc_init_return(vi_pipe);
    black_level->black_level_mode    = ot_ext_system_black_level_mode_read(vi_pipe);
    black_level->user_black_level_en = ot_ext_system_isp_user_blc_en_read(vi_pipe);
    isp_user_black_level_read(vi_pipe, black_level->user_black_level);
    isp_black_level_manual_attr_read(vi_pipe, &black_level->manual_attr);
    isp_black_level_dynamic_attr_read(vi_pipe, &black_level->dynamic_attr);
    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_PIPE_FPN
static td_bool isp_check_2_power(td_u32 value)
{
    td_bool ret = TD_TRUE;
    td_u32 temp_val = value;
    td_u32 num = 0;

    while (temp_val) {
        if (temp_val & 0x1) {
            num++;
            if (num > 1) {
                ret = TD_FALSE;
                break;
            }
        }

        temp_val >>= 1;
    }

    return ret;
}

static td_s32 ot_mpi_isp_fpn_calibrate_attr_check(ot_vi_pipe vi_pipe, const ot_isp_fpn_calibrate_attr *calibrate_attr)
{
    td_bool fpn_cor_enable;
    td_u8 wdr_mode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    if (is_wdr_mode(wdr_mode)) {
        isp_err_trace("do not support FPN calibration in WDR mode!\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    fpn_cor_enable = isp_ext_system_fpn_cor_enable_read(vi_pipe);
    if (fpn_cor_enable) {
        isp_err_trace("cannot do FPN calibration when FPN correction is on!\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (calibrate_attr->fpn_type != OT_ISP_FPN_TYPE_FRAME) {
        isp_err_trace("invalid fpn_type %d, must be ISP_FPN_TYPE_FRAME.\n", calibrate_attr->fpn_type);
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (calibrate_attr->fpn_mode >= OT_ISP_FPN_OUT_MODE_BUTT) {
        isp_err_trace("invalid fpn_mode %d.\n", calibrate_attr->fpn_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((calibrate_attr->frame_num < 1) || (calibrate_attr->frame_num > 16)) { /* [1, 16] */
        isp_err_trace("invalid frame_num %u, must be in [1, 16].\n", calibrate_attr->frame_num);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (isp_check_2_power(calibrate_attr->frame_num) == TD_FALSE) {
        isp_err_trace("invalid frame_num %u, must be 2^N.\n", calibrate_attr->frame_num);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if ((calibrate_attr->threshold > 0xfff) || (calibrate_attr->threshold < 1)) {
        isp_err_trace("invalid threshold %u, must be in [1, 0xfff].\n", calibrate_attr->threshold);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 ot_mpi_isp_fpn_correction_attr_check(ot_vi_pipe vi_pipe, const ot_isp_fpn_attr *fpn_attr)
{
    td_u8 i;
    td_u8 wdr_mode = ot_ext_system_sensor_wdr_mode_read(vi_pipe);
    if (is_2to1_wdr_mode(wdr_mode)) {
        isp_err_trace("do not support FPN correction in 2to1 WDR mode!\n");
        return OT_ERR_ISP_NOT_SUPPORT;
    }

    if (fpn_attr->op_type >= OT_OP_MODE_BUTT) {
        isp_err_trace("invalid op_type %d, which should be OT_OP_MODE_AUTO or OT_OP_MODE_MANUAL.\n", fpn_attr->op_type);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (fpn_attr->fpn_type != OT_ISP_FPN_TYPE_FRAME) {
        isp_err_trace("invalid fpn_type %d.\n", fpn_attr->fpn_type);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
        if (fpn_attr->fpn_frm_info.offset[i] > 0x3FFF) {
            isp_err_trace("invalid offset[%u], must be in [0, 0x3FFF].\n", i);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    if (fpn_attr->op_type == OT_OP_MODE_MANUAL) {
        if (fpn_attr->manual_attr.strength > 0x3FF) {
            isp_err_trace("invalid fpn strength, must be in [0, 1023]!\n");
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

#endif

td_s32 ot_mpi_isp_fpn_calibrate(ot_vi_pipe vi_pipe, ot_isp_fpn_calibrate_attr *calibrate_attr)
{
#ifdef CONFIG_OT_VI_PIPE_FPN
    td_s32 ret;
    isp_check_phy_pipe_return(vi_pipe);
    isp_check_pointer_return(calibrate_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = ot_mpi_isp_fpn_calibrate_attr_check(vi_pipe, calibrate_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return isp_set_calibrate_attr(vi_pipe, calibrate_attr);
#else
    ot_unused(vi_pipe);
    ot_unused(calibrate_attr);

    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_set_fpn_attr(ot_vi_pipe vi_pipe, const ot_isp_fpn_attr *fpn_attr)
{
#ifdef CONFIG_OT_VI_PIPE_FPN
    td_s32 ret;
    isp_check_phy_pipe_return(vi_pipe);
    isp_check_pointer_return(fpn_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bool_return(fpn_attr->enable);
    isp_check_bool_return(fpn_attr->aibnr_mode);
    ret = ot_mpi_isp_fpn_correction_attr_check(vi_pipe, fpn_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return isp_set_correction_attr(vi_pipe, fpn_attr);
#else
    ot_unused(vi_pipe);
    ot_unused(fpn_attr);
    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_fpn_attr(ot_vi_pipe vi_pipe, ot_isp_fpn_attr *fpn_attr)
{
#ifdef CONFIG_OT_VI_PIPE_FPN
    isp_check_phy_pipe_return(vi_pipe);
    isp_check_pointer_return(fpn_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    return isp_get_correction_attr(vi_pipe, fpn_attr);
#else
    ot_unused(vi_pipe);
    ot_unused(fpn_attr);
    isp_err_trace("Not support this interface!\n");
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_isp_reg_attr(ot_vi_pipe vi_pipe, ot_isp_reg_attr *isp_reg_attr)
{
    td_s32 ret;
    td_u32 isp_bind_attr;
    ot_isp_3a_alg_lib ae_lib;
    ot_isp_3a_alg_lib awb_lib;
    isp_usr_ctx *isp_ctx_info = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx_info);
    isp_check_pointer_return(isp_ctx_info);
    isp_check_pointer_return(isp_reg_attr);
    isp_mutex_lock(vi_pipe);
    isp_check_open_goto(vi_pipe, ret, exit);
    isp_check_mem_init_goto(vi_pipe, ret, exit);
    isp_check_vreg_permission_goto(vi_pipe, ret, exit);

    isp_bind_attr = ot_ext_system_bind_attr_read(vi_pipe);
    ae_lib.id  = (isp_bind_attr >> 8) & 0xFF;      /* bit0~7: awb_lib.id, bit8~15: ae_lib.id */
    awb_lib.id = isp_bind_attr & 0xFF;

    isp_reg_attr->isp_ext_reg_addr = vreg_get_virt_addr_base(isp_vir_reg_base(vi_pipe));
    isp_reg_attr->isp_ext_reg_size = ISP_VREG_SIZE;
    isp_reg_attr->ae_ext_reg_addr  = vreg_get_virt_addr_base(ae_lib_vreg_base(ae_lib.id));
    isp_reg_attr->ae_ext_reg_size  = AE_VREG_SIZE;
    isp_reg_attr->awb_ext_reg_addr = vreg_get_virt_addr_base(awb_lib_vreg_base(awb_lib.id));
    isp_reg_attr->awb_ext_reg_size = AWB_VREG_SIZE;
    ret = TD_SUCCESS;
exit:
    isp_mutex_unlock(vi_pipe);
    return ret;
}

td_s32 ot_mpi_isp_set_debug(ot_vi_pipe vi_pipe, const ot_isp_debug_info *isp_debug)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(isp_debug);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    isp_check_bool_return(isp_debug->debug_en);
    return isp_dbg_reg_set(vi_pipe, isp_debug);
}

td_s32 ot_mpi_isp_get_debug(ot_vi_pipe vi_pipe, ot_isp_debug_info *isp_debug)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(isp_debug);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    return isp_dbg_reg_get(vi_pipe, isp_debug);
}

td_s32 ot_mpi_isp_set_ctrl_param(ot_vi_pipe vi_pipe, const ot_isp_ctrl_param *isp_ctrl_param)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(isp_ctrl_param);

    isp_check_open_return(vi_pipe);

    return ioctl(isp_get_fd(vi_pipe), ISP_SET_CTRL_PARAM, isp_ctrl_param);
}

td_s32 ot_mpi_isp_get_ctrl_param(ot_vi_pipe vi_pipe, ot_isp_ctrl_param *isp_ctrl_param)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(isp_ctrl_param);
    isp_check_open_return(vi_pipe);

    return ioctl(isp_get_fd(vi_pipe), ISP_GET_CTRL_PARAM, isp_ctrl_param);
}

td_s32 ot_mpi_isp_query_inner_state_info(ot_vi_pipe vi_pipe, ot_isp_inner_state_info *inner_state_info)
{
    td_s32 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(inner_state_info);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    inner_state_info->over_shoot    = ot_ext_system_actual_sharpen_overshoot_amt_read(vi_pipe);
    inner_state_info->under_shoot   = ot_ext_system_actual_sharpen_undershoot_amt_read(vi_pipe);
    inner_state_info->shoot_sup_strength = ot_ext_system_actual_sharpen_shoot_sup_read(vi_pipe);
    inner_state_info->texture_freq  = ot_ext_system_actual_sharpen_texture_frequence_read(vi_pipe);
    inner_state_info->edge_freq     = ot_ext_system_actual_sharpen_edge_frequence_read(vi_pipe);
    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        inner_state_info->edge_strength[i]    = ot_ext_system_actual_sharpen_edge_str_read(vi_pipe, i);
        inner_state_info->texture_strength[i] = ot_ext_system_actual_sharpen_texture_str_read(vi_pipe, i);
    }

    inner_state_info->nr_lsc_ratio = ot_ext_system_bayernr_actual_nr_lsc_ratio_read(vi_pipe);
    inner_state_info->coring_wgt   = ot_ext_system_bayernr_actual_coring_weight_read(vi_pipe);
    inner_state_info->fine_strength  = ot_ext_system_bayernr_actual_fine_strength_read(vi_pipe);

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        inner_state_info->coarse_strength[i] = ot_ext_system_bayernr_actual_coarse_strength_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        inner_state_info->wdr_frame_strength[i] = ot_ext_system_bayernr_actual_wdr_frame_strength_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM - 1; i++) {
        inner_state_info->wdr_exp_ratio_actual[i] = ot_ext_system_actual_wdr_exposure_ratio_read(vi_pipe, i);
    }

    inner_state_info->drc_strength_actual    = ot_ext_system_drc_actual_strength_read(vi_pipe);
    inner_state_info->dehaze_strength_actual = ot_ext_system_dehaze_actual_strength_read(vi_pipe);
    inner_state_info->wdr_switch_finish      = ot_ext_top_wdr_switch_read(vi_pipe);
    inner_state_info->res_switch_finish      = ot_ext_top_res_switch_read(vi_pipe);

    isp_black_level_actual_value_read(vi_pipe, &inner_state_info->black_level_actual[0]);
    sns_black_level_actual_value_read(vi_pipe, inner_state_info->sns_black_level);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_dng_image_static_info(ot_vi_pipe vi_pipe, ot_isp_dng_image_static_info *dng_image_static_info)
{
    td_bool valid_dng_image_static_info;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dng_image_static_info);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    valid_dng_image_static_info = ot_ext_system_dng_static_info_valid_read(vi_pipe);
    if (valid_dng_image_static_info == TD_FALSE) {
        isp_err_trace("dng_image_static_info have not been set in xxx_cmos.x!\n");
        return TD_SUCCESS;
    }

    if (ioctl(isp_get_fd(vi_pipe), ISP_DNG_INFO_GET, dng_image_static_info) != TD_SUCCESS) {
        return OT_ERR_ISP_NOMEM;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_dng_color_param(ot_vi_pipe vi_pipe, const ot_isp_dng_color_param *dng_color_param)
{
    td_u32 i;
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dng_color_param);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = isp_dng_param_check("mpi", dng_color_param);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_dng_low_wb_gain_r_write(vi_pipe, dng_color_param->wb_gain1.r_gain);
    ot_ext_system_dng_low_wb_gain_g_write(vi_pipe, dng_color_param->wb_gain1.g_gain);
    ot_ext_system_dng_low_wb_gain_b_write(vi_pipe, dng_color_param->wb_gain1.b_gain);
    ot_ext_system_dng_high_wb_gain_r_write(vi_pipe, dng_color_param->wb_gain2.r_gain);
    ot_ext_system_dng_high_wb_gain_g_write(vi_pipe, dng_color_param->wb_gain2.g_gain);
    ot_ext_system_dng_high_wb_gain_b_write(vi_pipe, dng_color_param->wb_gain2.b_gain);

    ot_ext_system_dng_low_temp_write(vi_pipe, dng_color_param->ccm_tab1.color_temp);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ot_ext_system_dng_low_ccm_write(vi_pipe, i, direct_to_complement_u16(dng_color_param->ccm_tab1.ccm[i]));
    }

    ot_ext_system_dng_high_temp_write(vi_pipe, dng_color_param->ccm_tab2.color_temp);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ot_ext_system_dng_high_ccm_write(vi_pipe, i, direct_to_complement_u16(dng_color_param->ccm_tab2.ccm[i]));
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_dng_color_param(ot_vi_pipe vi_pipe, ot_isp_dng_color_param *dng_color_param)
{
    td_u32 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(dng_color_param);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    dng_color_param->wb_gain1.r_gain = ot_ext_system_dng_low_wb_gain_r_read(vi_pipe);
    dng_color_param->wb_gain1.g_gain = ot_ext_system_dng_low_wb_gain_g_read(vi_pipe);
    dng_color_param->wb_gain1.b_gain = ot_ext_system_dng_low_wb_gain_b_read(vi_pipe);

    dng_color_param->wb_gain2.r_gain = ot_ext_system_dng_high_wb_gain_r_read(vi_pipe);
    dng_color_param->wb_gain2.g_gain = ot_ext_system_dng_high_wb_gain_g_read(vi_pipe);
    dng_color_param->wb_gain2.b_gain = ot_ext_system_dng_high_wb_gain_b_read(vi_pipe);

    dng_color_param->ccm_tab1.color_temp = ot_ext_system_dng_low_temp_read(vi_pipe);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        dng_color_param->ccm_tab1.ccm[i] = complement_to_direct_u16(ot_ext_system_dng_low_ccm_read(vi_pipe, i));
    }

    dng_color_param->ccm_tab2.color_temp = ot_ext_system_dng_high_temp_read(vi_pipe);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        dng_color_param->ccm_tab2.ccm[i] = complement_to_direct_u16(ot_ext_system_dng_high_ccm_read(vi_pipe, i));
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_ir_auto(ot_vi_pipe vi_pipe, ot_isp_ir_auto_attr *ir_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ir_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_bool_return(ir_attr->enable);

    if (ir_attr->rg_max > 0xFFF) {
        isp_err_trace("Invalid rg_max :%u!\n", ir_attr->rg_max);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->rg_min > 0xFFF) {
        isp_err_trace("Invalid rg_min :%u!\n", ir_attr->rg_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->bg_max > 0xFFF) {
        isp_err_trace("Invalid bg_max :%u!\n", ir_attr->bg_max);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->bg_min > 0xFFF) {
        isp_err_trace("Invalid bg_min :%u!\n", ir_attr->bg_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->rg_max < ir_attr->rg_min) {
        isp_err_trace("rg_max(%u) should not be less than rg_min(%u)!\n", ir_attr->rg_max, ir_attr->rg_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->bg_max < ir_attr->bg_min) {
        isp_err_trace("bg_max(%u) should not be less than bg_min(%u)!\n", ir_attr->bg_max, ir_attr->bg_min);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (ir_attr->ir_status >= OT_ISP_IR_BUTT) {
        isp_err_trace("Invalid ir_status :%u!\n", ir_attr->ir_status);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return isp_ir_auto_run_once(vi_pipe, ir_attr);
}

static td_s32 isp_smart_info_param_check(ot_vi_pipe vi_pipe, const ot_isp_smart_info *smart_info)
{
    td_s32 i;
    td_u16 max_width, max_height;
    const td_u32 max_area_ratio = 10000; /* maximum value of max_area_ratio */
    const td_u32 max_exp_pref = 10000;   /* maximum value of tunnel_exp_perf */

    max_width = ot_ext_sync_total_width_read(vi_pipe);
    max_height = ot_ext_sync_total_height_read(vi_pipe);

    for (i = 0; i < OT_ISP_FACE_NUM; i++) {
        if ((smart_info->face_roi.face_rect[i].x < 0) || (smart_info->face_roi.face_rect[i].y < 0)) {
            isp_err_trace("face_rect[%d].x:%d or face_rect[%d].y:%d is less than zero!\n",
                i, smart_info->face_roi.face_rect[i].x, i, smart_info->face_roi.face_rect[i].y);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if ((smart_info->face_roi.face_rect[i].x > max_width) || (smart_info->face_roi.face_rect[i].y > max_height)) {
            isp_err_trace("face_rect[%d].x:%d or face_rect[%d].y:%d exceeds the maximum,max_width:%u max_height:%u!\n",
                i, smart_info->face_roi.face_rect[i].x, i, smart_info->face_roi.face_rect[i].y,
                max_width, max_height);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if (smart_info->face_roi.face_rect[i].x + smart_info->face_roi.face_rect[i].width > max_width) {
            isp_err_trace("The sum of face_rect[%d].x:%d and face_rect[%d].width:%u exceeds the maximum:%u!\n",
                i, smart_info->face_roi.face_rect[i].x, i, smart_info->face_roi.face_rect[i].width,
                max_width);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if (smart_info->face_roi.face_rect[i].y + smart_info->face_roi.face_rect[i].height > max_height) {
            isp_err_trace("The sum of face_rect[%d].y:%d and face_rect[%d].height :%u exceeds the maximum:%u!\n",
                i, smart_info->face_roi.face_rect[i].y, i, smart_info->face_roi.face_rect[i].height,
                max_height);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
        if (smart_info->tunnel_roi[i].tunnel_area_ratio > max_area_ratio) {
            isp_err_trace("tunnel_roi[%d].tunnel_area_ratio:%d is more than maximum:%d!\n",
                i, smart_info->tunnel_roi[i].tunnel_area_ratio, max_area_ratio);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if (smart_info->tunnel_roi[i].tunnel_exp_perf > max_exp_pref) {
            isp_err_trace("tunnel_roi[%d].tunnel_exp_perf:%d is more than maximum:%d!\n",
                i, smart_info->tunnel_roi[i].tunnel_exp_perf, max_exp_pref);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }
    return TD_SUCCESS;
}
td_s32 ot_mpi_isp_set_smart_info(ot_vi_pipe vi_pipe, const ot_isp_smart_info *smart_info)
{
    td_u16 i;
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(smart_info);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = isp_smart_info_param_check(vi_pipe, smart_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    for (i = 0; i < OT_ISP_PEOPLE_CLASS_MAX; i++) {
        isp_check_bool_return(smart_info->people_roi[i].enable);
        isp_check_bool_return(smart_info->people_roi[i].available);
        ot_ext_system_smart_enable_write(vi_pipe, i, smart_info->people_roi[i].enable);
        ot_ext_system_smart_available_write(vi_pipe, i, smart_info->people_roi[i].available);
        ot_ext_system_smart_luma_write(vi_pipe, i, smart_info->people_roi[i].luma);
    }
    ot_ext_system_smart_update_write(vi_pipe, TD_TRUE);

    for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
        isp_check_bool_return(smart_info->tunnel_roi[i].enable);
        isp_check_bool_return(smart_info->tunnel_roi[i].available);
        ot_ext_system_tunnel_enable_write(vi_pipe, i, smart_info->tunnel_roi[i].enable);
        ot_ext_system_tunnel_available_write(vi_pipe, i, smart_info->tunnel_roi[i].available);
        ot_ext_system_tunnel_area_ratio_write(vi_pipe, i, smart_info->tunnel_roi[i].tunnel_area_ratio);
        ot_ext_system_tunnel_exp_perf_write(vi_pipe, i, smart_info->tunnel_roi[i].tunnel_exp_perf);
    }
    ot_ext_system_tunnel_update_write(vi_pipe, TD_TRUE);

    isp_check_bool_return(smart_info->face_roi.enable);
    isp_check_bool_return(smart_info->face_roi.available);
    ot_ext_system_fast_face_enable_write(vi_pipe, smart_info->face_roi.enable);
    ot_ext_system_fast_face_available_write(vi_pipe, smart_info->face_roi.available);
    ot_ext_system_fast_face_frame_pts_write(vi_pipe, smart_info->face_roi.frame_pts);
    for (i = 0; i < OT_ISP_FACE_NUM; i++) {
        ot_ext_system_fast_face_x_write(vi_pipe, i, (td_u32)smart_info->face_roi.face_rect[i].x);
        ot_ext_system_fast_face_y_write(vi_pipe, i, (td_u32)smart_info->face_roi.face_rect[i].y);
        ot_ext_system_fast_face_width_write(vi_pipe, i, smart_info->face_roi.face_rect[i].width);
        ot_ext_system_fast_face_height_write(vi_pipe, i, smart_info->face_roi.face_rect[i].height);
    }
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_smart_info(ot_vi_pipe vi_pipe, ot_isp_smart_info *smart_info)
{
    td_u16 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(smart_info);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    for (i = 0; i < OT_ISP_PEOPLE_CLASS_MAX; i++) {
        smart_info->people_roi[i].enable    = ot_ext_system_smart_enable_read(vi_pipe, i);
        smart_info->people_roi[i].available = ot_ext_system_smart_available_read(vi_pipe, i);
        smart_info->people_roi[i].luma      = ot_ext_system_smart_luma_read(vi_pipe, i);
    }
    for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
        smart_info->tunnel_roi[i].enable       = ot_ext_system_tunnel_enable_read(vi_pipe, i);
        smart_info->tunnel_roi[i].available    = ot_ext_system_tunnel_available_read(vi_pipe, i);
        smart_info->tunnel_roi[i].tunnel_area_ratio = ot_ext_system_tunnel_area_ratio_read(vi_pipe, i);
        smart_info->tunnel_roi[i].tunnel_exp_perf   = ot_ext_system_tunnel_exp_perf_read(vi_pipe, i);
    }
    smart_info->face_roi.enable = ot_ext_system_fast_face_enable_read(vi_pipe);
    smart_info->face_roi.available = ot_ext_system_fast_face_available_read(vi_pipe);
    smart_info->face_roi.frame_pts = ot_ext_system_fast_face_frame_pts_read(vi_pipe);
    for (i = 0; i < OT_ISP_FACE_NUM; i++) {
        smart_info->face_roi.face_rect[i].x = ot_ext_system_fast_face_x_read(vi_pipe, i);
        smart_info->face_roi.face_rect[i].y = ot_ext_system_fast_face_y_read(vi_pipe, i);
        smart_info->face_roi.face_rect[i].width = ot_ext_system_fast_face_width_read(vi_pipe, i);
        smart_info->face_roi.face_rect[i].height = ot_ext_system_fast_face_height_read(vi_pipe, i);
    }
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_calc_flicker_type(ot_vi_pipe vi_pipe, ot_isp_calc_flicker_input *input_param,
                                    ot_isp_calc_flicker_output *output_param, ot_video_frame_info frame[],
                                    td_u32 array_size)
{
    return isp_calc_flicker_type(vi_pipe, input_param, output_param, frame, array_size);
}

td_s32 ot_mpi_isp_set_expander_attr(ot_vi_pipe vi_pipe, const ot_isp_expander_attr *expander_attr)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(expander_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_expander_init_return(vi_pipe);

    ret = isp_expander_attr_check("mpi", vi_pipe, expander_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_expander_attr_write(vi_pipe, expander_attr);

    ot_ext_system_expander_param_update_write(vi_pipe, TD_TRUE);
    ot_ext_system_expander_blc_param_update_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_expander_attr(ot_vi_pipe vi_pipe, ot_isp_expander_attr *expander_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(expander_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_expander_init_return(vi_pipe);

    isp_expander_attr_read(vi_pipe, expander_attr);

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_be_frame_attr(ot_vi_pipe vi_pipe, const ot_isp_be_frame_attr *be_frame_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_frame_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (be_frame_attr->frame_pos < 0 || be_frame_attr->frame_pos >= OT_ISP_DUMP_FRAME_POS_BUTT) {
        isp_err_trace("Invalid frame position %d!\n", be_frame_attr->frame_pos);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_isp_dump_frame_pos_write(vi_pipe, be_frame_attr->frame_pos);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_be_frame_attr(ot_vi_pipe vi_pipe, ot_isp_be_frame_attr *be_frame_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_frame_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    be_frame_attr->frame_pos      = ot_ext_system_isp_dump_frame_pos_read(vi_pipe);
    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_get_noise_calibration(ot_vi_pipe vi_pipe, ot_isp_noise_calibration *noise_calibration)
{
    td_u32 i;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(noise_calibration);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    for (i = 0; i < OT_BAYER_CALIBRATION_PARA_NUM_NEW; i++) {
        noise_calibration->calibration_coef[i] =
            ot_ext_system_sns_noise_calibration_lut_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_isp_set_detail_stats_cfg(ot_vi_pipe vi_pipe,
    const ot_isp_detail_stats_cfg *detail_stats_cfg)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_check_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(detail_stats_cfg);
    isp_check_open_return(vi_pipe);
    return ioctl(isp_get_fd(vi_pipe), ISP_SET_DETAIL_STATS_CFG, detail_stats_cfg);
#else
    ot_unused(vi_pipe);
    ot_unused(detail_stats_cfg);
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_detail_stats_cfg(ot_vi_pipe vi_pipe, ot_isp_detail_stats_cfg *detail_stats_cfg)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_check_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(detail_stats_cfg);
    isp_check_open_return(vi_pipe);

    return ioctl(isp_get_fd(vi_pipe), ISP_GET_DETAIL_STATS_CFG, detail_stats_cfg);
#else
    ot_unused(vi_pipe);
    ot_unused(detail_stats_cfg);
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}

td_s32 ot_mpi_isp_get_detail_stats(ot_vi_pipe vi_pipe, ot_isp_detail_stats *detail_stats)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_detail_stats_user stats;
    isp_check_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(detail_stats);
    isp_check_open_return(vi_pipe);

    stats.detail_stats = detail_stats;
    stats.ae_grid_status = detail_stats->ae_stats.ae_grid_info.status;
    stats.awb_grid_status = detail_stats->awb_stats.awb_grid_info.status;
    return ioctl(isp_get_fd(vi_pipe), ISP_GET_DETAIL_STATS, &stats);
#else
    ot_unused(vi_pipe);
    ot_unused(detail_stats);
    return OT_ERR_ISP_NOT_SUPPORT;
#endif
}
