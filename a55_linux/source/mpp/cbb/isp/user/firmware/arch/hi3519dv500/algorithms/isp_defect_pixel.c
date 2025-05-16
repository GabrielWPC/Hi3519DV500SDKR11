/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_proc.h"
#include "isp_sensor.h"
#include "isp_param_check.h"

#include <stdio.h>

#define ISP_DPCC_MODE              35
#define ISP_DPCC_HOT_MODE          7
#define ISP_DPCC_DEAD_MODE         71
#define ISP_DPCC_HIGHLIGHT_MODE    160
#define ISP_HOT_DEV_THRESH         26
#define ISP_DEAD_DEV_THRESH        19

#define ISP_DPC_SLOPE_GRADE        5
#define ISP_DPC_SOFT_SLOPE_GRADE   5
#define ISP_DPC_HIGH_SLOPE_GRADE  27
#define ISP_DPC_HIGH_SLOP_OFFSET  101

#define ISP_DPC_COLOR_NUM        2

#define ISP_DPC_STRENGTH_MAX_THR     255
#define ISP_DPC_BLEND_RATIO_MAX_THR  0x80
#define DPC_STATIC_THR_MAX           255
#define DPC_TWINKLE_THR_MIN          (-128)
#define DPC_TWINKLE_THR_MAX          127
#define DPC_STATIC_HOT               0
#define DPC_STATIC_DARK              1
#define ISP_DPC_LINE_STD_GAIN        9
#define ISP_DPC_RG_FAC_GAIN         11
#define ISP_DPC_STATIC_TAB_SIZE      (OT_ISP_STATIC_DP_COUNT_NORMAL * sizeof(td_s32))

#ifdef OT_ASIC
#define ISP_DPC_CALIB_FRAME_CNT_FOR_READ 6
#else
#define ISP_DPC_CALIB_FRAME_CNT_FOR_READ 7
#endif

#define isp_dpc_calc_mode(base_mode, sta_en, dy_en) \
    (((base_mode)  & 0x3dd) + (((td_u32)(sta_en)) << 5) + (((td_u32)(dy_en)) << 1))

static const ot_isp_dp_frame_dynamic_attr g_dpc_dynamic_attr = {
    1,          /* twinkle suppression enable */
    6,          /* twinkle suppression threshold */
    21,         /* twinkle suppression slop */
    0,          /* 0, auto mode; 1, manual mode */
    {
        0,      /* manual mode strength */
        0,      /* manual mode blend ratio */
    },
    {
        { 0, 0, 0, 152, 200, 200, 220, 220, 220, 220, 152, 152, 152, 152, 152, 152 }, /* auto mode strength */
        { 0, 0, 0,  0,  0,  0,  0,  0,  0,  0, 50, 50, 50, 50, 50, 50 },              /* auto mode blend */
    },
    96,         /* strength for bright area */
    63,         /* strength for dark area */
};

static const td_u8 g_slope_grade[ISP_DPC_SLOPE_GRADE] = {0, 76, 99, 100, 127};
static const td_u8  g_soft_slope_grade[ISP_DPC_SOFT_SLOPE_GRADE] = {0, 76, 100, 115, 120};
static const td_u16 g_soft_line_thr[ISP_DPC_SOFT_SLOPE_GRADE] = {0x5454, 0x1818, 0x1212, 0x0a0a, 0x0a0a};
static const td_u16 g_soft_line_mad_fac[ISP_DPC_SOFT_SLOPE_GRADE] = {0x1810, 0x1810, 0x1810, 0x1010, 0x0a0a};

typedef struct {
    /* public */
    td_bool init;
    td_bool enable;  /* enable dpc module */
    td_bool stat_en;
    td_u16 dpcc_mode;
    td_u32 dpcc_bad_thresh;
    /* static calib */
    td_bool sta_calibration_en;  /* enable static calibration */
    td_u8 pixel_detect_type;  /* 0: hot pixel detect; 1: dead pixel detect; */
    td_u8 frame_cnt;
    td_u8 static_dp_thresh;
    td_u8 trial_count;
    td_u8 trial_cnt_limit;
    td_u8 calib_started;
    td_u8 calib_finished;
    td_u8 hot_dev_thresh;
    td_u8 dead_dev_thresh;
    td_u16 dp_count_max;
    td_u16 dp_count_min;
    td_u16 bpt_calib_num;
    td_u16 blk_bp_calib_num[OT_ISP_STRIPING_MAX_NUM];
    td_u32 *bpt_calib_table[OT_ISP_STRIPING_MAX_NUM]; /* max size: ISP_STRIPING_MAX_NUM * ISP_DPC_MAX_BPT_NUM_NORMAL */

    /* static cor */
    td_bool static_enable;
    td_bool staic_show;
    td_bool static_attr_update;
    td_u16 bpt_cor_num;
    td_u16 offset[OT_ISP_STRIPING_MAX_NUM + 1];
    td_u16 offset_for_split[OT_ISP_STRIPING_MAX_NUM + 1];

    /* dynamic cor */
    td_bool dyna_attr_update_en;
    td_u16 actual_strength[ISP_DPC_MAX_CHN_NUM];
    td_u16 actual_blend_ratio[ISP_DPC_MAX_CHN_NUM];
    td_bool sup_twinkle_adapt[ISP_DPC_MAX_CHN_NUM];

    ot_isp_dp_dynamic_attr cmos_dpc;
} isp_defect_pixel;

typedef struct {
    td_u8 dpcc_set_use;
    td_u16 dpcc_methods_set1;
    td_u32 dpcc_bad_thresh;
} isp_dpc_cfg;

static const isp_dpc_cfg g_dpc_def_cfg[ISP_DPC_SLOPE_GRADE] = {
    {0x01, 0x1F1F, 0xff800080},  /* 0~75 */
    {0x03, 0x1F1F, 0xff800080},  /* ori set 1 (76) */
    {0x03, 0x1F1F, 0xff800080},  /* ori set 2 (99) */
    {0x07, 0x1F1F, 0xff800080},  /* set 23(RB set3, G set2) (100) */
    {0x07, 0x1F1F, 0xff800080},  /* 101 ~127 */
};

typedef struct {
    td_u8  dpcc_line_thr[ISP_DPC_COLOR_NUM];
    td_u8  dpcc_line_mad_fac[ISP_DPC_COLOR_NUM];
    td_u8  dpcc_pg_fac[ISP_DPC_COLOR_NUM];
    td_u8  dpcc_rg_fac[ISP_DPC_COLOR_NUM];
    td_u8  dpcc_ro[ISP_DPC_COLOR_NUM];
    td_u16 dpcc_rg_fac_mtp[ISP_DPC_COLOR_NUM];
} isp_dpcc_derived_param;

static const isp_dpcc_derived_param g_dpc_der_param[ISP_DPC_SLOPE_GRADE] = {
    {
        {0x54, 0x54},
        {0x1B, 0x1B},
        {0x20, 0x20},
        {0x98, 0x98},
        {0x01, 0x01},
        {0x16, 0x16},
    },

    {
        {0x06, 0x08},
        {0x12, 0x1B},
        {0x20, 0x20},
        {0x80, 0x80},
        {0x01, 0x01},
        {0x16, 0x16},
    },

    {
        {0x06, 0x08},
        {0x03, 0x04},
        {0x20, 0x20},
        {0x58, 0x58},
        {0x02, 0x02},
        {0x21, 0x21},
    },

    {
        {0x06, 0x08},
        {0x03, 0x04},
        {0x20, 0x20},
        {0x30, 0x30},
        {0x02, 0x02},
        {0x21, 0x21},
    },

    {
        {0x01, 0x01},
        {0x03, 0x03},
        {0x0c, 0x0c},
        {0x18, 0x18},
        {0x02, 0x02},
        {0x21, 0x21},
    },
};

typedef struct {
    td_u16 dpc_line_thr1;
    td_u16 dpc_line_mad_fac1;
    td_u16 dpc_pg_fac1;
    td_u16 dpc_rg_fac1;
    td_u16 dpc_ro_limits1;
} isp_dpcc_high_derived_param;

