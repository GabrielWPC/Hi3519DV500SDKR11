/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "isp_alg.h"
#include "isp_ext_config.h"
#ifdef OT_ISP_AIBNR_SUPPORT
#include "isp_aiisp_ext_config.h"
#endif
#include "isp_math_utils.h"
#include "isp_sensor.h"
#include "isp_intf.h"
#include "isp_proc.h"
#include "ot_common_3a.h"
#include "isp_thermo.h"

#define OT_AE_WEIGHT_TABLE_WIDTH  17
#define OT_AE_WEIGHT_TABLE_HEIGHT 15
#define OT_AE_STAT_WEIGHT_MAX 15
#define OT_AE_WDR_EXP_RATIO_MIN  0x40   /* min expratio 1X */
#define OT_AE_ISO_MIN  100
#define OT_AE_WDR_EXP_RATIO_SHIFT    6
#define OT_AE_HIST_SKIP_MAX 6
#define OT_AE_HIST_SKIP_MIN 1
#define OT_AE_ISP_DGAIN_SHIFT    8
#define OT_AE_ISP_DGAIN_THRESHOLD 0x400  /* 4X ispdgain */
#define OT_AE_GAIN_USER_LINEAR_SHIFT     10       /* this value should not be less than 10  */
#define OT_AE_SNS_GAIN_MAX 0x3FFFFF
#define OT_AE_SNS_GAIN_MIN 0x400
#define OT_AE_ISPDGAIN_MAX 0x10000
#define OT_AE_ISPDGAIN_MIN 0x100

static td_u32 ae_piris_lin_to_fno(td_u32 value)
{
    td_u32 i = 0;
    td_u32 tmp = value;

    if (value <= 1) {
        i = 0;
    } else {
        while (tmp > 1) {
            tmp = tmp >> 1;
            i++;
        }
    }

    return i;
}

static td_s32 ae_get_dcf_info(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_dcf_update_info *isp_update_info = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_update_info = &isp_ctx->dcf_update_info;

    if (isp_update_info == TD_NULL) {
        return TD_FAILURE;
    }

    isp_update_info->exposure_bias_value = ae_result->update_info.exposure_bias_value;
    isp_update_info->iso_speed_ratings   = ae_result->update_info.iso_speed_ratings;
    isp_update_info->exposure_program    = ae_result->update_info.exposure_program;
    isp_update_info->exposure_mode       = ae_result->update_info.exposure_mode;
    isp_update_info->exposure_time       = ae_result->update_info.exposure_time;
    isp_update_info->max_aperture_value  = ae_result->update_info.max_aperture_value;
    isp_update_info->f_number            = ae_result->update_info.f_number;

    return TD_SUCCESS;
}

static td_s32 ae_get_frame_info(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, td_bool isp_dgain_forward)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_frame_info *isp_frame = TD_NULL;
    td_u32 result = 0;
    td_u8 i = 0;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_frame = &isp_ctx->frame_info;

    if (isp_frame == TD_NULL) {
        return TD_FAILURE;
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        isp_frame->exposure_time[i] = ae_result->int_time[i] >> 4;  /* accuracy diff 4 bit */
    }
    isp_frame->iso = ae_result->iso;
    isp_frame->again[0] = ae_result->again;
    isp_frame->dgain[0] = ae_result->dgain;

    if (is_linear_mode(isp_ctx->sns_wdr_mode) ||
        (is_wdr_mode(isp_ctx->sns_wdr_mode) && ae_result->fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE)) {
        isp_mul_u32_limit(result, (td_u64)ae_result->isp_dgain, ae_result->wdr_gain[0]);
        isp_frame->isp_dgain[0] = ((result << 2) >> /* 2 */
            OT_AE_ISP_DGAIN_SHIFT);
    } else {
        isp_frame->isp_dgain[0] = ae_result->isp_dgain << 2; /* left shift 2 bit, from 8 bit to 10 bit */
    }

    if (is_2to1_wdr_mode(isp_ctx->sns_wdr_mode)) {
        isp_frame->ratio[0] = ((((td_u64)ae_result->int_time[1] * ae_result->iso) << OT_AE_WDR_EXP_RATIO_SHIFT) +
                               ((td_u64)ae_result->int_time[0] * ae_result->iso_sf) / 2) /     /* 2 */
                              div_0_to_1((td_u64)ae_result->int_time[0] * ae_result->iso_sf);
        isp_frame->ratio[1] = OT_AE_WDR_EXP_RATIO_MIN;
        isp_frame->ratio[2] = OT_AE_WDR_EXP_RATIO_MIN;     /* 2 */
    } else if (is_3to1_wdr_mode(isp_ctx->sns_wdr_mode)) {
        isp_frame->ratio[0] = ((td_u64)ae_result->int_time[1] << OT_AE_WDR_EXP_RATIO_SHIFT) /
            div_0_to_1(ae_result->int_time[0]);
        isp_frame->ratio[1] = ((td_u64)ae_result->int_time[2] << OT_AE_WDR_EXP_RATIO_SHIFT) /     /* 2 */
            div_0_to_1(ae_result->int_time[1]);
        isp_frame->ratio[2] = OT_AE_WDR_EXP_RATIO_MIN;     /* 2 */
    } else {
        isp_frame->ratio[0] = OT_AE_WDR_EXP_RATIO_MIN;
        isp_frame->ratio[1] = OT_AE_WDR_EXP_RATIO_MIN;
        isp_frame->ratio[2] = OT_AE_WDR_EXP_RATIO_MIN;     /* 2 */
    }

    isp_frame->f_number   = ae_piris_lin_to_fno(ae_result->piris_gain);
    isp_frame->hmax_times = ae_result->hmax_times;
    isp_frame->vmax       = ae_result->vmax;

    return TD_SUCCESS;
}

static td_void ae_regs_range_check(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result)
{
    td_u32 i, j;

    ae_result->stat_attr.ae_be_sel       = MIN2(ae_result->stat_attr.ae_be_sel, 2); /* 2 mean be after DRC */
    ae_result->stat_attr.four_plane_mode = MIN2(ae_result->stat_attr.four_plane_mode, 1);
    if (ae_result->stat_attr.four_plane_mode) {
        ae_result->stat_attr.hist_skip_x = MIN2(ae_result->stat_attr.hist_skip_x, OT_AE_HIST_SKIP_MAX);
    } else {
        ae_result->stat_attr.hist_skip_x = MIN2(ae_result->stat_attr.hist_skip_x, OT_AE_HIST_SKIP_MAX);
        ae_result->stat_attr.hist_skip_x = MAX2(ae_result->stat_attr.hist_skip_x, OT_AE_HIST_SKIP_MIN);
    }
    ae_result->stat_attr.hist_skip_y   = MIN2(ae_result->stat_attr.hist_skip_y, OT_AE_HIST_SKIP_MAX);
    ae_result->stat_attr.hist_offset_x = MIN2(ae_result->stat_attr.hist_offset_x, 1);
    ae_result->stat_attr.hist_offset_y = MIN2(ae_result->stat_attr.hist_offset_y, 1);

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ae_result->stat_attr.weight_table[vi_pipe][i][j] =
                MIN2(ae_result->stat_attr.weight_table[vi_pipe][i][j], OT_AE_STAT_WEIGHT_MAX);
        }
    }

    ae_result->stat_attr.hist_mode    = MIN2(ae_result->stat_attr.hist_mode, 1);
    ae_result->stat_attr.aver_mode    = MIN2(ae_result->stat_attr.aver_mode, 1);
    ae_result->stat_attr.max_gain_mode = MIN2(ae_result->stat_attr.max_gain_mode, 1);
}

static td_void ae_crop_ext_regs_default(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx)
{
    ot_ext_system_ae_crop_en_write(vi_pipe, OT_EXT_SYSTEM_CROP_EN_DEFAULT);
    ot_ext_system_ae_crop_x_write(vi_pipe, 0);
    ot_ext_system_ae_crop_y_write(vi_pipe, 0);
    ot_ext_system_ae_crop_height_write(vi_pipe, isp_ctx->block_attr.frame_rect.height);
    ot_ext_system_ae_crop_width_write(vi_pipe, isp_ctx->block_attr.frame_rect.width);

    ot_ext_system_ae_fe_crop_en_write(vi_pipe, OT_EXT_SYSTEM_FE_CROP_EN_DEFAULT);
    ot_ext_system_ae_fe_crop_x_write(vi_pipe, 0);
    ot_ext_system_ae_fe_crop_y_write(vi_pipe, 0);
    ot_ext_system_ae_fe_crop_height_write(vi_pipe, isp_ctx->sys_rect.height);
    ot_ext_system_ae_fe_crop_width_write(vi_pipe, isp_ctx->sys_rect.width);
}

static td_void ae_res_regs_default(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8  i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_rect block_rect;
    isp_mg_static_cfg  *mg_static_reg_cfg  = TD_NULL;
    isp_ae_static_cfg  *ae_static_reg_cfg  = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    /* AE BE configs */
    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        /* AE&MG&DG BLC configs */
        ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.static_reg_cfg;
        mg_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.static_reg_cfg;

        /* AE&MG&DG size configs */
        isp_get_block_rect(&block_rect, &isp_ctx->block_attr, i);

        ae_static_reg_cfg->be_crop_pos_y      = 0;
        ae_static_reg_cfg->be_crop_out_height = block_rect.height;
        ae_static_reg_cfg->be_crop_pos_x      = 0;
        ae_static_reg_cfg->be_crop_out_width  = block_rect.width;
        mg_static_reg_cfg->crop_pos_y         = 0;
        mg_static_reg_cfg->crop_out_height    = block_rect.height;
        mg_static_reg_cfg->crop_pos_x         = 0;
        mg_static_reg_cfg->crop_out_width     = block_rect.width;

        /* AE&MG overlap configs */
        if (i == 0) {
            if (isp_ctx->block_attr.block_num > 1) {
                ae_static_reg_cfg->be_crop_out_width  = block_rect.width - isp_ctx->block_attr.over_lap;
                mg_static_reg_cfg->crop_out_width    = block_rect.width - isp_ctx->block_attr.over_lap;
            }
        } else if (i == (isp_ctx->block_attr.block_num - 1)) {
            ae_static_reg_cfg->be_crop_pos_x     = isp_ctx->block_attr.over_lap;
            ae_static_reg_cfg->be_crop_out_width = block_rect.width - isp_ctx->block_attr.over_lap;
            mg_static_reg_cfg->crop_pos_x        = isp_ctx->block_attr.over_lap;
            mg_static_reg_cfg->crop_out_width    = block_rect.width - isp_ctx->block_attr.over_lap;
        } else {
            ae_static_reg_cfg->be_crop_pos_x     = isp_ctx->block_attr.over_lap;
            ae_static_reg_cfg->be_crop_out_width = block_rect.width - (isp_ctx->block_attr.over_lap << 1);
            mg_static_reg_cfg->crop_pos_x        = isp_ctx->block_attr.over_lap;
            mg_static_reg_cfg->crop_out_width    = block_rect.width - (isp_ctx->block_attr.over_lap << 1);
        }
    }

    /* AE FE configs */
    ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.static_reg_cfg;

    /* crop configs */
    ae_static_reg_cfg->fe_crop_pos_x = 0;
    ae_static_reg_cfg->fe_crop_pos_y = 0;
    ae_static_reg_cfg->fe_crop_out_height = isp_ctx->sys_rect.height;
    ae_static_reg_cfg->fe_crop_out_width  = isp_ctx->sys_rect.width;

    ae_crop_ext_regs_default(vi_pipe, isp_ctx);

    reg_cfg->cfg_key.bit1_ae_cfg1 = TD_TRUE;
    reg_cfg->cfg_key.bit1_ae_cfg2 = TD_TRUE;
}

