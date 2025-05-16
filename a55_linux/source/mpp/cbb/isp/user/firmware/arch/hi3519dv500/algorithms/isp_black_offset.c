/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_proc.h"
#include "isp_param_check.h"
#include "isp_ext_reg_access.h"
#include "securec.h"

#define BLACK_OFFSET_BIT_WIDTH 14
#define EXPANDER_X_BIT_WIDTH   8
#define EXPANDER_Y_BIT_WIDTH   20
#define INIT_AGAIN            1024
#define AGAIN_OFFSET_THR       5
#define AGAIN_CHANGE_THR       10

#define DYNABLC_ISO_NUM 16
#define checkodd(x) (((x) % 2 == 1) ? ((x) - 1) : (x))

static const td_u32 dynablc_iso_cali[DYNABLC_ISO_NUM] = {
    100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800
};

static const td_u32 dynablc_ag_cali[DYNABLC_ISO_NUM] = {
    1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 524288, 524288, 524288, 524288, 524288, 524288
};

typedef struct {
    td_bool enable;
    td_s16  offset;
    td_s16  ag_offset;
    td_u32  pre_again;
    td_u32  pre_pre_again;
    ot_isp_black_level_dynamic_attr mpi_cfg;
    vi_blc_crop_info                bas_crop_attr;
    ot_isp_ob_stats_update_pos      ob_stats_update_pos;
}isp_dyna_blc_ctx;

typedef struct {
    td_bool init;
    td_bool pre_defect_pixel;
    td_bool expander_update;
    td_bool user_black_level_en;
    td_u8   black_level_change;
    td_u8   merge_frame;
    td_u16  sns_black_level[OT_ISP_WDR_MAX_FRAME_NUM][OT_ISP_BAYER_CHN_NUM];
    td_u16  user_black_level[OT_ISP_WDR_MAX_FRAME_NUM][OT_ISP_BAYER_CHN_NUM];
    td_u16  isp_black_level[OT_ISP_WDR_MAX_FRAME_NUM][OT_ISP_BAYER_CHN_NUM];
    td_u16  actual_black_level[OT_ISP_WDR_MAX_FRAME_NUM][OT_ISP_BAYER_CHN_NUM];
    isp_blc_actual_info  actual;
    isp_blc_process_sel blc_mode;
    ot_isp_black_level_mode op_mode;
    ot_isp_black_level_manual_attr  manual_attr;
    isp_dyna_blc_ctx     dyna_blc;
    ot_isp_expander_attr expander_attr;
} isp_blacklevel_ctx;

typedef struct {
    td_u8 shift;
    td_u16 x0;
    td_u16 x1;
    td_s32 y0;
    td_s32 y1;
} blc_linear_interp_cfg;

isp_blacklevel_ctx g_black_level_ctx[OT_ISP_MAX_PIPE_NUM] = { { 0 } };
#define blacklevel_get_ctx(pipe, blc_ctx) blc_ctx = &g_black_level_ctx[pipe]

static td_void dynablc_static_regs_initialize(ot_vi_pipe vi_pipe, isp_dynamicblc_static_cfg *static_reg_cfg)
{
    static_reg_cfg->dynablc_raw_array = ot_ext_system_rggb_cfg_read(vi_pipe);
    static_reg_cfg->dynablc_dpstat_en = TD_TRUE;

    static_reg_cfg->static_resh = TD_TRUE;
}

static td_void dynablc_usr_regs_initialize(isp_dynamicblc_usr_cfg *usr_reg_cfg,
    const ot_isp_black_level_dynamic_attr *dynamic_ctx)
{
    usr_reg_cfg->dynablc_pattern    = dynamic_ctx->pattern;
    usr_reg_cfg->dynablc_start_ypos = dynamic_ctx->ob_area.y;
    usr_reg_cfg->dynablc_start_xpos = dynamic_ctx->ob_area.x;
    usr_reg_cfg->dynablc_end_ypos   = dynamic_ctx->ob_area.y + dynamic_ctx->ob_area.height - 1;
    usr_reg_cfg->dynablc_end_xpos   = dynamic_ctx->ob_area.x + dynamic_ctx->ob_area.width - 1;
    usr_reg_cfg->dynablc_low_threshold = dynamic_ctx->low_threshold;
    usr_reg_cfg->dynablc_high_threshold = dynamic_ctx->high_threshold;
    usr_reg_cfg->bfilter_en         = TD_FALSE;
    usr_reg_cfg->dynablc_tar0       = dynamic_ctx->filter_strength;
    usr_reg_cfg->dynablc_tar1       = dynamic_ctx->tolerance;

    usr_reg_cfg->usr_resh = TD_TRUE;
}

static td_void isp_dynamic_blc_update_sync_cfg(const isp_blacklevel_ctx *blc_ctx, isp_dynamic_blc_sync_cfg *sync_cfg)
{
    sync_cfg->user_black_level_en = blc_ctx->user_black_level_en;
    sync_cfg->blc_mode            = blc_ctx->blc_mode;
    sync_cfg->black_level_mode    = blc_ctx->op_mode;
    sync_cfg->tolerance           = blc_ctx->dyna_blc.mpi_cfg.tolerance;
    sync_cfg->separate_en         = blc_ctx->dyna_blc.mpi_cfg.separate_en;
    sync_cfg->offset              = blc_ctx->dyna_blc.offset;
    sync_cfg->filter_strength     = blc_ctx->dyna_blc.mpi_cfg.filter_strength;
    sync_cfg->ag_offset           = blc_ctx->dyna_blc.ag_offset;
    (td_void)memcpy_s(sync_cfg->user_black_level, OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16),
                      blc_ctx->user_black_level, OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16));
    sync_cfg->filter_thr          = blc_ctx->dyna_blc.mpi_cfg.filter_thr;
}

static td_void dynablc_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_blacklevel_ctx *blc_ctx)
{
    td_u8 i;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].dynamic_blc_reg_cfg.dynablc_enable = blc_ctx->dyna_blc.enable;
        dynablc_static_regs_initialize(vi_pipe, &reg_cfg->alg_reg_cfg[i].dynamic_blc_reg_cfg.static_reg_cfg);
        dynablc_usr_regs_initialize(&reg_cfg->alg_reg_cfg[i].dynamic_blc_reg_cfg.usr_reg_cfg,
                                    &blc_ctx->dyna_blc.mpi_cfg);
        isp_dynamic_blc_update_sync_cfg(blc_ctx, &reg_cfg->alg_reg_cfg[i].dynamic_blc_reg_cfg.sync_cfg);
    }

    reg_cfg->cfg_key.bit1_dynablc_cfg = 1;
}

static td_void dynablc_ext_regs_initialize(ot_vi_pipe vi_pipe, const isp_blacklevel_ctx *blc_ctx)
{
    isp_black_level_dynamic_attr_write(vi_pipe, &blc_ctx->dyna_blc.mpi_cfg);

    ot_ext_system_isp_dynamic_blc_attr_updata_write(vi_pipe, TD_FALSE);
}
static td_u8 get_iso_blc_index(td_u32 iso)
{
    td_u8 index;
    for (index = 0; index < DYNABLC_ISO_NUM - 1; index++) {
        if (iso <= dynablc_iso_cali[index]) {
            break;
        }
    }
    return index;
}

static td_void dynablc_offset_inter(td_u32 iso, isp_blacklevel_ctx *blc_ctx)
{
    td_u8 iso_index_up, iso_index_low;
    td_u32 iso1, iso2;
    td_s16 offset1, offset2;
    iso_index_up = get_iso_blc_index(iso);

    iso_index_low = MAX2((td_s8)iso_index_up - 1, 0);
    iso1          = dynablc_iso_cali[iso_index_low];
    iso2          = dynablc_iso_cali[iso_index_up];
    offset1       = blc_ctx->dyna_blc.mpi_cfg.offset[iso_index_low];
    offset2       = blc_ctx->dyna_blc.mpi_cfg.offset[iso_index_up];

    blc_ctx->dyna_blc.offset = (td_s16)linear_inter(iso, iso1, offset1, iso2, offset2);
}

