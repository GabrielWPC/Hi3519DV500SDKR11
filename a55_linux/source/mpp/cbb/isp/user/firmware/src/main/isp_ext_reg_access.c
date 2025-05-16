/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_ext_reg_access.h"
#include <math.h>
#include "ot_math.h"
#include "mkp_isp.h"
#include "ot_common_isp.h"
#include "ot_isp_debug.h"
#include "ot_isp_define.h"
#include "ot_common_video.h"
#include "isp_ext_config.h"

td_void isp_sharpen_auto_attr_write(ot_vi_pipe vi_pipe, const ot_isp_sharpen_auto_attr *auto_attr)
{
    td_u8 i, j;
    td_u16 index;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
            index = i + j * OT_ISP_AUTO_ISO_NUM;
            ot_ext_system_sharpen_texture_str_write(vi_pipe, index, auto_attr->texture_strength[j][i]);
            ot_ext_system_sharpen_edge_str_write(vi_pipe, index, auto_attr->edge_strength[j][i]);
            ot_ext_system_sharpen_auto_mot_texture_str_write(vi_pipe, index, auto_attr->motion_texture_strength[j][i]);
            ot_ext_system_sharpen_auto_mot_edge_str_write(vi_pipe, index, auto_attr->motion_edge_strength[j][i]);
        }
        for (j = 0; j < OT_ISP_SHARPEN_LUMA_NUM; j++) {
            index = i + j * OT_ISP_AUTO_ISO_NUM;
            ot_ext_system_sharpen_luma_wgt_write(vi_pipe, index, auto_attr->luma_wgt[j][i]);
        }

        ot_ext_system_sharpen_texture_freq_write(vi_pipe, i, auto_attr->texture_freq[i]);
        ot_ext_system_sharpen_edge_freq_write(vi_pipe, i, auto_attr->edge_freq[i]);
        ot_ext_system_sharpen_over_shoot_write(vi_pipe, i, auto_attr->over_shoot[i]);
        ot_ext_system_sharpen_under_shoot_write(vi_pipe, i, auto_attr->under_shoot[i]);
        ot_ext_system_sharpen_auto_mot_texture_freq_write(vi_pipe, i, auto_attr->motion_texture_freq[i]);
        ot_ext_system_sharpen_auto_mot_edge_freq_write(vi_pipe, i, auto_attr->motion_edge_freq[i]);
        ot_ext_system_sharpen_auto_mot_over_shoot_write(vi_pipe, i, auto_attr->motion_over_shoot[i]);
        ot_ext_system_sharpen_auto_mot_under_shoot_write(vi_pipe, i, auto_attr->motion_under_shoot[i]);
        ot_ext_system_sharpen_shoot_sup_str_write(vi_pipe, i, auto_attr->shoot_sup_strength[i]);
        ot_ext_system_sharpen_detailctrl_write(vi_pipe, i, auto_attr->detail_ctrl[i]);
        ot_ext_system_sharpen_edge_filt_str_write(vi_pipe, i, auto_attr->edge_filt_strength[i]);
        ot_ext_system_sharpen_edge_filt_max_cap_write(vi_pipe, i, auto_attr->edge_filt_max_cap[i]);
        ot_ext_system_sharpen_r_gain_write(vi_pipe, i, auto_attr->r_gain[i]);
        ot_ext_system_sharpen_g_gain_write(vi_pipe, i, auto_attr->g_gain[i]);
        ot_ext_system_sharpen_b_gain_write(vi_pipe, i, auto_attr->b_gain[i]);
        ot_ext_system_sharpen_skin_gain_write(vi_pipe, i, auto_attr->skin_gain[i]);
        ot_ext_system_sharpen_shoot_sup_adj_write(vi_pipe, i, auto_attr->shoot_sup_adj[i]);
        ot_ext_system_sharpen_detailctrl_thr_write(vi_pipe, i, auto_attr->detail_ctrl_threshold[i]);
        ot_ext_system_sharpen_max_sharp_gain_write(vi_pipe, i, auto_attr->max_sharp_gain[i]);
    }
}

td_void isp_sharpen_manual_attr_write(ot_vi_pipe vi_pipe, const ot_isp_sharpen_manual_attr *manual_attr)
{
    td_u8 j;
    for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
        ot_ext_system_manual_sharpen_texture_str_write(vi_pipe, j, manual_attr->texture_strength[j]);
        ot_ext_system_manual_sharpen_edge_str_write(vi_pipe, j, manual_attr->edge_strength[j]);
        ot_ext_system_sharpen_manual_mot_texture_str_write(vi_pipe, j, manual_attr->motion_texture_strength[j]);
        ot_ext_system_sharpen_manual_mot_edge_str_write(vi_pipe, j, manual_attr->motion_edge_strength[j]);
    }
    for (j = 0; j < OT_ISP_SHARPEN_LUMA_NUM; j++) {
        ot_ext_system_manual_sharpen_luma_wgt_write(vi_pipe, j, manual_attr->luma_wgt[j]);
    }
    ot_ext_system_manual_sharpen_texture_freq_write(vi_pipe, manual_attr->texture_freq);
    ot_ext_system_manual_sharpen_edge_freq_write(vi_pipe, manual_attr->edge_freq);
    ot_ext_system_manual_sharpen_over_shoot_write(vi_pipe, manual_attr->over_shoot);
    ot_ext_system_manual_sharpen_under_shoot_write(vi_pipe, manual_attr->under_shoot);
    ot_ext_system_sharpen_manual_mot_texture_freq_write(vi_pipe, manual_attr->motion_texture_freq);
    ot_ext_system_sharpen_manual_mot_edge_freq_write(vi_pipe, manual_attr->motion_edge_freq);
    ot_ext_system_sharpen_manual_mot_over_shoot_write(vi_pipe, manual_attr->motion_over_shoot);
    ot_ext_system_sharpen_manual_mot_under_shoot_write(vi_pipe, manual_attr->motion_under_shoot);
    ot_ext_system_manual_sharpen_shoot_sup_str_write(vi_pipe, manual_attr->shoot_sup_strength);
    ot_ext_system_manual_sharpen_detailctrl_write(vi_pipe, manual_attr->detail_ctrl);
    ot_ext_system_manual_sharpen_edge_filt_str_write(vi_pipe, manual_attr->edge_filt_strength);
    ot_ext_system_manual_sharpen_edge_filt_max_cap_write(vi_pipe, manual_attr->edge_filt_max_cap);
    ot_ext_system_manual_sharpen_r_gain_write(vi_pipe, manual_attr->r_gain);
    ot_ext_system_manual_sharpen_g_gain_write(vi_pipe, manual_attr->g_gain);
    ot_ext_system_manual_sharpen_b_gain_write(vi_pipe, manual_attr->b_gain);
    ot_ext_system_manual_sharpen_skin_gain_write(vi_pipe, manual_attr->skin_gain);
    ot_ext_system_manual_sharpen_shoot_sup_adj_write(vi_pipe, manual_attr->shoot_sup_adj);
    ot_ext_system_manual_sharpen_max_sharp_gain_write(vi_pipe, manual_attr->max_sharp_gain);
    ot_ext_system_manual_sharpen_detailctrl_thr_write(vi_pipe, manual_attr->detail_ctrl_threshold);
}

td_void isp_sharpen_comm_attr_write(ot_vi_pipe vi_pipe, const ot_isp_sharpen_attr *shp_attr)
{
    ot_ext_system_manual_sharpen_en_write(vi_pipe, shp_attr->enable);
    ot_ext_system_sharpen_motion_en_write(vi_pipe, shp_attr->motion_en);
    ot_ext_system_sharpen_manu_mode_write(vi_pipe, shp_attr->op_type);
    ot_ext_system_manual_sharpen_skin_umax_write(vi_pipe, shp_attr->skin_umax);
    ot_ext_system_manual_sharpen_skin_vmax_write(vi_pipe, shp_attr->skin_vmax);
    ot_ext_system_manual_sharpen_skin_umin_write(vi_pipe, shp_attr->skin_umin);
    ot_ext_system_manual_sharpen_skin_vmin_write(vi_pipe, shp_attr->skin_vmin);
    ot_ext_system_sharpen_detail_map_write(vi_pipe, shp_attr->detail_map);
    ot_ext_system_sharpen_motion_thr0_write(vi_pipe, shp_attr->motion_threshold0);
    ot_ext_system_sharpen_motion_thr1_write(vi_pipe, shp_attr->motion_threshold1);
    ot_ext_system_sharpen_motion_gain0_write(vi_pipe, shp_attr->motion_gain0);
    ot_ext_system_sharpen_motion_gain1_write(vi_pipe, shp_attr->motion_gain1);
}

td_void isp_sharpen_auto_attr_read(ot_vi_pipe vi_pipe, ot_isp_sharpen_auto_attr *auto_attr)
{
    td_u8 i, j;
    td_u16 index;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
            index = i + j * OT_ISP_AUTO_ISO_NUM;
            auto_attr->texture_strength[j][i] = ot_ext_system_sharpen_texture_str_read(vi_pipe, index);
            auto_attr->edge_strength[j][i]    = ot_ext_system_sharpen_edge_str_read(vi_pipe, index);

            auto_attr->motion_texture_strength[j][i] = ot_ext_system_sharpen_auto_mot_texture_str_read(vi_pipe, index);
            auto_attr->motion_edge_strength[j][i]    = ot_ext_system_sharpen_auto_mot_edge_str_read(vi_pipe, index);
        }
        for (j = 0; j < OT_ISP_SHARPEN_LUMA_NUM; j++) {
            index = i + j * OT_ISP_AUTO_ISO_NUM;
            auto_attr->luma_wgt[j][i] = ot_ext_system_sharpen_luma_wgt_read(vi_pipe, index);
        }
        auto_attr->texture_freq[i]  = ot_ext_system_sharpen_texture_freq_read(vi_pipe, i);
        auto_attr->edge_freq[i]     = ot_ext_system_sharpen_edge_freq_read(vi_pipe, i);
        auto_attr->over_shoot[i]    = ot_ext_system_sharpen_over_shoot_read(vi_pipe, i);
        auto_attr->under_shoot[i]   = ot_ext_system_sharpen_under_shoot_read(vi_pipe, i);
        auto_attr->motion_texture_freq[i]  = ot_ext_system_sharpen_auto_mot_texture_freq_read(vi_pipe, i);
        auto_attr->motion_edge_freq[i]     = ot_ext_system_sharpen_auto_mot_edge_freq_read(vi_pipe, i);
        auto_attr->motion_over_shoot[i]    = ot_ext_system_sharpen_auto_mot_over_shoot_read(vi_pipe, i);
        auto_attr->motion_under_shoot[i]   = ot_ext_system_sharpen_auto_mot_under_shoot_read(vi_pipe, i);
        auto_attr->shoot_sup_strength[i] = ot_ext_system_sharpen_shoot_sup_str_read(vi_pipe, i);
        auto_attr->detail_ctrl[i]   = ot_ext_system_sharpen_detailctrl_read(vi_pipe, i);
        auto_attr->edge_filt_strength[i] = ot_ext_system_sharpen_edge_filt_str_read(vi_pipe, i);
        auto_attr->edge_filt_max_cap[i] = ot_ext_system_sharpen_edge_filt_max_cap_read(vi_pipe, i);
        auto_attr->r_gain[i]         = ot_ext_system_sharpen_r_gain_read(vi_pipe, i);
        auto_attr->g_gain[i]         = ot_ext_system_sharpen_g_gain_read(vi_pipe, i);
        auto_attr->b_gain[i]         = ot_ext_system_sharpen_b_gain_read(vi_pipe, i);
        auto_attr->skin_gain[i]      = ot_ext_system_sharpen_skin_gain_read(vi_pipe, i);
        auto_attr->shoot_sup_adj[i]  = ot_ext_system_sharpen_shoot_sup_adj_read(vi_pipe, i);
        auto_attr->max_sharp_gain[i] = ot_ext_system_sharpen_max_sharp_gain_read(vi_pipe, i);
        auto_attr->detail_ctrl_threshold[i] = ot_ext_system_sharpen_detailctrl_thr_read(vi_pipe, i);
    }
}

td_void isp_sharpen_manual_attr_read(ot_vi_pipe vi_pipe, ot_isp_sharpen_manual_attr *manual_attr)
{
    td_u8 j;
    for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
        manual_attr->texture_strength[j] = ot_ext_system_manual_sharpen_texture_str_read(vi_pipe, j);
        manual_attr->edge_strength[j]    = ot_ext_system_manual_sharpen_edge_str_read(vi_pipe, j);
    }
    for (j = 0; j < OT_ISP_SHARPEN_LUMA_NUM; j++) {
        manual_attr->luma_wgt[j] = ot_ext_system_manual_sharpen_luma_wgt_read(vi_pipe, j);
    }
    manual_attr->texture_freq = ot_ext_system_manual_sharpen_texture_freq_read(vi_pipe);
    manual_attr->edge_freq    = ot_ext_system_manual_sharpen_edge_freq_read(vi_pipe);
    manual_attr->over_shoot   = ot_ext_system_manual_sharpen_over_shoot_read(vi_pipe);
    manual_attr->under_shoot  = ot_ext_system_manual_sharpen_under_shoot_read(vi_pipe);
    for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
        manual_attr->motion_texture_strength[j] = ot_ext_system_sharpen_manual_mot_texture_str_read(vi_pipe, j);
        manual_attr->motion_edge_strength[j]    = ot_ext_system_sharpen_manual_mot_edge_str_read(vi_pipe, j);
    }
    manual_attr->motion_texture_freq = ot_ext_system_sharpen_manual_mot_texture_freq_read(vi_pipe);
    manual_attr->motion_edge_freq    = ot_ext_system_sharpen_manual_mot_edge_freq_read(vi_pipe);
    manual_attr->motion_over_shoot   = ot_ext_system_sharpen_manual_mot_over_shoot_read(vi_pipe);
    manual_attr->motion_under_shoot  = ot_ext_system_sharpen_manual_mot_under_shoot_read(vi_pipe);
    manual_attr->shoot_sup_strength = ot_ext_system_manual_sharpen_shoot_sup_str_read(vi_pipe);
    manual_attr->detail_ctrl   = ot_ext_system_manual_sharpen_detailctrl_read(vi_pipe);
    manual_attr->edge_filt_strength = ot_ext_system_manual_sharpen_edge_filt_str_read(vi_pipe);
    manual_attr->edge_filt_max_cap = ot_ext_system_manual_sharpen_edge_filt_max_cap_read(vi_pipe);
    manual_attr->r_gain        = ot_ext_system_manual_sharpen_r_gain_read(vi_pipe);
    manual_attr->g_gain        = ot_ext_system_manual_sharpen_g_gain_read(vi_pipe);
    manual_attr->b_gain        = ot_ext_system_manual_sharpen_b_gain_read(vi_pipe);
    manual_attr->skin_gain     = ot_ext_system_manual_sharpen_skin_gain_read(vi_pipe);
    manual_attr->shoot_sup_adj = ot_ext_system_manual_sharpen_shoot_sup_adj_read(vi_pipe);
    manual_attr->max_sharp_gain   = ot_ext_system_manual_sharpen_max_sharp_gain_read(vi_pipe);
    manual_attr->detail_ctrl_threshold  = ot_ext_system_manual_sharpen_detailctrl_thr_read(vi_pipe);
}
td_void isp_sharpen_comm_attr_read(ot_vi_pipe vi_pipe, ot_isp_sharpen_attr *shp_attr)
{
    shp_attr->enable         = ot_ext_system_manual_sharpen_en_read(vi_pipe);
    shp_attr->op_type    = (ot_op_mode)ot_ext_system_sharpen_manu_mode_read(vi_pipe);
    shp_attr->motion_en  = ot_ext_system_sharpen_motion_en_read(vi_pipe);
    shp_attr->detail_map = ot_ext_system_sharpen_detail_map_read(vi_pipe);
    shp_attr->motion_gain0 = ot_ext_system_sharpen_motion_gain0_read(vi_pipe);
    shp_attr->motion_gain1 = ot_ext_system_sharpen_motion_gain1_read(vi_pipe);
    shp_attr->motion_threshold0 = ot_ext_system_sharpen_motion_thr0_read(vi_pipe);
    shp_attr->motion_threshold1 = ot_ext_system_sharpen_motion_thr1_read(vi_pipe);
    shp_attr->skin_umax = ot_ext_system_manual_sharpen_skin_umax_read(vi_pipe);
    shp_attr->skin_vmax = ot_ext_system_manual_sharpen_skin_vmax_read(vi_pipe);
    shp_attr->skin_umin = ot_ext_system_manual_sharpen_skin_umin_read(vi_pipe);
    shp_attr->skin_vmin = ot_ext_system_manual_sharpen_skin_vmin_read(vi_pipe);
}

