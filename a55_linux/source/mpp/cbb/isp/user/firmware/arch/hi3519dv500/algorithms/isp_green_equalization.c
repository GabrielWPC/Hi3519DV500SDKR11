/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <math.h>
#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_sensor.h"
#include "isp_math_utils.h"
#include "isp_proc.h"
#include "isp_param_check.h"

#define OT_MINIISP_BITDEPTH        14
#define ISP_GE_SLOPE_DEFAULT       9
#define ISP_GE_SENSI_SLOPE_DEFAULT 9
#define ISP_GE_SENSI_THR_DEFAULT   300
#define ISP_GE_MV_BIT              0 /* user param is 12 bit, logic param is 12 bit */

static const  td_u16 g_threshold[OT_ISP_AUTO_ISO_NUM] = {
    300,  300,  300,  300,  310,  310,  310,  310,  320,  320,  320,  320,  330,  330,  330,  330
};
static const  td_u16 g_strength[OT_ISP_AUTO_ISO_NUM] = {
    128,  128,  128,  128,  129,  129,  129,  129,  130,  130,  130,  130,  131,  131,  131,  131
};
static const  td_u8 g_filter_mode[OT_ISP_AUTO_ISO_NUM] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};
static const  td_u16 g_np_offset[OT_ISP_AUTO_ISO_NUM] = {
    1024, 1024, 1024, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048
};

typedef struct {
    td_bool  enable;
    td_u8    slope;
    td_u8    sensi_slope;
    td_u16   sensi_threshold;
    td_u16   strength;
    td_u16   threshold;
    td_u8    filter_mode;
    td_u16   np_offset;
} ot_isp_cr_actual_attr;

typedef struct {
    td_bool init;
    td_bool enable;
    td_bool coef_update_en;
    td_u8   ge_chn_num;

    td_u16  threshold;  /* Range: [0, 0x3FFF] */
    td_u16  strength;   /* Range: [0, 0x100]  */
    td_u16  np_offset;  /* Range: [0x200, 0x3FFF] */

    td_u32  bit_depth;
    ot_isp_cr_attr cmos_ge;
    ot_isp_cr_actual_attr cmos_ge_actual;
} isp_ge;

isp_ge *g_ge_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define ge_get_ctx(pipe, ctx)   ((ctx) = g_ge_ctx[pipe])
#define ge_set_ctx(pipe, ctx)   (g_ge_ctx[pipe] = (ctx))
#define ge_reset_ctx(pipe)         (g_ge_ctx[pipe] = TD_NULL)