static td_void ae_get_fe_crop_info(ot_vi_pipe vi_pipe, td_u32 *x, td_u32 *y, td_u32 *width, td_u32 *height)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    (*x) = (ot_ext_system_ae_fe_crop_x_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*y) = (ot_ext_system_ae_fe_crop_y_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*width) = (ot_ext_system_ae_fe_crop_width_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*height) = (ot_ext_system_ae_fe_crop_height_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */

    (*width)  = MAX2((*width),  OT_ISP_AE_MIN_WIDTH);
    (*height) = MAX2((*height), OT_ISP_AE_MIN_HEIGHT);
    (*width)  = MIN2((*width), isp_ctx->sys_rect.width);
    (*height) = MIN2((*height), isp_ctx->sys_rect.height);
    (*x)      = MIN2((*x), (isp_ctx->sys_rect.width - (*width)));
    (*y)      = MIN2((*y), (isp_ctx->sys_rect.height - (*height)));
}

static td_void ae_get_crop_info(ot_vi_pipe vi_pipe, td_u32 *x, td_u32 *y, td_u32 *width, td_u32 *height)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    (*x) = (ot_ext_system_ae_crop_x_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*y) = (ot_ext_system_ae_crop_y_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*width) = (ot_ext_system_ae_crop_width_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */
    (*height) = (ot_ext_system_ae_crop_height_read(vi_pipe) >> 2) << 2; /* shift 2 bit to avoid error */

    (*width)  = MAX2((*width),  OT_ISP_AE_MIN_WIDTH);
    (*height) = MAX2((*height), OT_ISP_AE_MIN_HEIGHT);
    (*width)  = MIN2((*width), isp_ctx->block_attr.frame_rect.width);
    (*height) = MIN2((*height), isp_ctx->block_attr.frame_rect.height);
    (*x)      = MIN2((*x), (isp_ctx->block_attr.frame_rect.width - (*width)));
    (*y)      = MIN2((*y), (isp_ctx->block_attr.frame_rect.height - (*height)));
}

static td_void ae_set_crop_info(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8  i, block_num, crop_en;
    td_u32 crop_x, crop_y, crop_height, crop_width;

    isp_rect block_rect;
    isp_mg_static_cfg *mg_static_reg_cfg = TD_NULL;
    isp_ae_static_cfg *ae_static_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;
    td_u16 overlap = isp_ctx->block_attr.over_lap;
    crop_en = ot_ext_system_ae_crop_en_read(vi_pipe);
    ae_get_crop_info(vi_pipe, &crop_x, &crop_y, &crop_width, &crop_height);

    for (i = 0; i < block_num; i++) {
        /* AE&MG&DG BLC configs */
        ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.static_reg_cfg;
        mg_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.static_reg_cfg;

        /* AE&MG&DG size configs */
        isp_get_block_rect(&block_rect, &isp_ctx->block_attr, i);

        /* AE&MG overlap configs */
        if (i == 0) {
            if (block_num > 1) {
                ae_static_reg_cfg->be_crop_pos_x      = 0;
                ae_static_reg_cfg->be_crop_pos_y      = 0;
                ae_static_reg_cfg->be_crop_out_height = block_rect.height;
                ae_static_reg_cfg->be_crop_out_width  = block_rect.width - overlap;

                mg_static_reg_cfg->crop_pos_x      = 0;
                mg_static_reg_cfg->crop_pos_y      = 0;
                mg_static_reg_cfg->crop_out_height = block_rect.height;
                mg_static_reg_cfg->crop_out_width  = block_rect.width - overlap;
            } else {
                ae_static_reg_cfg->be_crop_pos_x      = crop_en ? crop_x : 0;
                ae_static_reg_cfg->be_crop_pos_y      = crop_en ? crop_y : 0;
                ae_static_reg_cfg->be_crop_out_height = crop_en ? crop_height : block_rect.height;
                ae_static_reg_cfg->be_crop_out_width  = crop_en ? crop_width : block_rect.width;

                mg_static_reg_cfg->crop_pos_x         = crop_en ? crop_x : 0;
                mg_static_reg_cfg->crop_pos_y         = crop_en ? crop_y : 0;
                mg_static_reg_cfg->crop_out_height    = crop_en ? crop_height : block_rect.height;
                mg_static_reg_cfg->crop_out_width     = crop_en ? crop_width : block_rect.width;
            }
        } else {
            ae_static_reg_cfg->be_crop_pos_x      = overlap;
            ae_static_reg_cfg->be_crop_pos_y      = 0;
            ae_static_reg_cfg->be_crop_out_height = block_rect.height;
            ae_static_reg_cfg->be_crop_out_width  = block_rect.width;
            ae_static_reg_cfg->be_crop_out_width  -= (i == (block_num - 1) ? overlap : (overlap << 1));

            mg_static_reg_cfg->crop_pos_x      = overlap;
            mg_static_reg_cfg->crop_pos_y      = 0;
            mg_static_reg_cfg->crop_out_height = block_rect.height;
            mg_static_reg_cfg->crop_out_width  = block_rect.width;
            mg_static_reg_cfg->crop_out_width  -= (i == (block_num - 1) ? overlap : (overlap << 1));
        }
    }
}


static td_void ae_res_read_extregs(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 crop_en;
    td_u32 crop_x, crop_y, crop_height, crop_width;

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ae_static_cfg *ae_static_reg_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    crop_en = ot_ext_system_ae_fe_crop_en_read(vi_pipe);
    ae_get_fe_crop_info(vi_pipe, &crop_x, &crop_y, &crop_width, &crop_height);
    /* AE BE configs */
    ae_set_crop_info(vi_pipe, reg_cfg);
    /* AE FE configs */
    ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.static_reg_cfg;

    /* crop configs */
    ae_static_reg_cfg->fe_crop_pos_x       = crop_en ? crop_x : 0;
    ae_static_reg_cfg->fe_crop_pos_y       = crop_en ? crop_y : 0;
    ae_static_reg_cfg->fe_crop_out_height  = crop_en ? crop_height : isp_ctx->sys_rect.height;
    ae_static_reg_cfg->fe_crop_out_width   = crop_en ? crop_width : isp_ctx->sys_rect.width;

    reg_cfg->cfg_key.bit1_ae_cfg1 = TD_TRUE;
    reg_cfg->cfg_key.bit1_ae_cfg2 = TD_TRUE;
}

static td_void ae_update_be_working_status(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    td_u8 ai_mode = 0;
    isp_ae_dyna_cfg  *ae_dyna_reg  = TD_NULL;
    isp_mg_dyna_cfg  *mg_dyna_reg  = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        ae_dyna_reg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg;
        mg_dyna_reg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.dyna_reg_cfg;
        if ((ae_dyna_reg->be_ae_sel == OT_ISP_AE_AFTER_DRC) && ((ai_mode == OT_ISP_AI_DRC))) {
            ae_dyna_reg->is_in_pre_be = TD_FALSE;
        } else {
            ae_dyna_reg->is_in_pre_be = TD_TRUE;
        }
        ae_dyna_reg->is_online = (is_online_mode(isp_ctx->block_attr.running_mode)) ||
            (is_pre_online_post_offline(isp_ctx->block_attr.running_mode) && (ae_dyna_reg->is_in_pre_be == TD_TRUE));

        if (ai_mode == OT_ISP_AI_DRC) {
            mg_dyna_reg->is_in_pre_be = TD_FALSE;
        } else {
            mg_dyna_reg->is_in_pre_be = TD_TRUE;
        }

        mg_dyna_reg->is_online = (is_online_mode(isp_ctx->block_attr.running_mode)) ||
            (is_pre_online_post_offline(isp_ctx->block_attr.running_mode) && (mg_dyna_reg->is_in_pre_be == TD_TRUE));
    }
}

static td_void ae_set_hist_offset(ot_vi_pipe vi_pipe, td_u8 *skip_x, td_u8 *skip_y, td_u8 *offset_x, td_u8 *offset_y)
{
    td_u8 sensor_pattern_type;
    sensor_pattern_type = ot_ext_system_rggb_cfg_read(vi_pipe);
    (*skip_x) = 1;
    (*skip_y) = 1;
    (*offset_y) = 0;
    td_bool set_offset_x = (sensor_pattern_type == OT_ISP_TOP_RGGB_START_R_GR_GB_B) ||
        (sensor_pattern_type == OT_ISP_TOP_RGGB_START_B_GB_GR_R);
    (*offset_x) = set_offset_x ? 1 : 0;
}

