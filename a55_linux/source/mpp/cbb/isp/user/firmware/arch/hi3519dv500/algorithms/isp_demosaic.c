/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "isp_demosaic.h"
#include <math.h>
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_proc.h"
#include "isp_math_utils.h"
#include "isp_param_check.h"
#include "isp_ext_reg_access.h"

#define OT_DEMOSAIC_BITDEPTH    12
#define DEMOSAIC_FILTER_NUM     7
#define DEMOSAIC_FILTER_COEF    3
#define BITFIX_NUM             10

static const  td_s32 g_demosaic_filtcoef[DEMOSAIC_FILTER_NUM][DEMOSAIC_FILTER_COEF] = {
    {0, 0, 32},
    {-1, 4, 26},
    {-1, 5, 24},
    {-1, 6, 22},
    {-1, 7, 20},
    {-1, 8, 18},
    {0, 8, 16}
};

static const  td_u8  g_nddm_str[OT_ISP_AUTO_ISO_NUM] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
static const  td_u8  g_nddm_mf_detailehc_str[OT_ISP_AUTO_ISO_NUM] = {
    32, 24, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};
static const  td_u8  g_nddm_hf_detailehc_str[OT_ISP_AUTO_ISO_NUM] = {
    0, 0, 3, 5, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};
static const  td_u8  g_detail_smooth_range[OT_ISP_AUTO_ISO_NUM] = {
    2, 2, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7
};
static const  td_u8  g_color_noise_thdf[OT_ISP_AUTO_ISO_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const  td_u8  g_color_noise_strf[OT_ISP_AUTO_ISO_NUM] = {
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};
static const  td_u8  g_color_noise_thdy[OT_ISP_AUTO_ISO_NUM] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const  td_u8  g_color_noise_stry[OT_ISP_AUTO_ISO_NUM] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10
};

/* Linear parameters */
static const td_u8   g_lut_cc_hf_max_ratio_lin[OT_ISP_AUTO_ISO_NUM] = {
    3, 4, 5, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8
};
static const td_u8   g_lut_cc_hf_min_ratio_lin[OT_ISP_AUTO_ISO_NUM] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const td_s8   g_lut_fcr_gf_gain_lin[OT_ISP_AUTO_ISO_NUM]     = {
    12, 12, 12, 10, 8, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const td_u8   g_lut_awb_gfgain_max_lin[OT_ISP_AUTO_ISO_NUM]  = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const td_u8   g_lut_awb_gfgain_low_lin[OT_ISP_AUTO_ISO_NUM]  = {
    32, 32, 30, 26, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};
static const td_u8   g_lut_awb_gfgain_hig_lin[OT_ISP_AUTO_ISO_NUM]  = {
    64, 64, 60, 56, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48
};
static const td_u16  g_lut_fcr_det_low_lin[OT_ISP_AUTO_ISO_NUM]     = {
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
};
static const td_u16  g_lut_desat_low_lin[OT_ISP_AUTO_ISO_NUM]       = {
    166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166
};
static const td_u16  g_lut_desat_prot_th_lin[OT_ISP_AUTO_ISO_NUM]   = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};
static const td_u16  g_hf_intp_blur_th_low_lin[OT_ISP_AUTO_ISO_NUM] = {
    1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
};
static const td_u16  g_hf_intp_blur_th_hig_lin[OT_ISP_AUTO_ISO_NUM] = {
    1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
};

/* wdr parameters */
static const td_u8   g_lut_cc_hf_max_ratio_wdr[OT_ISP_AUTO_ISO_NUM] = {
    8, 8, 8, 8, 10, 10, 10, 12, 12, 14, 14, 16, 16, 16, 16, 16
};
static const td_u8   g_lut_cc_hf_min_ratio_wdr[OT_ISP_AUTO_ISO_NUM] = {
    0, 0, 0, 0, 0, 0, 2, 2, 4, 4, 4, 8, 10, 10, 12, 14
};
static const td_s8   g_lut_fcr_gf_gain_wdr[OT_ISP_AUTO_ISO_NUM]     = {
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const td_u8   g_lut_awb_gfgain_low_wdr[OT_ISP_AUTO_ISO_NUM]  = {
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
};
static const td_u8   g_lut_awb_gfgain_hig_wdr[OT_ISP_AUTO_ISO_NUM]  = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
static const td_u8   g_lut_awb_gfgain_max_wdr[OT_ISP_AUTO_ISO_NUM]  = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const td_u16  g_lut_fcr_det_low_wdr[OT_ISP_AUTO_ISO_NUM]     = {
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150
};
static const td_u16  g_lut_desat_low_wdr[OT_ISP_AUTO_ISO_NUM]       = {
    166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166, 166
};
static const td_u16  g_lut_desat_prot_th_wdr[OT_ISP_AUTO_ISO_NUM]   = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};
static const td_u16  g_hf_intp_blur_th_low_wdr[OT_ISP_AUTO_ISO_NUM] = {
    1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
};
static const td_u16  g_hf_intp_blur_th_hig_wdr[OT_ISP_AUTO_ISO_NUM] = {
    1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
};

static isp_demosaic *g_demosaic_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define demosaic_get_ctx(pipe, ctx)   ((ctx) = g_demosaic_ctx[pipe])
#define demosaic_set_ctx(pipe, ctx)   (g_demosaic_ctx[pipe] = (ctx))
#define demosaic_reset_ctx(pipe)      (g_demosaic_ctx[pipe] = TD_NULL)

