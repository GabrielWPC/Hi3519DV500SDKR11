/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_ext_config.h"

typedef struct {
    td_bool enable;
} isp_dg;

isp_dg g_dg_ctx[OT_ISP_MAX_PIPE_NUM] = {{0}};
#define dg_get_ctx(pipe, ctx)   ctx = &g_dg_ctx[pipe]
#define DG_CHANNEL_NUM 4

static td_void be_dg_static_init(ot_vi_pipe vi_vipe, isp_dg_static_cfg *static_reg_cfg)
{
    ot_unused(vi_vipe);
    static_reg_cfg->resh = TD_TRUE;
}

static td_void be_dg_dyna_init(ot_vi_pipe vi_vipe, isp_dg_dyna_cfg *dyna_reg_cfg, td_u16 gain)
{
    ot_unused(vi_vipe);
    dyna_reg_cfg->clip_value = 0xFFFFF;
    dyna_reg_cfg->gain_r     = gain;
    dyna_reg_cfg->gain_gr    = gain;
    dyna_reg_cfg->gain_gb    = gain;
    dyna_reg_cfg->gain_b     = gain;
    dyna_reg_cfg->resh       = TD_TRUE;
}

static td_void wdr_dg_static_init(ot_vi_pipe vi_vipe, td_u8 i, isp_4dg_static_cfg *static_reg_cfg)
{
    ot_unused(vi_vipe);
    ot_unused(i);
    static_reg_cfg->gain_r0  = 0x100;
    static_reg_cfg->gain_gr0 = 0x100;
    static_reg_cfg->gain_gb0 = 0x100;
    static_reg_cfg->gain_b0  = 0x100;
    static_reg_cfg->gain_r1  = 0x100;
    static_reg_cfg->gain_gr1 = 0x100;
    static_reg_cfg->gain_gb1 = 0x100;
    static_reg_cfg->gain_b1  = 0x100;
    static_reg_cfg->gain_r2  = 0x100;
    static_reg_cfg->gain_gr2 = 0x100;
    static_reg_cfg->gain_gb2 = 0x100;
    static_reg_cfg->gain_b2  = 0x100;
    static_reg_cfg->gain_r3  = 0x100;
    static_reg_cfg->gain_gr3 = 0x100;
    static_reg_cfg->gain_gb3 = 0x100;
    static_reg_cfg->gain_b3  = 0x100;
    static_reg_cfg->resh     = TD_TRUE;
}

static td_void wdr_dg_dyna_init(ot_vi_pipe vi_vipe, isp_4dg_dyna_cfg *dyna_reg_cfg)
{
    ot_unused(vi_vipe);
    dyna_reg_cfg->clip_value0 = 0xFFFFF;
    dyna_reg_cfg->clip_value1 = 0xFFFFF;
    dyna_reg_cfg->clip_value2 = 0xFFFFF;
    dyna_reg_cfg->clip_value3 = 0xFFFFF;
    dyna_reg_cfg->resh = TD_TRUE;
}

static td_void fe_dg_dyna_init(ot_vi_pipe vi_vipe, isp_fe_dg_dyna_cfg *dyna_reg_cfg, td_u16 gain)
{
    td_u8 i;
    ot_unused(vi_vipe);

    for (i = 0; i < DG_CHANNEL_NUM; i++) {
        dyna_reg_cfg->gain_r[i]  = gain;
        dyna_reg_cfg->gain_gr[i] = gain;
        dyna_reg_cfg->gain_gb[i] = gain;
        dyna_reg_cfg->gain_b[i]  = gain;
    }
    dyna_reg_cfg->clip_value  = 0xFFFFF;
    dyna_reg_cfg->resh        = TD_TRUE;
}

static td_void wdr_dg_en_init(ot_vi_pipe vi_vipe, td_u8 i, isp_reg_cfg *reg_cfg)
{
    td_u8 wdr_mode;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_vipe, isp_ctx);

    wdr_mode = isp_ctx->sns_wdr_mode;

    if (is_linear_mode(wdr_mode)) {
        reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.enable = TD_TRUE;
    } else if (is_2to1_wdr_mode(wdr_mode) || is_3to1_wdr_mode(wdr_mode) || is_4to1_wdr_mode(wdr_mode)) {
        reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.enable = TD_TRUE;
    } else if (is_built_in_wdr_mode(wdr_mode)) {
        reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.enable = TD_FALSE;
    }
}