static td_u8 get_again_blc_index(td_u32 again)
{
    td_u8 index;
    for (index = 0; index < DYNABLC_ISO_NUM - 1; index++) {
        if (again <= dynablc_ag_cali[index]) {
            break;
        }
    }
    return index;
}

static td_u16 dynablc_calibration_black_level_get(td_u32 again, const isp_blacklevel_ctx *blc_ctx)
{
    td_u8 ag_index_up, ag_index_low;
    td_u16 ag1, ag2;
    td_s16 offset1, offset2;
    td_u16 cali_blc;

    ag_index_up  = get_again_blc_index(again);
    ag_index_low = MAX2((td_s8)ag_index_up - 1, 0);
    ag1          = dynablc_ag_cali[ag_index_low];
    ag2          = dynablc_ag_cali[ag_index_up];
    offset1      = blc_ctx->dyna_blc.mpi_cfg.calibration_black_level[ag_index_low];
    offset2      = blc_ctx->dyna_blc.mpi_cfg.calibration_black_level[ag_index_up];
    cali_blc = (td_u16)linear_inter(again, ag1, offset1, ag2, offset2);

    return cali_blc;
}

static td_void dynablc_ag_offset_inter(td_u32 again, isp_blacklevel_ctx *blc_ctx)
{
    td_s16 ag_change;
    td_u16 pre_calib_blc;
    td_u32 last_again;
    td_u16 calib_black_level;

    if (blc_ctx->dyna_blc.ob_stats_update_pos == OT_ISP_UPDATE_OB_STATS_FE_FRAME_START) {
        last_again = blc_ctx->dyna_blc.pre_pre_again;
    } else {
        last_again = blc_ctx->dyna_blc.pre_again;
    }

    ag_change = last_again - again;
    if (ABS(ag_change) < AGAIN_CHANGE_THR) {
        blc_ctx->dyna_blc.ag_offset = 0;
        return;
    }

    calib_black_level = dynablc_calibration_black_level_get(again, blc_ctx);
    pre_calib_blc     = dynablc_calibration_black_level_get(last_again, blc_ctx);

    blc_ctx->dyna_blc.ag_offset = calib_black_level - pre_calib_blc;

    if (ABS(blc_ctx->dyna_blc.ag_offset) < AGAIN_OFFSET_THR) {
        blc_ctx->dyna_blc.ag_offset = 0;
    }
}
static td_void blc_get_merge_frame(td_u8 wdr_mode, isp_blacklevel_ctx *blc_ctx)
{
    if (is_linear_mode(wdr_mode) || is_built_in_wdr_mode(wdr_mode)) {
        blc_ctx->merge_frame = 1;
    } else if (is_2to1_wdr_mode(wdr_mode)) {
        blc_ctx->merge_frame = 2;     /* 2to1 wdr */
    } else if (is_3to1_wdr_mode(wdr_mode)) {
        blc_ctx->merge_frame = 3;    /* 3to1 wdr */
    } else {
        blc_ctx->merge_frame = 1;
    }
}

static td_void isp_update_sns_blc_fix(const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx)
{
    if (isp_ctx->linkage.blc_fix.enable == TD_TRUE) {
        blc_ctx->blc_mode = ISP_BLC_NORMAL;
    }
}

static td_void get_sns_black_level_value(const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx,
    const ot_isp_cmos_black_level *sns_black_level)
{
    td_u8 i, j;

    if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_MANUAL) {
        for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
            for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
                blc_ctx->sns_black_level[i][j] = blc_ctx->manual_attr.black_level[i][j];
            }
        }
    } else {
        for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
            for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
                blc_ctx->sns_black_level[i][j] = sns_black_level->auto_attr.black_level[i][j];
            }
        }
    }
    isp_update_sns_blc_fix(isp_ctx, blc_ctx);
}

static td_void get_isp_black_level_value(isp_blacklevel_ctx *blc_ctx)
{
    size_t blc_size = OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);
    if (blc_ctx->user_black_level_en == TD_TRUE) {
        (td_void)memcpy_s(blc_ctx->isp_black_level, blc_size, blc_ctx->user_black_level, blc_size);
    } else {
        (td_void)memcpy_s(blc_ctx->isp_black_level, blc_size, blc_ctx->sns_black_level, blc_size);
    }
}

