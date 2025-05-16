/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "isp_alg.h"
#include "ot_common_isp.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_ext_reg_access.h"

static ot_isp_focus_stats_cfg  g_focus_cfg[OT_ISP_MAX_PIPE_NUM] = {
    [0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = {
        {1, 17, 15, 1, 0, {0, 0, 0, 1920, 1080}, {0, 0, 0, 1920, 1080}, 0, {0x0, 0x1, 0}, {1, 0x9bff}, 0xf0},
        {
            1, {1, 1, 1}, 15, {188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 240, 8, 14},
            {127, 12, 2047}
        },
        {
            0, {1, 1, 0},  2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0},
            { 15, 12, 2047 }
        },
        {{ 20,  16, 0, -16, -20 }, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800}},
        {{ -12, -24, 0,  24,  12 }, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047}},
        {4, {0, 0}, {1, 1}, 0}
    }
};

static td_void isp_af_ext_regs_default(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_focus_stats_cfg *af_cfg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    af_cfg = &g_focus_cfg[vi_pipe];

    af_cfg->config.crop.x = 0;
    af_cfg->config.crop.y = 0;
    af_cfg->config.crop.width = isp_ctx->block_attr.frame_rect.width / 8 * 8;   /* alignment 8byte */
    af_cfg->config.crop.height = isp_ctx->block_attr.frame_rect.height / 2 * 2;   /* alignment 2byte */

    af_cfg->config.fe_crop.x = 0;
    af_cfg->config.fe_crop.y = 0;
    af_cfg->config.fe_crop.width = isp_ctx->sys_rect.width / 8 * 8;   /* alignment 8byte */
    af_cfg->config.fe_crop.height = isp_ctx->sys_rect.height / 2 * 2;   /* alignment 2byte */

    af_cfg->config.raw_cfg.bayer_format = (ot_isp_bayer_format)ot_ext_system_rggb_cfg_read(vi_pipe);

    isp_af_stats_config_write(vi_pipe, af_cfg);

    ot_ext_af_set_flag_write(vi_pipe, OT_EXT_AF_SET_FLAG_DISABLE);
}

static td_void isp_af_blk_crop(isp_af_reg_cfg *af_reg_cfg, isp_usr_ctx *isp_ctx, td_u8 index, td_u8 block_num)
{
    td_u16 overlap;
    isp_rect block_rect;

    isp_get_block_rect(&block_rect, &isp_ctx->block_attr, index);

    overlap = isp_ctx->block_attr.over_lap;

    af_reg_cfg->crop_enable = TD_TRUE;
    af_reg_cfg->crop_pos_y  = block_rect.y;
    af_reg_cfg->crop_v_size = block_rect.height;

    if (index == 0) {
        if (block_num > 1) {
            af_reg_cfg->crop_pos_x  = 0;
            af_reg_cfg->crop_h_size = block_rect.width - overlap;
        } else {
            af_reg_cfg->crop_pos_x  = 0;
            af_reg_cfg->crop_h_size = block_rect.width;
        }
    } else if (index == (block_num - 1)) {
        af_reg_cfg->crop_pos_x   = overlap;
        af_reg_cfg->crop_h_size  = block_rect.width - overlap;
    } else {
        af_reg_cfg->crop_pos_x   = overlap;
        af_reg_cfg->crop_h_size  = block_rect.width - (overlap << 1);
    }
}