static const isp_dpcc_high_derived_param g_dpc_high_der_parm[ISP_DPC_HIGH_SLOPE_GRADE] = {
    {0x0508, 0x0304, 0x1010, 0x3030, 0x0dfa},
    {0x0508, 0x0304, 0x1010, 0x3030, 0x0dfa},
    {0x0508, 0x0304, 0x1010, 0x2c2c, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2c2c, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2828, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2828, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2424, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2424, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2020, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x2020, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x1c1c, 0x0efe},
    {0x0508, 0x0304, 0x0c0c, 0x1c1c, 0x0fff},
    {0x0507, 0x0304, 0x0c0c, 0x1818, 0x0fff},
    {0x0507, 0x0304, 0x0c0c, 0x1818, 0x0fff},
    {0x0507, 0x0304, 0x0c0c, 0x1818, 0x0fff},
    {0x0406, 0x0304, 0x0c0c, 0x1818, 0x0fff},
    {0x0406, 0x0304, 0x0c0c, 0x1414, 0x0fff},
    {0x0305, 0x0304, 0x0c0c, 0x1414, 0x0fff},
    {0x0305, 0x0304, 0x0c0c, 0x1414, 0x0fff},
    {0x0304, 0x0304, 0x0c0c, 0x1414, 0x0fff},
    {0x0304, 0x0304, 0x0c0c, 0x1010, 0x0fff},
    {0x0203, 0x0304, 0x0c0c, 0x1010, 0x0fff},
    {0x0203, 0x0203, 0x0c0c, 0x1010, 0x0fff},
    {0x0202, 0x0203, 0x0c0c, 0x1010, 0x0fff},
    {0x0202, 0x0203, 0x0c0c, 0x0c0c, 0x0fff},
    {0x0202, 0x0203, 0x0c0c, 0x0c0c, 0x0fff},
    {0x0202, 0x0203, 0x0c0c, 0x0808, 0x0fff}
};

isp_defect_pixel *g_dp_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define dp_get_ctx(pipe, ctx)   ((ctx) = g_dp_ctx[pipe])
#define dp_set_ctx(pipe, ctx)   (g_dp_ctx[pipe] = (ctx))
#define dp_reset_ctx(pipe)      (g_dp_ctx[pipe] = TD_NULL)

static td_s32 dp_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);

    if (dp == TD_NULL) {
        dp = (isp_defect_pixel *)isp_malloc(sizeof(isp_defect_pixel));
        if (dp == TD_NULL) {
            isp_err_trace("Isp[%d] DpCtx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(dp, sizeof(isp_defect_pixel), 0, sizeof(isp_defect_pixel));

    dp_set_ctx(vi_pipe, dp);

    return TD_SUCCESS;
}

static td_void dp_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);
    isp_free(dp);
    dp_reset_ctx(vi_pipe);
}

static td_u8 dp_get_chn_num(td_u8 wdr_mode)
{
    if (is_linear_mode(wdr_mode)) {
        return 0x1;
    } else if (is_built_in_wdr_mode(wdr_mode)) {
        return 0x1;
    } else if (is_2to1_wdr_mode(wdr_mode)) {
        return 0x2;
    } else {
        /* unknown mode */
        return 0x1;
    }
}

static td_void dpc_safe_free_calib_lut(isp_defect_pixel *dp, td_u8 cnt)
{
    td_u8 i;

    for (i = 0; i < cnt; i++) {
        if (dp->bpt_calib_table[i] != TD_NULL) {
            isp_free(dp->bpt_calib_table[i]);
            dp->bpt_calib_table[i] = TD_NULL;
        }
    }

    dp->calib_finished = 0;
}

static td_void isp_dp_enable_cfg(ot_vi_pipe vi_pipe, td_u8 cfg_num, isp_reg_cfg *reg_cfg)
{
    td_u8 i, j, chn_num;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    chn_num = dp_get_chn_num(isp_ctx->sns_wdr_mode);

    for (i = 0; i < cfg_num; i++) {
        for (j = 0; j < ISP_DPC_MAX_CHN_NUM; j++) {
            reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dpc_en[j] = (j < chn_num) ? (TD_TRUE) : (TD_FALSE);
        }
    }
}

static td_void dp_static_regs_initialize(isp_dpc_static_cfg *static_reg_cfg)
{
    static_reg_cfg->dpcc_bpt_ctrl = OT_ISP_DPC_DEFAULT_BPT_CTRL;
    static_reg_cfg->static_resh = TD_TRUE;
}

static td_void dp_usr_regs_initialize(isp_dpc_usr_cfg *usr_reg_cfg)
{
    td_u8 i;

    for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
        usr_reg_cfg->usr_dyna_cor_reg_cfg.hard_thr_en[i] = OT_ISP_DPC_DEFAULT_HARD_THR_ENABLE;
        usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_max[i] = OT_ISP_DPC_DEFAULT_SOFT_THR_MAX;
        usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_min[i] = OT_ISP_DPC_DEFAULT_SOFT_THR_MIN;
        usr_reg_cfg->usr_dyna_cor_reg_cfg.rake_ratio[i] = OT_ISP_DPC_DEFAULT_SOFT_RAKE_RATIO;
    }
    usr_reg_cfg->usr_dyna_cor_reg_cfg.resh = TD_TRUE;

    usr_reg_cfg->usr_sta_cor_reg_cfg.dpcc_bpt_number = OT_ISP_DPC_DEFAULT_BPT_NUMBER;
    usr_reg_cfg->usr_sta_cor_reg_cfg.update_index = 1;
    usr_reg_cfg->usr_sta_cor_reg_cfg.buf_id = 0;
    usr_reg_cfg->usr_sta_cor_reg_cfg.resh = TD_FALSE;
}

static td_void dp_dyna_regs_initialize(isp_dpc_dyna_cfg *dyna_reg_cfg)
{
    td_u8 i;
    dyna_reg_cfg->resh = TD_TRUE;
    dyna_reg_cfg->dpc_stat_en = 0;
    dyna_reg_cfg->dpcc_bad_thresh = OT_ISP_DPC_DEFAULT_BPT_THRESH;
    dyna_reg_cfg->dpcc_bit_depth_sel = OT_ISP_DPC_GE_DEFAULT_BIT_DEPTH_SEL;

    for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
        dyna_reg_cfg->dpcc_alpha[i] = OT_ISP_DPC_DEFAULT_ALPHA;
        dyna_reg_cfg->dpcc_mode[i] = OT_ISP_DPC_DEFAULT_MODE;
        dyna_reg_cfg->dpcc_set_use[i] = OT_ISP_DPC_DEFAULT_SET_USE;
        dyna_reg_cfg->dpcc_methods_set1[i] = OT_ISP_DPC_DEFAULT_METHODS_SET_1;
        dyna_reg_cfg->dpcc_line_thr[i] = OT_ISP_DPC_DEFAULT_LINE_THRESH_1;
        dyna_reg_cfg->dpcc_line_mad_fac[i] = OT_ISP_DPC_DEFAULT_LINE_MAD_FAC_1;
        dyna_reg_cfg->dpcc_pg_fac[i] = OT_ISP_DPC_DEFAULT_PG_FAC_1;
        dyna_reg_cfg->dpcc_rg_fac[i] = OT_ISP_DPC_DEFAULT_RG_FAC_1;
        dyna_reg_cfg->dpcc_ro_limits[i] = OT_ISP_DPC_DEFAULT_RO_LIMITS;
        dyna_reg_cfg->dpcc_line_kerdiff_fac[i] = OT_ISP_DPC_DEFAULT_LINE_KERDIFF_FAC;
        dyna_reg_cfg->dpcc_line_std_thr[i] = OT_ISP_DPC_DEFAULT_LINE_STD_THR_1 * ISP_DPC_LINE_STD_GAIN;
        dyna_reg_cfg->dpcc_line_diff_thr[i] = OT_ISP_DPC_DEFAULT_LINE_DIFF_THR_1;
        dyna_reg_cfg->dpcc_line_aver_fac[i] = OT_ISP_DPC_DEFAULT_LINE_AVER_FAC_1;
        dyna_reg_cfg->dpcc_amp_coef_k[i] = OT_ISP_DPC_DEFAULT_AMP_COEF_K;
        dyna_reg_cfg->dpcc_amp_coef_min[i] = OT_ISP_DPC_DEFAULT_AMP_COEF_MIN;
    }
}

static td_void dp_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        dp_static_regs_initialize(&reg_cfg->alg_reg_cfg[i].dp_reg_cfg.static_reg_cfg);
        dp_dyna_regs_initialize(&reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg);
        dp_usr_regs_initialize(&reg_cfg->alg_reg_cfg[i].dp_reg_cfg.usr_reg_cfg);

        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.lut2_stt_en = TD_TRUE;
    }

    isp_dp_enable_cfg(vi_pipe, reg_cfg->cfg_num, reg_cfg);

    reg_cfg->cfg_key.bit1_dp_cfg = 1;
}

