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

static const td_u8 g_afc_thd[OT_ISP_AUTO_ISO_NUM] = {8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 4, 3, 2, 1, 0};
static const td_u8 g_afc_str[OT_ISP_AUTO_ISO_NUM] = {8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 4, 3, 2, 1, 0};

typedef struct {
    td_bool init;
    td_u8   lut_afc_gray_ratio[OT_ISP_AUTO_ISO_NUM];             /* u5.0, */
    td_u8   lut_afc_color_cmax_sel[OT_ISP_AUTO_ISO_NUM];         /* u4.0, */
    td_u8   lut_afc_detg_sel[OT_ISP_AUTO_ISO_NUM];               /* u4.0, */
    td_u16  lut_afc_hf_thresh_low[OT_ISP_AUTO_ISO_NUM];          /* 10.0, */
    td_u16  lut_afc_hf_thresh_hig[OT_ISP_AUTO_ISO_NUM];          /* 10.0, */

    td_u8   fcr_gray_ratio;
    td_u8   fcr_cmax_sel;
    td_u8   fcr_detg_sel;
    td_u16  fcr_hf_thresh_low;
    td_u16  fcr_hf_thresh_hig;
    ot_isp_anti_false_color_manual_attr actual;
    ot_isp_anti_false_color_attr mpi_cfg;
} isp_antifalsecolor;

isp_antifalsecolor *g_afc_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define antifalsecolor_get_ctx(pipe, ctx)   ((ctx) = g_afc_ctx[pipe])
#define antifalsecolor_set_ctx(pipe, ctx)   (g_afc_ctx[pipe] = (ctx))
#define antifalsecolor_reset_ctx(pipe)      (g_afc_ctx[pipe] = TD_NULL)

static td_s32 anti_false_color_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_antifalsecolor *fcr_ctx = TD_NULL;

    antifalsecolor_get_ctx(vi_pipe, fcr_ctx);

    if (fcr_ctx == TD_NULL) {
        fcr_ctx = (isp_antifalsecolor *)isp_malloc(sizeof(isp_antifalsecolor));
        if (fcr_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] anti_false_color_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(fcr_ctx, sizeof(isp_antifalsecolor), 0, sizeof(isp_antifalsecolor));

    antifalsecolor_set_ctx(vi_pipe, fcr_ctx);

    return TD_SUCCESS;
}

static td_void anti_false_color_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_antifalsecolor *afc_ctx = TD_NULL;

    antifalsecolor_get_ctx(vi_pipe, afc_ctx);
    isp_free(afc_ctx);
    antifalsecolor_reset_ctx(vi_pipe);
}