static td_s32 isp_blc_check_cmos_param(ot_vi_pipe vi_pipe, const ot_isp_cmos_black_level *sns_black_level)
{
    td_s32 ret;
    isp_check_bool_return(sns_black_level->auto_attr.update);

    if (sns_black_level->black_level_mode >= OT_ISP_BLACK_LEVEL_MODE_BUTT) {
        isp_err_trace("Err cmos black_level_mode %d!\n", sns_black_level->black_level_mode);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ret = isp_black_level_value_check("cmos manual", sns_black_level->manual_attr.black_level);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_black_level_value_check("cmos auto", sns_black_level->auto_attr.black_level);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_black_level_value_check("cmos user", sns_black_level->user_black_level);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_user_black_level_en_check(vi_pipe, sns_black_level->user_black_level_en);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_dynamic_blc_attr_check(vi_pipe, "cmos", &sns_black_level->dynamic_attr,
                                     sns_black_level->black_level_mode);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 bas_change_para(ot_size *ob_sns_size, ot_rect *ori_ob_area, td_float h_ratio, td_float w_ratio)
{
    ori_ob_area->width  = checkodd((td_u32)(ori_ob_area->width * w_ratio));
    ori_ob_area->height = checkodd((td_u32)(ori_ob_area->height * h_ratio));
    ori_ob_area->x = checkodd((td_u32)(ori_ob_area->x * w_ratio));
    ori_ob_area->y = checkodd((td_u32)(ori_ob_area->y * h_ratio));
    ob_sns_size->height = checkodd((td_u32)(ob_sns_size->height * h_ratio));
    ob_sns_size->width  = checkodd((td_u32)(ob_sns_size->width  * w_ratio));
    return TD_SUCCESS;
}

static td_s32 isp_dynablc_change_ob_area_part3(ot_size *ob_sns_size, ot_rect *ori_ob_area,
    const isp_blacklevel_ctx *blc_ctx)
{
    td_bool  dev_bas_en;
    td_s32   bas_h;
    td_s32   bas_w;
    td_bool  vi_crop_en;
    ot_rect  vi_crop_offset;
    td_float h_ratio;
    td_float w_ratio;
    bas_h    = blc_ctx->dyna_blc.bas_crop_attr.bas_size.height;
    bas_w    = blc_ctx->dyna_blc.bas_crop_attr.bas_size.width;
    h_ratio  = bas_h / div_0_to_1_float((td_float)ob_sns_size->height);
    w_ratio  = bas_w / div_0_to_1_float((td_float)ob_sns_size->width);
    dev_bas_en = blc_ctx->dyna_blc.bas_crop_attr.bas_en;
    (td_void)memcpy_s(&vi_crop_offset, sizeof(ot_rect),
                      &blc_ctx->dyna_blc.bas_crop_attr.pre_crop_info.rect, sizeof(ot_rect));
    vi_crop_en = blc_ctx->dyna_blc.bas_crop_attr.pre_crop_info.enable;
    if (dev_bas_en == 1) {
        bas_change_para(ob_sns_size, ori_ob_area, h_ratio, w_ratio);
        if (vi_crop_en == 1) {
            if (vi_crop_offset.y != 0) {
                isp_err_trace("Err vi_crop set error !\n");
                return TD_FAILURE;
            } else {
                ori_ob_area->x      = 0;
                ori_ob_area->y      = 0;
                ori_ob_area->width  = vi_crop_offset.width;
                ori_ob_area->height = ori_ob_area->height - vi_crop_offset.y;
            }
        }
    } else {
        if (vi_crop_en == 1) {
            if (vi_crop_offset.y != 0) {
                isp_err_trace("Err vi_crop set error !\n");
                return TD_FAILURE;
            } else {
                ori_ob_area->x      = 0;
                ori_ob_area->y      = 0;
                ori_ob_area->width  = vi_crop_offset.width;
                ori_ob_area->height = ori_ob_area->height - vi_crop_offset.y;
            }
        }
    }
    return TD_SUCCESS;
}

static td_s32 isp_dynablc_change_ob_area_part2(ot_size *ob_sns_size, ot_rect *ori_ob_area,
                                               const isp_blacklevel_ctx *blc_ctx)
{
    td_bool  dev_bas_en;
    td_s32   bas_h;
    td_s32   bas_w;
    td_bool  vi_crop_en;
    ot_rect  vi_crop_offset;
    td_float h_ratio;
    td_float w_ratio;
    bas_h    = blc_ctx->dyna_blc.bas_crop_attr.bas_size.height;
    bas_w    = blc_ctx->dyna_blc.bas_crop_attr.bas_size.width;
    h_ratio  = bas_h / div_0_to_1_float((td_float)ob_sns_size->height);
    w_ratio  = bas_w / div_0_to_1_float((td_float)ob_sns_size->width);
    dev_bas_en = blc_ctx->dyna_blc.bas_crop_attr.bas_en;
    (td_void)memcpy_s(&vi_crop_offset, sizeof(ot_rect),
                      &blc_ctx->dyna_blc.bas_crop_attr.pre_crop_info.rect, sizeof(ot_rect));
    vi_crop_en = blc_ctx->dyna_blc.bas_crop_attr.pre_crop_info.enable;
    if (dev_bas_en == 1) {
        bas_change_para(ob_sns_size, ori_ob_area, h_ratio, w_ratio);
        if (vi_crop_en == 1) {
            if ((vi_crop_offset.y + vi_crop_offset.height) != ob_sns_size->height) {
                isp_err_trace("Err vi_crop set error !\n");
                return TD_FAILURE;
            } else {
                ori_ob_area->x      = 0;
                ori_ob_area->width  = vi_crop_offset.width;
                ori_ob_area->height = ori_ob_area->height - ob_sns_size->height + vi_crop_offset.y +
                                      vi_crop_offset.height;
                ori_ob_area->y      = ob_sns_size->height - ori_ob_area->height - vi_crop_offset.y;
            }
        }
    } else {
        if (vi_crop_en == 1) {
            if ((vi_crop_offset.y + vi_crop_offset.height) != ob_sns_size->height) {
                isp_err_trace("Err vi_crop set error !\n");
                return TD_FAILURE;
            } else {
                ori_ob_area->x      = 0;
                ori_ob_area->width  = vi_crop_offset.width;
                ori_ob_area->height = ori_ob_area->height - ob_sns_size->height + vi_crop_offset.y +
                                      vi_crop_offset.height;
                ori_ob_area->y      = ob_sns_size->height - ori_ob_area->height - vi_crop_offset.y;
            }
        }
    }
    return TD_SUCCESS;
}

static td_s32 dynablc_input_para_check(ot_vi_pipe vi_pipe, ot_size *ob_sns_size, ot_rect *ori_ob_area,
    ot_rect *mipi_crop_offset, const ot_isp_cmos_black_level *sns_blc_ctx)
{
    (td_void)memcpy_s(ori_ob_area, sizeof(ot_rect), &sns_blc_ctx->dynamic_attr.ob_area, sizeof(ot_rect));

    mipi_crop_offset->x = ot_mipi_crop_pos_x_read(vi_pipe);
    mipi_crop_offset->y = ot_mipi_crop_pos_y_read(vi_pipe);
    mipi_crop_offset->width = ot_mipi_crop_width_read(vi_pipe);
    mipi_crop_offset->height = ot_mipi_crop_height_read(vi_pipe);
    ob_sns_size->height   = sns_blc_ctx->sns_with_ob_attr.ob_sns_size.height;
    ob_sns_size->width    = sns_blc_ctx->sns_with_ob_attr.ob_sns_size.width;

    if (ori_ob_area->x < 0 || ori_ob_area->x % 2 != 0 || /* 2 is num */
        (td_u32)ori_ob_area->x >= ob_sns_size->width) {
        isp_err_trace("Err ob_area x set error!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (ori_ob_area->y < 0 ||ori_ob_area->y % 2 != 0 || /* 2 is num */
        (td_u32)ori_ob_area->y >= ob_sns_size->height) {
        isp_err_trace("Err ob_area y set error!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((ori_ob_area->y + ori_ob_area->height) > ob_sns_size->height ||
        ori_ob_area->height % 2 != 0x0) { /* 2 is num */
        isp_err_trace("Err ob_area height set error!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((ori_ob_area->width + ori_ob_area->x) > ob_sns_size->width ||
        ori_ob_area->width % 2 != 0x0) { /* 2 is num */
        isp_err_trace("Err ob_area width set error!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 mipi_change_ob_sensor_flipen(ot_size *ob_sns_size, ot_rect *ori_ob_area,
                                           const ot_rect *mipi_crop_offset)
{
    ori_ob_area->y         = ob_sns_size->height - ori_ob_area->height - mipi_crop_offset->y;
    ori_ob_area->height    = mipi_crop_offset->height - ori_ob_area->y;
    ori_ob_area->width     = mipi_crop_offset->width;
    ob_sns_size->width  = mipi_crop_offset->width;
    ob_sns_size->height = mipi_crop_offset->height;
    return TD_SUCCESS;
}

static td_s32 mipi_change_ob_sensor_flipdisen(ot_size *ob_sns_size, ot_rect *ori_ob_area,
                                              const ot_rect *mipi_crop_offset)
{
    ori_ob_area->y         = 0;
    ori_ob_area->height    = ori_ob_area->height - mipi_crop_offset->y;
    ori_ob_area->width     = mipi_crop_offset->width;
    ob_sns_size->width  = mipi_crop_offset->width;
    ob_sns_size->height = mipi_crop_offset->height;
    return TD_SUCCESS;
}

static td_s32 isp_dynablc_change_ob_area_part1(ot_vi_pipe vi_pipe, const isp_blacklevel_ctx *blc_ctx,
    ot_isp_cmos_black_level *sns_blc_ctx)
{
    td_s32  ret;
    td_bool sensor_flip_en;
    td_bool mipi_crop_en;
    ot_rect ori_ob_area;
    ot_size ob_sns_size;
    ot_rect mipi_crop_offset;

    ret = dynablc_input_para_check(vi_pipe, &ob_sns_size, &ori_ob_area, &mipi_crop_offset, sns_blc_ctx);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Get Dev_Bas Pipe_Pre_Crop Attr Err\n");
        return ret;
    }

    sensor_flip_en = ot_sensor_flip_enable_read(vi_pipe);
    mipi_crop_en   = ot_mipi_crop_enable_read(vi_pipe);
    if (sensor_flip_en == 1) {
        ori_ob_area.y = ob_sns_size.height - ori_ob_area.height;
        if (mipi_crop_en == 1) {
            if ((mipi_crop_offset.height + mipi_crop_offset.y) != ob_sns_size.height) {
                isp_err_trace("Err mipi_crop set error !\n");
                return TD_FAILURE;
                } else {
                    mipi_change_ob_sensor_flipen(&ob_sns_size, &ori_ob_area, &mipi_crop_offset);
                    ret = isp_dynablc_change_ob_area_part2(&ob_sns_size, &ori_ob_area, blc_ctx);
                }
        } else {
            ret = isp_dynablc_change_ob_area_part2(&ob_sns_size, &ori_ob_area, blc_ctx);
        }
    } else {
        if (mipi_crop_en == 1) {
            if (mipi_crop_offset.y != 0) {
                isp_err_trace("Err mipi_crop set error !\n");
                return TD_FAILURE;
                } else {
                    mipi_change_ob_sensor_flipdisen(&ob_sns_size, &ori_ob_area, &mipi_crop_offset);
                    ret = isp_dynablc_change_ob_area_part3(&ob_sns_size, &ori_ob_area, blc_ctx);
                }
        } else {
            ret = isp_dynablc_change_ob_area_part3(&ob_sns_size, &ori_ob_area, blc_ctx);
        }
    }
    (td_void)memcpy_s(&sns_blc_ctx->dynamic_attr.ob_area, sizeof(ot_rect), &ori_ob_area, sizeof(ot_rect));
    return ret;
}

static td_s32 isp_dynablc_change_ob_area(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx,
    ot_isp_cmos_black_level *sns_blc_ctx)
{
    td_s32  ret;
    ot_isp_ctrl_param isp_ctrl_param;

    if (is_no_fe_pipe(vi_pipe)) {
        return TD_SUCCESS;
    }
    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_BAS_CROP_ATTR, &blc_ctx->dyna_blc.bas_crop_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Get Dev_Bas Pipe_Pre_Crop Attr Err\n");
        return ret;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_CTRL_PARAM, &isp_ctrl_param);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Get isp ctrl param failed\n");
        return ret;
    }

    blc_ctx->dyna_blc.ob_stats_update_pos = isp_ctrl_param.ob_stats_update_pos;

    if (sns_blc_ctx->black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        ret = isp_dynablc_change_ob_area_part1(vi_pipe, blc_ctx, sns_blc_ctx);
    }

    return ret;
}

static td_void isp_blc_ctx_cmos_initialize(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx,
    const ot_isp_cmos_black_level *cmos_blc, td_u8 wdr_mode)
{
    size_t blc_size;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    blc_size = OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);
    blc_ctx->op_mode = cmos_blc->black_level_mode;
    (td_void)memcpy_s(&blc_ctx->manual_attr, sizeof(ot_isp_black_level_manual_attr),
                      &cmos_blc->manual_attr, sizeof(ot_isp_black_level_manual_attr));
    if (is_fs_wdr_mode(wdr_mode) == TD_TRUE) {
        /* wdr only support normal, not support global, to prevent the case of different frame with different blc */
        blc_ctx->blc_mode = ISP_BLC_NORMAL;
    } else if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        // only ob region is upper than frame, the real_time mode is valid, otherwise can't effect within the frame
        blc_ctx->blc_mode =
            (blc_ctx->dyna_blc.mpi_cfg.ob_area.y + blc_ctx->dyna_blc.mpi_cfg.ob_area.height <=
            (td_u32)isp_ctx->sys_rect.y) ? ISP_BLC_REAL_TIME : ISP_BLC_GLOBAL;
    } else {
        blc_ctx->blc_mode = ISP_BLC_GLOBAL;
    }
    blc_ctx->user_black_level_en = cmos_blc->user_black_level_en;
    (td_void)memcpy_s(blc_ctx->user_black_level, blc_size, cmos_blc->user_black_level, blc_size);
    get_sns_black_level_value(isp_ctx, blc_ctx, cmos_blc);
    get_isp_black_level_value(blc_ctx);
    isp_update_sns_blc_fix(isp_ctx, blc_ctx);
}

static td_void isp_dynablc_ctx_initialize(isp_dyna_blc_ctx *dyna_blc, const ot_isp_cmos_black_level *cmos_blc)
{
    dyna_blc->enable = (cmos_blc->black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) ? TD_TRUE : TD_FALSE;
    (td_void)memcpy_s(&dyna_blc->mpi_cfg, sizeof(ot_isp_black_level_dynamic_attr),
                      &cmos_blc->dynamic_attr, sizeof(ot_isp_black_level_dynamic_attr));
    dyna_blc->offset = 0;
    dyna_blc->ag_offset = 0;
    dyna_blc->pre_again = INIT_AGAIN;
    dyna_blc->pre_pre_again = INIT_AGAIN;
}

static td_s32 blc_initialize(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx)
{
    td_u8  wdr_mode;
    td_s32 ret;
    ot_isp_cmos_black_level *sns_blc = TD_NULL;
    isp_usr_ctx             *isp_ctx = TD_NULL;

    isp_sensor_get_blc(vi_pipe, &sns_blc);
    isp_get_ctx(vi_pipe, isp_ctx);

    wdr_mode = isp_ctx->sns_wdr_mode;
    blc_ctx->blc_mode = ISP_BLC_GLOBAL;
    ret = isp_dynablc_change_ob_area(vi_pipe, blc_ctx, sns_blc);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_blc_check_cmos_param(vi_pipe, sns_blc);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_blc_ctx_cmos_initialize(vi_pipe, blc_ctx, sns_blc, wdr_mode);
    isp_dynablc_ctx_initialize(&blc_ctx->dyna_blc, sns_blc);

    isp_expander_attr_read(vi_pipe, &blc_ctx->expander_attr);

    blc_get_merge_frame(wdr_mode, blc_ctx);

    blc_ctx->pre_defect_pixel = TD_FALSE;

    return TD_SUCCESS;
}

static td_void blc_ext_regs_initialize(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx)
{
    ot_ext_system_black_level_mode_write(vi_pipe, blc_ctx->op_mode);
    isp_black_level_manual_attr_write(vi_pipe, &blc_ctx->manual_attr);
    ot_ext_system_isp_user_blc_en_write(vi_pipe, blc_ctx->user_black_level_en);
    isp_user_black_level_write(vi_pipe, blc_ctx->user_black_level);
    black_level_actual_value_write(vi_pipe, &blc_ctx->actual);
    ot_ext_system_black_level_change_write(vi_pipe, TD_FALSE);
}

static td_s32 blc_linear_interpol(td_u16 xm, const blc_linear_interp_cfg *intp_cfg)
{
    td_s32 ym;
    td_s64 y0 = intp_cfg->y0;
    td_s64 y1 = intp_cfg->y1;
    td_s64 tmp;

    if (xm <= (intp_cfg->x0 << intp_cfg->shift)) {
        return y0;
    }
    if (xm >= (intp_cfg->x1 << intp_cfg->shift)) {
        return y1;
    }

    tmp = (y1 - y0) * (xm - (intp_cfg->x0 << intp_cfg->shift)) / div_0_to_1(intp_cfg->x1 - intp_cfg->x0);
    ym = (td_s32)signed_right_shift(tmp, intp_cfg->shift) + y0;

    return ym;
}

static td_u32 get_index(td_u32 x, const td_u16 *x_lut, td_u16 point_num)
{
    td_u32 index;

    for (index = 0; index < point_num; index++) {
        if (x <= x_lut[index]) {
            break;
        }
    }

    return index;
}

static td_void get_built_in_expander_blc(const ot_isp_expander_attr *expander_attr,
                                         td_u16 (*sensor_blc)[OT_ISP_BAYER_CHN_NUM], td_u16 *expander_blc,
                                         td_u8 array_length)
{
    td_u16  i, j;
    const td_u8 x_shift = BLACK_OFFSET_BIT_WIDTH - EXPANDER_X_BIT_WIDTH;
    const td_u8 y_shift = EXPANDER_Y_BIT_WIDTH - BLACK_OFFSET_BIT_WIDTH;
    td_u16 idx_up, idx_low;
    td_u16 expander_point_num;
    td_u16 x[OT_ISP_EXPANDER_POINT_NUM_MAX + 1] = {0};
    td_u32 y[OT_ISP_EXPANDER_POINT_NUM_MAX + 1] = {0};
    blc_linear_interp_cfg  intp_cfg;

    expander_point_num = expander_attr->knee_point_num;
    for (i = 1; i < expander_point_num + 1; i++) {
        x[i] = expander_attr->knee_point_coord[i - 1].x;
        y[i] = expander_attr->knee_point_coord[i - 1].y;
    }

    for (j = 0; j < array_length; j++) {
        idx_up  = get_index(sensor_blc[0][j] >> x_shift, x, expander_point_num);
        idx_low = (td_u8)MAX2((td_s16)idx_up - 1, 0);

        /* update blc_linear_interp_cfg */
        intp_cfg.x0 = x[idx_low];
        intp_cfg.x1 = x[idx_up];
        intp_cfg.y0 = y[idx_low];
        intp_cfg.y1 = y[idx_up];
        intp_cfg.shift = x_shift;

        expander_blc[j] = ((td_u32)blc_linear_interpol(sensor_blc[0][j], &intp_cfg)) >> y_shift;
    }
}

static td_void update_actual_blc_normal(isp_blacklevel_ctx *blc_ctx)
{
    td_u8 i, j;
    for (i = 0; i < blc_ctx->merge_frame; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            blc_ctx->actual.sns_black_level[i][j] = blc_ctx->sns_black_level[i][j];
            blc_ctx->actual.isp_black_level[i][j] = blc_ctx->isp_black_level[i][j];
        }
    }

    for (i = blc_ctx->merge_frame; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            blc_ctx->actual.sns_black_level[i][j] = 0;
            blc_ctx->actual.isp_black_level[i][j] = 0;
        }
    }
}