static td_void ae_set_be_reg(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, td_u8 weight_table[][OT_AE_WEIGHT_TABLE_WIDTH])
{
    td_u8 i, j, k;
    td_u8 wdr_mode, block_num, hist_skip_x, hist_skip_y, hist_offset_x, hist_offset_y;
    td_u8 block_offset_x = 0;
    ae_set_hist_offset(vi_pipe, &hist_skip_x, &hist_skip_y, &hist_offset_x, &hist_offset_y);

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_mg_dyna_cfg      *mg_dyna_reg_cfg      = TD_NULL;
    isp_mg_static_cfg    *mg_static_reg_cfg    = TD_NULL;
    isp_ae_static_cfg    *ae_static_reg_cfg    = TD_NULL;
    isp_ae_dyna_cfg      *ae_dyna_reg_cfg      = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    block_num = isp_ctx->block_attr.block_num;
    wdr_mode  = isp_ctx->sns_wdr_mode;

    /* AE BE configs */
    for (i = 0; i < block_num; i++) {
        /* AE&MG&DG BLC configs */
        ae_dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg;
        ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.static_reg_cfg;
        mg_dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.dyna_reg_cfg;
        mg_static_reg_cfg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.static_reg_cfg;
        ae_static_reg_cfg->be_enable  = TD_TRUE;
        mg_static_reg_cfg->enable    = TD_TRUE;

        /* AE&MG WDR configs */
        ae_dyna_reg_cfg->be_bit_move    = 0;
        ae_dyna_reg_cfg->be_gamma_limit = 6;  /* experience value 6 */
        mg_dyna_reg_cfg->bit_move       = 0;
        mg_dyna_reg_cfg->gamma_limit    = 3;  /* experience value 3 */

        ae_dyna_reg_cfg->be_hist_gamma_mode = is_linear_mode(wdr_mode) ? 0 : 1;
        ae_dyna_reg_cfg->be_aver_gamma_mode = is_linear_mode(wdr_mode) ? 0 : 1;
        mg_dyna_reg_cfg->gamma_mode         = is_linear_mode(wdr_mode) ? 0 : 1;

        /* MPI configs */
        ae_dyna_reg_cfg->be_ae_sel          = 1;
        ae_dyna_reg_cfg->be_four_plane_mode = 0;
        ae_dyna_reg_cfg->be_hist_skip_x     = hist_skip_x;
        ae_dyna_reg_cfg->be_hist_skip_y     = hist_skip_y;
        ae_dyna_reg_cfg->be_hist_offset_x   = hist_offset_x;
        ae_dyna_reg_cfg->be_hist_offset_y   = hist_offset_y;

        /* weight table configs */
        ae_dyna_reg_cfg->be_weight_table_update = TD_TRUE;
        ae_dyna_reg_cfg->be_weight_table_width = OT_AE_WEIGHT_TABLE_WIDTH / div_0_to_1(block_num);
        mg_dyna_reg_cfg->zone_width          = OT_AE_WEIGHT_TABLE_WIDTH / div_0_to_1(block_num);
        ae_dyna_reg_cfg->be_weight_table_width += ((i < OT_AE_WEIGHT_TABLE_WIDTH % div_0_to_1(block_num)) ? 1 : 0);
        mg_dyna_reg_cfg->zone_width += ((i < OT_AE_WEIGHT_TABLE_WIDTH % div_0_to_1(block_num)) ? 1 : 0);
        ae_dyna_reg_cfg->be_weight_table_height = OT_AE_WEIGHT_TABLE_HEIGHT;
        mg_dyna_reg_cfg->zone_height          = OT_AE_WEIGHT_TABLE_HEIGHT;
        for (j = 0; j < ae_dyna_reg_cfg->be_weight_table_height; j++) {
            for (k = 0; k < ae_dyna_reg_cfg->be_weight_table_width; k++) {
                ae_dyna_reg_cfg->be_weight_table[j][k] = weight_table[j][k + block_offset_x];
            }
        }
        block_offset_x += ae_dyna_reg_cfg->be_weight_table_width;
    }

    ae_update_be_working_status(vi_pipe, reg_cfg);
}

static td_void ae_set_default_smart_regs(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    ot_ext_system_smart_update_write(vi_pipe, OT_EXT_SYSTEM_SMART_UPDATE_DEFAULT);
    for (i = 0; i < OT_ISP_PEOPLE_CLASS_MAX; i++) {
        ot_ext_system_smart_enable_write(vi_pipe, i, OT_EXT_SYSTEM_SMART_ENABLE_DEFAULT);
        ot_ext_system_smart_available_write(vi_pipe, i, OT_EXT_SYSTEM_SMART_AVAILABLE_DEFAULT);
        ot_ext_system_smart_luma_write(vi_pipe, i, OT_EXT_SYSTEM_SMART_LUMA_DEFAULT);
    }

    ot_ext_system_tunnel_update_write(vi_pipe, OT_EXT_SYSTEM_TUNNEL_UPDATE_DEFAULT);
    for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
        ot_ext_system_tunnel_enable_write(vi_pipe, i, OT_EXT_SYSTEM_TUNNEL_ENABLE_DEFAULT);
        ot_ext_system_tunnel_available_write(vi_pipe, i, OT_EXT_SYSTEM_TUNNEL_AVAILABLE_DEFAULT);
        ot_ext_system_tunnel_area_ratio_write(vi_pipe, i, OT_EXT_SYSTEM_TUNNEL_AREA_RATIO_DEFAULT);
        ot_ext_system_tunnel_exp_perf_write(vi_pipe, i, OT_EXT_SYSTEM_TUNNEL_EXP_PERF_DEFAULT);
    }

    ot_ext_system_fast_face_enable_write(vi_pipe, OT_EXT_SYSTEM_FAST_FACE_ENABLE);
    ot_ext_system_fast_face_available_write(vi_pipe, OT_EXT_SYSTEM_FAST_FACE_AVAILABLE);
    ot_ext_system_fast_face_frame_pts_write(vi_pipe, OT_EXT_SYSTEM_FACE_FRAME_PTS_DEFAULT);
    for (i = 0; i < OT_ISP_FACE_NUM; i++) {
        ot_ext_system_fast_face_x_write(vi_pipe, i, OT_EXT_SYSTEM_FAST_FACE_X_DEFAULT);
        ot_ext_system_fast_face_y_write(vi_pipe, i, OT_EXT_SYSTEM_FAST_FACE_Y_DEFAULT);
        ot_ext_system_fast_face_width_write(vi_pipe, i, OT_EXT_SYSTEM_FAST_FACE_WIDTH_DEFAULT);
        ot_ext_system_fast_face_height_write(vi_pipe, i, OT_EXT_SYSTEM_FAST_FACE_HEIGHT_DEFAULT);
    }
}

static td_void ae_write_default_system_info(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 wdr_mode, mode_value;
    td_u8 hist_skip_x, hist_skip_y, hist_offset_x, hist_offset_y;
    ae_set_hist_offset(vi_pipe, &hist_skip_x, &hist_skip_y, &hist_offset_x, &hist_offset_y);

    isp_mg_static_cfg    *mg_static_reg_cfg    = TD_NULL;
    isp_ae_static_cfg    *ae_static_reg_cfg    = TD_NULL;
    isp_ae_dyna_cfg      *ae_dyna_reg_cfg      = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    wdr_mode  = isp_ctx->sns_wdr_mode;
    ae_dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg;
    ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.static_reg_cfg;
    mg_static_reg_cfg = &reg_cfg->alg_reg_cfg[0].mg_reg_cfg.static_reg_cfg;
    ot_ext_system_ae_be_sel_write(vi_pipe, ae_dyna_reg_cfg->be_ae_sel);
    ot_ext_system_ae_fourplanemode_write(vi_pipe, ae_dyna_reg_cfg->be_four_plane_mode);

    ot_ext_system_ae_hist_skip_x_write(vi_pipe, hist_skip_x);
    ot_ext_system_ae_hist_skip_y_write(vi_pipe, hist_skip_y);
    ot_ext_system_ae_hist_offset_x_write(vi_pipe, hist_offset_x);
    ot_ext_system_ae_hist_offset_y_write(vi_pipe, hist_offset_y);

    ot_ext_system_ae_fe_en_write(vi_pipe, ae_static_reg_cfg->fe_enable);
    ot_ext_system_ae_be_en_write(vi_pipe, ae_static_reg_cfg->be_enable);
    ot_ext_system_mg_en_write(vi_pipe, mg_static_reg_cfg->enable);

    mode_value = is_linear_mode(wdr_mode) ? 0 : 1;
    ot_ext_system_ae_histmode_write(vi_pipe, mode_value);
    ot_ext_system_ae_avermode_write(vi_pipe, mode_value);
    ot_ext_system_ae_maxgainmode_write(vi_pipe, mode_value);

    ae_res_regs_default(vi_pipe, reg_cfg);
    ae_set_default_smart_regs(vi_pipe);
}

static td_void ae_reg_cfg2_default(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    isp_ae_reg_cfg_2  *ae_reg_cfg_2 = TD_NULL;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;

    isp_sensor_get_blc(vi_pipe, &sns_black_level);

    ae_reg_cfg_2 = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg2;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        ae_reg_cfg_2->wdr_gain[i] = 0x100;
        ae_reg_cfg_2->int_time[i] = 1000;   /* default init time:1000 */
    }
    ae_reg_cfg_2->isp_dgain = 0x100;
    ae_reg_cfg_2->exposure = 10000;    /* default exposure:10000 */
    ae_reg_cfg_2->exposure_sf = 10000; /* default exposure_sf:10000 */
    ae_reg_cfg_2->piris_valid = 0;
    ae_reg_cfg_2->piris_pos = 0;
    ae_reg_cfg_2->fs_wdr_mode = OT_ISP_FSWDR_NORMAL_MODE;
    ae_reg_cfg_2->thermo_enable = TD_FALSE;
}

static td_void ae_regs_default(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    size_t wgt_table_size;
    td_u8 i, j, hist_skip_x, hist_skip_y, hist_offset_x, hist_offset_y;

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ae_static_cfg    *ae_static_reg_cfg    = TD_NULL;
    isp_ae_dyna_cfg      *ae_dyna_reg_cfg      = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    ae_set_hist_offset(vi_pipe, &hist_skip_x, &hist_skip_y, &hist_offset_x, &hist_offset_y);

    td_u8 ae_weight_table[OT_AE_WEIGHT_TABLE_HEIGHT][OT_AE_WEIGHT_TABLE_WIDTH] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},     /* 2 */
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                ae_weight_table[i][j] = 1;
            }
        }
    }
    ae_set_be_reg(vi_pipe, reg_cfg, ae_weight_table);
    /* AE FE configs */
    ae_dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg;
    ae_static_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.static_reg_cfg;

    /* BLC configs */
    ae_static_reg_cfg->fe_enable = TD_TRUE;

    /* WDR configs */
    ae_dyna_reg_cfg->fe_bit_move       = 0;
    ae_dyna_reg_cfg->fe_gamma_limit    = 6;  /* experience value 6 */
    ae_dyna_reg_cfg->fe_hist_gamma_mode = 0;
    ae_dyna_reg_cfg->fe_aver_gamma_mode = 0;

    /* MPI configs */
    ae_dyna_reg_cfg->fe_four_plane_mode = 0;
    ae_dyna_reg_cfg->fe_hist_skip_x     = hist_skip_x;
    ae_dyna_reg_cfg->fe_hist_skip_y     = hist_skip_y;
    ae_dyna_reg_cfg->fe_hist_offset_x   = hist_offset_x;
    ae_dyna_reg_cfg->fe_hist_offset_y   = hist_offset_y;

    /* weight tbale configs */
    ae_dyna_reg_cfg->fe_weight_table_update  = TD_TRUE;
    ae_dyna_reg_cfg->fe_weight_table_width  = OT_AE_WEIGHT_TABLE_WIDTH;
    ae_dyna_reg_cfg->fe_weight_table_height = OT_AE_WEIGHT_TABLE_HEIGHT;
    wgt_table_size = OT_AE_WEIGHT_TABLE_HEIGHT * OT_AE_WEIGHT_TABLE_WIDTH * sizeof(td_u8);

    (td_void)memcpy_s(ae_dyna_reg_cfg->fe_weight_table, wgt_table_size, ae_weight_table, wgt_table_size);

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ot_ext_system_ae_weight_table_write(vi_pipe, (i * OT_ISP_AE_ZONE_COLUMN + j), ae_weight_table[i][j]);
        }
    }
    ae_write_default_system_info(vi_pipe, reg_cfg);
}

