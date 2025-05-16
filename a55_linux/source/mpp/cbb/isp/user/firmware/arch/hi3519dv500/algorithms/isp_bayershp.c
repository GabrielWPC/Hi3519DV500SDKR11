/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_isp_debug.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"
#include "isp_param_check.h"
#include "isp_ext_reg_access.h"

#define BSHP_BIT_DEPTH                   12
#define BSHP_DIR_RATE_HV_DEFAULT         7
#define BSHP_DIR_RATE_CD_DEFAULT         7
#define BSHP_SHT_CTRL_WGT_LOW_DEFAULT    128
#define BSHP_SHT_CTRL_WGT_HIGH_DEFAULT   0
#define BSHP_DARK_THD_LOW_DEFAULT        180
#define BSHP_DARK_THD_HIGH_DEFAULT       220
#define BSHP_SHT_CTRL_THD_LOW_DEFAULT    200
#define BSHP_SHT_CTRL_THD_HIGH_DEFAULT   300
#define BSHP_GAIN_LUT_VALUE_DEFAULT      32
#define BSHP_MF_GAIN_DEFAULT             3
#define BSHP_HF_GAIN_DEFAULT             2
#define BSHP_DARK_GAIN_DEFAULT           1
#define BSHP_TEXTURE_THD_DEFAULT         400
#define BSHP_OVER_SHT_AMT_DEFAULT        128
#define BSHP_UNDER_SHT_AMT_DEFAULT       128
#define BSHP_OVER_MAX_CHG_DEFAULT        300
#define BSHP_UNDER_MAX_CHG_DEFAULT       300

typedef struct {
    td_bool init;
    td_bool bayershp_att_update;
    ot_isp_bayershp_manual_attr actual; /* actual param */
    ot_isp_bayershp_attr mpi_cfg;
} isp_bayershp;


static const td_s16 g_lpf_coef_ud[OT_ISP_BSHP_LPF_SIZE] = {0, 16, 32};
static const td_s16 g_hsf_coef_ud[OT_ISP_BSHP_LPF_SIZE] = {-4, 18, 36};

static const td_u16 g_mf_gain[OT_ISP_AUTO_ISO_NUM] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

static const td_u16 g_hf_gain[OT_ISP_AUTO_ISO_NUM] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

static const td_u16 g_dark_gain[OT_ISP_AUTO_ISO_NUM] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const td_u16 g_over_sht_amt[OT_ISP_AUTO_ISO_NUM] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};

static const td_u16 g_under_sht_amt[OT_ISP_AUTO_ISO_NUM] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};

static const td_u16 g_tex_var_map_thd[OT_ISP_AUTO_ISO_NUM] = {
    400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400
};

static const td_u16 g_omax_chg[OT_ISP_AUTO_ISO_NUM] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300
};

static const td_u16 g_umax_chg[OT_ISP_AUTO_ISO_NUM] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300
};


isp_bayershp *g_bayershp_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define bayershp_get_ctx(pipe, ctx)   ((ctx) = g_bayershp_ctx[pipe])
#define bayershp_set_ctx(pipe, ctx)   (g_bayershp_ctx[pipe] = (ctx))
#define bayershp_reset_ctx(pipe)      (g_bayershp_ctx[pipe] = TD_NULL)

