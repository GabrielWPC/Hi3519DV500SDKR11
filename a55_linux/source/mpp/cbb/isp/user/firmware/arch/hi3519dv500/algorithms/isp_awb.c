/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_proc.h"
#include "isp_math_utils.h"
#include "ot_math.h"

#define ISP_AWB_GAIN_MAX_THR            0xFFF
#define ISP_AWB_STAT_CR_CB_MAX_THR      0xFFF
#define ISP_AWB_STAT_Y_MAX_THR          0xFFFF
#define ISP_AWB_RIGHT_SHIFT             8
#define ISP_CCM_NORM_VALUE              0x100
#define isp_awb_rshift8(x)    (((x) + 0x80) / 0x100)

static td_u32 g_d50_wb_gain_recomm[OT_ISP_BAYER_CHN_NUM] = {0x1ED, 0x100, 0x100, 0x1AB};
static td_u16 g_identity_matrix[OT_ISP_CCM_MATRIX_SIZE] = {0x100, 0x0, 0x0, 0x0, 0x100, 0x0, 0x0, 0x0, 0x100};

typedef struct {
    td_u8 crop_en;
    td_u16 overlap;
    td_u32 crop_x;
    td_u32 crop_y;
    td_u32 crop_height;
    td_u32 crop_width;
} isp_awb_crop_info;

typedef struct {
    td_u16 blend_ratio;
    td_u16 strength;
    td_u16 scale;
    td_u16 index;
    td_u16 gain_shift;
    td_u16 mesh_lut_r[OT_ISP_LSC_GRID_ROW][OT_ISP_LSC_GRID_COL];
    td_u16 mesh_lut_g[OT_ISP_LSC_GRID_ROW][OT_ISP_LSC_GRID_COL];
    td_u16 mesh_lut_b[OT_ISP_LSC_GRID_ROW][OT_ISP_LSC_GRID_COL];
} isp_awb_lsc_lut;

typedef struct {
    ot_isp_awb_stat_1 awb_glob_stat;
    ot_isp_awb_stat_2 awb_zone_stat;
} isp_awb_stat_info;


