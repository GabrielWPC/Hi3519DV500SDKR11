/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "isp_sharpen.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"
#include "isp_param_check.h"
#include "isp_ext_reg_access.h"

#define  SHRP_SHT_VAR_MUL_PRECS     4
#define  SHRP_SKIN_EDGE_MUL_PRECS   4
#define  SHRP_CHR_MUL_SFT           4
#define  SHRP_DETAIL_SHT_MUL_PRECS  4
#define  SHRP_DETAIL_CTRL_THR_DELTA 16
#define  SHRP_ISO_NUM               OT_ISP_AUTO_ISO_NUM

static isp_sharpen_ctx *g_sharpen_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define sharpen_get_ctx(pipe, ctx)   ((ctx) = g_sharpen_ctx[pipe])
#define sharpen_set_ctx(pipe, ctx)   (g_sharpen_ctx[pipe] = (ctx))
#define sharpen_reset_ctx(pipe)      (g_sharpen_ctx[pipe] = TD_NULL)

static td_s32 shrp_blend(td_u8 sft, td_s32 wgt1, td_s32 v1, td_s32 wgt2, td_s32 v2)
{
    td_s32 res;
    res = signed_right_shift(((td_s64)v1 * wgt1) + ((td_s64)v2 * wgt2), sft);
    return res;
}

