/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AI3DNR_MCF_COMMON_H
#define SAMPLE_AI3DNR_MCF_COMMON_H

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
#include "sample_comm.h"

td_void sample_aiisp_mcf_get_default_grp_attr(ot_mcf_grp_attr *grp_attr, ot_size *mono_size,
    ot_size *color_size);
td_void sample_aiisp_mcf_get_large_size(ot_size mono_size, ot_size color_size, ot_size *large_size);
td_void sample_aiisp_mcf_set_chn_defualt_outsize(ot_size out_size);
td_s32 sample_aiisp_mcf_start_vi(const ot_mcf_grp_attr *mcf_grp_attr, sample_vi_cfg *vi_cfg1,
    sample_vi_cfg *vi_cfg0, ot_size *in_size);
td_void sample_aiisp_mcf_stop_vi(ot_vi_pipe vi_color_pipe, sample_vi_cfg *vi_cfg1, sample_vi_cfg *vi_cfg0);
td_s32 sample_aiisp_start_mcf_data_flow(ot_mcf_grp mcf_grp, ot_mcf_grp_attr *mcf_grp_attr,
    ot_size *out_size, ot_mcf_crop_info *grp_crop_info);
td_void sample_aiisp_stop_mcf_data_flow(ot_mcf_grp mcf_grp, ot_mcf_grp_attr *mcf_grp_attr);
#endif
