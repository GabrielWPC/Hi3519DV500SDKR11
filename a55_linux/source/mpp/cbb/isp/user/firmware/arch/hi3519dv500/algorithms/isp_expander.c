/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_ext_config.h"
#include "isp_ext_reg_access.h"
#include "isp_math_utils.h"
#include "isp_param_check.h"
#include "isp_sensor.h"

typedef struct {
    td_bool init;
    td_bool param_update;
    ot_isp_expander_attr mpi_cfg;
} isp_expander_ctx;

isp_expander_ctx *g_expander_ctx[OT_ISP_MAX_PIPE_NUM] = { TD_NULL };

#define expander_get_ctx(vi_pipe, ctx)   ((ctx) = g_expander_ctx[vi_pipe])
#define expander_set_ctx(vi_pipe, ctx)   (g_expander_ctx[vi_pipe] = (ctx))
#define expander_reset_ctx(vi_pipe)      (g_expander_ctx[vi_pipe] = TD_NULL)

static td_s32 expander_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_expander_ctx *expander_ctx = TD_NULL;

    expander_get_ctx(vi_pipe, expander_ctx);

    if (expander_ctx == TD_NULL) {
        expander_ctx = (isp_expander_ctx *)isp_malloc(sizeof(isp_expander_ctx));
        if (expander_ctx == TD_NULL) {
            isp_err_trace("isp[%d] expander_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(expander_ctx, sizeof(isp_expander_ctx), 0, sizeof(isp_expander_ctx));

    expander_set_ctx(vi_pipe, expander_ctx);

    return TD_SUCCESS;
}

static td_void expander_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_expander_ctx *expander_ctx = TD_NULL;

    expander_get_ctx(vi_pipe, expander_ctx);
    isp_free(expander_ctx);
    expander_reset_ctx(vi_pipe);
}

static td_u32 get_index(td_u32 x, const td_u16 *x_lut, td_u16 point_num, td_u16 x_lut_length)
{
    td_u32 index;
    ot_unused(x_lut_length);

    for (index = 0; index < point_num; index++) {
        if (x <= x_lut[index]) {
            break;
        }
    }

    return index;
}

static td_void isp_expander_initialize_def(isp_expander_ctx *expander_ctx)
{
    expander_ctx->mpi_cfg.enable         = TD_FALSE;
    expander_ctx->mpi_cfg.bit_depth_in   = 12;  /* bit depth in  12 */
    expander_ctx->mpi_cfg.bit_depth_out  = 16;  /* bit depth out 16 */
    expander_ctx->mpi_cfg.knee_point_num = 1;
    expander_ctx->mpi_cfg.knee_point_coord[0].x = 0x100;
    expander_ctx->mpi_cfg.knee_point_coord[0].y = 0x100000;
}

static td_void isp_expander_reg_calc(isp_expander_usr_cfg *usr_reg_cfg, const ot_isp_expander_attr *expander_attr)
{
    td_u32 i, idx_high, idx_low;
    td_u16 expander_point_num;
    td_u16 x_lut[OT_ISP_EXPANDER_POINT_NUM_MAX + 1] = { 0 };
    td_u32 y_lut[OT_ISP_EXPANDER_POINT_NUM_MAX + 1] = { 0 };

    usr_reg_cfg->bit_depth_in  = expander_attr->bit_depth_in;
    usr_reg_cfg->bit_depth_out = expander_attr->bit_depth_out;
    expander_point_num         = expander_attr->knee_point_num;

    for (i = 1; i < (td_u32)(expander_point_num + 1); i++) {
        x_lut[i] = expander_attr->knee_point_coord[i - 1].x;
        y_lut[i] = expander_attr->knee_point_coord[i - 1].y;
    }

    for (i = 0; i < OT_ISP_EXPANDER_NODE_NUM; i++) {
        idx_high = get_index(i, x_lut, expander_point_num, OT_ISP_EXPANDER_POINT_NUM_MAX + 1);
        idx_low = MAX2((td_s32)idx_high - 1, 0);
        usr_reg_cfg->lut[i] =
            (td_u32)linear_inter(i, x_lut[idx_low], y_lut[idx_low], x_lut[idx_high], y_lut[idx_high]);
    }

    usr_reg_cfg->resh = TD_TRUE;
}

static td_void isp_expander_reg_update(isp_reg_cfg *reg_cfg, const isp_expander_ctx *expander_ctx)
{
    td_u8 i;
    if (expander_ctx->param_update != TD_TRUE) {
        return;
    }

    isp_expander_reg_calc(&reg_cfg->alg_reg_cfg[0].expander_cfg.usr_cfg, &expander_ctx->mpi_cfg);
    reg_cfg->alg_reg_cfg[0].expander_cfg.usr_cfg.update_index += 1;
    for (i = 1; i < reg_cfg->cfg_num; i++) {
        (td_void)memcpy_s(&reg_cfg->alg_reg_cfg[i].expander_cfg, sizeof(isp_expander_reg_cfg),
                          &reg_cfg->alg_reg_cfg[0].expander_cfg, sizeof(isp_expander_reg_cfg));
    }
}

static td_void expander_regs_initialize(isp_reg_cfg *reg_cfg, const isp_expander_ctx *expander_ctx)
{
    td_u8 i;
    reg_cfg->alg_reg_cfg[0].expander_cfg.enable = expander_ctx->mpi_cfg.enable;
    isp_expander_reg_calc(&reg_cfg->alg_reg_cfg[0].expander_cfg.usr_cfg, &expander_ctx->mpi_cfg);
    reg_cfg->alg_reg_cfg[0].expander_cfg.usr_cfg.update_index = 1;

    for (i = 1; i < reg_cfg->cfg_num; i++) {
        (td_void)memcpy_s(&reg_cfg->alg_reg_cfg[i].expander_cfg, sizeof(isp_expander_reg_cfg),
                          &reg_cfg->alg_reg_cfg[0].expander_cfg, sizeof(isp_expander_reg_cfg));
    }
    reg_cfg->cfg_key.bit1_expander_cfg = 1;

    return;
}

static td_s32 isp_expander_initialize(ot_vi_pipe vi_pipe, isp_expander_ctx *expander_ctx)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);
    isp_get_ctx(vi_pipe, isp_ctx);

    /* only enable expander in built-in mode */
    if (isp_ctx->sns_wdr_mode != OT_WDR_MODE_BUILT_IN) {
        isp_expander_initialize_def(expander_ctx);
        return TD_SUCCESS;
    }

    if (sns_dft->key.bit1_expander) {
        isp_check_pointer_return(sns_dft->expander);
        ret = isp_expander_attr_check("cmos", vi_pipe, sns_dft->expander);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        (td_void)memcpy_s(&expander_ctx->mpi_cfg, sizeof(ot_isp_expander_attr),
                          sns_dft->expander, sizeof(ot_isp_expander_attr));
    } else {
        isp_expander_initialize_def(expander_ctx);
    }

    return TD_SUCCESS;
}