static td_s32 sharpen_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_sharpen_ctx *sharpen_ctx = TD_NULL;

    sharpen_get_ctx(vi_pipe, sharpen_ctx);

    if (sharpen_ctx == TD_NULL) {
        sharpen_ctx = (isp_sharpen_ctx *)isp_malloc(sizeof(isp_sharpen_ctx));
        if (sharpen_ctx == TD_NULL) {
            isp_err_trace("isp[%d] sharpen_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(sharpen_ctx, sizeof(isp_sharpen_ctx), 0, sizeof(isp_sharpen_ctx));

    sharpen_set_ctx(vi_pipe, sharpen_ctx);

    return TD_SUCCESS;
}

static td_void sharpen_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_sharpen_ctx *sharpen_ctx = TD_NULL;

    sharpen_get_ctx(vi_pipe, sharpen_ctx);
    isp_free(sharpen_ctx);
    sharpen_reset_ctx(vi_pipe);
}

static td_s32 sharpen_check_cmos_param(const ot_isp_sharpen_attr *sharpen)
{
    td_s32 ret;

    ret = isp_sharpen_comm_attr_check("cmos", sharpen);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_sharpen_auto_attr_check("cmos", &sharpen->auto_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_sharpen_manual_attr_check("cmos", &sharpen->manual_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void sharpen_ctx_def_comm_initialize(ot_isp_sharpen_attr *mpi_cfg)
{
    mpi_cfg->enable        = TD_TRUE;
    mpi_cfg->motion_en = TD_FALSE;
    mpi_cfg->op_type   = OT_OP_MODE_AUTO;
    mpi_cfg->detail_map = OT_ISP_SHARPEN_NORMAL;
    mpi_cfg->skin_umax = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINUMAX_DEF;
    mpi_cfg->skin_umin = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINUMIN_DEF;
    mpi_cfg->skin_vmin = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINVMIN_DEF;
    mpi_cfg->skin_vmax = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINVMAX_DEF;
    mpi_cfg->motion_threshold0 = 2; /* def motion_threshold0: 2 */
    mpi_cfg->motion_threshold1 = 8; /* def motion_threshold1: 8 */
    mpi_cfg->motion_gain0 = 0;      /* def motion_gain0: 0 */
    mpi_cfg->motion_gain1 = 256;    /* def motion_gain1: 256 */
}

static td_void sharpen_ctx_def_auto_initialize(ot_isp_sharpen_auto_attr *auto_ctx)
{
    td_u8 i, j;
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        for (j = 0; j < OT_ISP_SHARPEN_GAIN_NUM; j++) {
            auto_ctx->texture_strength[j][i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTURESTR_DEF;
            auto_ctx->edge_strength[j][i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGESTR_DEF;
            auto_ctx->motion_texture_strength[j][i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTURESTR_DEF;
            auto_ctx->motion_edge_strength[j][i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGESTR_DEF;
        }
        for (j = 0; j < OT_ISP_SHARPEN_LUMA_NUM; j++) {
            auto_ctx->luma_wgt[j][i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_LUMAWGT_DEF;
        }

        auto_ctx->texture_freq[i]       = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTUREFREQ_DEF;
        auto_ctx->edge_freq[i]          = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFREQ_DEF;
        auto_ctx->over_shoot[i]         = OT_EXT_SYSTEM_MANUAL_SHARPEN_OVERSHOOT_DEF;
        auto_ctx->under_shoot[i]        = OT_EXT_SYSTEM_MANUAL_SHARPEN_UNDERSHOOT_DEF;
        auto_ctx->motion_texture_freq[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTUREFREQ_DEF;
        auto_ctx->motion_edge_freq[i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFREQ_DEF;
        auto_ctx->motion_over_shoot[i]   = OT_EXT_SYSTEM_MANUAL_SHARPEN_OVERSHOOT_DEF;
        auto_ctx->motion_under_shoot[i]  = OT_EXT_SYSTEM_MANUAL_SHARPEN_UNDERSHOOT_DEF;
        auto_ctx->shoot_sup_strength[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_SHOOTSUPSTR_DEF;
        auto_ctx->detail_ctrl[i]        = OT_EXT_SYSTEM_MANUAL_SHARPEN_DETAILCTRL_DEF;
        auto_ctx->edge_filt_strength[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFILTSTR_DEF;
        auto_ctx->edge_filt_max_cap[i]  = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFILTMAXCAP_DEF;
        auto_ctx->r_gain[i]             = OT_EXT_SYSTEM_MANUAL_SHARPEN_RGAIN_DEF;
        auto_ctx->g_gain[i]             = OT_EXT_SYSTEM_MANUAL_SHARPEN_GGAIN_DEF;
        auto_ctx->b_gain[i]             = OT_EXT_SYSTEM_MANUAL_SHARPEN_BGAIN_DEF;
        auto_ctx->skin_gain[i]          = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINGAIN_DEF;
        auto_ctx->shoot_sup_adj[i]      = OT_EXT_SYSTEM_MANUAL_SHARPEN_SHOOTSUPADJ_DEF;
        auto_ctx->max_sharp_gain[i]     = OT_EXT_SYSTEM_MANUAL_SHARPEN_MAXSHARPGAIN_DEF;
        auto_ctx->detail_ctrl_threshold[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_DETAILCTRLTHR_DEF;
    }
}

static td_void sharpen_ctx_def_manual_initialize(ot_isp_sharpen_manual_attr *manual_ctx)
{
    td_u8 i;

    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        manual_ctx->texture_strength[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTURESTR_DEF;
        manual_ctx->edge_strength[i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGESTR_DEF;
        manual_ctx->motion_texture_strength[i] = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTURESTR_DEF;
        manual_ctx->motion_edge_strength[i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGESTR_DEF;
    }
    for (i = 0; i < OT_ISP_SHARPEN_LUMA_NUM; i++) {
        manual_ctx->luma_wgt[i]    = OT_EXT_SYSTEM_MANUAL_SHARPEN_LUMAWGT_DEF;
    }
    manual_ctx->texture_freq        = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTUREFREQ_DEF;
    manual_ctx->edge_freq           = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFREQ_DEF;
    manual_ctx->over_shoot          = OT_EXT_SYSTEM_MANUAL_SHARPEN_OVERSHOOT_DEF;
    manual_ctx->under_shoot         = OT_EXT_SYSTEM_MANUAL_SHARPEN_UNDERSHOOT_DEF;
    manual_ctx->motion_texture_freq = OT_EXT_SYSTEM_MANUAL_SHARPEN_TEXTUREFREQ_DEF;
    manual_ctx->motion_edge_freq    = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFREQ_DEF;
    manual_ctx->motion_over_shoot   = OT_EXT_SYSTEM_MANUAL_SHARPEN_OVERSHOOT_DEF;
    manual_ctx->motion_under_shoot  = OT_EXT_SYSTEM_MANUAL_SHARPEN_UNDERSHOOT_DEF;
    manual_ctx->shoot_sup_strength  = OT_EXT_SYSTEM_MANUAL_SHARPEN_SHOOTSUPSTR_DEF;
    manual_ctx->detail_ctrl         = OT_EXT_SYSTEM_MANUAL_SHARPEN_DETAILCTRL_DEF;
    manual_ctx->edge_filt_strength  = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFILTSTR_DEF;
    manual_ctx->edge_filt_max_cap   = OT_EXT_SYSTEM_MANUAL_SHARPEN_EDGEFILTMAXCAP_DEF;
    manual_ctx->r_gain              = OT_EXT_SYSTEM_MANUAL_SHARPEN_RGAIN_DEF;
    manual_ctx->g_gain              = OT_EXT_SYSTEM_MANUAL_SHARPEN_GGAIN_DEF;
    manual_ctx->b_gain              = OT_EXT_SYSTEM_MANUAL_SHARPEN_BGAIN_DEF;
    manual_ctx->skin_gain           = OT_EXT_SYSTEM_MANUAL_SHARPEN_SKINGAIN_DEF;
    manual_ctx->shoot_sup_adj       = OT_EXT_SYSTEM_MANUAL_SHARPEN_SHOOTSUPADJ_DEF;
    manual_ctx->max_sharp_gain      = OT_EXT_SYSTEM_MANUAL_SHARPEN_MAXSHARPGAIN_DEF;
    manual_ctx->detail_ctrl_threshold = OT_EXT_SYSTEM_MANUAL_SHARPEN_DETAILCTRLTHR_DEF;
}

static td_s32 isp_sharpen_ctx_initialize(ot_vi_pipe vi_pipe, isp_sharpen_ctx *shp_ctx)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    shp_ctx->sharpen_mpi_update_en = TD_TRUE;
    shp_ctx->iso_last              = 0;

    shp_ctx->gain_thd_sft_d    = 0;
    shp_ctx->dir_var_sft       = 10;  /* dir var sft        10 */
    shp_ctx->sel_pix_wgt       = 31;  /* sel pix wgt        31 */
    shp_ctx->dir_rly_thrhigh   = 120; /* dir rly thr high  120 */
    shp_ctx->dir_rly_thr_low   = 2;   /* dir rly thr low     2 */

    shp_ctx->rmf_gain_scale    = 2;   /* rmf gain scale      2 */
    shp_ctx->bmf_gain_scale    = 4;   /* bmf gain scale      4 */
    shp_ctx->gain_thd_sel_ud   = 2;   /* mf thd sel ud       2 */
    shp_ctx->gain_thd_sft_ud   = 0;   /* mf thd sft ud       0 */

    shp_ctx->sht_var_wgt0      = 10;  /* sht var wgt0       10 */
    shp_ctx->sht_var_diff_thd0 = 20;  /* sht var diff thd0  20 */
    shp_ctx->sht_var_diff_thd1 = 35;  /* sht var diff thd1  35 */
    shp_ctx->sht_var_diff_wgt1 = 27;  /* sht var diff wgt1  27 */

    /* auto ext_regs initial */
    if (sns_dft->key.bit1_sharpen) {
        isp_check_pointer_return(sns_dft->sharpen);

        ret = sharpen_check_cmos_param(sns_dft->sharpen);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        (td_void)memcpy_s(&shp_ctx->mpi_cfg, sizeof(ot_isp_sharpen_attr),
                          sns_dft->sharpen, sizeof(ot_isp_sharpen_attr));
    } else {
        sharpen_ctx_def_comm_initialize(&shp_ctx->mpi_cfg);
        sharpen_ctx_def_auto_initialize(&shp_ctx->mpi_cfg.auto_attr);
        sharpen_ctx_def_manual_initialize(&shp_ctx->mpi_cfg.manual_attr);
    }

    return TD_SUCCESS;
}

static td_void sharpen_ext_regs_initialize(ot_vi_pipe vi_pipe, isp_sharpen_ctx *shp_ctx)
{
    isp_sharpen_comm_attr_write(vi_pipe, &shp_ctx->mpi_cfg);
    isp_sharpen_auto_attr_write(vi_pipe, &shp_ctx->mpi_cfg.auto_attr);
    isp_sharpen_manual_attr_write(vi_pipe, &shp_ctx->mpi_cfg.manual_attr);

    ot_ext_system_sharpen_mpi_update_en_write(vi_pipe, shp_ctx->sharpen_mpi_update_en);
}

/* sharpen hardware regs that will not change */
static td_void sharpen_static_shoot_reg_init(isp_sharpen_static_reg_cfg *static_cfg)
{
    td_u8 i;
    static_cfg->static_resh      = TD_TRUE;
    static_cfg->gain_thd_sel_d    = 1;
    static_cfg->dir_var_scale     = 0;
    static_cfg->dir_rly[0]        = 127;  /* dir rly0   127 */
    static_cfg->dir_rly[1]        = 0;
    static_cfg->max_var_clip_min  = 3;    /* max var clip 3 */
    static_cfg->o_max_chg         = 1000; /* o max chg 1000 */
    static_cfg->u_max_chg         = 1000; /* u max chg 1000 */
    static_cfg->sht_var_sft       = 0;

    for (i = 0; i < ISP_SHARPEN_FREQ_CORING_LENGTH; i++) {
        static_cfg->lmt_mf[i]    = 0;
        static_cfg->lmt_hf[i]    = 0;
    }

    static_cfg->sht_var_wgt1      = 127; /* sht var wgt1 127 */
    static_cfg->sht_var_diff_wgt0 = 127; /* sht var wgt0 127 */
    static_cfg->sht_var_thd0      = 0;
    static_cfg->en_shp8_dir       = 1;

    static_cfg->lf_gain_wgt       = 14;  /* lf gain wgt 14 */
    static_cfg->hf_gain_sft       = 5;   /* hf gain sft  5 */
    static_cfg->mf_gain_sft       = 5;   /* mf gain sft  5 */
    static_cfg->sht_var_sel       = 1;
    static_cfg->sht_var5x5_sft    = 1;
    static_cfg->detail_thd_sel    = 0;
    static_cfg->detail_thd_sft    = 1;
}

static td_void sharpen_static_chrome_reg_init(isp_sharpen_static_reg_cfg *static_cfg)
{
    static_cfg->chr_r_var_sft    = 7;   /* chr rvar sft  7 */
    static_cfg->chr_r_ori_cb     = 120; /* chr rori cb 120 */
    static_cfg->chr_r_ori_cr     = 220; /* chr rori cr 220 */
    static_cfg->chr_r_sft[0]     = 7;   /* chr red sft0  7 */
    static_cfg->chr_r_sft[1]     = 7;   /* chr red sft1  7 */
    static_cfg->chr_r_sft[2]     = 7;   /* chr red sft2  7 */
    static_cfg->chr_r_sft[3]     = 6;   /* chr red sft3  6 */
    static_cfg->chr_r_thd[0]     = 40;  /* chr red thd0 40 */
    static_cfg->chr_r_thd[1]     = 60;  /* chr red thd1 60 */

    static_cfg->chr_b_var_sft    = 2;   /* chr bvar sft  2 */
    static_cfg->chr_b_ori_cb     = 200; /* chr bori cb 200 */
    static_cfg->chr_b_ori_cr     = 64;  /* chr bori cr 64  */
    static_cfg->chr_b_sft[0]     = 7;   /* chr blu sft0 7  */
    static_cfg->chr_b_sft[1]     = 7;   /* chr blu sft1 7  */
    static_cfg->chr_b_sft[2]     = 7;   /* chr blu sft2 7  */
    static_cfg->chr_b_sft[3]     = 7;   /* chr blu sft3 7  */
    static_cfg->chr_b_thd[0]     = 50;  /* chr bl thd0 50  */
    static_cfg->chr_b_thd[1]     = 100; /* chr bl thd1 100 */

    static_cfg->chr_g_ori_cb     = 90;  /* chr gori cb  90 */
    static_cfg->chr_g_ori_cr     = 110; /* chr gori cr 110 */
    static_cfg->chr_g_sft[0]     = 4;   /* chr gre sft0 4  */
    static_cfg->chr_g_sft[1]     = 7;   /* chr gre sft1 7  */
    static_cfg->chr_g_sft[2]     = 4;   /* chr gre sft2 4  */
    static_cfg->chr_g_sft[3]     = 7;   /* chr gre sft3 7  */
    static_cfg->chr_g_thd[0]     = 20;  /* chr gre thd0 20 */
    static_cfg->chr_g_thd[1]     = 40;  /* chr gre thd1 40 */
}

static td_void sharpen_static_hsf_reg_init(isp_sharpen_static_reg_cfg *static_cfg)
{
    static_cfg->hsf_coef_ud[0]   = -2;   /* hsf coef ud0  -2 */
    static_cfg->hsf_coef_ud[1]   = 9;    /* hsf coef ud1  9  */
    static_cfg->hsf_coef_ud[2]   = 18;   /* hsf coef ud2  18 */

    static_cfg->hsf_coef_d0[0]   = -1;   /* hsf coef d00  -1 */
    static_cfg->hsf_coef_d0[1]   = -2;   /* hsf coef d01  -2 */
    static_cfg->hsf_coef_d0[2]   = -3;   /* hsf coef d02  -3 */
    static_cfg->hsf_coef_d0[3]   = -2;   /* hsf coef d03  -2 */
    static_cfg->hsf_coef_d0[4]   = -1;   /* hsf coef d04  -1 */
    static_cfg->hsf_coef_d0[5]   = 12;   /* hsf coef d05  12 */
    static_cfg->hsf_coef_d0[6]   = 27;   /* hsf coef d06  27 */
    static_cfg->hsf_coef_d0[7]   = 36;   /* hsf coef d07  36 */
    static_cfg->hsf_coef_d0[8]   = 27;   /* hsf coef d08  27 */
    static_cfg->hsf_coef_d0[9]   = 12;   /* hsf coef d09  12 */
    static_cfg->hsf_coef_d0[10]  = 31;   /* hsf coef d010 31 */
    static_cfg->hsf_coef_d0[11]  = 72;   /* hsf coef d011 72 */
    static_cfg->hsf_coef_d0[12]  = 96;   /* hsf coef d012 96 */

    static_cfg->hsf_coef_d1[0]   = -2;   /* hsf coef d10  -2 */
    static_cfg->hsf_coef_d1[1]   = -3;   /* hsf coef d11  -3 */
    static_cfg->hsf_coef_d1[2]   = -1;   /* hsf coef d12  -1 */
    static_cfg->hsf_coef_d1[3]   = 6;    /* hsf coef d13  6  */
    static_cfg->hsf_coef_d1[4]   = 6;    /* hsf coef d14  6  */
    static_cfg->hsf_coef_d1[5]   = 0;    /* hsf coef d15  0  */
    static_cfg->hsf_coef_d1[6]   = 13;   /* hsf coef d16  13 */
    static_cfg->hsf_coef_d1[7]   = 41;   /* hsf coef d17  41 */
    static_cfg->hsf_coef_d1[8]   = 47;   /* hsf coef d18  47 */
    static_cfg->hsf_coef_d1[9]   = 23;   /* hsf coef d19  23 */
    static_cfg->hsf_coef_d1[10]  = 18;   /* hsf coef d110 18 */
    static_cfg->hsf_coef_d1[11]  = 62;   /* hsf coef d111 62 */
    static_cfg->hsf_coef_d1[12]  = 92;   /* hsf coef d112 92 */

    static_cfg->hsf_coef_d2[0]   = -1;   /* hsf coef d20  -1 */
    static_cfg->hsf_coef_d2[1]   = -3;   /* hsf coef d21  -3 */
    static_cfg->hsf_coef_d2[2]   = 6;    /* hsf coef d22  6  */
    static_cfg->hsf_coef_d2[3]   = 16;   /* hsf coef d23  16 */
    static_cfg->hsf_coef_d2[4]   = 10;   /* hsf coef d24  10 */
    static_cfg->hsf_coef_d2[5]   = -3;   /* hsf coef d25  -3 */
    static_cfg->hsf_coef_d2[6]   = 10;   /* hsf coef d26  10 */
    static_cfg->hsf_coef_d2[7]   = 50;   /* hsf coef d27  50 */
    static_cfg->hsf_coef_d2[8]   = 53;   /* hsf coef d28  53 */
    static_cfg->hsf_coef_d2[9]   = 16;   /* hsf coef d29  16 */
    static_cfg->hsf_coef_d2[10]  = 6;    /* hsf coef d210 6  */
    static_cfg->hsf_coef_d2[11]  = 50;   /* hsf coef d211 50 */
    static_cfg->hsf_coef_d2[12]  = 92;   /* hsf coef d212 92 */
}

static td_void sharpen_static_lpf_reg_init(isp_sharpen_static_reg_cfg *static_cfg)
{
    static_cfg->hsf_coef_d3[0]   = -2;   /* hsf coef d30  -2 */
    static_cfg->hsf_coef_d3[1]   = 0;    /* hsf coef d31  0  */
    static_cfg->hsf_coef_d3[2]   = 18;   /* hsf coef d32  18 */
    static_cfg->hsf_coef_d3[3]   = 23;   /* hsf coef d33  23 */
    static_cfg->hsf_coef_d3[4]   = 6;    /* hsf coef d34  6  */
    static_cfg->hsf_coef_d3[5]   = -3;   /* hsf coef d35  -3 */
    static_cfg->hsf_coef_d3[6]   = 13;   /* hsf coef d36  13 */
    static_cfg->hsf_coef_d3[7]   = 62;   /* hsf coef d37  62 */
    static_cfg->hsf_coef_d3[8]   = 47;   /* hsf coef d38  47 */
    static_cfg->hsf_coef_d3[9]   = 6;    /* hsf coef d39  6  */
    static_cfg->hsf_coef_d3[10]  = -1;   /* hsf coef d310 -1 */
    static_cfg->hsf_coef_d3[11]  = 41;   /* hsf coef d311 41 */
    static_cfg->hsf_coef_d3[12]  = 92;   /* hsf coef d312 92 */

    static_cfg->lpf_coef_ud[0]   = 5;    /* lpf coef ud0  5  */
    static_cfg->lpf_coef_ud[1]   = 7;    /* lpf coef ud1  7  */
    static_cfg->lpf_coef_ud[2]   = 8;    /* lpf coef ud2  8  */

    static_cfg->lpf_coef_d[0]    = 4;    /* lpf coef d0   4  */
    static_cfg->lpf_coef_d[1]    = 7;    /* lpf coef d1   7  */
    static_cfg->lpf_coef_d[2]    = 10;   /* lpf coef d2  10  */

    static_cfg->lpf_sft_ud       = 5;    /* lpf sft ud    5  */
    static_cfg->lpf_sft_d        = 5;    /* lpf sft d     5  */
    static_cfg->hsf_sft_ud       = 5;    /* hsf sft ud    5  */
    static_cfg->hsf_sft_d        = 9;    /* hsf sft d     9  */

    static_cfg->skin_src_sel     = 0;   /* skin src sel   0 */
    static_cfg->skin_cnt_thd[0]  = 5;   /* skin cnt thd0  5 */
    static_cfg->skin_edge_thd[0] = 10;  /* skin edg thd0 10 */
    static_cfg->skin_cnt_thd[1]  = 8;   /* skin cnt thd1  8 */
    static_cfg->skin_edge_thd[1] = 30;  /* skin edg thd1 30 */
    static_cfg->skin_edge_sft    = 1;

    static_cfg->skin_cnt_mul = calc_mul_coef(static_cfg->skin_cnt_thd[0], 0,
                                             static_cfg->skin_cnt_thd[1], 31, 0); /* Range: [0, 31] */
}

/* sharpen hardware regs that will change with MPI and ISO */
static td_void sharpen_mpi_dyna_mot_reg_init(isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8 i;
    for (i = 0; i < SHRP_GAIN_LUT_SIZE; i++) {
        mpi_dyna_cfg->mf_mot_gain_d[i]  = mpi_dyna_cfg->mf_gain_d[i];
        mpi_dyna_cfg->mf_mot_gain_ud[i] = mpi_dyna_cfg->mf_gain_ud[i];
        mpi_dyna_cfg->hf_mot_gain_d[i]  = mpi_dyna_cfg->hf_gain_d[i];
        mpi_dyna_cfg->hf_mot_gain_ud[i] = mpi_dyna_cfg->hf_gain_ud[i];
    }

    mpi_dyna_cfg->osht_mot_amt = mpi_dyna_cfg->osht_amt;
    mpi_dyna_cfg->usht_mot_amt = mpi_dyna_cfg->usht_amt;
    mpi_dyna_cfg->mot_enable = shp_ctx->mpi_cfg.motion_en; /* mot_enable : sharpen_en && bnr_en && bnr_entmpnr */
    mpi_dyna_cfg->mot_thd0     = shp_ctx->mpi_cfg.motion_threshold0;
    mpi_dyna_cfg->mot_thd1     = shp_ctx->mpi_cfg.motion_threshold1;
    mpi_dyna_cfg->mot_gain0    = shp_ctx->mpi_cfg.motion_gain0;
    mpi_dyna_cfg->mot_gain1    = shp_ctx->mpi_cfg.motion_gain1;
    mpi_dyna_cfg->mot_mul      = (td_u16)calc_mul_coef(mpi_dyna_cfg->mot_thd0, mpi_dyna_cfg->mot_gain0,
                                                       mpi_dyna_cfg->mot_thd1, mpi_dyna_cfg->mot_gain1, 0);
    mpi_dyna_cfg->print_sel    = shp_ctx->mpi_cfg.detail_map;
}

static td_void sharpen_mpi_dyna_reg_init(isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8 i;
    for (i = 0; i < SHRP_GAIN_LUT_SIZE; i++) {
        mpi_dyna_cfg->mf_gain_d[i]  = 300;  /* mf gain d  300 */
        mpi_dyna_cfg->mf_gain_ud[i] = 200;  /* mf gain ud 200 */
        mpi_dyna_cfg->hf_gain_d[i]  = 450;  /* hf gain d  450 */
        mpi_dyna_cfg->hf_gain_ud[i] = 400;  /* hf gain ud 400 */
    }

    mpi_dyna_cfg->osht_amt           = 100; /* over  shoot amt 100  */
    mpi_dyna_cfg->usht_amt           = 127; /* under shoot amt 127  */
    mpi_dyna_cfg->en_sht_ctrl_by_var = 1;   /* sht ctrl by var 1    */
    mpi_dyna_cfg->sht_bld_rt         = 9;   /* shoot blend rt  9    */
    mpi_dyna_cfg->sht_var_thd1       = 5;   /* oshoot var thd1 5    */

    mpi_dyna_cfg->en_weak_detail_adj = 0;
    mpi_dyna_cfg->weak_detail_gain   = 2;   /* weak detail gain 2   */

    mpi_dyna_cfg->en_chr_ctrl        = 1;   /* en chrome ctrl   1   */
    mpi_dyna_cfg->chr_r_gain         = 6;   /* chrome red gain  6   */
    mpi_dyna_cfg->chr_g_gain         = 32;  /* chrome gre gain  32  */
    mpi_dyna_cfg->chr_gmf_gain       = 32;  /* chrome gmf gain  32  */
    mpi_dyna_cfg->chr_b_gain         = 14;  /* chrome blue gain 14  */
    mpi_dyna_cfg->en_skin_ctrl       = 0;   /* en skin ctrl     0   */
    mpi_dyna_cfg->skin_edge_wgt[1]   = 31;  /* skin edge wgt1   31  */
    mpi_dyna_cfg->skin_edge_wgt[0]   = 31;  /* skin edge wgt0   31  */

    mpi_dyna_cfg->en_luma_ctrl       = 0;   /* en luma   ctrl   0   */
    mpi_dyna_cfg->en_detail_ctrl     = 0;   /* en detail ctrl   0   */
    mpi_dyna_cfg->detail_osht_amt    = 100; /* detail osht amt  100 */
    mpi_dyna_cfg->detail_usht_amt    = 127; /* detail usht amt  127 */
    mpi_dyna_cfg->dir_diff_sft       = 10;  /* dir diff asft    10  */
    mpi_dyna_cfg->dir_rt[0]          = 6;   /* direction rt0    6   */
    mpi_dyna_cfg->dir_rt[1]          = 18;  /* direction rt1    18  */
    mpi_dyna_cfg->skin_max_u         = 127; /* skin max u       127 */
    mpi_dyna_cfg->skin_min_u         = 95;  /* skin min u       95  */
    mpi_dyna_cfg->skin_max_v         = 155; /* skin max v       155 */
    mpi_dyna_cfg->skin_min_v         = 135; /* skin min v       135 */
    mpi_dyna_cfg->o_max_gain         = 160; /* oshoot max gain  160 */
    mpi_dyna_cfg->u_max_gain         = 160; /* ushoot max gain  160 */
    mpi_dyna_cfg->detail_osht_thr[0] = 65;  /* detail osht thr0 65  */
    mpi_dyna_cfg->detail_osht_thr[1] = 90;  /* detail osht thr1 90  */
    mpi_dyna_cfg->detail_usht_thr[0] = 65;  /* detail usht thr0 65  */
    mpi_dyna_cfg->detail_usht_thr[1] = 90;  /* detail usht thr1 90  */

    for (i = 0; i < OT_ISP_SHARPEN_LUMA_NUM; i++) {
        mpi_dyna_cfg->luma_wgt[i]  = 31;   /* luma weight      31 */
    }

    sharpen_mpi_dyna_mot_reg_init(mpi_dyna_cfg, shp_ctx);

    mpi_dyna_cfg->update_index      = 1;
    mpi_dyna_cfg->buf_id            = 0;
    mpi_dyna_cfg->resh              = TD_TRUE;
    mpi_dyna_cfg->switch_mode       = TD_TRUE;
}

/* sharpen hardware regs that will change only with ISO */
static td_void sharpen_default_dyna_reg_init(isp_sharpen_default_dyna_reg_cfg *def_dyna_cfg)
{
    def_dyna_cfg->resh                = TD_TRUE;
    def_dyna_cfg->gain_thd_sft_d      = 0;   /* mf thd sft d       0  */
    def_dyna_cfg->gain_thd_sel_ud     = 2;   /* mf thd sel ud      2  */
    def_dyna_cfg->gain_thd_sft_ud     = 0;   /* mf thd sft ud      0  */
    def_dyna_cfg->dir_var_sft         = 10;  /* dir var sft        10 */
    def_dyna_cfg->sel_pix_wgt         = 31;  /* sel pix wgt        31 */
    def_dyna_cfg->sht_var_diff_thd[0] = 27;  /* sht var diff thd0  27 */
    def_dyna_cfg->sht_var_wgt0        = 0;
    def_dyna_cfg->sht_var_diff_thd[1] = 50;  /* sht var diff thd1  50 */
    def_dyna_cfg->sht_var_diff_wgt1   = 5;   /* sht var diff wgt1  5  */
    def_dyna_cfg->rmf_gain_scale      = 24;  /* rmf gain scale     24 */
    def_dyna_cfg->bmf_gain_scale      = 10;  /* bmf gain scale     10 */
    def_dyna_cfg->dir_rly_thr_low     = 2;   /* dir rly thr low     2 */
    def_dyna_cfg->dir_rly_thrhigh     = 120; /* dir rly thr high  120 */
}

static td_void sharpen_dyna_reg_init(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    isp_sharpen_default_dyna_reg_cfg *def_cfg = TD_NULL;
    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg = TD_NULL;
    isp_sharpen_static_reg_cfg       *sta_cfg = TD_NULL;

    def_cfg = &sharpen_reg_cfg->dyna_reg_cfg.default_dyna_reg_cfg;
    mpi_cfg = &sharpen_reg_cfg->dyna_reg_cfg.mpi_dyna_reg_cfg;
    sta_cfg = &sharpen_reg_cfg->static_reg_cfg;

    sharpen_default_dyna_reg_init(def_cfg);
    sharpen_mpi_dyna_reg_init(mpi_cfg, shp_ctx);

    /* calc all mul_coef */
    /* mpi */
    mpi_cfg->sht_var_mul      = calc_mul_coef(sta_cfg->sht_var_thd0,  def_cfg->sht_var_wgt0,
                                              mpi_cfg->sht_var_thd1, sta_cfg->sht_var_wgt1,
                                              SHRP_SHT_VAR_MUL_PRECS);
    mpi_cfg->chr_r_mul         = calc_mul_coef(sta_cfg->chr_r_thd[0], mpi_cfg->chr_r_gain,
                                               sta_cfg->chr_r_thd[1], 0x20,
                                               SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_g_mul         = calc_mul_coef(sta_cfg->chr_g_thd[0], mpi_cfg->chr_g_gain,
                                               sta_cfg->chr_g_thd[1], 0x20,
                                               SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_gmf_mul       = calc_mul_coef(sta_cfg->chr_g_thd[0], mpi_cfg->chr_gmf_gain,
                                               sta_cfg->chr_g_thd[1], 0x20,
                                               SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_b_mul         = calc_mul_coef(sta_cfg->chr_b_thd[0], mpi_cfg->chr_b_gain,
                                               sta_cfg->chr_b_thd[1], 0x20,
                                               SHRP_CHR_MUL_SFT);
    mpi_cfg->skin_edge_mul     = calc_mul_coef(sta_cfg->skin_edge_thd[0], mpi_cfg->skin_edge_wgt[0],
                                               sta_cfg->skin_edge_thd[1], mpi_cfg->skin_edge_wgt[1],
                                               SHRP_SKIN_EDGE_MUL_PRECS);
    mpi_cfg->detail_osht_mul   = calc_mul_coef(mpi_cfg->detail_osht_thr[0], mpi_cfg->detail_osht_amt,
                                               mpi_cfg->detail_osht_thr[1], mpi_cfg->osht_amt,
                                               SHRP_DETAIL_SHT_MUL_PRECS);
    mpi_cfg->detail_usht_mul   = calc_mul_coef(mpi_cfg->detail_usht_thr[0], mpi_cfg->detail_usht_amt,
                                               mpi_cfg->detail_usht_thr[1], mpi_cfg->usht_amt,
                                               SHRP_DETAIL_SHT_MUL_PRECS);

    /* default */
    def_cfg->sht_var_diff_mul = calc_mul_coef(def_cfg->sht_var_diff_thd[0], sta_cfg->sht_var_diff_wgt0,
                                              def_cfg->sht_var_diff_thd[1], def_cfg->sht_var_diff_wgt1,
                                              SHRP_SHT_VAR_MUL_PRECS);
}

static td_void sharpen_regs_initialize(isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u32 i;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.enable      = TD_TRUE;
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.lut2_stt_en = TD_TRUE;

        sharpen_static_shoot_reg_init(&(reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.static_reg_cfg));
        sharpen_static_chrome_reg_init(&(reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.static_reg_cfg));
        sharpen_static_hsf_reg_init(&(reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.static_reg_cfg));
        sharpen_static_lpf_reg_init(&(reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.static_reg_cfg));

        sharpen_dyna_reg_init(&reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg, shp_ctx);
    }

    reg_cfg->cfg_key.bit1_sharpen_cfg = 1;
}

td_void isp_sharpen_read_extregs(ot_vi_pipe vi_pipe, isp_sharpen_ctx *shp_ctx)
{
    shp_ctx->sharpen_mpi_update_en = ot_ext_system_sharpen_mpi_update_en_read(vi_pipe);
    if (shp_ctx->sharpen_mpi_update_en != TD_TRUE) {
        return;
    }

    ot_ext_system_sharpen_mpi_update_en_write(vi_pipe, TD_FALSE);
    isp_sharpen_comm_attr_read(vi_pipe, &shp_ctx->mpi_cfg);
    if (shp_ctx->mpi_cfg.op_type == OT_OP_MODE_MANUAL) {
        isp_sharpen_manual_attr_read(vi_pipe, &shp_ctx->mpi_cfg.manual_attr);
    } else {
        isp_sharpen_auto_attr_read(vi_pipe, &shp_ctx->mpi_cfg.auto_attr);
    }
}

td_void isp_sharpen_read_pro_mode(ot_vi_pipe vi_pipe, const isp_sharpen_ctx *shp_ctx)
{
    ot_unused(vi_pipe);
    ot_unused(shp_ctx);
}

static td_s32 sharpen_mot_set(ot_vi_pipe vi_pipe, const isp_sharpen_ctx *shp_ctx)
{
    td_s32 ret;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_MOT_CFG_SET, &shp_ctx->mpi_cfg.motion_en);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set mot cfg failed%x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sharpen_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_s32 ret;
    shp_ctx->init = TD_FALSE;
    ret = isp_sharpen_ctx_initialize(vi_pipe, shp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    sharpen_regs_initialize(reg_cfg, shp_ctx);
    sharpen_ext_regs_initialize(vi_pipe, shp_ctx);
    ret = sharpen_mot_set(vi_pipe, shp_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ot_ext_system_isp_sharpen_init_status_write(vi_pipe, TD_TRUE);
    shp_ctx->init = TD_TRUE;
    return TD_SUCCESS;
}

static td_s32 isp_sharpen_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    isp_sharpen_ctx *shp_ctx = TD_NULL;

    ot_ext_system_isp_sharpen_init_status_write(vi_pipe, TD_FALSE);
    ret = sharpen_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    sharpen_get_ctx(vi_pipe, shp_ctx);
    isp_check_pointer_return(shp_ctx);

    return isp_sharpen_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg, shp_ctx);
}

static td_void isp_sharpen_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_u8  i;
    td_u32 update_idx[OT_ISP_STRIPING_MAX_NUM] = {0};
    td_s32 ret;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        update_idx[i] = reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.update_index;
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.enable = TD_FALSE;
    }
    reg_cfg->cfg_key.bit1_sharpen_cfg = 1;

    ret = isp_sharpen_init(vi_pipe, reg_cfg);
    if (ret != TD_SUCCESS) {
        return;
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.update_index = update_idx[i] + 1;
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.switch_mode  = TD_TRUE;
    }
}

static td_void isp_sharpen_get_linear_default_reg_cfg(isp_sharpen_ctx *sharpen_para)
{
    /* linear mode default regs */
    td_u8  gain_thd_sel_ud_linear[SHRP_ISO_NUM]    = {1,   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1};
    td_u8  gain_thd_sft_ud_linear[SHRP_ISO_NUM]    = {0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
    td_u8  sht_var_wgt0_linear[SHRP_ISO_NUM]       = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
    td_u8  sht_var_diff_thd0_linear[SHRP_ISO_NUM]  = {27, 29, 31, 33, 37, 40, 42, 48, 49, 50, 53, 53, 53, 53, 53, 53};
    td_u8  sht_var_diff_thd1_linear[SHRP_ISO_NUM]  = {49, 50, 51, 52, 54, 56, 56, 61, 63, 64, 66, 67, 67, 67, 67, 67};
    td_u8  sht_var_diff_wgt1_linear[SHRP_ISO_NUM]  = {10, 10, 10, 10, 15, 15, 15, 18, 20, 20, 20, 20, 20, 20, 20, 20};

    td_u8  sft     = sharpen_para->inter_info.sft;
    td_u16 wgt_pre = sharpen_para->inter_info.wgt_pre;
    td_u16 wgt_cur = sharpen_para->inter_info.wgt_cur;
    td_u32 idx_cur = sharpen_para->inter_info.idx_cur;
    td_u32 idx_pre = sharpen_para->inter_info.idx_pre;

    sharpen_para->gain_thd_sel_ud    = shrp_blend(sft, wgt_pre, gain_thd_sel_ud_linear[idx_pre],
                                                  wgt_cur, gain_thd_sel_ud_linear[idx_cur]);
    sharpen_para->gain_thd_sft_ud    = shrp_blend(sft, wgt_pre, gain_thd_sft_ud_linear[idx_pre],
                                                  wgt_cur, gain_thd_sft_ud_linear[idx_cur]);
    sharpen_para->sht_var_wgt0       =  shrp_blend(sft, wgt_pre, sht_var_wgt0_linear[idx_pre],
                                                   wgt_cur, sht_var_wgt0_linear[idx_cur]);
    sharpen_para->sht_var_diff_thd0  =  shrp_blend(sft, wgt_pre, sht_var_diff_thd0_linear[idx_pre],
                                                   wgt_cur, sht_var_diff_thd0_linear[idx_cur]);
    sharpen_para->sht_var_diff_thd1  =  shrp_blend(sft, wgt_pre, sht_var_diff_thd1_linear[idx_pre],
                                                   wgt_cur, sht_var_diff_thd1_linear[idx_cur]);
    sharpen_para->sht_var_diff_wgt1  =  shrp_blend(sft, wgt_pre, sht_var_diff_wgt1_linear[idx_pre],
                                                   wgt_cur, sht_var_diff_wgt1_linear[idx_cur]);
}

static td_void isp_sharpen_get_wdr_default_reg_cfg(isp_sharpen_ctx *sharpen_para)
{
    /* WDR mode default regs */
    td_u8  gain_thd_sel_ud_wdr[SHRP_ISO_NUM]   = { 2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 };
    td_u8  gain_thd_sft_ud_wdr[SHRP_ISO_NUM]   = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
    td_u8  sht_var_wgt0_wdr[SHRP_ISO_NUM]      = { 65, 65, 65, 65, 65, 55, 25, 25, 25, 25, 25, 20, 20, 20, 20, 20 };
    td_u8  sht_var_diff_thd0_wdr[SHRP_ISO_NUM] = { 27, 29, 31, 33, 37, 40, 42, 48, 49, 50, 53, 53, 53, 53, 53, 53 };
    td_u8  sht_var_diff_thd1_wdr[SHRP_ISO_NUM] = { 49, 50, 51, 52, 54, 56, 56, 61, 63, 64, 66, 67, 67, 67, 67, 67 };
    td_u8  sht_var_diff_wgt1_wdr[SHRP_ISO_NUM] = { 5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5 };

    td_u8  sft     = sharpen_para->inter_info.sft;
    td_u16 wgt_pre = sharpen_para->inter_info.wgt_pre;
    td_u16 wgt_cur = sharpen_para->inter_info.wgt_cur;
    td_u32 idx_cur = sharpen_para->inter_info.idx_cur;
    td_u32 idx_pre = sharpen_para->inter_info.idx_pre;

    sharpen_para->gain_thd_sel_ud    =  shrp_blend(sft, wgt_pre, gain_thd_sel_ud_wdr[idx_pre],
                                                   wgt_cur, gain_thd_sel_ud_wdr[idx_cur]);
    sharpen_para->gain_thd_sft_ud    =  shrp_blend(sft, wgt_pre, gain_thd_sft_ud_wdr[idx_pre],
                                                   wgt_cur, gain_thd_sft_ud_wdr[idx_cur]);
    sharpen_para->sht_var_wgt0       =  shrp_blend(sft, wgt_pre, sht_var_wgt0_wdr[idx_pre],
                                                   wgt_cur, sht_var_wgt0_wdr[idx_cur]);
    sharpen_para->sht_var_diff_thd0  =  shrp_blend(sft, wgt_pre, sht_var_diff_thd0_wdr[idx_pre],
                                                   wgt_cur, sht_var_diff_thd0_wdr[idx_cur]);
    sharpen_para->sht_var_diff_thd1  =  shrp_blend(sft, wgt_pre, sht_var_diff_thd1_wdr[idx_pre],
                                                   wgt_cur, sht_var_diff_thd1_wdr[idx_cur]);
    sharpen_para->sht_var_diff_wgt1  =  shrp_blend(sft, wgt_pre, sht_var_diff_wgt1_wdr[idx_pre],
                                                   wgt_cur, sht_var_diff_wgt1_wdr[idx_cur]);
}

static td_void isp_sharpen_get_default_common_reg_cfg(isp_sharpen_ctx *sharpen_para)
{
    /* common regs */
    td_u8  gain_sft[SHRP_ISO_NUM]  = {  0,   0,   0,  0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 };
    td_u8  dir_sft[SHRP_ISO_NUM]   = { 10,  10,  10, 10,  10,   10,  10,   8,   7,   6,   5,   4,   3,   3,   3,   3 };
    td_u8  sel_wgt[SHRP_ISO_NUM]   = { 31,  31,  31, 31,   31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31 };
    td_u16 rmf_scale[SHRP_ISO_NUM] = { 24,  24,  24, 24,   24,  20,  14,  10,   6,   4,   2,   2,   2,   2,   2,   2 };
    td_u16 bmf_scale[SHRP_ISO_NUM] = { 16,  12,  10,  6,    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4 };
    td_u8  dir_low[SHRP_ISO_NUM]   = {  2,   2,   2,  2,    2,   3,   4,   4,   5,   5,   6,   6,   7,   7,   7,   7 };
    td_u8  dir_high[SHRP_ISO_NUM]  = {120, 120, 120, 120, 120, 121, 122, 123, 123, 124, 125, 126, 126, 126, 126, 126};

    td_u8  sft     = sharpen_para->inter_info.sft;
    td_u16 wgt_pre = sharpen_para->inter_info.wgt_pre;
    td_u16 wgt_cur = sharpen_para->inter_info.wgt_cur;
    td_u32 idx_cur = sharpen_para->inter_info.idx_cur;
    td_u32 idx_pre = sharpen_para->inter_info.idx_pre;

    sharpen_para->gain_thd_sft_d  = shrp_blend(sft, wgt_pre, gain_sft[idx_pre],  wgt_cur, gain_sft[idx_cur]);
    sharpen_para->dir_var_sft     = shrp_blend(sft, wgt_pre, dir_sft[idx_pre],   wgt_cur, dir_sft[idx_cur]);
    sharpen_para->sel_pix_wgt     = shrp_blend(sft, wgt_pre, sel_wgt[idx_pre],   wgt_cur, sel_wgt[idx_cur]);
    sharpen_para->rmf_gain_scale  = shrp_blend(sft, wgt_pre, rmf_scale[idx_pre], wgt_cur, rmf_scale[idx_cur]);
    sharpen_para->bmf_gain_scale  = shrp_blend(sft, wgt_pre, bmf_scale[idx_pre], wgt_cur, bmf_scale[idx_cur]);
    sharpen_para->dir_rly_thr_low = shrp_blend(sft, wgt_pre, dir_low[idx_pre],   wgt_cur, dir_low[idx_cur]);
    sharpen_para->dir_rly_thrhigh = shrp_blend(sft, wgt_pre, dir_high[idx_pre],  wgt_cur, dir_high[idx_cur]);
}

static td_void isp_sharpen_get_default_reg_cfg(ot_vi_pipe vi_pipe, isp_sharpen_ctx *shp_ctx)
{
    td_u8  wdr_mode;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    wdr_mode = isp_ctx->sns_wdr_mode;

    isp_sharpen_get_default_common_reg_cfg(shp_ctx);

    /* linear mode default regs */
    if (is_linear_mode(wdr_mode) || (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) ||
        (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE)) {
        isp_sharpen_get_linear_default_reg_cfg(shp_ctx);
    } else { /* WDR mode default regs */
        isp_sharpen_get_wdr_default_reg_cfg(shp_ctx);
    }
}

static td_void isp_sharpen_get_auto_motion_inter_result(const isp_sharpen_inter_info *inter_info,
    ot_isp_sharpen_auto_attr *auto_ctx, ot_isp_sharpen_manual_attr *actual)
{
    td_u8  i;
    td_u8  sft     = inter_info->sft;
    td_u16 wgt_pre = inter_info->wgt_pre;
    td_u16 wgt_cur = inter_info->wgt_cur;
    td_u32 idx_cur = inter_info->idx_cur;
    td_u32 idx_pre = inter_info->idx_pre;

    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        actual->motion_texture_strength[i] = (td_u16)shrp_blend(sft, wgt_pre,
                                                                auto_ctx->motion_texture_strength[i][idx_pre],
                                                                wgt_cur, auto_ctx->motion_texture_strength[i][idx_cur]);
        actual->motion_edge_strength[i] = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->motion_edge_strength[i][idx_pre],
                                                             wgt_cur, auto_ctx->motion_edge_strength[i][idx_cur]);
    }

    actual->motion_texture_freq  = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->motion_texture_freq[idx_pre],
                                                      wgt_cur, auto_ctx->motion_texture_freq[idx_cur]);
    actual->motion_edge_freq     = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->motion_edge_freq[idx_pre],
                                                      wgt_cur, auto_ctx->motion_edge_freq[idx_cur]);
    actual->motion_over_shoot    = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->motion_over_shoot[idx_pre],
                                                     wgt_cur, auto_ctx->motion_over_shoot[idx_cur]);
    actual->motion_under_shoot   = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->motion_under_shoot[idx_pre],
                                                     wgt_cur, auto_ctx->motion_under_shoot[idx_cur]);
    return;
}

static td_void isp_sharpen_get_auto_strength_inter_result(const isp_sharpen_inter_info *inter_info,
    ot_isp_sharpen_auto_attr *auto_ctx, ot_isp_sharpen_manual_attr *actual)
{
    td_u8  i;
    td_u8  sft     = inter_info->sft;
    td_u16 wgt_pre = inter_info->wgt_pre;
    td_u16 wgt_cur = inter_info->wgt_cur;
    td_u32 idx_cur = inter_info->idx_cur;
    td_u32 idx_pre = inter_info->idx_pre;
    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        actual->texture_strength[i] = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->texture_strength[i][idx_pre],
                                                         wgt_cur, auto_ctx->texture_strength[i][idx_cur]);
        actual->edge_strength[i]    = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->edge_strength[i][idx_pre],
                                                         wgt_cur, auto_ctx->edge_strength[i][idx_cur]);
    }

    for (i = 0; i < OT_ISP_SHARPEN_LUMA_NUM; i++) {
        actual->luma_wgt[i]    = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->luma_wgt[i][idx_pre],
                                                   wgt_cur, auto_ctx->luma_wgt[i][idx_cur]);
    }

    actual->texture_freq       = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->texture_freq[idx_pre],
                                                    wgt_cur, auto_ctx->texture_freq[idx_cur]);
    actual->edge_freq          = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->edge_freq[idx_pre],
                                                    wgt_cur, auto_ctx->edge_freq[idx_cur]);
}

static td_void isp_sharpen_get_auto_shoot_inter_result(const isp_sharpen_inter_info *inter_info,
    ot_isp_sharpen_auto_attr *auto_ctx, ot_isp_sharpen_manual_attr *actual)
{
    td_u8  sft     = inter_info->sft;
    td_u16 wgt_pre = inter_info->wgt_pre;
    td_u16 wgt_cur = inter_info->wgt_cur;
    td_u32 idx_cur = inter_info->idx_cur;
    td_u32 idx_pre = inter_info->idx_pre;
    actual->over_shoot         = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->over_shoot[idx_pre],
                                                   wgt_cur, auto_ctx->over_shoot[idx_cur]);
    actual->under_shoot        = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->under_shoot[idx_pre],
                                                   wgt_cur, auto_ctx->under_shoot[idx_cur]);
    actual->shoot_sup_strength = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->shoot_sup_strength[idx_pre],
                                                   wgt_cur, auto_ctx->shoot_sup_strength[idx_cur]);
    actual->detail_ctrl        = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->detail_ctrl[idx_pre],
                                                   wgt_cur, auto_ctx->detail_ctrl[idx_cur]);
    actual->edge_filt_strength = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->edge_filt_strength[idx_pre],
                                                   wgt_cur, auto_ctx->edge_filt_strength[idx_cur]);
    actual->edge_filt_max_cap  = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->edge_filt_max_cap[idx_pre],
                                                   wgt_cur, auto_ctx->edge_filt_max_cap[idx_cur]);
    actual->r_gain             = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->r_gain[idx_pre],
                                                   wgt_cur, auto_ctx->r_gain[idx_cur]);
    actual->g_gain             = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->g_gain[idx_pre],
                                                   wgt_cur, auto_ctx->g_gain[idx_cur]);
    actual->b_gain             = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->b_gain[idx_pre],
                                                   wgt_cur, auto_ctx->b_gain[idx_cur]);
    actual->skin_gain          = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->skin_gain[idx_pre],
                                                   wgt_cur, auto_ctx->skin_gain[idx_cur]);
    actual->shoot_sup_adj      = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->shoot_sup_adj[idx_pre],
                                                   wgt_cur, auto_ctx->shoot_sup_adj[idx_cur]);
    actual->detail_ctrl_threshold = (td_u8)shrp_blend(sft, wgt_pre, auto_ctx->detail_ctrl_threshold[idx_pre],
                                                      wgt_cur, auto_ctx->detail_ctrl_threshold[idx_cur]);
    actual->max_sharp_gain     = (td_u16)shrp_blend(sft, wgt_pre, auto_ctx->max_sharp_gain[idx_pre],
                                                    wgt_cur, auto_ctx->max_sharp_gain[idx_cur]);
}