static td_void be_blc_calc_bulit_in(isp_be_blc_dyna_cfg *dyna_blc, isp_blacklevel_ctx *blc_ctx)
{
    td_u8 i, j;
    td_u16 black_level[OT_ISP_BAYER_CHN_NUM] = { 0 }; /* 14 bits */

    if (blc_ctx->expander_attr.enable == TD_TRUE) {
        get_built_in_expander_blc(&blc_ctx->expander_attr, blc_ctx->isp_black_level, black_level, OT_ISP_BAYER_CHN_NUM);
    } else {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            black_level[i]    = blc_ctx->isp_black_level[0][i];
        }
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            dyna_blc->global_blc.global_blc0[i][j] = 0;
            dyna_blc->global_blc.global_blc1[i][j] = black_level[j];
            dyna_blc->global_blc.target_blc[i][j]  = black_level[j];
        }
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        blc_ctx->actual.isp_black_level[0][i] = black_level[i];
        blc_ctx->actual.sns_black_level[0][i] = blc_ctx->sns_black_level[0][i];
    }
    for (i = 1; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            blc_ctx->actual.isp_black_level[i][j] = 0;
            blc_ctx->actual.sns_black_level[i][j] = 0;
        }
    }
}

static td_void be_blc_calc_by_wdr_mode(td_u8 wdr_mode, isp_be_blc_dyna_cfg *dyna_blc, isp_blacklevel_ctx *blc_ctx)
{
    td_u8 i, j;

    if (is_built_in_wdr_mode(wdr_mode)) {
        be_blc_calc_bulit_in(dyna_blc, blc_ctx);
    } else {
        for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
            for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
                dyna_blc->global_blc.global_blc0[i][j] = blc_ctx->isp_black_level[i][j];
                dyna_blc->global_blc.global_blc1[i][j] = blc_ctx->sns_black_level[i][j];
                dyna_blc->global_blc.target_blc[i][j]  = blc_ctx->isp_black_level[i][j];
            }
        }
        /* actual */
        update_actual_blc_normal(blc_ctx);
    }
}