static td_s32 ge_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_ge *ge_ctx = TD_NULL;

    ge_get_ctx(vi_pipe, ge_ctx);

    if (ge_ctx == TD_NULL) {
        ge_ctx = (isp_ge *)isp_malloc(sizeof(isp_ge));
        if (ge_ctx == TD_NULL) {
            isp_err_trace("isp[%d] ge_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(ge_ctx, sizeof(isp_ge), 0, sizeof(isp_ge));

    ge_set_ctx(vi_pipe, ge_ctx);

    return TD_SUCCESS;
}

static td_void ge_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_ge *ge_ctx = TD_NULL;

    ge_get_ctx(vi_pipe, ge_ctx);
    isp_free(ge_ctx);
    ge_reset_ctx(vi_pipe);
}

static td_s32 ge_initialize(ot_vi_pipe vi_pipe, isp_ge *ge)
{
    td_s32 ret;
    ot_isp_cmos_default  *sns_dft = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    if (sns_dft->key.bit1_ge) {
        isp_check_pointer_return(sns_dft->ge);

        ret = isp_crosstalk_attr_check("cmos", sns_dft->ge);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        (td_void)memcpy_s(&ge->cmos_ge, sizeof(ot_isp_cr_attr), sns_dft->ge, sizeof(ot_isp_cr_attr));
    } else {
        ge->cmos_ge.enable      = TD_TRUE;
        ge->cmos_ge.slope       = ISP_GE_SLOPE_DEFAULT;
        ge->cmos_ge.sensi_slope = ISP_GE_SENSI_SLOPE_DEFAULT;
        ge->cmos_ge.sensi_threshold = ISP_GE_SENSI_THR_DEFAULT;
        (td_void)memcpy_s(ge->cmos_ge.strength, OT_ISP_AUTO_ISO_NUM * sizeof(td_u16),
                          g_strength,  OT_ISP_AUTO_ISO_NUM * sizeof(td_u16));
        (td_void)memcpy_s(ge->cmos_ge.threshold, OT_ISP_AUTO_ISO_NUM * sizeof(td_u16),
                          g_threshold, OT_ISP_AUTO_ISO_NUM * sizeof(td_u16));
        (td_void)memcpy_s(ge->cmos_ge.filter_mode, OT_ISP_AUTO_ISO_NUM * sizeof(td_u8),
                          g_filter_mode,  OT_ISP_AUTO_ISO_NUM * sizeof(td_u8));
        (td_void)memcpy_s(ge->cmos_ge.np_offset, OT_ISP_AUTO_ISO_NUM * sizeof(td_u16),
                          g_np_offset,  OT_ISP_AUTO_ISO_NUM * sizeof(td_u16));
    }

    ge->enable    = ge->cmos_ge.enable;
    ge->bit_depth = OT_MINIISP_BITDEPTH;

    return TD_SUCCESS;
}

static td_void ge_ext_regs_initialize(ot_vi_pipe vi_pipe, const isp_ge *ge)
{
    td_u32 i;

    /* initial ext register of ge */
    ot_ext_system_ge_enable_write(vi_pipe, ge->enable);
    ot_ext_system_ge_slope_write(vi_pipe, ge->cmos_ge.slope);
    ot_ext_system_ge_sensitivity_write(vi_pipe, ge->cmos_ge.sensi_slope);
    ot_ext_system_ge_sensithreshold_write(vi_pipe, ge->cmos_ge.sensi_threshold);

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_ge_strength_write(vi_pipe, i, ge->cmos_ge.strength[i]);
        ot_ext_system_ge_npoffset_write(vi_pipe, i, ge->cmos_ge.np_offset[i]);
        ot_ext_system_ge_filter_mode_write(vi_pipe, i, ge->cmos_ge.filter_mode[i]);
        ot_ext_system_ge_threshold_write(vi_pipe, i, ge->cmos_ge.threshold[i]);
    }

    ot_ext_system_ge_coef_update_en_write(vi_pipe, TD_TRUE);
}

static td_u8 ge_get_chn_num(td_u8 wdr_mode)
{
    td_u8 chn_num;

    if (is_linear_mode(wdr_mode)) {
        chn_num = 0x1;
    } else if (is_built_in_wdr_mode(wdr_mode)) {
        chn_num = 0x1;
    } else if (is_2to1_wdr_mode(wdr_mode)) {
        chn_num = 0x2;
    } else if (is_3to1_wdr_mode(wdr_mode)) {
        chn_num = 0x3;
    } else {
        /* unknown mode */
        chn_num = 0x1;
    }

    return chn_num;
}

static td_void ge_static_regs_initialize(ot_vi_pipe vi_pipe, isp_ge_static_cfg *ge_static_reg_cfg)
{
    ot_unused(vi_pipe);
    ge_static_reg_cfg->ge_gr_gb_en = TD_TRUE;
    ge_static_reg_cfg->ge_gr_en    = TD_FALSE;
    ge_static_reg_cfg->ge_gb_en    = TD_FALSE;
    ge_static_reg_cfg->static_resh = TD_TRUE;
}

static td_void ge_usr_regs_initialize(ot_vi_pipe vi_pipe, td_u8 i, td_u8 chn_num, isp_ge_usr_cfg *ge_usr_reg_cfg,
                                      const isp_usr_ctx *isp_ctx)
{
    td_u8 j;
    ot_unused(vi_pipe);
    ot_unused(i);
    ot_unused(isp_ctx);

    for (j = 0; j < chn_num; j++) {
        ge_usr_reg_cfg->ge_ct_th2[j]    = OT_ISP_GE_SENSITHR_DEFAULT << ISP_GE_MV_BIT;
        ge_usr_reg_cfg->ge_ct_slope1[j] = OT_ISP_GE_SENSISLOPE_DEFAULT + ISP_GE_MV_BIT;
        ge_usr_reg_cfg->ge_ct_slope2[j] = OT_ISP_GE_SLOPE_DEFAULT + ISP_GE_MV_BIT;
    }

    ge_usr_reg_cfg->resh = TD_TRUE;
}