static td_void isp_af_be_blk_cfg(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_usr_ctx *isp_ctx)
{
    td_u8  i;
    td_u8  block_num   = reg_cfg->cfg_num;
    td_u8  window_hnum = ot_ext_af_window_hnum_read(vi_pipe);
    td_u8  window_vnum = ot_ext_af_window_vnum_read(vi_pipe);
    isp_af_reg_cfg *af_default_cfg = TD_NULL;

    for (i = 0; i < block_num; i++) {
        af_default_cfg = &reg_cfg->alg_reg_cfg[i].be_af_reg_cfg;

        if (i < window_hnum % div_0_to_1(block_num)) {
            af_default_cfg->window_hnum = window_hnum / div_0_to_1(block_num) + 1;
        } else {
            af_default_cfg->window_hnum = window_hnum / div_0_to_1(block_num);
        }

        af_default_cfg->window_vnum = window_vnum;

        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[i].be_af_zone_cfg.column = af_default_cfg->window_hnum;
        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[i].be_af_zone_cfg.row   = af_default_cfg->window_vnum;

        if (block_num == 1) {
            af_default_cfg->crop_enable = ot_ext_af_crop_enable_read(vi_pipe);
            af_default_cfg->crop_pos_y  = ot_ext_af_crop_pos_y_read(vi_pipe);
            af_default_cfg->crop_pos_x  = ot_ext_af_crop_pos_x_read(vi_pipe);
            af_default_cfg->crop_h_size = ot_ext_af_crop_h_size_read(vi_pipe);
            af_default_cfg->crop_v_size = ot_ext_af_crop_v_size_read(vi_pipe);
        } else {
            isp_af_blk_crop(af_default_cfg, isp_ctx, i, block_num);
        }
    }
}

static td_void isp_af_fe_blk_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->window_hnum = ot_ext_af_window_hnum_read(vi_pipe);
    af_cfg->window_vnum = ot_ext_af_window_vnum_read(vi_pipe);

    af_cfg->fe_crop_enable = ot_ext_af_fe_crop_enable_read(vi_pipe);
    af_cfg->fe_crop_pos_y  = ot_ext_af_fe_crop_pos_y_read(vi_pipe);
    af_cfg->fe_crop_pos_x  = ot_ext_af_fe_crop_pos_x_read(vi_pipe);
    af_cfg->fe_crop_h_size = ot_ext_af_fe_crop_h_size_read(vi_pipe);
    af_cfg->fe_crop_v_size = ot_ext_af_fe_crop_v_size_read(vi_pipe);
}

static td_void isp_af_pls_plg_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir_plg_group0 = ot_ext_af_iir_plg_group0_read(vi_pipe);
    af_cfg->iir_pls_group0 = ot_ext_af_iir_pls_group0_read(vi_pipe);
    af_cfg->iir_plg_group1 = ot_ext_af_iir_plg_group1_read(vi_pipe);
    af_cfg->iir_pls_group1 = ot_ext_af_iir_pls_group1_read(vi_pipe);
}

static td_void isp_af_ds_en_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir0_ds_enable = ot_ext_af_iir0_ds_enable_read(vi_pipe);
    af_cfg->iir1_ds_enable = ot_ext_af_iir1_ds_enable_read(vi_pipe);
    af_cfg->iir_dilate0    = af_cfg->iir0_ds_enable * 3 + 3; /* ds enable(3)->dilate */
    af_cfg->iir_dilate1    = af_cfg->iir1_ds_enable * 3 + 3; /* ds enable(3)->dilate */
}

static td_void isp_af_iir_en_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir0_enable0 = ot_ext_af_iir0_enable0_read(vi_pipe);
    af_cfg->iir0_enable1 = ot_ext_af_iir0_enable1_read(vi_pipe);
    af_cfg->iir0_enable2 = ot_ext_af_iir0_enable2_read(vi_pipe);
    af_cfg->iir1_enable0 = ot_ext_af_iir1_enable0_read(vi_pipe);
    af_cfg->iir1_enable1 = ot_ext_af_iir1_enable1_read(vi_pipe);
    af_cfg->iir1_enable2 = ot_ext_af_iir1_enable2_read(vi_pipe);
}