static td_void isp_drc_comm_attr_write(ot_vi_pipe vi_pipe, const ot_isp_drc_attr *drc_attr)
{
    ot_ext_system_drc_enable_write(vi_pipe, drc_attr->enable);
    ot_ext_system_drc_curve_select_write(vi_pipe, drc_attr->curve_select);
    ot_ext_system_drc_manual_mode_write(vi_pipe, drc_attr->op_type);
    ot_ext_system_drc_auto_strength_write(vi_pipe, drc_attr->auto_attr.strength);
    ot_ext_system_drc_auto_strength_max_write(vi_pipe, drc_attr->auto_attr.strength_max);
    ot_ext_system_drc_auto_strength_min_write(vi_pipe, drc_attr->auto_attr.strength_min);
    ot_ext_system_drc_manual_strength_write(vi_pipe, drc_attr->manual_attr.strength);

    ot_ext_system_drc_pd_strength_write(vi_pipe, drc_attr->purple_reduction_strength);

    ot_ext_system_drc_asymmetry_write(vi_pipe, drc_attr->asymmetry_curve.asymmetry);
    ot_ext_system_drc_secondpole_write(vi_pipe, drc_attr->asymmetry_curve.second_pole);
    ot_ext_system_drc_stretch_write(vi_pipe, drc_attr->asymmetry_curve.stretch);
    ot_ext_system_drc_compress_write(vi_pipe, drc_attr->asymmetry_curve.compress);

    ot_ext_system_drc_bright_gain_lmt_write(vi_pipe, drc_attr->bright_gain_limit);
    ot_ext_system_drc_bright_gain_lmt_step_write(vi_pipe, drc_attr->bright_gain_limit_step);
    ot_ext_system_drc_dark_gain_lmt_y_write(vi_pipe, drc_attr->dark_gain_limit_luma);
    ot_ext_system_drc_dark_gain_lmt_c_write(vi_pipe, drc_attr->dark_gain_limit_chroma);

    ot_ext_system_drc_contrast_ctrl_write(vi_pipe, drc_attr->contrast_ctrl);
    ot_ext_system_drc_high_sat_ctrl_write(vi_pipe, drc_attr->high_saturation_color_ctrl);
    ot_ext_system_drc_global_color_ctrl_write(vi_pipe, drc_attr->global_color_ctrl);
    ot_ext_system_drc_shoot_reduction_en_write(vi_pipe, drc_attr->shoot_reduction_en);
}

static td_void isp_drc_flt_attr_write(ot_vi_pipe vi_pipe, const ot_isp_drc_attr *drc_attr)
{
    ot_ext_system_drc_grad_rev_max_write(vi_pipe, drc_attr->rim_reduction_strength);
    ot_ext_system_drc_grad_rev_thr_write(vi_pipe, drc_attr->rim_reduction_threshold);

    ot_ext_system_drc_spa_flt_coef_write(vi_pipe, drc_attr->spatial_filter_coef);
    ot_ext_system_drc_rng_flt_coef_write(vi_pipe, drc_attr->range_filter_coef);
    ot_ext_system_drc_detail_adjust_coef_write(vi_pipe, drc_attr->detail_adjust_coef);
}


static td_void isp_drc_lut_attr_write(ot_vi_pipe vi_pipe, const ot_isp_drc_attr *drc_attr)
{
    td_u8 i;

    for (i = 0; i < OT_ISP_DRC_TM_NODE_NUM; i++) {
        ot_ext_system_drc_tm_lut_write(vi_pipe, i, drc_attr->tone_mapping_value[i]);
    }

    for (i = 0; i < OT_ISP_DRC_CC_NODE_NUM; i++) {
        ot_ext_system_drc_cc_lut_write(vi_pipe, i, drc_attr->color_correction_lut[i]);
    }

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        ot_ext_system_drc_mixing_bright_lut_write(vi_pipe, i, drc_attr->local_mixing_bright[i]);
        ot_ext_system_drc_mixing_dark_lut_write(vi_pipe, i, drc_attr->local_mixing_dark[i]);
    }
}

td_void isp_drc_attr_write(ot_vi_pipe vi_pipe, const ot_isp_drc_attr *drc_attr)
{
    isp_drc_comm_attr_write(vi_pipe, drc_attr);
    isp_drc_flt_attr_write(vi_pipe, drc_attr);
    isp_drc_lut_attr_write(vi_pipe, drc_attr);
}

td_void isp_drc_blend_write(ot_vi_pipe vi_pipe, const isp_drc_blend *drc_blend)
{
    td_u8 i;

    ot_ext_system_drc_tone_mapping_wgt_x_write(vi_pipe, drc_blend->tone_mapping_wgt_x);
    ot_ext_system_drc_detail_adjust_coef_x_write(vi_pipe, drc_blend->detail_adjust_coef_x);

    ot_ext_system_drc_flt_bld_l_max_write(vi_pipe, drc_blend->blend_luma_max);
    ot_ext_system_drc_flt_bld_l_bright_min_write(vi_pipe, drc_blend->blend_luma_bright_min);
    ot_ext_system_drc_flt_bld_l_bright_thr_write(vi_pipe, drc_blend->blend_luma_bright_threshold);
    ot_ext_system_drc_flt_bld_l_dark_min_write(vi_pipe, drc_blend->blend_luma_dark_min);
    ot_ext_system_drc_flt_bld_l_dark_thr_write(vi_pipe, drc_blend->blend_luma_dark_threshold);

    ot_ext_system_drc_flt_bld_d_max_write(vi_pipe, drc_blend->blend_detail_max);
    ot_ext_system_drc_flt_bld_d_bright_min_write(vi_pipe, drc_blend->blend_detail_bright_min);
    ot_ext_system_drc_flt_bld_d_bright_thr_write(vi_pipe, drc_blend->blend_detail_bright_threshold);
    ot_ext_system_drc_flt_bld_d_dark_min_write(vi_pipe, drc_blend->blend_detail_dark_min);
    ot_ext_system_drc_flt_bld_d_dark_thr_write(vi_pipe, drc_blend->blend_detail_dark_threshold);
    ot_ext_system_drc_detail_gain_bld_write(vi_pipe, drc_blend->detail_adjust_coef_blend);

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        ot_ext_system_drc_mixing_bright_x_lut_write(vi_pipe, i, drc_blend->local_mixing_bright_x[i]);
        ot_ext_system_drc_mixing_dark_x_lut_write(vi_pipe, i, drc_blend->local_mixing_dark_x[i]);
    }
}

static td_void isp_drc_comm_attr_read(ot_vi_pipe vi_pipe, ot_isp_drc_attr *drc_attr)
{
    drc_attr->enable               = ot_ext_system_drc_enable_read(vi_pipe);
    drc_attr->curve_select         = ot_ext_system_drc_curve_select_read(vi_pipe);
    drc_attr->op_type              = (ot_op_mode)ot_ext_system_drc_manual_mode_read(vi_pipe);
    drc_attr->auto_attr.strength    = ot_ext_system_drc_auto_strength_read(vi_pipe);
    drc_attr->auto_attr.strength_max = ot_ext_system_drc_auto_strength_max_read(vi_pipe);
    drc_attr->auto_attr.strength_min = ot_ext_system_drc_auto_strength_min_read(vi_pipe);
    drc_attr->manual_attr.strength  = ot_ext_system_drc_manual_strength_read(vi_pipe);

    drc_attr->purple_reduction_strength  = ot_ext_system_drc_pd_strength_read(vi_pipe);

    drc_attr->asymmetry_curve.asymmetry  = ot_ext_system_drc_asymmetry_read(vi_pipe);
    drc_attr->asymmetry_curve.second_pole = ot_ext_system_drc_secondpole_read(vi_pipe);
    drc_attr->asymmetry_curve.stretch    = ot_ext_system_drc_stretch_read(vi_pipe);
    drc_attr->asymmetry_curve.compress   = ot_ext_system_drc_compress_read(vi_pipe);

    drc_attr->bright_gain_limit      = ot_ext_system_drc_bright_gain_lmt_read(vi_pipe);
    drc_attr->bright_gain_limit_step = ot_ext_system_drc_bright_gain_lmt_step_read(vi_pipe);
    drc_attr->dark_gain_limit_luma = ot_ext_system_drc_dark_gain_lmt_y_read(vi_pipe);
    drc_attr->dark_gain_limit_chroma = ot_ext_system_drc_dark_gain_lmt_c_read(vi_pipe);

    drc_attr->contrast_ctrl  = ot_ext_system_drc_contrast_ctrl_read(vi_pipe);
    drc_attr->high_saturation_color_ctrl = ot_ext_system_drc_high_sat_ctrl_read(vi_pipe);
    drc_attr->global_color_ctrl = ot_ext_system_drc_global_color_ctrl_read(vi_pipe);
    drc_attr->shoot_reduction_en = ot_ext_system_drc_shoot_reduction_en_read(vi_pipe);
}

static td_void isp_drc_flt_attr_read(ot_vi_pipe vi_pipe, ot_isp_drc_attr *drc_attr)
{
    drc_attr->rim_reduction_strength = ot_ext_system_drc_grad_rev_max_read(vi_pipe);
    drc_attr->rim_reduction_threshold = ot_ext_system_drc_grad_rev_thr_read(vi_pipe);

    drc_attr->spatial_filter_coef = ot_ext_system_drc_spa_flt_coef_read(vi_pipe);
    drc_attr->range_filter_coef = ot_ext_system_drc_rng_flt_coef_read(vi_pipe);
    drc_attr->detail_adjust_coef = ot_ext_system_drc_detail_adjust_coef_read(vi_pipe);
}

static td_void isp_drc_lut_attr_read(ot_vi_pipe vi_pipe, ot_isp_drc_attr *drc_attr)
{
    td_u8 i;

    for (i = 0; i < OT_ISP_DRC_CC_NODE_NUM; i++) {
        drc_attr->color_correction_lut[i] = ot_ext_system_drc_cc_lut_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_DRC_TM_NODE_NUM; i++) {
        drc_attr->tone_mapping_value[i] = ot_ext_system_drc_tm_lut_read(vi_pipe, i);
        if (i > 0 && drc_attr->tone_mapping_value[i] < drc_attr->tone_mapping_value[i - 1]) {
            drc_attr->tone_mapping_value[i] = drc_attr->tone_mapping_value[i - 1];
            ot_ext_system_drc_tm_lut_write(vi_pipe, i, drc_attr->tone_mapping_value[i]);
        }
    }

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        drc_attr->local_mixing_bright[i] = ot_ext_system_drc_mixing_bright_lut_read(vi_pipe, i);
        drc_attr->local_mixing_dark[i] = ot_ext_system_drc_mixing_dark_lut_read(vi_pipe, i);
    }
}

td_void isp_drc_attr_read(ot_vi_pipe vi_pipe, ot_isp_drc_attr *drc_attr)
{
    isp_drc_comm_attr_read(vi_pipe, drc_attr);
    isp_drc_flt_attr_read(vi_pipe, drc_attr);
    isp_drc_lut_attr_read(vi_pipe, drc_attr);
}

td_void isp_drc_blend_read(ot_vi_pipe vi_pipe, isp_drc_blend *drc_blend)
{
    td_u8 i;

    drc_blend->tone_mapping_wgt_x = ot_ext_system_drc_tone_mapping_wgt_x_read(vi_pipe);
    drc_blend->detail_adjust_coef_x = ot_ext_system_drc_detail_adjust_coef_x_read(vi_pipe);

    drc_blend->blend_luma_max = ot_ext_system_drc_flt_bld_l_max_read(vi_pipe);
    drc_blend->blend_luma_bright_min = ot_ext_system_drc_flt_bld_l_bright_min_read(vi_pipe);
    drc_blend->blend_luma_bright_threshold = ot_ext_system_drc_flt_bld_l_bright_thr_read(vi_pipe);
    drc_blend->blend_luma_dark_min = ot_ext_system_drc_flt_bld_l_dark_min_read(vi_pipe);
    drc_blend->blend_luma_dark_threshold = ot_ext_system_drc_flt_bld_l_dark_thr_read(vi_pipe);

    drc_blend->blend_detail_max = ot_ext_system_drc_flt_bld_d_max_read(vi_pipe);
    drc_blend->blend_detail_bright_min = ot_ext_system_drc_flt_bld_d_bright_min_read(vi_pipe);
    drc_blend->blend_detail_bright_threshold = ot_ext_system_drc_flt_bld_d_bright_thr_read(vi_pipe);
    drc_blend->blend_detail_dark_min = ot_ext_system_drc_flt_bld_d_dark_min_read(vi_pipe);
    drc_blend->blend_detail_dark_threshold = ot_ext_system_drc_flt_bld_d_dark_thr_read(vi_pipe);
    drc_blend->detail_adjust_coef_blend = ot_ext_system_drc_detail_gain_bld_read(vi_pipe);

    for (i = 0; i < OT_ISP_DRC_LMIX_NODE_NUM; i++) {
        drc_blend->local_mixing_bright_x[i] = ot_ext_system_drc_mixing_bright_x_lut_read(vi_pipe, i);
        drc_blend->local_mixing_dark_x[i] = ot_ext_system_drc_mixing_dark_x_lut_read(vi_pipe, i);
    }
}

static td_void isp_ldci_auto_attr_write(ot_vi_pipe vi_pipe, const ot_isp_ldci_attr *ldci_attr)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_ldci_he_pos_wgt_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.wgt);
        ot_ext_system_ldci_he_pos_sigma_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.sigma);
        ot_ext_system_ldci_he_pos_mean_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.mean);
        ot_ext_system_ldci_he_neg_wgt_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.wgt);
        ot_ext_system_ldci_he_neg_sigma_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.sigma);
        ot_ext_system_ldci_he_neg_mean_write(vi_pipe, i, ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.mean);
        ot_ext_system_ldci_blc_ctrl_write(vi_pipe, i, ldci_attr->auto_attr.blc_ctrl[i]);
    }
}

static td_void isp_ldci_manual_attr_write(ot_vi_pipe vi_pipe, const ot_isp_ldci_attr *ldci_attr)
{
    ot_ext_system_ldci_manu_he_poswgt_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_pos_wgt.wgt);
    ot_ext_system_ldci_manu_he_pos_sigma_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_pos_wgt.sigma);
    ot_ext_system_ldci_manu_he_pos_mean_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_pos_wgt.mean);
    ot_ext_system_ldci_manu_he_negwgt_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_neg_wgt.wgt);
    ot_ext_system_ldci_manu_he_negsigma_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_neg_wgt.sigma);
    ot_ext_system_ldci_manu_he_negmean_write(vi_pipe, ldci_attr->manual_attr.he_wgt.he_neg_wgt.mean);
    ot_ext_system_ldci_manu_blc_ctrl_write(vi_pipe, ldci_attr->manual_attr.blc_ctrl);
}

td_void isp_ldci_attr_write(ot_vi_pipe vi_pipe, const ot_isp_ldci_attr *ldci_attr)
{
    ot_ext_system_ldci_enable_write(vi_pipe, ldci_attr->enable);
    ot_ext_system_ldci_gauss_lpfsigma_write(vi_pipe, ldci_attr->gauss_lpf_sigma);
    ot_ext_system_ldci_manu_mode_write(vi_pipe, ldci_attr->op_type);
    ot_ext_system_ldci_tpr_incr_coef_write(vi_pipe, ldci_attr->tpr_incr_coef);
    ot_ext_system_ldci_tpr_decr_coef_write(vi_pipe, ldci_attr->tpr_decr_coef);

    isp_ldci_auto_attr_write(vi_pipe, ldci_attr);
    isp_ldci_manual_attr_write(vi_pipe, ldci_attr);
}

static td_void isp_ldci_auto_attr_read(ot_vi_pipe vi_pipe, ot_isp_ldci_attr *ldci_attr)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.wgt   = ot_ext_system_ldci_he_pos_wgt_read(vi_pipe, i);
        ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.sigma = ot_ext_system_ldci_he_pos_sigma_read(vi_pipe, i);
        ldci_attr->auto_attr.he_wgt[i].he_pos_wgt.mean  = ot_ext_system_ldci_he_pos_mean_read(vi_pipe, i);
        ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.wgt   = ot_ext_system_ldci_he_neg_wgt_read(vi_pipe, i);
        ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.sigma = ot_ext_system_ldci_he_neg_sigma_read(vi_pipe, i);
        ldci_attr->auto_attr.he_wgt[i].he_neg_wgt.mean  = ot_ext_system_ldci_he_neg_mean_read(vi_pipe, i);
        ldci_attr->auto_attr.blc_ctrl[i]                = ot_ext_system_ldci_blc_ctrl_read(vi_pipe, i);
    }
}