static td_s32 demosaic_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_demosaic *demosaic_ctx = TD_NULL;

    demosaic_get_ctx(vi_pipe, demosaic_ctx);

    if (demosaic_ctx == TD_NULL) {
        demosaic_ctx = (isp_demosaic *)isp_malloc(sizeof(isp_demosaic));
        if (demosaic_ctx == TD_NULL) {
            isp_err_trace("isp[%d] demosaic_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(demosaic_ctx, sizeof(isp_demosaic), 0, sizeof(isp_demosaic));

    demosaic_set_ctx(vi_pipe, demosaic_ctx);

    return TD_SUCCESS;
}

static td_void demosaic_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_demosaic *demosaic_ctx = TD_NULL;

    demosaic_get_ctx(vi_pipe, demosaic_ctx);
    isp_free(demosaic_ctx);
    demosaic_reset_ctx(vi_pipe);
}

static td_void demosaic_init_fw_linear(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    ot_unused(vi_pipe);
    (td_void)memcpy_s(demosaic->lut_cc_hf_max_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_cc_hf_max_ratio_lin,     sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_cc_hf_min_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_cc_hf_min_ratio_lin,     sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_fcr_gf_gain,     sizeof(td_s8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_fcr_gf_gain_lin,         sizeof(td_s8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_low,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_low_lin,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_hig,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_hig_lin,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_max,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_max_lin,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_fcr_det_low,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_fcr_det_low_lin,         sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_desat_low,       sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_desat_low_lin,           sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_desat_prot_th,   sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_desat_prot_th_lin,       sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->hf_intp_blur_th_low, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_hf_intp_blur_th_low_lin,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->hf_intp_blur_th_hig, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_hf_intp_blur_th_hig_lin,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
}

static td_void demosaic_init_fw_wdr(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    ot_unused(vi_pipe);
    (td_void)memcpy_s(demosaic->lut_cc_hf_max_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_cc_hf_max_ratio_wdr,     sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_cc_hf_min_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_cc_hf_min_ratio_wdr,     sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_fcr_gf_gain,     sizeof(td_s8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_fcr_gf_gain_wdr,         sizeof(td_s8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_low,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_low_wdr,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_hig,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_hig_wdr,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_awb_gfgain_max,  sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_awb_gfgain_max_wdr,      sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_fcr_det_low,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_fcr_det_low_wdr,         sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_desat_low,       sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_desat_low_wdr,           sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->lut_desat_prot_th,   sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_lut_desat_prot_th_wdr,       sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->hf_intp_blur_th_low, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_hf_intp_blur_th_low_wdr,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(demosaic->hf_intp_blur_th_hig, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      g_hf_intp_blur_th_hig_wdr,     sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
}

static td_float demosaic_detail_com_decom(td_float x)
{
    const td_float alpha = 32.0f;
    td_float y;
    x = MAX2(0.0f, MIN2(1.0f, x));
    y = x / (alpha + (1.0f - alpha) * x);
    return MAX2(0.0f, MIN2(1.0f, y));
}

static td_void demosaic_static_regs_initialize(ot_vi_pipe vi_pipe, isp_demosaic_static_cfg *static_reg_cfg, td_u32 i)
{
    ot_unused(vi_pipe);
    ot_unused(i);
    td_u16  j;
    static_reg_cfg->de_sat_enable      = OT_ISP_DEMOSAIC_DESAT_ENABLE_DEFAULT;
    static_reg_cfg->ahd_enable         = OT_ISP_DEMOSAIC_AND_EN_DEFAULT;
    static_reg_cfg->de_fake_enable     = OT_ISP_DEMOSAIC_DE_FAKE_EN_DEFAULT;
    static_reg_cfg->lpff3              = OT_ISP_DEMOSAIC_LPF_F3_DEFAULT;
    static_reg_cfg->hv_blend_limit1    = OT_ISP_DEMOSAIC_BLENDLIMIT1_DEFAULT;
    static_reg_cfg->hv_blend_limit2    = OT_ISP_DEMOSAIC_BLENDLIMIT2_DEFAULT;
    static_reg_cfg->hv_color_ratio     = OT_ISP_DEMOSAIC_HV_RATIO_DEFAULT;
    static_reg_cfg->hv_selection       = OT_ISP_DEMOSAIC_HV_SEL_DEFAULT;
    static_reg_cfg->g_clip_bit_sft     = OT_ISP_DEMOSAIC_G_CLIP_SFT_BIT_DEFAULT;
    static_reg_cfg->dither_mask        = OT_ISP_DITHER_MASK_DEFAULT;
    static_reg_cfg->dither_ratio       = OT_ISP_DITHER_RATIO_DEFAULT;
    static_reg_cfg->cc_var_thresh      = OT_ISP_DEMOSAIC_CC_VAR_THRESH_DEFAULT;
    static_reg_cfg->de_sat_thresh1     = OT_ISP_DEMOSAIC_DESAT_THRESH1_DEFAULT;
    static_reg_cfg->de_sat_thresh2     = OT_ISP_DEMOSAIC_DESAT_THRESH2_DEFAULT;
    static_reg_cfg->de_sat_hig         = OT_ISP_DEMOSAIC_DESAT_HIG_DEFAULT;
    static_reg_cfg->de_sat_prot_sl     = OT_ISP_DEMOSAIC_DESAT_PROTECT_SL_DEFAULT;
    static_reg_cfg->ahd_part1          = OT_ISP_DEMOSAIC_AHDPART1_DEFAULT;
    static_reg_cfg->ahd_part2          = OT_ISP_DEMOSAIC_AHDPART2_DEFAULT;
    static_reg_cfg->gf_th_low          = OT_ISP_NDDM_GF_TH_LOW_DEFAULT;
    static_reg_cfg->gf_th_hig          = OT_ISP_NDDM_GF_TH_HIGH_DEFAULT;
    static_reg_cfg->cb_cr_avg_thr      = OT_ISP_DEMOSAIC_CBCR_AVG_THLD_DEFAULT;

    for (j = 0; j < OT_ISP_DEMOSAIC_AIDM_LUT_NUM; j++) {
        static_reg_cfg->aidm_detail_lut[j] = (td_u16)(demosaic_detail_com_decom((td_float)j / 127.0f) * 4095.0f);
    }
    static_reg_cfg->buf_id = 0;
    static_reg_cfg->resh = TD_TRUE;
}

static td_void demosaic_dyna_regs_initialize(isp_demosaic_dyna_cfg *dyna_reg_cfg)
{
    td_u32 n;

    dyna_reg_cfg->lpff0                = OT_ISP_DEMOSAIC_LPF_F0_DEFAULT;
    dyna_reg_cfg->lpff1                = OT_ISP_DEMOSAIC_LPF_F1_DEFAULT;
    dyna_reg_cfg->lpff2                = OT_ISP_DEMOSAIC_LPF_F2_DEFAULT;
    dyna_reg_cfg->cc_hf_max_ratio      = OT_ISP_DEMOSAIC_CC_HF_MAX_RATIO_DEFAULT;
    dyna_reg_cfg->cc_hf_min_ratio      = OT_ISP_DEMOSAIC_CC_HF_MIN_RATIO_DEFAULT;
    dyna_reg_cfg->dither_max           = OT_ISP_DITH_MAX_DEFAULT;
    dyna_reg_cfg->fcr_gf_gain          = OT_ISP_NDDM_FCR_GF_GAIN_DEFAULT;
    dyna_reg_cfg->awb_gf_gain_low      = OT_ISP_NDDM_AWB_GF_GN_LOW_DEFAULT;
    dyna_reg_cfg->awb_gf_gain_hig      = OT_ISP_NDDM_AWB_GF_GN_HIGH_DEFAULT;
    dyna_reg_cfg->awb_gf_gain_max      = OT_ISP_NDDM_AWB_GF_GN_MAX_DEFAULT;
    dyna_reg_cfg->ehc_gray             = OT_ISP_NDDM_EHC_GRAY_DEFAULT;
    dyna_reg_cfg->de_sat_low           = OT_ISP_DEMOSAIC_DESAT_LOW_DEFAULT;
    dyna_reg_cfg->de_sat_ratio         = OT_ISP_DEMOSAIC_DESAT_RATIO_DEFAULT;
    dyna_reg_cfg->de_sat_prot_th       = OT_ISP_DEMOSAIC_DESAT_PROTECT_TH_DEFAULT;
    dyna_reg_cfg->hf_intp_blur_th_low  = OT_ISP_DEMOSAIC_HF_INPT_BLUR_TH_LOW_DEFAULT;
    dyna_reg_cfg->hf_intp_blur_th_hig  = OT_ISP_DEMOSAIC_HF_INPT_BLUR_TH_HIGH_DEFAULT;
    dyna_reg_cfg->hf_intp_blur_ratio   = OT_ISP_DEMOSAIC_HF_INPT_BLUR_RATIO_DEFAULT;
    dyna_reg_cfg->fcr_det_low          = OT_ISP_NDDM_FCR_DET_LOW_DEFAULT;
    dyna_reg_cfg->fake_cr_var_thr_low  = OT_ISP_DEMOSAIC_FAKECR_VAR_THR_LOW_DEFAULT;
    dyna_reg_cfg->fake_cr_var_thr_high = OT_ISP_DEMOSAIC_FAKECR_VAR_THR_HIGH_DEFAULT;
    dyna_reg_cfg->de_f_color_cb        = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - OT_ISP_DEMOSAIC_DEFCOLOR_CB_DEFAULT;
    dyna_reg_cfg->de_f_color_cr        = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - OT_ISP_DEMOSAIC_DEFCOLOR_CR_DEFAULT;

    for (n = 0; n < OT_ISP_DEMOSAIC_LUT_LENGTH; n++) {
        dyna_reg_cfg->gf_blur_luts[n] = 0;
    }

    dyna_reg_cfg->color_noise_thd_y              = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DESAT_DARK_RANGE_DEFAULT;
    dyna_reg_cfg->color_noise_str_y              = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DESAT_DARK_STRENGTH_DEFAULT;
    dyna_reg_cfg->pro_dark_region.ccm_prot_en    = OT_ISP_CCM_PROT_EN_DEFAULT;

    dyna_reg_cfg->update_gf = TD_TRUE;
    dyna_reg_cfg->resh      = TD_TRUE;
}

static td_void demosaic_usr_regs_initialize(isp_demosaic_usr_cfg *usr_reg_cfg, isp_demosaic *demosaic_ctx)
{
    usr_reg_cfg->ai_detail_strength = demosaic_ctx->mpi_cfg.ai_detail_strength;
    usr_reg_cfg->resh             = TD_TRUE;
    usr_reg_cfg->update_index     = 1;
}

static td_void demosaic_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg  *reg_cfg, isp_demosaic *demosaic_ctx)
{
    td_u32 i;
    isp_demosaic_static_cfg *static_reg_cfg = TD_NULL;
    isp_demosaic_dyna_cfg   *dyna_reg_cfg   = TD_NULL;
    isp_demosaic_usr_cfg    *usr_reg_cfg    = TD_NULL;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        static_reg_cfg = &reg_cfg->alg_reg_cfg[i].dem_reg_cfg.static_reg_cfg;
        dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[i].dem_reg_cfg.dyna_reg_cfg;
        usr_reg_cfg    = &reg_cfg->alg_reg_cfg[i].dem_reg_cfg.usr_reg_cfg;
        reg_cfg->alg_reg_cfg[i].dem_reg_cfg.vhdm_enable = demosaic_ctx->mpi_cfg.enable;
        reg_cfg->alg_reg_cfg[i].dem_reg_cfg.nddm_enable = demosaic_ctx->mpi_cfg.enable;
        demosaic_static_regs_initialize(vi_pipe, static_reg_cfg, i);
        demosaic_dyna_regs_initialize(dyna_reg_cfg);
        demosaic_usr_regs_initialize(usr_reg_cfg, demosaic_ctx);
    }

    reg_cfg->cfg_key.bit1_dem_cfg = 1;
}

static td_s32 demosaic_check_cmos_param(const ot_isp_demosaic_attr *cmos_demosaic)
{
    td_s32 ret;
    isp_check_bool_return(cmos_demosaic->enable);
    ret = isp_demosaic_attr_check("cmos", cmos_demosaic);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_s32 demosaic_update_from_sns(ot_isp_cmos_default *sns, isp_demosaic *demosaic)
{
    td_s32 ret;

    isp_check_pointer_return(sns->demosaic);
    ret = demosaic_check_cmos_param(sns->demosaic);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    (td_void)memcpy_s(&demosaic->mpi_cfg, sizeof(ot_isp_demosaic_attr), sns->demosaic, sizeof(ot_isp_demosaic_attr));

    return TD_SUCCESS;
}

static td_void demosaic_update_from_default(isp_demosaic *demosaic)
{
    demosaic->mpi_cfg.enable  = TD_TRUE;
    demosaic->mpi_cfg.op_type = OT_OP_MODE_AUTO;
    demosaic->mpi_cfg.ai_detail_strength = OT_EXT_SYSTEM_DEMOSAIC_AI_DETAIL_STRENGTH_DEFAULT;

    if (is_wdr_mode(demosaic->wdr_mode)) {    /* Manual:WDR mode */
        demosaic->mpi_cfg.manual_attr.nddm_strength           = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_STR_WDRDFT;
        demosaic->mpi_cfg.manual_attr.nddm_mf_detail_strength = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_MF_EHNC_STR_WDRDFT;
        demosaic->mpi_cfg.manual_attr.nddm_hf_detail_strength = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_HF_EHNC_STR_WDRDFT;
        demosaic->mpi_cfg.manual_attr.detail_smooth_range     = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DTLSMOOTH_RANGE_WDRDFT;
    } else {   /* Manual:Linear Mode */
        demosaic->mpi_cfg.manual_attr.nddm_strength           = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_STR_LINDFT;
        demosaic->mpi_cfg.manual_attr.nddm_mf_detail_strength = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_MF_EHNC_STR_LINDFT;
        demosaic->mpi_cfg.manual_attr.nddm_hf_detail_strength = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_NDDM_HF_EHNC_STR_LINDFT;
        demosaic->mpi_cfg.manual_attr.detail_smooth_range     = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DTLSMOOTH_RANGE_LINDFT;
    }

    demosaic->mpi_cfg.manual_attr.color_noise_f_threshold = OT_EXT_SYSTEM_DEMOSAIC_COLORNOISE_CTRL_THDF_DEFAULT;
    demosaic->mpi_cfg.manual_attr.color_noise_f_strength  = OT_EXT_SYSTEM_DEMOSAIC_COLORNOISE_CTRL_STRF_DEFAULT;
    demosaic->mpi_cfg.manual_attr.color_noise_y_threshold = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DESAT_DARK_RANGE_DEFAULT;
    demosaic->mpi_cfg.manual_attr.color_noise_y_strength  = OT_EXT_SYSTEM_DEMOSAIC_MANUAL_DESAT_DARK_STRENGTH_DEFAULT;

    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.nddm_strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_nddm_str, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.nddm_mf_detail_strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_nddm_mf_detailehc_str, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.nddm_hf_detail_strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_nddm_hf_detailehc_str, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.detail_smooth_range, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_detail_smooth_range, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.color_noise_f_threshold, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_color_noise_thdf, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.color_noise_f_strength,  OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_color_noise_strf, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.color_noise_y_threshold, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_color_noise_thdy, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    (td_void)memcpy_s(demosaic->mpi_cfg.auto_attr.color_noise_y_strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                      g_color_noise_stry, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
}


static td_s32 demosaic_update(ot_isp_cmos_default *sns_dft, isp_demosaic *demosaic)
{
    td_s32 ret;
    if (sns_dft->key.bit1_demosaic) {
        ret = demosaic_update_from_sns(sns_dft, demosaic);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else {
        demosaic_update_from_default(demosaic);
    }
    return TD_SUCCESS;
}

static td_s32 isp_demosaic_ctx_initialize(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    td_u8 i;
    td_s32 ret;
    const td_s32   sigma_norm_factor = 16;
    td_float       n_cur, n_pre, n_fct;
    const td_float drc_compensate = 0.6f;
    const td_float max_num = 256.0f;
    td_float       afsigma[OT_ISP_DEMOSAIC_LUT_LENGTH];

    ot_isp_cmos_default *sns_dft = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_sensor_get_default(vi_pipe, &sns_dft);

    demosaic->wdr_mode = isp_ctx->sns_wdr_mode;
    demosaic->bit_depth_prc = OT_DEMOSAIC_BITDEPTH;

    if (demosaic->wdr_mode != 0) {
        demosaic_init_fw_wdr(vi_pipe, demosaic);

        for (i = 0; i < OT_ISP_DEMOSAIC_LUT_LENGTH; i++) {
            n_cur = (td_float)(i * sigma_norm_factor);
            n_pre = (td_float)(max_num * pow(n_cur / max_num, 1.0f / div_0_to_1_float(drc_compensate))) + 0.5f;
            n_fct = (td_float)(isp_sqr(n_cur / div_0_to_1_float(n_pre)));

            afsigma[i] = n_cur * n_fct;
            demosaic->sigma_demcfg[i] = (td_u32)(afsigma[i] * isp_bitfix(BITFIX_NUM));
        }
    } else {
        demosaic_init_fw_linear(vi_pipe, demosaic);

        for (i = 0; i < OT_ISP_DEMOSAIC_LUT_LENGTH; i++) {
            demosaic->sigma_demcfg[i]  = (td_u32)(i * sigma_norm_factor * isp_bitfix(BITFIX_NUM));
        }
    }
    /* ctx loading from sns */
    ret = demosaic_update(sns_dft, demosaic);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void demosaic_ext_regs_initialize(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    isp_demosaic_attr_write(vi_pipe, &demosaic->mpi_cfg);

    return;
}

td_void isp_demosaic_set_long_frame_mode(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    td_u8          i;
    const td_s32   sigma_norm_factor = 16;
    td_float       n_cur, n_pre, n_fct;
    const td_float       drc_compensate = 0.6f;
    const td_float max_num = 256.0f;
    td_float       afsigma[OT_ISP_DEMOSAIC_LUT_LENGTH];
    isp_usr_ctx   *isp_ctx  = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->linkage.fswdr_mode == isp_ctx->linkage.pre_fswdr_mode) {
        return;
    }

    if (is_linear_mode(isp_ctx->sns_wdr_mode) ||
        (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) ||
        (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE)) {
        demosaic_init_fw_linear(vi_pipe, demosaic);

        for (i = 0; i < OT_ISP_DEMOSAIC_LUT_LENGTH; i++) {
            demosaic->sigma_demcfg[i]  = (td_u32)(i * sigma_norm_factor * isp_bitfix(BITFIX_NUM));
        }
    } else {
        demosaic_init_fw_wdr(vi_pipe, demosaic);

        for (i = 0; i < OT_ISP_DEMOSAIC_LUT_LENGTH; i++) {
            n_cur = (td_float)(i * sigma_norm_factor);
            n_pre = (td_float)(max_num * pow(n_cur / max_num, 1.0f / div_0_to_1_float(drc_compensate))) + 0.5f;
            n_fct = (td_float)(isp_sqr(n_cur / div_0_to_1_float(n_pre)));

            afsigma[i]  =  n_cur * n_fct;
            demosaic->sigma_demcfg[i] = (td_u32)(afsigma[i] * isp_bitfix(BITFIX_NUM));
        }
    }
}

td_void isp_demosaic_read_extregs(ot_vi_pipe vi_pipe, isp_demosaic *demosaic)
{
    demosaic->dem_attr_update = ot_ext_system_demosaic_attr_update_en_read(vi_pipe);
    if (demosaic->dem_attr_update) {
        ot_ext_system_demosaic_attr_update_en_write(vi_pipe, TD_FALSE);
        isp_demosaic_attr_read(vi_pipe, &demosaic->mpi_cfg);
    }
}

td_s32 demosaic_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc, const isp_demosaic *demosaic)
{
    ot_unused(vi_pipe);
    ot_isp_ctrl_proc_write proc_tmp;

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "demosaic info");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16s" "%16s" "%16s" "%16s" "%16s\n",
                    "enable", "nondir_str", "nondir_mf_str", "nondir_hf_str", "de_smth_range");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16u" "%16u" "%16u" "%16u" "%16u\n",
                    demosaic->mpi_cfg.enable, demosaic->actual.nddm_strength, demosaic->actual.nddm_mf_detail_strength,
                    demosaic->actual.nddm_hf_detail_strength, demosaic->actual.detail_smooth_range);

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%20s" "%20s" "%20s" "%20s\n",
                    "color_noise_thdf", "color_noise_strf", "color_noise_thdy", "color_noise_stry");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%20u" "%20u" "%20u" "%20u\n",
                    demosaic->actual.color_noise_f_threshold, demosaic->actual.color_noise_f_strength,
                    demosaic->actual.color_noise_y_threshold, demosaic->actual.color_noise_y_strength);

    proc->write_len += 1;

    return TD_SUCCESS;
}

td_s32 isp_demosaic_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_demosaic *demosaic_ctx)
{
    td_s32 ret;
    demosaic_ctx->init = TD_FALSE;
    ret = isp_demosaic_ctx_initialize(vi_pipe, demosaic_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    demosaic_regs_initialize(vi_pipe, reg_cfg, demosaic_ctx);
    demosaic_ext_regs_initialize(vi_pipe, demosaic_ctx);
    demosaic_ctx->init = TD_TRUE;
    ot_ext_system_isp_demosaic_init_status_write(vi_pipe, TD_TRUE);
    return TD_SUCCESS;
}

static td_s32 isp_demosaic_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    isp_demosaic *demosaic = TD_NULL;

    ot_ext_system_isp_demosaic_init_status_write(vi_pipe, TD_FALSE);
    ret = demosaic_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    demosaic_get_ctx(vi_pipe, demosaic);
    isp_check_pointer_return(demosaic);

    return isp_demosaic_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg, demosaic);
}

static td_s32 isp_demosaic_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_u8 i;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].dem_reg_cfg.vhdm_enable = TD_FALSE;
        reg_cfg->alg_reg_cfg[i].dem_reg_cfg.nddm_enable = TD_FALSE;
    }
    reg_cfg->cfg_key.bit1_dem_cfg = 1;

    return isp_demosaic_init(vi_pipe, reg_cfg_info);
}