static td_void isp_af_iir_gain_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir_gain0_group0 = ot_ext_af_iir_gain0_group0_read(vi_pipe);
    af_cfg->iir_gain0_group1 = ot_ext_af_iir_gain0_group1_read(vi_pipe);
    af_cfg->iir_gain1_group0 = ot_ext_af_iir_gain1_group0_read(vi_pipe);
    af_cfg->iir_gain1_group1 = ot_ext_af_iir_gain1_group1_read(vi_pipe);
    af_cfg->iir_gain2_group0 = ot_ext_af_iir_gain2_group0_read(vi_pipe);
    af_cfg->iir_gain2_group1 = ot_ext_af_iir_gain2_group1_read(vi_pipe);
    af_cfg->iir_gain3_group0 = ot_ext_af_iir_gain3_group0_read(vi_pipe);
    af_cfg->iir_gain3_group1 = ot_ext_af_iir_gain3_group1_read(vi_pipe);
    af_cfg->iir_gain4_group0 = ot_ext_af_iir_gain4_group0_read(vi_pipe);
    af_cfg->iir_gain4_group1 = ot_ext_af_iir_gain4_group1_read(vi_pipe);
    af_cfg->iir_gain5_group0 = ot_ext_af_iir_gain5_group0_read(vi_pipe);
    af_cfg->iir_gain5_group1 = ot_ext_af_iir_gain5_group1_read(vi_pipe);
    af_cfg->iir_gain6_group0 = ot_ext_af_iir_gain6_group0_read(vi_pipe);
    af_cfg->iir_gain6_group1 = ot_ext_af_iir_gain6_group1_read(vi_pipe);
}

static td_void isp_af_iir_shift_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir0_shift_group0 = ot_ext_af_iir0_shift_group0_read(vi_pipe);
    af_cfg->iir1_shift_group0 = ot_ext_af_iir1_shift_group0_read(vi_pipe);
    af_cfg->iir2_shift_group0 = ot_ext_af_iir2_shift_group0_read(vi_pipe);
    af_cfg->iir3_shift_group0 = ot_ext_af_iir3_shift_group0_read(vi_pipe);
    af_cfg->iir0_shift_group1 = ot_ext_af_iir0_shift_group1_read(vi_pipe);
    af_cfg->iir1_shift_group1 = ot_ext_af_iir1_shift_group1_read(vi_pipe);
    af_cfg->iir2_shift_group1 = ot_ext_af_iir2_shift_group1_read(vi_pipe);
    af_cfg->iir3_shift_group1 = ot_ext_af_iir3_shift_group1_read(vi_pipe);
}

static td_void isp_af_fir_gain_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->fir_h_gain0_group0 = ot_ext_af_fir_h_gain0_group0_read(vi_pipe);
    af_cfg->fir_h_gain0_group1 = ot_ext_af_fir_h_gain0_group1_read(vi_pipe);
    af_cfg->fir_h_gain1_group0 = ot_ext_af_fir_h_gain1_group0_read(vi_pipe);
    af_cfg->fir_h_gain1_group1 = ot_ext_af_fir_h_gain1_group1_read(vi_pipe);
    af_cfg->fir_h_gain2_group0 = ot_ext_af_fir_h_gain2_group0_read(vi_pipe);
    af_cfg->fir_h_gain2_group1 = ot_ext_af_fir_h_gain2_group1_read(vi_pipe);
    af_cfg->fir_h_gain3_group0 = ot_ext_af_fir_h_gain3_group0_read(vi_pipe);
    af_cfg->fir_h_gain3_group1 = ot_ext_af_fir_h_gain3_group1_read(vi_pipe);
    af_cfg->fir_h_gain4_group0 = ot_ext_af_fir_h_gain4_group0_read(vi_pipe);
    af_cfg->fir_h_gain4_group1 = ot_ext_af_fir_h_gain4_group1_read(vi_pipe);
}

static td_void isp_af_be_raw_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    af_cfg->af_pos_sel = ot_ext_af_pos_sel_read(vi_pipe);
    af_cfg->raw_mode   = ot_ext_af_rawmode_read(vi_pipe);
    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        af_cfg->af_pos_sel = OT_ISP_AF_STATS_AFTER_CSC;
        af_cfg->raw_mode   = 0;
    }
    af_cfg->gain_limit     = ot_ext_af_gain_limit_read(vi_pipe);
    af_cfg->gamma          = ot_ext_af_gamma_read(vi_pipe);
}