static td_s32 bayershp_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_bayershp *bayershp_ctx = TD_NULL;

    bayershp_get_ctx(vi_pipe, bayershp_ctx);

    /* isp_malloc memory */
    if (bayershp_ctx == TD_NULL) {
        bayershp_ctx = (isp_bayershp *)isp_malloc(sizeof(isp_bayershp));
        if (bayershp_ctx == TD_NULL) {
            isp_err_trace("isp[%d] bayershp_ctx isp_malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }
    (td_void)memset_s(bayershp_ctx, sizeof(isp_bayershp), 0, sizeof(isp_bayershp));
    bayershp_set_ctx(vi_pipe, bayershp_ctx);

    return TD_SUCCESS;
}

static td_void bayershp_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_bayershp *sharpen_ctx = TD_NULL;
    bayershp_get_ctx(vi_pipe, sharpen_ctx);
    isp_free(sharpen_ctx);
    bayershp_reset_ctx(vi_pipe);
}

/*  bayershp hardware regs that will not change  */
static td_void bshp_sta_reg_init(ot_vi_pipe vi_pipe, isp_bshp_static_cfg *static_regcfg)
{
    td_u8 i;
    ot_unused(vi_pipe);

    static_regcfg->sht_ctrl_by_var_en  = TD_TRUE;
    static_regcfg->sht_ctrl_shrink_en  = TD_TRUE;
    static_regcfg->bit_depth           = BSHP_BIT_DEPTH;
    static_regcfg->dir_rate_hv         = BSHP_DIR_RATE_HV_DEFAULT;
    static_regcfg->dir_rate_cd         = BSHP_DIR_RATE_CD_DEFAULT;
    static_regcfg->sht_ctrl_var_wgt[0] = BSHP_SHT_CTRL_WGT_LOW_DEFAULT;
    static_regcfg->sht_ctrl_var_wgt[1] = BSHP_SHT_CTRL_WGT_HIGH_DEFAULT;
    for (i = 0; i < 3; i++) { /* fliter num is 3 */
        static_regcfg->lpf_coef_ud[i]  = g_lpf_coef_ud[i];
        static_regcfg->hsf_coef_ud[i]  = g_hsf_coef_ud[i];
    }
    static_regcfg->static_resh         = TD_TRUE;
}

static td_void bshp_user_reg_init(isp_bshp_user_cfg *user_regcfg)
{
    user_regcfg->dark_thd[0]          = BSHP_DARK_THD_LOW_DEFAULT;
    user_regcfg->dark_thd[1]          = BSHP_DARK_THD_HIGH_DEFAULT;
    user_regcfg->sht_ctrl_var_thd[0]  = BSHP_SHT_CTRL_THD_LOW_DEFAULT;
    user_regcfg->sht_ctrl_var_thd[1]  = BSHP_SHT_CTRL_THD_HIGH_DEFAULT;
    user_regcfg->user_resh = TD_TRUE;
    return;
}

static td_void bshp_dyna_reg_init(isp_bshp_dyna_cfg *dyna_regcfg)
{
    td_u8 i;
    dyna_regcfg->mf_gain         = BSHP_MF_GAIN_DEFAULT;
    dyna_regcfg->hf_gain         = BSHP_HF_GAIN_DEFAULT;
    dyna_regcfg->dark_gain       = BSHP_DARK_GAIN_DEFAULT;
    dyna_regcfg->tex_var_map_thd = BSHP_TEXTURE_THD_DEFAULT;
    dyna_regcfg->over_shoot_amt  = BSHP_OVER_SHT_AMT_DEFAULT;
    dyna_regcfg->under_shoot_amt = BSHP_UNDER_SHT_AMT_DEFAULT;
    dyna_regcfg->over_max_chg    = BSHP_OVER_MAX_CHG_DEFAULT;
    dyna_regcfg->under_max_chg   = BSHP_UNDER_MAX_CHG_DEFAULT;
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        dyna_regcfg->mf_gain_lut[i]   =  BSHP_GAIN_LUT_VALUE_DEFAULT;
        dyna_regcfg->hf_gain_lut[i]   =  BSHP_GAIN_LUT_VALUE_DEFAULT;
        dyna_regcfg->dark_gain_lut[i] =  BSHP_GAIN_LUT_VALUE_DEFAULT;
    }
    dyna_regcfg->dyna_resh = TD_TRUE;
    dyna_regcfg->buf_id = 0;
    return;
}

static td_void bayershp_ext_reg_initialize(ot_vi_pipe vi_pipe, isp_bayershp *bayer_sharpen)
{
    ot_ext_system_bshp_manual_mode_write(vi_pipe, bayer_sharpen->mpi_cfg.op_type);
    ot_ext_system_bshp_enable_write(vi_pipe, bayer_sharpen->mpi_cfg.enable);

    isp_bayershp_manual_attr_write(vi_pipe, &bayer_sharpen->mpi_cfg.manual_attr);
    isp_bayershp_auto_attr_write(vi_pipe, &bayer_sharpen->mpi_cfg.auto_attr);
    isp_bayershp_common_attr_write(vi_pipe, &bayer_sharpen->mpi_cfg);
}

