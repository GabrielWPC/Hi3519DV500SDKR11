/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "isp_csc.h"
#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"

#define ISP_CSC_DFT_LUMA 50
#define ISP_CSC_DFT_CON  50
#define ISP_CSC_DFT_HUE  50
#define ISP_CSC_DFT_SAT  50
#define ISP_CSC_SIN_TABLE_NUM 61

/* BT.709 -> RGB coefficient matrix */
static ot_isp_csc_matrx g_csc_attr_601 = {
    /* csc input DC component (IDC) */
    {0, 0, 0},
    /* csc output DC component (ODC) */
    {0, 128, 128},
    /* csc multiplier coefficient */
    {299, 587, 114, -172, -339, 511, 511, -428, -83},
};

/* BT.709 -> RGB coefficient matrix */
static ot_isp_csc_matrx g_csc_attr_601_limited = {
    /* csc input DC component (IDC) */
    {0, 0, 0},
    /* csc output DC component (ODC) */
    {16, 128, 128},
    /* csc multiplier coefficient */
    {256, 503, 98, -148, -291, 439, 439, -368, -71},
};

/* RGB -> YUV BT.709 coefficient matrix */
static ot_isp_csc_matrx g_csc_attr_709 = {
    /* csc input DC component (IDC) */
    {0, 0, 0},
    /* csc output DC component (ODC) */
    {0, 128, 128},
    /* csc multiplier coefficient */
    {213, 715, 72, -117, -394, 511, 511, -464, -47},
};

/* BT.709 -> RGB coefficient matrix */
static ot_isp_csc_matrx g_csc_attr_709_limited = {
    /* csc input DC component (IDC) */
    {0, 0, 0},
    /* csc output DC component (ODC) */
    {16, 128, 128},
    /* csc multiplier coefficient */
    {182, 614, 62, -101, -338, 439, 439, -399, -40},
};

static const int g_csc_sin_table[ISP_CSC_SIN_TABLE_NUM] = {
    -500,  -485,  -469,  -454,  -438,  -422,  -407,  -391,  -374,  -358,
    -342,  -325,  -309,  -292,  -276,  -259,  -242,  -225,  -208,  -191,
    -174,  -156,  -139,  -122,  -104,   -87,   -70,   -52,   -35,   -17,
    0,    17,    35,    52,    70,    87,   104,   122,   139,   156,
    174,   191,   208,   225,   242,   259,   276,   292,   309,   325,
    342,   358,   374,   391,   407,   422,   438,   454,   469,   485,
    500
};

static const int g_csc_cos_table[ISP_CSC_SIN_TABLE_NUM] = {
    866,   875,   883,   891,   899,   906,   914,   921,   927,   934,
    940,   946,   951,   956,   961,   966,   970,   974,   978,   982,
    985,   988,   990,   993,   995,   996,   998,   999,   999,  1000,
    1000,  1000,  999,   999,   998,   996,   995,   993,   990,   988,
    985,   982,   978,   974,   970,   966,   961,   956,   951,   946,
    940,   934,   927,   921,   914,   906,   899,   891,   883,   875,
    866
};

static isp_csc g_csc_ctx[OT_ISP_MAX_PIPE_NUM];

#define csc_get_ctx(pipe, ctx)            ctx = &g_csc_ctx[pipe]

/* CSC reg write interface */
static td_void csc_dyna_regs_config(const ot_isp_csc_matrx *csc_attr, isp_csc_dyna_cfg *dyna_reg_cfg)
{
    td_u8 i;
    const td_u16 norm_coeff1 = 1024;
    const td_u16 norm_coeff2 = 1000;

    for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
        dyna_reg_cfg->csc_coef[i] = csc_attr->csc_coef[i] * norm_coeff1 / norm_coeff2;
    }

    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        dyna_reg_cfg->csc_in_dc[i] = (td_s16)(csc_attr->csc_in_dc[i] * 4); /* multiply by 4 bits to fit */
        dyna_reg_cfg->csc_out_dc[i] = (td_s16)(csc_attr->csc_out_dc[i] * 4); /* multiply by 4 bits to fit */
    }

    dyna_reg_cfg->resh = TD_TRUE;
}