static td_void anti_false_color_init_fw_wdr(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    ot_unused(vi_pipe);

    td_u8 lut_afc_gray_ratio_wdr[OT_ISP_AUTO_ISO_NUM] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
    td_u8 lut_afc_color_cmax_sel_wdr[OT_ISP_AUTO_ISO_NUM] = { 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
    td_u8 lut_afc_detg_sel_wdr[OT_ISP_AUTO_ISO_NUM] = { 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    td_u16 lut_afc_hf_thd_low_wdr[OT_ISP_AUTO_ISO_NUM] = {
        96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96
    };
    td_u16 lut_afc_hf_thd_hig_wdr[OT_ISP_AUTO_ISO_NUM] = {
        256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256
    };

    (td_void)memcpy_s(fcr_ctx->lut_afc_gray_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_gray_ratio_wdr, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_color_cmax_sel, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_color_cmax_sel_wdr, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_detg_sel, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_detg_sel_wdr, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_hf_thresh_low, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_hf_thd_low_wdr, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_hf_thresh_hig, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_hf_thd_hig_wdr, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
}

static td_void anti_false_color_init_fw_linear(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    ot_unused(vi_pipe);

    td_u8   lut_afc_gray_ratio_lin[OT_ISP_AUTO_ISO_NUM] = {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
    };
    td_u8   lut_afc_color_cmax_sel_lin[OT_ISP_AUTO_ISO_NUM] = {
        4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
    };
    td_u8   lut_afc_detg_sel_lin[OT_ISP_AUTO_ISO_NUM] = {
        4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    td_u16  lut_afc_hf_thd_low_lin[OT_ISP_AUTO_ISO_NUM] = {
        30, 30, 35, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48
    };
    td_u16  lut_afc_hf_thd_hig_lin[OT_ISP_AUTO_ISO_NUM] = {
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
    };

    (td_void)memcpy_s(fcr_ctx->lut_afc_gray_ratio, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_gray_ratio_lin, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_color_cmax_sel, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_color_cmax_sel_lin, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_detg_sel, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_detg_sel_lin, sizeof(td_u8) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_hf_thresh_low, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_hf_thd_low_lin, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
    (td_void)memcpy_s(fcr_ctx->lut_afc_hf_thresh_hig, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM,
                      lut_afc_hf_thd_hig_lin, sizeof(td_u16) * OT_ISP_AUTO_ISO_NUM);
}

static td_s32 anti_false_color_set_long_frame_mode(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_linear_mode(isp_ctx->sns_wdr_mode) ||
        (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) ||
        (isp_ctx->linkage.fswdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE)) {
        anti_false_color_init_fw_linear(vi_pipe, fcr_ctx);
    } else {
        anti_false_color_init_fw_wdr(vi_pipe, fcr_ctx);
    }

    return TD_SUCCESS;
}

static td_void anti_false_color_ext_regs_initialize(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    isp_anti_false_color_attr_write(vi_pipe, &fcr_ctx->mpi_cfg);
}

static td_void anti_false_color_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg  *reg_cfg, isp_antifalsecolor *fcr_ctx)
{
    td_u32 i;
    isp_antifalsecolor_static_cfg *static_reg_cfg = TD_NULL;
    isp_antifalsecolor_dyna_cfg   *dyna_reg_cfg   = TD_NULL;

    ot_unused(vi_pipe);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].anti_false_color_reg_cfg.fcr_enable = fcr_ctx->mpi_cfg.enable;
        static_reg_cfg = &reg_cfg->alg_reg_cfg[i].anti_false_color_reg_cfg.static_reg_cfg;
        dyna_reg_cfg   = &reg_cfg->alg_reg_cfg[i].anti_false_color_reg_cfg.dyna_reg_cfg;

        /* static */
        static_reg_cfg->resh       = TD_TRUE;
        static_reg_cfg->fcr_limit1 = OT_ISP_DEMOSAIC_FCR_LIMIT1_DEFAULT;
        static_reg_cfg->fcr_limit2 = OT_ISP_DEMOSAIC_FCR_LIMIT2_DEFAULT;

        /* dynamic */
        dyna_reg_cfg->resh              = TD_TRUE;
        dyna_reg_cfg->fcr_gain          = OT_ISP_DEMOSAIC_FCR_GAIN_DEFAULT;
        dyna_reg_cfg->fcr_ratio         = OT_ISP_DEMOSAIC_FCR_RATIO_DEFAULT;
        dyna_reg_cfg->fcr_gray_ratio    = OT_ISP_DEMOSAIC_FCR_GRAY_RATIO_DEFAULT;
        dyna_reg_cfg->fcr_cmax_sel      = OT_ISP_DEMOSAIC_FCR_CMAX_SEL_DEFAULT;
        dyna_reg_cfg->fcr_detg_sel      = OT_ISP_DEMOSAIC_FCR_DETG_SEL_DEFAULT;
        dyna_reg_cfg->fcr_hf_thresh_low = OT_ISP_DEMOSAIC_FCR_HF_THRESH_LOW_DEFAULT;
        dyna_reg_cfg->fcr_hf_thresh_hig = OT_ISP_DEMOSAIC_FCR_HF_THRESH_HIGH_DEFAULT;
    }

    reg_cfg->cfg_key.bit1_fcr_cfg = 1;
}

static td_void anti_false_color_read_extregs(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    isp_anti_false_color_attr_read(vi_pipe, &fcr_ctx->mpi_cfg);
    return;
}

static td_s32 isp_anti_false_color_ctx_initialize(ot_vi_pipe vi_pipe, isp_antifalsecolor *fcr_ctx)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_sensor_get_default(vi_pipe, &sns_dft);

    if (isp_ctx->sns_wdr_mode != 0) {
        anti_false_color_init_fw_wdr(vi_pipe, fcr_ctx);
    } else {
        anti_false_color_init_fw_linear(vi_pipe, fcr_ctx);
    }

    if (sns_dft->key.bit1_anti_false_color) {
        isp_check_pointer_return(sns_dft->anti_false_color);
        ret = isp_anti_false_color_attr_check("cmos", sns_dft->anti_false_color);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        (td_void)memcpy_s(&fcr_ctx->mpi_cfg, sizeof(ot_isp_anti_false_color_attr),
                          sns_dft->anti_false_color, sizeof(ot_isp_anti_false_color_attr));
    } else {
        fcr_ctx->mpi_cfg.enable  = OT_EXT_SYSTEM_ANTIFALSECOLOR_ENABLE_DEFAULT;
        fcr_ctx->mpi_cfg.op_type = OT_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_MODE_DEFAULT;
        fcr_ctx->mpi_cfg.manual_attr.threshold = OT_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_THRESHOLD_DEFAULT;
        fcr_ctx->mpi_cfg.manual_attr.strength  = OT_EXT_SYSTEM_ANTIFALSECOLOR_MANUAL_STRENGTH_DEFAULT;
        (td_void)memcpy_s(fcr_ctx->mpi_cfg.auto_attr.threshold, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                          g_afc_thd, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
        (td_void)memcpy_s(fcr_ctx->mpi_cfg.auto_attr.strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                          g_afc_str, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
    }

    return TD_SUCCESS;
}

static td_s32 isp_anti_false_color_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_antifalsecolor *fcr_ctx)
{
    td_s32 ret;
    fcr_ctx->init = TD_FALSE;
    ret = isp_anti_false_color_ctx_initialize(vi_pipe, fcr_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    anti_false_color_regs_initialize(vi_pipe, reg_cfg, fcr_ctx);
    anti_false_color_ext_regs_initialize(vi_pipe, fcr_ctx);
    fcr_ctx->init = TD_TRUE;
    ot_ext_system_isp_afc_init_status_write(vi_pipe, fcr_ctx->init);
    return TD_SUCCESS;
}

static td_s32 isp_anti_false_color_init(ot_vi_pipe vi_pipe, td_void *reg_cfg)
{
    td_s32 ret;
    isp_antifalsecolor *anti_false_color = TD_NULL;

    ot_ext_system_isp_afc_init_status_write(vi_pipe, TD_FALSE);
    ret = anti_false_color_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    antifalsecolor_get_ctx(vi_pipe, anti_false_color);
    isp_check_pointer_return(anti_false_color);

    return isp_anti_false_color_param_init(vi_pipe, (isp_reg_cfg *)reg_cfg, anti_false_color);
}

static __inline td_bool  check_anti_false_color_open(const isp_antifalsecolor *anti_false_color)
{
    return (anti_false_color->mpi_cfg.enable == TD_TRUE);
}

static td_void isp_anti_false_color_fw(isp_antifalsecolor_dyna_cfg *dyna_cfg, isp_antifalsecolor *afc_ctx)
{
    dyna_cfg->fcr_gray_ratio = afc_ctx->fcr_gray_ratio;
    dyna_cfg->fcr_cmax_sel   = afc_ctx->fcr_cmax_sel;
    dyna_cfg->fcr_detg_sel   = afc_ctx->fcr_detg_sel;
    dyna_cfg->fcr_hf_thresh_low = afc_ctx->fcr_hf_thresh_low;
    dyna_cfg->fcr_hf_thresh_hig = afc_ctx->fcr_hf_thresh_hig;

    dyna_cfg->fcr_ratio = afc_ctx->actual.threshold;
    dyna_cfg->fcr_gain  = afc_ctx->actual.strength;

    dyna_cfg->resh = TD_TRUE;
}

static td_void isp_anti_false_color_actual_calc(td_u32 iso, isp_antifalsecolor *afc_ctx)
{
    td_u8  iso_index_upper, iso_index_lower;
    td_u32 iso1, iso2;

    iso_index_upper = get_iso_index(iso);
    iso_index_lower = MAX2((td_s8)iso_index_upper - 1, 0);
    iso1 = get_iso(iso_index_lower);
    iso2 = get_iso(iso_index_upper);

    afc_ctx->fcr_gray_ratio = (td_u8)linear_inter(iso, iso1, afc_ctx->lut_afc_gray_ratio[iso_index_lower],
                                                  iso2, afc_ctx->lut_afc_gray_ratio[iso_index_upper]);
    afc_ctx->fcr_cmax_sel   = (td_u8)linear_inter(iso, iso1, afc_ctx->lut_afc_color_cmax_sel[iso_index_lower],
                                                  iso2, afc_ctx->lut_afc_color_cmax_sel[iso_index_upper]);
    afc_ctx->fcr_detg_sel   = (td_u8)linear_inter(iso, iso1, afc_ctx->lut_afc_detg_sel[iso_index_lower],
                                                  iso2, afc_ctx->lut_afc_detg_sel[iso_index_upper]);
    afc_ctx->fcr_hf_thresh_low = (td_u16)linear_inter(iso, iso1, afc_ctx->lut_afc_hf_thresh_low[iso_index_lower],
                                                      iso2, afc_ctx->lut_afc_hf_thresh_low[iso_index_upper]);
    afc_ctx->fcr_hf_thresh_hig = (td_u16)linear_inter(iso, iso1, afc_ctx->lut_afc_hf_thresh_hig[iso_index_lower],
                                                      iso2, afc_ctx->lut_afc_hf_thresh_hig[iso_index_upper]);

    if (afc_ctx->mpi_cfg.op_type == OT_OP_MODE_AUTO) {
        afc_ctx->actual.strength  = (td_u8)linear_inter(iso, iso1, afc_ctx->mpi_cfg.auto_attr.strength[iso_index_lower],
                                                        iso2, afc_ctx->mpi_cfg.auto_attr.strength[iso_index_upper]);
        afc_ctx->actual.threshold = (td_u8)linear_inter(iso,
                                                        iso1, afc_ctx->mpi_cfg.auto_attr.threshold[iso_index_lower],
                                                        iso2, afc_ctx->mpi_cfg.auto_attr.threshold[iso_index_upper]);
    } else {
        afc_ctx->actual.strength  = afc_ctx->mpi_cfg.manual_attr.strength;
        afc_ctx->actual.threshold = afc_ctx->mpi_cfg.manual_attr.threshold;
    }
}

static td_void isp_anti_false_color_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_antifalsecolor *afc_ctx)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_anti_false_color_actual_calc(isp_ctx->linkage.iso, afc_ctx);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        isp_anti_false_color_fw(&reg_cfg->alg_reg_cfg[i].anti_false_color_reg_cfg.dyna_reg_cfg, afc_ctx);
    }
}