static td_void ae_read_extregs(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result)
{
    td_u32 i, j;

    if (!ae_result->stat_attr.hist_adjust) {
        if (ae_result->stat_attr.ae_be_sel != ot_ext_system_ae_be_sel_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.ae_be_sel = ot_ext_system_ae_be_sel_read(vi_pipe);
        }

        if (ae_result->stat_attr.four_plane_mode != ot_ext_system_ae_fourplanemode_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.four_plane_mode = ot_ext_system_ae_fourplanemode_read(vi_pipe);
        }

        if (ae_result->stat_attr.hist_offset_x != ot_ext_system_ae_hist_offset_x_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.hist_offset_x = ot_ext_system_ae_hist_offset_x_read(vi_pipe);
        }

        if (ae_result->stat_attr.hist_offset_y != ot_ext_system_ae_hist_offset_y_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.hist_offset_y = ot_ext_system_ae_hist_offset_y_read(vi_pipe);
        }

        if (ae_result->stat_attr.hist_skip_x != ot_ext_system_ae_hist_skip_x_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.hist_skip_x = ot_ext_system_ae_hist_skip_x_read(vi_pipe);
        }

        if (ae_result->stat_attr.hist_skip_y != ot_ext_system_ae_hist_skip_y_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.hist_skip_y = ot_ext_system_ae_hist_skip_y_read(vi_pipe);
        }
    } else {
        ot_ext_system_ae_be_sel_write(vi_pipe, ae_result->stat_attr.ae_be_sel);
        ot_ext_system_ae_fourplanemode_write(vi_pipe, ae_result->stat_attr.four_plane_mode);
        ot_ext_system_ae_hist_skip_x_write(vi_pipe, ae_result->stat_attr.hist_skip_x);
        ot_ext_system_ae_hist_skip_y_write(vi_pipe, ae_result->stat_attr.hist_skip_y);
        ot_ext_system_ae_hist_offset_x_write(vi_pipe, ae_result->stat_attr.hist_offset_x);
        ot_ext_system_ae_hist_offset_y_write(vi_pipe, ae_result->stat_attr.hist_offset_y);
    }

    if (!ae_result->stat_attr.wight_table_update) {
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                ae_result->stat_attr.change = (ae_result->stat_attr.weight_table[vi_pipe][i][j] !=
                    ot_ext_system_ae_weight_table_read(vi_pipe, (i * OT_ISP_AE_ZONE_COLUMN + j)) ?
                    TD_TRUE : ae_result->stat_attr.change);
                ae_result->stat_attr.weight_table[vi_pipe][i][j] =
                    ot_ext_system_ae_weight_table_read(vi_pipe, (i * OT_ISP_AE_ZONE_COLUMN + j));
            }
        }
    } else {
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                ot_ext_system_ae_weight_table_write(vi_pipe,
                    (i * OT_ISP_AE_ZONE_COLUMN + j), ae_result->stat_attr.weight_table[vi_pipe][i][j]);
            }
        }
    }

    if (!ae_result->stat_attr.mode_update) {
        if (ae_result->stat_attr.hist_mode != ot_ext_system_ae_histmode_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.hist_mode = ot_ext_system_ae_histmode_read(vi_pipe);
        }

        if (ae_result->stat_attr.aver_mode != ot_ext_system_ae_avermode_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.aver_mode = ot_ext_system_ae_avermode_read(vi_pipe);
        }

        if (ae_result->stat_attr.max_gain_mode != ot_ext_system_ae_maxgainmode_read(vi_pipe)) {
            ae_result->stat_attr.change = TD_TRUE;
            ae_result->stat_attr.max_gain_mode = ot_ext_system_ae_maxgainmode_read(vi_pipe);
        }
    } else {
        ot_ext_system_ae_histmode_write(vi_pipe, ae_result->stat_attr.hist_mode);
        ot_ext_system_ae_avermode_write(vi_pipe, ae_result->stat_attr.aver_mode);
        ot_ext_system_ae_maxgainmode_write(vi_pipe, ae_result->stat_attr.max_gain_mode);
    }
}

static td_void ae_update_be_config_part(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    ot_isp_ae_stat_attr *stat_attr = TD_NULL;
    isp_ae_dyna_cfg  *ae_dyna_reg_cfg  = TD_NULL;
    isp_mg_dyna_cfg  *mg_dyna_reg_cfg  = TD_NULL;
    stat_attr   = &ae_result->stat_attr;
    td_s32 i, j;

    ae_dyna_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg;
    if (stat_attr->hist_adjust) {
        if ((ae_dyna_reg_cfg->be_ae_sel != stat_attr->ae_be_sel) ||
            (ae_dyna_reg_cfg->be_four_plane_mode != stat_attr->four_plane_mode) ||
            (ae_dyna_reg_cfg->be_hist_skip_x != stat_attr->hist_skip_x) ||
            (ae_dyna_reg_cfg->be_hist_skip_y != stat_attr->hist_skip_y) ||
            (ae_dyna_reg_cfg->be_hist_offset_x != stat_attr->hist_offset_x) ||
            (ae_dyna_reg_cfg->be_hist_offset_y != stat_attr->hist_offset_y)) {
            stat_attr->change = TD_TRUE;
        }
    }

    if (stat_attr->mode_update) {
        mg_dyna_reg_cfg = &reg_cfg->alg_reg_cfg[0].mg_reg_cfg.dyna_reg_cfg;
        if ((ae_dyna_reg_cfg->be_hist_gamma_mode != stat_attr->hist_mode) ||
            (ae_dyna_reg_cfg->be_aver_gamma_mode != stat_attr->aver_mode) ||
            (mg_dyna_reg_cfg->gamma_mode != stat_attr->max_gain_mode)) {
            stat_attr->change = TD_TRUE;
        }
    }

    if (stat_attr->wight_table_update) {
        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                stat_attr->change = (ae_dyna_reg_cfg->be_weight_table[i][j] != stat_attr->weight_table[vi_pipe][i][j] ?
                    TD_TRUE : stat_attr->change);
            }
        }
    }
}

static td_void ae_update_be_config(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    td_u8 block_num;
    ot_isp_ae_stat_attr *stat_attr = TD_NULL;
    td_u8 block_offset_x = 0;
    td_s32 i, j, k;
    isp_ae_dyna_cfg *ae_dyna_reg_cfg = TD_NULL;
    isp_mg_dyna_cfg *mg_dyna_reg_cfg = TD_NULL;

    block_num = isp_ctx->block_attr.block_num;
    stat_attr = &ae_result->stat_attr;

    /* BE configs update */
    ae_update_be_config_part(vi_pipe, ae_result, reg_cfg);

    for (i = 0; i < block_num; i++) {
        ae_dyna_reg_cfg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg;
        mg_dyna_reg_cfg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.dyna_reg_cfg;

        ae_dyna_reg_cfg->be_ae_sel          = stat_attr->ae_be_sel;
        ae_dyna_reg_cfg->be_four_plane_mode = stat_attr->four_plane_mode;
        ae_dyna_reg_cfg->be_hist_skip_x     = stat_attr->hist_skip_x;
        ae_dyna_reg_cfg->be_hist_skip_y     = stat_attr->hist_skip_y;
        ae_dyna_reg_cfg->be_hist_offset_x   = stat_attr->hist_offset_x;
        ae_dyna_reg_cfg->be_hist_offset_y   = stat_attr->hist_offset_y;

        ae_dyna_reg_cfg->be_hist_gamma_mode = stat_attr->hist_mode;
        ae_dyna_reg_cfg->be_aver_gamma_mode = stat_attr->aver_mode;
        mg_dyna_reg_cfg->gamma_mode       = stat_attr->max_gain_mode;
        ae_dyna_reg_cfg->be_weight_table_update = TD_TRUE;

        if (i < OT_AE_WEIGHT_TABLE_WIDTH % div_0_to_1(block_num)) {
            ae_dyna_reg_cfg->be_weight_table_width = OT_AE_WEIGHT_TABLE_WIDTH / div_0_to_1(block_num) + 1;
        } else {
            ae_dyna_reg_cfg->be_weight_table_width = OT_AE_WEIGHT_TABLE_WIDTH / div_0_to_1(block_num);
        }

        ae_dyna_reg_cfg->be_weight_table_height = OT_AE_WEIGHT_TABLE_HEIGHT;
        for (j = 0; j < ae_dyna_reg_cfg->be_weight_table_height; j++) {
            for (k = 0; k < ae_dyna_reg_cfg->be_weight_table_width; k++) {
                ae_dyna_reg_cfg->be_weight_table[j][k] = stat_attr->weight_table[vi_pipe][j][k + block_offset_x];
            }
        }
        block_offset_x += ae_dyna_reg_cfg->be_weight_table_width;
        reg_cfg->cfg_key.bit1_ae_cfg1 = TD_TRUE;
        reg_cfg->cfg_key.bit1_ae_cfg2 = TD_TRUE;
    }

    ae_update_be_working_status(vi_pipe, reg_cfg);
}

static td_void ae_update_fe_config(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    td_s32 j, k;
    ot_isp_ae_stat_attr *stat_attr     = TD_NULL;
    stat_attr   = &ae_result->stat_attr;
    isp_ae_dyna_cfg  *ae_dyna_reg_cfg  = TD_NULL;
    ae_dyna_reg_cfg = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg;

    ae_dyna_reg_cfg->fe_four_plane_mode = stat_attr->four_plane_mode;
    ae_dyna_reg_cfg->fe_hist_skip_x     = stat_attr->hist_skip_x;
    ae_dyna_reg_cfg->fe_hist_skip_y     = stat_attr->hist_skip_y;
    ae_dyna_reg_cfg->fe_hist_offset_x   = stat_attr->hist_offset_x;
    ae_dyna_reg_cfg->fe_hist_offset_y   = stat_attr->hist_offset_y;

    ae_dyna_reg_cfg->fe_weight_table_update = TD_TRUE;
    ae_dyna_reg_cfg->fe_weight_table_width  = OT_AE_WEIGHT_TABLE_WIDTH;
    ae_dyna_reg_cfg->fe_weight_table_height = OT_AE_WEIGHT_TABLE_HEIGHT;

    for (j = 0; j < OT_AE_WEIGHT_TABLE_HEIGHT; j++) {
        for (k = 0; k < OT_AE_WEIGHT_TABLE_WIDTH; k++) {
            ae_dyna_reg_cfg->fe_weight_table[j][k] = stat_attr->weight_table[vi_pipe][j][k];
        }
    }
    stat_attr->change = TD_FALSE;
}

static td_void ae_update_stat_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8  block_num;
    td_s32 i;
    isp_ae_static_cfg *ae_stat_reg_cfg = TD_NULL;
    isp_mg_static_cfg *mg_stat_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;

    for (i = 0; i < block_num; i++) {
        ae_stat_reg_cfg = &reg_cfg->alg_reg_cfg[i].ae_reg_cfg.static_reg_cfg;
        mg_stat_reg_cfg = &reg_cfg->alg_reg_cfg[i].mg_reg_cfg.static_reg_cfg;

        ae_stat_reg_cfg->fe_enable = ot_ext_system_ae_fe_en_read(vi_pipe);
        ae_stat_reg_cfg->be_enable = ot_ext_system_ae_be_en_read(vi_pipe);
        mg_stat_reg_cfg->enable    = ot_ext_system_mg_en_read(vi_pipe);
    }
}