static td_void dp_ext_regs_initialize(ot_vi_pipe vi_pipe)
{
    td_u8 i, j;
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);

    /* dynamic attr */
    ot_ext_system_dpc_dynamic_cor_enable_write(vi_pipe, dp->cmos_dpc.enable);

    for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
        ot_isp_dp_frame_dynamic_attr *frame_dynamic = &(dp->cmos_dpc.frame_dynamic[i]);

        ot_ext_system_dpc_dynamic_manual_enable_write(vi_pipe, i, frame_dynamic->op_type);
        ot_ext_system_dpc_dynamic_strength_write(vi_pipe, i, frame_dynamic->manual_attr.strength);
        ot_ext_system_dpc_dynamic_blend_ratio_write(vi_pipe, i, frame_dynamic->manual_attr.blend_ratio);
        ot_ext_system_dpc_suppress_twinkle_enable_write(vi_pipe, i, frame_dynamic->sup_twinkle_en);
        ot_ext_system_dpc_suppress_twinkle_thr_write(vi_pipe, i, frame_dynamic->soft_thr);
        ot_ext_system_dpc_suppress_twinkle_slope_write(vi_pipe, i, frame_dynamic->soft_slope);
        ot_ext_system_dpc_bright_strength_write(vi_pipe, i, frame_dynamic->bright_strength);
        ot_ext_system_dpc_dark_strength_write(vi_pipe, i, frame_dynamic->dark_strength);

        for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
            td_u8 index = i * OT_ISP_AUTO_ISO_NUM + j;
            ot_ext_system_dpc_dynamic_strength_table_write(vi_pipe, index, frame_dynamic->auto_attr.strength[j]);
            ot_ext_system_dpc_dynamic_blend_ratio_table_write(vi_pipe, index, frame_dynamic->auto_attr.blend_ratio[j]);
        }
    }
    ot_ext_system_dpc_dynamic_attr_update_write(vi_pipe, TD_TRUE);

    /* static calib */
    ot_ext_system_dpc_static_calib_enable_write(vi_pipe, OT_EXT_SYSTEM_DPC_STATIC_CALIB_ENABLE_DEFAULT);
    ot_ext_system_dpc_count_max_write(vi_pipe, OT_EXT_SYSTEM_DPC_COUNT_MAX_DEFAULT);
    ot_ext_system_dpc_count_min_write(vi_pipe, OT_EXT_SYSTEM_DPC_COUNT_MIN_DEFAULT);
    ot_ext_system_dpc_start_thresh_write(vi_pipe, OT_EXT_SYSTEM_DPC_START_THRESH_DEFAULT);
    ot_ext_system_dpc_trigger_status_write(vi_pipe, OT_EXT_SYSTEM_DPC_TRIGGER_STATUS_DEFAULT);
    ot_ext_system_dpc_trigger_time_write(vi_pipe, OT_EXT_SYSTEM_DPC_TRIGGER_TIME_DEFAULT);
    ot_ext_system_dpc_static_defect_type_write(vi_pipe, OT_EXT_SYSTEM_DPC_STATIC_DEFECT_TYPE_DEFAULT);
    ot_ext_system_dpc_finish_thresh_write(vi_pipe, OT_EXT_SYSTEM_DPC_START_THRESH_DEFAULT);
    ot_ext_system_dpc_bpt_calib_number_write(vi_pipe, OT_EXT_SYSTEM_DPC_BPT_CALIB_NUMBER_DEFAULT);

    /* static attr */
    ot_ext_system_dpc_bpt_cor_number_write(vi_pipe, OT_EXT_SYSTEM_DPC_BPT_COR_NUMBER_DEFAULT);
    ot_ext_system_dpc_static_cor_enable_write(vi_pipe, dp->enable);
    ot_ext_system_dpc_static_dp_show_write(vi_pipe, OT_EXT_SYSTEM_DPC_STATIC_DP_SHOW_DEFAULT);
    ot_ext_system_dpc_static_attr_update_write(vi_pipe, TD_TRUE);
}

static td_void dpc_image_size(ot_vi_pipe vi_pipe, isp_defect_pixel *dp)
{
    td_u8 i, block_num;
    isp_rect block_rect;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    block_num = isp_ctx->block_attr.block_num;

    for (i = 0; i < block_num; i++) {
        isp_get_block_rect(&block_rect, &isp_ctx->block_attr, i);

        dp->offset[i] = block_rect.x;
        dp->offset_for_split[i] = (i == 0) ? 0 : (dp->offset[i] + isp_ctx->block_attr.over_lap);
    }

    dp->offset_for_split[block_num] = isp_ctx->block_attr.frame_rect.width;
    dp->offset[block_num] = isp_ctx->block_attr.frame_rect.width;
}

static td_void dpc_def_calib_param(isp_defect_pixel *dp)
{
    dp->trial_count = 0;
    dp->calib_started = 0;
    dp->calib_finished = 0;
    dp->stat_en = 0;
    dp->hot_dev_thresh = ISP_HOT_DEV_THRESH;
    dp->dead_dev_thresh = ISP_DEAD_DEV_THRESH;
    dp->frame_cnt = 0;
    dp->dpcc_bad_thresh = 0xff800080;
}

static td_s32 isp_dpc_cmos_param_def(ot_vi_pipe vi_pipe, isp_defect_pixel *dp)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;
    isp_sensor_get_default(vi_pipe, &sns_dft);

    if (sns_dft->key.bit1_dpc) {
        isp_check_pointer_return(sns_dft->dpc);

        ret = isp_dp_dynamic_attr_check("cmos", sns_dft->dpc);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        (td_void)memcpy_s(&(dp->cmos_dpc), sizeof(ot_isp_dp_dynamic_attr),
                          sns_dft->dpc, sizeof(ot_isp_dp_dynamic_attr));
    } else {
        td_u8 i;
        dp->cmos_dpc.enable = TD_TRUE;

        for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
            (td_void)memcpy_s(&(dp->cmos_dpc.frame_dynamic[i]), sizeof(ot_isp_dp_frame_dynamic_attr),
                              &g_dpc_dynamic_attr, sizeof(ot_isp_dp_frame_dynamic_attr));
        }
    }
    return TD_SUCCESS;
}

static td_s32 dp_initialize(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);

    dp->enable = TD_TRUE;
    dp->init  = TD_FALSE;

    ret = isp_dpc_cmos_param_def(vi_pipe, dp);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    dpc_def_calib_param(dp);

    dpc_image_size(vi_pipe, dp);

    dp->init = TD_TRUE;

    return TD_SUCCESS;
}

static td_void dp_enter(ot_vi_pipe vi_pipe, isp_defect_pixel *dp)
{
    isp_sensor_set_pixel_detect(vi_pipe, TD_TRUE);
    dp->static_dp_thresh = ot_ext_system_dpc_start_thresh_read(vi_pipe);
    dp->calib_started = 1;
}

static td_void dp_exit(ot_vi_pipe vi_pipe, isp_defect_pixel *dp)
{
    dpc_def_calib_param(dp);
    isp_sensor_set_pixel_detect(vi_pipe, TD_FALSE);
    dp->calib_started = 0;
    dp->calib_finished = 1;
    dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_MODE, dp->static_enable, dp->cmos_dpc.enable);
}

static td_void dp_read_static_calib_extregs(ot_vi_pipe vi_pipe)
{
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);

    dp->pixel_detect_type = ot_ext_system_dpc_static_defect_type_read(vi_pipe);
    dp->trial_cnt_limit = (td_u8)(ot_ext_system_dpc_trigger_time_read(vi_pipe) >> 0x3);
    dp->dp_count_max = ot_ext_system_dpc_count_max_read(vi_pipe);
    dp->dp_count_min = ot_ext_system_dpc_count_min_read(vi_pipe);
}

static td_s32 dpc_striping_read_calib_num(td_u8 blk_num, isp_defect_pixel *dp, isp_stat *stat_info)
{
    td_u8 i;
    td_u16 j, cnt_temp;
    td_u16 bpt_cnt = 0;
    td_u32 bpt_value;
    td_u16 bp_calib_num[OT_ISP_STRIPING_MAX_NUM] = {0};

    for (i = 0; i < blk_num; i++) {
        isp_check_pointer_return(dp->bpt_calib_table[i]);
        cnt_temp = MIN2(stat_info->dp_stat.defect_pixel_count[i], OT_ISP_STATIC_DP_COUNT_NORMAL);

        for (j = 0; j < cnt_temp; j++) {
            bpt_value = stat_info->dp_stat.defect_pixel_lut[i][j] + dp->offset[i];

            if ((bpt_value & 0x1FFF) < dp->offset[i + 1]) {
                dp->bpt_calib_table[i][bp_calib_num[i]++] = bpt_value;
                bpt_cnt++;
            }
        }
        dp->blk_bp_calib_num[i] = bp_calib_num[i];
    }
    dp->bpt_calib_num = bpt_cnt;

    return TD_SUCCESS;
}