static td_void ge_dyna_regs_initialize(td_u8 chn_num, isp_ge_dyna_cfg *ge_dyna_reg_cfg)
{
    td_u8 i;

    for (i = 0; i < chn_num; i++) {
        ge_dyna_reg_cfg->ge_ct_th1[i] = OT_ISP_GE_NPOFFSET_DEFAULT << ISP_GE_MV_BIT;
        ge_dyna_reg_cfg->ge_ct_th3[i] = OT_ISP_GE_THRESHOLD_DEFAULT << ISP_GE_MV_BIT;
    }
    ge_dyna_reg_cfg->ge_strength = OT_ISP_GE_STRENGTH_DEFAULT;
    ge_dyna_reg_cfg->resh        = TD_TRUE;
}

static td_void ge_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_ge *ge)
{
    td_u8 i, j;
    isp_usr_ctx  *isp_ctx = TD_NULL;
    td_u8 ge_chn_num;

    isp_get_ctx(vi_pipe, isp_ctx);

    ge_chn_num = ge_get_chn_num(isp_ctx->sns_wdr_mode);
    ge->ge_chn_num = ge_chn_num;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        ge_static_regs_initialize(vi_pipe, &reg_cfg->alg_reg_cfg[i].ge_reg_cfg.static_reg_cfg);
        ge_usr_regs_initialize(vi_pipe, i, ge_chn_num, &reg_cfg->alg_reg_cfg[i].ge_reg_cfg.usr_reg_cfg, isp_ctx);
        ge_dyna_regs_initialize(ge_chn_num, &reg_cfg->alg_reg_cfg[i].ge_reg_cfg.dyna_reg_cfg);
        reg_cfg->alg_reg_cfg[i].ge_reg_cfg.chn_num = ge_chn_num;

        for (j = 0; j < ge_chn_num; j++) {
            reg_cfg->alg_reg_cfg[i].ge_reg_cfg.ge_en[j] = ge->enable;
        }

        for (j = ge_chn_num; j < 0x4; j++) {
            reg_cfg->alg_reg_cfg[i].ge_reg_cfg.ge_en[j] = TD_FALSE;
        }
    }

    reg_cfg->cfg_key.bit1_ge_cfg = 1;
}

static td_void ge_read_extregs(ot_vi_pipe vi_pipe)
{
    td_u32 i;
    isp_ge *ge = TD_NULL;

    ge_get_ctx(vi_pipe, ge);

    /* read ext register of ge */
    ge->coef_update_en         = ot_ext_system_ge_coef_update_en_read(vi_pipe);
    if (ge->coef_update_en) {
        ot_ext_system_ge_coef_update_en_write(vi_pipe, TD_FALSE);
        ge->cmos_ge.slope       = ot_ext_system_ge_slope_read(vi_pipe);
        ge->cmos_ge.sensi_slope = ot_ext_system_ge_sensitivity_read(vi_pipe);
        ge->cmos_ge.sensi_threshold = ot_ext_system_ge_sensithreshold_read(vi_pipe);

        for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
            ge->cmos_ge.strength[i]  = ot_ext_system_ge_strength_read(vi_pipe, i);
            ge->cmos_ge.np_offset[i]  = ot_ext_system_ge_npoffset_read(vi_pipe, i);
            ge->cmos_ge.filter_mode[i]  = ot_ext_system_ge_filter_mode_read(vi_pipe, i);
            ge->cmos_ge.threshold[i] = ot_ext_system_ge_threshold_read(vi_pipe, i);
        }
    }
}

static td_u8 ge_slop_convert(td_u8 slope)
{
    if (slope == 0) {
        return slope;
    } else {
        return clip_max(slope + ISP_GE_MV_BIT, OT_MINIISP_BITDEPTH);
    }
}

static td_void ge_usr_fw(const isp_ge *ge, isp_ge_usr_cfg *ge_usr_reg_cfg)
{
    td_u8 j;

    for (j = 0; j < ge->ge_chn_num; j++) {
        ge_usr_reg_cfg->ge_ct_th2[j] = clip_max(ge->cmos_ge.sensi_threshold << ISP_GE_MV_BIT, (1 << ge->bit_depth) - 1);
        ge_usr_reg_cfg->ge_ct_slope1[j] = ge_slop_convert(ge->cmos_ge.sensi_slope);
        ge_usr_reg_cfg->ge_ct_slope2[j] = ge_slop_convert(ge->cmos_ge.slope);
    }

    ge_usr_reg_cfg->resh = TD_TRUE;
}