isp_awb_stat_info *g_awb_stat_info[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define awb_get_stat_info(pipe, stat)   ((stat) = g_awb_stat_info[pipe])
#define awb_set_stat_info(pipe, stat)   (g_awb_stat_info[pipe] = (stat))
#define awb_reset_stat_info(pipe)       (g_awb_stat_info[pipe] = TD_NULL)

static td_s32 awb_stat_info_init(ot_vi_pipe vi_pipe)
{
    isp_awb_stat_info *awb_stat = TD_NULL;

    awb_get_stat_info(vi_pipe, awb_stat);

    if (awb_stat == TD_NULL) {
        awb_stat = (isp_awb_stat_info *)isp_malloc(sizeof(isp_awb_stat_info));
        if (awb_stat == TD_NULL) {
            isp_err_trace("Isp[%d] awb stat malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }
    memset_s(awb_stat, sizeof(isp_awb_stat_info), 0, sizeof(isp_awb_stat_info));

    awb_set_stat_info(vi_pipe, awb_stat);

    return TD_SUCCESS;
}

static td_void awb_stat_info_exit(ot_vi_pipe vi_pipe)
{
    isp_awb_stat_info *awb_stat = TD_NULL;

    awb_get_stat_info(vi_pipe, awb_stat);
    isp_free(awb_stat);
    awb_reset_stat_info(vi_pipe);
}

/* multiply the two matrixes a1[dim1 x dim2] a2[dim2 x dim3] */
static td_void awb_matrix_multiply(const td_s16 *matrix1, const td_s16 *matrix2, td_s16 *result, td_s32 dim)
{
    td_s32 i, j, k;
    td_s32 temp;

    for (i = 0; i < dim; ++i) {
        for (j = 0; j < dim; ++j) {
            temp = 0;

            for (k = 0; k < dim; ++k) {
                temp += (((td_s32)matrix1[i * dim + k] * matrix2[k * dim + j]));
            }

            result[i * dim + j] = (td_s16)isp_awb_rshift8(temp);
        }
    }

    for (i = 0; i < dim; ++i) {
        temp = 0;

        for (j = 0; j < dim; ++j) {
            temp += (td_s32)result[i * dim + j];
        }

        if (temp != 0x0) {
            for (j = 0; j < dim; ++j) {
                result[i * dim + j] = (td_s16)(result[i * dim + j] * ISP_CCM_NORM_VALUE / div_0_to_1(temp));
            }
        }
    }

    for (i = 0; i < dim; ++i) {
        temp = 0;

        for (j = 0; j < dim; ++j) {
            temp += (td_s16)result[i * dim + j];
        }

        if (temp != ISP_CCM_NORM_VALUE) {
            result[i * dim + i] += (ISP_CCM_NORM_VALUE - temp);
        }
    }
}

static td_void awb_res_regs_default(ot_vi_pipe vi_pipe, td_u8 i, td_u8 block_num, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg)
{
    td_u16 overlap;
    isp_rect block_rect;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    /* BEDYNAMIC */
    isp_get_block_rect(&block_rect, &isp_ctx->block_attr, i);
    awb_reg_dyn_cfg->be_width  = block_rect.width;
    awb_reg_dyn_cfg->be_height = block_rect.height;

    overlap = isp_ctx->block_attr.over_lap;

    /* awb overlap configs */
    if (i == 0) {
        if (block_num > 1) {
            awb_reg_dyn_cfg->be_crop_pos_x      = 0;
            awb_reg_dyn_cfg->be_crop_pos_y      = 0;
            awb_reg_dyn_cfg->be_crop_out_width  = awb_reg_dyn_cfg->be_width - overlap;
            awb_reg_dyn_cfg->be_crop_out_height = awb_reg_dyn_cfg->be_height;
        } else {
            awb_reg_dyn_cfg->be_crop_pos_x      = 0;
            awb_reg_dyn_cfg->be_crop_pos_y      = 0;
            awb_reg_dyn_cfg->be_crop_out_height = block_rect.height;
            awb_reg_dyn_cfg->be_crop_out_width  = block_rect.width;
        }
    } else if (i == (block_num - 1)) {
        awb_reg_dyn_cfg->be_crop_pos_x      = overlap;
        awb_reg_dyn_cfg->be_crop_pos_y      = 0;
        awb_reg_dyn_cfg->be_crop_out_width  = awb_reg_dyn_cfg->be_width - overlap;
        awb_reg_dyn_cfg->be_crop_out_height = awb_reg_dyn_cfg->be_height;
    } else {
        awb_reg_dyn_cfg->be_crop_pos_x      = overlap;
        awb_reg_dyn_cfg->be_crop_pos_y      = 0;
        awb_reg_dyn_cfg->be_crop_out_width  = awb_reg_dyn_cfg->be_width - (overlap << 1);
        awb_reg_dyn_cfg->be_crop_out_height = awb_reg_dyn_cfg->be_height;
    }

    ot_ext_system_awb_crop_en_write(vi_pipe, OT_EXT_SYSTEM_AWB_CROP_EN_DEFAULT);
    ot_ext_system_awb_crop_x_write(vi_pipe, 0);
    ot_ext_system_awb_crop_y_write(vi_pipe, 0);
    ot_ext_system_awb_crop_height_write(vi_pipe, isp_ctx->block_attr.frame_rect.height);
    ot_ext_system_awb_crop_width_write(vi_pipe, isp_ctx->block_attr.frame_rect.width);
}

static td_void awb_res_read_ext_crop_regs(td_u8 i, td_u8 block_num, isp_awb_crop_info *awb_crop, \
                                          isp_rect *block_rect, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg)
{
    if (i == 0) {
        if (block_num > 1) {
            awb_reg_dyn_cfg->be_crop_pos_x = 0;
            awb_reg_dyn_cfg->be_crop_pos_y = 0;
            awb_reg_dyn_cfg->be_crop_out_height = block_rect->height;
            awb_reg_dyn_cfg->be_crop_out_width = block_rect->width - awb_crop->overlap;
        } else {
            if (awb_crop->crop_en) {
                awb_reg_dyn_cfg->be_crop_pos_x = awb_crop->crop_x;
                awb_reg_dyn_cfg->be_crop_pos_y = awb_crop->crop_y;
                awb_reg_dyn_cfg->be_crop_out_height = awb_crop->crop_height;
                awb_reg_dyn_cfg->be_crop_out_width = awb_crop->crop_width;
            } else {
                awb_reg_dyn_cfg->be_crop_pos_x = 0;
                awb_reg_dyn_cfg->be_crop_pos_y = 0;
                awb_reg_dyn_cfg->be_crop_out_height = block_rect->height;
                awb_reg_dyn_cfg->be_crop_out_width = block_rect->width;
            }
        }
    } else if (i == (block_num - 1)) {
        awb_reg_dyn_cfg->be_crop_pos_x = awb_crop->overlap;
        awb_reg_dyn_cfg->be_crop_pos_y = 0;
        awb_reg_dyn_cfg->be_crop_out_height = block_rect->height;
        awb_reg_dyn_cfg->be_crop_out_width = block_rect->width - awb_crop->overlap;
    } else {
        awb_reg_dyn_cfg->be_crop_pos_x = awb_crop->overlap;
        awb_reg_dyn_cfg->be_crop_pos_y = 0;
        awb_reg_dyn_cfg->be_crop_out_height = block_rect->height;
        awb_reg_dyn_cfg->be_crop_out_width = block_rect->width - (awb_crop->overlap << 1);
    }
}

static td_void awb_res_read_ext_regs(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8  i, awb_zone_row, awb_zone_col, block_num;
    isp_rect block_rect;
    isp_awb_crop_info awb_crop;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;

    awb_crop.overlap = isp_ctx->block_attr.over_lap;
    awb_crop.crop_en = ot_ext_system_awb_crop_en_read(vi_pipe);
    awb_crop.crop_x  = (ot_ext_system_awb_crop_x_read(vi_pipe) >> 1) << 1;
    awb_crop.crop_y  = (ot_ext_system_awb_crop_y_read(vi_pipe) >> 1) << 1;
    awb_crop.crop_height = (ot_ext_system_awb_crop_height_read(vi_pipe) >> 1) << 1;
    awb_crop.crop_width = (ot_ext_system_awb_crop_width_read(vi_pipe) >> 1) << 1;

    awb_zone_row = ot_ext_system_awb_vnum_read(vi_pipe);
    awb_zone_col = ot_ext_system_awb_hnum_read(vi_pipe);

    awb_crop.crop_height = clip3(awb_crop.crop_height, awb_zone_row * OT_ISP_AWB_MIN_HEIGHT,
                                 isp_ctx->block_attr.frame_rect.height);
    awb_crop.crop_width = clip3(awb_crop.crop_width, awb_zone_col * OT_ISP_AWB_MIN_WIDTH,
                                isp_ctx->block_attr.frame_rect.width);
    awb_crop.crop_y = MIN2(awb_crop.crop_y, (isp_ctx->block_attr.frame_rect.height - awb_crop.crop_height));
    awb_crop.crop_x = MIN2(awb_crop.crop_x, (isp_ctx->block_attr.frame_rect.width - awb_crop.crop_width));

    /* AWB BE configs */
    for (i = 0; i < block_num; i++) {
        /* AWB configs */
        awb_reg_dyn_cfg = &reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg;

        /* AWB size configs */
        isp_get_block_rect(&block_rect, &isp_ctx->block_attr, i);

        /* AWB overlap configs */
        awb_res_read_ext_crop_regs(i, block_num, &awb_crop, &block_rect, awb_reg_dyn_cfg);
    }
    reg_cfg->cfg_key.bit1_awb_dyn_cfg = 1;
    reg_cfg->kernel_reg_cfg.cfg_key.bit1_awb_cfg = 1;
}

static td_void awb_image_mode_set(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8  i;
    td_u8  awb_zone_col, awb_zone_row;
    td_u16 height, width;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_awb_reg_usr_cfg *awb_reg_usr_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    height = isp_ctx->block_attr.frame_rect.height;
    width = isp_ctx->block_attr.frame_rect.width;
    awb_zone_col = ot_ext_system_awb_hnum_read(vi_pipe);
    awb_zone_row = ot_ext_system_awb_vnum_read(vi_pipe);

    if (awb_zone_col * OT_ISP_AWB_MIN_WIDTH > width) {
        awb_zone_col = width / OT_ISP_AWB_MIN_WIDTH;
    }

    if (awb_zone_row * OT_ISP_AWB_MIN_HEIGHT > height) {
        awb_zone_row = height / OT_ISP_AWB_MIN_HEIGHT;
    }

    awb_zone_col = clip3(awb_zone_col, isp_ctx->block_attr.block_num, OT_ISP_AWB_ZONE_ORIG_COLUMN);
    awb_zone_row = clip3(awb_zone_row, 1, OT_ISP_AWB_ZONE_ORIG_ROW);

    ot_ext_system_awb_hnum_write(vi_pipe, awb_zone_col);
    ot_ext_system_awb_vnum_write(vi_pipe, awb_zone_row);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        awb_reg_usr_cfg = &reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_usr_cfg;

        awb_res_regs_default(vi_pipe, i, reg_cfg->cfg_num, &reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg);

        if (i < awb_zone_col % div_0_to_1(reg_cfg->cfg_num)) {
            awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(reg_cfg->cfg_num) + 1;
        } else {
            awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(reg_cfg->cfg_num);
        }

        awb_reg_usr_cfg->be_zone_row = awb_zone_row;

        awb_reg_usr_cfg->resh = TD_TRUE;
        awb_reg_usr_cfg->update_index += 1;
    }

    reg_cfg->cfg_key.bit1_awb_dyn_cfg = 1;
}

static td_void awb_regs_default_fe(isp_reg_cfg *reg_cfg)
{
    td_u8 i;

    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;
    isp_awb_reg_sta_cfg *awb_reg_sta_cfg = TD_NULL;

    awb_reg_dyn_cfg = &reg_cfg->alg_reg_cfg[0].awb_reg_cfg.awb_reg_dyn_cfg; /* dynamic */
    awb_reg_sta_cfg = &reg_cfg->alg_reg_cfg[0].awb_reg_cfg.awb_reg_sta_cfg; /* static */

    /* FE DYNAMIC */
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        awb_reg_dyn_cfg->fe_white_balance_gain[i] = g_d50_wb_gain_recomm[i];
    }

    awb_reg_dyn_cfg->fe_wb_work_en = TD_TRUE;

    /* FE STATIC */
    awb_reg_sta_cfg->fe_awb_sta_cfg = 1;
    awb_reg_sta_cfg->fe_clip_value = 0xFFFFFF;
}

static td_void awb_regs_default_be_sta(ot_vi_pipe vi_pipe, isp_awb_reg_sta_cfg *awb_reg_sta_cfg)
{
    awb_reg_sta_cfg->be_awb_sta_cfg = 1;
    awb_reg_sta_cfg->be_awb_bitmove = 0x0;
    awb_reg_sta_cfg->be_wb_work_en = TD_TRUE;
    awb_reg_sta_cfg->be_awb_work_en = TD_TRUE;

    awb_reg_sta_cfg->be_awb_stat_raddr = 0x000;
    awb_reg_sta_cfg->be_cc_colortone_en = 0x0;

    awb_reg_sta_cfg->be_top_k = 0x0;
    awb_reg_sta_cfg->be_top_b = 0xFFFFF;
    awb_reg_sta_cfg->be_bot_k = 0x0;
    awb_reg_sta_cfg->be_bot_b = 0x0;

    awb_reg_sta_cfg->be_cc_in_dc0 = 0x0;
    awb_reg_sta_cfg->be_cc_in_dc1 = 0x0;
    awb_reg_sta_cfg->be_cc_in_dc2 = 0x0;
    awb_reg_sta_cfg->be_cc_out_dc0 = 0x0;
    awb_reg_sta_cfg->be_cc_out_dc1 = 0x0;
    awb_reg_sta_cfg->be_cc_out_dc2 = 0x0;
    awb_reg_sta_cfg->be_wb_clip_value = 0xFFFFFF;
    awb_reg_sta_cfg->be_awb_offset_comp = OT_ISP_AWB_OFFSET_COMP_DEF;
    ot_ext_system_awb_sta_enable_write(vi_pipe, awb_reg_sta_cfg->be_awb_work_en);
}

static td_void awb_regs_default_be_dyn(ot_vi_pipe vi_pipe, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg)
{
    td_u8 j;

    for (j = 0; j < OT_ISP_CCM_MATRIX_SIZE; j++) {
        awb_reg_dyn_cfg->be_color_matrix[j] = g_identity_matrix[j];
    }

    for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
        awb_reg_dyn_cfg->be_white_balance_gain[j] = g_d50_wb_gain_recomm[j];
    }

    awb_reg_dyn_cfg->be_metering_white_level_awb = ISP_AWB_STAT_Y_MAX_THR;
    awb_reg_dyn_cfg->be_metering_black_level_awb = 0x0;
    awb_reg_dyn_cfg->be_metering_cr_ref_max_awb = 0x180;
    awb_reg_dyn_cfg->be_metering_cr_ref_min_awb = 0x40;
    awb_reg_dyn_cfg->be_metering_cb_ref_max_awb = 0x180;
    awb_reg_dyn_cfg->be_metering_cb_ref_min_awb = 0x40;

    awb_reg_dyn_cfg->be_wb_work_en = TD_TRUE;
    awb_reg_dyn_cfg->be_cc_en = TD_TRUE;
    awb_reg_dyn_cfg->be_cc_colortone_en = OT_ISP_CCM_COLORTONE_EN_DEFAULT;
    awb_reg_dyn_cfg->be_cc_r_gain = OT_ISP_CCM_COLORTONE_RGAIN_DEFAULT;
    awb_reg_dyn_cfg->be_cc_g_gain = OT_ISP_CCM_COLORTONE_GGAIN_DEFAULT;
    awb_reg_dyn_cfg->be_cc_b_gain = OT_ISP_CCM_COLORTONE_BGAIN_DEFAULT;

    ot_ext_system_cc_enable_write(vi_pipe, awb_reg_dyn_cfg->be_cc_en);
    ot_ext_system_awb_gain_enable_write(vi_pipe, awb_reg_dyn_cfg->be_wb_work_en);
    ot_ext_system_awb_white_level_write(vi_pipe, awb_reg_dyn_cfg->be_metering_white_level_awb);
    ot_ext_system_awb_black_level_write(vi_pipe, awb_reg_dyn_cfg->be_metering_black_level_awb);
    ot_ext_system_awb_cr_ref_max_write(vi_pipe, awb_reg_dyn_cfg->be_metering_cr_ref_max_awb);
    ot_ext_system_awb_cr_ref_min_write(vi_pipe, awb_reg_dyn_cfg->be_metering_cr_ref_min_awb);
    ot_ext_system_awb_cb_ref_max_write(vi_pipe, awb_reg_dyn_cfg->be_metering_cb_ref_max_awb);
    ot_ext_system_awb_cb_ref_min_write(vi_pipe, awb_reg_dyn_cfg->be_metering_cb_ref_min_awb);
    ot_ext_system_cc_colortone_rgain_write(vi_pipe, awb_reg_dyn_cfg->be_cc_r_gain);
    ot_ext_system_cc_colortone_ggain_write(vi_pipe, awb_reg_dyn_cfg->be_cc_g_gain);
    ot_ext_system_cc_colortone_bgain_write(vi_pipe, awb_reg_dyn_cfg->be_cc_b_gain);
}

static td_void awb_update_be_working_status(ot_vi_pipe vi_pipe, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg,
    ot_isp_awb_switch be_awb_switch)
{
    td_u8 ai_mode = 0;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if ((be_awb_switch == OT_ISP_AWB_AFTER_DRC) && (ai_mode == OT_ISP_AI_DRC)) {
        awb_reg_dyn_cfg->is_in_pre_be = TD_FALSE;
    } else {
        awb_reg_dyn_cfg->is_in_pre_be = TD_TRUE;
    }

    awb_reg_dyn_cfg->is_online = (is_online_mode(isp_ctx->block_attr.running_mode)) ||
        (is_pre_online_post_offline(isp_ctx->block_attr.running_mode) && (awb_reg_dyn_cfg->is_in_pre_be == TD_TRUE));
}

static td_void awb_regs_default(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, td_u8 awb_zone_row, td_u8 awb_zone_col)
{
    td_u8 i, block_num;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    block_num = isp_ctx->block_attr.block_num;

    awb_regs_default_fe(reg_cfg);

    reg_cfg->cfg_key.bit1_awb_dyn_cfg = 1;

    for (i = 0; i < block_num; i++) {
        isp_awb_reg_sta_cfg *awb_reg_sta_cfg = &(reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_sta_cfg); /* static */
        isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = &(reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg); /* dynamic */
        isp_awb_reg_usr_cfg *awb_reg_usr_cfg = &(reg_cfg->alg_reg_cfg[i].awb_reg_cfg.awb_reg_usr_cfg); /* user */

        awb_regs_default_be_sta(vi_pipe, awb_reg_sta_cfg);
        awb_regs_default_be_dyn(vi_pipe, awb_reg_dyn_cfg);

        awb_res_regs_default(vi_pipe, i, block_num, awb_reg_dyn_cfg);

        if (i < awb_zone_col % div_0_to_1(block_num)) {
            awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(block_num) + 1;
        } else {
            awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(block_num);
        }
        awb_reg_usr_cfg->be_zone_row = awb_zone_row;
        awb_reg_usr_cfg->be_awb_switch = OT_ISP_AWB_AFTER_DG;
        awb_reg_usr_cfg->resh = TD_TRUE;
        awb_reg_usr_cfg->update_index = 1;

        reg_cfg->kernel_reg_cfg.cfg_key.bit1_awb_cfg = 1;
        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[i].be_awb_zone_cfg.row = awb_zone_row;
        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[i].be_awb_zone_cfg.column = awb_zone_col;

        /* user */
        ot_ext_system_wb_statistics_mpi_update_en_write(vi_pipe, awb_reg_usr_cfg->resh);
        ot_ext_system_awb_switch_write(vi_pipe, awb_reg_usr_cfg->be_awb_switch);
        ot_ext_system_awb_hnum_write(vi_pipe, awb_zone_col); /* the col num of the whole picture */
        ot_ext_system_awb_vnum_write(vi_pipe, awb_zone_row);

        awb_update_be_working_status(vi_pipe, awb_reg_dyn_cfg, awb_reg_usr_cfg->be_awb_switch);
    }
}

static td_void awb_cfg_regs_dyn(ot_vi_pipe vi_pipe, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg,
                                const td_u32 *white_balance_gain, td_u16 *color_matrix)
{
    td_u8 i;

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        awb_reg_dyn_cfg->be_color_matrix[i] = color_matrix[i];
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        td_u16 gain = MIN2(isp_awb_rshift8(white_balance_gain[i]), ISP_AWB_GAIN_MAX_THR);
        awb_reg_dyn_cfg->be_white_balance_gain[i] = gain;
    }

    awb_reg_dyn_cfg->be_metering_cr_ref_max_awb = ot_ext_system_awb_cr_ref_max_read(vi_pipe);
    awb_reg_dyn_cfg->be_metering_cr_ref_min_awb = ot_ext_system_awb_cr_ref_min_read(vi_pipe);
    awb_reg_dyn_cfg->be_metering_cb_ref_max_awb = ot_ext_system_awb_cb_ref_max_read(vi_pipe);
    awb_reg_dyn_cfg->be_metering_cb_ref_min_awb = ot_ext_system_awb_cb_ref_min_read(vi_pipe);
    awb_reg_dyn_cfg->be_metering_white_level_awb = ot_ext_system_awb_white_level_read(vi_pipe);
    awb_reg_dyn_cfg->be_metering_black_level_awb = ot_ext_system_awb_black_level_read(vi_pipe);

    awb_reg_dyn_cfg->be_cc_en = ot_ext_system_cc_enable_read(vi_pipe);
    awb_reg_dyn_cfg->be_cc_b_gain = ot_ext_system_cc_colortone_bgain_read(vi_pipe);
    awb_reg_dyn_cfg->be_cc_g_gain = ot_ext_system_cc_colortone_ggain_read(vi_pipe);
    awb_reg_dyn_cfg->be_cc_r_gain = ot_ext_system_cc_colortone_rgain_read(vi_pipe);

    awb_reg_dyn_cfg->be_wb_work_en = ot_ext_system_awb_gain_enable_read(vi_pipe);
    awb_reg_dyn_cfg->fe_wb_work_en = ot_ext_system_awb_gain_enable_read(vi_pipe);
}