static td_void ae_update_config(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    td_s32 i;
    td_u32 sns_gain_sf, sns_gain, isp_gain_sf, isp_gain;

    isp_usr_ctx          *isp_ctx      = TD_NULL;
    isp_ae_reg_cfg_2 *ae_reg_cfg2      = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    ae_reg_cfg2 = &reg_cfg->alg_reg_cfg[0].ae_reg_cfg2;

    /* stat configs update */
    ae_update_stat_config(vi_pipe, reg_cfg);
    /* BE configs update */
    ae_update_be_config(vi_pipe, ae_result, reg_cfg);
    /* FE configs update */
    ae_update_fe_config(vi_pipe, ae_result, reg_cfg);

    ae_reg_cfg2->isp_dgain   = ae_result->isp_dgain;
    ae_reg_cfg2->piris_valid = ae_result->piris_valid;
    ae_reg_cfg2->piris_pos   = ae_result->piris_pos;
    ae_reg_cfg2->fs_wdr_mode = ae_result->fswdr_mode;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        ae_reg_cfg2->int_time[i] = ae_result->int_time[i];
        ae_result->wdr_gain[i] = MAX2(ae_result->wdr_gain[i], 1);
        ae_reg_cfg2->wdr_gain[i] = ae_result->wdr_gain[i];
        ae_reg_cfg2->fe_nr_dgain[i] = 0x100;
    }
    /* be careful avoid overflow */
    if (is_2to1_wdr_mode(isp_ctx->sns_wdr_mode)) {
        sns_gain_sf = (ae_result->again_sf * ae_result->dgain_sf) >> 10; /* 10 */
        sns_gain = (ae_result->again * ae_result->dgain) >> 10; /* 10 */
        isp_gain_sf = (ae_result->isp_dgain * ae_result->wdr_gain[0]) >> 8; /* 8 */
        isp_gain = (ae_result->isp_dgain * ae_result->wdr_gain[1]) >> 8; /* 8 */

        if (ae_result->piris_valid == TD_TRUE) {
            ae_reg_cfg2->exposure = ((td_u64)ae_result->sns_lhcg_exp_ratio *
                ae_result->int_time[1] * sns_gain * isp_gain * ae_result->piris_gain) >> OT_AE_WDR_EXP_RATIO_SHIFT;
            ae_reg_cfg2->exposure_sf = (td_u64)ae_result->int_time[0] * sns_gain_sf * isp_gain_sf *
                ae_result->piris_gain;
        } else {
            ae_reg_cfg2->exposure = ((td_u64)ae_result->sns_lhcg_exp_ratio *
                ae_result->int_time[1] * sns_gain * isp_gain) >> OT_AE_WDR_EXP_RATIO_SHIFT;
            ae_reg_cfg2->exposure_sf = (td_u64)ae_result->int_time[0] * sns_gain_sf * isp_gain_sf;
        }

        if (ae_result->fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) {
            ae_reg_cfg2->exposure_sf = ae_reg_cfg2->exposure;
        }
    } else {
        sns_gain = (ae_result->again * ae_result->dgain) >> 10; /* 10 */
        ae_reg_cfg2->exposure = (td_u64)ae_result->int_time[0] * sns_gain * ae_result->isp_dgain;
        ae_reg_cfg2->exposure *= (ae_result->piris_valid ? ae_result->piris_gain : 1);
    }
}

static td_u8 isp_ae_get_delay_max(isp_usr_ctx *isp_ctx)
{
    td_u8 delay_max;

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        delay_max = isp_ctx->linkage.cfg2valid_delay_max;
    } else {
        delay_max = isp_ctx->linkage.cfg2valid_delay_max + 1;
    }
    delay_max = clip3(delay_max, 1, ISP_SYNC_ISO_BUF_MAX - 1);

    return delay_max;
}

static td_void ae_update_wdr2to1_linkage(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_linkage *linkage)
{
    td_u64 exp_ratio_tmp = 0x40;
    td_u8 delay_max, delay_index, i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    delay_max = isp_ae_get_delay_max(isp_ctx);

    /* WDR exposure ratio is 6bit precision */
    linkage->int_time = ae_result->int_time[1] >> 4; /* right shift 4 bit */
    if (ae_result->fswdr_mode != OT_ISP_FSWDR_LONG_FRAME_MODE) {
        exp_ratio_tmp = (((td_u64) ae_result->int_time[1] * ae_result->iso * ae_result->sns_lhcg_exp_ratio) +
                         ((td_u64) ae_result->int_time[0] * ae_result->iso_sf) / 2) /     /* div 2 */
                        div_0_to_1((td_u64) ae_result->int_time[0] * ae_result->iso_sf);
    }
    for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
        linkage->sync_all_exp_ratio_buf[i] = linkage->sync_all_exp_ratio_buf[i - 1];
        linkage->sync_exp_ratio_buf[0][i] = linkage->sync_exp_ratio_buf[0][i - 1];
    }

    linkage->sync_all_exp_ratio_buf[0] = (td_u32)exp_ratio_tmp;
    linkage->sync_exp_ratio_buf[0][0] = (td_u32)exp_ratio_tmp;

    delay_index = linkage->update_pos == 0 ? delay_max : delay_max - 1;
    linkage->exp_ratio = linkage->sync_all_exp_ratio_buf[delay_index];
    linkage->exp_ratio_lut[0] = linkage->sync_exp_ratio_buf[0][delay_index];

    if (isp_ctx->linkage.run_once == TD_TRUE) {
        linkage->exp_ratio = linkage->sync_all_exp_ratio_buf[0];
        linkage->exp_ratio_lut[0] = linkage->sync_exp_ratio_buf[0][0];
    }
    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 0, linkage->exp_ratio_lut[0]);
    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 1, OT_AE_WDR_EXP_RATIO_MIN);
    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 2, OT_AE_WDR_EXP_RATIO_MIN);     /* index 2 */
    linkage->wdr_gain[0] = ae_result->wdr_gain[0];
    linkage->wdr_gain[1] = ae_result->wdr_gain[1];
}

static td_void ae_update_wdr3to1_linkage(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_linkage *linkage)
{
    td_u64 exp_ratio_tmp = 0x40;
    td_u64 arr_exp_ratio_tmp[OT_ISP_WDR_MAX_FRAME_NUM - 1] = {0x40, 0x40, 0x40};
    td_u8 delay_max, delay_index, i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    delay_max = isp_ae_get_delay_max(isp_ctx);
    const td_u8 shift = OT_AE_WDR_EXP_RATIO_SHIFT;
    linkage->int_time = ae_result->int_time[2] >> 4;  /* 2 right shift 4 bit */
    if (ae_result->fswdr_mode != OT_ISP_FSWDR_LONG_FRAME_MODE) {
        exp_ratio_tmp = ((td_u64)(ae_result->int_time[2] * ae_result->wdr_gain[2]) << shift) / /* index 2 */
                        div_0_to_1(ae_result->int_time[0] * ae_result->wdr_gain[0]);
        arr_exp_ratio_tmp[0] = ((td_u64)(ae_result->int_time[1] * ae_result->wdr_gain[1]) << shift) /
                               div_0_to_1(ae_result->int_time[0] * ae_result->wdr_gain[0]);
        arr_exp_ratio_tmp[1] = ((td_u64)(ae_result->int_time[2] * ae_result->wdr_gain[2]) << shift) / /* index 2 */
                               div_0_to_1(ae_result->int_time[1] * ae_result->wdr_gain[1]);
    }

    for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
        linkage->sync_all_exp_ratio_buf[i] = linkage->sync_all_exp_ratio_buf[i - 1];
        linkage->sync_exp_ratio_buf[0][i] = linkage->sync_exp_ratio_buf[0][i - 1];
        linkage->sync_exp_ratio_buf[1][i] = linkage->sync_exp_ratio_buf[1][i - 1];
    }

    linkage->sync_all_exp_ratio_buf[0] = (td_u32)exp_ratio_tmp;
    linkage->sync_exp_ratio_buf[0][0] = (td_u32)arr_exp_ratio_tmp[0];
    linkage->sync_exp_ratio_buf[1][0] = (td_u32)arr_exp_ratio_tmp[1];

    delay_index = linkage->update_pos == 0 ? delay_max : delay_max - 1;
    linkage->exp_ratio = linkage->sync_all_exp_ratio_buf[delay_index];
    linkage->exp_ratio_lut[0] = linkage->sync_exp_ratio_buf[0][delay_index];
    linkage->exp_ratio_lut[1] = linkage->sync_exp_ratio_buf[1][delay_index];

    if (isp_ctx->linkage.run_once == TD_TRUE) {
        linkage->exp_ratio = linkage->sync_all_exp_ratio_buf[0];
        linkage->exp_ratio_lut[0] = linkage->sync_exp_ratio_buf[0][0];
        linkage->exp_ratio_lut[1] = linkage->sync_exp_ratio_buf[1][0];
    }

    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 0, linkage->exp_ratio_lut[0]);
    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 1, linkage->exp_ratio_lut[1]);
    ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 2, OT_AE_WDR_EXP_RATIO_MIN);     /* index 2 */

    linkage->wdr_gain[0] = ae_result->wdr_gain[0];
    linkage->wdr_gain[1] = ae_result->wdr_gain[1];
    linkage->wdr_gain[2] = ae_result->wdr_gain[2];     /* index 2 */
}

