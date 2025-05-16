/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "isp_alg.h"
#include "isp_ext_config.h"
#include "isp_param_check.h"
#include "isp_config.h"
#include "isp_sensor.h"
#include "isp_math_utils.h"
#include "isp_proc.h"

#define LBLC_RAW_BITWIDTH      12
#define LBLC_WIN_INV_BITWIDTH  10
#define LBLC_DEFAULT_VALUE  4096

typedef struct {
    td_bool init;
    td_bool lblc_enable;
    td_bool lblc_coef_update;
    td_bool lblc_lut_update;

    td_u16  strength;
    td_u16  delta_x[OT_ISP_LBLC_GRID_COL - 1];
    td_u16  delta_y[ISP_LBLC_Y_HALF_GRID_NUM];
    td_u16  inv_x[OT_ISP_LBLC_GRID_COL - 1];
    td_u16  inv_y[ISP_LBLC_Y_HALF_GRID_NUM];
} isp_lblc;

isp_lblc *g_lblc_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define lblc_get_ctx(pipe, ctx)   ((ctx) = g_lblc_ctx[pipe])
#define lblc_set_ctx(pipe, ctx)   (g_lblc_ctx[pipe] = (ctx))
#define lblc_reset_ctx(pipe)      (g_lblc_ctx[pipe] = TD_NULL)

static td_s32 lblc_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_lblc *lblc_ctx = TD_NULL;

    lblc_get_ctx(vi_pipe, lblc_ctx);

    if (lblc_ctx == TD_NULL) {
        lblc_ctx = (isp_lblc *)isp_malloc(sizeof(isp_lblc));
        if (lblc_ctx == TD_NULL) {
            isp_err_trace("isp[%d] lblc_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(lblc_ctx, sizeof(isp_lblc), 0, sizeof(isp_lblc));

    lblc_set_ctx(vi_pipe, lblc_ctx);

    return TD_SUCCESS;
}

static td_void lblc_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_lblc *lblc_ctx = TD_NULL;

    lblc_get_ctx(vi_pipe, lblc_ctx);
    isp_free(lblc_ctx);
    lblc_reset_ctx(vi_pipe);
}

static td_void lblc_static_regs_initialize(const isp_lblc *lblc, isp_lblc_static_cfg *static_reg_cfg)
{
    td_u16 i;
    static_reg_cfg->lblc_num_col   = OT_ISP_LBLC_GRID_COL - 1; /* will add 1 in logic */
    static_reg_cfg->lblc_num_row   = OT_ISP_LBLC_GRID_ROW - 1; /* will add 1 in logic */
    static_reg_cfg->static_resh = TD_TRUE;

    static_reg_cfg->lblc_dither_en = TD_TRUE;
    static_reg_cfg->width_offset = 0;
    static_reg_cfg->bitwidth_in = LBLC_RAW_BITWIDTH;
    static_reg_cfg->lut_shift_bit = 0;

    for (i = 0; i < OT_ISP_LBLC_GRID_COL - 1; i++) {
        static_reg_cfg->delta_x[i] = lblc->delta_x[i];
        static_reg_cfg->inv_x[i]   = lblc->inv_x[i];
    }

    for (i = 0; i < ISP_LBLC_Y_HALF_GRID_NUM; i++) {
        static_reg_cfg->delta_y[i] = lblc->delta_y[i];
        static_reg_cfg->inv_y[i]   = lblc->inv_y[i];
    }

    return;
}

static td_void lblc_usr_regs_initialize(ot_vi_pipe vi_pipe, isp_lblc_usr_cfg *usr_reg_cfg)
{
    td_u32 i;
    isp_lblc *lblc = TD_NULL;

    ot_isp_cmos_default    *sns_dft   = TD_NULL;
    const ot_isp_cmos_lblc *cmos_lblc = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);
    lblc_get_ctx(vi_pipe, lblc);
    isp_check_pointer_void_return(lblc);
    if (sns_dft->key.bit1_lblc) {
        isp_check_pointer_void_return(sns_dft->lblc);
        cmos_lblc = sns_dft->lblc;

        usr_reg_cfg->strength = cmos_lblc->lblc_attr.strength;
        for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
            usr_reg_cfg->mesh_blc_r[i]  = cmos_lblc->lblc_lut.mesh_blc_r[i];
            usr_reg_cfg->mesh_blc_gr[i]  = cmos_lblc->lblc_lut.mesh_blc_gr[i];
            usr_reg_cfg->mesh_blc_gb[i]  = cmos_lblc->lblc_lut.mesh_blc_gb[i];
            usr_reg_cfg->mesh_blc_b[i]  = cmos_lblc->lblc_lut.mesh_blc_b[i];
        }
        usr_reg_cfg->offset_r = cmos_lblc->lblc_lut.offset_r;
        usr_reg_cfg->offset_gr = cmos_lblc->lblc_lut.offset_gr;
        usr_reg_cfg->offset_gb = cmos_lblc->lblc_lut.offset_gb;
        usr_reg_cfg->offset_b = cmos_lblc->lblc_lut.offset_b;
    } else {
        usr_reg_cfg->strength = OT_ISP_LBLC_DEFAULT_STRENGTH;
        for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
            usr_reg_cfg->mesh_blc_r[i]  = LBLC_DEFAULT_VALUE;
            usr_reg_cfg->mesh_blc_gr[i]  = LBLC_DEFAULT_VALUE;
            usr_reg_cfg->mesh_blc_gb[i]  = LBLC_DEFAULT_VALUE;
            usr_reg_cfg->mesh_blc_b[i]  = LBLC_DEFAULT_VALUE;
        }
        usr_reg_cfg->offset_r = LBLC_DEFAULT_VALUE;
        usr_reg_cfg->offset_gr = LBLC_DEFAULT_VALUE;
        usr_reg_cfg->offset_gb = LBLC_DEFAULT_VALUE;
        usr_reg_cfg->offset_b = LBLC_DEFAULT_VALUE;
    }

    usr_reg_cfg->lut2stt_sync_cfg.offset_update = TD_TRUE;
    usr_reg_cfg->lut2stt_sync_cfg.offset_r = usr_reg_cfg->offset_r;
    usr_reg_cfg->lut2stt_sync_cfg.offset_gr = usr_reg_cfg->offset_gr;
    usr_reg_cfg->lut2stt_sync_cfg.offset_gb = usr_reg_cfg->offset_gb;
    usr_reg_cfg->lut2stt_sync_cfg.offset_b = usr_reg_cfg->offset_b;

    usr_reg_cfg->update_index = 1;
    usr_reg_cfg->lut_update           = TD_TRUE;
    usr_reg_cfg->coef_update          = TD_TRUE;
    usr_reg_cfg->buf_id               = 0;
}