static td_void awb_result_check(ot_isp_awb_result *awb_result)
{
    td_u8 i, j;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        td_u32 gain = isp_awb_rshift8(awb_result->white_balance_gain[i]);
        if ((gain == 0) || (gain > ISP_AWB_GAIN_MAX_THR)) {
            isp_err_trace("awb result white_balance_gain[%u] %u should be in (%d, %d] \n",
                i, awb_result->white_balance_gain[i],
                0, (ISP_AWB_GAIN_MAX_THR << ISP_AWB_RIGHT_SHIFT));
        }
    }

    for (i = 0; i < OT_ISP_RGB_CHN_NUM; i++) {
        td_u32 ccm_sum = 0;
        for (j = i * OT_ISP_RGB_CHN_NUM; j < (i + 1) * OT_ISP_RGB_CHN_NUM; j++) {
            ccm_sum += awb_result->color_matrix[j];
        }
        if (ccm_sum == 0) {
            isp_err_trace("awb result color_matrix %d row sum is %u, should not be 0 \n", i, ccm_sum);
        }
    }

    if (awb_result->raw_stat_attr.metering_black_level_awb > awb_result->raw_stat_attr.metering_white_level_awb) {
        isp_err_trace("awb result black_level %u should not be larger than white_level %u \n",
            awb_result->raw_stat_attr.metering_black_level_awb, awb_result->raw_stat_attr.metering_white_level_awb);
    }
    if (awb_result->raw_stat_attr.metering_cr_ref_max_awb > ISP_AWB_STAT_CR_CB_MAX_THR) {
        isp_err_trace("awb result metering_cr_ref_max_awb %u should not be larger than %u \n",
            awb_result->raw_stat_attr.metering_cr_ref_max_awb, ISP_AWB_STAT_CR_CB_MAX_THR);
    }
    if (awb_result->raw_stat_attr.metering_cr_ref_min_awb > awb_result->raw_stat_attr.metering_cr_ref_max_awb) {
        isp_err_trace("awb result metering_cr_ref_min_awb %u is larger than cr_max %u \n",
            awb_result->raw_stat_attr.metering_cr_ref_min_awb, awb_result->raw_stat_attr.metering_cr_ref_max_awb);
    }
    if (awb_result->raw_stat_attr.metering_cb_ref_max_awb > ISP_AWB_STAT_CR_CB_MAX_THR) {
        isp_err_trace("awb result metering_cb_ref_max_awb %u should not be larger than %u \n",
            awb_result->raw_stat_attr.metering_cb_ref_max_awb, ISP_AWB_STAT_CR_CB_MAX_THR);
    }
    if (awb_result->raw_stat_attr.metering_cb_ref_min_awb > awb_result->raw_stat_attr.metering_cb_ref_max_awb) {
        isp_err_trace("awb result metering_cb_ref_min_awb %u is larger than cb_max %u \n",
            awb_result->raw_stat_attr.metering_cb_ref_min_awb, awb_result->raw_stat_attr.metering_cb_ref_max_awb);
    }
}

