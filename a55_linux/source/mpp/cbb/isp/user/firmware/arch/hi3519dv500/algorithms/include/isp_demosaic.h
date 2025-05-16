/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef ISP_DEMOSAIC_H
#define ISP_DEMOSAIC_H

#include "isp_config.h"
#include "ot_isp_debug.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    ot_vi_pipe vi_pipe;
    td_u8   non_dir_str;
    td_u32  iso;
} gf_blur_lut_cfg;

typedef struct {
    td_u32 iso;
    td_u32 iso1;
    td_u32 iso2;
    td_u8  idx_upper;
    td_u8  idx_low;
} demosaic_iso_cfg;

typedef struct {
    /* Processing Depth */
    td_bool  init;
    td_bool  dem_attr_update;

    td_u8    bit_depth_prc;    /* u5.0 */
    td_u8    wdr_mode;
    td_u32   sigma_demcfg[OT_ISP_DEMOSAIC_LUT_LENGTH];

    td_u8    lut_awb_gfgain_low[OT_ISP_AUTO_ISO_NUM]; /* u3.4, */
    td_u8    lut_awb_gfgain_hig[OT_ISP_AUTO_ISO_NUM]; /* u3.4, */
    td_u8    lut_awb_gfgain_max[OT_ISP_AUTO_ISO_NUM]; /* u4.0, */
    td_u8    lut_cc_hf_max_ratio[OT_ISP_AUTO_ISO_NUM]; /* u5.0, 0~16 */
    td_u8    lut_cc_hf_min_ratio[OT_ISP_AUTO_ISO_NUM]; /* u5.0, 0~16 */
    td_s8    lut_fcr_gf_gain[OT_ISP_AUTO_ISO_NUM]; /* s3.2, fcr control */
    td_u16   lut_desat_low[OT_ISP_AUTO_ISO_NUM]; /* u9.0, 0~256 */
    td_u16   lut_desat_prot_th[OT_ISP_AUTO_ISO_NUM]; /* u10.0, 0~1023 */
    td_u16   lut_fcr_det_low[OT_ISP_AUTO_ISO_NUM]; /* u12.0,  fcr det thresh */
    td_u16   hf_intp_blur_th_low[OT_ISP_AUTO_ISO_NUM]; /* u10.0 */
    td_u16   hf_intp_blur_th_hig[OT_ISP_AUTO_ISO_NUM]; /* u10.0, */

    ot_isp_demosaic_manual_attr actual; /* actual param */

    /* MPI */
    ot_isp_demosaic_attr mpi_cfg; /* param read from mpi */
} isp_demosaic;

td_s32 isp_demosaic_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_demosaic *demosaic_ctx);
td_void isp_demosaic_read_extregs(ot_vi_pipe vi_pipe, isp_demosaic *demosaic);
td_void isp_demosaic_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg, isp_demosaic *demosaic);
td_void isp_demosaic_set_long_frame_mode(ot_vi_pipe vi_pipe, isp_demosaic *demosaic);
td_s32 demosaic_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc, const isp_demosaic *demosaic);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