static td_s32 dpc_read_calib_num(ot_vi_pipe vi_pipe, td_u8 blk_num, isp_defect_pixel *dp, isp_stat *stat_info)
{
    td_u16 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        dp->bpt_calib_num = isp_dpc_bpt_calib_number_read(vi_pipe, 0);
        return TD_SUCCESS;
    }

    if (is_offline_mode(isp_ctx->block_attr.running_mode)) {
        dp->bpt_calib_num = MIN2(stat_info->dp_stat.defect_pixel_count[0], OT_ISP_STATIC_DP_COUNT_NORMAL);

        isp_check_pointer_return(dp->bpt_calib_table[0]);

        for (i = 0; i < dp->bpt_calib_num; i++) {
            dp->bpt_calib_table[0][i] = stat_info->dp_stat.defect_pixel_lut[0][i];
        }
        return TD_SUCCESS;
    }

    if (is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return dpc_striping_read_calib_num(blk_num, dp, stat_info);
    }
    return TD_SUCCESS;
}

static td_void dpc_calib_time_out(ot_vi_pipe vi_pipe, isp_defect_pixel *dp)
{
    printf("BAD PIXEL CALIBRATION TIME OUT  0x%x\n", dp->trial_cnt_limit);
    dp->sta_calibration_en = TD_FALSE;
    ot_ext_system_dpc_static_calib_enable_write(vi_pipe, TD_FALSE);
    ot_ext_system_dpc_finish_thresh_write(vi_pipe, dp->static_dp_thresh);
    ot_ext_system_dpc_trigger_status_write(vi_pipe, 0x2);
}

static td_void dpc_calib_max(td_u16 bad_pixels_count, isp_defect_pixel *dp, td_u8 static_type)
{
    printf("BAD_PIXEL_COUNT_UPPER_LIMIT 0x%x, 0x%x\n", dp->static_dp_thresh, bad_pixels_count);
    dp->frame_cnt = 0x2;
    dp->trial_count++;

    if (static_type == DPC_STATIC_HOT) {
        if (dp->static_dp_thresh == DPC_STATIC_THR_MAX) {
            dp->trial_count = dp->trial_cnt_limit;
        } else {
            dp->static_dp_thresh++;
        }
    } else if (static_type == DPC_STATIC_DARK) {
        if (dp->static_dp_thresh == 0) {
            dp->trial_count = dp->trial_cnt_limit;
        } else {
            dp->static_dp_thresh--;
        }
    }
}

static td_void dpc_calib_min(td_u16 bad_pixels_count, isp_defect_pixel *dp, td_u8 static_type)
{
    printf("BAD_PIXEL_COUNT_LOWER_LIMIT 0x%x, 0x%x\n", dp->static_dp_thresh, bad_pixels_count);
    dp->frame_cnt = 0x2;
    dp->trial_count++;

    if (static_type == DPC_STATIC_HOT) {
        if (dp->static_dp_thresh == 0) {
            dp->trial_count = dp->trial_cnt_limit;
        } else {
            dp->static_dp_thresh--;
        }
    } else if (static_type == DPC_STATIC_DARK) {
        if (dp->static_dp_thresh == DPC_STATIC_THR_MAX) {
            dp->trial_count = dp->trial_cnt_limit;
        } else {
            dp->static_dp_thresh++;
        }
    }
}

static td_u16 sorting_dp_calib_lut(td_u32 *lut0, size_t bpt_size, td_u32 *lut1, td_u16 cnt0, td_u16 cnt1)
{
    td_s32 ret;
    td_u16 i = 0;
    td_u16 j = 0;
    td_u16 cnt_sum = 0;
    td_u32 *temp_lut = TD_NULL;

    temp_lut = (td_u32 *)isp_malloc((cnt0 + cnt1) * sizeof(td_u32));
    if (temp_lut == TD_NULL) {
        return TD_FAILURE;
    }

    while ((i < cnt0) && (j < cnt1)) {
        if (lut0[i] > (lut1[j])) {
            temp_lut[cnt_sum++] = lut1[j++];
        } else if (lut0[i] < (lut1[j])) {
            temp_lut[cnt_sum++] = lut0[i++];
        } else {
            temp_lut[cnt_sum++] = lut0[i];
            i++;
            j++;
        }
    }

    if (i >= cnt0) {
        while (j < cnt1) {
            temp_lut[cnt_sum++] = lut1[j++];
        }
    }

    if (j >= cnt1) {
        while (i < cnt0) {
            temp_lut[cnt_sum++] = lut0[i++];
        }
    }
    ret = memcpy_s(lut0, bpt_size, temp_lut, cnt_sum * sizeof(td_u32));
    if (ret != EOK) {
        isp_err_trace("lut0 size:%lu is larger than temp_lut size:%lu\n", bpt_size, cnt_sum * sizeof(td_u32));
        cnt_sum = 0;
    }

    isp_free(temp_lut);

    return cnt_sum;
}

static td_s32 merging_dp_calib_lut(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, td_u8 blk_num)
{
    td_u8 k;
    td_u16 i;
    td_u16 bp_num;
    td_s32 ret;
    td_u16 cnt_temp;
    size_t bpt_size;
    td_u32 *bp_table = TD_NULL;

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(dp->bpt_calib_table[k]);
    }

    bp_table = (td_u32 *)isp_malloc(dp->bpt_calib_num * sizeof(td_u32));
    if (bp_table == TD_NULL) {
        return TD_FAILURE;
    }
    bpt_size = dp->bpt_calib_num * sizeof(td_u32);
    (td_void)memset_s(bp_table, bpt_size, 0, bpt_size);
    ret = memcpy_s(bp_table, dp->blk_bp_calib_num[0] * sizeof(td_u32),
                   dp->bpt_calib_table[0], dp->blk_bp_calib_num[0] * sizeof(td_u32));
    if (ret != EOK) {
        isp_free(bp_table);
        return TD_FAILURE;
    }
    bp_num = dp->blk_bp_calib_num[0];

    for (k = 1; k < blk_num; k++) {
        cnt_temp = sorting_dp_calib_lut(bp_table, bpt_size, dp->bpt_calib_table[k], bp_num, dp->blk_bp_calib_num[k]);
        if (cnt_temp == 0) {
            isp_free(bp_table);
            return TD_FAILURE;
        }

        bp_num = cnt_temp;
    }

    for (i = 0; i < dp->bpt_calib_num; i++) {
        ot_ext_system_dpc_calib_bpt_write(vi_pipe, i, bp_table[i]);
    }

    isp_free(bp_table);

    return TD_SUCCESS;
}

static td_void dpc_calib_success(ot_vi_pipe vi_pipe, td_u8 blk_num, isp_defect_pixel *dp)
{
    td_u16 j;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    printf("trial: 0x%x, findshed: 0x%x\n", dp->trial_count, dp->bpt_calib_num);

    ot_ext_system_dpc_bpt_calib_number_write(vi_pipe, dp->bpt_calib_num);

    if (is_online_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        isp_dpc_bpt_raddr_write(vi_pipe, 0, 0);

        for (j = 0; j < dp->bpt_calib_num; j++) {
            ot_ext_system_dpc_calib_bpt_write(vi_pipe, j, isp_dpc_bpt_rdata_read(vi_pipe, 0));
        }
    } else if (is_offline_mode(isp_ctx->block_attr.running_mode)) {
        isp_check_pointer_void_return(dp->bpt_calib_table[0]);
        for (j = 0; j < dp->bpt_calib_num; j++) {
            ot_ext_system_dpc_calib_bpt_write(vi_pipe, j, dp->bpt_calib_table[0][j]);
        }
    } else {
        merging_dp_calib_lut(vi_pipe, dp, blk_num);
    }

    dp->stat_en = 0;
    dp->sta_calibration_en = TD_FALSE;
    ot_ext_system_dpc_static_calib_enable_write(vi_pipe, TD_FALSE);
    ot_ext_system_dpc_finish_thresh_write(vi_pipe, dp->static_dp_thresh);
    ot_ext_system_dpc_trigger_status_write(vi_pipe, 0x1);
}

static td_void set_read_dp_statis_key(ot_vi_pipe vi_pipe, td_bool is_start)
{
    td_u32 isr_access;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) ||
        is_striping_mode(isp_ctx->block_attr.running_mode)) {
        isr_access = ot_ext_system_statistics_ctrl_highbit_read(vi_pipe);

        if (is_start == TD_TRUE) {
            isr_access |= (1 << DP_STAT_KEY_BIT);
        } else {
            isr_access &= (~(1 << DP_STAT_KEY_BIT));
        }

        ot_ext_system_statistics_ctrl_highbit_write(vi_pipe, isr_access);
    }
}