static td_void lblc_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, const isp_lblc *lblc)
{
    lblc_static_regs_initialize(lblc, &reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.static_reg_cfg);
    lblc_usr_regs_initialize(vi_pipe, &reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.usr_reg_cfg);

    reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.lblc_en = lblc->lblc_enable;
    reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.lut2_stt_en          = TD_TRUE;
    reg_cfg->alg_reg_cfg[0].fe_lut_update_cfg.lblc_lut_update = TD_TRUE;

    reg_cfg->cfg_key.bit1_lblc_cfg = 1;
}

static td_void lblc_ext_regs_initialize(ot_vi_pipe vi_pipe, const isp_lblc *lblc)
{
    td_u16 i;
    ot_isp_cmos_default    *sns_dft  = TD_NULL;
    const ot_isp_cmos_lblc *cmos_lblc = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    ot_ext_system_isp_lblc_enable_write(vi_pipe, lblc->lblc_enable);
    ot_ext_system_isp_lblc_attr_updata_write(vi_pipe, TD_FALSE);
    ot_ext_system_isp_lblc_lut_attr_updata_write(vi_pipe, TD_FALSE);
    ot_ext_system_isp_lblc_strength_write(vi_pipe, lblc->strength);

    if (sns_dft->key.bit1_lblc) {
        isp_check_pointer_void_return(sns_dft->lblc);
        cmos_lblc = sns_dft->lblc;

        for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
            ot_ext_system_isp_lblc_mesh_blc_r_write(vi_pipe, i, cmos_lblc->lblc_lut.mesh_blc_r[i]);
            ot_ext_system_isp_lblc_mesh_blc_gr_write(vi_pipe, i, cmos_lblc->lblc_lut.mesh_blc_gr[i]);
            ot_ext_system_isp_lblc_mesh_blc_gb_write(vi_pipe, i, cmos_lblc->lblc_lut.mesh_blc_gb[i]);
            ot_ext_system_isp_lblc_mesh_blc_b_write(vi_pipe, i, cmos_lblc->lblc_lut.mesh_blc_b[i]);
        }
        ot_ext_system_isp_lblc_offset_r_write(vi_pipe, cmos_lblc->lblc_lut.offset_r);
        ot_ext_system_isp_lblc_offset_gr_write(vi_pipe, cmos_lblc->lblc_lut.offset_gr);
        ot_ext_system_isp_lblc_offset_gb_write(vi_pipe, cmos_lblc->lblc_lut.offset_gb);
        ot_ext_system_isp_lblc_offset_b_write(vi_pipe, cmos_lblc->lblc_lut.offset_b);
    } else {
        for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
            ot_ext_system_isp_lblc_mesh_blc_r_write(vi_pipe, i, LBLC_DEFAULT_VALUE);
            ot_ext_system_isp_lblc_mesh_blc_gr_write(vi_pipe, i, LBLC_DEFAULT_VALUE);
            ot_ext_system_isp_lblc_mesh_blc_gb_write(vi_pipe, i, LBLC_DEFAULT_VALUE);
            ot_ext_system_isp_lblc_mesh_blc_b_write(vi_pipe, i, LBLC_DEFAULT_VALUE);
        }
        ot_ext_system_isp_lblc_offset_r_write(vi_pipe, LBLC_DEFAULT_VALUE);
        ot_ext_system_isp_lblc_offset_gr_write(vi_pipe, LBLC_DEFAULT_VALUE);
        ot_ext_system_isp_lblc_offset_gb_write(vi_pipe, LBLC_DEFAULT_VALUE);
        ot_ext_system_isp_lblc_offset_b_write(vi_pipe, LBLC_DEFAULT_VALUE);
    }

    return;
}