static td_void bayershp_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_bayershp *bayer_sharpen)
{
    td_u32 i;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].bshp_reg_cfg.bshp_enable = bayer_sharpen->mpi_cfg.enable;
        reg_cfg->alg_reg_cfg[i].bshp_reg_cfg.lut2_stt_en  = TD_TRUE;
        bshp_sta_reg_init(vi_pipe, &reg_cfg->alg_reg_cfg[i].bshp_reg_cfg.static_reg_cfg);
        bshp_user_reg_init(&reg_cfg->alg_reg_cfg[i].bshp_reg_cfg.usr_reg_cfg);
        bshp_dyna_reg_init(&reg_cfg->alg_reg_cfg[i].bshp_reg_cfg.dyna_reg_cfg);
    }
    reg_cfg->cfg_key.bit1_bshp_cfg = 1;
}

static td_s32 bshp_check_cmos_param(const ot_isp_bayershp_attr *bshp)
{
    td_s32 ret;

    ret = isp_bshp_comm_attr_check("cmos", bshp);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_bshp_auto_attr_check("cmos", &bshp->auto_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_bshp_manual_attr_check("cmos", &bshp->manual_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}
static td_s32 isp_bshp_ctx_sns_init(isp_bayershp *bshp, ot_isp_cmos_default *sns_dft)
{
    td_s32 ret;
    isp_check_pointer_return(sns_dft->bshp);
    ret = bshp_check_cmos_param(sns_dft->bshp);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    (td_void)memcpy_s(&bshp->mpi_cfg, sizeof(ot_isp_bayershp_attr), sns_dft->bshp, sizeof(ot_isp_bayershp_attr));

    return TD_SUCCESS;
}

static td_void bshp_create_mf_strength_def(ot_isp_bayershp_attr *mpi_cfg)
{
    td_u16 i;
    size_t size;
    td_s32 ret;
    const td_u8 mf_strength[OT_ISP_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM] = {
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32}
    };
    size = OT_ISP_BSHP_CURVE_NUM * OT_ISP_AUTO_ISO_NUM;
    ret = memcpy_s(mpi_cfg->auto_attr.mf_strength, size * sizeof(td_u8), mf_strength, size * sizeof(td_u8));
    if (ret != EOK) {
        isp_err_trace("ISP init bayersharpen mf_strength lut failed!\n");
        return;
    }
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        mpi_cfg->manual_attr.mf_strength[i] = mf_strength[i][0];
    }
}

static td_void bshp_create_hf_strength_def(ot_isp_bayershp_attr *mpi_cfg)
{
    td_u16 i;
    size_t size;
    td_s32 ret;
    const td_u8 hf_strength[OT_ISP_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM] = {
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32},
        {16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16},
        {16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
    };
    size = OT_ISP_BSHP_CURVE_NUM * OT_ISP_AUTO_ISO_NUM;
    ret = memcpy_s(mpi_cfg->auto_attr.hf_strength, size * sizeof(td_u8), hf_strength, size * sizeof(td_u8));
    if (ret != EOK) {
        isp_err_trace("ISP init bayersharpen hf_strength lut failed!\n");
        return;
    }
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        mpi_cfg->manual_attr.hf_strength[i] = hf_strength[i][0];
    }
}

static td_void bshp_create_dark_strength_def(ot_isp_bayershp_attr *mpi_cfg)
{
    td_u16 i;
    size_t size;
    td_s32 ret;
    const td_u8 dark_strength[OT_ISP_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM] = {
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16},
        {16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16},
        {4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4},
        {2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
        {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}
    };
    size = OT_ISP_BSHP_CURVE_NUM * OT_ISP_AUTO_ISO_NUM;
    ret = memcpy_s(mpi_cfg->auto_attr.dark_strength, size * sizeof(td_u8), dark_strength, size * sizeof(td_u8));
    if (ret != EOK) {
        isp_err_trace("ISP init bayersharpen dark_strength lut failed!\n");
        return;
    }
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        mpi_cfg->manual_attr.dark_strength[i] = dark_strength[i][0];
    }
}