static td_void be_alg_blc_dyna_cfg(ot_vi_pipe vi_pipe, isp_be_blc_dyna_cfg *dyna_blc)
{
    td_u8 i, j;
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
        for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
            dyna_blc->ge_blc[i].blc[j]      = dyna_blc->global_blc.global_blc0[i][j]; /* GE */
            dyna_blc->wdr_dg_blc[i].blc[j]  = dyna_blc->global_blc.global_blc0[i][j]; /* 4DG */
            dyna_blc->wdr_blc[i].blc[j]     = dyna_blc->global_blc.global_blc0[i][j]; /* WDR */
            dyna_blc->flicker_blc[i].blc[j] = dyna_blc->global_blc.global_blc0[i][j]; /* flicker */
        }

        dyna_blc->expander_blc.blc[j] = 0;
        dyna_blc->lsc_blc.blc[j]      = dyna_blc->global_blc.target_blc[0][j];  /* lsc */
        if (isp_ctx->linkage.blc_fix.enable == TD_TRUE) {
            dyna_blc->dg_blc.blc[j] = (td_u16)isp_ctx->linkage.blc_fix.blc;      /* Dg */
            dyna_blc->dg_real_blc.blc[j]  = (td_u16)isp_ctx->linkage.blc_fix.blc; /* Dg */
        } else {
            dyna_blc->dg_blc.blc[j] = dyna_blc->global_blc.target_blc[0][j];  /* Dg */
            dyna_blc->dg_real_blc.blc[j]  = dyna_blc->global_blc.global_blc1[0][j]; /* Dg */
        }
        dyna_blc->wb_blc.blc[j]       = dyna_blc->global_blc.target_blc[0][j];  /* WB */
    }
    dyna_blc->bnr_blc.blc[0] = dyna_blc->global_blc.target_blc[0][0] >> 2; /* shift 2 */
}

static td_void be_blc_dyna_regs(ot_vi_pipe vi_pipe, td_u8 wdr_mode, isp_be_blc_dyna_cfg *dyna_blc,
    isp_blacklevel_ctx *blc_ctx)
{
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    dyna_blc->global_blc.blc_mode = blc_ctx->blc_mode;
    if ((is_offline_mode(isp_ctx->block_attr.running_mode) ||
         is_striping_mode(isp_ctx->block_attr.running_mode)) &&
        (blc_ctx->blc_mode == ISP_BLC_REAL_TIME)) {
        dyna_blc->global_blc.blc_mode = ISP_BLC_GLOBAL;
    }
    if (dyna_blc->global_blc.blc_mode == 0) {
        dyna_blc->global_blc.target_mode = 0;
    } else {
        dyna_blc->global_blc.target_mode = blc_ctx->user_black_level_en;
    }

    be_blc_calc_by_wdr_mode(wdr_mode, dyna_blc, blc_ctx);

    be_alg_blc_dyna_cfg(vi_pipe, dyna_blc);

    dyna_blc->black_level_mode = blc_ctx->op_mode;
    dyna_blc->resh_dyna = TD_TRUE;
}

static td_void be_blc_static_regs(td_u8 wdr_mode, isp_be_blc_static_cfg *static_blc)
{
    td_u8 i;
    ot_unused(wdr_mode);
    /* 4DG */
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        static_blc->ge_blc[i].blc_in      = TD_TRUE;
        static_blc->ge_blc[i].blc_out     = TD_TRUE;
        static_blc->wdr_dg_blc[i].blc_in  = TD_TRUE;
        static_blc->wdr_dg_blc[i].blc_out = TD_TRUE;
    }

    /* WDR */
    static_blc->wdr_blc[0].blc_out = TD_TRUE;
    /* lsc */
    static_blc->lsc_blc.blc_in     = TD_TRUE;
    static_blc->lsc_blc.blc_out    = TD_TRUE;
    /* Dg */
    static_blc->dg_blc.blc_in      = TD_TRUE;
    static_blc->dg_blc.blc_out     = TD_FALSE;
    static_blc->dg_blc.blc_compensation_en = TD_TRUE;
    /* AF */
    static_blc->af_blc.blc_in      = TD_FALSE;
    /* AE */
    static_blc->ae_blc.blc_in      = TD_FALSE;
    /* MG */
    static_blc->mg_blc.blc_in      = TD_FALSE;
    /* WB */
    static_blc->wb_blc.blc_in      = TD_FALSE;
    static_blc->wb_blc.blc_out     = TD_FALSE;
    static_blc->wb_blc.blc_compensation_en     = TD_FALSE;

    static_blc->resh_static = TD_TRUE;
}
static td_void fe_global_blc_regs(isp_gloal_blc_cfg *global_blc, isp_blacklevel_ctx *blc_ctx)
{
    td_u8 i, j;
    global_blc->blc_mode = blc_ctx->blc_mode;
    if (global_blc->blc_mode == 0) {
        global_blc->target_mode = 0;
    } else {
        global_blc->target_mode = blc_ctx->user_black_level_en;
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            global_blc->global_blc1[i][j] = blc_ctx->sns_black_level[i][j];
            global_blc->target_blc[i][j]  = blc_ctx->isp_black_level[i][j];
        }
    }
}
static td_void fe_blc_dyna_regs(isp_fe_blc_dyna_cfg *dyna_blc, isp_blacklevel_ctx *blc_ctx)
{
    td_u8  i, j;
    td_s16 global_blc1, target_blc, diff;

    fe_global_blc_regs(&dyna_blc->global_blc, blc_ctx);

    dyna_blc->fe_blc1_en = blc_ctx->user_black_level_en;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            global_blc1 = dyna_blc->global_blc.global_blc1[i][j];
            target_blc  = dyna_blc->global_blc.target_blc[i][j];
            diff = clip3(global_blc1 - target_blc, -16383, 16383); /* range[-16383, 16383] */
            dyna_blc->fe_blc[i].blc[j]            = (td_u16)diff;
            dyna_blc->fe_nr_dg_blc[i].blc[j]      = target_blc;  /* Fe Dg1 */
            dyna_blc->fe_nr_dg_real_blc[i].blc[j] = global_blc1; /* Fe Dg1 */
            dyna_blc->fe_dg_blc[i].blc[j]         = target_blc;  /* Fe Dg2 */
            dyna_blc->fe_dg_real_blc[i].blc[j]    = global_blc1; /* Fe Dg2 */
            dyna_blc->fe_wb_blc[i].blc[j]         = target_blc; /* Fe WB */
            dyna_blc->fe_af_blc[i].blc[j]         = target_blc; /* Fe AF */
            dyna_blc->fe_ae_blc[i].blc[j]         = target_blc; /* Fe AE */
        }
    }

    dyna_blc->resh_dyna = TD_TRUE;
}