static td_s32 isp_anti_false_color_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8 i;
    const td_u8 cnt_num = 2;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_ANTIFALSECOLOR;
    isp_reg_cfg *reg = (isp_reg_cfg *)reg_cfg;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_antifalsecolor *anti_false_color = TD_NULL;

    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    antifalsecolor_get_ctx(vi_pipe, anti_false_color);
    isp_check_pointer_return(anti_false_color);

    ot_ext_system_isp_afc_init_status_write(vi_pipe, anti_false_color->init);
    if (anti_false_color->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.fswdr_mode != isp_ctx->linkage.pre_fswdr_mode) {
        anti_false_color_set_long_frame_mode(vi_pipe, anti_false_color);
    }

    /* calculate every two interrupts */
    if ((isp_ctx->frame_cnt % cnt_num != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) {
        return TD_SUCCESS;
    }

    anti_false_color->mpi_cfg.enable = ot_ext_system_afc_enable_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].anti_false_color_reg_cfg.fcr_enable = anti_false_color->mpi_cfg.enable;
    }

    reg->cfg_key.bit1_fcr_cfg = 1;

    /* check hardware setting */
    if (!check_anti_false_color_open(anti_false_color)) {
        return TD_SUCCESS;
    }

    anti_false_color_read_extregs(vi_pipe, anti_false_color);

    isp_anti_false_color_reg_update(vi_pipe, reg_cfg, anti_false_color);

    return TD_SUCCESS;
}

