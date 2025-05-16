/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef ISP_SHARPEN_H
#define ISP_SHARPEN_H

#include "isp_config.h"
#include "ot_isp_debug.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"
#include "isp_ext_reg_access.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    td_u8   sft;
    td_u32  wgt_pre;
    td_u32  wgt_cur;
    td_s32  idx_cur;
    td_s32  idx_pre;
} isp_sharpen_inter_info;

typedef struct {
    td_bool init;
    td_bool sharpen_mpi_update_en;
    td_u32  iso_last;
    td_u32  iso;
    isp_sharpen_inter_info inter_info;

    /* tmp registers */
    td_u8   gain_thd_sft_d;
    td_u8   dir_var_sft;

    td_u8   sel_pix_wgt;
    td_u8   rmf_gain_scale;
    td_u8   bmf_gain_scale;

    td_u8   gain_thd_sel_ud;
    td_u8   gain_thd_sft_ud;

    td_u8    dir_rly_thr_low;
    td_u8    dir_rly_thrhigh;

    td_u8   sht_var_wgt0;
    td_u8   sht_var_diff_thd0;
    td_u8   sht_var_diff_thd1;
    td_u8   sht_var_diff_wgt1;

    ot_isp_sharpen_manual_attr actual; /* actual param */

    /* MPI */
    ot_isp_sharpen_attr mpi_cfg;  /* param read from mpi */
} isp_sharpen_ctx;

td_s32 isp_sharpen_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx);

td_void isp_sharpen_read_extregs(ot_vi_pipe vi_pipe, isp_sharpen_ctx *shp_ctx);
td_void isp_sharpen_read_pro_mode(ot_vi_pipe vi_pipe, const isp_sharpen_ctx *shp_ctx);
td_void isp_sharpen_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_sharpen_ctx *shp_ctx);
td_s32  sharpen_proc_write(ot_isp_ctrl_proc_write *proc, isp_sharpen_ctx *shp_ctx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