static td_void csc_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    /* csc regs initialize */
    td_u8 i;
    ot_isp_csc_matrx csc_attr;
    ot_unused(vi_pipe);

    /* initialize color gamut information */
    (td_void)memcpy_s(&csc_attr, sizeof(ot_isp_csc_matrx), &g_csc_attr_709, sizeof(ot_isp_csc_matrx));

    /* config all four csc attributes. */
    for (i = 0; i < reg_cfg->cfg_num; i++) {
        /* coefficiets config */
        csc_dyna_regs_config(&csc_attr, &reg_cfg->alg_reg_cfg[i].csc_cfg.dyna_reg_cfg);

        reg_cfg->alg_reg_cfg[i].csc_cfg.enable = TD_TRUE;
    }

    /* refresh flags are forced on */
    reg_cfg->cfg_key.bit1_csc_cfg = TD_TRUE;
}

static td_void csc_ext_regs_initialize(ot_vi_pipe vi_pipe)
{
    td_u8 i;

    ot_isp_csc_matrx csc_attr;
    ot_ext_system_csc_gamuttype_write(vi_pipe, OT_COLOR_GAMUT_BT709);
    (td_void)memcpy_s(&csc_attr, sizeof(ot_isp_csc_matrx), &g_csc_attr_709, sizeof(ot_isp_csc_matrx));

    /* write csc related parameters */
    ot_ext_system_csc_hue_write(vi_pipe, ISP_CSC_DFT_HUE);
    ot_ext_system_csc_sat_write(vi_pipe, ISP_CSC_DFT_SAT);
    ot_ext_system_csc_contrast_write(vi_pipe, ISP_CSC_DFT_CON);
    ot_ext_system_csc_luma_write(vi_pipe, ISP_CSC_DFT_LUMA);
    ot_ext_system_csc_limitrange_en_write(vi_pipe, TD_FALSE);
    ot_ext_system_csc_ext_en_write(vi_pipe, TD_TRUE);
    ot_ext_system_csc_ctmode_en_write(vi_pipe, TD_TRUE);

    /* write DC components */
    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        ot_ext_system_csc_dcin_write(vi_pipe, i,  csc_attr.csc_in_dc[i]);
        ot_ext_system_csc_dcout_write(vi_pipe, i, csc_attr.csc_out_dc[i]);
    }

    /* write coefficients */
    for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
        ot_ext_system_csc_coef_write(vi_pipe, i, csc_attr.csc_coef[i]);
    }

    ot_ext_system_csc_enable_write(vi_pipe, TD_TRUE);
    ot_ext_system_csc_attr_update_write(vi_pipe, TD_TRUE);
}

td_void isp_csc_read_ext_regs(ot_vi_pipe vi_pipe, isp_csc *csc)
{
    td_u8 i;
    td_u8 gamut;

    csc->update = ot_ext_system_csc_attr_update_read(vi_pipe);
    if (csc->update) {
        ot_ext_system_csc_attr_update_write(vi_pipe, TD_FALSE);
        csc->mpi_cfg.limited_range_en = ot_ext_system_csc_limitrange_en_read(vi_pipe);
        csc->mpi_cfg.ext_csc_en = ot_ext_system_csc_ext_en_read(vi_pipe);
        csc->mpi_cfg.ct_mode_en = ot_ext_system_csc_ctmode_en_read(vi_pipe);

        gamut = ot_ext_system_csc_gamuttype_read(vi_pipe);
        if (gamut == OT_COLOR_GAMUT_BT2020) {
            isp_warn_trace("invalid color gamut type, set back to previous gamut type!\n");
        } else {
            csc->mpi_cfg.color_gamut = gamut;
        }

        csc->mpi_cfg.contr = ot_ext_system_csc_contrast_read(vi_pipe);
        csc->mpi_cfg.hue   = ot_ext_system_csc_hue_read(vi_pipe);
        csc->mpi_cfg.satu  = ot_ext_system_csc_sat_read(vi_pipe);
        csc->mpi_cfg.luma = ot_ext_system_csc_luma_read(vi_pipe);

        for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
            csc->mpi_cfg.csc_magtrx.csc_coef[i] = ot_ext_system_csc_coef_read(vi_pipe, i);
        }

        /* DC components read */
        for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
            csc->mpi_cfg.csc_magtrx.csc_in_dc[i] = (td_s16)ot_ext_system_csc_dcin_read(vi_pipe, i);
            csc->mpi_cfg.csc_magtrx.csc_out_dc[i] = (td_s16)ot_ext_system_csc_dcout_read(vi_pipe, i);
        }
    }
}