static td_void isp_sharpen_get_mpi_auto_inter_result(isp_sharpen_ctx *shp_ctx)
{
    ot_isp_sharpen_auto_attr   *auto_ctx = &shp_ctx->mpi_cfg.auto_attr;
    ot_isp_sharpen_manual_attr *actual   = &shp_ctx->actual;

    isp_sharpen_get_auto_strength_inter_result(&shp_ctx->inter_info, auto_ctx, actual);
    isp_sharpen_get_auto_shoot_inter_result(&shp_ctx->inter_info, auto_ctx, actual);
    isp_sharpen_get_auto_motion_inter_result(&shp_ctx->inter_info, auto_ctx, actual);
}

static td_void sharpen_mpi2reg_def(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    isp_sharpen_default_dyna_reg_cfg *def_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg         *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    def_cfg      = &dyna_reg_cfg->default_dyna_reg_cfg;

    def_cfg->gain_thd_sft_d      = shp_ctx->gain_thd_sft_d;
    def_cfg->gain_thd_sel_ud     = shp_ctx->gain_thd_sel_ud;
    def_cfg->gain_thd_sft_ud     = shp_ctx->gain_thd_sft_ud;
    def_cfg->dir_var_sft         = shp_ctx->dir_var_sft;
    def_cfg->sel_pix_wgt         = shp_ctx->sel_pix_wgt;
    def_cfg->sht_var_diff_thd[0] = shp_ctx->sht_var_diff_thd0;
    def_cfg->sht_var_wgt0        = shp_ctx->sht_var_wgt0;
    def_cfg->sht_var_diff_thd[1] = shp_ctx->sht_var_diff_thd1;
    def_cfg->sht_var_diff_wgt1   = shp_ctx->sht_var_diff_wgt1;
    def_cfg->rmf_gain_scale      = shp_ctx->rmf_gain_scale;
    def_cfg->bmf_gain_scale      = shp_ctx->bmf_gain_scale;
    def_cfg->dir_rly_thr_low     = shp_ctx->dir_rly_thr_low;
    def_cfg->dir_rly_thrhigh     = shp_ctx->dir_rly_thrhigh;
}