static td_void isp_af_fe_raw_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->af_pos_sel         = 0;
    af_cfg->raw_mode           = 0x1; /* FE,raw mode only */
    af_cfg->gain_limit         = ot_ext_af_gain_limit_read(vi_pipe);
    af_cfg->gamma              = ot_ext_af_gamma_read(vi_pipe);
}

static td_void isp_af_pre_filter_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->mean_enable = ot_ext_af_mean_enable_read(vi_pipe);
    af_cfg->mean_thres  = ot_ext_af_mean_thres_read(vi_pipe);
}

static td_void isp_af_level_depend_gain_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir0_ldg_enable = ot_ext_af_iir0_ldg_enable_read(vi_pipe);
    af_cfg->iir_thre0_low   = ot_ext_af_iir_thre0_low_read(vi_pipe);
    af_cfg->iir_thre0_high  = ot_ext_af_iir_thre0_high_read(vi_pipe);
    af_cfg->iir_slope0_low  = ot_ext_af_iir_slope0_low_read(vi_pipe);
    af_cfg->iir_slope0_high = ot_ext_af_iir_slope0_high_read(vi_pipe);
    af_cfg->iir_gain0_low   = ot_ext_af_iir_gain0_low_read(vi_pipe);
    af_cfg->iir_gain0_high  = ot_ext_af_iir_gain0_high_read(vi_pipe);

    af_cfg->iir1_ldg_enable = ot_ext_af_iir1_ldg_enable_read(vi_pipe);
    af_cfg->iir_thre1_low   = ot_ext_af_iir_thre1_low_read(vi_pipe);
    af_cfg->iir_thre1_high  = ot_ext_af_iir_thre1_high_read(vi_pipe);
    af_cfg->iir_slope1_low  = ot_ext_af_iir_slope1_low_read(vi_pipe);
    af_cfg->iir_slope1_high = ot_ext_af_iir_slope1_high_read(vi_pipe);
    af_cfg->iir_gain1_low   = ot_ext_af_iir_gain1_low_read(vi_pipe);
    af_cfg->iir_gain1_high  = ot_ext_af_iir_gain1_high_read(vi_pipe);

    af_cfg->fir0_ldg_enable = ot_ext_af_fir0_ldg_enable_read(vi_pipe);
    af_cfg->fir_thre0_low   = ot_ext_af_fir_thre0_low_read(vi_pipe);
    af_cfg->fir_thre0_high  = ot_ext_af_fir_thre0_high_read(vi_pipe);
    af_cfg->fir_slope0_low  = ot_ext_af_fir_slope0_low_read(vi_pipe);
    af_cfg->fir_slope0_high = ot_ext_af_fir_slope0_high_read(vi_pipe);
    af_cfg->fir_gain0_low   = ot_ext_af_fir_gain0_low_read(vi_pipe);
    af_cfg->fir_gain0_high  = ot_ext_af_fir_gain0_high_read(vi_pipe);

    af_cfg->fir1_ldg_enable = ot_ext_af_fir1_ldg_enable_read(vi_pipe);
    af_cfg->fir_thre1_low   = ot_ext_af_fir_thre1_low_read(vi_pipe);
    af_cfg->fir_thre1_high  = ot_ext_af_fir_thre1_high_read(vi_pipe);
    af_cfg->fir_slope1_low  = ot_ext_af_fir_slope1_low_read(vi_pipe);
    af_cfg->fir_slope1_high = ot_ext_af_fir_slope1_high_read(vi_pipe);
    af_cfg->fir_gain1_low   = ot_ext_af_fir_gain1_low_read(vi_pipe);
    af_cfg->fir_gain1_high  = ot_ext_af_fir_gain1_high_read(vi_pipe);
}