static td_void fe_blc_static_regs(isp_fe_blc_static_cfg *static_blc)
{
     /* Fe nr Dg */
    static_blc->fe_nr_dg_blc.blc_in  = TD_TRUE;
    static_blc->fe_nr_dg_blc.blc_out = TD_TRUE;
    static_blc->fe_nr_dg_blc.blc_compensation_en = TD_FALSE;
    static_blc->fe_nr_dg_offset_shift = 0;

    /* Fe Dg */
    static_blc->fe_dg_blc.blc_in  = TD_TRUE;
    static_blc->fe_dg_blc.blc_out = TD_TRUE;
    static_blc->fe_dg_blc.blc_compensation_en = TD_FALSE;
    static_blc->fe_dg_offset_shift = 0;

    /* Fe WB */
    static_blc->fe_wb_blc.blc_in  = TD_TRUE;
    static_blc->fe_wb_blc.blc_out = TD_TRUE;
    /* fe af */
    static_blc->fe_af_blc.blc_in  = TD_TRUE;
    /* Fe AE */
    static_blc->fe_ae_blc.blc_in  = TD_FALSE;

    static_blc->resh_static = TD_TRUE;
}

static td_void blc_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_blacklevel_ctx *blc_ctx)
{
    td_u8  i, wdr_mode;
    isp_usr_ctx  *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    wdr_mode = isp_ctx->sns_wdr_mode;

    /* BE */
    for (i = 0; i < reg_cfg->cfg_num; i++) {
        be_blc_dyna_regs(vi_pipe, wdr_mode, &reg_cfg->alg_reg_cfg[i].be_blc_cfg.dyna_blc, blc_ctx);
        be_blc_static_regs(wdr_mode, &reg_cfg->alg_reg_cfg[i].be_blc_cfg.static_blc);
        reg_cfg->alg_reg_cfg[i].be_blc_cfg.resh_dyna_init = TD_TRUE;
    }
    reg_cfg->cfg_key.bit1_be_blc_cfg = 1;

    /* FE */
    fe_blc_dyna_regs(&reg_cfg->alg_reg_cfg[0].fe_blc_cfg.dyna_blc, blc_ctx);
    fe_blc_static_regs(&reg_cfg->alg_reg_cfg[0].fe_blc_cfg.static_blc);
    reg_cfg->alg_reg_cfg[0].fe_blc_cfg.resh_dyna_init = TD_TRUE;
    reg_cfg->cfg_key.bit1_fe_blc_cfg                  = 1;
}

static td_void blc_read_extregs(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx, td_u8 wdr_mode)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    blc_ctx->black_level_change = ot_ext_system_black_level_change_read(vi_pipe);
    if (blc_ctx->black_level_change) {
        ot_ext_system_black_level_change_write(vi_pipe, TD_FALSE);
        blc_ctx->op_mode = ot_ext_system_black_level_mode_read(vi_pipe);
        if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_MANUAL) {
            isp_black_level_manual_attr_read(vi_pipe, &blc_ctx->manual_attr);
        } else if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
            isp_black_level_dynamic_attr_read(vi_pipe, &blc_ctx->dyna_blc.mpi_cfg);
        }

        if (is_fs_wdr_mode(wdr_mode) == TD_TRUE) {
            /* WDR supports different BLC for different frames, but does not support global and real-time modes */
            /* to prevent possible switching issues. WDR uses the normal mode. */
            blc_ctx->blc_mode = ISP_BLC_NORMAL;
        } else if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
            blc_ctx->blc_mode = ISP_BLC_REAL_TIME;
        } else {
            blc_ctx->blc_mode = ISP_BLC_GLOBAL;
        }

        blc_ctx->user_black_level_en = ot_ext_system_isp_user_blc_en_read(vi_pipe);
        if (blc_ctx->user_black_level_en == TD_TRUE) {
            isp_user_black_level_read(vi_pipe, blc_ctx->user_black_level);
        }
    }

    // When the OB area is above the ISP processing data, REAL_TIME can take effect in real time. */
    /* If it is not above, it cannot take effect within the frame. */
    if (is_fs_wdr_mode(wdr_mode) != TD_TRUE && blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        blc_ctx->blc_mode = (blc_ctx->dyna_blc.mpi_cfg.ob_area.y + blc_ctx->dyna_blc.mpi_cfg.ob_area.height <=
        (td_u32)isp_ctx->sys_rect.y) ? ISP_BLC_REAL_TIME : ISP_BLC_GLOBAL;
    }

    blc_ctx->expander_update = ot_ext_system_expander_blc_param_update_read(vi_pipe);
    if (blc_ctx->expander_update) {
        ot_ext_system_expander_blc_param_update_write(vi_pipe, TD_FALSE);
        isp_expander_attr_read(vi_pipe, &blc_ctx->expander_attr);
    }
}

static td_void dynablc_regs_config(const ot_isp_black_level_dynamic_attr *dynamic_attr, isp_dynamicblc_usr_cfg *dynablc,
    const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx)
{
    dynablc->dynablc_pattern    = dynamic_attr->pattern;
    dynablc->dynablc_start_xpos = dynamic_attr->ob_area.x;
    dynablc->dynablc_start_ypos = dynamic_attr->ob_area.y;
    dynablc->dynablc_end_xpos   = dynamic_attr->ob_area.x + dynamic_attr->ob_area.width - 1;
    dynablc->dynablc_end_ypos   = dynamic_attr->ob_area.y + dynamic_attr->ob_area.height - 1;
    dynablc->dynablc_low_threshold = dynamic_attr->low_threshold;
    dynablc->dynablc_high_threshold = dynamic_attr->high_threshold;
    dynablc->bfilter_en         = TD_TRUE;
    dynablc->dynablc_tar0       = dynamic_attr->filter_thr;
    dynablc->dynablc_tar1       = dynamic_attr->tolerance;

    dynablc->usr_resh = TD_TRUE;
}