static td_void sharpen_mpi2reg_mf_gain_d(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u32 tmp;
    td_u16 *eg_str = shp_ctx->actual.edge_strength;
    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        j = i << 1;
        if (i < OT_ISP_SHARPEN_GAIN_NUM - 1) {
            mpi_cfg->mf_gain_d[j]      = MIN2(0xFFF, (0x20 + eg_str[i]));
            mpi_cfg->mf_gain_d[j + 1]  = MIN2(0xFFF, (0x20 + ((eg_str[i] + eg_str[i + 1]) >> 1)));
        } else { /* 31 */
            mpi_cfg->mf_gain_d[j]      = MIN2(0xFFF, (0x20 + eg_str[i]));
            mpi_cfg->mf_gain_d[j + 1]  = MIN2(0xFFF, (0x20 + eg_str[i]));
        }
        tmp = (td_u32)mpi_cfg->mf_gain_d[j] * shp_ctx->actual.edge_freq;
        mpi_cfg->hf_gain_d[j]     = (td_u16)MIN2(0xFFF, (tmp >> 0x6));
        tmp = (td_u32)mpi_cfg->mf_gain_d[j + 1] * shp_ctx->actual.edge_freq;
        mpi_cfg->hf_gain_d[j + 1] = (td_u16)MIN2(0xFFF, (tmp >> 0x6));
    }
}

