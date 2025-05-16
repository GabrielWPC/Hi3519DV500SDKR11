/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_config.h"
#include "isp_alg.h"

#define MCDS_FILTER_MODE    1    /* 1: filter mode; 0: discard mode */

static td_void mcds_static_reg_init(ot_vi_pipe vi_pipe, isp_mcds_static_reg_cfg *static_reg_cfg,
    const isp_usr_ctx *isp_ctx)
{
    static const td_s16 coeff_filter_8tap_h[2][8] = { /* table size 2*8 */
        { -16, 0, 145, 254, 145, 0, -16, 0 },          /* 1st -16, 0, 145, 254, 145, 0, -16, 0 */
        {  0, 0,   0, 256, 256, 0,   0, 0 }            /* 2nd   0, 0,   0, 256, 256, 0,   0, 0 */
    };
    static const td_s16 coeff_discard_8pixel_h[8] = {0, 0, 0, 512, 0, 0, 0, 0};   /* table 8  4th 512  */
    static const td_s8 coeff_filer_4tap_v[2][4]   = {{4, 4, 6, 6}, {3, 3, 3, 3}}; /* table 2*4 4, 6, 3 */
    static const td_s8 coeff_discard_4tap_v[4]    = {5, 6, 6, 6};                 /* table   4 5, 6,   */

    ot_unused(vi_pipe);
    ot_unused(isp_ctx);
    if (MCDS_FILTER_MODE) { /* filter mode */
        (td_void)memcpy_s(static_reg_cfg->h_coef, 8 * sizeof(td_s16),  /* table 8 */
                          coeff_filter_8tap_h[0], 8 * sizeof(td_s16)); /* table 8 */
        (td_void)memcpy_s(static_reg_cfg->v_coef, 2 * sizeof(td_s8),    /* table 2 */
                          coeff_filer_4tap_v[0],   2 * sizeof(td_s8));  /* table 2 */
    } else {    /* discard mode */
        (td_void)memcpy_s(static_reg_cfg->h_coef, 8 * sizeof(td_s16),  /* table 8 */
                          coeff_discard_8pixel_h, 8 * sizeof(td_s16)); /* table 8 */
        (td_void)memcpy_s(static_reg_cfg->v_coef, 2 * sizeof(td_s8),  /* table 2 */
                          coeff_discard_4tap_v,    2 * sizeof(td_s8));  /* table 2 */
    }

    static_reg_cfg->coring_limit = 0;
    static_reg_cfg->midf_bldr    = 0x8;
    static_reg_cfg->static_resh  = TD_TRUE;
}

static td_void mcds_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u32 i;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].mcds_reg_cfg.hcds_en = TD_TRUE;
        mcds_static_reg_init(vi_pipe, &(reg_cfg->alg_reg_cfg[i].mcds_reg_cfg.static_reg_cfg), isp_ctx);
    }

    reg_cfg->cfg_key.bit1_mcds_cfg = 1;

    return;
}

static td_s32 isp_mcds_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_usr)
{
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_usr;

    mcds_regs_initialize(vi_pipe, reg_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_mcds_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_usr, td_s32 rsv)
{
    td_u8   i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_usr;

    ot_unused(stat_info);
    ot_unused(rsv);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].mcds_reg_cfg.hcds_en = TD_TRUE;
    }

    reg_cfg->cfg_key.bit1_mcds_cfg = 1;

    return TD_SUCCESS;
}

static td_s32 isp_mcds_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_mcds_init(vi_pipe, &reg_cfg->reg_cfg);
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

static td_s32 isp_mcds_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].mcds_reg_cfg.hcds_en = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_mcds_cfg = 1;
    return TD_SUCCESS;
}

td_s32 isp_alg_register_mcds(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_mcds);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_MCDS;
    algs->alg_func.pfn_alg_init = isp_mcds_init;
    algs->alg_func.pfn_alg_run  = isp_mcds_run;
    algs->alg_func.pfn_alg_ctrl = isp_mcds_ctrl;
    algs->alg_func.pfn_alg_exit = isp_mcds_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