static td_s32 isp_bshp_ctx_default_init(isp_bayershp *bshp)
{
    td_s32 i;
    bshp->mpi_cfg.enable = TD_TRUE;
    bshp->mpi_cfg.dark_threshold[0]     = OT_EXT_SYSTEM_BSHP_DARK_THD_LOW_DEFAULT;
    bshp->mpi_cfg.dark_threshold[1]     = OT_EXT_SYSTEM_BSHP_DARK_THD_HIGH_DEFAULT;
    bshp->mpi_cfg.texture_threshold[0]  = OT_EXT_SYSTEM_BSHP_TEXTURE_THD_LOW_DEFAULT;
    bshp->mpi_cfg.texture_threshold[1]  = OT_EXT_SYSTEM_BSHP_TEXTURE_THD_HIGH_DEFAULT;

    /* auto */
    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        bshp->mpi_cfg.auto_attr.mf_gain[i]    = g_mf_gain[i];
        bshp->mpi_cfg.auto_attr.hf_gain[i]    = g_hf_gain[i];
        bshp->mpi_cfg.auto_attr.dark_gain[i]  = g_dark_gain[i];
        bshp->mpi_cfg.auto_attr.overshoot[i]  = g_over_sht_amt[i];
        bshp->mpi_cfg.auto_attr.undershoot[i] = g_under_sht_amt[i];
    }

    /* manual */
    bshp->mpi_cfg.manual_attr.mf_gain    = OT_EXT_SYSTEM_BSHP_MF_GAIN_DEFAULT;
    bshp->mpi_cfg.manual_attr.hf_gain    = OT_EXT_SYSTEM_BSHP_HF_GAIN_DEFAULT;
    bshp->mpi_cfg.manual_attr.dark_gain  = OT_EXT_SYSTEM_BSHP_DARK_GAIN_DEFAULT;
    bshp->mpi_cfg.manual_attr.overshoot  = OT_EXT_SYSTEM_BSHP_OVERSHOOT_DEFAULT;
    bshp->mpi_cfg.manual_attr.undershoot = OT_EXT_SYSTEM_BSHP_UNDERSHOOT_DEFAULT;

    bshp_create_mf_strength_def(&bshp->mpi_cfg);
    bshp_create_hf_strength_def(&bshp->mpi_cfg);
    bshp_create_dark_strength_def(&bshp->mpi_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_bshp_ctx_initialize(ot_vi_pipe vi_pipe, isp_bayershp *bshp)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    if (sns_dft->key.bit1_bshp) {
        ret = isp_bshp_ctx_sns_init(bshp, sns_dft);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else {
        ret = isp_bshp_ctx_default_init(bshp);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_bayershp_curve_proc_write(ot_isp_ctrl_proc_write *proc_tmp,
    ot_isp_ctrl_proc_write *proc, const td_u8 *str, const char *curve_name)
{
    td_u8 i;
    isp_proc_printf(proc_tmp, proc->write_len, "%s:\n", curve_name);
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        isp_proc_printf(proc_tmp, proc->write_len, "%8u", str[i]);
    }
    isp_proc_printf(proc_tmp, proc->write_len, "\n\n");
}

static td_s32 bayershp_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc, const isp_bayershp *bayershp_ctx)
{
    ot_isp_ctrl_proc_write proc_tmp;
    ot_unused(vi_pipe);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "bayershp info");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16s" "%16s" "%16s" "%16s" "%16s" "%16s\n",
                    "enable", "mf_gain", "hf_gain", "dark_gain", "overshoot", "undershoot");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%16u" "%16u" "%16u" "%16u" "%16u" "%16u\n",
                    bayershp_ctx->mpi_cfg.enable, bayershp_ctx->actual.mf_gain,
                    bayershp_ctx->actual.hf_gain, bayershp_ctx->actual.dark_gain,
                    bayershp_ctx->actual.overshoot, bayershp_ctx->actual.undershoot);

    isp_bayershp_curve_proc_write(&proc_tmp, proc, bayershp_ctx->actual.mf_strength, "mf_strength");
    isp_bayershp_curve_proc_write(&proc_tmp, proc, bayershp_ctx->actual.hf_strength, "hf_strength");
    isp_bayershp_curve_proc_write(&proc_tmp, proc, bayershp_ctx->actual.dark_strength, "dark_strength");

    proc->write_len += 1;
    return TD_SUCCESS;
}