static td_void ge_dyna_fw(td_u32 iso, isp_ge *ge, isp_ge_dyna_cfg *ge_dyna_reg_cfg)
{
    td_u8 i;
    td_u8 index_upper = get_iso_index(iso);
    td_u8 index_lower = MAX2((td_s8)index_upper - 1, 0);
    td_u32 iso_low  = get_iso(index_lower);
    td_u32 iso_high = get_iso(index_upper);

    for (i = 0; i < ge->ge_chn_num; i++) {
        ge_dyna_reg_cfg->ge_ct_th3[i] = clip_max((td_u16)linear_inter(iso,
            iso_low, ge->cmos_ge.threshold[index_lower],
            iso_high, ge->cmos_ge.threshold[index_upper]) << ISP_GE_MV_BIT, (td_u16)(1 << ge->bit_depth) - 1);

        ge_dyna_reg_cfg->ge_ct_th1[i] = clip_max((td_u32)linear_inter(iso,
            iso_low, ge->cmos_ge.np_offset[index_lower],
            iso_high, ge->cmos_ge.np_offset[index_upper]) << ISP_GE_MV_BIT, (td_u32)(1 << ge->bit_depth) - 1);
    }
    ge_dyna_reg_cfg->ge_strength  = (td_u16)linear_inter(iso, iso_low, ge->cmos_ge.strength[index_lower],
        iso_high, ge->cmos_ge.strength[index_upper]);
    ge->threshold  = ge_dyna_reg_cfg->ge_ct_th3[0] >> ISP_GE_MV_BIT;
    ge->np_offset  = ge_dyna_reg_cfg->ge_ct_th1[0] >> ISP_GE_MV_BIT;
    ge->strength   = ge_dyna_reg_cfg->ge_strength;
    ge_dyna_reg_cfg->resh = TD_TRUE;
}

static td_void ge_actual_dyna_fw(td_u32 iso, isp_ge *ge)
{
    td_u8 index_upper = get_iso_index(iso);
    td_u8 index_lower = MAX2((td_s8)index_upper - 1, 0);
    td_u32 iso_low  = get_iso(index_lower);
    td_u32 iso_high = get_iso(index_upper);

    ge->cmos_ge_actual.enable = ge->cmos_ge.enable;
    ge->cmos_ge_actual.slope = ge->cmos_ge.slope;
    ge->cmos_ge_actual.sensi_slope = ge->cmos_ge.sensi_slope;
    ge->cmos_ge_actual.sensi_threshold = ge->cmos_ge.sensi_threshold;

    ge->cmos_ge_actual.strength   = (td_u16)linear_inter(iso, iso_low, ge->cmos_ge.strength[index_lower],
                                                         iso_high, ge->cmos_ge.strength[index_upper]);
    ge->cmos_ge_actual.threshold   = (td_u16)linear_inter(iso, iso_low, ge->cmos_ge.threshold[index_lower],
                                                          iso_high, ge->cmos_ge.threshold[index_upper]);
    ge->cmos_ge_actual.filter_mode   = (td_u8)linear_inter(iso, iso_low, ge->cmos_ge.filter_mode[index_lower],
                                                           iso_high, ge->cmos_ge.filter_mode[index_upper]);
}

__inline static td_bool check_ge_open(const isp_ge *ge)
{
    return (ge->enable == TD_TRUE);
}

static td_void ge_bypass(isp_reg_cfg *reg, const isp_ge *ge)
{
    td_u8 i, j;

    for (i = 0; i < reg->cfg_num; i++) {
        for (j = 0; j < ge->ge_chn_num; j++) {
            reg->alg_reg_cfg[i].ge_reg_cfg.ge_en[j] = TD_FALSE;
        }
    }

    reg->cfg_key.bit1_ge_cfg = 1;
}