#define GFBLUR_PROCESS_BIT 8
static td_void demosaic_gf_blur_lut(const isp_demosaic *demosaic, const gf_blur_lut_cfg *info_cfg,
    isp_demosaic_dyna_cfg *dm_cfg)
{
    td_u8    n;
    const td_u8 shift_sigma1 = 14;
    const td_u8 shift_sigma2 = 5;
    const td_u8    non_dir_str = info_cfg->non_dir_str;
    const td_u32   bit_scale   = isp_bitfix(demosaic->bit_depth_prc - GFBLUR_PROCESS_BIT);
    const td_u32   max_sigma = 0xffffffff;
    td_u32   iso = info_cfg->iso;
    td_u32   alpha, sigma;
    td_u64   sigma_long;
    ot_vi_pipe  vi_pipe = info_cfg->vi_pipe;
    td_float k_a, k_b, calibration_coef, iso_change;
    ot_isp_cmos_default *sns_dft = TD_NULL;
    isp_sensor_get_default(vi_pipe, &sns_dft);

    iso_change = sns_dft->noise_calibration.calibration_coef[0];

    if (iso > iso_change) {
        k_a = sns_dft->noise_calibration.calibration_coef[1]; /* 1 */
        k_b = sns_dft->noise_calibration.calibration_coef[2]; /* 2 */
    } else {
        k_a = sns_dft->noise_calibration.calibration_coef[6]; /* 6 */
        k_b = sns_dft->noise_calibration.calibration_coef[7]; /* 7 */
    }

    calibration_coef = k_a * iso + k_b;

    alpha = (td_u32)(calibration_coef * isp_bitfix(BITFIX_NUM));

    for (n = 0; n < OT_ISP_DEMOSAIC_LUT_LENGTH; n++) {
        sigma_long  = (td_u64)demosaic->sigma_demcfg[n] * alpha;
        sigma_long *= non_dir_str;
        sigma  = MIN2((sigma_long >> shift_sigma1), max_sigma);
        sigma  = (td_u32)sqrt32(sigma);
        sigma = (sigma * bit_scale) >> shift_sigma2;
        dm_cfg->gf_blur_luts[n] = MIN2(sigma, (td_u32)isp_bitmask(demosaic->bit_depth_prc));
    }

    dm_cfg->gf_blur_luts[0] = dm_cfg->gf_blur_luts[1] / 2; /* algorithm related, [0] must be [1] / 2 */
    dm_cfg->update_gf       = TD_TRUE;

    return;
}