static td_void isp_ldci_manual_attr_read(ot_vi_pipe vi_pipe, ot_isp_ldci_attr *ldci_attr)
{
    ldci_attr->manual_attr.he_wgt.he_pos_wgt.wgt   = ot_ext_system_ldci_manu_he_pos_wgt_read(vi_pipe);
    ldci_attr->manual_attr.he_wgt.he_pos_wgt.sigma = ot_ext_system_ldci_manu_he_pos_sigma_read(vi_pipe);
    ldci_attr->manual_attr.he_wgt.he_pos_wgt.mean  = ot_ext_system_ldci_manu_he_pos_mean_read(vi_pipe);
    ldci_attr->manual_attr.he_wgt.he_neg_wgt.wgt   = ot_ext_system_ldci_manu_he_neg_wgt_read(vi_pipe);
    ldci_attr->manual_attr.he_wgt.he_neg_wgt.sigma = ot_ext_system_ldci_manu_he_neg_sigma_read(vi_pipe);
    ldci_attr->manual_attr.he_wgt.he_neg_wgt.mean  = ot_ext_system_ldci_manu_he_neg_mean_read(vi_pipe);
    ldci_attr->manual_attr.blc_ctrl                = ot_ext_system_ldci_manu_blc_ctrl_read(vi_pipe);
}

td_void isp_ldci_attr_read(ot_vi_pipe vi_pipe, ot_isp_ldci_attr *ldci_attr)
{
    ldci_attr->enable              = ot_ext_system_ldci_enable_read(vi_pipe);
    ldci_attr->gauss_lpf_sigma = ot_ext_system_ldci_gauss_lpfsigma_read(vi_pipe);
    ldci_attr->op_type         = (ot_op_mode)ot_ext_system_ldci_manu_mode_read(vi_pipe);
    ldci_attr->tpr_incr_coef   = ot_ext_system_ldci_tpr_incr_coef_read(vi_pipe);
    ldci_attr->tpr_decr_coef   = ot_ext_system_ldci_tpr_decr_coef_read(vi_pipe);

    isp_ldci_auto_attr_read(vi_pipe, ldci_attr);
    isp_ldci_manual_attr_read(vi_pipe, ldci_attr);
}

td_void isp_nr_comm_attr_write(ot_vi_pipe vi_pipe, const ot_isp_nr_attr *nr_attr)
{
    td_u8 i;
    ot_ext_system_bayernr_enable_write(vi_pipe, nr_attr->enable);
    ot_ext_system_bayernr_op_type_write(vi_pipe, nr_attr->op_type);
    ot_ext_system_bayernr_tnr_enable_write(vi_pipe, nr_attr->tnr_en);
    ot_ext_system_bayernr_lsc_nr_enable_write(vi_pipe, nr_attr->lsc_nr_en);
    ot_ext_system_bayernr_lsc_ratio1_write(vi_pipe, nr_attr->lsc_ratio1);
    for (i = 0; i < OT_ISP_BAYERNR_LUT_LENGTH; i++) {
        ot_ext_system_bayernr_coring_ratio_lut_write(vi_pipe, i, nr_attr->coring_ratio[i]);
    }

    for (i = 0; i < OT_ISP_BAYERNR_LUT_LENGTH1; i++) {
        ot_ext_system_bayernr_mix_gain_lut_write(vi_pipe, i, nr_attr->mix_gain[i]);
    }
}

td_void isp_nr_snr_attr_write(ot_vi_pipe pipe, const ot_isp_nr_snr_attr *snr_cfg)
{
    td_u8 i;
    const ot_isp_nr_snr_auto_attr *snr_auto = &snr_cfg->snr_attr.snr_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_bayernr_auto_sfm0_coarse_strength_r_lut_write(pipe, i, snr_auto->sfm0_coarse_strength[0][i]);
        ot_ext_system_bayernr_auto_sfm0_coarse_strength_gr_lut_write(pipe, i, snr_auto->sfm0_coarse_strength[1][i]);
        ot_ext_system_bayernr_auto_sfm0_coarse_strength_gb_lut_write(pipe, i, snr_auto->sfm0_coarse_strength[0x2][i]);
        ot_ext_system_bayernr_auto_sfm0_coarse_strength_b_lut_write(pipe, i, snr_auto->sfm0_coarse_strength[0x3][i]);
        ot_ext_system_bayernr_auto_sfm6_strength_lut_write(pipe, i, snr_auto->sfm6_strength[i]);
        ot_ext_system_bayernr_auto_sfm7_strength_lut_write(pipe, i, snr_auto->sfm7_strength[i]);
        ot_ext_system_bayernr_auto_sth_lut_write(pipe, i, snr_auto->sth[i]);
        ot_ext_system_bayernr_auto_sfm1_adp_strength_lut_write(pipe, i, snr_auto->sfm1_adp_strength[i]);
        ot_ext_system_bayernr_auto_sfm1_strength_lut_write(pipe, i, snr_auto->sfm1_strength[i]);
        ot_ext_system_bayernr_auto_sfm0_detail_prot_lut_write(pipe, i, snr_auto->sfm0_detail_prot[i]);
        ot_ext_system_bayernr_auto_fine_strength_lut_write(pipe, i, snr_auto->fine_strength[i]);
        ot_ext_system_bayernr_auto_coring_wgt_lut_write(pipe, i, snr_auto->coring_wgt[i]);
        ot_ext_system_bayernr_auto_coring_mot_ratio_lut_write(pipe, i, snr_auto->coring_mot_ratio[i]);
    }

    ot_ext_system_bayernr_manual_sfm0_coarse_strength_r_write(pipe,
                                                              snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0]);
    ot_ext_system_bayernr_manual_sfm0_coarse_strength_gr_write(pipe,
                                                               snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[1]);
    ot_ext_system_bayernr_manual_sfm0_coarse_strength_gb_write(pipe,
                                                               snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0x2]);
    ot_ext_system_bayernr_manual_sfm0_coarse_strength_b_write(pipe,
                                                              snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0x3]);
    ot_ext_system_bayernr_manual_sfm6_strength_write(pipe, snr_cfg->snr_attr.snr_manual.sfm6_strength);
    ot_ext_system_bayernr_manual_sfm7_strength_write(pipe, snr_cfg->snr_attr.snr_manual.sfm7_strength);
    ot_ext_system_bayernr_manual_sth_write(pipe, snr_cfg->snr_attr.snr_manual.sth);
    ot_ext_system_bayernr_manual_sfm1_adp_strength_write(pipe, snr_cfg->snr_attr.snr_manual.sfm1_adp_strength);
    ot_ext_system_bayernr_manual_sfm1_strength_write(pipe, snr_cfg->snr_attr.snr_manual.sfm1_strength);
    ot_ext_system_bayernr_manual_sfm0_detail_prot_write(pipe, snr_cfg->snr_attr.snr_manual.sfm0_detail_prot);
    ot_ext_system_bayernr_manual_fine_strength_write(pipe, snr_cfg->snr_attr.snr_manual.fine_strength);
    ot_ext_system_bayernr_manual_coring_wgt_write(pipe, snr_cfg->snr_attr.snr_manual.coring_wgt);
    ot_ext_system_bayernr_manual_coring_mot_ratio_write(pipe, snr_cfg->snr_attr.snr_manual.coring_mot_ratio);
}


td_void isp_nr_tnr_attr_write(ot_vi_pipe pipe, const ot_isp_nr_tnr_attr *tnr_cfg)
{
    td_u8 i;
    const ot_isp_nr_tnr_auto_attr *tnr_auto = &tnr_cfg->tnr_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_bayernr_auto_md_mode_lut_write(pipe, i, tnr_auto->md_mode[i]);
        ot_ext_system_bayernr_auto_md_size_ratio_lut_write(pipe, i, tnr_auto->md_size_ratio[i]);
        ot_ext_system_bayernr_auto_md_anti_flicker_strength_lut_write(pipe, i, tnr_auto->md_anti_flicker_strength[i]);
        ot_ext_system_bayernr_auto_md_static_ratio_lut_write(pipe, i, tnr_auto->md_static_ratio[i]);
        ot_ext_system_bayernr_auto_md_motion_ratio_lut_write(pipe, i, tnr_auto->md_motion_ratio[i]);
        ot_ext_system_bayernr_auto_md_static_fine_strength_lut_write(pipe, i, tnr_auto->md_static_fine_strength[i]);
        ot_ext_system_bayernr_auto_tfs_lut_write(pipe, i, tnr_auto->tfs[i]);
        ot_ext_system_bayernr_auto_user_define_md_lut_write(pipe, i, tnr_auto->user_define_md[i]);
        ot_ext_system_bayernr_auto_user_define_bri_thresh_lut_write(pipe, i, tnr_auto->user_define_slope[i]);
        ot_ext_system_bayernr_auto_user_define_dark_thresh_lut_write(pipe, i, tnr_auto->user_define_dark_thresh[i]);
        ot_ext_system_bayernr_auto_user_define_color_thresh_lut_write(pipe, i, tnr_auto->user_define_color_thresh[i]);
        ot_ext_system_bayernr_auto_sfr_r_lut_write(pipe, i, tnr_auto->sfr_r[i]);
        ot_ext_system_bayernr_auto_sfr_g_lut_write(pipe, i, tnr_auto->sfr_g[i]);
        ot_ext_system_bayernr_auto_sfr_b_lut_write(pipe, i, tnr_auto->sfr_b[i]);
    }

    ot_ext_system_bayernr_manual_md_mode_write(pipe, tnr_cfg->tnr_manual.md_mode);
    ot_ext_system_bayernr_manual_md_size_ratio_write(pipe, tnr_cfg->tnr_manual.md_size_ratio);
    ot_ext_system_bayernr_manual_md_anti_flicker_strength_write(pipe, tnr_cfg->tnr_manual.md_anti_flicker_strength);
    ot_ext_system_bayernr_manual_md_static_ratio_write(pipe, tnr_cfg->tnr_manual.md_static_ratio);
    ot_ext_system_bayernr_manual_md_motion_ratio_write(pipe, tnr_cfg->tnr_manual.md_motion_ratio);
    ot_ext_system_bayernr_manual_md_static_fine_strength_write(pipe, tnr_cfg->tnr_manual.md_static_fine_strength);
    ot_ext_system_bayernr_manual_tfs_write(pipe, tnr_cfg->tnr_manual.tfs);
    ot_ext_system_bayernr_manual_user_define_md_write(pipe, tnr_cfg->tnr_manual.user_define_md);
    ot_ext_system_bayernr_manual_user_define_bri_thresh_write(pipe, tnr_cfg->tnr_manual.user_define_slope);
    ot_ext_system_bayernr_manual_user_define_dark_thresh_write(pipe, tnr_cfg->tnr_manual.user_define_dark_thresh);
    ot_ext_system_bayernr_manual_user_define_color_thresh_write(pipe, tnr_cfg->tnr_manual.user_define_color_thresh);
    ot_ext_system_bayernr_manual_sfr_r_write(pipe, tnr_cfg->tnr_manual.sfr_r);
    ot_ext_system_bayernr_manual_sfr_g_write(pipe, tnr_cfg->tnr_manual.sfr_g);
    ot_ext_system_bayernr_manual_sfr_b_write(pipe, tnr_cfg->tnr_manual.sfr_b);
}

td_void isp_nr_wdr_attr_write(ot_vi_pipe vi_pipe, const ot_isp_nr_wdr_attr *wdr_cfg)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        ot_ext_system_bayernr_snr_sfm0_wdr_strength_lut_write(vi_pipe, i, wdr_cfg->snr_sfm0_wdr_strength[i]);
        ot_ext_system_bayernr_snr_sfm0_fusion_strength_lut_write(vi_pipe, i, wdr_cfg->snr_sfm0_fusion_strength[i]);
        ot_ext_system_bayernr_snr_wdr_sfm6_strength_lut_write(vi_pipe, i, wdr_cfg->snr_wdr_sfm6_strength[i]);
        ot_ext_system_bayernr_snr_fusion_sfm6_strength_lut_write(vi_pipe, i, wdr_cfg->snr_fusion_sfm6_strength[i]);
        ot_ext_system_bayernr_snr_wdr_sfm7_strength_lut_write(vi_pipe, i, wdr_cfg->snr_wdr_sfm7_strength[i]);
        ot_ext_system_bayernr_snr_fusion_sfm7_strength_lut_write(vi_pipe, i, wdr_cfg->snr_fusion_sfm7_strength[i]);
        ot_ext_system_bayernr_md_wdr_strength_lut_write(vi_pipe, i, wdr_cfg->md_wdr_strength[i]);
        ot_ext_system_bayernr_md_fusion_strength_lut_write(vi_pipe, i, wdr_cfg->md_fusion_strength[i]);
    }
}

td_void isp_nr_ynet_attr_write(ot_vi_pipe vi_pipe, const ot_isp_nr_ynet_attr *ynet_cfg)
{
    ot_ext_system_bayernr_denoise_y_alpha_write(vi_pipe, ynet_cfg->denoise_y_alpha);

    td_u8 i;
    for (i = 0; i < OT_ISP_BAYERNR_DENOISE_Y_LUT_NUM; i++) {
        ot_ext_system_bayernr_denoise_y_fg_str_lut_write(vi_pipe, i, ynet_cfg->denoise_y_fg_str_lut[i]);
        ot_ext_system_bayernr_denoise_y_bg_str_lut_write(vi_pipe, i, ynet_cfg->denoise_y_bg_str_lut[i]);
    }
}

td_void isp_nr_dering_attr_write(ot_vi_pipe vi_pipe, const ot_isp_nr_dering_attr *dering_cfg)
{
    td_u8 i;
    const ot_isp_nr_dering_auto_attr *dering_auto = &dering_cfg->dering_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_bayernr_auto_dering_strength_lut_write(vi_pipe, i, dering_auto->dering_strength[i]);
        ot_ext_system_bayernr_auto_dering_thresh_lut_write(vi_pipe, i, dering_auto->dering_thresh[i]);
        ot_ext_system_bayernr_auto_dering_static_strength_lut_write(vi_pipe, i, dering_auto->dering_static_strength[i]);
        ot_ext_system_bayernr_auto_dering_motion_strength_lut_write(vi_pipe, i, dering_auto->dering_motion_strength[i]);
    }

    ot_ext_system_bayernr_manual_dering_strength_write(vi_pipe, dering_cfg->dering_manual.dering_strength);
    ot_ext_system_bayernr_manual_dering_thresh_write(vi_pipe, dering_cfg->dering_manual.dering_thresh);
    ot_ext_system_bayernr_manual_dering_static_strength_write(vi_pipe,
        dering_cfg->dering_manual.dering_static_strength);
    ot_ext_system_bayernr_manual_dering_motion_strength_write(vi_pipe,
        dering_cfg->dering_manual.dering_motion_strength);
}

td_void isp_nr_comm_attr_read(ot_vi_pipe vi_pipe, ot_isp_nr_attr *nr_attr)
{
    td_u8 i;
    nr_attr->enable         = ot_ext_system_bayernr_enable_read(vi_pipe);
    nr_attr->op_type    = ot_ext_system_bayernr_op_type_read(vi_pipe);
    nr_attr->tnr_en     = ot_ext_system_bayernr_tnr_enable_read(vi_pipe);
    nr_attr->lsc_nr_en  = ot_ext_system_bayernr_lsc_nr_enable_read(vi_pipe);
    nr_attr->lsc_ratio1 = ot_ext_system_bayernr_lsc_ratio1_read(vi_pipe);
    for (i = 0; i < OT_ISP_BAYERNR_LUT_LENGTH; i++) {
        nr_attr->coring_ratio[i] = ot_ext_system_bayernr_coring_ratio_lut_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_BAYERNR_LUT_LENGTH1; i++) {
        nr_attr->mix_gain[i] = ot_ext_system_bayernr_mix_gain_lut_read(vi_pipe, i);
    }
}