static td_void sharpen_mpi2reg_mf_mot_gain_d(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u32 tmp;
    td_u16 *eg_str = shp_ctx->actual.motion_edge_strength;
    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        j = i << 1;
        if (i < OT_ISP_SHARPEN_GAIN_NUM - 1) {
            mpi_cfg->mf_mot_gain_d[j]      = MIN2(0xFFF, (0x20 + eg_str[i]));
            mpi_cfg->mf_mot_gain_d[j + 1]  = MIN2(0xFFF, (0x20 + ((eg_str[i] + eg_str[i + 1]) >> 1)));
        } else { /* 31 */
            mpi_cfg->mf_mot_gain_d[j]      = MIN2(0xFFF, (0x20 + eg_str[i]));
            mpi_cfg->mf_mot_gain_d[j + 1]  = MIN2(0xFFF, (0x20 + eg_str[i]));
        }
        tmp = (td_u32)mpi_cfg->mf_mot_gain_d[j] * shp_ctx->actual.motion_edge_freq;
        mpi_cfg->hf_mot_gain_d[j] = (td_u16)MIN2(0xFFF, (tmp >> 0x6));
        tmp = (td_u32)mpi_cfg->mf_mot_gain_d[j + 1] * shp_ctx->actual.motion_edge_freq;
        mpi_cfg->hf_mot_gain_d[j + 1] = (td_u16)MIN2(0xFFF, (tmp >> 0x6));
    }
}