#define RGB_THD_MIN    2
#define RGB_THD_MAX    511
#define TRANSIT_RB_THD 70
#define TRANSIT_G_THD  54
#define PROT_G_THD     24
#define PROT_G_THD_START 48
#define DARK_PROT_BITSHIFT 3
#define DARK_PROT_RATIO_MAX 257
static td_void demosaic_dark_pro_cfg(isp_demosaic_dyna_cfg *dm_cfg)
{
    td_u32 data_rb_thd0;
    td_u32 data_g_thd0;
    td_u32 data_rb_thd1;
    td_u32 data_g_thd1;

    if (dm_cfg->color_noise_str_y == 0) {
        dm_cfg->pro_dark_region.ccm_prot_en = 0;
    } else {
        dm_cfg->pro_dark_region.ccm_prot_en = 1;
    }

    dm_cfg->pro_dark_region.cc_prot_ratio  = DARK_PROT_RATIO_MAX - dm_cfg->color_noise_str_y;
    dm_cfg->pro_dark_region.rgb_prot_ratio = DARK_PROT_RATIO_MAX - dm_cfg->color_noise_str_y;

    data_rb_thd0 = dm_cfg->color_noise_thd_y;

    if (dm_cfg->color_noise_thd_y > PROT_G_THD_START) {
        data_g_thd0 = dm_cfg->color_noise_thd_y - PROT_G_THD;
    } else {
        data_g_thd0 = dm_cfg->color_noise_thd_y >> 1;
    }

    data_rb_thd0 = MAX2(data_rb_thd0, RGB_THD_MIN);
    data_g_thd0  = MAX2(data_g_thd0, RGB_THD_MIN);
    data_rb_thd0 = MIN2(data_rb_thd0, RGB_THD_MAX);
    data_g_thd0  = MIN2(data_g_thd0, RGB_THD_MAX);
    data_rb_thd1 = data_rb_thd0 + TRANSIT_RB_THD;
    data_g_thd1  = data_g_thd0 + TRANSIT_G_THD;
    data_rb_thd1 = MIN2(data_rb_thd1, RGB_THD_MAX);
    data_g_thd1  = MIN2(data_g_thd1, RGB_THD_MAX);

    dm_cfg->pro_dark_region.data_cc_thd0 = 0;
    dm_cfg->pro_dark_region.data_cc_thd1 = data_rb_thd0 << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_rr_thd0 = data_rb_thd0 << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_rr_thd1 = data_rb_thd1 << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_gg_thd0 = data_g_thd0  << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_gg_thd1 = data_g_thd1  << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_bb_thd0 = data_rb_thd0 << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.data_bb_thd1 = data_rb_thd1 << DARK_PROT_BITSHIFT;
    dm_cfg->pro_dark_region.max_rgb_thd  = 0xFFF;
}