td_void isp_nr_snr_attr_read(ot_vi_pipe pipe, ot_isp_nr_snr_attr *snr_cfg)
{
    td_u8 i;
    ot_isp_nr_snr_auto_attr *snr_auto = &snr_cfg->snr_attr.snr_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        snr_auto->sfm0_coarse_strength[0][i] = ot_ext_system_bayernr_auto_sfm0_coarse_strength_r_lut_read(pipe, i);
        snr_auto->sfm0_coarse_strength[1][i] = ot_ext_system_bayernr_auto_sfm0_coarse_strength_gr_lut_read(pipe, i);
        snr_auto->sfm0_coarse_strength[0x2][i] = ot_ext_system_bayernr_auto_sfm0_coarse_strength_gb_lut_read(pipe, i);
        snr_auto->sfm0_coarse_strength[0x3][i] = ot_ext_system_bayernr_auto_sfm0_coarse_strength_b_lut_read(pipe, i);
        snr_auto->sfm6_strength[i] = ot_ext_system_bayernr_auto_sfm6_strength_lut_read(pipe, i);
        snr_auto->sfm7_strength[i] = ot_ext_system_bayernr_auto_sfm7_strength_lut_read(pipe, i);
        snr_auto->sth[i] = ot_ext_system_bayernr_auto_sth_lut_read(pipe, i);
        snr_auto->sfm1_adp_strength[i] = ot_ext_system_bayernr_auto_sfm1_adp_strength_lut_read(pipe, i);
        snr_auto->sfm1_strength[i] = ot_ext_system_bayernr_auto_sfm1_strength_lut_read(pipe, i);
        snr_auto->sfm0_detail_prot[i] = ot_ext_system_bayernr_auto_sfm0_detail_prot_lut_read(pipe, i);
        snr_auto->fine_strength[i] = ot_ext_system_bayernr_auto_fine_strength_lut_read(pipe, i);
        snr_auto->coring_wgt[i] = ot_ext_system_bayernr_auto_coring_wgt_lut_read(pipe, i);
        snr_auto->coring_mot_ratio[i] = ot_ext_system_bayernr_auto_coring_mot_ratio_lut_read(pipe, i);
    }

    snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0] =
            ot_ext_system_bayernr_manual_sfm0_coarse_strength_r_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[1] =
            ot_ext_system_bayernr_manual_sfm0_coarse_strength_gr_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0x2] =
            ot_ext_system_bayernr_manual_sfm0_coarse_strength_gb_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm0_coarse_strength[0x3] =
            ot_ext_system_bayernr_manual_sfm0_coarse_strength_b_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm6_strength = ot_ext_system_bayernr_manual_sfm6_strength_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm7_strength = ot_ext_system_bayernr_manual_sfm7_strength_read(pipe);
    snr_cfg->snr_attr.snr_manual.sth = ot_ext_system_bayernr_manual_sth_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm1_adp_strength = ot_ext_system_bayernr_manual_sfm1_adp_strength_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm1_strength = ot_ext_system_bayernr_manual_sfm1_strength_read(pipe);
    snr_cfg->snr_attr.snr_manual.sfm0_detail_prot = ot_ext_system_bayernr_manual_sfm0_detail_prot_read(pipe);
    snr_cfg->snr_attr.snr_manual.fine_strength = ot_ext_system_bayernr_manual_fine_strength_read(pipe);
    snr_cfg->snr_attr.snr_manual.coring_wgt = ot_ext_system_bayernr_manual_coring_wgt_read(pipe);
    snr_cfg->snr_attr.snr_manual.coring_mot_ratio = ot_ext_system_bayernr_manual_coring_mot_ratio_read(pipe);
}

td_void isp_nr_tnr_attr_read(ot_vi_pipe pipe, ot_isp_nr_tnr_attr *tnr_cfg)
{
    td_u8 i;
    ot_isp_nr_tnr_auto_attr *tnr_auto = &tnr_cfg->tnr_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        tnr_auto->md_mode[i] = ot_ext_system_bayernr_auto_md_mode_lut_read(pipe, i);
        tnr_auto->md_size_ratio[i] = ot_ext_system_bayernr_auto_md_size_ratio_lut_read(pipe, i);
        tnr_auto->md_anti_flicker_strength[i] = ot_ext_system_bayernr_auto_md_anti_flicker_strength_lut_read(pipe, i);
        tnr_auto->md_static_ratio[i] = ot_ext_system_bayernr_auto_md_static_ratio_lut_read(pipe, i);
        tnr_auto->md_motion_ratio[i] = ot_ext_system_bayernr_auto_md_motion_ratio_lut_read(pipe, i);
        tnr_auto->md_static_fine_strength[i] = ot_ext_system_bayernr_auto_md_static_fine_strength_lut_read(pipe, i);
        tnr_auto->tfs[i] = ot_ext_system_bayernr_auto_tfs_lut_read(pipe, i);
        tnr_auto->user_define_md[i]   = ot_ext_system_bayernr_auto_user_define_md_lut_read(pipe, i);
        tnr_auto->user_define_slope[i]   = ot_ext_system_bayernr_auto_user_define_bri_thresh_lut_read(pipe, i);
        tnr_auto->user_define_dark_thresh[i]  = ot_ext_system_bayernr_auto_user_define_dark_thresh_lut_read(pipe, i);
        tnr_auto->user_define_color_thresh[i] = ot_ext_system_bayernr_auto_user_define_color_thresh_lut_read(pipe, i);
        tnr_auto->sfr_r[i]   = ot_ext_system_bayernr_auto_sfr_r_lut_read(pipe, i);
        tnr_auto->sfr_g[i]   = ot_ext_system_bayernr_auto_sfr_g_lut_read(pipe, i);
        tnr_auto->sfr_b[i]   = ot_ext_system_bayernr_auto_sfr_b_lut_read(pipe, i);
    }

    tnr_cfg->tnr_manual.md_mode = ot_ext_system_bayernr_manual_md_mode_read(pipe);
    tnr_cfg->tnr_manual.md_size_ratio = ot_ext_system_bayernr_manual_md_size_ratio_read(pipe);
    tnr_cfg->tnr_manual.md_anti_flicker_strength = ot_ext_system_bayernr_manual_md_anti_flicker_strength_read(pipe);
    tnr_cfg->tnr_manual.md_static_ratio = ot_ext_system_bayernr_manual_md_static_ratio_read(pipe);
    tnr_cfg->tnr_manual.md_motion_ratio = ot_ext_system_bayernr_manual_md_motion_ratio_read(pipe);
    tnr_cfg->tnr_manual.md_static_fine_strength = ot_ext_system_bayernr_manual_md_static_fine_strength_read(pipe);
    tnr_cfg->tnr_manual.tfs = ot_ext_system_bayernr_manual_tfs_read(pipe);
    tnr_cfg->tnr_manual.user_define_md           = ot_ext_system_bayernr_manual_user_define_md_read(pipe);
    tnr_cfg->tnr_manual.user_define_slope = ot_ext_system_bayernr_manual_user_define_bri_thresh_read(pipe);
    tnr_cfg->tnr_manual.user_define_dark_thresh  = ot_ext_system_bayernr_manual_user_define_dark_thresh_read(pipe);
    tnr_cfg->tnr_manual.user_define_color_thresh = ot_ext_system_bayernr_manual_user_define_color_thresh_read(pipe);
    tnr_cfg->tnr_manual.sfr_r   = ot_ext_system_bayernr_manual_sfr_r_read(pipe);
    tnr_cfg->tnr_manual.sfr_g   = ot_ext_system_bayernr_manual_sfr_g_read(pipe);
    tnr_cfg->tnr_manual.sfr_b   = ot_ext_system_bayernr_manual_sfr_b_read(pipe);
}

td_void isp_nr_wdr_attr_read(ot_vi_pipe vi_pipe, ot_isp_nr_wdr_attr *wdr_cfg)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        wdr_cfg->snr_sfm0_wdr_strength[i] = ot_ext_system_bayernr_snr_sfm0_wdr_strength_lut_read(vi_pipe, i);
        wdr_cfg->snr_sfm0_fusion_strength[i] = ot_ext_system_bayernr_snr_sfm0_fusion_strength_lut_read(vi_pipe, i);
        wdr_cfg->snr_wdr_sfm6_strength[i] = ot_ext_system_bayernr_snr_wdr_sfm6_strength_lut_read(vi_pipe, i);
        wdr_cfg->snr_fusion_sfm6_strength[i] = ot_ext_system_bayernr_snr_fusion_sfm6_strength_lut_read(vi_pipe, i);
        wdr_cfg->snr_wdr_sfm7_strength[i] = ot_ext_system_bayernr_snr_wdr_sfm7_strength_lut_read(vi_pipe, i);
        wdr_cfg->snr_fusion_sfm7_strength[i] = ot_ext_system_bayernr_snr_fusion_sfm7_strength_lut_read(vi_pipe, i);
        wdr_cfg->md_wdr_strength[i] = ot_ext_system_bayernr_md_wdr_strength_lut_read(vi_pipe, i);
        wdr_cfg->md_fusion_strength[i] = ot_ext_system_bayernr_md_fusion_strength_lut_read(vi_pipe, i);
    }
}

td_void isp_nr_ynet_attr_read(ot_vi_pipe vi_pipe, ot_isp_nr_ynet_attr *ynet_cfg)
{
    ynet_cfg->denoise_y_alpha = ot_ext_system_bayernr_denoise_y_alpha_read(vi_pipe);

    td_u8 i;
    for (i = 0; i < OT_ISP_BAYERNR_DENOISE_Y_LUT_NUM; i++) {
        ynet_cfg->denoise_y_fg_str_lut[i] = ot_ext_system_bayernr_denoise_y_fg_str_lut_read(vi_pipe, i);
        ynet_cfg->denoise_y_bg_str_lut[i] = ot_ext_system_bayernr_denoise_y_bg_str_lut_read(vi_pipe, i);
    }
}

td_void isp_nr_dering_attr_read(ot_vi_pipe vi_pipe, ot_isp_nr_dering_attr *dering_cfg)
{
    td_u8 i;
    ot_isp_nr_dering_auto_attr *dering_auto = &dering_cfg->dering_auto;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        dering_auto->dering_strength[i] = ot_ext_system_bayernr_auto_dering_strength_lut_read(vi_pipe, i);
        dering_auto->dering_thresh[i] = ot_ext_system_bayernr_auto_dering_thresh_lut_read(vi_pipe, i);
        dering_auto->dering_static_strength[i] = ot_ext_system_bayernr_auto_dering_static_strength_lut_read(vi_pipe, i);
        dering_auto->dering_motion_strength[i] = ot_ext_system_bayernr_auto_dering_motion_strength_lut_read(vi_pipe, i);
    }

    dering_cfg->dering_manual.dering_strength = ot_ext_system_bayernr_manual_dering_strength_read(vi_pipe);
    dering_cfg->dering_manual.dering_thresh = ot_ext_system_bayernr_manual_dering_thresh_read(vi_pipe);
    dering_cfg->dering_manual.dering_static_strength =
        ot_ext_system_bayernr_manual_dering_static_strength_read(vi_pipe);
    dering_cfg->dering_manual.dering_motion_strength =
        ot_ext_system_bayernr_manual_dering_motion_strength_read(vi_pipe);
}

td_void isp_black_level_manual_attr_write(ot_vi_pipe vi_pipe, const ot_isp_black_level_manual_attr *manual_attr)
{
    ot_ext_system_black_level_f0_r_write(vi_pipe, manual_attr->black_level[0][OT_ISP_CHN_R]); /* wdr 0 */
    ot_ext_system_black_level_f0_gr_write(vi_pipe, manual_attr->black_level[0][OT_ISP_CHN_GR]); /* wdr 0 */
    ot_ext_system_black_level_f0_gb_write(vi_pipe, manual_attr->black_level[0][OT_ISP_CHN_GB]); /* wdr 0 */
    ot_ext_system_black_level_f0_b_write(vi_pipe, manual_attr->black_level[0][OT_ISP_CHN_B]); /* wdr 0 */

    ot_ext_system_black_level_f1_r_write(vi_pipe, manual_attr->black_level[1][OT_ISP_CHN_R]); /* wdr1 */
    ot_ext_system_black_level_f1_gr_write(vi_pipe, manual_attr->black_level[1][OT_ISP_CHN_GR]); /* wdr1 */
    ot_ext_system_black_level_f1_gb_write(vi_pipe, manual_attr->black_level[1][OT_ISP_CHN_GB]); /* wdr1 */
    ot_ext_system_black_level_f1_b_write(vi_pipe, manual_attr->black_level[1][OT_ISP_CHN_B]); /* wdr1 */

    ot_ext_system_black_level_f2_r_write(vi_pipe, manual_attr->black_level[2][OT_ISP_CHN_R]); /* wdr2 */
    ot_ext_system_black_level_f2_gr_write(vi_pipe, manual_attr->black_level[2][OT_ISP_CHN_GR]); /* wdr2 */
    ot_ext_system_black_level_f2_gb_write(vi_pipe, manual_attr->black_level[2][OT_ISP_CHN_GB]); /* wdr2 */
    ot_ext_system_black_level_f2_b_write(vi_pipe, manual_attr->black_level[2][OT_ISP_CHN_B]); /* wdr2 */

    ot_ext_system_black_level_f3_r_write(vi_pipe, manual_attr->black_level[3][OT_ISP_CHN_R]); /* wdr3 */
    ot_ext_system_black_level_f3_gr_write(vi_pipe, manual_attr->black_level[3][OT_ISP_CHN_GR]); /* wdr3 */
    ot_ext_system_black_level_f3_gb_write(vi_pipe, manual_attr->black_level[3][OT_ISP_CHN_GB]); /* wdr3 */
    ot_ext_system_black_level_f3_b_write(vi_pipe, manual_attr->black_level[3][OT_ISP_CHN_B]); /* wdr3 */
}

td_void isp_user_black_level_write(ot_vi_pipe vi_pipe, const td_u16 (*user_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;

    isp_check_pointer_void_return(user_black_level);
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ot_ext_system_user_black_level_f0_write(vi_pipe, i, user_black_level[0][i]); /* wdr 0 */
        ot_ext_system_user_black_level_f1_write(vi_pipe, i, user_black_level[1][i]); /* wdr 1 */
        ot_ext_system_user_black_level_f2_write(vi_pipe, i, user_black_level[2][i]); /* wdr 2 */
        ot_ext_system_user_black_level_f3_write(vi_pipe, i, user_black_level[3][i]); /* wdr 3 */
    }
}

td_void isp_black_level_dynamic_attr_write(ot_vi_pipe vi_pipe, const ot_isp_black_level_dynamic_attr *dynamic_attr)
{
    td_u8 i;
    ot_ext_system_isp_dynamic_blc_high_threshold_write(vi_pipe, dynamic_attr->high_threshold);
    ot_ext_system_isp_dynamic_blc_low_threshold_write(vi_pipe, dynamic_attr->low_threshold);
    ot_ext_system_isp_dynamic_blc_startxpos_write(vi_pipe, dynamic_attr->ob_area.x);
    ot_ext_system_isp_dynamic_blc_startypos_write(vi_pipe, dynamic_attr->ob_area.y);
    ot_ext_system_isp_dynamic_blc_obheight_write(vi_pipe, dynamic_attr->ob_area.height);
    ot_ext_system_isp_dynamic_blc_obwidth_write(vi_pipe, dynamic_attr->ob_area.width);
    ot_ext_system_isp_dynamic_blc_pattern_write(vi_pipe, dynamic_attr->pattern);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_isp_dynamic_blc_offset_updata_write(vi_pipe, i, dynamic_attr->offset[i]);
    }

    ot_ext_system_isp_dynamic_blc_tolerance_updata_write(vi_pipe, dynamic_attr->tolerance);
    ot_ext_system_isp_dynamic_blc_filter_strength_updata_write(vi_pipe, dynamic_attr->filter_strength);
    ot_ext_system_isp_dynamic_blc_separate_en_updata_write(vi_pipe, dynamic_attr->separate_en);
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_isp_ag_cali_dynamic_blc_updata_write(vi_pipe, i, dynamic_attr->calibration_black_level[i]);
    }
    ot_ext_system_isp_dynamic_blc_filter_thr_updata_write(vi_pipe, dynamic_attr->filter_thr);
}
td_void isp_black_level_manual_attr_read(ot_vi_pipe vi_pipe, ot_isp_black_level_manual_attr *manual_attr)
{
    manual_attr->black_level[0][OT_ISP_CHN_R]  = ot_ext_system_black_level_f0_r_read(vi_pipe);
    manual_attr->black_level[0][OT_ISP_CHN_GR] = ot_ext_system_black_level_f0_gr_read(vi_pipe);
    manual_attr->black_level[0][OT_ISP_CHN_GB] = ot_ext_system_black_level_f0_gb_read(vi_pipe);
    manual_attr->black_level[0][OT_ISP_CHN_B]  = ot_ext_system_black_level_f0_b_read(vi_pipe);

    manual_attr->black_level[1][OT_ISP_CHN_R]  = ot_ext_system_black_level_f1_r_read(vi_pipe);
    manual_attr->black_level[1][OT_ISP_CHN_GR] = ot_ext_system_black_level_f1_gr_read(vi_pipe);
    manual_attr->black_level[1][OT_ISP_CHN_GB] = ot_ext_system_black_level_f1_gb_read(vi_pipe);
    manual_attr->black_level[1][OT_ISP_CHN_B]  = ot_ext_system_black_level_f1_b_read(vi_pipe);

    manual_attr->black_level[2][OT_ISP_CHN_R]  = ot_ext_system_black_level_f2_r_read(vi_pipe); /* wdr2 */
    manual_attr->black_level[2][OT_ISP_CHN_GR] = ot_ext_system_black_level_f2_gr_read(vi_pipe); /* wdr2 */
    manual_attr->black_level[2][OT_ISP_CHN_GB] = ot_ext_system_black_level_f2_gb_read(vi_pipe); /* wdr2 */
    manual_attr->black_level[2][OT_ISP_CHN_B]  = ot_ext_system_black_level_f2_b_read(vi_pipe); /* wdr2 */

    manual_attr->black_level[3][OT_ISP_CHN_R]  = ot_ext_system_black_level_f3_r_read(vi_pipe); /* wdr3 */
    manual_attr->black_level[3][OT_ISP_CHN_GR] = ot_ext_system_black_level_f3_gr_read(vi_pipe); /* wdr3 */
    manual_attr->black_level[3][OT_ISP_CHN_GB] = ot_ext_system_black_level_f3_gb_read(vi_pipe); /* wdr3 */
    manual_attr->black_level[3][OT_ISP_CHN_B]  = ot_ext_system_black_level_f3_b_read(vi_pipe); /* wdr3 */
}