static td_void ae_update_linkage(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, isp_linkage *linkage)
{
    td_s32 i;
    td_u8 delay_max, delay_index;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    linkage->dgain           = ae_result->dgain;
    linkage->again           = ae_result->again;
    linkage->isp_dgain_shift = 8;  /* isp dgain shift 8 */

    for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
        linkage->sync_iso_buf[i] = linkage->sync_iso_buf[i - 1];
        linkage->sync_isp_dgain_buf[i] = linkage->sync_isp_dgain_buf[i - 1];
    }
    linkage->sync_iso_buf[0] = ae_result->iso;
    linkage->sync_isp_dgain_buf[0] = ae_result->isp_dgain;

    delay_max = isp_ae_get_delay_max(isp_ctx);
    delay_index = linkage->update_pos == 0 ? delay_max : delay_max - 1;
    linkage->iso = MAX2(linkage->sync_iso_buf[delay_index], OT_AE_ISO_MIN);
    linkage->isp_dgain = MAX2(linkage->sync_isp_dgain_buf[delay_index], 1);

    if (isp_ctx->linkage.run_once == TD_TRUE) {
        linkage->iso = MAX2(linkage->sync_iso_buf[0], OT_AE_ISO_MIN);
        linkage->isp_dgain = MAX2(linkage->sync_isp_dgain_buf[0], 1);
    }
    isp_ctx->linkage.iso_done_frm_cnt = isp_frame_cnt_get(vi_pipe);

    linkage->sensor_iso = ((td_u64)linkage->iso << 8) / div_0_to_1(linkage->isp_dgain); /* left shift 8 bit */
    linkage->sensor_iso = (linkage->sensor_iso < OT_AE_ISO_MIN) ? OT_AE_ISO_MIN : linkage->sensor_iso;
    linkage->int_time = ae_result->int_time[0] >> 4; /* right shift 4 bit */
    linkage->ae_run_interval = ae_result->ae_run_interval;

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        linkage->wdr_gain[i] = 0x100;
    }

    if (is_2to1_wdr_mode(isp_ctx->sns_wdr_mode)) {
        /* WDR exposure ratio is 6bit precision */
        ae_update_wdr2to1_linkage(vi_pipe, ae_result, linkage);
    } else if (is_3to1_wdr_mode(isp_ctx->sns_wdr_mode)) {
        ae_update_wdr3to1_linkage(vi_pipe, ae_result, linkage);
    } else {
        ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 0, OT_AE_WDR_EXP_RATIO_MIN);
        ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 1, OT_AE_WDR_EXP_RATIO_MIN);
        ot_ext_system_actual_wdr_exposure_ratio_write(vi_pipe, 2, OT_AE_WDR_EXP_RATIO_MIN); /* index 2 */
        linkage->wdr_gain[0] = ae_result->wdr_gain[0];
    }

    linkage->piris_gain = ae_result->piris_valid ? ae_result->piris_gain : 0;
    linkage->pre_fswdr_mode = linkage->fswdr_mode;
    linkage->fswdr_mode     = ae_result->fswdr_mode;
    ot_ext_system_fswdr_mode_write(vi_pipe, ae_result->fswdr_mode);
}

static td_void ae_update_people_info(ot_vi_pipe vi_pipe, ot_isp_ae_info *ae_info)
{
    td_s32 i;
    static td_u32 update_cnt = 0;
    static td_u32 wait_cnt = 0;
    static td_bool in_time = TD_FALSE;

    for (i = 0; i < OT_ISP_PEOPLE_CLASS_MAX; i++) {
        ae_info->smart_info.people_roi[i].enable     = ot_ext_system_smart_enable_read(vi_pipe, i);
        ae_info->smart_info.people_roi[i].luma       = ot_ext_system_smart_luma_read(vi_pipe, i);
        ae_info->smart_info.people_roi[i].available  = ot_ext_system_smart_available_read(vi_pipe, i);
    }

    if (ot_ext_system_smart_update_read(vi_pipe)) {
        update_cnt++;
        ot_ext_system_smart_update_write(vi_pipe, TD_FALSE);
    } else {
        wait_cnt++;
    }

    if (update_cnt) {
        if (wait_cnt < 20) {  /* experience value 20 */
            in_time = TD_TRUE;
        } else {
            in_time = TD_FALSE;
        }
        update_cnt = 0;
        wait_cnt   = 0;
    } else if (wait_cnt >= 20) {  /* experience value 20 */
        in_time = TD_FALSE;
    }

    if (!in_time) {
        for (i = 0; i < OT_ISP_PEOPLE_CLASS_MAX; i++) {
            ae_info->smart_info.people_roi[i].available = TD_FALSE;
        }
    }
}

static td_void ae_update_tunnel_info(ot_vi_pipe vi_pipe, ot_isp_ae_info *ae_info)
{
    td_s32 i;
    static td_u32  tunnel_update_cnt = 0;
    static td_u32  tunnel_wait_cnt = 0;
    static td_bool tunnel_in_time = TD_FALSE;

    for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
        ae_info->smart_info.tunnel_roi[i].enable       = ot_ext_system_tunnel_enable_read(vi_pipe, i);
        ae_info->smart_info.tunnel_roi[i].available    = ot_ext_system_tunnel_available_read(vi_pipe, i);
        ae_info->smart_info.tunnel_roi[i].tunnel_area_ratio = ot_ext_system_tunnel_area_ratio_read(vi_pipe, i);
        ae_info->smart_info.tunnel_roi[i].tunnel_exp_perf = ot_ext_system_tunnel_exp_perf_read(vi_pipe, i);
    }
    if (ot_ext_system_tunnel_update_read(vi_pipe)) {
        tunnel_update_cnt++;
        ot_ext_system_tunnel_update_write(vi_pipe, TD_FALSE);
    } else {
        tunnel_wait_cnt++;
    }
    if (tunnel_update_cnt) {
        if (tunnel_wait_cnt < 20) {  /* experience value 20 */
            tunnel_in_time = TD_TRUE;
        } else {
            tunnel_in_time = TD_FALSE;
        }
        tunnel_update_cnt = 0;
        tunnel_wait_cnt   = 0;
    } else if (tunnel_wait_cnt >= 20) {  /* experience value 20 */
        tunnel_in_time = TD_FALSE;
    }
    if (!tunnel_in_time) {
        for (i = 0; i < OT_ISP_TUNNEL_CLASS_MAX; i++) {
            ae_info->smart_info.tunnel_roi[i].available = TD_FALSE;
        }
    }
}

static td_s32 isp_ae_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value);
static td_s32 isp_ae_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 i;
    ot_isp_ae_param ae_param;
    ot_isp_ae_init_info ae_init_info = { 0 };
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_lib_node *lib     = TD_NULL;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    ae_regs_default(vi_pipe, (isp_reg_cfg *)reg_cfg);
    ae_reg_cfg2_default(vi_pipe, (isp_reg_cfg *)reg_cfg);
    isp_sensor_get_blc(vi_pipe, &sns_black_level);

    ae_param.sns_id   = isp_ctx->bind_attr.sns_id;
    ae_param.wdr_mode    = isp_ctx->sns_wdr_mode;
    ae_param.hdr_mode    = isp_ctx->hdr_attr.dynamic_range;
    ae_param.bayer       = ot_ext_system_rggb_cfg_read(vi_pipe);
    ae_param.fps         = isp_ctx->sns_image_mode.fps;
    ae_param.black_level = sns_black_level->auto_attr.black_level[0][1];

    ae_param.stitch_attr.main_pipe       = isp_ctx->stitch_attr.main_pipe;
    ae_param.stitch_attr.stitch_enable   = isp_ctx->stitch_attr.stitch_enable;
    ae_param.stitch_attr.stitch_pipe_num = isp_ctx->stitch_attr.stitch_pipe_num;
    (td_void)memcpy_s(ae_param.stitch_attr.stitch_bind_id, sizeof(td_s8) * OT_ISP_MAX_STITCH_NUM,
                      isp_ctx->stitch_attr.stitch_bind_id, sizeof(td_s8) * OT_ISP_MAX_STITCH_NUM);

    /* init all registered ae libs */
    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (isp_ctx->ae_lib_info.libs[i].used) {
            lib = &isp_ctx->ae_lib_info.libs[i];
            if (lib->ae_regsiter.ae_exp_func.pfn_ae_init != TD_NULL) {
                lib->ae_regsiter.ae_exp_func.pfn_ae_init(lib->alg_lib.id, &ae_param);
            }
            if (lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl != TD_NULL) {
                lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id,
                    OT_ISP_AE_INIT_INFO_GET, (td_void *)&ae_init_info);
            }
        }
    }
    isp_ctx->linkage.blc_fix.enable = TD_FALSE;
    isp_ctx->linkage.blc_fix.blc = 1024; // default 1024
    isp_ctx->linkage.isp_dgain = MAX2(ae_init_info.isp_dgain, 0x100);
    isp_ctx->linkage.iso = MAX2(ae_init_info.iso, OT_AE_ISO_MIN);
    isp_ctx->attach_info_ctrl.attach_info->init_iso = isp_ctx->linkage.iso;

    for (i = 0; i < ISP_SYNC_ISO_BUF_MAX; i++) {
        isp_ctx->linkage.sync_iso_buf[i]       = isp_ctx->linkage.iso;
        isp_ctx->linkage.sync_isp_dgain_buf[i] = isp_ctx->linkage.isp_dgain;
    }
    isp_ctx->linkage.sensor_iso =
        ((td_u64)isp_ctx->linkage.iso << OT_AE_ISP_DGAIN_SHIFT) / div_0_to_1(isp_ctx->linkage.isp_dgain);
    isp_ctx->linkage.sensor_iso = MAX2(isp_ctx->linkage.sensor_iso, OT_AE_ISO_MIN);
    return TD_SUCCESS;
}

static td_void ae_wdr_result_check(isp_usr_ctx *isp_ctx, ot_isp_ae_result *ae_result)
{
    td_u32 i = 0;
    td_u32 frm_num = 0;
    frm_num = is_2to1_wdr_mode(isp_ctx->sns_wdr_mode) ?
              2 : (is_3to1_wdr_mode(isp_ctx->sns_wdr_mode) ? 3 : 4); /* 2,3,4 */
    if (ae_result->again_sf < OT_AE_SNS_GAIN_MIN || ae_result->again_sf > OT_AE_SNS_GAIN_MAX) {
        isp_err_trace("again_sf within [%d %d] current is %u\n",
                      OT_AE_SNS_GAIN_MIN, OT_AE_SNS_GAIN_MAX, ae_result->again_sf);
    }
    if (ae_result->dgain_sf < OT_AE_SNS_GAIN_MIN || ae_result->dgain_sf > OT_AE_SNS_GAIN_MAX) {
        isp_err_trace("dgain_sf within [%d %d] current is %u\n",
                      OT_AE_SNS_GAIN_MIN, OT_AE_SNS_GAIN_MAX, ae_result->dgain_sf);
    }
    for (i = 0; i < frm_num; i++) {
        if (ae_result->int_time[i] <= 0) {
            isp_err_trace("int_time[%u] should not be less than 1, current is %u\n", i, ae_result->int_time[i]);
        }
        if (ae_result->wdr_gain[i] < OT_AE_ISPDGAIN_MIN || ae_result->wdr_gain[i] > OT_AE_ISPDGAIN_MAX) {
            isp_err_trace("wdr_gain[%u] within [%d %d] current is %u\n", i,
                          OT_AE_ISPDGAIN_MIN, OT_AE_ISPDGAIN_MAX, ae_result->wdr_gain[i]);
        }
    }
    if (ae_result->iso_sf < OT_AE_ISO_MIN) {
        isp_err_trace("ae iso_sf should bigger than %d current is %d\n", OT_AE_ISO_MIN, ae_result->iso_sf);
    }
}