static td_void dg_regs_initialize(ot_vi_pipe vi_vipe, isp_reg_cfg *reg_cfg)
{
    td_u8  i;
    td_u16 gain;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;

    isp_sensor_get_blc(vi_vipe, &sns_black_level);

    gain = clip3(0xFFF * 0x100 / div_0_to_1(0xFFF - sns_black_level->auto_attr.black_level[0][1]) + 1, 0x100, 0x200);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        be_dg_static_init(vi_vipe, &reg_cfg->alg_reg_cfg[i].dg_reg_cfg.static_reg_cfg);
        be_dg_dyna_init(vi_vipe, &reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg, gain);

        wdr_dg_static_init(vi_vipe, i, &reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.static_reg_cfg);
        wdr_dg_dyna_init(vi_vipe, &reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.dyna_reg_cfg);

        wdr_dg_en_init(vi_vipe, i, reg_cfg);

        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dg_en = TD_TRUE;
        reg_cfg->cfg_key.bit1_dg_cfg = 1;
        reg_cfg->cfg_key.bit1_wdr_dg_cfg = 1;
    }

    fe_dg_dyna_init(vi_vipe, &reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg, gain);
    reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dg_en = TD_TRUE;
    reg_cfg->cfg_key.bit1_fe_dg_cfg                 = 1;
}

static td_void dg_ext_regs_initialize(ot_vi_pipe vi_vipe)
{
    ot_ext_system_isp_dgain_enable_write(vi_vipe, TD_TRUE);
}

static td_void dg_initialize(ot_vi_pipe vi_vipe)
{
    isp_dg *dg = TD_NULL;

    dg_get_ctx(vi_vipe, dg);

    dg->enable = TD_TRUE;
}

static td_void isp_dg_wdr_mode_set(ot_vi_pipe vi_vipe, td_void *reg_cfg_input)
{
    td_u8  i, j;
    td_u16 gain;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_input;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;

    isp_sensor_get_blc(vi_vipe, &sns_black_level);

    gain = clip3(0xFFF * 0x100 / div_0_to_1(0xFFF - sns_black_level->auto_attr.black_level[0][1]) + 1, 0x100, 0x200);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        wdr_dg_en_init(vi_vipe, i, reg_cfg);
        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.static_reg_cfg.resh       = TD_TRUE;
        reg_cfg->alg_reg_cfg[i].four_dg_reg_cfg.static_reg_cfg.resh  = TD_TRUE;

        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg.gain_r  = gain;
        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg.gain_gr = gain;
        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg.gain_gb = gain;
        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg.gain_b  = gain;
    }

    reg_cfg->cfg_key.bit1_wdr_dg_cfg = 1;
    reg_cfg->cfg_key.bit1_fe_dg_cfg  = 1;
    reg_cfg->cfg_key.bit1_dg_cfg     = 1;

    for (j = 0; j < DG_CHANNEL_NUM; j++) {
        reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg.gain_r[j]  = gain;
        reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg.gain_gr[j] = gain;
        reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg.gain_gb[j] = gain;
        reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg.gain_b[j]  = gain;
    }
    reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
}

static td_s32 isp_dg_init(ot_vi_pipe vi_vipe, td_void *reg_cfg_input)
{
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_input;

    dg_regs_initialize(vi_vipe, reg_cfg);
    dg_ext_regs_initialize(vi_vipe);
    dg_initialize(vi_vipe);

    return TD_SUCCESS;
}

static __inline td_bool  check_dg_open(const isp_dg *dg)
{
    return (dg->enable == TD_TRUE);
}

static td_s32 isp_dg_run(ot_vi_pipe vi_vipe, const td_void *stat_info,
    td_void *reg_cfg_input, td_s32 rsv)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_dg *dg = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_input;
    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_vipe, isp_ctx);
    dg_get_ctx(vi_vipe, dg);

    if (isp_ctx->linkage.defect_pixel &&
        (ot_ext_system_dpc_static_defect_type_read(vi_vipe) == 0)) {
        for (i = 0; i < reg_cfg->cfg_num; i++) {
            reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dg_en = TD_FALSE;
        }

        reg_cfg->cfg_key.bit1_dg_cfg   = 1;

        return TD_SUCCESS;
    }

    dg->enable = ot_ext_system_isp_dgain_enable_read(vi_vipe);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].dg_reg_cfg.dg_en = dg->enable;
    }

    reg_cfg->alg_reg_cfg[0].fe_dg_reg_cfg.dg_en = dg->enable;

    reg_cfg->cfg_key.bit1_fe_dg_cfg = 1;
    reg_cfg->cfg_key.bit1_dg_cfg   = 1;
    reg_cfg->cfg_key.bit1_wdr_dg_cfg = 1;

    /* check hardware setting */
    if (!check_dg_open(dg)) {
        return TD_SUCCESS;
    }

    return TD_SUCCESS;
}

static td_s32 isp_dg_ctrl(ot_vi_pipe vi_vipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    ot_unused(value);

    switch (cmd) {
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_vipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_dg_wdr_mode_set(vi_vipe, (td_void *)&reg_cfg->reg_cfg);
            break;

        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_dg_exit(ot_vi_pipe vi_vipe)
{
    ot_unused(vi_vipe);
    return TD_SUCCESS;
}

td_s32 isp_alg_register_dg(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_dg);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_DG;
    algs->alg_func.pfn_alg_init = isp_dg_init;
    algs->alg_func.pfn_alg_run  = isp_dg_run;
    algs->alg_func.pfn_alg_ctrl = isp_dg_ctrl;
    algs->alg_func.pfn_alg_exit = isp_dg_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