static td_void isp_af_coring_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->iir_thre0_coring  = ot_ext_af_iir_thre0_coring_read(vi_pipe);
    af_cfg->iir_slope0_coring = ot_ext_af_iir_slope0_coring_read(vi_pipe);
    af_cfg->iir_peak0_coring  = ot_ext_af_iir_peak0_coring_read(vi_pipe);

    af_cfg->iir_thre1_coring  = ot_ext_af_iir_thre1_coring_read(vi_pipe);
    af_cfg->iir_slope1_coring = ot_ext_af_iir_slope1_coring_read(vi_pipe);
    af_cfg->iir_peak1_coring  = ot_ext_af_iir_peak1_coring_read(vi_pipe);

    af_cfg->fir_thre0_coring  = ot_ext_af_fir_thre0_coring_read(vi_pipe);
    af_cfg->fir_slope0_coring = ot_ext_af_fir_slope0_coring_read(vi_pipe);
    af_cfg->fir_peak0_coring  = ot_ext_af_fir_peak0_coring_read(vi_pipe);

    af_cfg->fir_thre1_coring  = ot_ext_af_fir_thre1_coring_read(vi_pipe);
    af_cfg->fir_slope1_coring = ot_ext_af_fir_slope1_coring_read(vi_pipe);
    af_cfg->fir_peak1_coring  = ot_ext_af_fir_peak1_coring_read(vi_pipe);
}

static td_void isp_af_output_shift_reg_cfg(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->acc_shift0_h  = ot_ext_af_acc_shift0_h_read(vi_pipe);
    af_cfg->acc_shift1_h  = ot_ext_af_acc_shift1_h_read(vi_pipe);
    af_cfg->acc_shift0_v  = ot_ext_af_acc_shift0_v_read(vi_pipe);
    af_cfg->acc_shift1_v  = ot_ext_af_acc_shift1_v_read(vi_pipe);
    af_cfg->acc_shift_y   = ot_ext_af_acc_shift_y_read(vi_pipe);
    af_cfg->shift_count_y = ot_ext_af_shift_count_y_read(vi_pipe);
}

static td_void isp_af_reg_update(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    af_cfg->bayer_mode = ot_ext_af_bayermode_read(vi_pipe);
    isp_af_pls_plg_reg_cfg(vi_pipe, af_cfg); /* PLG and PLS */
    isp_af_ds_en_reg_cfg(vi_pipe, af_cfg);
    isp_af_iir_en_reg_cfg(vi_pipe, af_cfg);

    af_cfg->peak_mode = ot_ext_af_peakmode_read(vi_pipe);
    af_cfg->squ_mode  = ot_ext_af_squmode_read(vi_pipe);

    isp_af_iir_gain_reg_cfg(vi_pipe, af_cfg);
    isp_af_iir_shift_reg_cfg(vi_pipe, af_cfg);
    isp_af_fir_gain_reg_cfg(vi_pipe, af_cfg);
    isp_af_pre_filter_reg_cfg(vi_pipe, af_cfg); /* AF pre median filter */
    isp_af_level_depend_gain_reg_cfg(vi_pipe, af_cfg); /* level depend gain */
    isp_af_coring_reg_cfg(vi_pipe, af_cfg);  /* AF coring */
    af_cfg->highlight_thre = ot_ext_af_highlight_thre_read(vi_pipe); /* high luma counter */
    isp_af_output_shift_reg_cfg(vi_pipe, af_cfg); /* AF output shift */
}

static td_void isp_af_update_be_working_status(ot_vi_pipe vi_pipe, isp_af_reg_cfg *af_cfg)
{
    td_u8 ai_mode = 0;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (((af_cfg->af_pos_sel == OT_ISP_AF_STATS_AFTER_DRC) && (ai_mode == OT_ISP_AI_DRC)) ||
        (af_cfg->af_pos_sel == OT_ISP_AF_STATS_AFTER_CSC)) {
        af_cfg->is_in_pre_be = TD_FALSE;
    } else {
        af_cfg->is_in_pre_be = TD_TRUE;
    }

    af_cfg->be_is_online = (is_online_mode(isp_ctx->block_attr.running_mode)) ||
        (is_pre_online_post_offline(isp_ctx->block_attr.running_mode) && (af_cfg->be_is_online == TD_TRUE));
}

