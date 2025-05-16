/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_param_check.h"
#include "isp_sensor.h"

#include "ot_mpi_sys_mem.h"

#define OT_ISP_CLUT_GAIN        128
#define OT_ISP_CLUT_FRAME_CNT   2

static const td_u16 g_gain_r = OT_ISP_CLUT_GAIN;
static const td_u16 g_gain_g = OT_ISP_CLUT_GAIN;
static const td_u16 g_gain_b = OT_ISP_CLUT_GAIN;

typedef struct {
    td_bool         init;
    td_bool         clut_lut_update_en;
    td_bool         clut_ctrl_update_en;
    td_u32          *vir_addr;
    ot_isp_clut_attr clut_ctrl;
} isp_clut_ctx;

isp_clut_ctx g_clut_ctx[OT_ISP_MAX_PIPE_NUM] = {{0}};

#define clut_get_ctx(pipe, ctx)   ctx = &g_clut_ctx[pipe]

static td_void clut_ext_regs_initialize(ot_vi_pipe vi_pipe, const isp_clut_ctx *clut_ctx)
{
    ot_ext_system_clut_en_write(vi_pipe, clut_ctx->clut_ctrl.enable);
    ot_ext_system_clut_gain_r_write(vi_pipe, (td_u16)clut_ctx->clut_ctrl.gain_r);
    ot_ext_system_clut_gain_g_write(vi_pipe, (td_u16)clut_ctx->clut_ctrl.gain_g);
    ot_ext_system_clut_gain_b_write(vi_pipe, (td_u16)clut_ctx->clut_ctrl.gain_b);
    ot_ext_system_clut_ctrl_update_en_write(vi_pipe, TD_FALSE);
    ot_ext_system_clut_lut_update_en_write(vi_pipe, TD_FALSE);
}

static td_void clut_usr_coef_regs_initialize(ot_vi_pipe vi_pipe, isp_clut_usr_coef_cfg *usr_coef_reg_cfg)
{
    td_s32 ret;
    isp_clut_ctx  *clut_ctx  = TD_NULL;
    clut_get_ctx(vi_pipe, clut_ctx);

    ret = memcpy_s(usr_coef_reg_cfg->lu_stt, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                   clut_ctx->vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    if (ret != EOK) {
        isp_err_trace("ISP[%d] init clut lut failed!\n", vi_pipe);
        return;
    }

    usr_coef_reg_cfg->resh          = TD_TRUE;
    usr_coef_reg_cfg->buf_id        = 0;
    usr_coef_reg_cfg->update_index = 1;
    return;
}

static td_void clut_usr_ctrl_regs_initialize(ot_vi_pipe vi_pipe, isp_clut_usr_ctrl_cfg *usr_ctrl_reg_cfg)
{
    ot_unused(vi_pipe);
    usr_ctrl_reg_cfg->demo_mode      = TD_FALSE;
    usr_ctrl_reg_cfg->gain_r         = OT_ISP_CLUT_GAIN;
    usr_ctrl_reg_cfg->gain_b         = OT_ISP_CLUT_GAIN;
    usr_ctrl_reg_cfg->gain_g         = OT_ISP_CLUT_GAIN;
    usr_ctrl_reg_cfg->demo_enable    = TD_FALSE;
    usr_ctrl_reg_cfg->resh           = TD_TRUE;
}
static td_void clut_usr_regs_initialize(ot_vi_pipe vi_pipe, isp_clut_usr_cfg *usr_reg_cfg)
{
    clut_usr_coef_regs_initialize(vi_pipe, &usr_reg_cfg->clut_usr_coef_cfg);
    clut_usr_ctrl_regs_initialize(vi_pipe, &usr_reg_cfg->clut_usr_ctrl_cfg);
}

static td_s32 clut_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, const isp_clut_ctx *clut_ctx)
{
    td_bool offline_mode;
    td_u8   i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode = (is_offline_mode(isp_ctx->block_attr.running_mode) ||
                    is_striping_mode(isp_ctx->block_attr.running_mode) ||
                    is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        clut_usr_regs_initialize(vi_pipe, &reg_cfg->alg_reg_cfg[i].clut_cfg.usr_reg_cfg);
        reg_cfg->alg_reg_cfg[i].clut_cfg.enable = offline_mode ? (clut_ctx->clut_ctrl.enable) : (TD_FALSE);
    }

    reg_cfg->cfg_key.bit1_clut_cfg   = 1;

    return TD_SUCCESS;
}