td_void isp_user_black_level_read(ot_vi_pipe vi_pipe, td_u16 (*user_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;

    isp_check_pointer_void_return(user_black_level);
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        user_black_level[0][i] = ot_ext_system_user_black_level_f0_read(vi_pipe, i); /* wdr 0 */
        user_black_level[1][i] = ot_ext_system_user_black_level_f1_read(vi_pipe, i); /* wdr 1 */
        user_black_level[2][i] = ot_ext_system_user_black_level_f2_read(vi_pipe, i); /* wdr 2 */
        user_black_level[3][i] = ot_ext_system_user_black_level_f3_read(vi_pipe, i); /* wdr 3 */
    }
}

td_void isp_black_level_dynamic_attr_read(ot_vi_pipe vi_pipe, ot_isp_black_level_dynamic_attr *dynamic_attr)
{
    td_u8 i;
    dynamic_attr->high_threshold = ot_ext_system_isp_dynamic_blc_high_threshold_read(vi_pipe);
    dynamic_attr->low_threshold = ot_ext_system_isp_dynamic_blc_low_threshold_read(vi_pipe);
    dynamic_attr->ob_area.height = ot_ext_system_isp_dynamic_blc_obheight_read(vi_pipe);
    dynamic_attr->ob_area.width = ot_ext_system_isp_dynamic_blc_obwidth_read(vi_pipe);
    dynamic_attr->ob_area.x = ot_ext_system_isp_dynamic_blc_startxpos_read(vi_pipe);
    dynamic_attr->ob_area.y = ot_ext_system_isp_dynamic_blc_startypos_read(vi_pipe);
    dynamic_attr->pattern = ot_ext_system_isp_dynamic_blc_pattern_read(vi_pipe);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        dynamic_attr->offset[i] = (td_s16)ot_ext_system_isp_dynamic_blc_offset_updata_read(vi_pipe, i);
    }

    dynamic_attr->tolerance = ot_ext_system_isp_dynamic_blc_tolerance_updata_read(vi_pipe);
    dynamic_attr->filter_strength = ot_ext_system_isp_dynamic_blc_filter_strength_updata_read(vi_pipe);
    dynamic_attr->separate_en = ot_ext_system_isp_dynamic_blc_separate_en_updata_read(vi_pipe);
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        dynamic_attr->calibration_black_level[i] =
            ot_ext_system_isp_ag_cali_dynamic_blc_updata_read(vi_pipe, i);
    }
    dynamic_attr->filter_thr = ot_ext_system_isp_dynamic_blc_filter_thr_updata_read(vi_pipe);
}

static td_void isp_black_level_actual_value_write(ot_vi_pipe vi_pipe,
    const td_u16 (*black_level_actual)[OT_ISP_BAYER_CHN_NUM])
{
    ot_ext_system_black_level_query_f0_r_write(vi_pipe, black_level_actual[0][OT_ISP_CHN_R]); /* wdr0 */
    ot_ext_system_black_level_query_f0_gr_write(vi_pipe, black_level_actual[0][OT_ISP_CHN_GR]); /* wdr0 */
    ot_ext_system_black_level_query_f0_gb_write(vi_pipe, black_level_actual[0][OT_ISP_CHN_GB]); /* wdr0 */
    ot_ext_system_black_level_query_f0_b_write(vi_pipe, black_level_actual[0][OT_ISP_CHN_B]); /* wdr0 */

    ot_ext_system_black_level_query_f1_r_write(vi_pipe, black_level_actual[1][OT_ISP_CHN_R]); /* wdr1 */
    ot_ext_system_black_level_query_f1_gr_write(vi_pipe, black_level_actual[1][OT_ISP_CHN_GR]); /* wdr1 */
    ot_ext_system_black_level_query_f1_gb_write(vi_pipe, black_level_actual[1][OT_ISP_CHN_GB]); /* wdr1 */
    ot_ext_system_black_level_query_f1_b_write(vi_pipe, black_level_actual[1][OT_ISP_CHN_B]); /* wdr1 */

    ot_ext_system_black_level_query_f2_r_write(vi_pipe, black_level_actual[2][OT_ISP_CHN_R]); /* wdr2 */
    ot_ext_system_black_level_query_f2_gr_write(vi_pipe, black_level_actual[2][OT_ISP_CHN_GR]); /* wdr2 */
    ot_ext_system_black_level_query_f2_gb_write(vi_pipe, black_level_actual[2][OT_ISP_CHN_GB]); /* wdr2 */
    ot_ext_system_black_level_query_f2_b_write(vi_pipe, black_level_actual[2][OT_ISP_CHN_B]); /* wdr2 */

    ot_ext_system_black_level_query_f3_r_write(vi_pipe, black_level_actual[3][OT_ISP_CHN_R]); /* wdr3 */
    ot_ext_system_black_level_query_f3_gr_write(vi_pipe, black_level_actual[3][OT_ISP_CHN_GR]); /* wdr3 */
    ot_ext_system_black_level_query_f3_gb_write(vi_pipe, black_level_actual[3][OT_ISP_CHN_GB]); /* wdr3 */
    ot_ext_system_black_level_query_f3_b_write(vi_pipe, black_level_actual[3][OT_ISP_CHN_B]); /* wdr3 */
}

static td_void sns_black_level_actual_value_write(ot_vi_pipe vi_pipe,
    const td_u16 (*sns_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ot_ext_system_sns_black_level_query_f0_write(vi_pipe, i, sns_black_level[0][i]); /* wdr 0 */
        ot_ext_system_sns_black_level_query_f1_write(vi_pipe, i, sns_black_level[1][i]); /* wdr 1 */
        ot_ext_system_sns_black_level_query_f2_write(vi_pipe, i, sns_black_level[2][i]); /* wdr 2 */
        ot_ext_system_sns_black_level_query_f3_write(vi_pipe, i, sns_black_level[3][i]); /* wdr 3 */
    }
}

static td_void ori_black_level_actual_value_write(ot_vi_pipe vi_pipe,
    const td_u16 (*ori_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ot_ext_system_ori_black_level_query_f0_write(vi_pipe, i, ori_black_level[0][i]); /* wdr 0 */
        ot_ext_system_ori_black_level_query_f1_write(vi_pipe, i, ori_black_level[1][i]); /* wdr 1 */
        ot_ext_system_ori_black_level_query_f2_write(vi_pipe, i, ori_black_level[2][i]); /* wdr 2 */
        ot_ext_system_ori_black_level_query_f3_write(vi_pipe, i, ori_black_level[3][i]); /* wdr 3 */
    }
}

td_void black_level_actual_value_write(ot_vi_pipe vi_pipe, const isp_blc_actual_info *actual)
{
    isp_check_pointer_void_return(actual);
    isp_black_level_actual_value_write(vi_pipe, actual->isp_black_level);
    sns_black_level_actual_value_write(vi_pipe, actual->sns_black_level);
    ori_black_level_actual_value_write(vi_pipe, actual->ori_black_level);
}

td_void isp_black_level_actual_value_read(ot_vi_pipe vi_pipe, td_u16 black_level_actual[][4])
{
    isp_check_pointer_void_return(black_level_actual);
    black_level_actual[0][OT_ISP_CHN_R]  = ot_ext_system_black_level_query_f0_r_read(vi_pipe); /* wdr0 */
    black_level_actual[0][OT_ISP_CHN_GR] = ot_ext_system_black_level_query_f0_gr_read(vi_pipe); /* wdr0 */
    black_level_actual[0][OT_ISP_CHN_GB] = ot_ext_system_black_level_query_f0_gb_read(vi_pipe); /* wdr0 */
    black_level_actual[0][OT_ISP_CHN_B]  = ot_ext_system_black_level_query_f0_b_read(vi_pipe); /* wdr0 */

    black_level_actual[1][OT_ISP_CHN_R]  = ot_ext_system_black_level_query_f1_r_read(vi_pipe); /* wdr1 */
    black_level_actual[1][OT_ISP_CHN_GR] = ot_ext_system_black_level_query_f1_gr_read(vi_pipe); /* wdr1 */
    black_level_actual[1][OT_ISP_CHN_GB] = ot_ext_system_black_level_query_f1_gb_read(vi_pipe); /* wdr1 */
    black_level_actual[1][OT_ISP_CHN_B]  = ot_ext_system_black_level_query_f1_b_read(vi_pipe); /* wdr1 */

    black_level_actual[2][OT_ISP_CHN_R]  = ot_ext_system_black_level_query_f2_r_read(vi_pipe); /* wdr2 */
    black_level_actual[2][OT_ISP_CHN_GR] = ot_ext_system_black_level_query_f2_gr_read(vi_pipe); /* wdr2 */
    black_level_actual[2][OT_ISP_CHN_GB] = ot_ext_system_black_level_query_f2_gb_read(vi_pipe); /* wdr2 */
    black_level_actual[2][OT_ISP_CHN_B]  = ot_ext_system_black_level_query_f2_b_read(vi_pipe); /* wdr2 */

    black_level_actual[3][OT_ISP_CHN_R]  = ot_ext_system_black_level_query_f3_r_read(vi_pipe); /* wdr3 */
    black_level_actual[3][OT_ISP_CHN_GR] = ot_ext_system_black_level_query_f3_gr_read(vi_pipe); /* wdr3 */
    black_level_actual[3][OT_ISP_CHN_GB] = ot_ext_system_black_level_query_f3_gb_read(vi_pipe); /* wdr3 */
    black_level_actual[3][OT_ISP_CHN_B]  = ot_ext_system_black_level_query_f3_b_read(vi_pipe); /* wdr3 */
}

td_void sns_black_level_actual_value_read(ot_vi_pipe vi_pipe, td_u16 (*sns_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;

    isp_check_pointer_void_return(sns_black_level);
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        sns_black_level[0][i] = ot_ext_system_sns_black_level_query_f0_read(vi_pipe, i); /* wdr 0 */
        sns_black_level[1][i] = ot_ext_system_sns_black_level_query_f1_read(vi_pipe, i); /* wdr 1 */
        sns_black_level[2][i] = ot_ext_system_sns_black_level_query_f2_read(vi_pipe, i); /* wdr 2 */
        sns_black_level[3][i] = ot_ext_system_sns_black_level_query_f3_read(vi_pipe, i); /* wdr 3 */
    }
}

td_void ori_black_level_actual_value_read(ot_vi_pipe vi_pipe, td_u16 (*ori_black_level)[OT_ISP_BAYER_CHN_NUM])
{
    td_u8 i;

    isp_check_pointer_void_return(ori_black_level);
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ori_black_level[0][i] = ot_ext_system_ori_black_level_query_f0_read(vi_pipe, i); /* wdr 0 */
        ori_black_level[1][i] = ot_ext_system_ori_black_level_query_f1_read(vi_pipe, i); /* wdr 1 */
        ori_black_level[2][i] = ot_ext_system_ori_black_level_query_f2_read(vi_pipe, i); /* wdr 2 */
        ori_black_level[3][i] = ot_ext_system_ori_black_level_query_f3_read(vi_pipe, i); /* wdr 3 */
    }
}

td_void isp_bayershp_auto_attr_write(ot_vi_pipe vi_pipe, const ot_isp_bayershp_auto_attr *auto_attr)
{
    td_u8 i, j, idx;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_bshp_mf_gain_auto_write(vi_pipe, i, auto_attr->mf_gain[i]);
        ot_ext_system_bshp_hf_gain_auto_write(vi_pipe, i, auto_attr->hf_gain[i]);
        ot_ext_system_bshp_dark_gain_auto_write(vi_pipe, i, auto_attr->dark_gain[i]);
        ot_ext_system_bshp_overshoot_auto_write(vi_pipe, i, auto_attr->overshoot[i]);
        ot_ext_system_bshp_undershoot_auto_write(vi_pipe, i, auto_attr->undershoot[i]);
        for (j = 0; j < OT_ISP_BSHP_CURVE_NUM; j++) {
            idx = j * OT_ISP_AUTO_ISO_NUM + i;
            ot_ext_system_bshp_mf_str_auto_lut_write(vi_pipe, idx, auto_attr->mf_strength[j][i]);
            ot_ext_system_bshp_hf_str_auto_lut_write(vi_pipe, idx, auto_attr->hf_strength[j][i]);
            ot_ext_system_bshp_dark_str_auto_lut_write(vi_pipe, idx, auto_attr->dark_strength[j][i]);
        }
    }
}
td_void isp_bayershp_manual_attr_write(ot_vi_pipe vi_pipe, const ot_isp_bayershp_manual_attr *manual_attr)
{
    td_u8 i;
    ot_ext_system_bshp_mf_gain_manual_write(vi_pipe, manual_attr->mf_gain);
    ot_ext_system_bshp_hf_gain_manual_write(vi_pipe, manual_attr->hf_gain);
    ot_ext_system_bshp_dark_gain_manual_write(vi_pipe, manual_attr->dark_gain);
    ot_ext_system_bshp_overshoot_manual_write(vi_pipe, manual_attr->overshoot);
    ot_ext_system_bshp_undershoot_manual_write(vi_pipe, manual_attr->undershoot);
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        ot_ext_system_bshp_mf_str_manual_lut_write(vi_pipe, i, manual_attr->mf_strength[i]);
        ot_ext_system_bshp_hf_str_manual_lut_write(vi_pipe, i, manual_attr->hf_strength[i]);
        ot_ext_system_bshp_dark_str_manual_lut_write(vi_pipe, i, manual_attr->dark_strength[i]);
    }
}
td_void isp_bayershp_common_attr_write(ot_vi_pipe vi_pipe, const ot_isp_bayershp_attr *bshp_attr)
{
    ot_ext_system_bshp_dark_thd_low_manual_write(vi_pipe, bshp_attr->dark_threshold[0]);
    ot_ext_system_bshp_dark_thd_high_manual_write(vi_pipe, bshp_attr->dark_threshold[1]);
    ot_ext_system_bshp_texture_thd_low_manual_write(vi_pipe, bshp_attr->texture_threshold[0]);
    ot_ext_system_bshp_texture_thd_high_manual_write(vi_pipe, bshp_attr->texture_threshold[1]);
}

td_void isp_bayershp_manual_attr_read(ot_vi_pipe vi_pipe, ot_isp_bayershp_manual_attr *manual_attr)
{
    td_u8 i;
    manual_attr->mf_gain       = ot_ext_system_bshp_mf_gain_manual_read(vi_pipe);
    manual_attr->hf_gain       = ot_ext_system_bshp_hf_gain_manual_read(vi_pipe);
    manual_attr->dark_gain     = ot_ext_system_bshp_dark_gain_manual_read(vi_pipe);
    manual_attr->overshoot     = ot_ext_system_bshp_overshoot_manual_read(vi_pipe);
    manual_attr->undershoot    = ot_ext_system_bshp_undershoot_manual_read(vi_pipe);
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        manual_attr->mf_strength[i]    = ot_ext_system_bshp_mf_str_manual_lut_read(vi_pipe, i);
        manual_attr->hf_strength[i]    = ot_ext_system_bshp_hf_str_manual_lut_read(vi_pipe, i);
        manual_attr->dark_strength[i]  = ot_ext_system_bshp_dark_str_manual_lut_read(vi_pipe, i);
    }
}

td_void isp_bayershp_auto_attr_read(ot_vi_pipe vi_pipe, ot_isp_bayershp_auto_attr *auto_attr)
{
    td_u8 i, j, idx;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        auto_attr->mf_gain[i]     = ot_ext_system_bshp_mf_gain_auto_read(vi_pipe, i);
        auto_attr->hf_gain[i]     = ot_ext_system_bshp_hf_gain_auto_read(vi_pipe, i);
        auto_attr->dark_gain[i]   = ot_ext_system_bshp_dark_gain_auto_read(vi_pipe, i);
        auto_attr->overshoot[i]   = ot_ext_system_bshp_overshoot_auto_read(vi_pipe, i);
        auto_attr->undershoot[i]  = ot_ext_system_bshp_undershoot_auto_read(vi_pipe, i);
        for (j = 0; j < OT_ISP_BSHP_CURVE_NUM; j++) {
            idx = j * OT_ISP_AUTO_ISO_NUM + i;
            auto_attr->mf_strength[j][i]    = ot_ext_system_bshp_mf_str_auto_lut_read(vi_pipe, idx);
            auto_attr->hf_strength[j][i]    = ot_ext_system_bshp_hf_str_auto_lut_read(vi_pipe, idx);
            auto_attr->dark_strength[j][i]  = ot_ext_system_bshp_dark_str_auto_lut_read(vi_pipe, idx);
        }
    }
}