static td_void demosaic_get_iso_info(td_u32 *iso_low, td_u32 *iso_up, td_u8 *idx_up, td_u8 *idx_lo, td_u32 input_iso)
{
    td_u32 inner_idx[2], inner_iso[2]; /* inner_idx and inner_iso length is 2 */

    inner_idx[0] = get_iso_index(input_iso);
    inner_idx[1] = MAX2((td_s8)inner_idx[0] - 1, 0);
    inner_iso[0] = get_iso(inner_idx[0]);
    inner_iso[1] = get_iso(inner_idx[1]);

    *idx_up  = inner_idx[0];
    *idx_lo  = inner_idx[1];
    *iso_up  = inner_iso[0];
    *iso_low = inner_iso[1];
}

static td_void demosaic_cfg(isp_demosaic_dyna_cfg *dm_cfg, const isp_demosaic_static_cfg *static_dm_cfg,
    isp_demosaic *demosaic, td_u32 iso)
{
    td_s16 desat_th1 = (td_s16)static_dm_cfg->de_sat_thresh1;
    td_s16 desat_th2 = (td_s16)static_dm_cfg->de_sat_thresh2;
    td_s16 desat_hig = (td_s16)static_dm_cfg->de_sat_hig;

    td_u8  iso_index_upper;
    td_u8  iso_index_lower;
    td_u32 iso2;
    td_u32 iso1;

    /* Calculate iso_index_upper & iso_index_lower by using input iso */
    iso_index_upper = get_iso_index(iso);
    iso_index_lower = MAX2((td_s8)iso_index_upper - 1, 0);

    /* Calculate iso_index_upper & iso_index_lower by using input iso */
    iso1 = get_iso(iso_index_lower);
    iso2 = get_iso(iso_index_upper);

    dm_cfg->cc_hf_max_ratio     = (td_u8)linear_inter(iso, iso1, demosaic->lut_cc_hf_max_ratio[iso_index_lower],
                                                      iso2, demosaic->lut_cc_hf_max_ratio[iso_index_upper]);
    dm_cfg->cc_hf_min_ratio     = (td_u8)linear_inter(iso, iso1, demosaic->lut_cc_hf_min_ratio[iso_index_lower],
                                                      iso2, demosaic->lut_cc_hf_min_ratio[iso_index_upper]);
    dm_cfg->fcr_gf_gain         = (td_u8)linear_inter(iso, iso1, demosaic->lut_fcr_gf_gain[iso_index_lower],
                                                      iso2, demosaic->lut_fcr_gf_gain[iso_index_upper]);
    dm_cfg->awb_gf_gain_low     = (td_u8)linear_inter(iso, iso1, demosaic->lut_awb_gfgain_low[iso_index_lower],
                                                      iso2, demosaic->lut_awb_gfgain_low[iso_index_upper]);
    dm_cfg->awb_gf_gain_hig     = (td_u8)linear_inter(iso, iso1, demosaic->lut_awb_gfgain_hig[iso_index_lower],
                                                      iso2, demosaic->lut_awb_gfgain_hig[iso_index_upper]);
    dm_cfg->awb_gf_gain_max     = (td_u8)linear_inter(iso, iso1, demosaic->lut_awb_gfgain_max[iso_index_lower],
                                                      iso2, demosaic->lut_awb_gfgain_max[iso_index_upper]);
    dm_cfg->fcr_det_low         = (td_u16)linear_inter(iso, iso1, demosaic->lut_fcr_det_low[iso_index_lower],
                                                       iso2, demosaic->lut_fcr_det_low[iso_index_upper]);
    dm_cfg->de_sat_low          = (td_u16)linear_inter(iso, iso1, demosaic->lut_desat_low[iso_index_lower],
                                                       iso2, demosaic->lut_desat_low[iso_index_upper]);
    dm_cfg->de_sat_prot_th      = (td_u16)linear_inter(iso, iso1, demosaic->lut_desat_prot_th[iso_index_lower],
                                                       iso2, demosaic->lut_desat_prot_th[iso_index_upper]);
    dm_cfg->hf_intp_blur_th_low = (td_u16)linear_inter(iso, iso1, demosaic->hf_intp_blur_th_low[iso_index_lower],
                                                       iso2, demosaic->hf_intp_blur_th_low[iso_index_upper]);
    dm_cfg->hf_intp_blur_th_hig = (td_u16)linear_inter(iso, iso1, demosaic->hf_intp_blur_th_hig[iso_index_lower],
                                                       iso2, demosaic->hf_intp_blur_th_hig[iso_index_upper]);

    if (desat_th1 == desat_th2) {
        dm_cfg->de_sat_ratio = 0;
    } else {
        /* 16 is a accurary norm factor for parameter de_sat_ratio */
        dm_cfg->de_sat_ratio = 16 * ((td_s16)dm_cfg->de_sat_low - desat_hig) /   /* 16 */
            div_0_to_1(desat_th2 - desat_th1);
    }

    if (dm_cfg->hf_intp_blur_th_hig == dm_cfg->hf_intp_blur_th_low) {
        dm_cfg->hf_intp_blur_ratio = 0;
    } else {
        /* 16 is an accurary norm factor for parameter hf_intp_blur_ratio */
        /* 256 is an accurary norm factor for recipocal */
        dm_cfg->hf_intp_blur_ratio  =
        256 * 16 / div_0_to_1(dm_cfg->hf_intp_blur_th_hig - dm_cfg->hf_intp_blur_th_low); /* 256 *16 */
    }
}

