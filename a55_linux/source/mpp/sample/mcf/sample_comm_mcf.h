/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __SAMPLE_COMM_MCF_H__
#define __SAMPLE_COMM_MCF_H__
#include "ot_common.h"
#include "ot_math.h"
#include "ot_buffer.h"
#include "securec.h"
#include "ot_mipi_rx.h"
#include "ot_mipi_tx.h"
#include "ot_common_sys.h"
#include "ot_common_isp.h"
#include "ot_common_vpss.h"
#include "ot_common_aio.h"
#include "ss_mpi_sys.h"
#include "ss_mpi_vpss.h"
#include "ss_mpi_mcf.h"
#include "ot_common_mcf_calibration.h"

typedef struct {
    td_char *input_file_name;
    ot_size input_img_size;
    td_char *output_file_name;
    ot_size output_img_size;
} sample_comm_mcf_scale_img_param;

typedef struct {
    td_u32 color_cnt;
    td_u32 mono_cnt;
} sample_yuv_cnt;

#define VI_MONO_PIPE    0
#define VI_COLOR_PIPE   1
#define MCF_VPSS_MAX_WIDTH   2688
#define MCF_VPSS_MAX_HEIGHT   1520

td_void sample_common_mcf_get_default_grp_attr(ot_mcf_grp_attr *grp_attr, ot_size *mono_size, ot_size *color_size);
td_s32 sample_common_vi_bind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp, ot_mcf_pipe mcf_pipe);
td_s32 sample_common_vi_unbind_mcf(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, ot_mcf_grp mcf_grp, ot_mcf_pipe mcf_pipe);
td_s32 sample_common_mcf_bind_vo(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn);
td_s32 sample_common_mcf_unbind_vo(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vo_layer vo_layer, ot_vo_chn vo_chn);
td_s32 sample_common_mcf_bind_venc(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_venc_chn venc_chn);
td_s32 sample_common_mcf_unbind_venc(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_venc_chn venc_chn);

td_s32 sample_common_mcf_start(ot_mcf_grp grp, const ot_mcf_grp_attr *grp_attr, ot_mcf_crop_info *grp_crop_info,
                               td_bool *mcf_chn_en, ot_size *chn_out_size);
td_void sample_common_mcf_get_default_chn_attr(ot_mcf_chn_attr *chn_attr, ot_size *out_size);

td_s32 sample_common_mcf_stop_vpss_one_grp(ot_vpss_grp grp);
td_s32 sample_common_mcf_stop_vpss(const ot_mcf_grp_attr *grp_attr);
td_s32 sample_common_mcf_stop(ot_mcf_grp grp, const td_bool *chn_enable, td_u32 chn_array_size);
td_s32 sample_common_mcf_start_vpss(const ot_mcf_grp_attr *mcf_grp_attr, td_u32 color_cnt, td_u32 mono_cnt,
    ot_fov_attr *fov_correction_attr);
td_s32 sample_common_mcf_bind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn, ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn);
td_s32 sample_common_mcf_unbind_vpss(ot_mcf_grp mcf_grp, ot_mcf_chn mcf_chn,
    ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn);
td_void sample_common_mcf_set_default_vpss_attr(ot_vpss_grp_attr *vpss_grp_attr,
                                                const ot_mcf_grp_attr *mcf_grp_attr, td_bool is_mono_pipe);
#endif