static td_void dpc_hot_calib(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, td_u8 blk_num, isp_stat *stat_info)
{
    if (dp->frame_cnt < 0x9) {
        if (dp->frame_cnt == 0) {
            ot_ext_system_dpc_trigger_status_write(vi_pipe, OT_ISP_STATE_INIT);
            dp_enter(vi_pipe, dp);
        }

        dp->frame_cnt++;

        if (dp->frame_cnt == 0x4) {
            dp->dpcc_bad_thresh = ((td_u32)dp->static_dp_thresh << 0x18) + (dp->hot_dev_thresh << 0x10) + 0x00000080;
            dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_HOT_MODE, 0, dp->cmos_dpc.enable);
            dp->stat_en = 1;

            set_read_dp_statis_key(vi_pipe, TD_TRUE);
        }

        /* calibrate frame 5 */
        if (dp->frame_cnt == ISP_DPC_CALIB_FRAME_CNT_FOR_READ) {
            dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_MODE, 0, dp->cmos_dpc.enable);
            dp->stat_en = 0;
            dpc_read_calib_num(vi_pipe, blk_num, dp, stat_info);
        }

        if (dp->frame_cnt == (ISP_DPC_CALIB_FRAME_CNT_FOR_READ + 1)) {
            set_read_dp_statis_key(vi_pipe, TD_FALSE);

            if (dp->trial_count >= dp->trial_cnt_limit) { /* TIMEOUT */
                dpc_calib_time_out(vi_pipe, dp);
                dp_exit(vi_pipe, dp);
            } else if (dp->bpt_calib_num >= dp->dp_count_max) {
                dpc_calib_max(dp->bpt_calib_num, dp, DPC_STATIC_HOT);
            } else if (dp->bpt_calib_num < dp->dp_count_min) {
                dpc_calib_min(dp->bpt_calib_num, dp, DPC_STATIC_HOT);
            } else { /* SUCCESS */
                dpc_calib_success(vi_pipe, blk_num, dp);
                dp_exit(vi_pipe, dp);
            }
        }
    }
}

static td_void dpc_dark_calib(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, td_u8 blk_num, isp_stat *stat_info)
{
    if (dp->frame_cnt < 0x9) {
        if (dp->frame_cnt == 0) {
            ot_ext_system_dpc_trigger_status_write(vi_pipe, OT_ISP_STATE_INIT);
            dp->calib_started = 1;
            dp->static_dp_thresh = ot_ext_system_dpc_start_thresh_read(vi_pipe);
        }

        dp->frame_cnt++;

        if (dp->frame_cnt == 0x4) {
            dp->dpcc_bad_thresh = 0xFF800000 + ((td_u32)dp->static_dp_thresh << 0x8) + dp->dead_dev_thresh;
            dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_DEAD_MODE, 0, dp->cmos_dpc.enable);
            dp->stat_en = 1;

            set_read_dp_statis_key(vi_pipe, TD_TRUE);
        }

        if (dp->frame_cnt == ISP_DPC_CALIB_FRAME_CNT_FOR_READ) {
            dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_MODE, 0, dp->cmos_dpc.enable);
            dp->stat_en = 0;
            dpc_read_calib_num(vi_pipe, blk_num, dp, stat_info);
        }

        if (dp->frame_cnt == (ISP_DPC_CALIB_FRAME_CNT_FOR_READ + 1)) {
            set_read_dp_statis_key(vi_pipe, TD_FALSE);

            if (dp->trial_count >= dp->trial_cnt_limit) {
                dpc_calib_time_out(vi_pipe, dp);
                dp_exit(vi_pipe, dp);
            } else if (dp->bpt_calib_num >= dp->dp_count_max) {
                dpc_calib_max(dp->bpt_calib_num, dp, DPC_STATIC_DARK);
            } else if (dp->bpt_calib_num < dp->dp_count_min) {
                dpc_calib_min(dp->bpt_calib_num, dp, DPC_STATIC_DARK);
            } else {
                dpc_calib_success(vi_pipe, blk_num, dp);
                dp_exit(vi_pipe, dp);
            }
        }
    }
}

static td_void isp_dpc_static_malloc(td_u8 blk_num, isp_defect_pixel *dp)
{
    td_u8 i;

    for (i = 0; i < blk_num; i++) {
        if (dp->bpt_calib_table[i] == TD_NULL) {
            dp->bpt_calib_table[i] = (td_u32 *)isp_malloc(ISP_DPC_STATIC_TAB_SIZE);

            if (dp->bpt_calib_table[i] == TD_NULL) {
                isp_err_trace("malloc dpc calibration table buffer failed\n");
                dpc_safe_free_calib_lut(dp, i);
                return;
            }
        }
        (td_void)memset_s(dp->bpt_calib_table[i], ISP_DPC_STATIC_TAB_SIZE, 0, ISP_DPC_STATIC_TAB_SIZE);
    }
}

static td_void isp_dpc_static_calibration(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, td_u8 blk_num, isp_stat *stat_info)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if ((dp->pixel_detect_type != 1) && (dp->pixel_detect_type != 0)) {
        isp_err_trace("invalid static defect pixel detect type!\n");
        return;
    }

    if (dp->calib_started == 0) {
        if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
            isp_dpc_static_malloc(blk_num, dp);
        }
    }

    isp_ctx->linkage.defect_pixel = TD_TRUE;

    if (dp->pixel_detect_type == 0) {
        dpc_hot_calib(vi_pipe, dp, blk_num, stat_info);
    } else {
        dpc_dark_calib(vi_pipe, dp, blk_num, stat_info);
    }

    if (dp->calib_finished == TD_TRUE) {
        if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
            dpc_safe_free_calib_lut(dp, blk_num);
        }
    }
}

static td_void isp_dpc_calib_mode(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, isp_reg_cfg *reg_cfg, isp_stat *stat_info)
{
    td_u8 i;

    dp_read_static_calib_extregs(vi_pipe);
    isp_dpc_static_calibration(vi_pipe, dp, reg_cfg->cfg_num, stat_info);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpcc_bad_thresh = dp->dpcc_bad_thresh;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpcc_mode[0] = dp->dpcc_mode;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpcc_mode[1] = dp->dpcc_mode;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpc_stat_en = dp->stat_en;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
    }
}

static td_void dp_read_extregs(ot_vi_pipe vi_pipe)
{
    td_u16 i, j;
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);

    dp->static_attr_update = ot_ext_system_dpc_static_attr_update_read(vi_pipe);
    if (dp->static_attr_update) {
        ot_ext_system_dpc_static_attr_update_write(vi_pipe, TD_FALSE);
        dp->bpt_cor_num = ot_ext_system_dpc_bpt_cor_number_read(vi_pipe);
        dp->staic_show = ot_ext_system_dpc_static_dp_show_read(vi_pipe);
    }

    dp->dyna_attr_update_en = ot_ext_system_dpc_dynamic_attr_update_read(vi_pipe);
    if (dp->dyna_attr_update_en) {
        ot_ext_system_dpc_dynamic_attr_update_write(vi_pipe, TD_FALSE);

        for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
            ot_isp_dp_frame_dynamic_attr *frame_dynamic = &(dp->cmos_dpc.frame_dynamic[i]);

            frame_dynamic->op_type = ot_ext_system_dpc_dynamic_manual_enable_read(vi_pipe, i);
            for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
                td_u8 index = i * OT_ISP_AUTO_ISO_NUM + j;
                frame_dynamic->auto_attr.strength[j] = ot_ext_system_dpc_dynamic_strength_table_read(vi_pipe, index);
                frame_dynamic->auto_attr.blend_ratio[j] =
                    ot_ext_system_dpc_dynamic_blend_ratio_table_read(vi_pipe, index);
            }

            frame_dynamic->manual_attr.blend_ratio = ot_ext_system_dpc_dynamic_blend_ratio_read(vi_pipe, i);
            frame_dynamic->manual_attr.strength = ot_ext_system_dpc_dynamic_strength_read(vi_pipe, i);
            frame_dynamic->sup_twinkle_en = ot_ext_system_dpc_suppress_twinkle_enable_read(vi_pipe, i);
            frame_dynamic->soft_thr = ot_ext_system_dpc_suppress_twinkle_thr_read(vi_pipe, i);
            frame_dynamic->soft_slope = ot_ext_system_dpc_suppress_twinkle_slope_read(vi_pipe, i);
            frame_dynamic->bright_strength = ot_ext_system_dpc_bright_strength_read(vi_pipe, i);
            frame_dynamic->dark_strength = ot_ext_system_dpc_dark_strength_read(vi_pipe, i);
        }
    }
}