static td_void demosaic_get_dynareg_auto_params(ot_vi_pipe vi_pipe, isp_demosaic_dyna_cfg *dm_cfg,
                                                isp_demosaic *demosaic)
{
    td_u8            idx_up, idx_lo, temp_de_f;
    td_u32           iso1, iso2, iso;
    isp_usr_ctx     *isp_ctx = TD_NULL;
    ot_isp_demosaic_auto_attr   *att_auto   = &demosaic->mpi_cfg.auto_attr;
    isp_get_ctx(vi_pipe, isp_ctx);
    iso = isp_ctx->linkage.iso;
    demosaic_get_iso_info(&iso1, &iso2, &idx_up, &idx_lo, iso);

    dm_cfg->ehc_gray   = (td_u8)linear_inter(iso, iso1, att_auto->nddm_mf_detail_strength[idx_lo],
                                             iso2, att_auto->nddm_mf_detail_strength[idx_up]);
    dm_cfg->dither_max = (td_u8)linear_inter(iso, iso1, att_auto->nddm_hf_detail_strength[idx_lo],
                                             iso2, att_auto->nddm_hf_detail_strength[idx_up]);
    dm_cfg->fake_cr_var_thr_low = (td_u16)linear_inter(iso, iso1, att_auto->color_noise_f_threshold[idx_lo],
                                                       iso2, att_auto->color_noise_f_threshold[idx_up]);
    temp_de_f          = (td_u8)linear_inter(iso, iso1, att_auto->color_noise_f_strength[idx_lo],
                                             iso2, att_auto->color_noise_f_strength[idx_up]);
    dm_cfg->de_f_color_cr = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - temp_de_f;
    dm_cfg->de_f_color_cb = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - temp_de_f;
    dm_cfg->color_noise_thd_y = (td_u8)linear_inter(iso, iso1, att_auto->color_noise_y_threshold[idx_lo],
                                                    iso2, att_auto->color_noise_y_threshold[idx_up]);
    dm_cfg->color_noise_str_y = (td_u8)linear_inter(iso, iso1, att_auto->color_noise_y_strength[idx_lo],
                                                    iso2, att_auto->color_noise_y_strength[idx_up]);
}