static td_s32 isp_bayershp_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_bayershp *bayer_sharpen)
{
    td_s32 ret;
    /* ctx value init */
    bayer_sharpen->init = TD_FALSE;
    ret = isp_bshp_ctx_initialize(vi_pipe, bayer_sharpen);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    /* regs init */
    bayershp_regs_initialize(vi_pipe, reg_cfg, bayer_sharpen);
    bayershp_ext_reg_initialize(vi_pipe, bayer_sharpen);

    bayer_sharpen->init = TD_TRUE;
    ot_ext_system_isp_bshp_init_status_write(vi_pipe, bayer_sharpen->init);
    return TD_SUCCESS;
}

static td_s32 isp_bayershp_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    isp_bayershp *bayer_sharpen = TD_NULL;

    ot_ext_system_isp_bshp_init_status_write(vi_pipe, TD_FALSE);
    ret = bayershp_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    bayershp_get_ctx(vi_pipe, bayer_sharpen);
    isp_check_pointer_return(bayer_sharpen);

    return isp_bayershp_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg, bayer_sharpen);
}

static td_void isp_bayershp_read_ext_regs(ot_vi_pipe vi_pipe, isp_bayershp *bshp)
{
    bshp->bayershp_att_update = ot_ext_system_bshp_attr_update_read(vi_pipe);
    if (bshp->bayershp_att_update != TD_TRUE) {
        return;
    }

    ot_ext_system_bshp_attr_update_write(vi_pipe, TD_FALSE);
    bshp->mpi_cfg.op_type = ot_ext_system_bshp_manual_mode_read(vi_pipe);

    if (bshp->mpi_cfg.op_type == OT_OP_MODE_MANUAL) {
        isp_bayershp_manual_attr_read(vi_pipe, &bshp->mpi_cfg.manual_attr);
    } else {
        isp_bayershp_auto_attr_read(vi_pipe, &bshp->mpi_cfg.auto_attr);
    }
    isp_bayershp_common_attr_read(vi_pipe, &bshp->mpi_cfg);

    return;
}

static td_void isp_bshp_dyna_reg_update(td_u32 vi_pipe, td_u32 iso, isp_bshp_dyna_cfg *dyna_cfg, isp_bayershp *bshp_ctx)
{
    td_u8  i, iso_u, iso_d;
    td_u32 iso1, iso2;

    ot_unused(vi_pipe);

    iso_u = get_iso_index(iso);
    iso_d = MAX2((td_s8)iso_u - 1, 0);
    iso1 = get_iso(iso_d);
    iso2 = get_iso(iso_u);

    dyna_cfg->mf_gain           = bshp_ctx->actual.mf_gain;
    dyna_cfg->hf_gain           = bshp_ctx->actual.hf_gain;
    dyna_cfg->dark_gain         = bshp_ctx->actual.dark_gain;
    dyna_cfg->over_shoot_amt    = bshp_ctx->actual.overshoot;
    dyna_cfg->under_shoot_amt   = bshp_ctx->actual.undershoot;
    dyna_cfg->tex_var_map_thd   = (td_u16)linear_inter(iso, iso1, g_tex_var_map_thd[iso_d],
                                                       iso2, g_tex_var_map_thd[iso_u]);
    dyna_cfg->over_max_chg      = (td_u16)linear_inter(iso, iso1, g_omax_chg[iso_d],
                                                       iso2, g_omax_chg[iso_u]);
    dyna_cfg->under_max_chg     = (td_u16)linear_inter(iso, iso1, g_umax_chg[iso_d],
                                                       iso2, g_umax_chg[iso_u]);

    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        dyna_cfg->mf_gain_lut[i] = bshp_ctx->actual.mf_strength[i];
        dyna_cfg->hf_gain_lut[i] = bshp_ctx->actual.hf_strength[i];
        dyna_cfg->dark_gain_lut[i] = bshp_ctx->actual.dark_strength[i];
    }
}