static td_void lblc_read_ext_regs(ot_vi_pipe vi_pipe, isp_lblc *lblc)
{
    lblc->lblc_coef_update = ot_ext_system_isp_lblc_attr_updata_read(vi_pipe);
    if (lblc->lblc_coef_update) {
        ot_ext_system_isp_lblc_attr_updata_write(vi_pipe, TD_FALSE);
        lblc->strength = ot_ext_system_isp_lblc_strength_read(vi_pipe);
    }

    lblc->lblc_lut_update = ot_ext_system_isp_lblc_lut_attr_updata_read(vi_pipe);
    if (lblc->lblc_lut_update) {
        ot_ext_system_isp_lblc_lut_attr_updata_write(vi_pipe, TD_FALSE);
        /* copy blc lut in lblc_usr_fw */
    }

    return;
}

static td_void geometric_grid_size(td_u16 *delta, td_u16 *inv, td_u16 length, td_u16 grid_size)
{
    td_u16 i, sum;
    td_u16 half_grid_size;
    td_u16 diff;
    td_u16 *tmp_step = TD_NULL;
    td_u16 sum_r;

    half_grid_size = (grid_size - 1) >> 1;

    if (half_grid_size == 0) {
        return;
    }

    tmp_step = (td_u16 *)isp_malloc(sizeof(td_u16) * half_grid_size);
    if (tmp_step == TD_NULL) {
        return;
    }

    (td_void)memset_s(tmp_step, sizeof(td_u16) * half_grid_size, 0, sizeof(td_u16) * half_grid_size);

    sum_r = (td_u16)((((length >> 1) * 0x400 / div_0_to_1(half_grid_size)) + 0x200) >> LBLC_WIN_INV_BITWIDTH);

    for (i = 0; i < half_grid_size; i++) {
        tmp_step[i] = sum_r;
    }

    sum = 0;
    for (i = 0; i < half_grid_size; i++) {
        sum = sum + tmp_step[i];
    }

    if (sum != (length >> 1)) {
        if (sum > (length >> 1)) {
            diff = sum - (length >> 1);
            for (i = 0; i < diff; i++) {
                tmp_step[i] = tmp_step[i] - 1;
            }
        } else {
            diff = (length >> 1) - sum;
            for (i = 0; i < diff; i++) {
                tmp_step[half_grid_size - i - 1] = tmp_step[half_grid_size - i - 1] + 1;
            }
        }
    }

    for (i = 0; i < half_grid_size; i++) {
        delta[i] = tmp_step[i];
        inv[i] = (delta[i] == 0) ? (0) : ((0x1000 * 0x400 / div_0_to_1(delta[i]) + 0x200) >> LBLC_WIN_INV_BITWIDTH);
    }

    isp_free(tmp_step);
}