static td_void demosaic_get_dynareg_params(ot_vi_pipe vi_pipe, isp_demosaic_dyna_cfg *dm_cfg,
                                           isp_demosaic_static_cfg *static_dm_cfg, isp_demosaic *demosaic)
{
    td_u8            idx_up, idx_lo, filter_coef_index, nddm_str;
    td_u32           iso1, iso2, iso;
    isp_usr_ctx     *isp_ctx = TD_NULL;
    gf_blur_lut_cfg  gfblur_info_cfg;
    ot_isp_demosaic_auto_attr   *att_auto   = &demosaic->mpi_cfg.auto_attr;
    ot_isp_demosaic_manual_attr *att_manual = &demosaic->mpi_cfg.manual_attr;
    isp_get_ctx(vi_pipe, isp_ctx);
    iso = isp_ctx->linkage.iso;
    demosaic_get_iso_info(&iso1, &iso2, &idx_up, &idx_lo, iso);
    (td_void)memset_s(&gfblur_info_cfg, sizeof(gf_blur_lut_cfg), 0, sizeof(gf_blur_lut_cfg));
    if (demosaic->mpi_cfg.op_type == OT_OP_MODE_AUTO) {
        demosaic_get_dynareg_auto_params(vi_pipe, dm_cfg, demosaic);
        nddm_str = (td_u8)linear_inter(iso, iso1, att_auto->nddm_strength[idx_lo],
                                       iso2, att_auto->nddm_strength[idx_up]);
        filter_coef_index  = (td_u8)linear_inter(iso, iso1, att_auto->detail_smooth_range[idx_lo],
                                                 iso2, att_auto->detail_smooth_range[idx_up]);
    } else {
        nddm_str                    = att_manual->nddm_strength;
        filter_coef_index           = att_manual->detail_smooth_range;
        dm_cfg->ehc_gray            = att_manual->nddm_mf_detail_strength;
        dm_cfg->dither_max          = att_manual->nddm_hf_detail_strength;
        dm_cfg->fake_cr_var_thr_low = (td_u16)att_manual->color_noise_f_threshold;
        dm_cfg->de_f_color_cb       = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - att_manual->color_noise_f_strength;
        dm_cfg->de_f_color_cr       = OT_ISP_DEMOSAIC_COLORNOISE_STR_MAX - att_manual->color_noise_f_strength;
        dm_cfg->color_noise_thd_y   = att_manual->color_noise_y_threshold;
        dm_cfg->color_noise_str_y   = att_manual->color_noise_y_strength;
    }
    demosaic_dark_pro_cfg(dm_cfg);
    demosaic_cfg(dm_cfg, static_dm_cfg, demosaic, iso);
    gfblur_info_cfg.iso = iso;
    gfblur_info_cfg.vi_pipe = vi_pipe;
    gfblur_info_cfg.non_dir_str = nddm_str;
    demosaic_gf_blur_lut(demosaic, &gfblur_info_cfg, dm_cfg);
    filter_coef_index = clip3(filter_coef_index, 1, DEMOSAIC_FILTER_NUM);
    dm_cfg->lpff0 = g_demosaic_filtcoef[filter_coef_index - 1][0]; /* lpff0 */
    dm_cfg->lpff1 = g_demosaic_filtcoef[filter_coef_index - 1][1]; /* lpff1 */
    dm_cfg->lpff2 = g_demosaic_filtcoef[filter_coef_index - 1][2]; /* lpff2 */
    dm_cfg->resh  = TD_TRUE;
}

static td_void demosaic_get_usr_reg_params(isp_demosaic_usr_cfg *usr_reg_cfg, isp_demosaic *demosaic)
{
    usr_reg_cfg->ai_detail_strength = demosaic->mpi_cfg.ai_detail_strength;
    usr_reg_cfg->resh = TD_TRUE;
    usr_reg_cfg->update_index += 1;
}

