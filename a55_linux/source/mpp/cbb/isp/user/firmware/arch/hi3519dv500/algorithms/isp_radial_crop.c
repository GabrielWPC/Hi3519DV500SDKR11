/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <math.h>
#include "isp_alg.h"
#include "isp_ext_config.h"
#include "isp_config.h"
#include "isp_proc.h"

typedef struct {
    td_bool rc_en;
    td_bool coef_update_en;
    td_u16 center_ver_coor;
    td_u16 center_hor_coor;
    td_u32 radius;
} isp_rc;

isp_rc g_radial_crop_ctx[OT_ISP_MAX_PIPE_NUM] = { { 0 } };
#define rc_get_ctx(pipe, ctx) ctx = &g_radial_crop_ctx[pipe]

static td_s16 rc_blk_center_hor_coor_calc(ot_vi_pipe vi_pipe, td_u8 cur_blk, td_s16 full_center_hor)
{
    td_u8 max_blk, i;
    td_s16 width_offset = 0;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    isp_block_attr *blk_attr = &isp_ctx->block_attr;

    max_blk = blk_attr->block_num;
    if (cur_blk >= max_blk) {
        return full_center_hor;
    }
    for (i = 0; i < cur_blk; i++) {
        width_offset += blk_attr->block_rect[i].width - (blk_attr->over_lap << 1);
    }
    return full_center_hor - width_offset;
}

static td_void rc_usr_regs_initialize(ot_vi_pipe vi_pipe, td_u8 cur_blk, isp_rc_usr_cfg *usr_reg_cfg, const isp_rc *rc)
{
    usr_reg_cfg->center_hor_coor = rc_blk_center_hor_coor_calc(vi_pipe, cur_blk, rc->center_hor_coor);
    usr_reg_cfg->center_ver_coor = rc->center_ver_coor;
    usr_reg_cfg->square_radius = rc->radius * rc->radius;
    usr_reg_cfg->usr_resh = TD_TRUE;
}

static td_void rc_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg)
{
    td_u8 i;
    isp_rc *rc = TD_NULL;

    rc_get_ctx(vi_pipe, rc);

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].rc_reg_cfg.rc_en = rc->rc_en;
        rc_usr_regs_initialize(vi_pipe, i, &reg_cfg->alg_reg_cfg[i].rc_reg_cfg.usr_reg_cfg, rc);
    }
    reg_cfg->cfg_key.bit1_rc_cfg = 1;

    return;
}

static td_void rc_ext_regs_initialize(ot_vi_pipe vi_pipe)
{
    isp_rc *rc = TD_NULL;

    rc_get_ctx(vi_pipe, rc);

    ot_ext_system_rc_en_write(vi_pipe, rc->rc_en);
    ot_ext_system_rc_center_hor_coord_write(vi_pipe, rc->center_hor_coor);
    ot_ext_system_rc_center_ver_coord_write(vi_pipe, rc->center_ver_coor);
    ot_ext_system_rc_radius_write(vi_pipe, rc->radius);
    ot_ext_system_rc_coef_update_en_write(vi_pipe, TD_FALSE);

    return;
}

static td_void rc_initialize(ot_vi_pipe vi_pipe)
{
    td_u32 hor_coor, ver_coor;
    isp_rc *rc = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    rc_get_ctx(vi_pipe, rc);

    hor_coor = isp_ctx->sys_rect.width >> 1;
    ver_coor = isp_ctx->sys_rect.height >> 1;

    rc->center_hor_coor = hor_coor;
    rc->center_ver_coor = ver_coor;
    rc->radius = (td_u32)sqrt((td_double)(hor_coor * hor_coor + ver_coor * ver_coor)) + 1;

    rc->rc_en = TD_FALSE;
}

inline static td_bool check_rc_open(const isp_rc *rc)
{
    return (rc->rc_en == TD_TRUE);
}

static td_void rc_read_ext_regs(ot_vi_pipe vi_pipe)
{
    isp_rc *rc = TD_NULL;
    rc_get_ctx(vi_pipe, rc);

    rc->coef_update_en = ot_ext_system_rc_coef_update_en_read(vi_pipe);

    ot_ext_system_rc_coef_update_en_write(vi_pipe, TD_FALSE);

    if (rc->coef_update_en) {
        rc->center_hor_coor = ot_ext_system_rc_center_hor_coord_read(vi_pipe);
        rc->center_ver_coor = ot_ext_system_rc_center_ver_coord_read(vi_pipe);
        rc->radius = ot_ext_system_rc_radius_read(vi_pipe);
    }
}