static td_void lblc_image_size(ot_vi_pipe vi_pipe, isp_lblc *lblc)
{
    td_u16 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    geometric_grid_size(lblc->delta_x, lblc->inv_x, isp_ctx->sys_rect.width >> 1, OT_ISP_LBLC_GRID_COL);
    geometric_grid_size(lblc->delta_y, lblc->inv_y, isp_ctx->sys_rect.height >> 1, OT_ISP_LBLC_GRID_ROW);

    for (i = 0; i < ISP_LBLC_X_HALF_GRID_NUM; i++) {
        lblc->delta_x[OT_ISP_LBLC_GRID_COL - 2 - i] = lblc->delta_x[i]; /* 2 for other half of delta_x */
        lblc->inv_x[OT_ISP_LBLC_GRID_COL - 2 - i]   = lblc->inv_x[i]; /* 2 for other half of inv_x */
    }
}

static td_s32 lblc_check_cmos_param(ot_vi_pipe vi_pipe, const ot_isp_cmos_lblc *cmos_lblc)
{
    isp_check_bool_return(cmos_lblc->lblc_attr.enable);

    if (cmos_lblc->lblc_attr.strength > OT_ISP_LBLC_STRENGTH_MAX) {
        isp_err_trace("Invalid lblc strength!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    return isp_lblc_lut_attr_check("cmos", vi_pipe, &cmos_lblc->lblc_lut);
}

static td_s32 lblc_initialize(ot_vi_pipe vi_pipe, isp_lblc *lblc)
{
    td_s32 ret;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);
    if (sns_dft->key.bit1_lblc) {
        isp_check_pointer_return(sns_dft->lblc);

        ret = lblc_check_cmos_param(vi_pipe, sns_dft->lblc);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        lblc->lblc_enable = sns_dft->lblc->lblc_attr.enable;
        lblc->strength    = sns_dft->lblc->lblc_attr.strength;

        lblc_image_size(vi_pipe, lblc);
    } else {
        lblc->lblc_enable = TD_TRUE;
        lblc->strength    = OT_ISP_LBLC_DEFAULT_STRENGTH;

        lblc_image_size(vi_pipe, lblc);
    }

    return TD_SUCCESS;
}


static td_void lblc_usr_fw(ot_vi_pipe vi_pipe, isp_lblc_usr_cfg *usr_reg_cfg)
{
    td_u16 i;

    usr_reg_cfg->update_index += 1;
    usr_reg_cfg->lut_update    = TD_TRUE;

    for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
        usr_reg_cfg->mesh_blc_r[i] = ot_ext_system_isp_lblc_mesh_blc_r_read(vi_pipe, i);
        usr_reg_cfg->mesh_blc_gr[i] = ot_ext_system_isp_lblc_mesh_blc_gr_read(vi_pipe, i);
        usr_reg_cfg->mesh_blc_gb[i] = ot_ext_system_isp_lblc_mesh_blc_gb_read(vi_pipe, i);
        usr_reg_cfg->mesh_blc_b[i] = ot_ext_system_isp_lblc_mesh_blc_b_read(vi_pipe, i);
    }
    usr_reg_cfg->offset_r = ot_ext_system_isp_lblc_offset_r_read(vi_pipe);
    usr_reg_cfg->offset_gr = ot_ext_system_isp_lblc_offset_gr_read(vi_pipe);
    usr_reg_cfg->offset_gb = ot_ext_system_isp_lblc_offset_gb_read(vi_pipe);
    usr_reg_cfg->offset_b = ot_ext_system_isp_lblc_offset_b_read(vi_pipe);

    return;
}