static td_void isp_expander_ext_regs_initialize(ot_vi_pipe vi_pipe, const ot_isp_expander_attr *expander_attr)
{
    isp_expander_attr_write(vi_pipe, expander_attr);
    ot_ext_system_expander_param_update_write(vi_pipe, TD_FALSE);
    ot_ext_system_expander_blc_param_update_write(vi_pipe, TD_FALSE);
}

static td_void isp_expander_read_ext_regs(ot_vi_pipe vi_pipe, isp_expander_ctx *expander_ctx)
{
    expander_ctx->param_update = ot_ext_system_expander_param_update_read(vi_pipe);
    if (expander_ctx->param_update == TD_TRUE) {
        ot_ext_system_expander_param_update_write(vi_pipe, TD_FALSE);
        isp_expander_attr_read(vi_pipe, &expander_ctx->mpi_cfg);
    }
}

static td_s32 isp_expander_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_s32 ret;
    isp_expander_ctx *expander_ctx = TD_NULL;

    ot_ext_system_isp_expander_init_status_write(vi_pipe, TD_FALSE);
    ret = expander_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    expander_get_ctx(vi_pipe, expander_ctx);
    isp_check_pointer_return(expander_ctx);
    expander_ctx->init = TD_FALSE;

    ret = isp_expander_initialize(vi_pipe, expander_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    expander_regs_initialize((isp_reg_cfg *)reg_cfg_info, expander_ctx);
    isp_expander_ext_regs_initialize(vi_pipe, &expander_ctx->mpi_cfg);

    expander_ctx->init = TD_TRUE;
    ot_ext_system_isp_expander_init_status_write(vi_pipe, expander_ctx->init);

    return TD_SUCCESS;
}