static td_s32 csc_cal_matrix(ot_vi_pipe vi_pipe, isp_csc *csc, ot_isp_csc_matrx *csc_coef)
{
    td_s32 luma = 0;
    td_s32 i, contrast, satu, hue;
    td_s16 black_in, black_out;
    const td_s16 norm_factor2 = 100;
    const td_s16 norm_factor3 = 1000;
    const td_s16 norm_factor4 = 10000;
    ot_isp_csc_matrx *csc_coef_tmp = TD_NULL;

    if (csc->mpi_cfg.ext_csc_en) {
        luma = (td_s32)csc->mpi_cfg.luma - 50;  /* -50 ~ 50 */
        luma = luma * 0x80 / 50;  /* -128 ~ 128 */ /* convert by 50 */
        luma = (luma == 128) ? 127 : luma; /* max value should be 127(7bits) , not 128(8bits) */
    } else {
        luma = (td_s16)csc->mpi_cfg.luma * 64 / 100 - 32; /* convert from [0, 100] to [-32, 32], use 64 as norm */
        luma = clip3(luma, -128, 128); /* limit the result of luma, ,[-128, 128] */
    }

    contrast = ((td_s16)csc->mpi_cfg.contr - 50) * 2 + 100; /*  contrast from [0, 100] to [0, 200], use 50 to convert */
    hue  = (td_s16)csc->mpi_cfg.hue * 60 / 100; /* convert hue from [0, 100] to [0, 60] */
    hue  = clip3(hue, 0, 60); /* hue range in [0, 60] */
    satu = ((td_s16)csc->mpi_cfg.satu - 50) * 2 + 100; /* convert satu from [0, 100] to [0, 200], use 50 to convert */

    switch (csc->mpi_cfg.color_gamut) {
        case OT_COLOR_GAMUT_BT601:
            csc_coef_tmp  = (csc->mpi_cfg.limited_range_en) ? &g_csc_attr_601_limited : &g_csc_attr_601;
            break;
        case OT_COLOR_GAMUT_BT709:
            csc_coef_tmp  = (csc->mpi_cfg.limited_range_en) ? &g_csc_attr_709_limited : &g_csc_attr_709;
            break;
        case OT_COLOR_GAMUT_USER:
            csc_coef_tmp  = &(csc->mpi_cfg.csc_magtrx);
            csc_coef_tmp->csc_out_dc[0] = (csc->mpi_cfg.limited_range_en) ? 16 : 0; /* 16 as DC value */
            break;
        default:
            isp_err_trace("invalid color gamut type!\n");
            return TD_FAILURE;
    }

    if ((csc->mpi_cfg.ct_mode_en) && (csc->mpi_cfg.color_gamut <= OT_COLOR_GAMUT_USER)) {
        black_in  = -128; /* -128 as default input dc value */
        /* 110 = (128 * 219) / 256, full range to limited range conversion */
        black_out = (csc->mpi_cfg.limited_range_en) ? 110 : 127; /* 110 equal (129*219)/256, 127 is default dc value */
    } else {
        black_in  = 0;
        black_out = 0;
    }

    csc_coef->csc_coef[0] = (td_s16)((contrast * ((td_s32)csc_coef_tmp->csc_coef[0])) / norm_factor2);
    csc_coef->csc_coef[1] = (td_s16)((contrast * ((td_s32)csc_coef_tmp->csc_coef[1])) / norm_factor2);
    csc_coef->csc_coef[2] = (td_s16)((contrast * ((td_s32)csc_coef_tmp->csc_coef[2])) / norm_factor2); /* [2] */
    csc_coef->csc_coef[3] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[3] * /* 3 3 */
                                                        g_csc_cos_table[hue] + csc_coef_tmp->csc_coef[6] * /* 6 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);
    csc_coef->csc_coef[4] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[4] * /* 4 4 */
                                                        g_csc_cos_table[hue] + csc_coef_tmp->csc_coef[7] * /* 7 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);
    csc_coef->csc_coef[5] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[5] * /* 5 5 */
                                                        g_csc_cos_table[hue] + csc_coef_tmp->csc_coef[8] * /* 8 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);
    csc_coef->csc_coef[6] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[6] * /* 6 6 */
                                                        g_csc_cos_table[hue] - csc_coef_tmp->csc_coef[3] * /* 3 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);
    csc_coef->csc_coef[7] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[7] * /* 7 7 */
                                                        g_csc_cos_table[hue] - csc_coef_tmp->csc_coef[4] * /* 4 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);
    csc_coef->csc_coef[8] = (td_s16)((contrast * satu * ((td_s32)(csc_coef_tmp->csc_coef[8] * /* 8 8 */
                                                        g_csc_cos_table[hue] - csc_coef_tmp->csc_coef[5] *  /* 5 */
                                                        g_csc_sin_table[hue]) / norm_factor3)) / norm_factor4);

    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        if (csc->mpi_cfg.color_gamut == OT_COLOR_GAMUT_USER) {
            csc_coef_tmp->csc_in_dc[i] = 0;
        }
        csc_coef->csc_in_dc[i] = csc_coef_tmp->csc_in_dc[i] + black_in;
        csc_coef->csc_out_dc[i] = csc_coef_tmp->csc_out_dc[i];
    }

    /* add luma */
    csc_coef->csc_out_dc[0] += ((td_s16)luma + black_out);

    /* update ext regs: */
    for (i = 0; i < OT_ISP_CSC_DC_NUM; i++) {
        ot_ext_system_csc_dcin_write(vi_pipe, i, (td_u16)csc_coef->csc_in_dc[i]);
        ot_ext_system_csc_dcout_write(vi_pipe, i, csc_coef->csc_out_dc[i]);
    }

    if (csc->mpi_cfg.color_gamut != OT_COLOR_GAMUT_USER) {
        for (i = 0; i < OT_ISP_CSC_COEF_NUM; i++) {
            ot_ext_system_csc_coef_write(vi_pipe, i, csc_coef->csc_coef[i]);
        }
    }

    return TD_SUCCESS;
}