static td_s32 isp_ge_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_in)
{
    td_s32 ret;
    isp_ge  *ge_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_in;

    ot_ext_system_isp_ge_init_status_write(vi_pipe, TD_FALSE);
    ret = ge_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ge_get_ctx(vi_pipe, ge_ctx);
    isp_check_pointer_return(ge_ctx);
    ge_ctx->init = TD_FALSE;

    ret = ge_initialize(vi_pipe, ge_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ge_regs_initialize(vi_pipe, reg_cfg, ge_ctx);
    ge_ext_regs_initialize(vi_pipe, ge_ctx);
    ge_ctx->init = TD_TRUE;
    ot_ext_system_isp_ge_init_status_write(vi_pipe, ge_ctx->init);
    return TD_SUCCESS;
}

static td_s32 isp_ge_run(ot_vi_pipe vi_pipe, const td_void *stat_info,
    td_void *reg_cfg, td_s32 rsv)
{
    td_u8 i, j;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_GE;
    isp_ge  *ge = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg *reg = (isp_reg_cfg *)reg_cfg;

    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    ge_get_ctx(vi_pipe, ge);
    isp_check_pointer_return(ge);

    ot_ext_system_isp_ge_init_status_write(vi_pipe, ge->init);
    if (ge->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    if ((isp_ctx->frame_cnt % 0x2 != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) {
        return TD_SUCCESS;
    }

    if (isp_ctx->linkage.defect_pixel) {
        ge_bypass(reg, ge);
        return TD_SUCCESS;
    }

    ge->enable = ot_ext_system_ge_enable_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        for (j = 0; j < ge->ge_chn_num; j++) {
            reg->alg_reg_cfg[i].ge_reg_cfg.ge_en[j] = ge->enable;
        }

        for (j = ge->ge_chn_num; j < 0x4; j++) {
            reg->alg_reg_cfg[i].ge_reg_cfg.ge_en[j] = TD_FALSE;
        }
    }

    reg->cfg_key.bit1_ge_cfg = 1;

    /* check hardware setting */
    if (!check_ge_open(ge)) {
        return TD_SUCCESS;
    }

    ge_read_extregs(vi_pipe);
    ge_actual_dyna_fw(isp_ctx->linkage.iso, ge);

    if (ge->coef_update_en) {
        for (i = 0; i < reg->cfg_num; i++) {
            ge_usr_fw(ge, &reg->alg_reg_cfg[i].ge_reg_cfg.usr_reg_cfg);
        }
    }

    for (i = 0; i < reg->cfg_num; i++) {
        ge_dyna_fw(isp_ctx->linkage.iso, ge, &reg->alg_reg_cfg[i].ge_reg_cfg.dyna_reg_cfg);
    }

    return TD_SUCCESS;
}

static td_void ge_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_ge  *ge_ctx = TD_NULL;

    ge_get_ctx(vi_pipe, ge_ctx);
    isp_check_pointer_void_return(ge_ctx);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "crosstalk info");
    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s\n",
                    "enable", "slope", "sensi_slope", "threshold", "sensi_th", "strength", "filter_md");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%12u" "%12u" "%12u" "%12u" "%12u" "%12u" "%12u\n",
                    ge_ctx->enable, ge_ctx->cmos_ge_actual.slope, ge_ctx->cmos_ge_actual.sensi_slope,
                    ge_ctx->cmos_ge_actual.threshold, ge_ctx->cmos_ge_actual.sensi_threshold,
                    ge_ctx->cmos_ge_actual.strength, ge_ctx->cmos_ge_actual.filter_mode);

    proc->write_len += 1;
}

static td_s32 isp_ge_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_ge_init(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;

        case OT_ISP_PROC_WRITE:
            ge_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

static td_s32 isp_ge_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    ot_ext_system_isp_ge_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].ge_reg_cfg.ge_en[0] = TD_FALSE; /* [0] */
        reg_cfg->reg_cfg.alg_reg_cfg[i].ge_reg_cfg.ge_en[1] = TD_FALSE; /* [1] */
        reg_cfg->reg_cfg.alg_reg_cfg[i].ge_reg_cfg.ge_en[2] = TD_FALSE; /* [2] */
        reg_cfg->reg_cfg.alg_reg_cfg[i].ge_reg_cfg.ge_en[3] = TD_FALSE; /* [3] */
    }

    reg_cfg->reg_cfg.cfg_key.bit1_ge_cfg = 1;

    ge_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_ge(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_ge);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_GE;
    algs->alg_func.pfn_alg_init = isp_ge_init;
    algs->alg_func.pfn_alg_run  = isp_ge_run;
    algs->alg_func.pfn_alg_ctrl = isp_ge_ctrl;
    algs->alg_func.pfn_alg_exit = isp_ge_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