static td_void split_dp_cor_lut(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, isp_reg_cfg *reg_cfg, td_u8 blk_num)
{
    td_s8 j;
    td_u16 bpt_num[OT_ISP_STRIPING_MAX_NUM] = {0};
    td_u16 i, x_value;
    td_u32 bpt_value;

    for (j = 0; j < blk_num; j++) {
        (td_void)memset_s(reg_cfg->alg_reg_cfg[j].dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.dpcc_bp_table,
                          ISP_DPC_STATIC_TAB_SIZE, 0, ISP_DPC_STATIC_TAB_SIZE);
    }

    for (j = (td_s8)blk_num - 1; j >= 0; j--) {
        for (i = 0; i < dp->bpt_cor_num; i++) {
            bpt_value = ot_ext_system_dpc_cor_bpt_read(vi_pipe, i);

            x_value = bpt_value & 0x1FFF;

            if ((x_value >= (dp->offset_for_split[j])) && (x_value < dp->offset_for_split[j + 1])) {
                reg_cfg->alg_reg_cfg[j].dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.dpcc_bp_table[bpt_num[j]] =
                        (bpt_value - dp->offset[j]);
                bpt_num[j]++;
            }

            if (bpt_num[j] >= OT_ISP_STATIC_DP_COUNT_NORMAL) {
                break;
            }
        }
    }

    for (j = 0; j < (td_s8)blk_num; j++) {
        reg_cfg->alg_reg_cfg[j].dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.dpcc_bpt_number = bpt_num[j];
        reg_cfg->alg_reg_cfg[j].dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.resh = TD_TRUE;
        reg_cfg->alg_reg_cfg[j].dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.update_index += 1;
    }
}

static td_u16 calc_rake_ratio(td_s32 x0, td_s32 y0, td_s32 x1, td_s32 y1, td_u32 shift)
{
    if (x0 == x1) {
        return 0;
    } else {
        return (td_u16)((y1 - y0) * (td_s32)(1u << shift)) / div_0_to_1(x1 - x0);
    }
}

static td_void dpc_usr_cfg(ot_vi_pipe vi_pipe, isp_defect_pixel *dp, isp_reg_cfg *reg_cfg)
{
    td_u8 i, j;
    isp_dpc_usr_dyna_cor_cfg *dyna_cfg = TD_NULL;

    if (dp->static_attr_update) {
        if (dp->staic_show || dp->static_enable) {
            split_dp_cor_lut(vi_pipe, dp, reg_cfg, reg_cfg->cfg_num);
        }
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        dyna_cfg = &reg_cfg->alg_reg_cfg[i].dp_reg_cfg.usr_reg_cfg.usr_dyna_cor_reg_cfg;

        for (j = 0; j < ISP_DPC_MAX_CHN_NUM; j++) {
            td_s8 soft_thr = dp->cmos_dpc.frame_dynamic[j].soft_thr;
            td_u8 soft_slope = dp->cmos_dpc.frame_dynamic[j].soft_slope;
            dyna_cfg->hard_thr_en[j] = dp->sup_twinkle_adapt[j] ? (TD_FALSE) : (TD_TRUE);
            dyna_cfg->sup_twinkle_thr_max[j] = soft_thr;
            dyna_cfg->sup_twinkle_thr_min[j] = clip3(dyna_cfg->sup_twinkle_thr_max[j] - soft_slope,
                                                     DPC_TWINKLE_THR_MIN, DPC_TWINKLE_THR_MAX);
            dyna_cfg->rake_ratio[j] = calc_rake_ratio(dyna_cfg->sup_twinkle_thr_min[j], 0,
                dyna_cfg->sup_twinkle_thr_max[j], 0x80, 0x8);
        }
        dyna_cfg->resh = TD_TRUE;
    }
}

static td_void dpc_res_switch(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    isp_defect_pixel *dp = TD_NULL;

    dp_get_ctx(vi_pipe, dp);
    isp_check_pointer_void_return(dp);

    dpc_image_size(vi_pipe, dp);

    dp->static_attr_update = TD_TRUE;
    dpc_usr_cfg(vi_pipe, dp, reg_cfg);
}

static td_void isp_dpc_show_mode(isp_reg_cfg *reg_cfg, isp_usr_ctx *isp_ctx)
{
    td_u8 i;

    isp_ctx->linkage.defect_pixel = TD_FALSE;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpcc_mode[0] = ISP_DPCC_HIGHLIGHT_MODE;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.dpcc_mode[1] = ISP_DPCC_HIGHLIGHT_MODE;
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
    }
}

static td_void soft_inter(isp_dpc_dyna_cfg *dpc_hw_cfg, td_u8 dpcc_stat, td_u8 i)
{
    td_u8 stat_idx_up, stat_idx_low;
    td_u8 stat_upper, stat_lower;
    td_u8 dpcc_line_thr_rb1, dpcc_line_thr_g1, dpcc_line_mad_fac_rb1, dpcc_line_mad_fac_g1;
    td_u8 j;

    stat_idx_up = ISP_DPC_SOFT_SLOPE_GRADE - 1;
    for (j = 0; j < ISP_DPC_SOFT_SLOPE_GRADE; j++) {
        if (dpcc_stat < g_soft_slope_grade[j]) {
            stat_idx_up = j;
            break;
        }
    }
    stat_idx_low = MAX2((td_s8)stat_idx_up - 1, 0);

    stat_upper = g_soft_slope_grade[stat_idx_up];
    stat_lower = g_soft_slope_grade[stat_idx_low];

    dpcc_line_thr_rb1 = (td_u8)linear_inter(dpcc_stat,
                                            stat_lower, (g_soft_line_thr[stat_idx_low] & 0xFF00) >> 0x8,
                                            stat_upper, (g_soft_line_thr[stat_idx_up]  & 0xFF00) >> 0x8);
    dpcc_line_thr_g1 = (td_u8)linear_inter(dpcc_stat,
                                           stat_lower, g_soft_line_thr[stat_idx_low] & 0xFF,
                                           stat_upper, g_soft_line_thr[stat_idx_up]  & 0xFF);

    dpcc_line_mad_fac_rb1 = (td_u8)linear_inter(dpcc_stat,
                                                stat_lower, (g_soft_line_mad_fac[stat_idx_low] & 0xFF00) >> 0x8,
                                                stat_upper, (g_soft_line_mad_fac[stat_idx_up]  & 0xFF00) >> 0x8);
    dpcc_line_mad_fac_g1 = (td_u8)linear_inter(dpcc_stat,
                                               stat_lower, g_soft_line_mad_fac[stat_idx_low] & 0xFF,
                                               stat_upper, g_soft_line_mad_fac[stat_idx_up]  & 0xFF);
    dpc_hw_cfg->dpcc_line_thr[i] = (((td_u16)dpcc_line_thr_rb1) << 0x8) + dpcc_line_thr_g1;
    dpc_hw_cfg->dpcc_line_mad_fac[i] = (((td_u16)(dpcc_line_mad_fac_rb1 & 0x3F)) << 0x8) +
                                   (dpcc_line_mad_fac_g1 & 0x3F);
}

static td_void set_dpcc_dec_param_inter(td_u8 dpcc_stat, td_u8 stat_idx_low, td_u8 stat_idx_up,
                                        isp_dpcc_derived_param *dpc_der_param)
{
    td_u8 i;
    td_u8 stat_upper, stat_lower;

    stat_upper = g_slope_grade[stat_idx_up];
    stat_lower = g_slope_grade[stat_idx_low];

    for (i = 0; i < ISP_DPC_COLOR_NUM; i++) {
        dpc_der_param->dpcc_line_thr[i] = (td_u8)linear_inter(dpcc_stat,
            stat_lower, g_dpc_der_param[stat_idx_low].dpcc_line_thr[i],
            stat_upper, g_dpc_der_param[stat_idx_up].dpcc_line_thr[i]);
        dpc_der_param->dpcc_line_mad_fac[i] = (td_u8)linear_inter(dpcc_stat,
            stat_lower, g_dpc_der_param[stat_idx_low].dpcc_line_mad_fac[i],
            stat_upper, g_dpc_der_param[stat_idx_up].dpcc_line_mad_fac[i]);
        dpc_der_param->dpcc_pg_fac[i] = (td_u8)linear_inter(dpcc_stat,
            stat_lower, g_dpc_der_param[stat_idx_low].dpcc_pg_fac[i],
            stat_upper, g_dpc_der_param[stat_idx_up].dpcc_pg_fac[i]);
        dpc_der_param->dpcc_rg_fac[i] = (td_u8)linear_inter(dpcc_stat,
            stat_lower, g_dpc_der_param[stat_idx_low].dpcc_rg_fac[i],
            stat_upper, g_dpc_der_param[stat_idx_up].dpcc_rg_fac[i]);
        dpc_der_param->dpcc_ro[i] = (td_u8)linear_inter(dpcc_stat,
            stat_lower, g_dpc_der_param[stat_idx_low].dpcc_ro[i],
            stat_upper, g_dpc_der_param[stat_idx_up].dpcc_ro[i]);
        dpc_der_param->dpcc_rg_fac_mtp[i] = ((dpc_der_param->dpcc_rg_fac[i] * ISP_DPC_RG_FAC_GAIN) >> 0x2);
    }
}