static __inline td_bool  check_csc_open(const isp_csc *csc)
{
    return (csc->mpi_cfg.enable == TD_TRUE);
}

static td_s32 csc_proc_write(ot_vi_pipe vi_pipe, const ot_isp_ctrl_proc_write *proc)
{
    ot_unused(vi_pipe);
    ot_unused(proc);
    return TD_SUCCESS;
}

static td_s32 csc_color_gamutinfo_get(ot_vi_pipe vi_pipe,  ot_isp_colorgammut_info *color_gamut_info)
{
    isp_csc *csc = TD_NULL;

    csc_get_ctx(vi_pipe, csc);

    color_gamut_info->color_gamut = csc->mpi_cfg.color_gamut;

    return TD_SUCCESS;
}

td_s32 isp_csc_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_csc *csc)
{
    td_u8  i;
    td_s32 ret;
    ot_isp_csc_matrx csc_attr;

    /* calculate actual csc coefs */
    ret = csc_cal_matrix(vi_pipe, csc, &csc_attr);
    if (ret == TD_FAILURE) {
        return TD_FAILURE;
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        /* coefficiets config */
        csc_dyna_regs_config(&csc_attr, &reg_cfg->alg_reg_cfg[i].csc_cfg.dyna_reg_cfg);
    }

    /* refresh flags are forced on */
    reg_cfg->cfg_key.bit1_csc_cfg = TD_TRUE;

    return TD_SUCCESS;
}

td_void isp_csc_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    csc_regs_initialize(vi_pipe, reg_cfg);
    csc_ext_regs_initialize(vi_pipe);
}

static td_s32 isp_csc_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    isp_csc_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_csc_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_info, td_s32 rsv)
{
    td_u16 i;
    isp_csc      *csc     = TD_NULL;
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_reg_cfg  *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    csc_get_ctx(vi_pipe, csc);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    csc->mpi_cfg.enable = ot_ext_system_csc_enable_read(vi_pipe);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].csc_cfg.enable = csc->mpi_cfg.enable;
    }

    reg_cfg->cfg_key.bit1_csc_cfg = 1;

    /* check hardware setting */
    if (!check_csc_open(csc)) {
        return TD_SUCCESS;
    }

    /* read ext regs */
    isp_csc_read_ext_regs(vi_pipe, csc);

    return isp_csc_reg_update(vi_pipe, reg_cfg, csc);
}

static td_s32 isp_csc_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    switch (cmd) {
        case OT_ISP_PROC_WRITE:
            csc_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        case OT_ISP_COLORGAMUTINFO_GET:
            csc_color_gamutinfo_get(vi_pipe, (ot_isp_colorgammut_info *)value);
            break;

        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_csc_exit(ot_vi_pipe vi_pipe)
{
    ot_unused(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_csc(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_csc);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_CSC;
    algs->alg_func.pfn_alg_init = isp_csc_init;
    algs->alg_func.pfn_alg_run  = isp_csc_run;
    algs->alg_func.pfn_alg_ctrl = isp_csc_ctrl;
    algs->alg_func.pfn_alg_exit = isp_csc_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