static td_void sharpen_mpi2reg_mf_thd_sel_ud_enable(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u16 *tx_str = shp_ctx->actual.texture_strength;

    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    j = 0;
    for (i = 0; i < 12; i++) {  /* 1st segment [0,12)  */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    }
    for (i = 12; i < 20; i++) { /* 2nd segment  [12,20) */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i] + tx_str[i + 1]) >> 1)));
    }
    for (i = 20; i < 31; i++) { /* 3rd segment  [20,31) */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1])) / 3)); /* (2+1)/3 */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) + (tx_str[i + 1]) * 2) / 3)); /* (1+2)/3 */
    }
    i = 31;                     /* last segment [31]    */
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
}

static td_void sharpen_mpi2reg_mf_thd_sel_ud_disable(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u16 *tx_str = shp_ctx->actual.texture_strength;

    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg         *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    j = 0;
    for (i = 0; i < 16; i++) {  /* 1st segment [0,16)  */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    }
    for (i = 16; i < 24; i++) { /* 2nd segment [16,24) */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i] + tx_str[i + 1]) >> 1)));
    }
    for (i = 24; i < 28; i++) { /* 3rd segment [24,28)  */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1])) / 3));     /* (2+1)/3 */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) + (tx_str[i + 1]) * 2) / 3));     /* (1+2)/3 */
    }
    for (i = 28; i < 31; i++) { /* 4th segment [28,31)  */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 4 + (tx_str[i + 1]) * 1) / 5)); /* (4+1)/5 */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 3 + (tx_str[i + 1]) * 2) / 5)); /* (3+2)/5 */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1]) * 3) / 5)); /* (2+3)/5 */
        mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 1 + (tx_str[i + 1]) * 4) / 5)); /* (1+4)/5 */
    }
    i = 31;                     /* last segment [31]    */
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
}

static td_void sharpen_mpi2reg_mf_thd_sel_mot_ud_enable(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u16 *tx_str = shp_ctx->actual.motion_texture_strength;
    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    j = 0;
    for (i = 0; i < 12; i++) {  /* 1st segment [0,12)  */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    }
    for (i = 12; i < 20; i++) { /* 2nd segment  [12,20) */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i] + tx_str[i + 1]) >> 1)));
    }
    for (i = 20; i < 31; i++) { /* 3rd segment  [20,31) */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1])) / 3)); /* (2+1)/3 */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) + (tx_str[i + 1]) * 2) / 3)); /* (1+2)/3 */
    }
    i = 31;                     /* last segment [31]    */
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
}

static td_void sharpen_mpi2reg_mf_thd_sel_mot_ud_disable(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i, j;
    td_u16 *tx_str = shp_ctx->actual.motion_texture_strength;
    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg         *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    j = 0;
    for (i = 0; i < 16; i++) {  /* 1st segment [0,16)  */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    }
    for (i = 16; i < 24; i++) { /* 2nd segment [16,24) */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i] + tx_str[i + 1]) >> 1)));
    }
    for (i = 24; i < 28; i++) { /* 3rd segment [24,28)  */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1])) / 3));     /* (2+1)/3 */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) + (tx_str[i + 1]) * 2) / 3));     /* (1+2)/3 */
    }
    for (i = 28; i < 31; i++) { /* 4th segment [28,31)  */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 4 + (tx_str[i + 1]) * 1) / 5)); /* (4+1)/5 */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 3 + (tx_str[i + 1]) * 2) / 5)); /* (3+2)/5 */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 2 + (tx_str[i + 1]) * 3) / 5)); /* (2+3)/5 */
        mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + ((tx_str[i]) * 1 + (tx_str[i + 1]) * 4) / 5)); /* (1+4)/5 */
    }
    i = 31;                     /* last segment [31]    */
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
    mpi_cfg->mf_mot_gain_ud[j++] = MIN2(0xFFF, (0x20 + tx_str[i]));
}

static td_void sharpen_mpi2reg_mpi_chr(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i;
    td_u32 tmp;
    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    for (i = 0; i < SHRP_GAIN_LUT_SIZE; i++) {
        tmp = (td_u32)mpi_cfg->mf_gain_ud[i] * shp_ctx->actual.texture_freq;
        mpi_cfg->hf_gain_ud[i] = (td_u16)MIN2(0xFFF, (tmp >> 0x6));

        tmp = (td_u32)mpi_cfg->mf_mot_gain_ud[i] * shp_ctx->actual.motion_texture_freq;
        mpi_cfg->hf_mot_gain_ud[i] = (td_u16)MIN2(0xFFF, (tmp >> 0x6));
    }

    mpi_cfg->osht_amt = shp_ctx->actual.over_shoot;
    mpi_cfg->usht_amt = shp_ctx->actual.under_shoot;
    mpi_cfg->osht_mot_amt = shp_ctx->actual.motion_over_shoot;
    mpi_cfg->usht_mot_amt = shp_ctx->actual.motion_under_shoot;
    /* skin ctrl */
    if (shp_ctx->actual.skin_gain == 0x1F) {
        mpi_cfg->en_skin_ctrl     = 0;
    } else {
        mpi_cfg->en_skin_ctrl     = 1;
        mpi_cfg->skin_edge_wgt[1] = clip3((0x1F - shp_ctx->actual.skin_gain), 0, 0x1F);
        mpi_cfg->skin_edge_wgt[0] = MIN2(0x1F, (mpi_cfg->skin_edge_wgt[1] << 1));
    }
    /* chr ctrl */
    if ((shp_ctx->actual.g_gain == 0x20) && (shp_ctx->actual.r_gain == 0x1F) && (shp_ctx->actual.b_gain == 0x1F)) {
        mpi_cfg->en_chr_ctrl  = 0;
    } else {
        mpi_cfg->en_chr_ctrl  = 1;
        mpi_cfg->chr_r_gain   = shp_ctx->actual.r_gain;
        mpi_cfg->chr_g_gain   = shp_ctx->actual.g_gain;
        mpi_cfg->chr_gmf_gain = shp_ctx->actual.g_gain;
        mpi_cfg->chr_b_gain   = shp_ctx->actual.b_gain;
    }

    if (shp_ctx->actual.detail_ctrl == 0x80) {
        mpi_cfg->en_detail_ctrl = 0;
    } else {
        mpi_cfg->en_detail_ctrl = 1;
    }
}