static td_s32 blc_proc_mode_check(const isp_blacklevel_ctx *blc_ctx, td_char mode_name[], td_u8 mode_name_size)
{
    td_s32 ret;
    if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_AUTO) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "auto");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_MANUAL) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "manual");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "dynamic");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "BUTT");
        if (ret < 0) {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 blc_sel_mode_check(const isp_blacklevel_ctx *blc_ctx, td_char mode_name[], td_u8 mode_name_size)
{
    td_s32 ret;
    if (blc_ctx->blc_mode == ISP_BLC_GLOBAL) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "global");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else if (blc_ctx->blc_mode == ISP_BLC_NORMAL) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "normal");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else if (blc_ctx->blc_mode == ISP_BLC_REAL_TIME) {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "real_time");
        if (ret < 0) {
            return TD_FAILURE;
        }
    } else {
        ret = snprintf_s(mode_name, mode_name_size, mode_name_size - 1, "BUTT");
        if (ret < 0) {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 isp_blc_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    td_u8 i;
    size_t mode_name_size, blc_sel_size;
    td_s32 ret;
    ot_isp_ctrl_proc_write proc_tmp;
    td_char mode_name[MAX_MMZ_NAME_LEN] = { 0 };
    td_char blc_sel_name[MAX_MMZ_NAME_LEN] = { 0 };
    isp_blacklevel_ctx *blc_ctx = TD_NULL;
    td_bool clamp_en = TD_TRUE;

    blacklevel_get_ctx(vi_pipe, blc_ctx);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    mode_name_size = sizeof(mode_name);
    blc_sel_size = sizeof(blc_sel_name);

    ret = blc_proc_mode_check(blc_ctx, mode_name, mode_name_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = blc_sel_mode_check(blc_ctx, blc_sel_name, blc_sel_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = isp_sensor_update_blc_clamp_info(vi_pipe, &clamp_en);
    isp_proc_print_title(&proc_tmp, &proc->write_len, "black level actual info");
    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%10s" "%10s" "%10s" "%11s" "%11s" "%11s" "%11s" "%11s" "%11s" "%11s" "%11s\n",
                    "mode", "user_en", "blc_mode", "isp_blc_r", "isp_blc_gr", "isp_blc_gb", "isp_blc_b",
                    "sns_blc_r", "sns_blc_gr", "sns_blc_gb", "sns_blc_b");

    for (i = 0; i < blc_ctx->merge_frame; i++) {
        isp_proc_printf(&proc_tmp, proc->write_len,
            "%10s" "%10u" "%11s" "%11u" "%11u" "%11u" "%11u" "%11u" "%11u" "%11u" "%11u\n",
            mode_name, blc_ctx->user_black_level_en, blc_sel_name,
            blc_ctx->actual.isp_black_level[i][OT_ISP_CHN_R], blc_ctx->actual.isp_black_level[i][OT_ISP_CHN_GR],
            blc_ctx->actual.isp_black_level[i][OT_ISP_CHN_GB], blc_ctx->actual.isp_black_level[i][OT_ISP_CHN_B],
            blc_ctx->actual.sns_black_level[i][OT_ISP_CHN_R], blc_ctx->actual.sns_black_level[i][OT_ISP_CHN_GR],
            blc_ctx->actual.sns_black_level[i][OT_ISP_CHN_GB], blc_ctx->actual.sns_black_level[i][OT_ISP_CHN_B]);
    }

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "\n%10s" "%10s" "%11s" "%11s" "%11s" "%16s\n",
                    "offset", "ob_x", "ob_y", "ob_width", "ob_height", "blc_clamp_info");
    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%10d" "%10d" "%11d" "%11u" "%11u" "%16s\n",
                    blc_ctx->dyna_blc.offset, blc_ctx->dyna_blc.mpi_cfg.ob_area.x, blc_ctx->dyna_blc.mpi_cfg.ob_area.y,
                    blc_ctx->dyna_blc.mpi_cfg.ob_area.width, blc_ctx->dyna_blc.mpi_cfg.ob_area.height,
                    (ret == TD_SUCCESS) ? ((clamp_en == TD_TRUE) ? "1" : "0") : "none");

    proc->write_len += 1;

    return TD_SUCCESS;
}

static td_s32 isp_blc_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_s32 ret;
    isp_blacklevel_ctx *blc_ctx = TD_NULL;
    blacklevel_get_ctx(vi_pipe, blc_ctx);
    blc_ctx->init = TD_FALSE;

    ot_ext_system_isp_blc_init_status_write(vi_pipe, blc_ctx->init);
    ret = blc_initialize(vi_pipe, blc_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    blc_regs_initialize(vi_pipe, (isp_reg_cfg *)reg_cfg_info, blc_ctx);
    blc_ext_regs_initialize(vi_pipe, blc_ctx);
    dynablc_regs_initialize(vi_pipe, (isp_reg_cfg *)reg_cfg_info, blc_ctx);
    dynablc_ext_regs_initialize(vi_pipe, blc_ctx);

    blc_ctx->init = TD_TRUE;
    ot_ext_system_isp_blc_init_status_write(vi_pipe, blc_ctx->init);

    return TD_SUCCESS;
}

static td_void dp_calib_mode_blc_cfg(const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx, isp_reg_cfg *reg_cfg)
{
    td_u8 i;

    if (isp_ctx->linkage.defect_pixel) {
        if (blc_ctx->pre_defect_pixel == TD_FALSE) {
            for (i = 0; i < reg_cfg->cfg_num; i++) {
                reg_cfg->alg_reg_cfg[i].be_blc_cfg.static_blc.wb_blc.blc_in = TD_TRUE;
                reg_cfg->alg_reg_cfg[i].be_blc_cfg.static_blc.resh_static   = TD_TRUE;
            }
        }
    } else if (blc_ctx->pre_defect_pixel) {
        for (i = 0; i < reg_cfg->cfg_num; i++) {
            reg_cfg->alg_reg_cfg[i].be_blc_cfg.static_blc.wb_blc.blc_in = TD_FALSE;
            reg_cfg->alg_reg_cfg[i].be_blc_cfg.static_blc.resh_static   = TD_TRUE;
        }
    }

    blc_ctx->pre_defect_pixel = isp_ctx->linkage.defect_pixel;
}

static td_void blc_regs_run(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx,
    isp_blacklevel_ctx *blc_ctx, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    get_isp_black_level_value(blc_ctx);
    fe_blc_dyna_regs(&reg_cfg->alg_reg_cfg[0].fe_blc_cfg.dyna_blc, blc_ctx);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        be_blc_dyna_regs(vi_pipe, isp_ctx->sns_wdr_mode, &reg_cfg->alg_reg_cfg[i].be_blc_cfg.dyna_blc, blc_ctx);
        isp_dynamic_blc_update_sync_cfg(blc_ctx, &reg_cfg->alg_reg_cfg[i].dynamic_blc_reg_cfg.sync_cfg);
    }

    black_level_actual_value_write(vi_pipe, &blc_ctx->actual);
}

static td_void isp_manual_blc_regs_run(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx,
    isp_reg_cfg *reg_cfg)
{
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;
    get_sns_black_level_value(isp_ctx, blc_ctx, sns_black_level);
    blc_regs_run(vi_pipe, isp_ctx, blc_ctx, reg_cfg);
}

static td_void isp_dynamic_blc_regs_run(const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx, isp_reg_cfg *reg_cfg)
{
    reg_cfg->alg_reg_cfg[0].dynamic_blc_reg_cfg.dynablc_enable = TD_TRUE;
    dynablc_regs_config(&blc_ctx->dyna_blc.mpi_cfg, &reg_cfg->alg_reg_cfg[0].dynamic_blc_reg_cfg.usr_reg_cfg,
                        isp_ctx, blc_ctx);
    dynablc_ag_offset_inter(isp_ctx->linkage.again, blc_ctx);
    dynablc_offset_inter(isp_ctx->linkage.sensor_iso, blc_ctx);
    isp_dynamic_blc_update_sync_cfg(blc_ctx, &reg_cfg->alg_reg_cfg[0].dynamic_blc_reg_cfg.sync_cfg);
    if (isp_ctx->actual_blc.is_ready == TD_TRUE) {
        (td_void)memcpy_s(&blc_ctx->actual, sizeof(isp_blc_actual_info),
                          &isp_ctx->actual_blc, sizeof(isp_blc_actual_info));
    }

    blc_ctx->dyna_blc.pre_pre_again = blc_ctx->dyna_blc.pre_again;

    blc_ctx->dyna_blc.pre_again = isp_ctx->linkage.again;
}