td_void isp_bayershp_common_attr_read(ot_vi_pipe vi_pipe, ot_isp_bayershp_attr *bshp_attr)
{
    bshp_attr->dark_threshold[0]   = ot_ext_system_bshp_dark_thd_low_manual_read(vi_pipe);
    bshp_attr->dark_threshold[1]   = ot_ext_system_bshp_dark_thd_high_manual_read(vi_pipe);
    bshp_attr->texture_threshold[0]   = ot_ext_system_bshp_texture_thd_low_manual_read(vi_pipe);
    bshp_attr->texture_threshold[1]   = ot_ext_system_bshp_texture_thd_high_manual_read(vi_pipe);
}

td_void isp_cac_acac_attr_write(ot_vi_pipe vi_pipe, const ot_isp_cac_acac_attr *acac_cfg)
{
    td_u8 i, j;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_cac_edge_gain_auto_write(vi_pipe, i, acac_cfg->acac_auto.edge_gain[i]);
        ot_ext_system_cac_rb_strength_auto_write(vi_pipe, i, acac_cfg->acac_auto.cac_rb_strength[i]);
        ot_ext_system_cac_purple_alpha_auto_write(vi_pipe, i, acac_cfg->acac_auto.purple_alpha[i]);
        ot_ext_system_cac_edge_alpha_auto_write(vi_pipe, i, acac_cfg->acac_auto.edge_alpha[i]);
        ot_ext_system_cac_satu_low_thd_auto_write(vi_pipe, i, acac_cfg->acac_auto.satu_low_threshold[i]);
        ot_ext_system_cac_satu_high_thd_auto_write(vi_pipe, i, acac_cfg->acac_auto.satu_high_threshold[i]);
        for (j = 0; j < OT_ISP_CAC_THR_NUM; j++) {
            ot_ext_system_cac_edge_thd_auto_write(vi_pipe, j * OT_ISP_AUTO_ISO_NUM + i,
                acac_cfg->acac_auto.edge_threshold[j][i]);
        }
    }
    ot_ext_system_cac_edge_gain_write(vi_pipe, acac_cfg->acac_manual.edge_gain);
    ot_ext_system_cac_rb_strength_write(vi_pipe, acac_cfg->acac_manual.cac_rb_strength);
    ot_ext_system_cac_purple_alpha_write(vi_pipe, acac_cfg->acac_manual.purple_alpha);
    ot_ext_system_cac_edge_alpha_write(vi_pipe, acac_cfg->acac_manual.edge_alpha);
    ot_ext_system_cac_satu_low_thd_write(vi_pipe, acac_cfg->acac_manual.satu_low_threshold);
    ot_ext_system_cac_satu_high_thd_write(vi_pipe, acac_cfg->acac_manual.satu_high_threshold);
    ot_ext_system_cac_edge_thd_write(vi_pipe, 0, acac_cfg->acac_manual.edge_threshold[0]);
    ot_ext_system_cac_edge_thd_write(vi_pipe, 1, acac_cfg->acac_manual.edge_threshold[1]);
}

td_void isp_cac_lcac_attr_write(ot_vi_pipe vi_pipe, const ot_isp_cac_lcac_attr *lcac_cfg)
{
    td_u8 i;
    ot_ext_system_cac_purple_det_range_write(vi_pipe, lcac_cfg->purple_detect_range);
    ot_ext_system_cac_var_threshold_write(vi_pipe, lcac_cfg->var_threshold);
    for (i = 0; i < OT_ISP_CAC_CURVE_NUM; i++) {
        ot_ext_system_cac_r_thd_table_write(vi_pipe, i, lcac_cfg->r_detect_threshold[i]);
        ot_ext_system_cac_g_thd_table_write(vi_pipe, i, lcac_cfg->g_detect_threshold[i]);
        ot_ext_system_cac_b_thd_table_write(vi_pipe, i, lcac_cfg->b_detect_threshold[i]);
    }
    for (i = 0; i < OT_ISP_CAC_EXP_RATIO_NUM; i++) {
        ot_ext_system_cac_cr_str_table_write(vi_pipe, i, lcac_cfg->lcac_auto.de_purple_cr_strength[i]);
        ot_ext_system_cac_cb_str_table_write(vi_pipe, i, lcac_cfg->lcac_auto.de_purple_cb_strength[i]);
    }
    ot_ext_system_cac_manual_cr_str_write(vi_pipe, lcac_cfg->lcac_manual.de_purple_cr_strength);
    ot_ext_system_cac_manual_cb_str_write(vi_pipe, lcac_cfg->lcac_manual.de_purple_cb_strength);
}

td_void isp_cac_acac_attr_read(ot_vi_pipe vi_pipe, ot_isp_cac_acac_attr *acac_cfg)
{
    td_u8 i, j;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        acac_cfg->acac_auto.edge_gain[i]           = ot_ext_system_cac_edge_gain_auto_read(vi_pipe, i);
        acac_cfg->acac_auto.cac_rb_strength[i]     = ot_ext_system_cac_rb_strength_auto_read(vi_pipe, i);
        acac_cfg->acac_auto.purple_alpha[i]        = ot_ext_system_cac_purple_alpha_auto_read(vi_pipe, i);
        acac_cfg->acac_auto.edge_alpha[i]          = ot_ext_system_cac_edge_alpha_auto_read(vi_pipe, i);
        acac_cfg->acac_auto.satu_low_threshold[i]  = ot_ext_system_cac_satu_low_thd_auto_read(vi_pipe, i);
        acac_cfg->acac_auto.satu_high_threshold[i] = ot_ext_system_cac_satu_high_thd_auto_read(vi_pipe, i);
        for (j = 0; j < OT_ISP_CAC_THR_NUM; j++) {
            acac_cfg->acac_auto.edge_threshold[j][i] = ot_ext_system_cac_edge_thd_auto_read(vi_pipe,
                j * OT_ISP_AUTO_ISO_NUM + i);
        }
    }
    acac_cfg->acac_manual.edge_gain             = ot_ext_system_cac_edge_gain_read(vi_pipe);
    acac_cfg->acac_manual.cac_rb_strength       = ot_ext_system_cac_rb_strength_read(vi_pipe);
    acac_cfg->acac_manual.purple_alpha          = ot_ext_system_cac_purple_alpha_read(vi_pipe);
    acac_cfg->acac_manual.edge_alpha            = ot_ext_system_cac_edge_alpha_read(vi_pipe);
    acac_cfg->acac_manual.satu_low_threshold    = ot_ext_system_cac_satu_low_thd_read(vi_pipe);
    acac_cfg->acac_manual.satu_high_threshold   = ot_ext_system_cac_satu_high_thd_read(vi_pipe);

    for (i = 0; i < OT_ISP_CAC_THR_NUM; i++) {
        acac_cfg->acac_manual.edge_threshold[i] = ot_ext_system_cac_edge_thd_read(vi_pipe, i);
    }
}

td_void isp_cac_lcac_attr_read(ot_vi_pipe vi_pipe, ot_isp_cac_lcac_attr *lcac_cfg)
{
    td_u8 i;
    lcac_cfg->purple_detect_range = ot_ext_system_cac_purple_det_range_read(vi_pipe);
    lcac_cfg->var_threshold       = ot_ext_system_cac_var_threshold_read(vi_pipe);
    for (i = 0; i < OT_ISP_CAC_CURVE_NUM; i++) {
        lcac_cfg->r_detect_threshold[i] = ot_ext_system_cac_r_thd_table_read(vi_pipe, i);
        lcac_cfg->g_detect_threshold[i] = ot_ext_system_cac_g_thd_table_read(vi_pipe, i);
        lcac_cfg->b_detect_threshold[i] = ot_ext_system_cac_b_thd_table_read(vi_pipe, i);
    }
    for (i = 0; i < OT_ISP_CAC_EXP_RATIO_NUM; i++) {
        lcac_cfg->lcac_auto.de_purple_cr_strength[i] = ot_ext_system_cac_cr_str_table_read(vi_pipe, i);
        lcac_cfg->lcac_auto.de_purple_cb_strength[i] = ot_ext_system_cac_cb_str_table_read(vi_pipe, i);
    }
    lcac_cfg->lcac_manual.de_purple_cr_strength = ot_ext_system_cac_manual_cr_str_read(vi_pipe);
    lcac_cfg->lcac_manual.de_purple_cb_strength = ot_ext_system_cac_manual_cb_str_read(vi_pipe);
}

td_void isp_anti_false_color_attr_write(ot_vi_pipe vi_pipe, const ot_isp_anti_false_color_attr *anti_false_color)
{
    td_u8 i;
    ot_ext_system_afc_enable_write(vi_pipe, anti_false_color->enable);
    ot_ext_system_afc_manual_mode_write(vi_pipe, anti_false_color->op_type);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_afc_auto_thd_write(vi_pipe, i, anti_false_color->auto_attr.threshold[i]);
        ot_ext_system_afc_auto_str_write(vi_pipe, i, anti_false_color->auto_attr.strength[i]);
    }

    ot_ext_system_afc_manual_thd_write(vi_pipe, anti_false_color->manual_attr.threshold);
    ot_ext_system_afc_manual_str_write(vi_pipe, anti_false_color->manual_attr.strength);
}

td_void isp_anti_false_color_attr_read(ot_vi_pipe vi_pipe, ot_isp_anti_false_color_attr *anti_false_color)
{
    td_u8 i;
    anti_false_color->enable      = ot_ext_system_afc_enable_read(vi_pipe);
    anti_false_color->op_type = ot_ext_system_afc_manual_mode_read(vi_pipe);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        anti_false_color->auto_attr.threshold[i] = ot_ext_system_afc_auto_thd_read(vi_pipe, i);
        anti_false_color->auto_attr.strength[i]  = ot_ext_system_afc_auto_str_read(vi_pipe, i);
    }

    anti_false_color->manual_attr.threshold = ot_ext_system_afc_manual_thd_read(vi_pipe);
    anti_false_color->manual_attr.strength  = ot_ext_system_afc_manual_str_read(vi_pipe);
}

td_void isp_demosaic_attr_write(ot_vi_pipe vi_pipe, const ot_isp_demosaic_attr *dm_attr)
{
    td_u32 i;
    ot_ext_system_demosaic_enable_write(vi_pipe, dm_attr->enable);
    ot_ext_system_demosaic_manual_mode_write(vi_pipe, dm_attr->op_type);
    ot_ext_system_demosaic_ai_detail_strength_write(vi_pipe, dm_attr->ai_detail_strength);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_demosaic_auto_nddm_str_write(vi_pipe, i, dm_attr->auto_attr.nddm_strength[i]);
        ot_ext_system_demosaic_auto_nddm_mfehnc_str_write(vi_pipe, i, dm_attr->auto_attr.nddm_mf_detail_strength[i]);
        ot_ext_system_demosaic_auto_nddm_hfehnc_str_write(vi_pipe, i, dm_attr->auto_attr.nddm_hf_detail_strength[i]);
        ot_ext_system_demosaic_auto_dtlsmooth_range_write(vi_pipe, i, dm_attr->auto_attr.detail_smooth_range[i]);
        ot_ext_system_demosaic_auto_colornoise_thdf_write(vi_pipe, i, dm_attr->auto_attr.color_noise_f_threshold[i]);
        ot_ext_system_demosaic_auto_colornoise_strf_write(vi_pipe, i, dm_attr->auto_attr.color_noise_f_strength[i]);
        ot_ext_system_demosaic_auto_desat_dark_range_write(vi_pipe, i, dm_attr->auto_attr.color_noise_y_threshold[i]);
        ot_ext_system_demosaic_auto_desat_dark_strength_write(vi_pipe, i, dm_attr->auto_attr.color_noise_y_strength[i]);
    }

    ot_ext_system_demosaic_manual_nddm_str_write(vi_pipe, dm_attr->manual_attr.nddm_strength);
    ot_ext_system_demosaic_manual_nddm_mfehnc_str_write(vi_pipe, dm_attr->manual_attr.nddm_mf_detail_strength);
    ot_ext_system_demosaic_manual_nddm_hfehnc_str_write(vi_pipe, dm_attr->manual_attr.nddm_hf_detail_strength);
    ot_ext_system_demosaic_manual_dtlsmooth_range_write(vi_pipe, dm_attr->manual_attr.detail_smooth_range);
    ot_ext_system_demosaic_manual_colornoise_thdf_write(vi_pipe, dm_attr->manual_attr.color_noise_f_threshold);
    ot_ext_system_demosaic_manual_colornoise_strf_write(vi_pipe, dm_attr->manual_attr.color_noise_f_strength);
    ot_ext_system_demosaic_manual_desat_dark_range_write(vi_pipe, dm_attr->manual_attr.color_noise_y_threshold);
    ot_ext_system_demosaic_manual_desat_dark_strength_write(vi_pipe, dm_attr->manual_attr.color_noise_y_strength);
}

td_void isp_demosaic_attr_read(ot_vi_pipe vi_pipe, ot_isp_demosaic_attr *dm_attr)
{
    td_u32 i;
    dm_attr->enable  = ot_ext_system_demosaic_enable_read(vi_pipe);
    dm_attr->op_type = ot_ext_system_demosaic_manual_mode_read(vi_pipe);
    dm_attr->ai_detail_strength = ot_ext_system_demosaic_ai_detail_strength_read(vi_pipe);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        dm_attr->auto_attr.nddm_strength[i]           = ot_ext_system_demosaic_auto_nddm_str_read(vi_pipe, i);
        dm_attr->auto_attr.nddm_mf_detail_strength[i] = ot_ext_system_demosaic_auto_nddm_mfehnc_str_read(vi_pipe, i);
        dm_attr->auto_attr.nddm_hf_detail_strength[i] = ot_ext_system_demosaic_auto_nddm_hfehnc_str_read(vi_pipe, i);
        dm_attr->auto_attr.detail_smooth_range[i]     = ot_ext_system_demosaic_auto_dtlsmooth_range_read(vi_pipe, i);
        dm_attr->auto_attr.color_noise_f_threshold[i] = ot_ext_system_demosaic_auto_colornoise_thdf_read(vi_pipe, i);
        dm_attr->auto_attr.color_noise_f_strength[i]  = ot_ext_system_demosaic_auto_colornoise_strf_read(vi_pipe, i);
        dm_attr->auto_attr.color_noise_y_threshold[i] = ot_ext_system_demosaic_auto_desat_dark_range_read(vi_pipe, i);
        dm_attr->auto_attr.color_noise_y_strength[i] = ot_ext_system_demosaic_auto_desat_dark_strength_read(vi_pipe, i);
    }

    dm_attr->manual_attr.nddm_strength           = ot_ext_system_demosaic_manual_nddm_str_read(vi_pipe);
    dm_attr->manual_attr.nddm_mf_detail_strength = ot_ext_system_demosaic_manual_nddm_mfehnc_str_read(vi_pipe);
    dm_attr->manual_attr.nddm_hf_detail_strength = ot_ext_system_demosaic_manual_nddm_hfehnc_str_read(vi_pipe);
    dm_attr->manual_attr.detail_smooth_range     = ot_ext_system_demosaic_manual_dtlsmooth_range_read(vi_pipe);
    dm_attr->manual_attr.color_noise_f_threshold = ot_ext_system_demosaic_manual_colornoise_thdf_read(vi_pipe);
    dm_attr->manual_attr.color_noise_f_strength  = ot_ext_system_demosaic_manual_colornoise_strf_read(vi_pipe);
    dm_attr->manual_attr.color_noise_y_threshold = ot_ext_system_demosaic_manual_desat_dark_range_read(vi_pipe);
    dm_attr->manual_attr.color_noise_y_strength  = ot_ext_system_demosaic_manual_desat_dark_strength_read(vi_pipe);
}