static td_void set_dpcc_parameters_inter(isp_dpc_dyna_cfg *isp_dpcc_hw_cfg, td_u8 dpcc_stat, td_u8 i)
{
    td_u8 j;
    td_u8 stat_idx_up, stat_idx_low;
    isp_dpcc_derived_param dpc_der_param;

    stat_idx_up = ISP_DPC_SLOPE_GRADE - 1;
    for (j = 0; j < ISP_DPC_SLOPE_GRADE; j++) {
        if (dpcc_stat < g_slope_grade[j]) {
            stat_idx_up = j;
            break;
        }
    }
    stat_idx_low = MAX2((td_s8)stat_idx_up - 1, 0);

    isp_dpcc_hw_cfg->dpcc_set_use[i] = g_dpc_def_cfg[stat_idx_low].dpcc_set_use;
    isp_dpcc_hw_cfg->dpcc_methods_set1[i] = g_dpc_def_cfg[stat_idx_low].dpcc_methods_set1;
    isp_dpcc_hw_cfg->dpcc_bad_thresh   = g_dpc_def_cfg[stat_idx_low].dpcc_bad_thresh;

    set_dpcc_dec_param_inter(dpcc_stat, stat_idx_low, stat_idx_up, &dpc_der_param);

    isp_dpcc_hw_cfg->dpcc_line_thr[i] = ((td_u16)(dpc_der_param.dpcc_line_thr[0]) << 0x8) +
        (dpc_der_param.dpcc_line_thr[1]);
    isp_dpcc_hw_cfg->dpcc_line_mad_fac[i] = ((td_u16)(dpc_der_param.dpcc_line_mad_fac[0] & 0x3F) << 0x8) +
        (dpc_der_param.dpcc_line_mad_fac[1] & 0x3F);
    isp_dpcc_hw_cfg->dpcc_pg_fac[i] = ((td_u16)(dpc_der_param.dpcc_pg_fac[0] & 0xFF) << 0x8) +
        (dpc_der_param.dpcc_pg_fac[1] & 0xFF);
    isp_dpcc_hw_cfg->dpcc_rg_fac[i] = ((td_u16)(dpc_der_param.dpcc_rg_fac[0] & 0xFF) << 0x8) +
        (dpc_der_param.dpcc_rg_fac[1] & 0xFF);
    isp_dpcc_hw_cfg->dpcc_rg_fac_mtp[i] = (((td_u32)dpc_der_param.dpcc_rg_fac_mtp[0] & 0x3FF) << 0xA) +
        (dpc_der_param.dpcc_rg_fac_mtp[1] & 0x3FF);

    isp_dpcc_hw_cfg->dpcc_ro_limits[i] = ((td_u16)(dpc_der_param.dpcc_ro[0] & 0x3) << 0x2) +
                                      (dpc_der_param.dpcc_ro[1] & 0x3);
}

static td_void isp_dynamic_cal_strength(td_u32 iso, ot_isp_dp_frame_dynamic_attr *frame_dynamic,
    td_u16 *strength, td_u16 *blend_ratio)
{
    td_u8 iso_index_upper, iso_index_lower;
    ot_isp_dp_dynamic_auto_attr *dpc_auto = &(frame_dynamic->auto_attr);

    iso_index_upper = get_iso_index(iso);
    iso_index_lower = MAX2((td_s8)iso_index_upper - 1, 0);

    if (frame_dynamic->op_type == OT_OP_MODE_MANUAL) {
        *strength = frame_dynamic->manual_attr.strength;
        *blend_ratio = frame_dynamic->manual_attr.blend_ratio;
    } else {
        *strength = (td_u16)linear_inter(iso,
                                         get_iso(iso_index_lower), (td_s32)dpc_auto->strength[iso_index_lower],
                                         get_iso(iso_index_upper), (td_s32)dpc_auto->strength[iso_index_upper]);

        *blend_ratio = (td_u16)linear_inter(iso,
                                            get_iso(iso_index_lower), (td_s32)dpc_auto->blend_ratio[iso_index_lower],
                                            get_iso(iso_index_upper), (td_s32)dpc_auto->blend_ratio[iso_index_upper]);
    }
}

static td_void isp_dynamic_set(td_u32 iso, isp_dpc_dyna_cfg *dpc_hw_cfg, isp_defect_pixel *dpc_fw_cfg, td_u8 i)
{
    td_u8 alpha1_rb, dpcc_stat;
    const td_u8 alpha1_g = 0; /* the blend ratio of input data and filtered result */
    td_u16 blend_ratio, strength;
    td_u16 dpcc_mode = dpc_fw_cfg->dpcc_mode;
    td_u16 dpcc_rg_fac_mtp0, dpcc_rg_fac_mtp1;
    ot_isp_dp_frame_dynamic_attr *frame_dynamic = &(dpc_fw_cfg->cmos_dpc.frame_dynamic[i]);

    isp_dynamic_cal_strength(iso, frame_dynamic, &strength, &blend_ratio);
    dpc_fw_cfg->actual_strength[i]    = strength;
    dpc_fw_cfg->actual_blend_ratio[i] = blend_ratio;

    dpcc_stat = strength >> 1;
    set_dpcc_parameters_inter(dpc_hw_cfg, dpcc_stat, i);
    if (dpcc_stat == 0) {
        dpcc_mode &= 0xFFFC;
    } else if (dpcc_stat >= ISP_DPC_HIGH_SLOP_OFFSET) {
        dpc_hw_cfg->dpcc_set_use[i] = 0x7;
        dpc_hw_cfg->dpcc_methods_set1[i] = 0x1f1f;
        dpc_hw_cfg->dpcc_line_thr[i] = g_dpc_high_der_parm[dpcc_stat - ISP_DPC_HIGH_SLOP_OFFSET].dpc_line_thr1;
        dpc_hw_cfg->dpcc_line_mad_fac[i] = g_dpc_high_der_parm[dpcc_stat - ISP_DPC_HIGH_SLOP_OFFSET].dpc_line_mad_fac1;
        dpc_hw_cfg->dpcc_pg_fac[i] = g_dpc_high_der_parm[dpcc_stat - ISP_DPC_HIGH_SLOP_OFFSET].dpc_pg_fac1;
        dpc_hw_cfg->dpcc_rg_fac[i] = g_dpc_high_der_parm[dpcc_stat - ISP_DPC_HIGH_SLOP_OFFSET].dpc_rg_fac1;
        dpc_hw_cfg->dpcc_ro_limits[i] = g_dpc_high_der_parm[dpcc_stat - ISP_DPC_HIGH_SLOP_OFFSET].dpc_ro_limits1;

        dpcc_rg_fac_mtp0 = (((dpc_hw_cfg->dpcc_rg_fac[i] & 0x0000FF00) >> 0x8) * ISP_DPC_RG_FAC_GAIN) >> 2; /* 2 */
        dpcc_rg_fac_mtp1 = ((dpc_hw_cfg->dpcc_rg_fac[i] & 0x000000FF) * ISP_DPC_RG_FAC_GAIN) >> 2; /* shift 2 */
        dpc_hw_cfg->dpcc_rg_fac_mtp[i] = (((td_u32)dpcc_rg_fac_mtp0 & 0x3FF) << 0xA) + (dpcc_rg_fac_mtp1 & 0x3FF);
    }
    dpc_hw_cfg->dpcc_amp_coef_k[i] = MAX2(0x0, 0xFF - (frame_dynamic->bright_strength << 1));
    dpc_hw_cfg->dpcc_amp_coef_min[i] = MIN2(0x7F, 0x7F - frame_dynamic->dark_strength);

    dpc_fw_cfg->sup_twinkle_adapt[i] = frame_dynamic->sup_twinkle_en;
    if (dpc_fw_cfg->sup_twinkle_adapt[i] == TD_TRUE) {
        if ((dpcc_stat == 0) || !((dpc_fw_cfg->dpcc_mode & 0x2) >> 1)) {
            dpc_fw_cfg->sup_twinkle_adapt[i] = TD_FALSE;
        } else {
            soft_inter(dpc_hw_cfg, dpcc_stat, i);
        }
    }

    if (!((dpcc_mode & 0x2) >> 1)) {
        blend_ratio = 0;
    }
    alpha1_rb = MIN2(blend_ratio, ISP_DPC_BLEND_RATIO_MAX_THR);
    dpc_hw_cfg->dpcc_alpha[i] = (((td_u16)alpha1_rb << 0x8)) + alpha1_g;
    dpc_hw_cfg->dpcc_mode[i] = dpcc_mode;
}