static td_void demosaic_get_actual_params(ot_vi_pipe vi_pipe, isp_demosaic_dyna_cfg *dm_cfg, isp_demosaic *demosaic)
{
    td_u8         non_dir_str, filter_coef_index, temp_de_f;
    td_u8         idx_up, idx_lo;
    td_u32        iso1, iso2, iso;
    isp_usr_ctx  *isp_ctx = TD_NULL;
    ot_isp_demosaic_auto_attr *att_auto = &demosaic->mpi_cfg.auto_attr;

    isp_get_ctx(vi_pipe, isp_ctx);
    iso = isp_ctx->linkage.iso;
    demosaic_get_iso_info(&iso1, &iso2, &idx_up, &idx_lo, iso);

    if (demosaic->mpi_cfg.op_type == OT_OP_MODE_AUTO) {
        non_dir_str       = (td_u8)linear_inter(iso, iso1, att_auto->nddm_strength[idx_lo],
                                                iso2, att_auto->nddm_strength[idx_up]);
        filter_coef_index = (td_u8)linear_inter(iso, iso1, att_auto->detail_smooth_range[idx_lo],
                                                iso2, att_auto->detail_smooth_range[idx_up]);
        temp_de_f         = (td_u8)linear_inter(iso, iso1, att_auto->color_noise_f_strength[idx_lo],
                                                iso2, att_auto->color_noise_f_strength[idx_up]);
    } else {
        non_dir_str       = demosaic->mpi_cfg.manual_attr.nddm_strength;
        filter_coef_index = demosaic->mpi_cfg.manual_attr.detail_smooth_range;
        temp_de_f         = demosaic->mpi_cfg.manual_attr.color_noise_f_strength;
    }

    demosaic->actual.color_noise_f_strength  = temp_de_f;
    demosaic->actual.nddm_strength           = non_dir_str;
    demosaic->actual.detail_smooth_range     = filter_coef_index;
    demosaic->actual.nddm_mf_detail_strength = dm_cfg->ehc_gray;
    demosaic->actual.nddm_hf_detail_strength = dm_cfg->dither_max;
    demosaic->actual.color_noise_f_threshold = dm_cfg->fake_cr_var_thr_low;
    demosaic->actual.color_noise_y_threshold = dm_cfg->color_noise_thd_y;
    demosaic->actual.color_noise_y_strength  = dm_cfg->color_noise_str_y;
}

td_void isp_demosaic_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg, isp_demosaic *demosaic)
{
    td_u8 i;
    isp_demosaic_dyna_cfg *dm_cfg  = TD_NULL;
    isp_demosaic_static_cfg *static_dm_cfg = TD_NULL;

    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        dm_cfg = &reg->alg_reg_cfg[i].dem_reg_cfg.dyna_reg_cfg;
        static_dm_cfg = &reg->alg_reg_cfg[i].dem_reg_cfg.static_reg_cfg;
        demosaic_get_dynareg_params(vi_pipe, dm_cfg, static_dm_cfg, demosaic);
    }

    if (demosaic->dem_attr_update == TD_TRUE) {
        for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
            demosaic_get_usr_reg_params(&reg->alg_reg_cfg[i].dem_reg_cfg.usr_reg_cfg, demosaic);
        }
    }

    demosaic_get_actual_params(vi_pipe, &reg->alg_reg_cfg[0].dem_reg_cfg.dyna_reg_cfg, demosaic);
}

static td_bool check_demosaic_open(const isp_demosaic *demosaic)
{
    return (demosaic->mpi_cfg.enable == TD_TRUE);
}

static td_s32 isp_demosaic_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8  i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_DEMOSAIC;
    isp_reg_cfg *reg = (isp_reg_cfg *)reg_cfg;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_demosaic *demosaic = TD_NULL;
    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    demosaic_get_ctx(vi_pipe, demosaic);
    isp_check_pointer_return(demosaic);

    ot_ext_system_isp_demosaic_init_status_write(vi_pipe, demosaic->init);
    if (demosaic->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    isp_demosaic_set_long_frame_mode(vi_pipe, demosaic);

    if ((isp_ctx->frame_cnt % 2 != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) { /* proc every 2 frames */
        return TD_SUCCESS;
    }

    demosaic->mpi_cfg.enable = ot_ext_system_demosaic_enable_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].dem_reg_cfg.vhdm_enable = demosaic->mpi_cfg.enable;
        reg->alg_reg_cfg[i].dem_reg_cfg.nddm_enable = demosaic->mpi_cfg.enable;
    }

    reg->cfg_key.bit1_dem_cfg = 1;

    /* check hardware setting */
    if (!check_demosaic_open(demosaic)) {
        return TD_SUCCESS;
    }

    isp_demosaic_read_extregs(vi_pipe, demosaic);

    isp_demosaic_reg_update(vi_pipe, reg, demosaic);

    return TD_SUCCESS;
}

static td_s32 isp_demosaic_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_demosaic *demosaic = TD_NULL;
    isp_reg_cfg_attr *local_reg_cfg = TD_NULL;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, local_reg_cfg);
            isp_check_pointer_return(local_reg_cfg);
            isp_demosaic_wdr_mode_set(vi_pipe, (td_void *)&local_reg_cfg->reg_cfg);
            break;

        case OT_ISP_PROC_WRITE:
            demosaic_get_ctx(vi_pipe, demosaic);
            isp_check_pointer_return(demosaic);
            demosaic_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value, demosaic);
            break;

        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_demosaic_exit(ot_vi_pipe vi_pipe)
{
    td_u16 i;
    isp_reg_cfg_attr *local_reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, local_reg_cfg);
    ot_ext_system_isp_demosaic_init_status_write(vi_pipe, TD_FALSE);

    for (i = 0; i < local_reg_cfg->reg_cfg.cfg_num; i++) {
        local_reg_cfg->reg_cfg.alg_reg_cfg[i].dem_reg_cfg.nddm_enable = TD_FALSE;
        local_reg_cfg->reg_cfg.alg_reg_cfg[i].dem_reg_cfg.vhdm_enable = TD_FALSE;
    }

    local_reg_cfg->reg_cfg.cfg_key.bit1_dem_cfg = 1;

    demosaic_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_demosaic(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_demosaic);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_DEMOSAIC;
    algs->alg_func.pfn_alg_init = isp_demosaic_init;
    algs->alg_func.pfn_alg_run  = isp_demosaic_run;
    algs->alg_func.pfn_alg_ctrl = isp_demosaic_ctrl;
    algs->alg_func.pfn_alg_exit = isp_demosaic_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