static td_void isp_bshp_user_reg_update(isp_bshp_user_cfg *user_cfg, isp_bayershp *bshp_ctx)
{
    user_cfg->dark_thd[0]          = bshp_ctx->mpi_cfg.dark_threshold[0];
    user_cfg->dark_thd[1]          = bshp_ctx->mpi_cfg.dark_threshold[1];
    user_cfg->sht_ctrl_var_thd[0]  = bshp_ctx->mpi_cfg.texture_threshold[0];
    user_cfg->sht_ctrl_var_thd[1]  = bshp_ctx->mpi_cfg.texture_threshold[1];
    user_cfg->user_resh = TD_TRUE;
}

static td_void isp_bayershp_fw(td_u32 iso, ot_vi_pipe vi_pipe, isp_bshp_reg_cfg *bshp_reg_cfg, isp_bayershp *bshp_ctx)
{
    isp_bshp_dyna_cfg *dyna_cfg = &bshp_reg_cfg->dyna_reg_cfg;
    isp_bshp_user_cfg *user_cfg = &bshp_reg_cfg->usr_reg_cfg;
    /* update dyna regs */
    isp_bshp_dyna_reg_update(vi_pipe, iso, dyna_cfg, bshp_ctx);
    /* update user regs */
    isp_bshp_user_reg_update(user_cfg, bshp_ctx);

    dyna_cfg->dyna_resh = TD_TRUE;
}

static td_void isp_bayershp_get_mpi_auto_inter_result(td_u32 iso, isp_bayershp *bshp)
{
    td_u8  i, iso_u, iso_d;
    td_u32 iso1, iso2;
    ot_isp_bayershp_auto_attr   *auto_attr   = &bshp->mpi_cfg.auto_attr;
    ot_isp_bayershp_manual_attr *actual      = &bshp->actual;

    iso_u = get_iso_index(iso);
    iso_d = MAX2((td_s8)iso_u - 1, 0);
    iso1 = get_iso(iso_d);
    iso2 = get_iso(iso_u);

    actual->mf_gain       = (td_u8)linear_inter(iso, iso1, auto_attr->mf_gain[iso_d],
                                                iso2, auto_attr->mf_gain[iso_u]);
    actual->hf_gain       = (td_u8)linear_inter(iso, iso1, auto_attr->hf_gain[iso_d],
                                                iso2, auto_attr->hf_gain[iso_u]);
    actual->dark_gain     = (td_u8)linear_inter(iso, iso1, auto_attr->dark_gain[iso_d],
                                                iso2, auto_attr->dark_gain[iso_u]);
    actual->overshoot     = (td_u16)linear_inter(iso, iso1, auto_attr->overshoot[iso_d],
                                                 iso2, auto_attr->overshoot[iso_u]);
    actual->undershoot    = (td_u16)linear_inter(iso, iso1, auto_attr->undershoot[iso_d],
                                                 iso2, auto_attr->undershoot[iso_u]);
    for (i = 0; i < OT_ISP_BSHP_CURVE_NUM; i++) {
        actual->mf_strength[i]   = (td_u8)linear_inter(iso, iso1, auto_attr->mf_strength[i][iso_d],
                                                       iso2, auto_attr->mf_strength[i][iso_u]);
        actual->hf_strength[i]   = (td_u8)linear_inter(iso, iso1, auto_attr->hf_strength[i][iso_d],
                                                       iso2, auto_attr->hf_strength[i][iso_u]);
        actual->dark_strength[i] = (td_u8)linear_inter(iso, iso1, auto_attr->dark_strength[i][iso_d],
                                                       iso2, auto_attr->dark_strength[i][iso_u]);
    }
}