static td_void pipe_diff_offset(ot_vi_pipe vi_pipe, isp_blacklevel_ctx *blc_ctx)
{
    td_u8  i;
    td_s16 blk_offset;
    td_s32 blc_tmp;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        blk_offset = ot_ext_system_isp_pipe_diff_offset_read(vi_pipe, i); /* 14 bits */
        blc_tmp = (td_s32)blc_ctx->sns_black_level[0][i] + blk_offset;
        blc_ctx->sns_black_level[0][i] = clip3(blc_tmp, 0, 0x3FFF);
    }
}

static td_void isp_blc_auto_cmos_mode(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx,
    isp_blacklevel_ctx *blc_ctx, isp_reg_cfg *reg_cfg)
{
    ot_vi_pipe main_pipe;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;
    isp_sensor_get_blc(vi_pipe, &sns_black_level);
#ifdef CONFIG_OT_SNAP_SUPPORT
    if (isp_ctx->linkage.snap_pipe_mode != ISP_SNAP_NONE) {
        if (vi_pipe == isp_ctx->linkage.picture_pipe_id) {
            main_pipe = isp_ctx->linkage.preview_pipe_id;
            isp_check_pipe_void_return(main_pipe);
            isp_sensor_get_blc(main_pipe, &sns_black_level);
        } else {
            isp_sensor_get_blc(vi_pipe, &sns_black_level);
        }

        get_sns_black_level_value(isp_ctx, blc_ctx, sns_black_level);
    }
#endif
    /* ISP Multi-pipe blackLevel different configs */
    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        if (isp_ctx->stitch_attr.main_pipe != TD_TRUE) {
            main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];
            isp_check_pipe_void_return(main_pipe);
            isp_sensor_get_blc(main_pipe, &sns_black_level);
        } else {
            isp_sensor_update_blc(vi_pipe);
        }
        get_sns_black_level_value(isp_ctx, blc_ctx, sns_black_level);
        pipe_diff_offset(vi_pipe, blc_ctx);
    } else {
        isp_sensor_update_blc(vi_pipe);
        get_sns_black_level_value(isp_ctx, blc_ctx, sns_black_level);
    }

    blc_regs_run(vi_pipe, isp_ctx, blc_ctx, reg_cfg);

    return;
}

static td_void isp_blc_auto_mode(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx,
    isp_blacklevel_ctx *blc_ctx, isp_reg_cfg *reg_cfg, ot_isp_cmos_black_level *sns_black_level)
{
    isp_sensor_update_blc(vi_pipe);
    get_sns_black_level_value(isp_ctx, blc_ctx, sns_black_level);
    /* ISP Multi-pipe blackLevel different configs */
    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        pipe_diff_offset(vi_pipe, blc_ctx);
    }

    blc_regs_run(vi_pipe, isp_ctx, blc_ctx, reg_cfg);
}

static td_void isp_blc_reg_update(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx, isp_blacklevel_ctx *blc_ctx,
    isp_reg_cfg *reg_cfg)
{
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;

    blc_read_extregs(vi_pipe, blc_ctx, isp_ctx->sns_wdr_mode);

    reg_cfg->alg_reg_cfg[0].dynamic_blc_reg_cfg.dynablc_enable = TD_FALSE;
    /* manual mode update */
    if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_MANUAL) {
        isp_manual_blc_regs_run(vi_pipe, isp_ctx, blc_ctx, reg_cfg);
        return;
    }

    /* dynamic blc */
    if (blc_ctx->op_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        if (is_no_fe_pipe(vi_pipe)) {
            isp_err_trace("ISP[%d]: Not Support dynamic blc when no fe pipe!\n", vi_pipe);
            return;
        }
        if (is_built_in_wdr_mode(isp_ctx->sns_wdr_mode)) {
            isp_err_trace("ISP[%d]: Not Support dynamic blc in sensor built-in mode!\n", vi_pipe);
            return;
        }
        isp_dynamic_blc_regs_run(isp_ctx, blc_ctx, reg_cfg);
        return;
    }

    /* auto mode */
    /* some sensors's blacklevel is changed with iso. */
    /* sensors's blacklevel is changed by cmos. */
    isp_sensor_get_blc(vi_pipe, &sns_black_level);
    if (sns_black_level->auto_attr.update == TD_TRUE) {
        isp_blc_auto_cmos_mode(vi_pipe, isp_ctx, blc_ctx, reg_cfg);
        return;
    }

    isp_blc_auto_mode(vi_pipe, isp_ctx, blc_ctx, reg_cfg, sns_black_level);
}

static td_s32 isp_blc_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_info, td_s32 rsv)
{
    td_u8 i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_BLC;
    ot_vi_pipe         stitch_pipe;
    isp_usr_ctx        *isp_ctx             = TD_NULL;
    isp_usr_ctx        *isp_ctx_stitch_pipe = TD_NULL;
    isp_blacklevel_ctx *blc_ctx             = TD_NULL;
    isp_blacklevel_ctx *blc_ctx_stitch_pipe = TD_NULL;
    isp_reg_cfg        *reg_cfg             = (isp_reg_cfg *)reg_cfg_info;
    isp_reg_cfg_attr   *reg_cfg_attr        = TD_NULL;
    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    blacklevel_get_ctx(vi_pipe, blc_ctx);

    ot_ext_system_isp_blc_init_status_write(vi_pipe, blc_ctx->init);
    if (blc_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }
    if (ot_ext_system_dpc_static_defect_type_read(vi_pipe) == 0) { /* hot pixel */
        dp_calib_mode_blc_cfg(isp_ctx, blc_ctx, reg_cfg);
    }

    reg_cfg->cfg_key.bit1_fe_blc_cfg  = 1;
    reg_cfg->cfg_key.bit1_be_blc_cfg  = 1;
    reg_cfg->cfg_key.bit1_dynablc_cfg = 1;

    if (isp_ctx->stitch_attr.stitch_enable == TD_FALSE) {
        isp_blc_reg_update(vi_pipe, isp_ctx, blc_ctx, reg_cfg);
        return TD_SUCCESS;
    }

    if (isp_ctx->stitch_attr.main_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    for (i = 0; i < isp_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = isp_ctx->stitch_attr.stitch_bind_id[i];
        isp_get_ctx(stitch_pipe, isp_ctx_stitch_pipe);
        blacklevel_get_ctx(stitch_pipe, blc_ctx_stitch_pipe);
        isp_regcfg_get_ctx(stitch_pipe, reg_cfg_attr);
        isp_check_pointer_return(reg_cfg_attr);
        isp_blc_reg_update(stitch_pipe, isp_ctx_stitch_pipe, blc_ctx_stitch_pipe, &reg_cfg_attr->reg_cfg);
    }

    return TD_SUCCESS;
}

static td_s32 isp_blc_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;

    switch (cmd) {
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
            isp_check_pointer_return(reg_cfg_attr);
            isp_blc_init(vi_pipe, (td_void *)&reg_cfg_attr->reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            isp_blc_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

static td_s32 isp_blc_exit(ot_vi_pipe vi_pipe)
{
    ot_ext_system_isp_blc_init_status_write(vi_pipe, TD_FALSE);
    return TD_SUCCESS;
}

td_s32 isp_alg_register_blc(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_blc);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_BLC;
    algs->alg_func.pfn_alg_init = isp_blc_init;
    algs->alg_func.pfn_alg_run  = isp_blc_run;
    algs->alg_func.pfn_alg_ctrl = isp_blc_ctrl;
    algs->alg_func.pfn_alg_exit = isp_blc_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