td_s32 isp_lblc_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_in)
{
    td_s32 ret;
    isp_lblc *lblc_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_in;

    ot_ext_system_isp_lblc_init_status_write(vi_pipe, TD_FALSE);
    ret = lblc_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    lblc_get_ctx(vi_pipe, lblc_ctx);
    isp_check_pointer_return(lblc_ctx);
    lblc_ctx->init = TD_FALSE;

    ret = lblc_initialize(vi_pipe, lblc_ctx);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    lblc_regs_initialize(vi_pipe, reg_cfg, lblc_ctx);
    lblc_ext_regs_initialize(vi_pipe, lblc_ctx);

    lblc_ctx->init = TD_TRUE;
    ot_ext_system_isp_lblc_init_status_write(vi_pipe, lblc_ctx->init);

    return TD_SUCCESS;
}

td_s32 isp_lblc_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg_in, td_s32 rsv)
{
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_LBLC;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lblc *lblc = TD_NULL;
    isp_reg_cfg *reg_cfg  = (isp_reg_cfg *)reg_cfg_in;
    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    lblc_get_ctx(vi_pipe, lblc);
    isp_check_pointer_return(lblc);

    if (isp_ctx->linkage.defect_pixel) {
        return TD_SUCCESS;
    }

    ot_ext_system_isp_lblc_init_status_write(vi_pipe, lblc->init);
    if (lblc->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    lblc->lblc_enable = ot_ext_system_isp_lblc_enable_read(vi_pipe);
    reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.lblc_en = lblc->lblc_enable;

    reg_cfg->cfg_key.bit1_lblc_cfg = 1;
    if (lblc->lblc_enable == TD_FALSE) {
        return TD_SUCCESS;
    }

    lblc_read_ext_regs(vi_pipe, lblc);

    if (lblc->lblc_coef_update) {
        reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.usr_reg_cfg.coef_update = TD_TRUE;
        reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.usr_reg_cfg.strength    = lblc->strength;
    }

    if (lblc->lblc_lut_update) {
        lblc_usr_fw(vi_pipe, &reg_cfg->alg_reg_cfg[0].lblc_reg_cfg.usr_reg_cfg);
    }

    return TD_SUCCESS;
}

static td_s32 lblc_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_lblc *lblc = TD_NULL;
    td_u16  offset_r, offset_gr, offset_gb, offset_b;

    lblc_get_ctx(vi_pipe, lblc);
    isp_check_pointer_return(lblc);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    offset_r = ot_ext_system_isp_lblc_offset_r_read(vi_pipe);
    offset_gr = ot_ext_system_isp_lblc_offset_gr_read(vi_pipe);
    offset_gb = ot_ext_system_isp_lblc_offset_gb_read(vi_pipe);
    offset_b = ot_ext_system_isp_lblc_offset_b_read(vi_pipe);

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "lblc info");
    isp_proc_printf(&proc_tmp, proc->write_len, "%12s" "%16s" "%16s" "%16s" "%16s" "%16s\n",
        "enable", "strength", "offset_r", "offset_gr", "offset_gb", "offset_b");
    isp_proc_printf(&proc_tmp, proc->write_len, "%12u" "%16u" "%16u" "%16u" "%16u" "%16u\n", (td_u16)lblc->lblc_enable,
        lblc->strength, offset_r, offset_gr, offset_gb, offset_b);

    proc->write_len += 1;

    return TD_SUCCESS;
}

td_s32 isp_lblc_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_CHANGE_IMAGE_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            lblc_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        case OT_ISP_WDR_MODE_SET:
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

td_s32 isp_lblc_exit(ot_vi_pipe vi_pipe)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    ot_ext_system_isp_lblc_init_status_write(vi_pipe, TD_FALSE);
    reg_cfg->reg_cfg.alg_reg_cfg[0].lblc_reg_cfg.lblc_en = TD_FALSE;

    reg_cfg->reg_cfg.cfg_key.bit1_lblc_cfg = 1;

    lblc_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_lblc(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_lblc);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_LBLC;
    algs->alg_func.pfn_alg_init = isp_lblc_init;
    algs->alg_func.pfn_alg_run  = isp_lblc_run;
    algs->alg_func.pfn_alg_ctrl = isp_lblc_ctrl;
    algs->alg_func.pfn_alg_exit = isp_lblc_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