static __inline td_bool check_expander_open(const isp_expander_ctx *expander_ctx)
{
    return (expander_ctx->mpi_cfg.enable == TD_TRUE);
}

static td_s32 isp_expander_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_info, td_s32 rsv)
{
    td_u8 i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_EXPANDER;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_expander_ctx *expander_ctx = TD_NULL;
    isp_reg_cfg  *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    ot_unused(stat_info);
    ot_unused(rsv);
    isp_check_pointer_success_return(reg_cfg);
    isp_get_ctx(vi_pipe, isp_ctx);
    expander_get_ctx(vi_pipe, expander_ctx);
    isp_check_pointer_return(expander_ctx);

    ot_ext_system_isp_expander_init_status_write(vi_pipe, expander_ctx->init);
    if (isp_ctx->sns_wdr_mode != OT_WDR_MODE_BUILT_IN) {
        return TD_SUCCESS;
    }

    if (expander_ctx->init == TD_FALSE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    expander_ctx->mpi_cfg.enable = ot_ext_system_expander_en_read(vi_pipe);
    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].expander_cfg.enable = expander_ctx->mpi_cfg.enable;
    }

    reg_cfg->cfg_key.bit1_expander_cfg = 1;

    /* check hardware setting */
    if (!check_expander_open(expander_ctx)) {
        return TD_SUCCESS;
    }

    isp_expander_read_ext_regs(vi_pipe, expander_ctx);

    isp_expander_reg_update(reg_cfg, expander_ctx);

    return TD_SUCCESS;
}

static td_s32 isp_expander_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_u8 i;
    td_s32 ret;
    td_u32 update_idx[OT_ISP_STRIPING_MAX_NUM] = {0};
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        update_idx[i] = reg_cfg->alg_reg_cfg[i].expander_cfg.usr_cfg.update_index;
    }

    ret = isp_expander_init(vi_pipe, reg_cfg_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp[%d] isp_expander_init failed!\n", vi_pipe);
        return ret;
    }

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].expander_cfg.usr_cfg.update_index = update_idx[i] + 1;
    }

    return TD_SUCCESS;
}

static td_s32 isp_expander_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    td_s32 ret = TD_SUCCESS;
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
            isp_check_pointer_return(reg_cfg_attr);
            ret = isp_expander_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg_attr->reg_cfg);
            break;
        default:
            break;
    }

    return ret;
}

static td_s32 isp_expander_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg_attr);
    ot_ext_system_isp_expander_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg_attr->reg_cfg.cfg_num; i++) {
        reg_cfg_attr->reg_cfg.alg_reg_cfg[i].expander_cfg.enable = TD_FALSE;
    }

    reg_cfg_attr->reg_cfg.cfg_key.bit1_expander_cfg = 1;

    expander_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_expander(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_expander);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_EXPANDER;
    algs->alg_func.pfn_alg_init = isp_expander_init;
    algs->alg_func.pfn_alg_run  = isp_expander_run;
    algs->alg_func.pfn_alg_ctrl = isp_expander_ctrl;
    algs->alg_func.pfn_alg_exit = isp_expander_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