td_void isp_rgbir_attr_write(ot_vi_pipe vi_pipe, const ot_isp_rgbir_attr *rgbir_attr)
{
    td_s32 i;
    td_s16 dft_cvt_matrix[OT_ISP_RGBIR_CVTMAT_NUM] = {1000, 0, 0, -1000, 0, 1000, 0, -1000, 0, 0, 1000, -1000};
    td_s16 mono_cvt_matrix[OT_ISP_RGBIR_CVTMAT_NUM] = {1000, 0, 0, 0, 0, 1000, 0, 0, 0, 0, 1000, 0};

    ot_ext_system_rgbir_enable_write(vi_pipe, rgbir_attr->rgbir_en);
    ot_ext_system_rgbir_autogain_enable_write(vi_pipe, rgbir_attr->auto_gain_en);
    ot_ext_system_rgbir_auto_gain_write(vi_pipe, rgbir_attr->auto_gain);
    ot_ext_system_rgbir_irstatus_write(vi_pipe, rgbir_attr->ir_cvtmat_mode);
    ot_ext_system_rgbir_smooth_enable_write(vi_pipe, rgbir_attr->smooth_en);
    for (i = 0; i < OT_ISP_RGBIR_CTRL_NUM; i++) {
        ot_ext_system_rgbir_expctrl_write(vi_pipe, rgbir_attr->exp_ctrl[i], i);
        ot_ext_system_rgbir_gain_write(vi_pipe, rgbir_attr->exp_gain[i], i);
        ot_ext_system_rgbir_wb_ctrl_write(vi_pipe, rgbir_attr->wb_ctrl_strength[i], i);
    }

    for (i = 0; i < ISP_RGBIR_CVTMATRIX_NUM; i++) {
        if (rgbir_attr->ir_cvtmat_mode == OT_ISP_IR_CVTMAT_MODE_USER) {
            ot_ext_system_rgbir_cvtmatrix_write(vi_pipe, (td_u16)rgbir_attr->cvt_matrix[i], i);
        } else if (rgbir_attr->ir_cvtmat_mode == OT_ISP_IR_CVTMAT_MODE_MONO) {
            ot_ext_system_rgbir_cvtmatrix_write(vi_pipe, (td_u16)mono_cvt_matrix[i], i);
        } else {
            ot_ext_system_rgbir_cvtmatrix_write(vi_pipe, (td_u16)dft_cvt_matrix[i], i);
        }
    }

    ot_ext_system_rgbir_irremove_enable_write(vi_pipe, rgbir_attr->ir_rm_en);
    ot_ext_system_rgbir_irremove_ratio_write(vi_pipe, rgbir_attr->ir_rm_ratio[0], 0); /* ratio 0 */
    ot_ext_system_rgbir_irremove_ratio_write(vi_pipe, rgbir_attr->ir_rm_ratio[1], 1); /* ratio 1 */
    ot_ext_system_rgbir_irremove_ratio_write(vi_pipe, rgbir_attr->ir_rm_ratio[2], 2); /* ratio 2 */

    ot_ext_system_rgbir_mode_write(vi_pipe, rgbir_attr->rgbir_cfg.mode);
    ot_ext_system_rgbir_inpattern_write(vi_pipe, rgbir_attr->rgbir_cfg.in_rgbir_pattern);
    ot_ext_system_rgbir_outpattern_write(vi_pipe, rgbir_attr->rgbir_cfg.out_pattern);
    ot_ext_system_rgbir_is_irupscale_write(vi_pipe, rgbir_attr->rgbir_cfg.is_ir_upscale);
    ot_ext_system_rgbir_in_bayer_write(vi_pipe, rgbir_attr->rgbir_cfg.in_bayer_pattern);
}

td_void isp_rgbir_attr_read(ot_vi_pipe vi_pipe, ot_isp_rgbir_attr *rgbir_attr)
{
    td_u8 i;
    rgbir_attr->rgbir_cfg.mode = ot_ext_system_rgbir_mode_read(vi_pipe);
    rgbir_attr->rgbir_en = ot_ext_system_rgbir_enable_read(vi_pipe);
    rgbir_attr->smooth_en = ot_ext_system_rgbir_smooth_enable_read(vi_pipe);
    rgbir_attr->rgbir_cfg.in_rgbir_pattern = ot_ext_system_rgbir_inpattern_read(vi_pipe);
    rgbir_attr->rgbir_cfg.out_pattern = ot_ext_system_rgbir_outpattern_read(vi_pipe);
    rgbir_attr->ir_cvtmat_mode = ot_ext_system_rgbir_irstatus_read(vi_pipe);

    for (i = 0; i < OT_ISP_RGBIR_CTRL_NUM; i++) {
        rgbir_attr->exp_ctrl[i] = ot_ext_system_rgbir_expctrl_read(vi_pipe, i);
        rgbir_attr->exp_gain[i] = ot_ext_system_rgbir_gain_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_RGBIR_CVTMAT_NUM; i++) {
        rgbir_attr->cvt_matrix[i] = (td_s16)ot_ext_system_rgbir_cvtmatrix_read(vi_pipe, i);
    }
    rgbir_attr->ir_rm_en = ot_ext_system_rgbir_irremove_enable_read(vi_pipe);
    for (i = 0; i < OT_ISP_RGBIR_CROSSTALK_NUM; i++) {
        rgbir_attr->ir_rm_ratio[i] = ot_ext_system_rgbir_irremove_ratio_read(vi_pipe, i);
    }
    rgbir_attr->ir_sum_info =  ot_ext_system_rgbir_ir_sum_read(vi_pipe);
    for (i = 0; i < OT_ISP_RGBIR_CTRL_NUM; i++) {
        rgbir_attr->wb_ctrl_strength[i] = ot_ext_system_rgbir_wb_ctrl_read(vi_pipe, i);
    }
    rgbir_attr->auto_gain_en = ot_ext_system_rgbir_autogain_enable_read(vi_pipe);
    rgbir_attr->auto_gain = ot_ext_system_rgbir_auto_gain_read(vi_pipe);
    rgbir_attr->rgbir_cfg.is_ir_upscale = ot_ext_system_rgbir_is_irupscale_read(vi_pipe);
    rgbir_attr->rgbir_cfg.in_bayer_pattern  = ot_ext_system_rgbir_in_bayer_read(vi_pipe);
}

static td_void isp_af_iir_en_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir0_enable0_write(vi_pipe, af_cfg->h_param_iir0.iir_en[0]); /* array index 0 */
    ot_ext_af_iir0_enable1_write(vi_pipe, af_cfg->h_param_iir0.iir_en[1]); /* array index 1 */
    ot_ext_af_iir0_enable2_write(vi_pipe, af_cfg->h_param_iir0.iir_en[2]); /* array index 2 */
    ot_ext_af_iir1_enable0_write(vi_pipe, af_cfg->h_param_iir1.iir_en[0]); /* array index 0 */
    ot_ext_af_iir1_enable1_write(vi_pipe, af_cfg->h_param_iir1.iir_en[1]); /* array index 1 */
    ot_ext_af_iir1_enable2_write(vi_pipe, af_cfg->h_param_iir1.iir_en[2]); /* array index 2 */
}

static td_void isp_af_iir_gain_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir_gain0_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[0]); /* array index 0 */
    ot_ext_af_iir_gain0_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[0]); /* array index 0 */
    ot_ext_af_iir_gain1_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[1]); /* array index 1 */
    ot_ext_af_iir_gain1_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[1]); /* array index 1 */
    ot_ext_af_iir_gain2_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[2]); /* array index 2 */
    ot_ext_af_iir_gain2_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[2]); /* array index 2 */
    ot_ext_af_iir_gain3_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[3]); /* array index 3 */
    ot_ext_af_iir_gain3_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[3]); /* array index 3 */
    ot_ext_af_iir_gain4_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[4]); /* array index 4 */
    ot_ext_af_iir_gain4_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[4]); /* array index 4 */
    ot_ext_af_iir_gain5_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[5]); /* array index 5 */
    ot_ext_af_iir_gain5_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[5]); /* array index 5 */
    ot_ext_af_iir_gain6_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_gain[6]); /* array index 6 */
    ot_ext_af_iir_gain6_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_gain[6]); /* array index 6 */
}

static td_void isp_af_iir_shift_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir0_shift_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_shift_lut[0]); /* array index 0 */
    ot_ext_af_iir1_shift_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_shift_lut[1]); /* array index 1 */
    ot_ext_af_iir2_shift_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_shift_lut[2]); /* array index 2 */
    ot_ext_af_iir3_shift_group0_write(vi_pipe, af_cfg->h_param_iir0.iir_shift_lut[3]); /* array index 3 */
    ot_ext_af_iir0_shift_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_shift_lut[0]); /* array index 0 */
    ot_ext_af_iir1_shift_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_shift_lut[1]); /* array index 1 */
    ot_ext_af_iir2_shift_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_shift_lut[2]); /* array index 2 */
    ot_ext_af_iir3_shift_group1_write(vi_pipe, af_cfg->h_param_iir1.iir_shift_lut[3]); /* array index 3 */
    ot_ext_af_iir0_shift_write(vi_pipe, af_cfg->h_param_iir0.iir_shift);
    ot_ext_af_iir1_shift_write(vi_pipe, af_cfg->h_param_iir1.iir_shift);
}

static td_void isp_af_fir_gain_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_fir_h_gain0_group0_write(vi_pipe, af_cfg->v_param_fir0.fir_gain[0]); /* array index 0 */
    ot_ext_af_fir_h_gain0_group1_write(vi_pipe, af_cfg->v_param_fir1.fir_gain[0]); /* array index 0 */
    ot_ext_af_fir_h_gain1_group0_write(vi_pipe, af_cfg->v_param_fir0.fir_gain[1]); /* array index 1 */
    ot_ext_af_fir_h_gain1_group1_write(vi_pipe, af_cfg->v_param_fir1.fir_gain[1]); /* array index 1 */
    ot_ext_af_fir_h_gain2_group0_write(vi_pipe, af_cfg->v_param_fir0.fir_gain[2]); /* array index 2 */
    ot_ext_af_fir_h_gain2_group1_write(vi_pipe, af_cfg->v_param_fir1.fir_gain[2]); /* array index 2 */
    ot_ext_af_fir_h_gain3_group0_write(vi_pipe, af_cfg->v_param_fir0.fir_gain[3]); /* array index 3 */
    ot_ext_af_fir_h_gain3_group1_write(vi_pipe, af_cfg->v_param_fir1.fir_gain[3]); /* array index 3 */
    ot_ext_af_fir_h_gain4_group0_write(vi_pipe, af_cfg->v_param_fir0.fir_gain[4]); /* array index 4 */
    ot_ext_af_fir_h_gain4_group1_write(vi_pipe, af_cfg->v_param_fir1.fir_gain[4]); /* array index 4 */
}

static td_void isp_af_ds_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir0_ds_enable_write(vi_pipe, af_cfg->h_param_iir0.narrow_band_en);
    ot_ext_af_iir1_ds_enable_write(vi_pipe, af_cfg->h_param_iir1.narrow_band_en);
}

static td_void isp_af_plg_pls_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    td_u8 i, shift0;
    td_s16 g0, g1, g2;
    td_u32 pls, plg;
    td_float pl, temp;
    const ot_isp_af_h_param *iir = TD_NULL;

    for (i = 0; i < OT_ISP_AF_PLGS_NUM; i++) {
        iir = i ? &(af_cfg->h_param_iir1) : &(af_cfg->h_param_iir0);

        shift0 = (td_u8)iir->iir_shift_lut[0];
        g0 = iir->iir_gain[0]; /* array index 0 */
        g1 = iir->iir_gain[1]; /* array index 1 */
        g2 = iir->iir_gain[2]; /* array index 2 */

        pl = (512.f / div_0_to_1(512 - 2 * g1 - g2) * g0) / (1 << shift0); /* const value 512 */
        temp = pl;
        temp = clip3(7 - floor(log(temp) / log(2)), 0, 7); /* max value 7, and const log(2) */

        pls = (td_u32)temp;
        plg = (td_u32)((pl * (1 << pls)) + 0.5); /* round(0.5) */

        if (i == 0) {
            ot_ext_af_iir_pls_group0_write(vi_pipe, pls);
            ot_ext_af_iir_plg_group0_write(vi_pipe, plg);
        } else {
            ot_ext_af_iir_pls_group1_write(vi_pipe, pls);
            ot_ext_af_iir_plg_group1_write(vi_pipe, plg);
        }
    }
}

static td_void isp_af_crop_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_crop_enable_write(vi_pipe, af_cfg->config.crop.enable);
    ot_ext_af_crop_pos_x_write(vi_pipe, af_cfg->config.crop.x);
    ot_ext_af_crop_pos_y_write(vi_pipe, af_cfg->config.crop.y);
    ot_ext_af_crop_h_size_write(vi_pipe, af_cfg->config.crop.width);  /* alignment 8byte */
    ot_ext_af_crop_v_size_write(vi_pipe, af_cfg->config.crop.height); /* alignment 2byte */

    ot_ext_af_fe_crop_enable_write(vi_pipe, af_cfg->config.fe_crop.enable);
    ot_ext_af_fe_crop_pos_x_write(vi_pipe, af_cfg->config.fe_crop.x);
    ot_ext_af_fe_crop_pos_y_write(vi_pipe, af_cfg->config.fe_crop.y);
    ot_ext_af_fe_crop_h_size_write(vi_pipe, af_cfg->config.fe_crop.width);  /* alignment 8byte */
    ot_ext_af_fe_crop_v_size_write(vi_pipe, af_cfg->config.fe_crop.height); /* alignment 2byte */
}

static td_void isp_af_raw_cfg_ext_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    td_u8 stat_pos;

    stat_pos = af_cfg->config.stats_pos;
    ot_ext_af_pos_sel_write(vi_pipe, stat_pos);
    ot_ext_af_rawmode_write(vi_pipe, ~((stat_pos >> 0x1) & 0x1));
    ot_ext_af_gain_limit_write(vi_pipe, af_cfg->config.raw_cfg.gamma_gain_limit);
    ot_ext_af_gamma_write(vi_pipe, af_cfg->config.raw_cfg.gamma_value);
    ot_ext_af_bayermode_write(vi_pipe, af_cfg->config.raw_cfg.bayer_format);
}

static td_void isp_af_pre_median_filter_ext_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_mean_enable_write(vi_pipe, af_cfg->config.pre_flt_cfg.enable);
    ot_ext_af_mean_thres_write(vi_pipe, af_cfg->config.pre_flt_cfg.strength);
}

static td_void isp_af_level_depend_gain_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir0_ldg_enable_write(vi_pipe, af_cfg->h_param_iir0.level_depend.enable);
    ot_ext_af_iir_thre0_low_write(vi_pipe, af_cfg->h_param_iir0.level_depend.threshold_low);
    ot_ext_af_iir_thre0_high_write(vi_pipe, af_cfg->h_param_iir0.level_depend.threshold_high);
    ot_ext_af_iir_slope0_low_write(vi_pipe, af_cfg->h_param_iir0.level_depend.slope_low);
    ot_ext_af_iir_slope0_high_write(vi_pipe, af_cfg->h_param_iir0.level_depend.slope_high);
    ot_ext_af_iir_gain0_low_write(vi_pipe, af_cfg->h_param_iir0.level_depend.gain_low);
    ot_ext_af_iir_gain0_high_write(vi_pipe, af_cfg->h_param_iir0.level_depend.gain_high);

    ot_ext_af_iir1_ldg_enable_write(vi_pipe, af_cfg->h_param_iir1.level_depend.enable);
    ot_ext_af_iir_thre1_low_write(vi_pipe, af_cfg->h_param_iir1.level_depend.threshold_low);
    ot_ext_af_iir_thre1_high_write(vi_pipe, af_cfg->h_param_iir1.level_depend.threshold_high);
    ot_ext_af_iir_slope1_low_write(vi_pipe, af_cfg->h_param_iir1.level_depend.slope_low);
    ot_ext_af_iir_slope1_high_write(vi_pipe, af_cfg->h_param_iir1.level_depend.slope_high);
    ot_ext_af_iir_gain1_low_write(vi_pipe, af_cfg->h_param_iir1.level_depend.gain_low);
    ot_ext_af_iir_gain1_high_write(vi_pipe, af_cfg->h_param_iir1.level_depend.gain_high);

    ot_ext_af_fir0_ldg_enable_write(vi_pipe, af_cfg->v_param_fir0.level_depend.enable);
    ot_ext_af_fir_thre0_low_write(vi_pipe, af_cfg->v_param_fir0.level_depend.threshold_low);
    ot_ext_af_fir_thre0_high_write(vi_pipe, af_cfg->v_param_fir0.level_depend.threshold_high);
    ot_ext_af_fir_slope0_low_write(vi_pipe, af_cfg->v_param_fir0.level_depend.slope_low);
    ot_ext_af_fir_slope0_high_write(vi_pipe, af_cfg->v_param_fir0.level_depend.slope_high);
    ot_ext_af_fir_gain0_low_write(vi_pipe, af_cfg->v_param_fir0.level_depend.gain_low);
    ot_ext_af_fir_gain0_high_write(vi_pipe, af_cfg->v_param_fir0.level_depend.gain_high);

    ot_ext_af_fir1_ldg_enable_write(vi_pipe, af_cfg->v_param_fir1.level_depend.enable);
    ot_ext_af_fir_thre1_low_write(vi_pipe, af_cfg->v_param_fir1.level_depend.threshold_low);
    ot_ext_af_fir_thre1_high_write(vi_pipe, af_cfg->v_param_fir1.level_depend.threshold_high);
    ot_ext_af_fir_slope1_low_write(vi_pipe, af_cfg->v_param_fir1.level_depend.slope_low);
    ot_ext_af_fir_slope1_high_write(vi_pipe, af_cfg->v_param_fir1.level_depend.slope_high);
    ot_ext_af_fir_gain1_low_write(vi_pipe, af_cfg->v_param_fir1.level_depend.gain_low);
    ot_ext_af_fir_gain1_high_write(vi_pipe, af_cfg->v_param_fir1.level_depend.gain_high);
}