static td_void ae_result_check(isp_usr_ctx *isp_ctx, ot_isp_ae_result *ae_result)
{
    if (is_linear_mode(isp_ctx->sns_wdr_mode) || is_built_in_wdr_mode(isp_ctx->sns_wdr_mode)) {
        if (ae_result->int_time[0] <= 0) {
            isp_err_trace("int_time 0 should not be less than 1, current is %u\n", ae_result->int_time[0]);
        }
    } else {
        ae_wdr_result_check(isp_ctx, ae_result);
    }
    if (ae_result->again < OT_AE_SNS_GAIN_MIN || ae_result->again > OT_AE_SNS_GAIN_MAX) {
        isp_err_trace("again within [%d %d] current is %u\n",
                      OT_AE_SNS_GAIN_MIN, OT_AE_SNS_GAIN_MAX, ae_result->again);
    }
    if (ae_result->dgain < OT_AE_SNS_GAIN_MIN || ae_result->dgain > OT_AE_SNS_GAIN_MAX) {
        isp_err_trace("dgain within [%d %d] current is %u\n",
                      OT_AE_SNS_GAIN_MIN, OT_AE_SNS_GAIN_MAX, ae_result->dgain);
    }
    if (ae_result->isp_dgain < OT_AE_ISPDGAIN_MIN || ae_result->isp_dgain > OT_AE_ISPDGAIN_MAX) {
        isp_err_trace("isp_dgain within [%d %d] current is %u\n",
                      OT_AE_ISPDGAIN_MIN, OT_AE_ISPDGAIN_MAX, ae_result->isp_dgain);
    }
    if (ae_result->ae_run_interval < 1) {
        isp_err_trace("ae_run_interval should bigger than 0 current is %u\n", ae_result->ae_run_interval);
    }
    if (ae_result->iso < OT_AE_ISO_MIN) {
        isp_err_trace("ae iso should bigger than %d current is %u\n", OT_AE_ISO_MIN, ae_result->iso);
    }
}
td_void ae_run_entry(ot_vi_pipe vi_pipe, isp_lib_node *lib, ot_isp_ae_info *ae_info, ot_isp_ae_result *ae_result,
    isp_reg_cfg *reg_cfg)
{
    td_s32 ret = TD_SUCCESS;
    isp_usr_ctx *isp_ctx    = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    if (lib->ae_regsiter.ae_exp_func.pfn_ae_run != TD_NULL) {
        ret = lib->ae_regsiter.ae_exp_func.pfn_ae_run(lib->alg_lib.id, ae_info, ae_result, 0);
        ae_result_check(isp_ctx, ae_result);
        if (ret != TD_SUCCESS) {
            isp_err_trace("WARNING!! ISP[%d] run ae lib err!\n", vi_pipe);
        }
    } else if (lib->ae_regsiter.ae_exp_func.pfn_thermo_run != TD_NULL) {
        isp_thermo_run_callback(vi_pipe, lib, ae_info, ae_result, reg_cfg);
    }
    ret = memcpy_s(&isp_ctx->ae_result, sizeof(ot_isp_ae_result), ae_result, sizeof(ot_isp_ae_result));
    isp_check_eok_void(ret);

    return;
}

static td_void ot_ae_run_callback(ot_vi_pipe vi_pipe, ot_isp_ae_info *ae_info, isp_stat *stat_info,
    ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    td_s8  stitch_main_pipe;
    isp_usr_ctx *isp_ctx    = TD_NULL;
    isp_usr_ctx *isp_ctx_ex = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_lib_node *lib = &isp_ctx->ae_lib_info.libs[isp_ctx->ae_lib_info.active_lib];

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];

        if (is_stitch_main_pipe(vi_pipe, stitch_main_pipe)) {
            ae_info->fe_ae_stat1 = &stat_info->stitch_stat.fe_ae_stat1;
            ae_info->fe_ae_stat2 = &stat_info->stitch_stat.fe_ae_stat2;
            ae_info->be_ae_stat1 = &stat_info->stitch_stat.be_ae_stat1;
            ae_info->be_ae_stat2 = &stat_info->stitch_stat.be_ae_stat2;
            ae_info->fe_ae_sti_stat = &stat_info->stitch_stat.fe_ae_stat3;
            ae_info->be_ae_sti_stat = &stat_info->stitch_stat.be_ae_stat3;
            ae_run_entry(vi_pipe, lib, ae_info, ae_result, reg_cfg);
        } else {
            isp_get_ctx(stitch_main_pipe, isp_ctx_ex);
            (td_void)memcpy_s(ae_result, sizeof(ot_isp_ae_result), &isp_ctx_ex->ae_result, sizeof(ot_isp_ae_result));
        }
    } else {
        ae_run_entry(vi_pipe, lib, ae_info, ae_result, reg_cfg);
    }
}
static td_void ae_result_adjust(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result, td_bool *isp_dgain_forward)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    td_u8 lf_index = 0;
    td_bool forward = TD_FALSE;
    if (is_linear_mode(isp_ctx->sns_wdr_mode)) {
        if (ot_ext_system_aibnr_en_read(vi_pipe) == TD_TRUE) {
            ae_result->wdr_gain[0] = ae_result->isp_dgain;
            ae_result->isp_dgain = 0x100;
            forward = TD_TRUE;
        } else {
            if (ae_result->isp_dgain > OT_AE_ISP_DGAIN_THRESHOLD) {
                ae_result->wdr_gain[0] =
                    ((td_u64)ae_result->isp_dgain << OT_AE_ISP_DGAIN_SHIFT) / OT_AE_ISP_DGAIN_THRESHOLD;
                ae_result->isp_dgain = OT_AE_ISP_DGAIN_THRESHOLD;
            }
        }
    } else if (ae_result->fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE || (ae_result->fswdr_mode ==
        OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE && ae_result->int_time[0] == ae_result->int_time[1])) {
        lf_index = is_2to1_wdr_mode(isp_ctx->sns_wdr_mode) ?
            1 : (is_3to1_wdr_mode(isp_ctx->sns_wdr_mode) ? 2 : 3); /* 2 3 */
        if (ot_ext_system_aibnr_en_read(vi_pipe) == TD_TRUE) {
            ae_result->wdr_gain[lf_index] = ae_result->isp_dgain;
            ae_result->isp_dgain = 0x100;
            forward = TD_TRUE;
        } else {
            if (ae_result->isp_dgain > OT_AE_ISP_DGAIN_THRESHOLD) {
                ae_result->wdr_gain[lf_index] =
                    ((td_u64)ae_result->isp_dgain << OT_AE_ISP_DGAIN_SHIFT) / OT_AE_ISP_DGAIN_THRESHOLD;
                ae_result->isp_dgain = OT_AE_ISP_DGAIN_THRESHOLD;
            }
        }
    }

    if (ae_result->again_sf == 0 || ae_result->dgain_sf == 0 ||
        ae_result->isp_dgain_sf == 0 || ae_result->iso_sf == 0) {
        ae_result->again_sf = ae_result->again;
        ae_result->dgain_sf = ae_result->dgain;
        ae_result->isp_dgain_sf = ae_result->isp_dgain;
        ae_result->iso_sf = ae_result->iso;
    }
    ae_result->sns_lhcg_exp_ratio = (ae_result->sns_lhcg_exp_ratio == 0) ?
                                   OT_AE_WDR_EXP_RATIO_MIN : ae_result->sns_lhcg_exp_ratio;

    *isp_dgain_forward = forward;
}

static td_s32 ae_not_run_check(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, const isp_linkage *linkage)
{
    if (linkage->defect_pixel || linkage->snap_state) {
        return TD_SUCCESS;
    }

#ifdef CONFIG_OT_SNAP_SUPPORT
    if ((is_online_mode(isp_ctx->block_attr.running_mode)) &&
        (isp_ctx->linkage.snap_pipe_mode == ISP_SNAP_PICTURE)) {
        isp_check_pipe_return(isp_ctx->linkage.preview_pipe_id);
        isp_ctx->linkage.iso_done_frm_cnt = isp_frame_cnt_get(isp_ctx->linkage.preview_pipe_id);
        return TD_SUCCESS;
    }
#endif

    if ((isp_ctx->ai_info.pq_ai_en == TD_TRUE) && (isp_ctx->ai_info.ai_pipe_id == vi_pipe)) {
        isp_check_pipe_return(isp_ctx->ai_info.ai_pipe_id);
        isp_ctx->linkage.iso_done_frm_cnt = isp_frame_cnt_get(isp_ctx->ai_info.ai_pipe_id);

        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.stat_ready == TD_FALSE) {
        return TD_SUCCESS;
    }

    return TD_FAILURE;
}

static td_void ae_update_fast_face_info(ot_vi_pipe vi_pipe, ot_isp_ae_info *ae_info)
{
    td_u16 i;
    ae_info->frame_width  = ot_ext_system_be_total_width_read(vi_pipe);
    ae_info->frame_height = ot_ext_system_be_total_height_read(vi_pipe);
    ae_info->smart_info.face_roi.enable = ot_ext_system_fast_face_enable_read(vi_pipe);
    ae_info->smart_info.face_roi.available = ot_ext_system_fast_face_available_read(vi_pipe);
    ae_info->smart_info.face_roi.frame_pts = ot_ext_system_fast_face_frame_pts_read(vi_pipe);
    for (i = 0; i < OT_ISP_FACE_NUM; i++) {
        ae_info->smart_info.face_roi.face_rect[i].x = ot_ext_system_fast_face_x_read(vi_pipe, i);
        ae_info->smart_info.face_roi.face_rect[i].y = ot_ext_system_fast_face_y_read(vi_pipe, i);
        ae_info->smart_info.face_roi.face_rect[i].width = ot_ext_system_fast_face_width_read(vi_pipe, i);
        ae_info->smart_info.face_roi.face_rect[i].height = ot_ext_system_fast_face_height_read(vi_pipe, i);
    }
}

static td_void ae_result_init(ot_isp_ae_result *ae_result)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; ++i) {
        ae_result->wdr_gain[i] = (1 << OT_AE_ISP_DGAIN_SHIFT);
    }
}

static td_void ae_calc_extend_stats(ot_vi_pipe vi_pipe, td_u32 *estimate_be_hist,
    ot_isp_fe_ae_stat_1 *fe_ae_stat1, const isp_be_stats_calc_info *be_stats_calc_info)
{
    ot_isp_stats_ctrl stat_key;
    stat_key.key = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    if ((td_u32)stat_key.bit1_extend_stats == 1) {
        isp_be_stats_estimate(vi_pipe, estimate_be_hist, fe_ae_stat1, be_stats_calc_info);
    }
}