static td_void isp_dpc_normal_mode(isp_defect_pixel *dp, isp_reg_cfg *reg_cfg, isp_usr_ctx *isp_ctx)
{
    td_u8 i;

    isp_ctx->linkage.defect_pixel = TD_FALSE;

    dp->dpcc_mode = isp_dpc_calc_mode(ISP_DPCC_MODE, dp->static_enable, dp->cmos_dpc.enable);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        isp_dynamic_set(isp_ctx->linkage.iso, &reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg, dp, 0);
        isp_dynamic_set(isp_ctx->linkage.iso, &reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg, dp, 1);
        reg_cfg->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
    }
}

static td_s32 isp_dp_init(ot_vi_pipe vi_pipe, td_void *alg_reg_cfg)
{
    td_s32 ret;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)alg_reg_cfg;

    ot_ext_system_isp_dpc_init_status_write(vi_pipe, TD_FALSE);
    ret = dp_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = dp_initialize(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    dp_regs_initialize(vi_pipe, reg_cfg);
    dp_ext_regs_initialize(vi_pipe);

    ot_ext_system_isp_dpc_init_status_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

static td_s32 isp_dp_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *alg_reg_cfg)
{
    td_u8 i, j;
    td_s32 ret;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)alg_reg_cfg;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_defect_pixel *dp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    dp_get_ctx(vi_pipe, dp_ctx);
    isp_check_pointer_return(dp_ctx);

    if (dp_ctx->init == TD_TRUE) {
        isp_dp_enable_cfg(vi_pipe, reg_cfg->cfg_num, reg_cfg);

        if (isp_ctx->block_attr.block_num != isp_ctx->block_attr.pre_block_num) {
            dpc_res_switch(vi_pipe, reg_cfg);
        }
        reg_cfg->cfg_key.bit1_dp_cfg = 1;
        ret = isp_dpc_cmos_param_def(vi_pipe, dp_ctx);
        if (ret != TD_SUCCESS) {
            dp_ctx->init = TD_FALSE;
            ot_ext_system_isp_dpc_init_status_write(vi_pipe, dp_ctx->init);
            return ret;
        }

        ot_ext_system_dpc_dynamic_cor_enable_write(vi_pipe, dp_ctx->cmos_dpc.enable);
        for (i = 0; i < ISP_DPC_MAX_CHN_NUM; i++) {
            ot_isp_dp_frame_dynamic_attr *frame_dynamic = &(dp_ctx->cmos_dpc.frame_dynamic[i]);

            ot_ext_system_dpc_dynamic_manual_enable_write(vi_pipe, i, frame_dynamic->op_type);
            ot_ext_system_dpc_dynamic_strength_write(vi_pipe, i, frame_dynamic->manual_attr.strength);
            ot_ext_system_dpc_dynamic_blend_ratio_write(vi_pipe, i, frame_dynamic->manual_attr.blend_ratio);
            ot_ext_system_dpc_suppress_twinkle_enable_write(vi_pipe, i, frame_dynamic->sup_twinkle_en);
            ot_ext_system_dpc_suppress_twinkle_thr_write(vi_pipe, i, frame_dynamic->soft_thr);
            ot_ext_system_dpc_suppress_twinkle_slope_write(vi_pipe, i, frame_dynamic->soft_slope);
            ot_ext_system_dpc_bright_strength_write(vi_pipe, i, frame_dynamic->bright_strength);
            ot_ext_system_dpc_dark_strength_write(vi_pipe, i, frame_dynamic->dark_strength);

            for (j = 0; j < OT_ISP_AUTO_ISO_NUM; j++) {
                td_u8 index = i * OT_ISP_AUTO_ISO_NUM + j;
                ot_ext_system_dpc_dynamic_strength_table_write(vi_pipe, index, frame_dynamic->auto_attr.strength[j]);
                ot_ext_system_dpc_dynamic_blend_ratio_table_write(vi_pipe, index,
                    frame_dynamic->auto_attr.blend_ratio[j]);
            }
        }
        dp_ctx->init = TD_TRUE;
        ot_ext_system_isp_dpc_init_status_write(vi_pipe, dp_ctx->init);
        return TD_SUCCESS;
    } else {
        return isp_dp_init(vi_pipe, alg_reg_cfg);
    }
}

static td_s32 isp_dp_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *alg_reg_cfg, td_s32 rsv)
{
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_DP;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_defect_pixel *dp = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)alg_reg_cfg;

    ot_unused(rsv);
    isp_get_ctx(vi_pipe, isp_ctx);
    dp_get_ctx(vi_pipe, dp);
    isp_check_pointer_return(dp);

    if (isp_ctx->linkage.stat_ready == TD_FALSE) {
        return TD_SUCCESS;
    }

    ot_ext_system_isp_dpc_init_status_write(vi_pipe, dp->init);
    if (dp->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    reg_cfg->cfg_key.bit1_dp_cfg = 1;
    dp->sta_calibration_en = ot_ext_system_dpc_static_calib_enable_read(vi_pipe);
    dp->cmos_dpc.enable = ot_ext_system_dpc_dynamic_cor_enable_read(vi_pipe);
    dp->static_enable = ot_ext_system_dpc_static_cor_enable_read(vi_pipe);

    if ((dp->sta_calibration_en == TD_FALSE) && (dp->calib_started == 1)) { /* quit calibration */
        dp_exit(vi_pipe, dp);
        if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
            dpc_safe_free_calib_lut(dp, reg_cfg->cfg_num);
        }
    }

    if (dp->sta_calibration_en == TD_TRUE) { /* calibration mode */
        isp_dpc_calib_mode(vi_pipe, dp, reg_cfg, (isp_stat *)stat_info);

        return TD_SUCCESS;
    }

    dp_read_extregs(vi_pipe);

    if (dp->staic_show == TD_TRUE) { /* highlight static defect pixels mode */
        isp_dpc_show_mode(reg_cfg, isp_ctx);
    } else { /* normal detection and correction mode */
        isp_dpc_normal_mode(dp, reg_cfg, isp_ctx);
    }

    dpc_usr_cfg(vi_pipe, dp, reg_cfg);

    return TD_SUCCESS;
}

static td_void dpc_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_defect_pixel *dp_ctx = TD_NULL;

    dp_get_ctx(vi_pipe, dp_ctx);
    isp_check_pointer_void_return(dp_ctx);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "dpc info");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%10s" "%15s" "%15s" "%18s" "%18s\n",
                    "enable", "strength[0]", "strength[1]", "blend_ratio[0]", "blend_ratio[1]");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%10u" "%15u" "%15u" "%18u" "%18u\n",
                    dp_ctx->cmos_dpc.enable, dp_ctx->actual_strength[0], dp_ctx->actual_strength[1],
                    dp_ctx->actual_blend_ratio[0], dp_ctx->actual_blend_ratio[1]);

    proc->write_len += 1;
}

static td_s32 isp_dp_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_dp_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            dpc_res_switch(vi_pipe, &reg_cfg->reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            dpc_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_dp_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i, j;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_defect_pixel *dp = TD_NULL;
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_get_ctx(vi_pipe, isp_ctx);
    dp_get_ctx(vi_pipe, dp);

    ot_ext_system_isp_dpc_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        for (j = 0; j < ISP_DPC_MAX_CHN_NUM; j++) {
            reg_cfg->reg_cfg.alg_reg_cfg[i].dp_reg_cfg.dpc_en[j] = TD_FALSE;
        }
    }

    if (dp->sta_calibration_en == TD_TRUE) {
        if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
            dpc_safe_free_calib_lut(dp, reg_cfg->reg_cfg.cfg_num);
        }
    }

    reg_cfg->reg_cfg.cfg_key.bit1_dp_cfg = 1;

    dp_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_dpc(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_dp);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_DP;
    algs->alg_func.pfn_alg_init = isp_dp_init;
    algs->alg_func.pfn_alg_run  = isp_dp_run;
    algs->alg_func.pfn_alg_ctrl = isp_dp_ctrl;
    algs->alg_func.pfn_alg_exit = isp_dp_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