static td_void rc_usr_fw(ot_vi_pipe vi_pipe, td_u8 cur_blk, isp_rc_usr_cfg *usr_reg_cfg, const isp_rc *rc)
{
    usr_reg_cfg->center_hor_coor = rc_blk_center_hor_coor_calc(vi_pipe, cur_blk, rc->center_hor_coor);
    usr_reg_cfg->center_ver_coor = rc->center_ver_coor;
    usr_reg_cfg->square_radius = rc->radius * rc->radius;
    usr_reg_cfg->usr_resh = TD_TRUE;
}

static td_void isp_rc_wdr_mode_set(ot_vi_pipe vi_pipe, td_void *reg_cfg_in)
{
    td_u8 i;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_in;
    ot_unused(vi_pipe);

    reg_cfg->cfg_key.bit1_rc_cfg = 1;

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].rc_reg_cfg.usr_reg_cfg.usr_resh = TD_TRUE;
    }
}

static td_s32 isp_rc_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_in)
{
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_in;

    rc_initialize(vi_pipe);
    rc_regs_initialize(vi_pipe, reg_cfg);
    rc_ext_regs_initialize(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_rc_run(ot_vi_pipe vi_pipe, const td_void *stat_info,
    td_void *reg_cfg_in, td_s32 rsv)
{
    td_u8 i;
    isp_rc *rc = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_in;
    ot_unused(stat_info);
    ot_unused(rsv);

    isp_get_ctx(vi_pipe, isp_ctx);
    rc_get_ctx(vi_pipe, rc);

    if ((isp_ctx->frame_cnt % 2 != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) { /* run every 2 interrupts */
        return TD_SUCCESS;
    }

    rc->rc_en = ot_ext_system_rc_en_read(vi_pipe);
    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].rc_reg_cfg.rc_en = rc->rc_en;
    }
    reg_cfg->cfg_key.bit1_rc_cfg = 1;

    /* check hardware setting */
    if (!check_rc_open(rc)) {
        return TD_SUCCESS;
    }

    rc_read_ext_regs(vi_pipe);

    if (rc->coef_update_en) {
        for (i = 0; i < reg_cfg->cfg_num; i++) {
            rc_usr_fw(vi_pipe, i, &reg_cfg->alg_reg_cfg[i].rc_reg_cfg.usr_reg_cfg, rc);
        }
    }

    return TD_SUCCESS;
}

static td_s32 rc_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_rc *rc = TD_NULL;

    rc_get_ctx(vi_pipe, rc);
    isp_check_pointer_return(rc);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return TD_FAILURE;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "radial_crop info");
    isp_proc_printf(&proc_tmp, proc->write_len, "%12s" "%16s" "%16s" "%16s\n",
        "enable", "center_ver", "center_hor", "radius");
    isp_proc_printf(&proc_tmp, proc->write_len, "%12u" "%16u" "%16u" "%16u\n", rc->rc_en,
        rc->center_ver_coor, rc->center_hor_coor, rc->radius);

    proc->write_len += 1;

    return TD_SUCCESS;
}

static td_s32 isp_rc_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    ot_unused(value);

    switch (cmd) {
        case OT_ISP_WDR_MODE_SET:
            isp_regcfg_get_ctx(vi_pipe, reg_cfg);
            isp_check_pointer_return(reg_cfg);
            isp_rc_wdr_mode_set(vi_pipe, (td_void *)&reg_cfg->reg_cfg);
            break;
        case OT_ISP_PROC_WRITE:
            rc_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

static td_s32 isp_rc_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);

    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].rc_reg_cfg.rc_en = TD_FALSE;
    }
    reg_cfg->reg_cfg.cfg_key.bit1_rc_cfg = 1;

    return TD_SUCCESS;
}

td_s32 isp_alg_register_rc(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx  *isp_ctx = TD_NULL;
    isp_alg_node *algs    = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_rc);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_RC;
    algs->alg_func.pfn_alg_init = isp_rc_init;
    algs->alg_func.pfn_alg_run  = isp_rc_run;
    algs->alg_func.pfn_alg_ctrl = isp_rc_ctrl;
    algs->alg_func.pfn_alg_exit = isp_rc_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}