static td_void awb_cfg_regs(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result, isp_reg_cfg *reg_cfg)
{
    td_u8 i, k, blk_num, awb_zone_col;
    td_bool usr_resh;

    blk_num = ((isp_reg_cfg *)reg_cfg)->cfg_num;

    usr_resh = ot_ext_system_wb_statistics_mpi_update_en_read(vi_pipe);
    if (usr_resh == TD_TRUE) {
        ot_ext_system_wb_statistics_mpi_update_en_write(vi_pipe, TD_FALSE);
    }

    if (awb_result->raw_stat_attr.stat_cfg_update) { /* CbCr */
        ot_ext_system_awb_cr_ref_max_write(vi_pipe, \
            MIN2(awb_result->raw_stat_attr.metering_cr_ref_max_awb, ISP_AWB_STAT_CR_CB_MAX_THR));
        ot_ext_system_awb_cr_ref_min_write(vi_pipe, \
            MIN2(awb_result->raw_stat_attr.metering_cr_ref_min_awb, ISP_AWB_STAT_CR_CB_MAX_THR));
        ot_ext_system_awb_cb_ref_max_write(vi_pipe, \
            MIN2(awb_result->raw_stat_attr.metering_cb_ref_max_awb, ISP_AWB_STAT_CR_CB_MAX_THR));
        ot_ext_system_awb_cb_ref_min_write(vi_pipe, \
            MIN2(awb_result->raw_stat_attr.metering_cb_ref_min_awb, ISP_AWB_STAT_CR_CB_MAX_THR));
    }

    for (k = 0; k < blk_num; k++) {
        isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = &(reg_cfg->alg_reg_cfg[k].awb_reg_cfg.awb_reg_dyn_cfg); /* dynamic */
        isp_awb_reg_usr_cfg *awb_reg_usr_cfg = &(reg_cfg->alg_reg_cfg[k].awb_reg_cfg.awb_reg_usr_cfg); /* user */
        isp_awb_reg_sta_cfg *awb_reg_sta_cfg = &(reg_cfg->alg_reg_cfg[k].awb_reg_cfg.awb_reg_sta_cfg); /* sta */

        awb_cfg_regs_dyn(vi_pipe, awb_reg_dyn_cfg, awb_result->white_balance_gain, awb_result->color_matrix);

        awb_zone_col = ot_ext_system_awb_hnum_read(vi_pipe);

        if (usr_resh == TD_TRUE) {
            if (k < awb_zone_col % div_0_to_1(blk_num)) {
                awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(blk_num) + 1;
            } else {
                awb_reg_usr_cfg->be_zone_col = awb_zone_col / div_0_to_1(blk_num);
            }

            awb_reg_usr_cfg->update_index += 1;

            awb_reg_usr_cfg->be_awb_switch = ot_ext_system_awb_switch_read(vi_pipe);
            awb_reg_usr_cfg->be_zone_row = ot_ext_system_awb_vnum_read(vi_pipe);
            awb_reg_usr_cfg->resh = TD_TRUE;
        }

        awb_reg_sta_cfg->be_awb_work_en = ot_ext_system_awb_sta_enable_read(vi_pipe);

        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[k].be_awb_zone_cfg.row = awb_reg_usr_cfg->be_zone_row;
        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[k].be_awb_zone_cfg.column = awb_zone_col;
        reg_cfg->kernel_reg_cfg.alg_kernel_cfg[k].be_awb_zone_cfg.zone_bin = 1;
        awb_update_be_working_status(vi_pipe, awb_reg_dyn_cfg, awb_reg_usr_cfg->be_awb_switch);
    }

    /* FE */
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        td_u16 gain = MIN2(isp_awb_rshift8(awb_result->white_balance_gain[i]), ISP_AWB_GAIN_MAX_THR);
        reg_cfg->alg_reg_cfg[0].awb_reg_cfg.awb_reg_dyn_cfg.fe_white_balance_gain[i] = gain;
    }

    reg_cfg->cfg_key.bit1_awb_dyn_cfg = 1;
    reg_cfg->kernel_reg_cfg.cfg_key.bit1_awb_cfg = 1;
}

static td_void awb_cfg_init_regs(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result, isp_reg_cfg *reg_cfg)
{
    td_s32 i, k;
    td_u32 wb_gain[OT_ISP_BAYER_CHN_NUM];
    td_u8 blk_num;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;
    blk_num = ((isp_reg_cfg *)reg_cfg)->cfg_num;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        wb_gain[i] = awb_result->white_balance_gain[i];
        wb_gain[i] = isp_awb_rshift8(wb_gain[i]);
        wb_gain[i] = MIN2(wb_gain[i], ISP_AWB_GAIN_MAX_THR);
        reg_cfg->alg_reg_cfg[0].awb_reg_cfg.awb_reg_dyn_cfg.fe_white_balance_gain[i] = wb_gain[i];
    }

    for (k = 0; k < blk_num; k++) {
        awb_reg_dyn_cfg = &(reg_cfg->alg_reg_cfg[k].awb_reg_cfg.awb_reg_dyn_cfg); /* dynamic */

        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            awb_reg_dyn_cfg->be_color_matrix[i] = awb_result->color_matrix[i];
        }

        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            awb_reg_dyn_cfg->be_white_balance_gain[i] = wb_gain[i];
        }
    }

    /* init linkage */
    for (k = 0; k < ISP_SYNC_ISO_BUF_MAX; k++) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            isp_ctx->linkage.sync_wb_gain[k][i] = awb_result->white_balance_gain[i];
        }

        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            isp_ctx->linkage.sync_ccm[k][i] = awb_result->color_matrix[i];
        }

        isp_ctx->linkage.sync_color_temp[k] = 5000; /* default color_tem: 5000 */
    }

    reg_cfg->cfg_key.bit1_awb_dyn_cfg = 1;
}

static td_s32 isp_awb_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    td_u8  i, actual_zone_col, actual_zone_row;
    td_u16 height, width;
    ot_isp_awb_param awb_param;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;
    ot_isp_awb_result awb_init_result = {{0}, {0}, 0, 0, {0}};

    ret = awb_stat_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_get_ctx(vi_pipe, isp_ctx);

    height = isp_ctx->block_attr.frame_rect.height;
    width = isp_ctx->block_attr.frame_rect.width;

    actual_zone_col = MIN2(width, OT_ISP_AWB_ZONE_ORIG_COLUMN * OT_ISP_AWB_MIN_WIDTH) / OT_ISP_AWB_MIN_WIDTH;
    actual_zone_row = MIN2(height, OT_ISP_AWB_ZONE_ORIG_ROW * OT_ISP_AWB_MIN_HEIGHT) / OT_ISP_AWB_MIN_HEIGHT;

    awb_param.sns_id = isp_ctx->bind_attr.sns_id;
    awb_param.wdr_mode = isp_ctx->sns_wdr_mode;
    awb_param.init_iso = isp_ctx->linkage.iso;
    awb_param.stitch_attr.main_pipe = isp_ctx->stitch_attr.main_pipe;
    awb_param.stitch_attr.stitch_enable = isp_ctx->stitch_attr.stitch_enable;
    awb_param.stitch_attr.stitch_pipe_num = isp_ctx->stitch_attr.stitch_pipe_num;

    for (i = 0; i < OT_ISP_MAX_STITCH_NUM; i++) {
        awb_param.stitch_attr.stitch_bind_id[i] = isp_ctx->stitch_attr.stitch_bind_id[i];
    }

    awb_regs_default(vi_pipe, (isp_reg_cfg *)reg_cfg, actual_zone_row, actual_zone_col);
    awb_param.awb_zone_row = (td_u8)(actual_zone_row);

    if (awb_param.stitch_attr.stitch_enable == 1) {
        awb_param.awb_zone_col = MIN2(0xFF, (actual_zone_col * awb_param.stitch_attr.stitch_pipe_num));
    } else {
        awb_param.awb_zone_col = actual_zone_col;
    }

    awb_param.awb_width = isp_ctx->block_attr.frame_rect.width;
    awb_param.awb_height = isp_ctx->block_attr.frame_rect.height;

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (isp_ctx->awb_lib_info.libs[i].used) {
            lib = &isp_ctx->awb_lib_info.libs[i];

            if (lib->awb_regsiter.awb_exp_func.pfn_awb_init != TD_NULL) {
                lib->awb_regsiter.awb_exp_func.pfn_awb_init(lib->alg_lib.id, &awb_param, &awb_init_result);
            }
        }
    }

    awb_cfg_init_regs(vi_pipe, &awb_init_result, (isp_reg_cfg *)reg_cfg);
    return TD_SUCCESS;
}
#ifdef CONFIG_OT_SNAP_SUPPORT
static td_void isp_awb_run_config_snap_ccm(const isp_lib_node *lib, td_u32 color_temp,
    const awb_ccm_config *ccm_conf_def)
{
    awb_ccm_config ccm_conf;

    if (lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl != TD_NULL) {
        lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl(lib->alg_lib.id, OT_ISP_AWB_CCM_CONFIG_GET,
                                                    (td_void *)ccm_conf_def);

        ccm_conf.awb_bypass_en = TD_TRUE;
        ccm_conf.manual_temp_en = TD_TRUE;
        ccm_conf.manual_temp_value = color_temp;
        ccm_conf.ccm_speed = 0xfff;

        ccm_conf.manual_sat_en = ccm_conf_def->manual_sat_en;
        ccm_conf.manual_sat_value = ccm_conf_def->manual_sat_value;
        lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl(lib->alg_lib.id, OT_ISP_AWB_CCM_CONFIG_SET, (td_void *)&ccm_conf);
    }
}
#endif
static td_void isp_awb_run_config_exp_info(isp_lib_info *awb_lib_info, isp_linkage *linkage, \
                                           ot_isp_awb_param *awb_param)
{
    td_u8 i;

    /* linkage with the iso of ae */
    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (awb_lib_info->libs[i].used) {
            isp_lib_node *lib = &awb_lib_info->libs[i];
            ot_isp_awb_exp_func *awb_exp_func = &lib->awb_regsiter.awb_exp_func;
            td_s32 id = lib->alg_lib.id;

            if (awb_exp_func->pfn_awb_ctrl != TD_NULL) {
                awb_exp_func->pfn_awb_ctrl(id, OT_ISP_AWB_ISO_SET, (td_void *)&linkage->iso);
                awb_exp_func->pfn_awb_ctrl(id, OT_ISP_AWB_INTTIME_SET, (td_void *)&linkage->int_time);
#ifdef CONFIG_OT_SNAP_SUPPORT
                awb_exp_func->pfn_awb_ctrl(id, OT_ISP_AWB_SNAP_MODE_SET, (td_void *)&linkage->snap_pipe_mode);
#endif
                awb_exp_func->pfn_awb_ctrl(id, OT_ISP_AWB_ZONE_ROW_SET, (td_void *)&awb_param->awb_zone_row);
                awb_exp_func->pfn_awb_ctrl(id, OT_ISP_AWB_ZONE_COL_SET, (td_void *)&awb_param->awb_zone_col);
            }
        }
    }
}
td_u16 stat_blc_dg(td_u16 stat_value, td_u32 offset, td_u32 dgain, td_u32 dgain_shift)
{
    if ((td_u32)stat_value <= offset) {
        return 0;
    } else {
        return MIN2(((((td_u32)stat_value - offset) * dgain) >> dgain_shift), 0xFFFF);
    }
}