static td_void isp_af_fe_reg_cfg_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    isp_af_reg_cfg *af_cfg = TD_NULL;

    af_cfg = &reg_cfg->alg_reg_cfg[0].fe_af_reg_cfg;

    af_cfg->lpf_enable      = OT_ISP_FE_AF_LPF_EN_DEFAULT;
    af_cfg->fir0_lpf_enable = OT_ISP_FE_AF_FIR0_LPF_EN_DEFAULT;
    af_cfg->fir1_lpf_enable = OT_ISP_FE_AF_FIR1_LPF_EN_DEFAULT;

    isp_af_reg_update(vi_pipe, af_cfg);

    af_cfg->af_enable = (ot_ext_system_af_enable_read(vi_pipe) & 0x1);

    isp_af_fe_raw_reg_cfg(vi_pipe, af_cfg);
    isp_af_fe_blk_cfg(vi_pipe, af_cfg);

    reg_cfg->cfg_key.bit1_af_fe_cfg = 1;
}

static td_void isp_af_be_reg_cfg_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i, block_num;
    isp_af_reg_cfg *af_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;

    for (i = 0; i < block_num; i++) {
        af_cfg = &reg_cfg->alg_reg_cfg[i].be_af_reg_cfg;

        af_cfg->lpf_enable      = OT_ISP_BE_AF_LPF_EN_DEFAULT;
        af_cfg->fir0_lpf_enable = OT_ISP_BE_AF_FIR0_LPF_EN_DEFAULT;
        af_cfg->fir1_lpf_enable = OT_ISP_BE_AF_FIR1_LPF_EN_DEFAULT;

        isp_af_reg_update(vi_pipe, af_cfg);

        /* be af enable */
        if ((isp_ctx->block_attr.frame_rect.height < OT_ISP_AF_MIN_HEIGHT) ||
            (isp_ctx->block_attr.frame_rect.width < OT_ISP_AF_MIN_WIDTH)) {
            af_cfg->af_enable = TD_FALSE;
        } else {
            af_cfg->af_enable = ((ot_ext_system_af_enable_read(vi_pipe) >> 1) & 0x1);
        }

        /* AF BE raw cfg */
        isp_af_be_raw_reg_cfg(vi_pipe, af_cfg);
        isp_af_update_be_working_status(vi_pipe, af_cfg);
        af_cfg->update_index = 1;
    }
    isp_af_be_blk_cfg(vi_pipe, reg_cfg, isp_ctx);

    reg_cfg->cfg_key.bit1_af_be_cfg  = 1;
    reg_cfg->kernel_reg_cfg.cfg_key.bit1_be_af_cfg = 1;
}

static td_void isp_af_fe_cfg_reg(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    isp_af_reg_cfg *af_cfg = TD_NULL;

    af_cfg = &reg_cfg->alg_reg_cfg[0].fe_af_reg_cfg;

    isp_af_reg_update(vi_pipe, af_cfg);

    af_cfg->af_enable = (ot_ext_system_af_enable_read(vi_pipe) & 0x1);

    isp_af_fe_raw_reg_cfg(vi_pipe, af_cfg);
    isp_af_fe_blk_cfg(vi_pipe, af_cfg);

    reg_cfg->cfg_key.bit1_af_fe_cfg = 1;
}

static td_void isp_af_be_cfg_reg(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i, block_num;
    isp_af_reg_cfg *af_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx  = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;

    for (i = 0; i < block_num; i++) {
        af_cfg = &reg_cfg->alg_reg_cfg[i].be_af_reg_cfg;

        isp_af_reg_update(vi_pipe, af_cfg);

        af_cfg->af_enable = ((ot_ext_system_af_enable_read(vi_pipe) >> 1) & 0x1);

        /* AF BE raw cfg */
        isp_af_be_raw_reg_cfg(vi_pipe, af_cfg);
        isp_af_update_be_working_status(vi_pipe, af_cfg);

        af_cfg->update_index += 1;
    }
    isp_af_be_blk_cfg(vi_pipe, reg_cfg, isp_ctx);
    reg_cfg->cfg_key.bit1_af_be_cfg  = 1;
    reg_cfg->kernel_reg_cfg.cfg_key.bit1_be_af_cfg = 1;
}