static td_void sharpen_mpi2reg_mpi_detail(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i;

    isp_sharpen_mpi_dyna_reg_cfg *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg     *dyna_reg_cfg = TD_NULL;
    ot_isp_sharpen_manual_attr   *shp_actual   = &shp_ctx->actual;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;

    mpi_cfg->detail_osht_amt = clip3((mpi_cfg->osht_amt) + (shp_actual->detail_ctrl) - 0x80, 0, 0x7F);
    mpi_cfg->detail_usht_amt = clip3((mpi_cfg->usht_amt) + (shp_actual->detail_ctrl) - 0x80, 0, 0x7F);
    mpi_cfg->dir_diff_sft    = 0x3F - shp_actual->edge_filt_strength;

    if (shp_actual->edge_filt_max_cap <= 12) { /* max cap thd0 12 */
        mpi_cfg->dir_rt[1] = shp_actual->edge_filt_max_cap;
        mpi_cfg->dir_rt[0] = (shp_actual->edge_filt_max_cap) >> 1;
    } else if (shp_actual->edge_filt_max_cap <= 30) { /* max cap thd1 30 */
        mpi_cfg->dir_rt[1] = shp_actual->edge_filt_max_cap;
        mpi_cfg->dir_rt[0] = 0x6;
    } else {
        mpi_cfg->dir_rt[1] = 0x1E;
        mpi_cfg->dir_rt[0] = shp_actual->edge_filt_max_cap - 0x18;
    }

    mpi_cfg->en_sht_ctrl_by_var = 1;
    mpi_cfg->sht_bld_rt         = shp_actual->shoot_sup_adj;
    mpi_cfg->sht_var_thd1       = shp_actual->shoot_sup_strength;
    mpi_cfg->o_max_gain         = shp_actual->max_sharp_gain;
    mpi_cfg->u_max_gain         = shp_actual->max_sharp_gain;
    mpi_cfg->skin_max_u         = shp_ctx->mpi_cfg.skin_umax;
    mpi_cfg->skin_min_u         = shp_ctx->mpi_cfg.skin_umin;
    mpi_cfg->skin_max_v         = shp_ctx->mpi_cfg.skin_vmax;
    mpi_cfg->skin_min_v         = shp_ctx->mpi_cfg.skin_vmin;
    mpi_cfg->detail_osht_thr[0] = shp_actual->detail_ctrl_threshold;
    mpi_cfg->detail_osht_thr[1] = MIN2(0xFF, shp_actual->detail_ctrl_threshold + SHRP_DETAIL_CTRL_THR_DELTA);
    mpi_cfg->detail_usht_thr[0] = shp_actual->detail_ctrl_threshold;
    mpi_cfg->detail_usht_thr[1] = MIN2(0xFF, shp_actual->detail_ctrl_threshold + SHRP_DETAIL_CTRL_THR_DELTA);
    mpi_cfg->en_luma_ctrl = 0;
    for (i = 0; i < OT_ISP_SHARPEN_LUMA_NUM; i++) {
        mpi_cfg->luma_wgt[i] = shp_actual->luma_wgt[i];

        if (mpi_cfg->luma_wgt[i] < 0x1F) {
            mpi_cfg->en_luma_ctrl = 1;
        }
    }
}

static td_void sharpen_mpi2reg_mul_coef(isp_sharpen_reg_cfg *sharpen_reg_cfg)
{
    isp_sharpen_default_dyna_reg_cfg *def_cfg      = TD_NULL;
    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg      = TD_NULL;
    isp_sharpen_static_reg_cfg       *static_cfg   = TD_NULL;
    isp_sharpen_dyna_reg_cfg         *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    def_cfg      = &dyna_reg_cfg->default_dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;
    static_cfg   = &sharpen_reg_cfg->static_reg_cfg;

    /* calc all mul_coef */
    /* default */
    def_cfg->sht_var_diff_mul = calc_mul_coef(def_cfg->sht_var_diff_thd[0], static_cfg->sht_var_diff_wgt0,
                                              def_cfg->sht_var_diff_thd[1], def_cfg->sht_var_diff_wgt1,
                                              SHRP_SHT_VAR_MUL_PRECS);
    /* mpi */
    mpi_cfg->sht_var_mul      = calc_mul_coef(static_cfg->sht_var_thd0, def_cfg->sht_var_wgt0,
                                              mpi_cfg->sht_var_thd1, static_cfg->sht_var_wgt1, SHRP_SHT_VAR_MUL_PRECS);
    mpi_cfg->detail_osht_mul   = calc_mul_coef(mpi_cfg->detail_osht_thr[0], mpi_cfg->detail_osht_amt,
                                               mpi_cfg->detail_osht_thr[1], mpi_cfg->osht_amt,
                                               SHRP_DETAIL_SHT_MUL_PRECS);
    mpi_cfg->detail_usht_mul   = calc_mul_coef(mpi_cfg->detail_usht_thr[0], mpi_cfg->detail_usht_amt,
                                               mpi_cfg->detail_usht_thr[1], mpi_cfg->usht_amt,
                                               SHRP_DETAIL_SHT_MUL_PRECS);
    mpi_cfg->chr_r_mul         = calc_mul_coef(static_cfg->chr_r_thd[0], mpi_cfg->chr_r_gain,
                                               static_cfg->chr_r_thd[1], 0x20, SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_g_mul         = calc_mul_coef(static_cfg->chr_g_thd[0], mpi_cfg->chr_g_gain,
                                               static_cfg->chr_g_thd[1], 0x20, SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_gmf_mul       = calc_mul_coef(static_cfg->chr_g_thd[0], mpi_cfg->chr_gmf_gain,
                                               static_cfg->chr_g_thd[1], 0x20, SHRP_CHR_MUL_SFT);
    mpi_cfg->chr_b_mul         = calc_mul_coef(static_cfg->chr_b_thd[0], mpi_cfg->chr_b_gain,
                                               static_cfg->chr_b_thd[1], 0x20, SHRP_CHR_MUL_SFT);
    mpi_cfg->skin_edge_mul     = calc_mul_coef(static_cfg->skin_edge_thd[0], mpi_cfg->skin_edge_wgt[0],
                                               static_cfg->skin_edge_thd[1], mpi_cfg->skin_edge_wgt[1],
                                               SHRP_SKIN_EDGE_MUL_PRECS);
}

static td_void sharpen_mpi2reg_mpi_mot_ctrl(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg      = TD_NULL;
    mpi_cfg      = &sharpen_reg_cfg->dyna_reg_cfg.mpi_dyna_reg_cfg;

    mpi_cfg->mot_enable = shp_ctx->mpi_cfg.motion_en;
    mpi_cfg->mot_thd0   = shp_ctx->mpi_cfg.motion_threshold0;
    mpi_cfg->mot_thd1   = shp_ctx->mpi_cfg.motion_threshold1;
    mpi_cfg->mot_gain0  = shp_ctx->mpi_cfg.motion_gain0;
    mpi_cfg->mot_gain1  = shp_ctx->mpi_cfg.motion_gain1;
    mpi_cfg->print_sel  = shp_ctx->mpi_cfg.detail_map;
    mpi_cfg->mot_mul    = (td_u16)calc_mul_coef(mpi_cfg->mot_thd0, mpi_cfg->mot_gain0,
                                                mpi_cfg->mot_thd1, mpi_cfg->mot_gain1, 0);
}

static td_void sharpen_mpi2reg(isp_sharpen_reg_cfg *sharpen_reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    isp_sharpen_default_dyna_reg_cfg *def_cfg      = TD_NULL;
    isp_sharpen_mpi_dyna_reg_cfg     *mpi_cfg      = TD_NULL;
    isp_sharpen_dyna_reg_cfg         *dyna_reg_cfg = TD_NULL;

    dyna_reg_cfg = &sharpen_reg_cfg->dyna_reg_cfg;
    mpi_cfg      = &dyna_reg_cfg->mpi_dyna_reg_cfg;
    def_cfg      = &dyna_reg_cfg->default_dyna_reg_cfg;

    if (def_cfg->resh) {
        sharpen_mpi2reg_def(sharpen_reg_cfg, shp_ctx);
    }

    if (mpi_cfg->resh) {
        sharpen_mpi2reg_mf_gain_d(sharpen_reg_cfg, shp_ctx);
        sharpen_mpi2reg_mf_mot_gain_d(sharpen_reg_cfg, shp_ctx);
        if (def_cfg->gain_thd_sel_ud == 1) {
            sharpen_mpi2reg_mf_thd_sel_ud_enable(sharpen_reg_cfg, shp_ctx);
            sharpen_mpi2reg_mf_thd_sel_mot_ud_enable(sharpen_reg_cfg, shp_ctx);
        } else {
            sharpen_mpi2reg_mf_thd_sel_ud_disable(sharpen_reg_cfg, shp_ctx);
            sharpen_mpi2reg_mf_thd_sel_mot_ud_disable(sharpen_reg_cfg, shp_ctx);
        }
        sharpen_mpi2reg_mpi_chr(sharpen_reg_cfg, shp_ctx);
        sharpen_mpi2reg_mpi_detail(sharpen_reg_cfg, shp_ctx);
        sharpen_mpi2reg_mpi_mot_ctrl(sharpen_reg_cfg, shp_ctx);
    }

    sharpen_mpi2reg_mul_coef(sharpen_reg_cfg);
}

static __inline td_bool check_sharpen_open(const isp_sharpen_ctx *shp_ctx)
{
    return (shp_ctx->mpi_cfg.enable == TD_TRUE);
}

static td_void isp_sharpen_actual_update(ot_vi_pipe vi_pipe, ot_isp_sharpen_manual_attr *actual)
{
    td_u8 i;

    ot_ext_system_actual_sharpen_overshoot_amt_write(vi_pipe, actual->over_shoot);
    ot_ext_system_actual_sharpen_undershoot_amt_write(vi_pipe, actual->under_shoot);
    ot_ext_system_actual_sharpen_shoot_sup_write(vi_pipe, actual->shoot_sup_strength);
    ot_ext_system_actual_sharpen_edge_frequence_write(vi_pipe, actual->edge_freq);
    ot_ext_system_actual_sharpen_texture_frequence_write(vi_pipe, actual->texture_freq);

    for (i = 0; i < OT_ISP_SHARPEN_GAIN_NUM; i++) {
        ot_ext_system_actual_sharpen_edge_str_write(vi_pipe, i, actual->edge_strength[i]);
        ot_ext_system_actual_sharpen_texture_str_write(vi_pipe, i, actual->texture_strength[i]);
    }
}

static td_void isp_sharpen_luma_wgt_proc_write(ot_isp_ctrl_proc_write *proc_tmp,
    ot_isp_ctrl_proc_write *proc, const td_u8 *luma_wgt)
{
    td_u8 i, index;
    for (i = 0; i < 0x4; i++) {
        index = i * 0x8;
        isp_proc_printf(proc_tmp, proc->write_len, "%s"  "(%u"  "-"  "%d):\n", "luma_wgt", index, index + 0x7);
        isp_proc_printf(proc_tmp, proc->write_len, "  [%u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u]\n\n",
                        luma_wgt[index + 0x0], luma_wgt[index + 0x1], luma_wgt[index + 0x2], luma_wgt[index + 0x3],
                        luma_wgt[index + 0x4], luma_wgt[index + 0x5], luma_wgt[index + 0x6], luma_wgt[index + 0x7]);
    }
}

static td_void isp_sharpen_texture_strength_proc_write(ot_isp_ctrl_proc_write *proc_tmp,
    ot_isp_ctrl_proc_write *proc, const td_u16 *texture_strength)
{
    td_u8 i, index;
    for (i = 0; i < 0x4; i++) {
        index = i * 0x8;
        isp_proc_printf(proc_tmp, proc->write_len, "%s"  "(%u"  "-"  "%d):\n", "texture_strength", index, index + 0x7);
        isp_proc_printf(proc_tmp, proc->write_len, "  [%u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u]\n\n",
                        texture_strength[index + 0x0], texture_strength[index + 0x1],
                        texture_strength[index + 0x2], texture_strength[index + 0x3],
                        texture_strength[index + 0x4], texture_strength[index + 0x5],
                        texture_strength[index + 0x6], texture_strength[index + 0x7]);
    }
}

static td_void isp_sharpen_edge_strength_proc_write(ot_isp_ctrl_proc_write *proc_tmp,
    ot_isp_ctrl_proc_write *proc, const td_u16 *edge_strength)
{
    td_u8 i, index;
    for (i = 0; i < 0x4; i++) {
        index = i * 0x8;
        isp_proc_printf(proc_tmp, proc->write_len, "%s"  "(%u"  "-"  "%d):\n", "edge_strength", index, index + 0x7);
        isp_proc_printf(proc_tmp, proc->write_len, "  [%u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u"  "%8u]\n\n",
                        edge_strength[index + 0x0], edge_strength[index + 0x1],
                        edge_strength[index + 0x2], edge_strength[index + 0x3],
                        edge_strength[index + 0x4], edge_strength[index + 0x5],
                        edge_strength[index + 0x6], edge_strength[index + 0x7]);
    }
}

td_s32 sharpen_proc_write(ot_isp_ctrl_proc_write *proc, isp_sharpen_ctx *shp_ctx)
{
    ot_isp_ctrl_proc_write proc_tmp;
    ot_isp_sharpen_manual_attr *actual = TD_NULL;

    actual = &shp_ctx->actual;

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "sharpen info");
    isp_proc_printf(&proc_tmp, proc->write_len, "%16s\n", "sharpen_en");
    isp_proc_printf(&proc_tmp, proc->write_len, "%16u\n", (td_u16)shp_ctx->mpi_cfg.enable);

    isp_sharpen_luma_wgt_proc_write(&proc_tmp, proc, actual->luma_wgt);
    isp_sharpen_texture_strength_proc_write(&proc_tmp, proc, actual->texture_strength);
    isp_sharpen_edge_strength_proc_write(&proc_tmp, proc, actual->edge_strength);

    isp_proc_printf(&proc_tmp, proc->write_len, "%12s" "%12s" "%12s" "%12s" "%16s" "%12s \n",
                    "texture_freq", "edge_freq", "over_shoot", "under_shoot", "shoot_sup_str", "detail_ctrl");

    isp_proc_printf(&proc_tmp, proc->write_len, "%12u"  "%12u"  "%12u"  "%12u"  "%16u"  "%12u\n\n",
                    actual->texture_freq, actual->edge_freq, actual->over_shoot,
                    actual->under_shoot, actual->shoot_sup_strength, actual->detail_ctrl);

    isp_proc_printf(&proc_tmp, proc->write_len, "%14s" "%20s" "%8s" "%8s" "%8s" "%12s\n",
                    "edge_filt_str", "edge_filt_max_cap", "r_gain", "g_gain", "b_gain", "skin_gain");

    isp_proc_printf(&proc_tmp, proc->write_len, "%14u"  "%20u"  "%8u" "%8u"  "%8u" "%12u\n\n",
                    actual->edge_filt_strength, actual->edge_filt_max_cap, actual->r_gain,
                    actual->g_gain, actual->b_gain, actual->skin_gain);

    isp_proc_printf(&proc_tmp, proc->write_len, "%14s" "%18s" "%16s" "%12s" "%12s" "%12s"  "%12s\n",
                    "shoot_sup_adj", "detail_ctrl_thr", "max_sharp_gain",
                    "skin_umax", "skin_umin", "skin_vmax", "skin_vmin");

    isp_proc_printf(&proc_tmp, proc->write_len, "%14u"  "%18u"  "%16u"  "%12u"  "%12u"  "%12u" "%12u\n\n",
                    actual->shoot_sup_adj, actual->detail_ctrl_threshold, actual->max_sharp_gain,
                    shp_ctx->mpi_cfg.skin_umax, shp_ctx->mpi_cfg.skin_umin, shp_ctx->mpi_cfg.skin_vmax,
                    shp_ctx->mpi_cfg.skin_vmin);

    proc->write_len += 1;

    return TD_SUCCESS;
}

static td_void isp_sharpen_actual_calc(isp_sharpen_ctx *shp_ctx)
{
    if (shp_ctx->mpi_cfg.op_type == OT_OP_MODE_MANUAL) {
        (td_void)memcpy_s(&shp_ctx->actual, sizeof(ot_isp_sharpen_manual_attr),
                          &shp_ctx->mpi_cfg.manual_attr, sizeof(ot_isp_sharpen_manual_attr));
    } else {
        isp_sharpen_get_mpi_auto_inter_result(shp_ctx);
    }
}

static td_void sharpen_iso_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    td_u8  i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if ((shp_ctx->iso != shp_ctx->iso_last) ||
        (isp_ctx->linkage.fswdr_mode != isp_ctx->linkage.pre_fswdr_mode)) {   /* will not work if ISO is the same */
        isp_sharpen_get_default_reg_cfg(vi_pipe, shp_ctx);
        for (i = 0; i < reg_cfg->cfg_num; i++) {
            reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.default_dyna_reg_cfg.resh = TD_TRUE;
        }
    }

    if (shp_ctx->sharpen_mpi_update_en) {
        isp_sharpen_actual_calc(shp_ctx);

        for (i = 0; i < reg_cfg->cfg_num; i++) {
            reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.resh           = TD_TRUE;
            reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.update_index  += 1;
            sharpen_mpi2reg(&reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg, shp_ctx);
        }
    } else if (shp_ctx->iso != shp_ctx->iso_last) {
        isp_sharpen_actual_calc(shp_ctx);
        if (shp_ctx->mpi_cfg.op_type == OT_OP_MODE_AUTO) {  /* auto mode */
            for (i = 0; i < reg_cfg->cfg_num; i++) {
                reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.resh          = TD_TRUE;
                reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.update_index += 1;
                sharpen_mpi2reg(&reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg, shp_ctx);
            }
        } else {
            for (i = 0; i < reg_cfg->cfg_num; i++) {
                sharpen_mpi2reg(&reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg, shp_ctx);
            }
        }
    }

    shp_ctx->iso_last = shp_ctx->iso;    /* will not work if ISO is the same */
}