static td_void clut_read_extregs(ot_vi_pipe vi_pipe, isp_clut_ctx *clut_ctx)
{
    clut_ctx->clut_ctrl_update_en = ot_ext_system_clut_ctrl_update_en_read(vi_pipe);
    if (clut_ctx->clut_ctrl_update_en) {
        ot_ext_system_clut_ctrl_update_en_write(vi_pipe, TD_FALSE);
        clut_ctx->clut_ctrl.enable = ot_ext_system_clut_en_read(vi_pipe);
        clut_ctx->clut_ctrl.gain_r = ot_ext_system_clut_gain_r_read(vi_pipe);
        clut_ctx->clut_ctrl.gain_g = ot_ext_system_clut_gain_g_read(vi_pipe);
        clut_ctx->clut_ctrl.gain_b = ot_ext_system_clut_gain_b_read(vi_pipe);
    }

    clut_ctx->clut_lut_update_en = ot_ext_system_clut_lut_update_en_read(vi_pipe);
    if (clut_ctx->clut_lut_update_en) {
        ot_ext_system_clut_lut_update_en_write(vi_pipe, TD_FALSE);
    }

    return;
}

static td_s32 clut_check_cmos_param(ot_vi_pipe vi_pipe, const ot_isp_clut_attr *clut_attr)
{
    td_s32 ret;
    ot_unused(vi_pipe);
    ret = isp_clut_attr_check("cmos", clut_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_s32 clut_initialize(ot_vi_pipe vi_pipe, isp_clut_ctx  *clut_ctx)
{
    td_s32 ret;
    isp_mmz_buf_ex clut_buf;
    ot_isp_cmos_default *sns_dft  = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    clut_ctx->vir_addr = TD_NULL;

    if (ioctl(isp_get_fd(vi_pipe), ISP_CLUT_BUF_GET, &clut_buf.phy_addr) != TD_SUCCESS) {
        isp_err_trace("Get Clut Buffer Err\n");
        return TD_FAILURE;
    }

    clut_buf.vir_addr = ot_mpi_sys_mmap(clut_buf.phy_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));

    if (clut_buf.vir_addr == TD_NULL) {
        return TD_FAILURE;
    }

    clut_ctx->vir_addr = (td_u32 *)clut_buf.vir_addr;

    if (sns_dft->key.bit1_clut) {
        isp_check_pointer_return(sns_dft->clut);

        ret = clut_check_cmos_param(vi_pipe, &sns_dft->clut->clut_attr);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        clut_ctx->clut_ctrl.enable = sns_dft->clut->clut_attr.enable;
        clut_ctx->clut_ctrl.gain_r = sns_dft->clut->clut_attr.gain_r;
        clut_ctx->clut_ctrl.gain_g = sns_dft->clut->clut_attr.gain_g;
        clut_ctx->clut_ctrl.gain_b = sns_dft->clut->clut_attr.gain_b;

        ret = memcpy_s(clut_ctx->vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                       sns_dft->clut->clut_lut.lut, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
        isp_check_eok_return(ret, TD_FAILURE);
    } else {
        clut_ctx->clut_ctrl.enable = TD_FALSE;
        clut_ctx->clut_ctrl.gain_r = (td_u32)g_gain_r;
        clut_ctx->clut_ctrl.gain_g = (td_u32)g_gain_g;
        clut_ctx->clut_ctrl.gain_b = (td_u32)g_gain_b;

        (td_void)memset_s(clut_ctx->vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                          0, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    }

    return TD_SUCCESS;
}

static td_void isp_clut_usr_coef_fw(const isp_clut_ctx *clut_ctx, isp_clut_usr_coef_cfg *clut_usr_coef_cfg)
{
    td_s32 ret;
    ret = memcpy_s(clut_usr_coef_cfg->lu_stt, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                   clut_ctx->vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    if (ret != EOK) {
        isp_err_trace("ISP update clut lut failed!\n");
        return;
    }

    clut_usr_coef_cfg->resh  = TD_TRUE;
    clut_usr_coef_cfg->update_index += 1;
    return;
}

static td_void isp_clut_usr_ctrl_fw(const ot_isp_clut_attr *clut_ctrl, isp_clut_usr_ctrl_cfg *clut_usr_ctrl_cfg)
{
    clut_usr_ctrl_cfg->gain_r       = clut_ctrl->gain_r;
    clut_usr_ctrl_cfg->gain_g       = clut_ctrl->gain_g;
    clut_usr_ctrl_cfg->gain_b       = clut_ctrl->gain_b;
    clut_usr_ctrl_cfg->resh         = TD_TRUE;
}

static td_bool check_clut_open(const isp_clut_ctx *clut_ctx)
{
    return (clut_ctx->clut_ctrl.enable == TD_TRUE);
}

static td_s32 isp_clut_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_s32 ret;
    isp_clut_ctx *clut_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    clut_get_ctx(vi_pipe, clut_ctx);
    clut_ctx->init = TD_FALSE;
    ot_ext_system_isp_clut_init_status_write(vi_pipe, clut_ctx->init);
    ret = clut_initialize(vi_pipe, clut_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = clut_regs_initialize(vi_pipe, reg_cfg, clut_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    clut_ext_regs_initialize(vi_pipe, clut_ctx);

    clut_ctx->init = TD_TRUE;
    ot_ext_system_isp_clut_init_status_write(vi_pipe, clut_ctx->init);
    return TD_SUCCESS;
}

static td_s32 isp_clut_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8 i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_CLUT;
    isp_usr_ctx  *isp_ctx  = TD_NULL;
    isp_clut_ctx *clut_ctx = TD_NULL;
    isp_reg_cfg  *reg      = (isp_reg_cfg *)reg_cfg;

    ot_unused(stat_info);
    ot_unused(rsv);
    isp_get_ctx(vi_pipe, isp_ctx);
    clut_get_ctx(vi_pipe, clut_ctx);

    ot_ext_system_isp_clut_init_status_write(vi_pipe, clut_ctx->init);
    if (clut_ctx->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    /* calculate every two interrupts */
    if ((isp_ctx->frame_cnt % OT_ISP_CLUT_FRAME_CNT != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) {
        return TD_SUCCESS;
    }

    clut_ctx->clut_ctrl.enable = ot_ext_system_clut_en_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].clut_cfg.enable = clut_ctx->clut_ctrl.enable;
    }

    reg->cfg_key.bit1_clut_cfg = 1;

    /* check hardware setting */
    if (!check_clut_open(clut_ctx)) {
        return TD_SUCCESS;
    }

    clut_read_extregs(vi_pipe, clut_ctx);

    if (clut_ctx->clut_ctrl_update_en) {
        for (i = 0; i < reg->cfg_num; i++) {
            isp_clut_usr_ctrl_fw(&clut_ctx->clut_ctrl, &reg->alg_reg_cfg[i].clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg);
        }
    }

    if (clut_ctx->clut_lut_update_en) {
        for (i = 0; i < reg->cfg_num; i++) {
            if (clut_ctx->vir_addr == TD_NULL) {
                return TD_FAILURE;
            }

            isp_clut_usr_coef_fw(clut_ctx, &reg->alg_reg_cfg[i].clut_cfg.usr_reg_cfg.clut_usr_coef_cfg);
        }
    }

    return TD_SUCCESS;
}
static td_s32 isp_clut_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    ot_unused(vi_pipe);
    ot_unused(cmd);
    ot_unused(value);
    return TD_SUCCESS;
}

static td_s32 isp_clut_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    isp_clut_ctx *clut_ctx = TD_NULL;

    clut_get_ctx(vi_pipe, clut_ctx);

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    ot_ext_system_isp_clut_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].clut_cfg.enable = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_clut_cfg = 1;

    if (clut_ctx->vir_addr != TD_NULL) {
        ot_mpi_sys_munmap((td_void *)clut_ctx->vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    }

    return TD_SUCCESS;
}

td_s32 isp_alg_register_clut(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_clut);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_CLUT;
    algs->alg_func.pfn_alg_init = isp_clut_init;
    algs->alg_func.pfn_alg_run  = isp_clut_run;
    algs->alg_func.pfn_alg_ctrl = isp_clut_ctrl;
    algs->alg_func.pfn_alg_exit = isp_clut_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