static td_void isp_awb_stat_blc_dg(td_u8 vi_pipe, ot_isp_awb_info *awb_info, isp_reg_cfg *reg_cfg)
{
    td_u32 i;
    td_u8 dgain_en;
    td_u32 blc_r, blc_g, blc_b;
    td_u32 dgain, shft;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_bool wdr_en;

    dgain_en = ot_ext_system_isp_dgain_enable_read(vi_pipe);
    if (dgain_en == TD_FALSE) {
        return;
    }

    wdr_en = ot_ext_system_wdr_en_read(vi_pipe);
    if (wdr_en == TD_TRUE) {
        td_u8 stat_shift = 4; /* 4 (20bit to 16 bit, for ISP_BLACK_LEVEL_RIGHT_SHIFT_BIT_WDR = 6) */
        blc_r = (reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_R] >> stat_shift);
        blc_g = reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_GR];
        blc_g += reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_GB];
        blc_g = (blc_g >> (stat_shift + 1));
        blc_b = (reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_B] >> stat_shift);
    } else {
        td_u8 stat_shift = 2; /* 2 (14bit to 16 bit) */
        blc_r = (reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_R] << stat_shift);
        blc_g = reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_GR];
        blc_g += reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_GB];
        blc_g = (blc_g << (stat_shift - 1));
        blc_b = (reg_cfg->alg_reg_cfg[0].be_blc_cfg.dyna_blc.lsc_blc.blc[OT_ISP_CHN_B] << stat_shift);
    }

    isp_get_ctx(vi_pipe, isp_ctx);

    dgain = isp_ctx->linkage.isp_dgain;
    shft = isp_ctx->linkage.isp_dgain_shift;

    awb_info->awb_stat1->metering_awb_avg_r	= stat_blc_dg(awb_info->awb_stat1->metering_awb_avg_r, blc_r, dgain, shft);
    awb_info->awb_stat1->metering_awb_avg_g	= stat_blc_dg(awb_info->awb_stat1->metering_awb_avg_g, blc_g, dgain, shft);
    awb_info->awb_stat1->metering_awb_avg_b	= stat_blc_dg(awb_info->awb_stat1->metering_awb_avg_b, blc_b, dgain, shft);

    for (i = 0; i < OT_ISP_AWB_ZONE_NUM; i++) {
        awb_info->awb_stat2.zone_avg_r[i] = stat_blc_dg(awb_info->awb_stat2.zone_avg_r[i], blc_r, dgain, shft);
        awb_info->awb_stat2.zone_avg_g[i] = stat_blc_dg(awb_info->awb_stat2.zone_avg_g[i], blc_g, dgain, shft);
        awb_info->awb_stat2.zone_avg_b[i] = stat_blc_dg(awb_info->awb_stat2.zone_avg_b[i], blc_b, dgain, shft);
    }
}

static td_void isp_awb_get_lsc_lut_line(td_u8 vi_pipe, isp_awb_lsc_lut *lsc_lut, td_u32 line_index)
{
    td_u32 i;
    td_u16 alpha = lsc_lut->blend_ratio;
    const td_u16 beta = 0x100 - alpha;
    const td_u16 blend_shift = 0x8;
    const td_u16 blend_round = 1 << (blend_shift - 1);
    td_u16 index = lsc_lut->index;
    td_u16 *mesh_gain_r = &lsc_lut->mesh_lut_r[line_index][0];
    td_u16 *mesh_gain_g = &lsc_lut->mesh_lut_g[line_index][0];
    td_u16 *mesh_gain_b = &lsc_lut->mesh_lut_b[line_index][0];

    for (i = 0; i < OT_ISP_LSC_GRID_COL; i++) {
        td_u16 r_gain0, gr_gain0, gb_gain0, b_gain0, g_gain0;
        td_u16 r_gain1, gr_gain1, gb_gain1, b_gain1, g_gain1;

        r_gain0 = ot_ext_system_isp_mesh_shading_r_gain0_read(vi_pipe, index);
        gr_gain0 = ot_ext_system_isp_mesh_shading_gr_gain0_read(vi_pipe, index);
        gb_gain0 = ot_ext_system_isp_mesh_shading_gb_gain0_read(vi_pipe, index);
        b_gain0 = ot_ext_system_isp_mesh_shading_b_gain0_read(vi_pipe, index);

        r_gain1 = ot_ext_system_isp_mesh_shading_r_gain1_read(vi_pipe, index);
        gr_gain1 = ot_ext_system_isp_mesh_shading_gr_gain1_read(vi_pipe, index);
        gb_gain1 = ot_ext_system_isp_mesh_shading_gb_gain1_read(vi_pipe, index);
        b_gain1 = ot_ext_system_isp_mesh_shading_b_gain1_read(vi_pipe, index);

        g_gain0 = (gr_gain0 + gb_gain0 + 0x1) >> 1; /* rshift 1 for average */
        g_gain1 = (gr_gain1 + gb_gain1 + 0x1) >> 1; /* rshift 1 for average */

        mesh_gain_r[i] = (alpha * r_gain0 + beta * r_gain1 + blend_round) >> blend_shift;
        mesh_gain_g[i] = (alpha * g_gain0 + beta * g_gain1 + blend_round) >> blend_shift;
        mesh_gain_b[i] = (alpha * b_gain0 + beta * b_gain1 + blend_round) >> blend_shift;
        index++;
    }
    lsc_lut->index += OT_ISP_LSC_GRID_COL;

    for (i = 0; i < (OT_ISP_LSC_GRID_COL - 1); i++) {
        mesh_gain_r[i] = (mesh_gain_r[i] + mesh_gain_r[i + 1] + 0x1) >> 0x1; /* rshift 1 for average */
        mesh_gain_g[i] = (mesh_gain_g[i] + mesh_gain_g[i + 1] + 0x1) >> 0x1; /* rshift 1 for average */
        mesh_gain_b[i] = (mesh_gain_b[i] + mesh_gain_b[i + 1] + 0x1) >> 0x1; /* rshift 1 for average */
    }
}

#define awb_stat_lsc(a, b, shift) (td_u16)(((a) * (b) + (1 << ((shift) - 1))) >> (shift))

