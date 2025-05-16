/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef SAMPLE_AE_MPI_H
#define SAMPLE_AE_MPI_H

#include <string.h>
#include <stdio.h>

#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "ot_common_ae.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

td_s32 sample_ot_mpi_ae_register(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib);
td_s32 sample_ot_mpi_ae_unregister(ot_vi_pipe vi_pipe, const ot_isp_3a_alg_lib *ae_lib);

/* The callback function of sensor register to ae lib. */
td_s32 sample_ot_mpi_ae_sensor_reg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib,
    ot_isp_sns_attr_info *sns_attr_info, ot_isp_ae_sensor_register *s_register);
td_s32 sample_ot_mpi_ae_sensor_unreg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib,
    ot_sensor_id sns_id);

td_s32 sample_ot_mpi_isp_set_exposure_attr(ot_vi_pipe vi_pipe, const ot_isp_exposure_attr *exp_attr);
td_s32 sample_ot_mpi_isp_get_exposure_attr(ot_vi_pipe vi_pipe, ot_isp_exposure_attr *exp_attr);

td_s32 sample_ot_mpi_isp_set_wdr_exposure_attr(ot_vi_pipe vi_pipe,
    const ot_isp_wdr_exposure_attr *wdr_exp_attr);
td_s32 sample_ot_mpi_isp_get_wdr_exposure_attr(ot_vi_pipe vi_pipe,
    ot_isp_wdr_exposure_attr *wdr_exp_attr);

td_s32 sample_ot_mpi_isp_set_ae_route_attr(ot_vi_pipe vi_pipe, const ot_isp_ae_route *ae_route_attr);
td_s32 sample_ot_mpi_isp_get_ae_route_attr(ot_vi_pipe vi_pipe, ot_isp_ae_route *ae_route_attr);

td_s32 sample_ot_mpi_isp_query_exposure_info(ot_vi_pipe vi_pipe, ot_isp_exp_info *exp_info);

td_s32 sample_ot_mpi_isp_set_iris_attr(ot_vi_pipe vi_pipe, const ot_isp_iris_attr *iris_attr);
td_s32 sample_ot_mpi_isp_get_iris_attr(ot_vi_pipe vi_pipe, ot_isp_iris_attr *iris_attr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
