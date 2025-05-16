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

typedef struct {
    td_bool init;
    td_bool coef_update_en;
    ot_isp_cac_acac_manual_attr acac_actual;
    ot_isp_cac_lcac_manual_attr lcac_actual;
    ot_isp_cac_attr mpi_cfg;
} isp_cac;


#define ISP_CAC_NORMFACTOR           12
#define ISP_CAC_HIGH_LIMIT_SHIFT     0
#define ISP_CAC_LOW_LIMIT_SHIFT      0

#define ISP_CAC_COUNT_THR_R_DEFAULT  1
#define ISP_CAC_COUNT_THR_G_DEFAULT  1
#define ISP_CAC_COUNT_THR_B_DEFAULT  1
#define ISP_CAC_DIFF_THD_DEFAULT     20

#define ISP_CAC_DETMODE_DEFAULT          1
#define ISP_CAC_CRCB_RATIO_HIGH_LIMIT    135
#define ISP_CAC_CRCB_RATIO_LOW_LIMIT     (-50)

#define ISP_CAC_PURPLE_DET_RANGE_DEFAULT 60
#define ISP_CAC_LUMA_THR_R_LINEAR        1500
#define ISP_CAC_LUMA_THR_G_LINEAR        1500
#define ISP_CAC_LUMA_THR_B_LINEAR        3500
#define ISP_CAC_LUMA_THR_R_WDR           1500
#define ISP_CAC_LUMA_THR_G_WDR           1500
#define ISP_CAC_LUMA_THR_B_WDR           2150
#define ISP_CAC_PURPLE_VAR_THR_DEFAULT   200

#define ISP_CAC_GLOBALSTR_DEFAULT      255
#define ISP_CAC_RB_STRENGTH_DEFAULT    10
#define ISP_CAC_PURPLE_ALPHA           63
#define ISP_CAC_EDGE_ALPHA             63
#define ISP_CAC_PURPLE_SATU_THD        100
#define ISP_CAC_PURPLE_SATU_HIGH_THD_LIN 16383
#define ISP_CAC_PURPLE_SATU_HIGH_THD_WDR 1000


#define ISP_CAC_DEPURPLE_CB_STR_LIN    3 /* [0,8] */
#define ISP_CAC_DEPURPLE_CR_STR_LIN    0 /* [0,8] */
#define ISP_CAC_DEPURPLE_CB_STR_WDR    7 /* [0,8] */
#define ISP_CAC_DEPURPLE_CR_STR_WDR    0 /* [0,8] */
#define ISP_CAC_DEPURPLE_MAX_STR       8
#define RANGE_MAX_VALUE                3

#define MIN_EXP_RATIO             64
#define MAX_EXP_RATIO             16384

#define CAC_LAMDA_PRECS           16
#define CAC_THDRB_PRECS           16
#define CAC_DISTANCE_PRECS        16

static const td_u16 g_cac_rb_thresh[OT_ISP_CAC_THR_NUM] = {50, 1000};
static const td_u16 g_cac_lamda_thresh[OT_ISP_CAC_THR_NUM] = {10, 300};
static const td_u16 g_cac_distance_thresh[OT_ISP_CAC_THR_NUM] = {100, 200};

static const td_u16 g_r_luma[OT_ISP_CAC_CURVE_NUM] = {1500, 1500, 0};
static const td_u16 g_g_luma[OT_ISP_CAC_CURVE_NUM] = {1500, 1500, 0};
static const td_u16 g_b_luma[OT_ISP_CAC_CURVE_NUM] = {4095, 1500, 0};
static const td_u16 g_purple_det_range[OT_ISP_CAC_CURVE_NUM] = {0, 260, 410};

static const td_u32 g_exp_ratio_lut[OT_ISP_CAC_EXP_RATIO_NUM] = {
    64, 128, 256, 512, 1024, 1536, 2048, 3072, 4096, 5120, 6144, 8192, 10240, 12288, 14336, 16384
};

static const td_u16  g_edge_thd[OT_ISP_CAC_THR_NUM][OT_ISP_AUTO_ISO_NUM] = {
    {100, 100, 100, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150},
    {500, 500, 500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600}
};
static const td_u16  g_cac_th_rb[OT_ISP_CAC_THR_NUM][OT_ISP_AUTO_ISO_NUM] = {
    {50,  50,  50,  50,  50,  50,  50,  60,  80, 100, 120, 150, 150, 150, 150, 150},
    {1000, 1000, 1000, 1000, 1000, 1000, 900, 900, 800, 600, 600, 600, 600, 600, 600, 600}
};
static const td_u16  g_distance_thd[OT_ISP_CAC_THR_NUM][OT_ISP_AUTO_ISO_NUM] = {
    {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
    {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200}
};

static const td_u16  g_edge_gain[OT_ISP_AUTO_ISO_NUM] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
static const td_u16  g_det_pur_sat_thd[OT_ISP_AUTO_ISO_NUM] = {
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
};
static const td_u16  g_det_pur_sat_high_thd_lin[OT_ISP_AUTO_ISO_NUM] = {
    16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383
};
static const td_u16  g_det_pur_sat_high_thd_wdr[OT_ISP_AUTO_ISO_NUM] = {
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};

static const td_u16  g_purple_alpha[OT_ISP_AUTO_ISO_NUM] = {
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63
};
static const td_u16  g_edge_alpha[OT_ISP_AUTO_ISO_NUM] = {
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63
};
static const td_u16  g_cac_rb_strength[OT_ISP_AUTO_ISO_NUM] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
};
static const td_u8  g_wdr_cb_strdef_lut[OT_ISP_CAC_EXP_RATIO_NUM] = {
    0, 0, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};
static const td_u8  g_wdr_cr_strdef_lut[OT_ISP_CAC_EXP_RATIO_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const td_u8  g_lin_cb_strdef_lut[OT_ISP_CAC_EXP_RATIO_NUM] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};
static const td_u8  g_lin_cr_strdef_lut[OT_ISP_CAC_EXP_RATIO_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

isp_cac *g_past_cac_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define cac_get_ctx(pipe, ctx)   ((ctx) = g_past_cac_ctx[pipe])
#define cac_set_ctx(pipe, ctx)   (g_past_cac_ctx[pipe] = (ctx))
#define cac_reset_ctx(pipe)      (g_past_cac_ctx[pipe] = TD_NULL)

static td_s32 cac_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_cac *past_cac_ctx = TD_NULL;
    cac_get_ctx(vi_pipe, past_cac_ctx);

    if (past_cac_ctx == TD_NULL) {
        past_cac_ctx = (isp_cac *)isp_malloc(sizeof(isp_cac));
        if (past_cac_ctx == TD_NULL) {
            isp_err_trace("isp[%d] cac_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(past_cac_ctx, sizeof(isp_cac), 0, sizeof(isp_cac));
    cac_set_ctx(vi_pipe, past_cac_ctx);

    return TD_SUCCESS;
}

static td_void cac_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_cac *past_cac_ctx = TD_NULL;
    cac_get_ctx(vi_pipe, past_cac_ctx);
    isp_free(past_cac_ctx);
    cac_reset_ctx(vi_pipe);
}

static td_s32 cac_check_cmos_param(const ot_isp_cac_attr *cac_attr)
{
    td_s32 ret;
    isp_check_bool_return(cac_attr->enable);
    ret = isp_cac_comm_attr_check("cmos", cac_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_cac_acac_attr_check("cmos", &cac_attr->acac_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_cac_lcac_attr_check("cmos", &cac_attr->lcac_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_void isp_cac_comm_attr_write(ot_vi_pipe vi_pipe, ot_isp_cac_attr *mpi_cfg)
{
    ot_ext_system_cac_enable_write(vi_pipe, mpi_cfg->enable);
    ot_ext_system_cac_op_type_write(vi_pipe, mpi_cfg->op_type);
    ot_ext_system_cac_det_mode_write(vi_pipe, mpi_cfg->detect_mode);
    ot_ext_system_cac_purple_upper_limit_write(vi_pipe, mpi_cfg->purple_upper_limit);
    ot_ext_system_cac_purple_lower_limit_write(vi_pipe, mpi_cfg->purple_lower_limit);
}

static td_void cac_ext_regs_init(ot_vi_pipe vi_pipe, isp_cac *cac_ctx)
{
    isp_cac_comm_attr_write(vi_pipe, &cac_ctx->mpi_cfg);
    isp_cac_acac_attr_write(vi_pipe, &cac_ctx->mpi_cfg.acac_cfg);
    isp_cac_lcac_attr_write(vi_pipe, &cac_ctx->mpi_cfg.lcac_cfg);
}

static td_void cac_static_regs_init(ot_vi_pipe vi_pipe, isp_cac_static_cfg *cac_static_reg_cfg)
{
    ot_unused(vi_pipe);
    cac_static_reg_cfg->local_cac_en          = TD_TRUE;
    cac_static_reg_cfg->cac_norm_factor       = ISP_CAC_NORMFACTOR;
    cac_static_reg_cfg->cac_upper_limit_shift = ISP_CAC_HIGH_LIMIT_SHIFT;
    cac_static_reg_cfg->cac_lower_limit_shift = ISP_CAC_LOW_LIMIT_SHIFT;
    cac_static_reg_cfg->r_counter_thr         = ISP_CAC_COUNT_THR_R_DEFAULT;
    cac_static_reg_cfg->g_counter_thr         = ISP_CAC_COUNT_THR_G_DEFAULT;
    cac_static_reg_cfg->b_counter_thr         = ISP_CAC_COUNT_THR_B_DEFAULT;
    cac_static_reg_cfg->diff_thd              = ISP_CAC_DIFF_THD_DEFAULT;
    cac_static_reg_cfg->static_resh           = TD_TRUE;
    return;
}

static td_void cac_usr_regs_init(isp_cac_usr_cfg *cac_usr_reg_cfg, td_u8 wdr_mode)
{
    cac_usr_reg_cfg->cac_det_mode           = ISP_CAC_DETMODE_DEFAULT;
    cac_usr_reg_cfg->cr_cb_ratio_high_limit = ISP_CAC_CRCB_RATIO_HIGH_LIMIT;
    cac_usr_reg_cfg->cr_cb_ratio_low_limit  = ISP_CAC_CRCB_RATIO_LOW_LIMIT;
    cac_usr_reg_cfg->var_thr                = ISP_CAC_PURPLE_VAR_THR_DEFAULT;
    if (is_linear_mode(wdr_mode)) {
        cac_usr_reg_cfg->r_luma_thr = ISP_CAC_LUMA_THR_R_LINEAR;
        cac_usr_reg_cfg->g_luma_thr = ISP_CAC_LUMA_THR_G_LINEAR;
        cac_usr_reg_cfg->b_luma_thr = ISP_CAC_LUMA_THR_B_LINEAR;
    } else {
        cac_usr_reg_cfg->r_luma_thr = ISP_CAC_LUMA_THR_R_WDR;
        cac_usr_reg_cfg->g_luma_thr = ISP_CAC_LUMA_THR_G_WDR;
        cac_usr_reg_cfg->b_luma_thr = ISP_CAC_LUMA_THR_B_WDR;
    }
    cac_usr_reg_cfg->usr_resh  = TD_TRUE;
    cac_usr_reg_cfg->update_index = 1;
    return;
}

static td_void cac_dyna_regs_init(isp_cac_dyna_cfg *cac_dyna_reg_cfg, td_u8 wdr_mode)
{
    td_u16 lamda_mul, edge_thdrb_mul, distance_mul;
    cac_dyna_reg_cfg->cac_edge_str = ISP_CAC_GLOBALSTR_DEFAULT;
    cac_dyna_reg_cfg->lamda_thd0   = g_cac_lamda_thresh[0];
    cac_dyna_reg_cfg->lamda_thd1   = g_cac_lamda_thresh[1];
    cac_dyna_reg_cfg->edge_thd0    = g_cac_rb_thresh[0];
    cac_dyna_reg_cfg->edge_thd1    = g_cac_rb_thresh[1];
    cac_dyna_reg_cfg->distance_thd0 = g_cac_distance_thresh[0];
    cac_dyna_reg_cfg->distance_thd1 = g_cac_distance_thresh[1];
    cac_dyna_reg_cfg->cac_tao = ISP_CAC_RB_STRENGTH_DEFAULT;
    cac_dyna_reg_cfg->purple_alpha = ISP_CAC_PURPLE_ALPHA;
    cac_dyna_reg_cfg->edge_alpha = ISP_CAC_EDGE_ALPHA;
    cac_dyna_reg_cfg->det_satu_thr = ISP_CAC_PURPLE_SATU_THD;

    if (is_linear_mode(wdr_mode)) {
        cac_dyna_reg_cfg->det_satu_thr_high = ISP_CAC_PURPLE_SATU_HIGH_THD_LIN;
        cac_dyna_reg_cfg->de_purple_ctr_cr = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CR_STR_LIN;
        cac_dyna_reg_cfg->de_purple_ctr_cb = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CB_STR_LIN;
    } else {
        cac_dyna_reg_cfg->det_satu_thr_high = ISP_CAC_PURPLE_SATU_HIGH_THD_WDR;
        cac_dyna_reg_cfg->de_purple_ctr_cr = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CR_STR_WDR;
        cac_dyna_reg_cfg->de_purple_ctr_cb = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CB_STR_WDR;
    }

    /* calculate by using default value */
    if (g_cac_lamda_thresh[0] == g_cac_lamda_thresh[1]) {
        lamda_mul = 0;
    } else {
        lamda_mul = (0x100 << CAC_LAMDA_PRECS) / (g_cac_lamda_thresh[1] - g_cac_lamda_thresh[0]);
    }
    if (g_cac_rb_thresh[0] == g_cac_rb_thresh[1]) {
        edge_thdrb_mul = 0;
    } else {
        edge_thdrb_mul = (1 << CAC_THDRB_PRECS) / (g_cac_rb_thresh[1] - g_cac_rb_thresh[0]);
    }
    if (g_cac_distance_thresh[0] == g_cac_distance_thresh[1]) {
        distance_mul = 0;
    } else {
        distance_mul = (64 << CAC_DISTANCE_PRECS) / (g_cac_distance_thresh[1] - g_cac_distance_thresh[0]); /* 64 */
    }
    cac_dyna_reg_cfg->lamda_mul    = lamda_mul;
    cac_dyna_reg_cfg->edge_thd_mul = edge_thdrb_mul;
    cac_dyna_reg_cfg->distance_mul = distance_mul;
    cac_dyna_reg_cfg->dyna_resh    = TD_TRUE;

    return;
}

static td_void cac_regs_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_cac *cac_ctx)
{
    td_u8 i;
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        cac_static_regs_init(vi_pipe, &reg_cfg->alg_reg_cfg[i].cac_reg_cfg.static_reg_cfg);
        cac_usr_regs_init(&reg_cfg->alg_reg_cfg[i].cac_reg_cfg.usr_reg_cfg, isp_ctx->sns_wdr_mode);
        cac_dyna_regs_init(&reg_cfg->alg_reg_cfg[i].cac_reg_cfg.dyna_reg_cfg, isp_ctx->sns_wdr_mode);
        reg_cfg->alg_reg_cfg[i].cac_reg_cfg.cac_en = cac_ctx->mpi_cfg.enable;
    }
    reg_cfg->cfg_key.bit1_cac_cfg = 1;
}

static td_void cac_ctx_def_comm_initialize(ot_isp_cac_attr *mpi_cfg)
{
    mpi_cfg->enable              = TD_TRUE;
    mpi_cfg->op_type             = OT_OP_MODE_AUTO;
    mpi_cfg->detect_mode         = ISP_CAC_DETMODE_DEFAULT;
    mpi_cfg->purple_upper_limit  = ISP_CAC_CRCB_RATIO_HIGH_LIMIT;
    mpi_cfg->purple_lower_limit  = ISP_CAC_CRCB_RATIO_LOW_LIMIT;
}

static td_void cac_ctx_def_acac_initialize(ot_isp_cac_acac_attr *acac_cfg, td_u8 wdr_mode)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        acac_cfg->acac_auto.edge_threshold[0][i]       = g_edge_thd[0][i];
        acac_cfg->acac_auto.edge_threshold[1][i]       = g_edge_thd[1][i];
        acac_cfg->acac_auto.edge_gain[i]               = g_edge_gain[i];
        acac_cfg->acac_auto.cac_rb_strength[i]         = g_cac_rb_strength[i];
        acac_cfg->acac_auto.purple_alpha[i]            = g_purple_alpha[i];
        acac_cfg->acac_auto.edge_alpha[i]              = g_edge_alpha[i];
        acac_cfg->acac_auto.satu_low_threshold[i]      = g_det_pur_sat_thd[i];
        if (is_linear_mode(wdr_mode)) {
            acac_cfg->acac_auto.satu_high_threshold[i] = g_det_pur_sat_high_thd_lin[i];
        } else {
            acac_cfg->acac_auto.satu_high_threshold[i] = g_det_pur_sat_high_thd_wdr[i];
        }
    }
    acac_cfg->acac_manual.edge_threshold[0]   = g_cac_lamda_thresh[0];
    acac_cfg->acac_manual.edge_threshold[1]   = g_cac_lamda_thresh[1];
    acac_cfg->acac_manual.edge_gain           = ISP_CAC_GLOBALSTR_DEFAULT;
    acac_cfg->acac_manual.cac_rb_strength     = ISP_CAC_RB_STRENGTH_DEFAULT;
    acac_cfg->acac_manual.purple_alpha        = ISP_CAC_PURPLE_ALPHA;
    acac_cfg->acac_manual.edge_alpha          = ISP_CAC_EDGE_ALPHA;
    acac_cfg->acac_manual.satu_low_threshold  = ISP_CAC_PURPLE_SATU_THD;
    if (is_linear_mode(wdr_mode)) {
        acac_cfg->acac_manual.satu_high_threshold = ISP_CAC_PURPLE_SATU_HIGH_THD_LIN;
    } else {
        acac_cfg->acac_manual.satu_high_threshold = ISP_CAC_PURPLE_SATU_HIGH_THD_WDR;
    }
}

static td_void cac_ctx_def_lcac_initialize(ot_isp_cac_lcac_attr *lcac_cfg, td_u8 wdr_mode)
{
    td_u8 i;
    lcac_cfg->purple_detect_range = ISP_CAC_PURPLE_DET_RANGE_DEFAULT;
    lcac_cfg->var_threshold       = ISP_CAC_PURPLE_VAR_THR_DEFAULT;
    for (i = 0; i < OT_ISP_CAC_CURVE_NUM; i++) {
        lcac_cfg->r_detect_threshold[i]  = g_r_luma[i];
        lcac_cfg->g_detect_threshold[i]  = g_g_luma[i];
        lcac_cfg->b_detect_threshold[i]  = g_b_luma[i];
    }

    for (i = 0; i < OT_ISP_CAC_EXP_RATIO_NUM; i++) {
        if (is_linear_mode(wdr_mode)) {
            lcac_cfg->lcac_auto.de_purple_cr_strength[i] = g_lin_cr_strdef_lut[i];
            lcac_cfg->lcac_auto.de_purple_cb_strength[i] = g_lin_cb_strdef_lut[i];
        } else {
            lcac_cfg->lcac_auto.de_purple_cr_strength[i] = g_wdr_cr_strdef_lut[i];
            lcac_cfg->lcac_auto.de_purple_cb_strength[i] = g_wdr_cb_strdef_lut[i];
        }
    }
    if (is_linear_mode(wdr_mode)) {
        lcac_cfg->lcac_manual.de_purple_cr_strength = ISP_CAC_DEPURPLE_CR_STR_LIN;
        lcac_cfg->lcac_manual.de_purple_cb_strength = ISP_CAC_DEPURPLE_CB_STR_LIN;
    } else {
        lcac_cfg->lcac_manual.de_purple_cr_strength = ISP_CAC_DEPURPLE_CR_STR_WDR;
        lcac_cfg->lcac_manual.de_purple_cb_strength = ISP_CAC_DEPURPLE_CB_STR_WDR;
    }
}

static td_s32 isp_cac_ctx_initialize(ot_vi_pipe vi_pipe, isp_cac *cac_ctx)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_sensor_get_default(vi_pipe, &sns_dft);
    if (sns_dft->key.bit1_cac) {
        isp_check_pointer_return(sns_dft->cac);

        ret = cac_check_cmos_param(sns_dft->cac);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        (td_void)memcpy_s(&cac_ctx->mpi_cfg, sizeof(ot_isp_cac_attr),
                          sns_dft->cac, sizeof(ot_isp_cac_attr));
    } else {
        cac_ctx_def_comm_initialize(&cac_ctx->mpi_cfg);
        cac_ctx_def_acac_initialize(&cac_ctx->mpi_cfg.acac_cfg, isp_ctx->sns_wdr_mode);
        cac_ctx_def_lcac_initialize(&cac_ctx->mpi_cfg.lcac_cfg, isp_ctx->sns_wdr_mode);
    }

    return TD_SUCCESS;
}

static td_void cac_read_ext_regs(ot_vi_pipe vi_pipe, isp_cac *cac_ctx)
{
    cac_ctx->coef_update_en = ot_ext_system_cac_coef_update_read(vi_pipe);
    if (cac_ctx->coef_update_en != TD_TRUE) {
        return;
    }

    ot_ext_system_cac_coef_update_write(vi_pipe, TD_FALSE);
    cac_ctx->mpi_cfg.enable              = ot_ext_system_cac_enable_read(vi_pipe);
    cac_ctx->mpi_cfg.op_type             = ot_ext_system_cac_op_type_read(vi_pipe);
    cac_ctx->mpi_cfg.detect_mode         = ot_ext_system_cac_det_mode_read(vi_pipe);
    cac_ctx->mpi_cfg.purple_upper_limit  = (td_s16)ot_ext_system_cac_purple_upper_limit_read(vi_pipe);
    cac_ctx->mpi_cfg.purple_lower_limit  = (td_s16)ot_ext_system_cac_purple_lower_limit_read(vi_pipe);

    isp_cac_acac_attr_read(vi_pipe, &cac_ctx->mpi_cfg.acac_cfg);
    isp_cac_lcac_attr_read(vi_pipe, &cac_ctx->mpi_cfg.lcac_cfg);
}

static td_void cac_dyn_reg_update(ot_vi_pipe vi_pipe, td_u32 iso, isp_cac_dyna_cfg *dyn_cfg, isp_cac *cac_ctx)
{
    td_u16 lamda_mul, edge_thd_mul, distance_mul;
    td_u8 iso_index_up, iso_index_low;
    td_u32 iso1, iso2;
    ot_unused(vi_pipe);

    iso_index_up  = get_iso_index(iso);
    iso_index_low = MAX2((td_s8)iso_index_up - 1, 0);
    iso1          = get_iso(iso_index_low);
    iso2          = get_iso(iso_index_up);

    dyn_cfg->cac_edge_str  = cac_ctx->acac_actual.edge_gain;
    dyn_cfg->lamda_thd0    = cac_ctx->acac_actual.edge_threshold[0];
    dyn_cfg->lamda_thd1    = cac_ctx->acac_actual.edge_threshold[1];
    dyn_cfg->edge_thd0 = (td_u16)linear_inter(iso, iso1, g_cac_th_rb[0][iso_index_low],
                                              iso2, g_cac_th_rb[0][iso_index_up]);
    dyn_cfg->edge_thd1 = (td_u16)linear_inter(iso, iso1, g_cac_th_rb[1][iso_index_low],
                                              iso2, g_cac_th_rb[1][iso_index_up]);
    dyn_cfg->distance_thd0 = (td_u16)linear_inter(iso, iso1, g_distance_thd[0][iso_index_low],
                                              iso2, g_distance_thd[0][iso_index_up]);
    dyn_cfg->distance_thd1 = (td_u16)linear_inter(iso, iso1, g_distance_thd[1][iso_index_low],
                                              iso2, g_distance_thd[1][iso_index_up]);
    dyn_cfg->cac_tao       = cac_ctx->acac_actual.cac_rb_strength;
    dyn_cfg->purple_alpha  = cac_ctx->acac_actual.purple_alpha;
    dyn_cfg->edge_alpha    = cac_ctx->acac_actual.edge_alpha;
    dyn_cfg->det_satu_thr  = cac_ctx->acac_actual.satu_low_threshold;
    dyn_cfg->det_satu_thr_high = cac_ctx->acac_actual.satu_high_threshold;
    dyn_cfg->de_purple_ctr_cr  = ISP_CAC_DEPURPLE_MAX_STR - cac_ctx->lcac_actual.de_purple_cr_strength;
    dyn_cfg->de_purple_ctr_cb  = ISP_CAC_DEPURPLE_MAX_STR - cac_ctx->lcac_actual.de_purple_cb_strength;

    if (dyn_cfg->lamda_thd1 <= dyn_cfg->lamda_thd0) {
        lamda_mul = 0;
    } else {
        lamda_mul = (0x100 << CAC_LAMDA_PRECS) / (dyn_cfg->lamda_thd1 - dyn_cfg->lamda_thd0);
    }

    if (dyn_cfg->edge_thd1 <= dyn_cfg->edge_thd0) {
        edge_thd_mul = 0;
    } else {
        edge_thd_mul = (1 << CAC_THDRB_PRECS) / (dyn_cfg->edge_thd1 - dyn_cfg->edge_thd0);
    }

    if (dyn_cfg->distance_thd1 <= dyn_cfg->distance_thd0) {
        distance_mul = 0;
    } else {
        distance_mul = (64 << CAC_DISTANCE_PRECS) / (dyn_cfg->distance_thd1 - dyn_cfg->distance_thd0); /* 64 */
    }
    dyn_cfg->lamda_mul = lamda_mul;
    dyn_cfg->edge_thd_mul = edge_thd_mul;
    dyn_cfg->distance_mul = distance_mul;

    dyn_cfg->dyna_resh    = TD_TRUE;
}

static td_s32 cac_get_purple_det_idx(td_u16 purple_det_range)
{
    td_s32 idx;
    for (idx = 0; idx < (RANGE_MAX_VALUE - 1); idx++) {
        if (purple_det_range < g_purple_det_range[idx]) {
            break;
        }
    }
    return idx;
}

static td_void cac_user_reg_update(ot_vi_pipe vi_pipe, td_u32 iso, isp_cac_usr_cfg *user_cfg)
{
    ot_unused(iso);

    isp_cac *cac_ctx;
    cac_get_ctx(vi_pipe, cac_ctx);
    ot_isp_cac_lcac_attr *lcac_attr = &cac_ctx->mpi_cfg.lcac_cfg;

    td_u16 pdet_range = lcac_attr->purple_detect_range;
    td_s32 r_low, r_up, range_idx_up, range_idx_low;
    range_idx_up  = cac_get_purple_det_idx(pdet_range);
    range_idx_low = MAX2(range_idx_up - 1, 0);
    r_up          = g_purple_det_range[range_idx_up];
    r_low         = g_purple_det_range[range_idx_low];

    user_cfg->cac_det_mode           = cac_ctx->mpi_cfg.detect_mode;
    user_cfg->cr_cb_ratio_high_limit = cac_ctx->mpi_cfg.purple_upper_limit;
    user_cfg->cr_cb_ratio_low_limit  = cac_ctx->mpi_cfg.purple_lower_limit;
    user_cfg->var_thr                = lcac_attr->var_threshold;
    user_cfg->r_luma_thr = (td_u16)linear_inter(pdet_range, r_low, lcac_attr->r_detect_threshold[range_idx_low],
                                                r_up, lcac_attr->r_detect_threshold[range_idx_up]);
    user_cfg->g_luma_thr = (td_u16)linear_inter(pdet_range, r_low, lcac_attr->g_detect_threshold[range_idx_low],
                                                r_up, lcac_attr->g_detect_threshold[range_idx_up]);
    user_cfg->b_luma_thr = (td_u16)linear_inter(pdet_range, r_low, lcac_attr->b_detect_threshold[range_idx_low],
                                                r_up, lcac_attr->b_detect_threshold[range_idx_up]);
    user_cfg->usr_resh       = TD_TRUE;
    user_cfg->update_index  += 1;
}

static td_u32 cac_get_exp_ratio(ot_vi_pipe vi_pipe)
{
    td_u32 exp_ratio;
    td_u8 wdr_mode;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    exp_ratio = isp_ctx->linkage.exp_ratio;
    wdr_mode = isp_ctx->sns_wdr_mode;

    if (is_linear_mode(wdr_mode)) {
        return MIN_EXP_RATIO;
    } else if (is_built_in_wdr_mode(wdr_mode)) {
        return MAX_EXP_RATIO;
    } else if (is_2to1_wdr_mode(wdr_mode) || is_3to1_wdr_mode(wdr_mode)) {
        if ((isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) ||
            (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE)) {
            return MIN_EXP_RATIO;
        } else {
            return exp_ratio;
        }
    } else {
        return MIN_EXP_RATIO;
    }
}

static td_u8 cac_get_ratio_idx(td_u32 ratio)
{
    td_u8 idx;
    for (idx = 0; idx < OT_ISP_CAC_EXP_RATIO_NUM - 1; idx++) {
        if (ratio <= g_exp_ratio_lut[idx]) {
            break;
        }
    }
    return idx;
}

static td_void isp_cac_get_mpi_auto_inter_result(ot_vi_pipe vi_pipe, td_u32 iso, isp_cac *cac_ctx)
{
    td_u8  iso_index_up, iso_index_low;
    td_u32 iso1, iso2;
    td_u32 ratio;
    td_u8  ratio_h, ratio_l;
    ot_isp_cac_acac_auto_attr   *acac_auto   = &cac_ctx->mpi_cfg.acac_cfg.acac_auto;
    ot_isp_cac_lcac_auto_attr   *lcac_auto   = &cac_ctx->mpi_cfg.lcac_cfg.lcac_auto;
    ot_isp_cac_acac_manual_attr *acac_actual = &cac_ctx->acac_actual;
    ot_isp_cac_lcac_manual_attr *lcac_actual = &cac_ctx->lcac_actual;

    iso_index_up  = get_iso_index(iso);
    iso_index_low = MAX2((td_s8)iso_index_up - 1, 0);
    iso1 = get_iso(iso_index_low);
    iso2 = get_iso(iso_index_up);

    acac_actual->edge_threshold[0] = (td_u16)linear_inter(iso, iso1, acac_auto->edge_threshold[0][iso_index_low],
                                                          iso2, acac_auto->edge_threshold[0][iso_index_up]);
    acac_actual->edge_threshold[1] = (td_u16)linear_inter(iso, iso1, acac_auto->edge_threshold[1][iso_index_low],
                                                          iso2, acac_auto->edge_threshold[1][iso_index_up]);
    acac_actual->edge_gain         = (td_u16)linear_inter(iso, iso1, acac_auto->edge_gain[iso_index_low],
                                                          iso2, acac_auto->edge_gain[iso_index_up]);
    acac_actual->cac_rb_strength   = (td_u16)linear_inter(iso, iso1, acac_auto->cac_rb_strength[iso_index_low],
                                                          iso2, acac_auto->cac_rb_strength[iso_index_up]);
    acac_actual->purple_alpha      = (td_u16)linear_inter(iso, iso1, acac_auto->purple_alpha[iso_index_low],
                                                          iso2, acac_auto->purple_alpha[iso_index_up]);
    acac_actual->edge_alpha        = (td_u16)linear_inter(iso, iso1, acac_auto->edge_alpha[iso_index_low],
                                                          iso2, acac_auto->edge_alpha[iso_index_up]);
    acac_actual->satu_low_threshold  = (td_u16)linear_inter(iso, iso1, acac_auto->satu_low_threshold[iso_index_low],
                                                            iso2, acac_auto->satu_low_threshold[iso_index_up]);
    acac_actual->satu_high_threshold = (td_u16)linear_inter(iso, iso1, acac_auto->satu_high_threshold[iso_index_low],
                                                            iso2, acac_auto->satu_high_threshold[iso_index_up]);

    ratio   = cac_get_exp_ratio(vi_pipe);
    ratio_h = cac_get_ratio_idx(ratio);
    ratio_l = MAX2(ratio_h - 1, 0);
    lcac_actual->de_purple_cr_strength = (td_u8)linear_inter(ratio,
        g_exp_ratio_lut[ratio_l], lcac_auto->de_purple_cr_strength[ratio_l],
        g_exp_ratio_lut[ratio_h], lcac_auto->de_purple_cr_strength[ratio_h]);
    lcac_actual->de_purple_cb_strength = (td_u8)linear_inter(ratio,
        g_exp_ratio_lut[ratio_l], lcac_auto->de_purple_cb_strength[ratio_l],
        g_exp_ratio_lut[ratio_h], lcac_auto->de_purple_cb_strength[ratio_h]);
}

static td_void isp_cac_actual_calc(ot_vi_pipe vi_pipe, td_u32 iso, isp_cac *cac_ctx)
{
    if (cac_ctx->mpi_cfg.op_type == OT_OP_MODE_MANUAL) {
        (td_void)memcpy_s(&cac_ctx->acac_actual, sizeof(ot_isp_cac_acac_manual_attr),
                          &cac_ctx->mpi_cfg.acac_cfg.acac_manual, sizeof(ot_isp_cac_acac_manual_attr));
        (td_void)memcpy_s(&cac_ctx->lcac_actual, sizeof(ot_isp_cac_lcac_manual_attr),
                          &cac_ctx->mpi_cfg.lcac_cfg.lcac_manual, sizeof(ot_isp_cac_lcac_manual_attr));
    } else {
        isp_cac_get_mpi_auto_inter_result(vi_pipe, iso, cac_ctx);
    }
}

static td_void isp_cac_fw(td_u32 iso, ot_vi_pipe vi_pipe, td_u8 cur_blk, isp_cac_reg_cfg *cac_reg_cfg, isp_cac *cac_ctx)
{
    ot_unused(cur_blk);

    /* update dyna regs */
    cac_dyn_reg_update(vi_pipe, iso, &cac_reg_cfg->dyna_reg_cfg, cac_ctx);
    /* update user regs */
    cac_user_reg_update(vi_pipe, iso, &cac_reg_cfg->usr_reg_cfg);
}

static td_void isp_cac_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_cac *cac_ctx)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx  = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_cac_actual_calc(vi_pipe, isp_ctx->linkage.iso, cac_ctx);

    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        isp_cac_fw(isp_ctx->linkage.iso, vi_pipe, i, &reg_cfg->alg_reg_cfg[i].cac_reg_cfg, cac_ctx);
    }
}

static td_s32  cac_set_long_frame_mode(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    isp_usr_ctx            *isp_ctx = TD_NULL;
    isp_cac_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_cac_usr_cfg  *usr_reg_cfg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        usr_reg_cfg  = &reg_cfg->alg_reg_cfg[i].cac_reg_cfg.usr_reg_cfg;
        dyna_reg_cfg = &reg_cfg->alg_reg_cfg[i].cac_reg_cfg.dyna_reg_cfg;

        if (is_linear_mode(isp_ctx->sns_wdr_mode) ||
            (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) ||
            (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE)) {
            usr_reg_cfg->r_luma_thr          = ISP_CAC_LUMA_THR_R_LINEAR;
            usr_reg_cfg->g_luma_thr          = ISP_CAC_LUMA_THR_G_LINEAR;
            usr_reg_cfg->b_luma_thr          = ISP_CAC_LUMA_THR_B_LINEAR;
            dyna_reg_cfg->det_satu_thr_high  = ISP_CAC_PURPLE_SATU_HIGH_THD_LIN;
            dyna_reg_cfg->de_purple_ctr_cb   = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CB_STR_LIN;
            dyna_reg_cfg->de_purple_ctr_cr   = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CR_STR_LIN;
        } else {
            usr_reg_cfg->r_luma_thr          = ISP_CAC_LUMA_THR_R_WDR;
            usr_reg_cfg->g_luma_thr          = ISP_CAC_LUMA_THR_G_WDR;
            usr_reg_cfg->b_luma_thr          = ISP_CAC_LUMA_THR_B_WDR;
            dyna_reg_cfg->det_satu_thr_high  = ISP_CAC_PURPLE_SATU_HIGH_THD_WDR;
            dyna_reg_cfg->de_purple_ctr_cb   = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CB_STR_WDR;
            dyna_reg_cfg->de_purple_ctr_cr   = ISP_CAC_DEPURPLE_MAX_STR - ISP_CAC_DEPURPLE_CR_STR_WDR;
        }

        usr_reg_cfg->usr_resh = TD_TRUE;
    }

    return TD_SUCCESS;
}


static __inline td_bool  check_cac_open(const isp_cac *cac)
{
    return (cac->mpi_cfg.enable == TD_TRUE);
}

static td_s32 isp_cac_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_cac *cac_ctx)
{
    td_s32 ret;
    cac_ctx->init = TD_FALSE;
    ret = isp_cac_ctx_initialize(vi_pipe, cac_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    cac_regs_init(vi_pipe, reg_cfg, cac_ctx);
    cac_ext_regs_init(vi_pipe, cac_ctx);

    cac_ctx->init = TD_TRUE;
    ot_ext_system_isp_cac_init_status_write(vi_pipe, cac_ctx->init);
    return TD_SUCCESS;
}

static td_s32 isp_cac_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    isp_cac *cac_ctx = TD_NULL;
    ot_ext_system_isp_cac_init_status_write(vi_pipe, TD_FALSE);
    ret = cac_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    cac_get_ctx(vi_pipe, cac_ctx);
    isp_check_pointer_return(cac_ctx);

    return isp_cac_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg, cac_ctx);
}

static td_s32 isp_cac_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8 i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_CAC;
    isp_usr_ctx *isp_ctx  = TD_NULL;
    isp_cac *cac_ctx      = TD_NULL;
    isp_reg_cfg *reg      = (isp_reg_cfg *)reg_cfg;

    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    cac_get_ctx(vi_pipe, cac_ctx);
    isp_check_pointer_return(cac_ctx);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    ot_ext_system_isp_cac_init_status_write(vi_pipe, cac_ctx->init);
    if (cac_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.fswdr_mode != isp_ctx->linkage.pre_fswdr_mode) {
        cac_set_long_frame_mode(vi_pipe, reg_cfg);
    }

    cac_ctx->mpi_cfg.enable = ot_ext_system_cac_enable_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].cac_reg_cfg.cac_en = cac_ctx->mpi_cfg.enable;
    }

    reg->cfg_key.bit1_cac_cfg = 1;

    /* check hardware setting */
    if (!check_cac_open(cac_ctx)) {
        return TD_SUCCESS;
    }

    cac_read_ext_regs(vi_pipe, cac_ctx);

    isp_cac_reg_update(vi_pipe, reg_cfg, cac_ctx);

    return TD_SUCCESS;
}

static td_s32 cac_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc, const isp_cac *cac_ctx)
{
    ot_isp_ctrl_proc_write  proc_tmp;
    ot_unused(vi_pipe);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "cac info");
    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16s" "%16s" "%16s" "%16s" "%16s\n",
                    "enable", "edge_thr[0]", "edge_thr[1]", "edge_gain", "cac_rb_strength");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16d" "%16u" "%16u" "%16u" "%16u\n",
                    cac_ctx->mpi_cfg.enable, cac_ctx->acac_actual.edge_threshold[0x0],
                    cac_ctx->acac_actual.edge_threshold[0x1], cac_ctx->acac_actual.edge_gain,
                    cac_ctx->acac_actual.cac_rb_strength);

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16s" "%16s" "%16s" "%16s" "%16s" "%16s\n",
                    "satu_low_thd", "satu_high_thd", "purple_alpha", "edge_alpha", "cr_str", "cb_str");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16u" "%16u" "%16u" "%16u" "%16u" "%16u\n",
                    cac_ctx->acac_actual.satu_low_threshold, cac_ctx->acac_actual.satu_high_threshold,
                    cac_ctx->acac_actual.purple_alpha, cac_ctx->acac_actual.edge_alpha,
                    cac_ctx->lcac_actual.de_purple_cr_strength, cac_ctx->lcac_actual.de_purple_cb_strength);

    proc->write_len += 1;
    return TD_SUCCESS;
}

static td_s32 isp_cac_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_cac *cac_ctx = TD_NULL;

    switch (cmd) {
        case OT_ISP_PROC_WRITE:
            cac_get_ctx(vi_pipe, cac_ctx);
            isp_check_pointer_return(cac_ctx);
            cac_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value, (const isp_cac *)cac_ctx);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_cac_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr  *reg_cfg    = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    ot_ext_system_isp_cac_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].cac_reg_cfg.cac_en = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_cac_cfg = 1;

    cac_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_cac(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_alg_node *algs    = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_cac);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_CAC;
    algs->alg_func.pfn_alg_init = isp_cac_init;
    algs->alg_func.pfn_alg_run  = isp_cac_run;
    algs->alg_func.pfn_alg_ctrl = isp_cac_ctrl;
    algs->alg_func.pfn_alg_exit = isp_cac_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