static td_void isp_awb_stat_lsc_process(td_u8 vi_pipe, isp_awb_lsc_lut *lsc_lut, ot_isp_awb_info *awb_info)
{
    td_u32 i, j, mul_shift, cur_line;
    td_u16 gain_shift = lsc_lut->gain_shift;
    const td_s16 mesh_gain_unit = (1 << gain_shift);
    const td_u16 mesh_gain_max = (1 << (gain_shift + 0x4)); /* 0x4 for 16 */
    td_s16 scale_gain_unit = 0;
    ot_isp_awb_stat_result *awb_stat_zone = &(awb_info->awb_stat2);
    td_u64 sum_zone_r = 0;
    td_u64 sum_zone_g = 0;
    td_u64 sum_zone_b = 0;
    td_u64 sum_zone_cnt = 0;

    if (lsc_lut->scale <= 3) { /* 0-3 mesh_scale */
        scale_gain_unit = (1 << (9 - lsc_lut->scale)); /* 9 for dec */
        mul_shift = (21 - gain_shift - lsc_lut->scale); /* 21 = 12 + 9 */
    } else {
        mul_shift = (26 - gain_shift - lsc_lut->scale); /* 26 = 12 + 9 + 5 */
    }

    lsc_lut->index = 0;
    cur_line = 0;
    isp_awb_get_lsc_lut_line(vi_pipe, lsc_lut, cur_line);

    for (i = 0; i < OT_ISP_AWB_ZONE_ORIG_ROW; i++) {
        isp_awb_get_lsc_lut_line(vi_pipe, lsc_lut, !cur_line);

        for (j = 0; j < OT_ISP_AWB_ZONE_ORIG_COLUMN; j++) {
            td_s32 gain_r = (lsc_lut->mesh_lut_r[cur_line][j] + lsc_lut->mesh_lut_r[!cur_line][j] + 0x1) / 0x2;
            td_s32 gain_g = (lsc_lut->mesh_lut_g[cur_line][j] + lsc_lut->mesh_lut_g[!cur_line][j] + 0x1) / 0x2;
            td_s32 gain_b = (lsc_lut->mesh_lut_b[cur_line][j] + lsc_lut->mesh_lut_b[!cur_line][j] + 0x1) / 0x2;
            td_u32 k = i * OT_ISP_AWB_ZONE_ORIG_COLUMN + j;

            gain_r = (td_s32)signed_right_shift((gain_r - scale_gain_unit) * lsc_lut->strength, mul_shift);
            gain_g = (td_s32)signed_right_shift((gain_g - scale_gain_unit) * lsc_lut->strength, mul_shift);
            gain_b = (td_s32)signed_right_shift((gain_b - scale_gain_unit) * lsc_lut->strength, mul_shift);

            gain_r = MIN2(MAX2(mesh_gain_unit + gain_r, 0x0), mesh_gain_max);
            gain_g = MIN2(MAX2(mesh_gain_unit + gain_g, 0x0), mesh_gain_max);
            gain_b = MIN2(MAX2(mesh_gain_unit + gain_b, 0x0), mesh_gain_max);

            awb_stat_zone->zone_avg_r[k] = awb_stat_lsc(awb_stat_zone->zone_avg_r[k], (td_u32)gain_r, gain_shift);
            awb_stat_zone->zone_avg_g[k] = awb_stat_lsc(awb_stat_zone->zone_avg_g[k], (td_u32)gain_g, gain_shift);
            awb_stat_zone->zone_avg_b[k] = awb_stat_lsc(awb_stat_zone->zone_avg_b[k], (td_u32)gain_b, gain_shift);

            sum_zone_r += ((td_u64)awb_stat_zone->zone_avg_r[k]) * awb_stat_zone->zone_count[k];
            sum_zone_g += ((td_u64)awb_stat_zone->zone_avg_g[k]) * awb_stat_zone->zone_count[k];
            sum_zone_b += ((td_u64)awb_stat_zone->zone_avg_b[k]) * awb_stat_zone->zone_count[k];
            sum_zone_cnt += (td_u64)awb_stat_zone->zone_count[k];
        }
        cur_line = !cur_line;
    }

    awb_info->awb_stat1->metering_awb_avg_r = (td_u16)(sum_zone_r / div_0_to_1(sum_zone_cnt));
    awb_info->awb_stat1->metering_awb_avg_g = (td_u16)(sum_zone_g / div_0_to_1(sum_zone_cnt));
    awb_info->awb_stat1->metering_awb_avg_b = (td_u16)(sum_zone_b / div_0_to_1(sum_zone_cnt));

    return;
}

static td_bool isp_awb_set_overlap_info(ot_vi_pipe vi_pipe, td_u8 stitch_pipe_num, ot_isp_awb_info *awb_info)
{
    td_u8 overlap = ot_ext_system_isp_pipe_diff_overlap_read(vi_pipe);
    td_u8 awb_zone_row = ot_ext_system_awb_vnum_read(vi_pipe);
    td_u8 awb_zone_col = ot_ext_system_awb_hnum_read(vi_pipe);
    awb_info->overlap_all_zone.stitch_pipe_num = stitch_pipe_num;

    for (int i = 0; i < OT_ISP_MAX_STITCH_NUM; i++) {
        awb_info->overlap_all_zone.left_zone_range[i].col_start = 0;
        awb_info->overlap_all_zone.left_zone_range[i].col_stop  = overlap - 1;
        awb_info->overlap_all_zone.left_zone_range[i].row_start = 0;
        awb_info->overlap_all_zone.left_zone_range[i].row_stop  = awb_zone_row - 1;

        awb_info->overlap_all_zone.right_zone_range[i].col_start = awb_zone_col - overlap;
        awb_info->overlap_all_zone.right_zone_range[i].col_stop  = awb_zone_col - 1;
        awb_info->overlap_all_zone.right_zone_range[i].row_start = 0;
        awb_info->overlap_all_zone.right_zone_range[i].row_stop  = awb_zone_row - 1;
    }
    return TD_FALSE;
}

static td_void isp_awb_run_config_stat_info(ot_vi_pipe vi_pipe, td_u32 frame_cnt, isp_reg_cfg *reg_cfg,
                                            isp_stat *stat_info, ot_isp_awb_info *awb_info)
{
    td_u32 i;
    isp_awb_stat_info *awb_stat = TD_NULL;
    td_u8 mesh_lsc_en;
    isp_awb_lsc_lut lsc_lut;

    awb_get_stat_info(vi_pipe, awb_stat);

    awb_info->awb_stat1 = &(awb_stat->awb_glob_stat);
    awb_info->awb_stat2.zone_avg_r = &(awb_stat->awb_zone_stat.metering_mem_array_avg_r[0]);
    awb_info->awb_stat2.zone_avg_g = &(awb_stat->awb_zone_stat.metering_mem_array_avg_g[0]);
    awb_info->awb_stat2.zone_avg_b = &(awb_stat->awb_zone_stat.metering_mem_array_avg_b[0]);
    awb_info->awb_stat2.zone_count = &(awb_stat->awb_zone_stat.metering_mem_array_count_all[0]);

    awb_info->frame_cnt = frame_cnt;
    awb_info->awb_gain_switch = OT_ISP_WDR_WBGAIN_IN_WB;
    awb_info->awb_stat_switch = ot_ext_system_awb_switch_read(vi_pipe);
    awb_info->wb_gain_in_sns = TD_FALSE;

    awb_info->awb_stat1->metering_awb_avg_r = stat_info->awb_stat1.metering_awb_avg_r;
    awb_info->awb_stat1->metering_awb_avg_g = stat_info->awb_stat1.metering_awb_avg_g;
    awb_info->awb_stat1->metering_awb_avg_b = stat_info->awb_stat1.metering_awb_avg_b;
    awb_info->awb_stat1->metering_awb_count_all = stat_info->awb_stat1.metering_awb_count_all;

    for (i = 0; i < OT_ISP_AWB_ZONE_NUM; i++) {
        awb_info->awb_stat2.zone_avg_r[i] = stat_info->awb_stat2.metering_mem_array_avg_r[i];
        awb_info->awb_stat2.zone_avg_g[i] = stat_info->awb_stat2.metering_mem_array_avg_g[i];
        awb_info->awb_stat2.zone_avg_b[i] = stat_info->awb_stat2.metering_mem_array_avg_b[i];
        awb_info->awb_stat2.zone_count[i] = stat_info->awb_stat2.metering_mem_array_count_all[i];
    }
    isp_awb_set_overlap_info(vi_pipe, 1, awb_info);

    if (awb_info->awb_stat_switch != OT_EXT_SYSTEM_AWB_SWITCH_AFTER_EXPANDER) {
        return;
    }

    isp_awb_stat_blc_dg(vi_pipe, awb_info, reg_cfg);

    mesh_lsc_en = ot_ext_system_isp_mesh_shading_enable_read(vi_pipe);
    lsc_lut.blend_ratio = ot_ext_system_isp_mesh_shading_blendratio_read(vi_pipe);
    lsc_lut.strength = ot_ext_system_isp_mesh_shading_mesh_strength_read(vi_pipe);
    lsc_lut.scale = ot_ext_system_isp_mesh_shading_mesh_scale_read(vi_pipe);

    if ((mesh_lsc_en == TD_FALSE) || (lsc_lut.strength == 0)) {
        return;
    }

    lsc_lut.gain_shift = 0x8; /* 8 bit dec */
    isp_awb_stat_lsc_process(vi_pipe, &lsc_lut, awb_info);

    return;
}

static td_void isp_awb_run_res_2_linkage(ot_isp_awb_result *awb_result, isp_linkage *linkage,
                                         ot_isp_attach_info *attach_info, td_u8 sns_wdr_mode)
{
    td_u8 i, j;

    linkage->color_temp = awb_result->color_temp;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        linkage->white_balance_gain[i] = awb_result->white_balance_gain[i];
    }
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        linkage->ccm[i] = awb_result->color_matrix[i];
    }

    for (i = ISP_SYNC_ISO_BUF_MAX - 1; i >= 1; i--) {
        linkage->sync_color_temp[i] = linkage->sync_color_temp[i - 1];
        for (j = 0; j < OT_ISP_CCM_MATRIX_SIZE; j++) {
            linkage->sync_ccm[i][j] = linkage->sync_ccm[i - 1][j];
        }
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            linkage->sync_wb_gain[i][j] = linkage->sync_wb_gain[i - 1][j];
        }
    }

    linkage->sync_color_temp[0] = awb_result->color_temp;
    for (j = 0; j < OT_ISP_CCM_MATRIX_SIZE; j++) {
        linkage->sync_ccm[0][j] = awb_result->color_matrix[j];
    }
    for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
        linkage->sync_wb_gain[0][j] = awb_result->white_balance_gain[j];
    }

    attach_info->iso = linkage->iso;
    attach_info->sns_wdr_mode = sns_wdr_mode;
    attach_info->isp_hdr.color_temp = linkage->color_temp;
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        attach_info->isp_hdr.ccm[i] = linkage->ccm[i];
    }
}