static td_void anti_false_color_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc,
    const isp_antifalsecolor *afc_ctx)
{
    ot_isp_ctrl_proc_write proc_tmp;
    ot_unused(vi_pipe);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "anti false color info");

    isp_proc_printf(&proc_tmp, proc->write_len, "%12s" "%12s" "%12s\n", "en", "threshold", "strength");

    isp_proc_printf(&proc_tmp, proc->write_len, "%12u" "%12u" "%12u\n",
                    afc_ctx->mpi_cfg.enable, afc_ctx->actual.threshold, afc_ctx->actual.strength);

    proc->write_len += 1;
}

static td_s32 isp_anti_false_color_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr   *local_reg_cfg   = TD_NULL;
    isp_antifalsecolor *afc_ctx = TD_NULL;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, local_reg_cfg);
            isp_check_pointer_return(local_reg_cfg);

            isp_anti_false_color_init(vi_pipe, (td_void *)&local_reg_cfg->reg_cfg);
            break;

        case OT_ISP_PROC_WRITE:
            antifalsecolor_get_ctx(vi_pipe, afc_ctx);
            isp_check_pointer_return(afc_ctx);
            anti_false_color_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value, afc_ctx);
            break;

        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_anti_false_color_exit(ot_vi_pipe vi_pipe)
{
    td_u16 i;
    isp_reg_cfg_attr *local_reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, local_reg_cfg);
    ot_ext_system_isp_afc_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < local_reg_cfg->reg_cfg.cfg_num; i++) {
        local_reg_cfg->reg_cfg.alg_reg_cfg[i].anti_false_color_reg_cfg.fcr_enable = TD_FALSE;
    }

    local_reg_cfg->reg_cfg.cfg_key.bit1_fcr_cfg = 1;

    anti_false_color_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_fcr(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_fcr);

    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_ANTIFALSECOLOR;
    algs->alg_func.pfn_alg_init = isp_anti_false_color_init;
    algs->alg_func.pfn_alg_run  = isp_anti_false_color_run;
    algs->alg_func.pfn_alg_ctrl = isp_anti_false_color_ctrl;
    algs->alg_func.pfn_alg_exit = isp_anti_false_color_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