static td_void sharpen_inter_wgt_calc(isp_usr_ctx *isp_ctx, isp_sharpen_ctx *shp_ctx)
{
    const td_u8  sft = 0x8;
    td_u32 iso_lvl_cur, iso_lvl_pre;
    isp_sharpen_inter_info *inter_info = &shp_ctx->inter_info;

    /* sharpen strength linkage with the iso calculated by ae */
    inter_info->sft = sft;
    shp_ctx->iso = isp_ctx->linkage.iso;
    inter_info->idx_cur = get_iso_index(shp_ctx->iso);
    inter_info->idx_pre = (inter_info->idx_cur == 0) ? 0 : MAX2(inter_info->idx_cur - 1, 0);

    iso_lvl_cur = get_iso(inter_info->idx_cur);
    iso_lvl_pre = get_iso(inter_info->idx_pre);
    if (shp_ctx->iso <= iso_lvl_pre) {
        inter_info->wgt_pre = signed_left_shift(1, sft);
    } else if (shp_ctx->iso >= iso_lvl_cur) {
        inter_info->wgt_pre = 0;
    } else {
        inter_info->wgt_pre = signed_left_shift((iso_lvl_cur - shp_ctx->iso), sft) / (iso_lvl_cur - iso_lvl_pre);
    }
    inter_info->wgt_cur = signed_left_shift(1, sft) - inter_info->wgt_pre;
}

td_void isp_sharpen_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    sharpen_inter_wgt_calc(isp_ctx, shp_ctx);
    sharpen_iso_update(vi_pipe, reg_cfg, shp_ctx);
}

static td_s32 isp_sharpen_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_info, td_s32 rsv)
{
    td_u8   i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_SHARPEN;
    isp_usr_ctx     *isp_ctx = TD_NULL;
    isp_sharpen_ctx *shp_ctx = TD_NULL;
    isp_reg_cfg     *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    isp_get_ctx(vi_pipe, isp_ctx);
    sharpen_get_ctx(vi_pipe, shp_ctx);
    isp_check_pointer_return(shp_ctx);
    ot_unused(stat_info);
    ot_unused(rsv);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    ot_ext_system_isp_sharpen_init_status_write(vi_pipe, shp_ctx->init);
    if (shp_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    shp_ctx->mpi_cfg.enable = ot_ext_system_manual_sharpen_en_read(vi_pipe);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.enable = shp_ctx->mpi_cfg.enable;
        reg_cfg->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.switch_mode = TD_FALSE;
    }

    reg_cfg->cfg_key.bit1_sharpen_cfg = 1;

    /* check hardware setting */
    if (!check_sharpen_open(shp_ctx)) {
        return TD_SUCCESS;
    }

    isp_sharpen_read_extregs(vi_pipe, shp_ctx);
    isp_sharpen_read_pro_mode(vi_pipe, shp_ctx);

    isp_sharpen_reg_update(vi_pipe, reg_cfg, shp_ctx);
    isp_sharpen_actual_update(vi_pipe, &shp_ctx->actual);

    return TD_SUCCESS;
}

static td_s32 isp_sharpen_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;
    isp_sharpen_ctx *shp_ctx = TD_NULL;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
            isp_check_pointer_return(reg_cfg_attr);
            isp_sharpen_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg_attr->reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            sharpen_get_ctx(vi_pipe, shp_ctx);
            isp_check_pointer_return(shp_ctx);
            sharpen_proc_write((ot_isp_ctrl_proc_write *)value, shp_ctx);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_sharpen_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg   = TD_NULL;
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    ot_ext_system_isp_sharpen_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].sharpen_reg_cfg.enable = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_sharpen_cfg = 1;

    ot_ext_system_sharpen_manu_mode_write(vi_pipe, TD_FALSE);

    sharpen_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_sharpen(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_sharpen);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_SHARPEN;
    algs->alg_func.pfn_alg_init = isp_sharpen_init;
    algs->alg_func.pfn_alg_run  = isp_sharpen_run;
    algs->alg_func.pfn_alg_ctrl = isp_sharpen_ctrl;
    algs->alg_func.pfn_alg_exit = isp_sharpen_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