static td_void isp_awb_run_calc_ccm(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result)
{
    td_u8 i;
    td_u16 ccm_elem;
    td_s16 calc_ccm[OT_ISP_CCM_MATRIX_SIZE] = {0};
    td_s16 diff_ccm[OT_ISP_CCM_MATRIX_SIZE] = {0};
    td_s16 res_ccm[OT_ISP_CCM_MATRIX_SIZE] = {0};

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        calc_ccm[i] = direct_to_complement_u16(awb_result->color_matrix[i]);
        ccm_elem = ot_ext_system_isp_pipe_diff_ccm_read(vi_pipe, i);
        diff_ccm[i] = direct_to_complement_u16(ccm_elem);
    }

    awb_matrix_multiply(calc_ccm, diff_ccm, res_ccm, 0x3);

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        awb_result->color_matrix[i] = complement_to_direct_u16(res_ccm[i]);
    }
    return;
}

static td_void isp_awb_run_apply_diff_manual(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result)
{
    td_u8 i;
    td_u32 diff_gain;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        diff_gain = ot_ext_system_isp_pipe_diff_gain_read(vi_pipe, i);
        awb_result->white_balance_gain[i] = isp_awb_rshift8(awb_result->white_balance_gain[i] * diff_gain);
    }

    isp_awb_run_calc_ccm(vi_pipe, awb_result);
}

static td_void isp_awb_run_apply_diff_auto(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result)
{
    td_u8 i;
    td_u32 diff_gain = 0x1000;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        td_u32 left_overlap_result = awb_result->overlap_result.left_overlap_zone_awb[1][i];
        td_u32 right_overlap_result = awb_result->overlap_result.right_overlap_zone_awb[0][i];

        if ((i == 0) || (i == 0x3)) {
            if (left_overlap_result == 0x1000 && right_overlap_result == 0x1000) {
                diff_gain = ot_ext_system_isp_pipe_diff_gain_read(vi_pipe, i) << 0x4;
            } else if (left_overlap_result != 0x1000 && right_overlap_result != 0x1000) {
                diff_gain = (left_overlap_result << 0xC) / div_0_to_1(right_overlap_result);
            }
        } else {
            diff_gain = 0x1000;
        }
        awb_result->white_balance_gain[i] = isp_awb_rshift8(awb_result->white_balance_gain[i] * diff_gain) >> 0x4;
        if ((i == 0) || (i == 0x3)) {
            ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, i, (diff_gain + 0x8) >> 0x4);
        }
    }

    isp_awb_run_calc_ccm(vi_pipe, awb_result);
}

static td_void isp_awb_run_config_pq_ai_gain(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, isp_reg_cfg *reg_cfg)
{
    td_u8 i, j;
    td_u32 wb_gain[OT_ISP_BAYER_CHN_NUM];
    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        wb_gain[i] = isp_ctx->linkage.white_balance_gain[i];
        wb_gain[i] = isp_awb_rshift8(wb_gain[i]);
        wb_gain[i] = MIN2(wb_gain[i], ISP_AWB_GAIN_MAX_THR);
    }

    for (j = 0; j < reg_cfg->cfg_num; j++) {
        awb_reg_dyn_cfg = &reg_cfg->alg_reg_cfg[j].awb_reg_cfg.awb_reg_dyn_cfg;
        awb_reg_dyn_cfg->be_wb_work_en = ot_ext_system_awb_gain_enable_read(vi_pipe);
        awb_reg_dyn_cfg->be_cc_en      = ot_ext_system_cc_enable_read(vi_pipe);
        awb_reg_dyn_cfg->be_cc_b_gain  = ot_ext_system_cc_colortone_bgain_read(vi_pipe);
        awb_reg_dyn_cfg->be_cc_g_gain  = ot_ext_system_cc_colortone_ggain_read(vi_pipe);
        awb_reg_dyn_cfg->be_cc_r_gain  = ot_ext_system_cc_colortone_rgain_read(vi_pipe);
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            awb_reg_dyn_cfg->be_white_balance_gain[i] = wb_gain[i];
        }
    }
}

static td_bool isp_awb_run_return_check(ot_vi_pipe vi_pipe, td_void *reg_cfg, const awb_ccm_config *ccm_conf_def)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
#ifdef CONFIG_OT_SNAP_SUPPORT
    isp_lib_node *lib = TD_NULL;
#endif

    ot_unused(ccm_conf_def);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_TRUE;
    }

    if ((isp_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == isp_ctx->ai_info.ai_pipe_id)) {
        isp_awb_run_config_pq_ai_gain(vi_pipe, isp_ctx, (isp_reg_cfg *)reg_cfg);
        return TD_TRUE;
    }

    if ((isp_ctx->linkage.snap_state != TD_TRUE) && (isp_ctx->linkage.stat_ready == TD_FALSE)) {
        return TD_TRUE;
    }

#ifdef CONFIG_OT_SNAP_SUPPORT
    if ((isp_ctx->linkage.snap_state == TD_TRUE) && (isp_ctx->linkage.load_ccm == TD_TRUE)) {
        return TD_TRUE;
    }

    lib = &isp_ctx->awb_lib_info.libs[isp_ctx->awb_lib_info.active_lib];
    if ((isp_ctx->linkage.snap_state == TD_TRUE) && (isp_ctx->linkage.load_ccm != TD_TRUE)) {
        isp_awb_run_config_snap_ccm(lib, isp_ctx->linkage.color_temp, ccm_conf_def);
    }

    if (is_online_mode(isp_ctx->block_attr.running_mode) &&
        (isp_ctx->linkage.snap_pipe_mode == ISP_SNAP_PICTURE)) {
        return TD_TRUE;
    }
#endif

    return TD_FALSE;
}
static td_void isp_awb_run_set_pipe_diff_gain(ot_vi_pipe vi_pipe)
{
    ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, OT_ISP_CHN_R, 0x100);
    ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, OT_ISP_CHN_GR, 0x100);
    ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, OT_ISP_CHN_GB, 0x100);
    ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, OT_ISP_CHN_B, 0x100);
}

static td_s32 isp_awb_run_stitch(ot_vi_pipe vi_pipe, isp_stitch_stat *stitch_stat, ot_isp_awb_info *awb_info)
{
    td_u8 i;
    td_s32 ret = TD_SUCCESS;
    ot_vi_pipe stitch_pipe;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_usr_ctx *isp_ctx_stitch_pipe = TD_NULL;
    isp_lib_node *lib = TD_NULL;
    ot_isp_awb_result awb_result = {{0}, {0}, 0, 0, {0}};
    isp_reg_cfg_attr *stitch_reg_cfg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    td_s8 stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];
    if (vi_pipe != stitch_main_pipe) {
        return TD_SUCCESS;
    }
    lib = &isp_ctx->awb_lib_info.libs[isp_ctx->awb_lib_info.active_lib];
    awb_info->awb_stat2.zone_avg_r = stitch_stat->awb_stat2.metering_mem_array_avg_r;
    awb_info->awb_stat2.zone_avg_g = stitch_stat->awb_stat2.metering_mem_array_avg_g;
    awb_info->awb_stat2.zone_avg_b = stitch_stat->awb_stat2.metering_mem_array_avg_b;
    awb_info->awb_stat2.zone_count = stitch_stat->awb_stat2.metering_mem_array_count_all;

    isp_awb_set_overlap_info(vi_pipe, isp_ctx->stitch_attr.stitch_pipe_num, awb_info);
    if (lib->awb_regsiter.awb_exp_func.pfn_awb_run != TD_NULL) {
        ret = lib->awb_regsiter.awb_exp_func.pfn_awb_run(lib->alg_lib.id, awb_info, &awb_result, 0);
        if (ret != TD_SUCCESS) {
            isp_err_trace("WARNING!! ISP[%d] run awb lib err!\n", vi_pipe);
        }
        awb_result_check(&awb_result);
    }

    for (i = 0; i < isp_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = isp_ctx->stitch_attr.stitch_bind_id[i];
        isp_get_ctx(stitch_pipe, isp_ctx_stitch_pipe);
        isp_regcfg_get_ctx(stitch_pipe, stitch_reg_cfg);
        isp_check_pointer_return(stitch_reg_cfg);

        (td_void)memcpy_s(&isp_ctx_stitch_pipe->awb_result, sizeof(ot_isp_awb_result),
                          &awb_result, sizeof(ot_isp_awb_result));
        isp_awb_run_res_2_linkage(&isp_ctx_stitch_pipe->awb_result, &isp_ctx_stitch_pipe->linkage,
                                  isp_ctx_stitch_pipe->attach_info_ctrl.attach_info, isp_ctx_stitch_pipe->sns_wdr_mode);

        if (ot_ext_system_isp_pipe_diff_type_read(stitch_main_pipe) == 0) {
            if (stitch_pipe != stitch_main_pipe) {
                isp_awb_run_apply_diff_auto(stitch_pipe, &isp_ctx_stitch_pipe->awb_result);
            } else {
                isp_awb_run_set_pipe_diff_gain(stitch_pipe);
                isp_awb_run_apply_diff_manual(stitch_pipe, &isp_ctx_stitch_pipe->awb_result);
            }
        } else {
            isp_awb_run_apply_diff_manual(stitch_pipe, &isp_ctx_stitch_pipe->awb_result);
        }

        awb_cfg_regs(stitch_pipe, &isp_ctx_stitch_pipe->awb_result, &stitch_reg_cfg->reg_cfg);
    }

    return ret;
}