static td_void isp_bayershp_actual_calc(td_u32 iso, isp_bayershp *bayershp_ctx)
{
    if (bayershp_ctx->mpi_cfg.op_type == OT_OP_MODE_MANUAL) {
        (td_void)memcpy_s(&bayershp_ctx->actual, sizeof(ot_isp_bayershp_manual_attr),
                          &bayershp_ctx->mpi_cfg.manual_attr, sizeof(ot_isp_bayershp_manual_attr));
    } else {
        isp_bayershp_get_mpi_auto_inter_result(iso, bayershp_ctx);
    }
}

static td_void isp_bayershp_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_bayershp *bayershp_ctx)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_bayershp_actual_calc(isp_ctx->linkage.iso, bayershp_ctx);

    isp_bayershp_fw(isp_ctx->linkage.iso, vi_pipe, &reg_cfg->alg_reg_cfg[0].bshp_reg_cfg, bayershp_ctx);

    for (i = 1; i < isp_ctx->block_attr.block_num; i++) {
        (td_void)memcpy_s(&reg_cfg->alg_reg_cfg[i].bshp_reg_cfg, sizeof(isp_bshp_reg_cfg),
                          &reg_cfg->alg_reg_cfg[0].bshp_reg_cfg, sizeof(isp_bshp_reg_cfg));
    }
}

static __inline td_bool  check_bshp_open(const isp_bayershp *bayershp_ctx)
{
    return (bayershp_ctx->mpi_cfg.enable == TD_TRUE);
}

static td_s32 isp_bayershp_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8 i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_BSHP;
    isp_reg_cfg *local_reg = (isp_reg_cfg *)reg_cfg;
    isp_bayershp *bayershp_ctx = TD_NULL;
    bayershp_get_ctx(vi_pipe, bayershp_ctx);
    isp_check_pointer_return(bayershp_ctx);

    ot_unused(stat_info);
    ot_unused(rsv);

    ot_ext_system_isp_bshp_init_status_write(vi_pipe, bayershp_ctx->init);
    if (bayershp_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }
    bayershp_ctx->mpi_cfg.enable = ot_ext_system_bshp_enable_read(vi_pipe);
    for (i = 0; i < local_reg->cfg_num; i++) {
        local_reg->alg_reg_cfg[i].bshp_reg_cfg.bshp_enable = bayershp_ctx->mpi_cfg.enable;
    }
    local_reg->cfg_key.bit1_bshp_cfg = 1;

    if (!check_bshp_open(bayershp_ctx)) {
        return TD_SUCCESS;
    }
    isp_bayershp_read_ext_regs(vi_pipe, bayershp_ctx);

    isp_bayershp_reg_update(vi_pipe, local_reg, bayershp_ctx);

    return TD_SUCCESS;
}

static td_s32 isp_bayershp_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_bayershp *bayershp_ctx = TD_NULL;
    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            break;

        case OT_ISP_PROC_WRITE:
            bayershp_get_ctx(vi_pipe, bayershp_ctx);
            isp_check_pointer_return(bayershp_ctx);
            bayershp_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value, bayershp_ctx);
            break;

        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_bayershp_exit(ot_vi_pipe vi_pipe)
{
    td_u16 i;
    isp_reg_cfg_attr *regcfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, regcfg);
    ot_ext_system_isp_bshp_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < regcfg->reg_cfg.cfg_num; i++) {
        regcfg->reg_cfg.alg_reg_cfg[i].bshp_reg_cfg.bshp_enable = TD_FALSE;
    }

    regcfg->reg_cfg.cfg_key.bit1_bshp_cfg = 1;

    bayershp_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_bayershp(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_bshp);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_BSHP;
    algs->alg_func.pfn_alg_init = isp_bayershp_init;
    algs->alg_func.pfn_alg_run  = isp_bayershp_run;
    algs->alg_func.pfn_alg_ctrl = isp_bayershp_ctrl;
    algs->alg_func.pfn_alg_exit = isp_bayershp_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