static td_void isp_af_coring_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_iir_thre0_coring_write(vi_pipe, af_cfg->h_param_iir0.coring.threshold);
    ot_ext_af_iir_slope0_coring_write(vi_pipe, af_cfg->h_param_iir0.coring.slope);
    ot_ext_af_iir_peak0_coring_write(vi_pipe, af_cfg->h_param_iir0.coring.limit);

    ot_ext_af_iir_thre1_coring_write(vi_pipe, af_cfg->h_param_iir1.coring.threshold);
    ot_ext_af_iir_slope1_coring_write(vi_pipe, af_cfg->h_param_iir1.coring.slope);
    ot_ext_af_iir_peak1_coring_write(vi_pipe, af_cfg->h_param_iir1.coring.limit);

    ot_ext_af_fir_thre0_coring_write(vi_pipe, af_cfg->v_param_fir0.coring.threshold);
    ot_ext_af_fir_slope0_coring_write(vi_pipe, af_cfg->v_param_fir0.coring.slope);
    ot_ext_af_fir_peak0_coring_write(vi_pipe, af_cfg->v_param_fir0.coring.limit);

    ot_ext_af_fir_thre1_coring_write(vi_pipe, af_cfg->v_param_fir1.coring.threshold);
    ot_ext_af_fir_slope1_coring_write(vi_pipe, af_cfg->v_param_fir1.coring.slope);
    ot_ext_af_fir_peak1_coring_write(vi_pipe, af_cfg->v_param_fir1.coring.limit);
}

static td_void isp_af_output_shift_ext_reg_init(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    ot_ext_af_acc_shift0_h_write(vi_pipe, af_cfg->fv_param.acc_shift_h[0]); /* array index 0 */
    ot_ext_af_acc_shift1_h_write(vi_pipe, af_cfg->fv_param.acc_shift_h[1]); /* array index 1 */
    ot_ext_af_acc_shift0_v_write(vi_pipe, af_cfg->fv_param.acc_shift_v[0]); /* array index 0 */
    ot_ext_af_acc_shift1_v_write(vi_pipe, af_cfg->fv_param.acc_shift_v[1]); /* array index 1 */
    ot_ext_af_acc_shift_y_write(vi_pipe, af_cfg->fv_param.acc_shift_y);
    ot_ext_af_shift_count_y_write(vi_pipe, af_cfg->fv_param.hl_cnt_shift);
}

td_void isp_af_stats_config_write(ot_vi_pipe vi_pipe, const ot_isp_focus_stats_cfg *af_cfg)
{
    td_u8 af_enable;
    af_enable = (af_cfg->config.af_en == TD_TRUE) ? 0x3 : 0x0;
    ot_ext_system_af_enable_write(vi_pipe, af_enable);

    /* iir en */
    isp_af_iir_en_ext_reg_init(vi_pipe, af_cfg);

    ot_ext_af_peakmode_write(vi_pipe, af_cfg->config.peak_mode);
    ot_ext_af_squmode_write(vi_pipe, af_cfg->config.square_mode);
    ot_ext_af_window_hnum_write(vi_pipe, af_cfg->config.zone_col);
    ot_ext_af_window_vnum_write(vi_pipe, af_cfg->config.zone_row);

    /* iir gain */
    isp_af_iir_gain_ext_reg_init(vi_pipe, af_cfg);

    /* iir shift */
    isp_af_iir_shift_ext_reg_init(vi_pipe, af_cfg);

    /* fir gain */
    isp_af_fir_gain_ext_reg_init(vi_pipe, af_cfg);

    /* ds */
    isp_af_ds_ext_reg_init(vi_pipe, af_cfg);

    /* PLG and PLS */
    isp_af_plg_pls_ext_reg_init(vi_pipe, af_cfg);

    /* AF crop */
    isp_af_crop_ext_reg_init(vi_pipe, af_cfg);

    /* AF raw cfg */
    isp_af_raw_cfg_ext_init(vi_pipe, af_cfg);

    /* AF pre median filter */
    isp_af_pre_median_filter_ext_init(vi_pipe, af_cfg);

    /* level depend gain */
    isp_af_level_depend_gain_ext_reg_init(vi_pipe, af_cfg);

    /* AF coring */
    isp_af_coring_ext_reg_init(vi_pipe, af_cfg);

    /* high luma counter */
    ot_ext_af_highlight_thre_write(vi_pipe, (td_u8)af_cfg->config.high_luma_threshold);

    /* AF output shift */
    isp_af_output_shift_ext_reg_init(vi_pipe, af_cfg);
}

td_void isp_fswdr_attr_write(ot_vi_pipe vi_pipe, const ot_isp_wdr_fs_attr *fswdr_attr)
{
    td_u32 i, j, idx;
    ot_ext_system_fusion_mode_write(vi_pipe, fswdr_attr->wdr_merge_mode);
    ot_ext_system_mdt_en_write(vi_pipe, fswdr_attr->wdr_combine.motion_comp);

    ot_ext_system_wdr_shortthr_write(vi_pipe, fswdr_attr->wdr_combine.short_threshold);
    ot_ext_system_wdr_longthr_write(vi_pipe, fswdr_attr->wdr_combine.long_threshold);
    ot_ext_system_wdr_forcelong_en_write(vi_pipe, fswdr_attr->wdr_combine.force_long);
    ot_ext_system_wdr_forcelong_low_thd_write(vi_pipe, fswdr_attr->wdr_combine.force_long_low_threshold);
    ot_ext_system_wdr_forcelong_high_thd_write(vi_pipe, fswdr_attr->wdr_combine.force_long_hig_threshold);

    ot_ext_system_wdr_shortexpo_chk_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.short_expo_chk);
    ot_ext_system_wdr_shortcheck_thd_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.short_check_threshold);
    ot_ext_system_wdr_mdref_flicker_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.md_ref_flicker);
    ot_ext_system_wdr_mdt_still_thr_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.mdt_still_threshold);
    ot_ext_system_wdr_mdt_full_thr_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.mdt_full_threshold);
    ot_ext_system_wdr_mdt_long_blend_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.mdt_long_blend);
    ot_ext_system_wdr_manual_mode_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.op_type);

    ot_ext_system_wdr_manual_mdthr_low_gain_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.manual_attr.md_thr_low_gain);
    ot_ext_system_wdr_manual_mdthr_hig_gain_write(vi_pipe, fswdr_attr->wdr_combine.wdr_mdt.manual_attr.md_thr_hig_gain);

    for (i = 0; i < OT_ISP_WDR_RATIO_NUM; i++) {
        for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
            idx = i * OT_ISP_AUTO_ISO_NUM + j;
            ot_ext_system_wdr_auto_mdthr_low_gain_write(vi_pipe, idx,
                fswdr_attr->wdr_combine.wdr_mdt.auto_attr.md_thr_low_gain[i][j]);
            ot_ext_system_wdr_auto_mdthr_hig_gain_write(vi_pipe, idx,
                fswdr_attr->wdr_combine.wdr_mdt.auto_attr.md_thr_hig_gain[i][j]);
        }
    }

    for (j = 0; j < OT_ISP_WDR_MAX_FRAME_NUM; j++) {
        ot_ext_system_fusion_thr_write(vi_pipe, j, fswdr_attr->fusion_attr.fusion_threshold[j]);
    }
    ot_ext_system_fusion_blend_en_write(vi_pipe, fswdr_attr->fusion_attr.fusion_blend_en);

    ot_ext_system_wdr_fusion_blend_wgt_write(vi_pipe, fswdr_attr->fusion_attr.fusion_blend_wgt);

    ot_ext_system_fusion_force_gray_en_write(vi_pipe, fswdr_attr->fusion_attr.fusion_force_gray_en);

    ot_ext_system_fusion_force_blend_threshold_write(vi_pipe, fswdr_attr->fusion_attr.fusion_force_blend_threshold);
}

td_void isp_fswdr_attr_read(ot_vi_pipe vi_pipe, ot_isp_wdr_fs_attr *fswdr_attr)
{
    td_u32 i, j, idx;
    fswdr_attr->wdr_merge_mode                = ot_ext_system_fusion_mode_read(vi_pipe);
    fswdr_attr->wdr_combine.motion_comp       = ot_ext_system_mdt_en_read(vi_pipe);

    fswdr_attr->wdr_combine.short_threshold   = ot_ext_system_wdr_shortthr_read(vi_pipe);
    fswdr_attr->wdr_combine.long_threshold    = ot_ext_system_wdr_longthr_read(vi_pipe);

    fswdr_attr->wdr_combine.force_long               = ot_ext_system_wdr_forcelong_en_read(vi_pipe);
    fswdr_attr->wdr_combine.force_long_low_threshold = ot_ext_system_wdr_forcelong_low_thd_read(vi_pipe);
    fswdr_attr->wdr_combine.force_long_hig_threshold = ot_ext_system_wdr_forcelong_high_thd_read(vi_pipe);

    fswdr_attr->wdr_combine.wdr_mdt.short_expo_chk        = ot_ext_system_wdr_shortexpo_chk_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.short_check_threshold = ot_ext_system_wdr_shortcheck_thd_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.md_ref_flicker        = ot_ext_system_wdr_mdref_flicker_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.mdt_still_threshold   = ot_ext_system_wdr_mdt_still_thr_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.mdt_full_threshold    = ot_ext_system_wdr_mdt_full_thr_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.mdt_long_blend        = ot_ext_system_wdr_mdt_long_blend_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.op_type               = ot_ext_system_wdr_manual_mode_read(vi_pipe);

    fswdr_attr->wdr_combine.wdr_mdt.manual_attr.md_thr_low_gain = ot_ext_system_wdr_manual_mdthr_low_gain_read(vi_pipe);
    fswdr_attr->wdr_combine.wdr_mdt.manual_attr.md_thr_hig_gain = ot_ext_system_wdr_manual_mdthr_hig_gain_read(vi_pipe);

    for (i = 0; i < OT_ISP_WDR_RATIO_NUM; i++) {
        for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
            idx = i * OT_ISP_AUTO_ISO_NUM + j;
            fswdr_attr->wdr_combine.wdr_mdt.auto_attr.md_thr_low_gain[i][j] =
                ot_ext_system_wdr_auto_mdthr_low_gain_read(vi_pipe, idx);
            fswdr_attr->wdr_combine.wdr_mdt.auto_attr.md_thr_hig_gain[i][j] =
                ot_ext_system_wdr_auto_mdthr_hig_gain_read(vi_pipe, idx);
        }
    }

    for (j = 0; j < OT_ISP_WDR_MAX_FRAME_NUM; j++) {
        fswdr_attr->fusion_attr.fusion_threshold[j] = ot_ext_system_fusion_thr_read(vi_pipe, j);
    }
    fswdr_attr->fusion_attr.fusion_blend_en = ot_ext_system_fusion_blend_en_read(vi_pipe);

    fswdr_attr->fusion_attr.fusion_blend_wgt = ot_ext_system_wdr_fusion_blend_wgt_read(vi_pipe);

    fswdr_attr->fusion_attr.fusion_force_gray_en = ot_ext_system_fusion_force_gray_en_read(vi_pipe);

    fswdr_attr->fusion_attr.fusion_force_blend_threshold = ot_ext_system_fusion_force_blend_threshold_read(vi_pipe);
}

td_void isp_expander_attr_write(ot_vi_pipe vi_pipe, const ot_isp_expander_attr *expander_attr)
{
    td_u16 i;
    isp_check_pointer_void_return(expander_attr);
    ot_ext_system_expander_en_write(vi_pipe, expander_attr->enable);
    ot_ext_system_expander_bit_depth_in_write(vi_pipe, expander_attr->bit_depth_in);
    ot_ext_system_expander_bit_depth_out_write(vi_pipe, expander_attr->bit_depth_out);
    ot_ext_system_expander_knee_point_num_write(vi_pipe, expander_attr->knee_point_num);
    for (i = 0; i < expander_attr->knee_point_num; i++) {
        ot_ext_system_expander_knee_x_coord_write(vi_pipe, i, expander_attr->knee_point_coord[i].x);
        ot_ext_system_expander_knee_y_coord_write(vi_pipe, i, expander_attr->knee_point_coord[i].y);
    }
}

td_void isp_expander_attr_read(ot_vi_pipe vi_pipe, ot_isp_expander_attr *expander_attr)
{
    td_u16 i;
    isp_check_pointer_void_return(expander_attr);
    expander_attr->enable             = ot_ext_system_expander_en_read(vi_pipe);
    expander_attr->bit_depth_in   = ot_ext_system_expander_bit_depth_in_read(vi_pipe);
    expander_attr->bit_depth_out  = ot_ext_system_expander_bit_depth_out_read(vi_pipe);
    expander_attr->knee_point_num = ot_ext_system_expander_knee_point_num_read(vi_pipe);

    for (i = 0; i < OT_ISP_EXPANDER_POINT_NUM_MAX; i++) {
        if (i < expander_attr->knee_point_num) {
            expander_attr->knee_point_coord[i].x = ot_ext_system_expander_knee_x_coord_read(vi_pipe, i);
            expander_attr->knee_point_coord[i].y = ot_ext_system_expander_knee_y_coord_read(vi_pipe, i);
        } else {
            expander_attr->knee_point_coord[i].x = 0x100;
            expander_attr->knee_point_coord[i].y = 0x100000;
        }
    }
}

td_void isp_get_fe_roi_config(ot_vi_pipe vi_pipe, ot_isp_fe_roi_cfg *fe_roi_cfg)
{
    fe_roi_cfg->enable = ot_ext_system_fe_roi_en_read(vi_pipe);
    fe_roi_cfg->roi_rect.x = ot_ext_system_fe_roi_x_read(vi_pipe);
    fe_roi_cfg->roi_rect.y = ot_ext_system_fe_roi_y_read(vi_pipe);
    fe_roi_cfg->roi_rect.width = ot_ext_system_fe_roi_width_read(vi_pipe);
    fe_roi_cfg->roi_rect.height = ot_ext_system_fe_roi_height_read(vi_pipe);
}

td_void isp_set_mesh_shading_gain_lut_attr(ot_vi_pipe vi_pipe,
    const ot_isp_shading_lut_attr *shading_lut_attr)
{
    td_s32 i;
    ot_ext_system_isp_mesh_shading_mesh_scale_write(vi_pipe, shading_lut_attr->mesh_scale);
    for (i = 0; i < OT_ISP_LSC_GRID_POINTS; i++) {
        ot_ext_system_isp_mesh_shading_r_gain0_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[0].r_gain[i]);
        ot_ext_system_isp_mesh_shading_gr_gain0_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[0].gr_gain[i]);
        ot_ext_system_isp_mesh_shading_gb_gain0_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[0].gb_gain[i]);
        ot_ext_system_isp_mesh_shading_b_gain0_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[0].b_gain[i]);

        ot_ext_system_isp_mesh_shading_r_gain1_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[1].r_gain[i]);
        ot_ext_system_isp_mesh_shading_gr_gain1_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[1].gr_gain[i]);
        ot_ext_system_isp_mesh_shading_gb_gain1_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[1].gb_gain[i]);
        ot_ext_system_isp_mesh_shading_b_gain1_write(vi_pipe, i, shading_lut_attr->lsc_gain_lut[1].b_gain[i]);
    }

    for (i = 0; i < OT_ISP_MLSC_X_HALF_GRID_NUM; i++) {
        ot_ext_system_isp_mesh_shading_xgrid_write(vi_pipe, i, shading_lut_attr->x_grid_width[i]);
    }

    for (i = 0; i < OT_ISP_MLSC_Y_HALF_GRID_NUM; i++) {
        ot_ext_system_isp_mesh_shading_ygrid_write(vi_pipe, i, shading_lut_attr->y_grid_width[i]);
    }

    ot_ext_system_isp_mesh_shading_lut_attr_updata_write(vi_pipe, TD_TRUE);
}

td_void isp_set_mesh_shading_attr(ot_vi_pipe vi_pipe, const ot_isp_shading_attr *shading_attr)
{
    ot_ext_system_isp_mesh_shading_enable_write(vi_pipe, shading_attr->enable);
    ot_ext_system_isp_mesh_shading_mesh_strength_write(vi_pipe, shading_attr->mesh_strength);
    ot_ext_system_isp_mesh_shading_blendratio_write(vi_pipe, shading_attr->blend_ratio);
    ot_ext_system_isp_mesh_shading_attr_updata_write(vi_pipe, TD_TRUE);
}