static td_void isp_af_image_res_write(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    ot_ext_af_crop_pos_x_write(vi_pipe, 0);
    ot_ext_af_crop_pos_y_write(vi_pipe, 0);
    ot_ext_af_crop_h_size_write(vi_pipe, isp_ctx->block_attr.frame_rect.width / 8 * 8);  /* alignment 8byte */
    ot_ext_af_crop_v_size_write(vi_pipe, isp_ctx->block_attr.frame_rect.height / 2 * 2); /* alignment 2byte */

    ot_ext_af_fe_crop_pos_x_write(vi_pipe, 0);
    ot_ext_af_fe_crop_pos_y_write(vi_pipe, 0);
    ot_ext_af_fe_crop_h_size_write(vi_pipe, isp_ctx->sys_rect.width / 8 * 8);  /* alignment 8byte */
    ot_ext_af_fe_crop_v_size_write(vi_pipe, isp_ctx->sys_rect.height / 2 * 2); /* alignment 2byte */

    ot_ext_af_input_h_size_write(vi_pipe, isp_ctx->block_attr.frame_rect.width);
    ot_ext_af_input_v_size_write(vi_pipe, isp_ctx->block_attr.frame_rect.height);
    isp_af_be_blk_cfg(vi_pipe, reg_cfg, isp_ctx);
    isp_af_fe_blk_cfg(vi_pipe, &reg_cfg->alg_reg_cfg[0].fe_af_reg_cfg);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].be_af_reg_cfg.update_index++;
    }

    reg_cfg->cfg_key.bit1_af_fe_cfg = 1;
}

static td_void isp_af_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->block_attr.block_num != isp_ctx->block_attr.pre_block_num) {
        isp_af_image_res_write(vi_pipe, reg_cfg);
    }

    reg_cfg->cfg_key.bit1_af_fe_cfg = 1;
}

static td_s32 isp_af_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    /* some registers about AF module shound be initial */
    isp_af_ext_regs_default(vi_pipe);
    isp_af_fe_reg_cfg_init(vi_pipe, (isp_reg_cfg *)reg_cfg);
    isp_af_be_reg_cfg_init(vi_pipe, (isp_reg_cfg *)reg_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_af_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    ot_unused(stat_info);
    ot_unused(rsv);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.snap_state == TD_TRUE) {
        return TD_SUCCESS;
    }

    if (ot_ext_af_set_flag_read(vi_pipe) == OT_EXT_AF_SET_FLAG_ENABLE) {
        ot_ext_af_set_flag_write(vi_pipe, OT_EXT_AF_SET_FLAG_DISABLE);
        isp_af_fe_cfg_reg(vi_pipe, (isp_reg_cfg *)reg_cfg);
        isp_af_be_cfg_reg(vi_pipe, (isp_reg_cfg *)reg_cfg);
    }

    return TD_SUCCESS;
}

static td_s32 isp_af_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
            isp_check_pointer_return(reg_cfg_attr);
            isp_af_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg_attr->reg_cfg);
            break;
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
            isp_check_pointer_return(reg_cfg_attr);
            isp_af_image_res_write(vi_pipe, &reg_cfg_attr->reg_cfg);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_af_exit(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);
    return TD_SUCCESS;
}

td_s32 isp_alg_register_af(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_af);

    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_AF;
    algs->alg_func.pfn_alg_init = isp_af_init;
    algs->alg_func.pfn_alg_run  = isp_af_run;
    algs->alg_func.pfn_alg_ctrl = isp_af_ctrl;
    algs->alg_func.pfn_alg_exit = isp_af_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