static td_s32 awb_get_frame_info(ot_vi_pipe vi_pipe, ot_isp_awb_result *awb_result)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_frame_info *isp_frame = TD_NULL;
    td_u32 i;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_frame = &isp_ctx->frame_info;

    if (isp_frame == TD_NULL) {
        return TD_FAILURE;
    }

    for (i = 0; i < OT_ISP_WB_GAIN_NUM; i++) {
        isp_frame->wb_gain[i] = isp_awb_rshift8(awb_result->white_balance_gain[i]);
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        isp_frame->ccm[i] = awb_result->color_matrix[i];
    }

    return TD_SUCCESS;
}

static td_s32 isp_awb_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_s32 ret;
    ot_isp_awb_param awb_param;
    ot_isp_awb_info awb_info = {0};
    ot_isp_awb_result awb_result = {{0}, {0}, 0, 0, {0}};
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;
    awb_ccm_config ccm_conf_def = {0};

    ot_unused(rsv);
    if (isp_awb_run_return_check(vi_pipe, reg_cfg, &ccm_conf_def) == TD_TRUE) {
        return TD_SUCCESS;
    }

    isp_get_ctx(vi_pipe, isp_ctx);

    lib = &isp_ctx->awb_lib_info.libs[isp_ctx->awb_lib_info.active_lib];

    awb_res_read_ext_regs(vi_pipe, (isp_reg_cfg *)reg_cfg);

    awb_param.awb_zone_row = ot_ext_system_awb_vnum_read(vi_pipe);
    awb_param.awb_zone_col = ot_ext_system_awb_hnum_read(vi_pipe);
    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        awb_param.awb_zone_col = MIN2(0xFF, (awb_param.awb_zone_col * isp_ctx->stitch_attr.stitch_pipe_num));
    }

    /* linkage with the iso of ae */
    isp_awb_run_config_exp_info(&isp_ctx->awb_lib_info, &isp_ctx->linkage, &awb_param);

    /* get statistics */
    isp_awb_run_config_stat_info(vi_pipe, isp_ctx->frame_cnt, (isp_reg_cfg *)reg_cfg,
        (isp_stat *)stat_info, &awb_info);

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        isp_awb_run_stitch(vi_pipe, &((isp_stat *)stat_info)->stitch_stat, &awb_info);
    } else {
        if (lib->awb_regsiter.awb_exp_func.pfn_awb_run != TD_NULL) {
            ret = lib->awb_regsiter.awb_exp_func.pfn_awb_run(lib->alg_lib.id, &awb_info, &awb_result, 0);
            if (ret != TD_SUCCESS) {
                isp_err_trace("WARNING!! ISP[%d] run awb lib err!\n", vi_pipe);
            }
            awb_result_check(&awb_result);
            (td_void)memcpy_s(&isp_ctx->awb_result, sizeof(ot_isp_awb_result), &awb_result, sizeof(ot_isp_awb_result));
        }
        isp_awb_run_res_2_linkage(&awb_result, &isp_ctx->linkage, isp_ctx->attach_info_ctrl.attach_info,
                                  isp_ctx->sns_wdr_mode);
        awb_cfg_regs(vi_pipe, &awb_result, (isp_reg_cfg *)reg_cfg);
    }

    if (isp_ctx->linkage.snap_state == TD_TRUE) {
        if (lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl != TD_NULL) {
            lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl(lib->alg_lib.id,
                                                        OT_ISP_AWB_CCM_CONFIG_SET, (td_void *)&ccm_conf_def);
        }
    }

    ret = awb_get_frame_info(vi_pipe, &awb_result);
    if (ret != TD_SUCCESS) {
        isp_err_trace("WARNING!! ISP[%d] awb_get_frame_info err!\n", vi_pipe);
    }

    return ret;
}

static td_s32 isp_awb_proc_write(ot_isp_ctrl_proc_write *proc, isp_usr_ctx *isp_ctx)
{
    ot_isp_ctrl_proc_write proc_tmp;

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    if (proc->write_len > 0) {
        proc->proc_buff[proc->write_len - 1] = '\n';
    }

    proc->proc_buff = &proc->proc_buff[proc->write_len];
    proc->buff_len  = proc->buff_len - proc->write_len;
    proc->write_len = 0;

    (td_void)memcpy_s(&proc_tmp, sizeof(ot_isp_ctrl_proc_write), proc, sizeof(ot_isp_ctrl_proc_write));

    isp_proc_print_title(&proc_tmp, &proc->write_len, "isp awb info");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%8s"     "%8s"    "%8s"    "%8s"    "%8s\n",
                    "gain0",  "gain1", "gain2", "gain3", "cotemp");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "   0x%3x" "   0x%3x" "   0x%3x" "   0x%3x" "%8u\n\n",
                    (isp_ctx->awb_result.white_balance_gain[0] + 0x80) >> ISP_AWB_RIGHT_SHIFT,
                    (isp_ctx->awb_result.white_balance_gain[1] + 0x80) >> ISP_AWB_RIGHT_SHIFT,
                    (isp_ctx->awb_result.white_balance_gain[2] + 0x80) >> ISP_AWB_RIGHT_SHIFT, /* 2 */
                    (isp_ctx->awb_result.white_balance_gain[3] + 0x80) >> ISP_AWB_RIGHT_SHIFT, /* 3 */
                    isp_ctx->awb_result.color_temp);

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s" "%8s\n",
                    "color00", "color01", "color02", "color10", "color11", "color12", "color20", "color21", "color22");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%#8x" "%#8x" "%#8x" "%#8x" "%#8x" "%#8x" "%#8x" "%#8x" "%#8x\n\n",
                    isp_ctx->awb_result.color_matrix[0], isp_ctx->awb_result.color_matrix[1],
                    isp_ctx->awb_result.color_matrix[2], isp_ctx->awb_result.color_matrix[3], /* 2, 3 */
                    isp_ctx->awb_result.color_matrix[4], isp_ctx->awb_result.color_matrix[5], /* 4, 5 */
                    isp_ctx->awb_result.color_matrix[6], isp_ctx->awb_result.color_matrix[7], /* 6, 7 */
                    isp_ctx->awb_result.color_matrix[8]); /* 8 */
    proc->write_len += 1;

    return TD_SUCCESS;
}

static td_s32 isp_awb_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    td_s32 i;
    td_s32 ret = TD_FAILURE;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    lib = &isp_ctx->awb_lib_info.libs[isp_ctx->awb_lib_info.active_lib];

    if (cmd == OT_ISP_PROC_WRITE) {
        if (lib->used) {
            if (lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl != TD_NULL) {
                ret = lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl(lib->alg_lib.id, cmd, value);
            }
        }

        isp_awb_proc_write(value, isp_ctx);

        return ret;
    }

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (isp_ctx->awb_lib_info.libs[i].used) {
            lib = &isp_ctx->awb_lib_info.libs[i];
            if (lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl != TD_NULL) {
                ret = lib->awb_regsiter.awb_exp_func.pfn_awb_ctrl(lib->alg_lib.id, cmd, value);
            }
        }
    }

    if (cmd == OT_ISP_CHANGE_IMAGE_MODE_SET) {
        awb_image_mode_set(vi_pipe, &reg_cfg->reg_cfg);
    }

    if ((cmd == OT_ISP_WDR_MODE_SET) && (isp_ctx->block_attr.block_num != isp_ctx->block_attr.pre_block_num)) {
        awb_image_mode_set(vi_pipe, &reg_cfg->reg_cfg);
    }

    return ret;
}

static td_s32 isp_awb_exit(ot_vi_pipe vi_pipe)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lib_node *lib = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < OT_ISP_MAX_REGISTER_ALG_LIB_NUM; i++) {
        if (isp_ctx->awb_lib_info.libs[i].used) {
            lib = &isp_ctx->awb_lib_info.libs[i];

            if (lib->awb_regsiter.awb_exp_func.pfn_awb_exit != TD_NULL) {
                lib->awb_regsiter.awb_exp_func.pfn_awb_exit(lib->alg_lib.id);
            }
        }
    }

    awb_stat_info_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_awb(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_awb);

    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_AWB;
    algs->alg_func.pfn_alg_init = isp_awb_init;
    algs->alg_func.pfn_alg_run  = isp_awb_run;
    algs->alg_func.pfn_alg_ctrl = isp_awb_ctrl;
    algs->alg_func.pfn_alg_exit = isp_awb_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}