static td_s32 isp_ae_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_input, td_s32 rsv)
{
    td_s32 ret;
    td_u16 black_offset;
    td_u8 rc_enable, bayer_format;
    td_bool isp_dgain_forward = TD_FALSE;
    ot_unused(rsv);

    ot_isp_ae_info   ae_info    = {0};
    ot_isp_ae_result ae_result  = {{0}};
    ae_result_init(&ae_result);

    isp_usr_ctx      *isp_ctx   = TD_NULL;
    isp_stat         *ae_stat_info = (isp_stat *)stat_info;
    isp_reg_cfg      *reg_cfg   = (isp_reg_cfg *)reg_cfg_input;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_lib_node     *lib     = &isp_ctx->ae_lib_info.libs[isp_ctx->ae_lib_info.active_lib];
    isp_linkage      *linkage   = &isp_ctx->linkage;

    if (ae_not_run_check(vi_pipe, isp_ctx, linkage) == TD_SUCCESS) {
        return TD_SUCCESS;
    }

    ae_calc_extend_stats(vi_pipe, ae_stat_info->be_ae_stat1.estimate_histogram_mem_array,
        &ae_stat_info->fe_ae_stat1, &ae_stat_info->be_stats_calc_info);

    ae_info.frame_cnt = isp_ctx->frame_cnt;
    ae_info.fe_ae_stat1 = &ae_stat_info->fe_ae_stat1;
    ae_info.fe_ae_stat2 = &ae_stat_info->fe_ae_stat2;
    ae_info.fe_ae_stat3 = &ae_stat_info->fe_ae_stat3;
    ae_info.be_ae_stat1 = &ae_stat_info->be_ae_stat1;
    ae_info.be_ae_stat2 = &ae_stat_info->be_ae_stat2;
    ae_info.be_ae_stat3 = &ae_stat_info->be_ae_stat3;

    ae_update_people_info(vi_pipe, &ae_info);
    ae_update_tunnel_info(vi_pipe, &ae_info);
    ae_update_fast_face_info(vi_pipe, &ae_info);
    if (lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl != TD_NULL) {
        bayer_format = ot_ext_system_rggb_cfg_read(vi_pipe);
        lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id, OT_ISP_AE_BAYER_FORMAT_SET, (td_void *)&bayer_format);
        black_offset = (td_u16)(ot_ext_system_black_level_query_f0_gr_read(vi_pipe) >> 2); /* shift 2bits to 12bits */
        lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id, OT_ISP_AE_BLC_SET, (td_void *)&black_offset);
        rc_enable = ot_ext_system_rc_en_read(vi_pipe);
        lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id, OT_ISP_AE_RC_SET, (td_void *)&rc_enable);
    }

    ot_ae_run_callback(vi_pipe, &ae_info, ae_stat_info, &ae_result, reg_cfg);

    ae_result_adjust(vi_pipe, &ae_result, &isp_dgain_forward);
    ot_ext_system_sys_iso_write(vi_pipe, ae_result.iso);

    ae_read_extregs(vi_pipe, &ae_result);
    ae_res_read_extregs(vi_pipe, reg_cfg);
    ae_regs_range_check(vi_pipe, &ae_result);
    ae_update_config(vi_pipe, &ae_result, reg_cfg);
    ae_update_linkage(vi_pipe, &ae_result, linkage);
    ret = ae_get_frame_info(vi_pipe, &ae_result, isp_dgain_forward);
    ret += ae_get_dcf_info(vi_pipe, &ae_result);

    return ret;
}

static td_void isp_ae_proc_write_wdr_exp_param(const isp_usr_ctx *isp_ctx,
    ot_isp_ctrl_proc_write *proc, ot_isp_ctrl_proc_write *proc_tmp)
{
    isp_proc_printf(proc_tmp, proc->write_len,
                    "%11s"    "%11s"  "%11s"     "%11s"       "%11s"    "%11s"       "%11s"  "%11s\n",
                    "again", "dgain", "again_sf", "dgain_sf", "isp_dg", "isp_dg_sf", "iso",  "iso_sf");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%11u"    "%11u"    "%11u"    "%11u"     "%11u"    "%11u"    "%11u"   "%11u\n\n",
                    (td_u32)((td_u64)isp_ctx->ae_result.again),
                    (td_u32)((td_u64)isp_ctx->ae_result.dgain),
                    (td_u32)((td_u64)isp_ctx->ae_result.again_sf),
                    (td_u32)((td_u64)isp_ctx->ae_result.dgain_sf),
                    (td_u32)(((td_u64)isp_ctx->ae_result.isp_dgain << OT_AE_GAIN_USER_LINEAR_SHIFT)
                        >> OT_AE_ISP_DGAIN_SHIFT),
                    (td_u32)(((td_u64)isp_ctx->ae_result.isp_dgain_sf << OT_AE_GAIN_USER_LINEAR_SHIFT)
                                       >> OT_AE_ISP_DGAIN_SHIFT),
                    isp_ctx->ae_result.iso,
                    isp_ctx->ae_result.iso_sf);

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15s"         "%15s"         "%15s"         "%15s"         "%11s"       "%16s\n",
                    "int_time[0]", "int_time[1]", "int_time[2]", "int_time[3]", "ae_inter",  "sns_ratio");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15u"   "%15u"   "%15u"   "%15u"   "%11u"   "%16u\n\n",
                    isp_ctx->ae_result.int_time[0],
                    isp_ctx->ae_result.int_time[1],
                    isp_ctx->ae_result.int_time[2], /* 2 */
                    isp_ctx->ae_result.int_time[3], /* 3 */
                    isp_ctx->ae_result.ae_run_interval,
                    isp_ctx->ae_result.sns_lhcg_exp_ratio);

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15s"         "%15s"         "%15s"         "%15s"         "%16s\n",
                    "wdr_gain[0]", "wdr_gain[1]", "wdr_gain[2]", "wdr_gain[3]", "fswdr_mode");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15u"   "%15u"   "%15u"   "%15u"   "%16u\n\n",
                    isp_ctx->ae_result.wdr_gain[0],
                    isp_ctx->ae_result.wdr_gain[1],
                    isp_ctx->ae_result.wdr_gain[2], /* 2 */
                    isp_ctx->ae_result.wdr_gain[3], /* 3 */
                    isp_ctx->ae_result.fswdr_mode);

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15s"         "%15s"         "%15s"         "%15s"         "%11s\n",
                    "piris_valid", "piris_pos",   "piris_gain", "hmax_times",   "vmax");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15d"   "%15d"   "%15u"   "%15u"   "%11u\n\n",
                    isp_ctx->ae_result.piris_valid,
                    isp_ctx->ae_result.piris_pos,
                    isp_ctx->ae_result.piris_gain,
                    isp_ctx->ae_result.hmax_times,
                    isp_ctx->ae_result.vmax);
}

static td_void isp_ae_proc_write_linear_exp_param(const isp_usr_ctx *isp_ctx,
    ot_isp_ctrl_proc_write *proc, ot_isp_ctrl_proc_write *proc_tmp)
{
    isp_proc_printf(proc_tmp, proc->write_len,
                    "%11s"    "%11s"    "%11s"     "%11s\n",
                    "again",  "dgain",  "isp_dg",  "iso");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%11u"    "%11u"    "%11u"    "%11u\n\n",
                    (td_u32)((td_u64)isp_ctx->ae_result.again),
                    (td_u32)((td_u64)isp_ctx->ae_result.dgain),
                    (td_u32)(((td_u64)isp_ctx->ae_result.isp_dgain << OT_AE_GAIN_USER_LINEAR_SHIFT)
                        >> OT_AE_ISP_DGAIN_SHIFT),
                    isp_ctx->ae_result.iso);

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15s"         "%15s"         "%15s"         "%15s"         "%11s"       "%16s\n",
                    "int_time[0]", "int_time[1]", "int_time[2]", "int_time[3]", "ae_inter",  "sns_ratio");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15u"   "%15u"   "%15u"   "%15u"   "%11u"   "%16u\n\n",
                    isp_ctx->ae_result.int_time[0],
                    isp_ctx->ae_result.int_time[1],
                    isp_ctx->ae_result.int_time[2], /* 2 */
                    isp_ctx->ae_result.int_time[3], /* 3 */
                    isp_ctx->ae_result.ae_run_interval,
                    isp_ctx->ae_result.sns_lhcg_exp_ratio);

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15s"         "%15s"         "%15s"         "%15s"         "%11s\n",
                    "piris_valid", "piris_pos",   "piris_gain", "hmax_times",   "vmax");

    isp_proc_printf(proc_tmp, proc->write_len,
                    "%15u"   "%15d"   "%15u"   "%15u"   "%11u\n\n",
                    isp_ctx->ae_result.piris_valid,
                    isp_ctx->ae_result.piris_pos,
                    isp_ctx->ae_result.piris_gain,
                    isp_ctx->ae_result.hmax_times,
                    isp_ctx->ae_result.vmax);
}

static td_s32 isp_ae_proc_write(ot_isp_ctrl_proc_write *proc, isp_usr_ctx *isp_ctx)
{
    ot_isp_ctrl_proc_write proc_tmp;

    if ((proc->proc_buff == TD_NULL) ||
        (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    if (proc->write_len > 0) {
        proc->proc_buff[proc->write_len - 1] = '\n';
    }

    proc->proc_buff = &proc->proc_buff[proc->write_len];
    proc->buff_len  = proc->buff_len - proc->write_len;
    proc->write_len = 0;

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "isp ae info");
    if (is_fs_wdr_mode(isp_ctx->wdr_attr.wdr_mode)) {
        isp_ae_proc_write_wdr_exp_param(isp_ctx, proc, &proc_tmp);
    } else {
        isp_ae_proc_write_linear_exp_param(isp_ctx, proc, &proc_tmp);
    }

    proc->write_len += 1;

    return TD_SUCCESS;
}

static td_s32 isp_ae_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    td_s32 i;
    td_s32 ret = TD_FAILURE;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    td_s8 stitch_main_pipe;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];
        if (!is_stitch_main_pipe(vi_pipe, stitch_main_pipe)) {
            isp_get_ctx(stitch_main_pipe, isp_ctx);
        }
    }

    lib = &isp_ctx->ae_lib_info.libs[isp_ctx->ae_lib_info.active_lib];

    if (cmd == OT_ISP_PROC_WRITE) {
        if (lib->used) {
            if (lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl != TD_NULL) {
                ret = lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id, cmd, value);
            }
        }
        isp_ae_proc_write(value, isp_ctx);
    } else {
        for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
            if (isp_ctx->ae_lib_info.libs[i].used) {
                lib = &isp_ctx->ae_lib_info.libs[i];
                ret = (lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl != TD_NULL ?
                       lib->ae_regsiter.ae_exp_func.pfn_ae_ctrl(lib->alg_lib.id, cmd, value) : TD_FAILURE);
            }
        }
    }

    if (cmd == OT_ISP_WDR_MODE_SET) {
        ae_regs_default(vi_pipe, &reg_cfg->reg_cfg);
    }

    if (cmd == OT_ISP_CHANGE_IMAGE_MODE_SET) {
        ae_regs_default(vi_pipe, &reg_cfg->reg_cfg);
    }

    return ret;
}

static td_s32 isp_ae_exit(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (isp_ctx->ae_lib_info.libs[i].used) {
            lib = &isp_ctx->ae_lib_info.libs[i];
            if (lib->ae_regsiter.ae_exp_func.pfn_ae_exit != TD_NULL) {
                lib->ae_regsiter.ae_exp_func.pfn_ae_exit(lib->alg_lib.id);
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_alg_register_ae(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_alg_check_return(isp_ctx->alg_key.bit1_ae);

    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_AE;
    algs->alg_func.pfn_alg_init = isp_ae_init;
    algs->alg_func.pfn_alg_run  = isp_ae_run;
    algs->alg_func.pfn_alg_ctrl = isp_ae_ctrl;
    algs->alg_func.pfn_alg_exit = isp_ae_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
