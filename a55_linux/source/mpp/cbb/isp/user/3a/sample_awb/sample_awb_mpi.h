/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef SAMPLE_AWB_MPI_H
#define SAMPLE_AWB_MPI_H

#include <string.h>
#include <stdio.h>

#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "ot_common_awb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

td_s32 sample_ot_mpi_awb_register(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib);
td_s32 sample_ot_mpi_awb_unregister(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib);

/* The callback function of sensor register to awb lib. */
td_s32 sample_ot_mpi_awb_sensor_reg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib,
    ot_isp_sns_attr_info *sns_attr_info, ot_isp_awb_sensor_register *awb_register);
td_s32 sample_ot_mpi_awb_sensor_unreg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib,
    ot_sensor_id sns_id);

td_s32 sample_ot_mpi_isp_set_awb_attr(ot_vi_pipe vi_pipe, const ot_isp_wb_attr *wb_attr);
td_s32 sample_ot_mpi_isp_get_wb_attr(ot_vi_pipe vi_pipe, ot_isp_wb_attr *wb_attr);

td_s32 sample_ot_mpi_isp_set_ccm_attr(ot_vi_pipe vi_pipe, const ot_isp_color_matrix_attr *ccm_attr);
td_s32 sample_ot_mpi_isp_get_ccm_attr(ot_vi_pipe vi_pipe, ot_isp_color_matrix_attr *ccm_attr);

td_s32 sample_ot_mpi_isp_set_saturation_attr(ot_vi_pipe vi_pipe, const ot_isp_saturation_attr *sat_attr);
td_s32 sample_ot_mpi_isp_get_saturation_attr(ot_vi_pipe vi_pipe, ot_isp_color_matrix_attr *sat_attr);

td_s32 sample_ot_mpi_isp_set_color_tone_attr(ot_vi_pipe vi_pipe, const ot_isp_color_tone_attr *color_tone_attr);
td_s32 sample_ot_mpi_isp_get_color_tone_attr(ot_vi_pipe vi_pipe, ot_isp_color_tone_attr *color_tone_attr);

td_s32 sample_ot_mpi_isp_query_wb_info(ot_vi_pipe vi_pipe, ot_isp_wb_info *wb_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif