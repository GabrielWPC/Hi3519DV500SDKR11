/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "sample_ae_ext_config.h"
#include "sample_ae_adp.h"

td_s32 sample_ot_mpi_isp_set_exposure_attr(ot_vi_pipe vi_pipe,
    const ot_isp_exposure_attr *exp_attr)
{
    ot_isp_3a_alg_lib ae_lib;
    ae_lib.id = 4; /* 4: ae_lib_id */
    strncpy_s(ae_lib.lib_name, ALG_LIB_NAME_SIZE_MAX, "ot_ae_lib", sizeof("ot_ae_lib"));

    if (exp_attr->auto_attr.ae_mode >= OT_ISP_AE_MODE_BUTT) {
        sample_ae_err_trace("Invalid AE mode!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_ae_mode_write((td_u8)ae_lib.id, (td_u8)exp_attr->auto_attr.ae_mode);

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_exposure_attr(ot_vi_pipe vi_pipe, ot_isp_exposure_attr *exp_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_wdr_exposure_attr(ot_vi_pipe vi_pipe,
    const ot_isp_wdr_exposure_attr *wdr_exp_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_wdr_exposure_attr(ot_vi_pipe vi_pipe,
    ot_isp_wdr_exposure_attr *wdr_exp_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_route_check(td_u8 id, const ot_isp_ae_route *route)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_ae_route_attr(ot_vi_pipe vi_pipe, const ot_isp_ae_route *ae_route_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_ae_route_attr(ot_vi_pipe vi_pipe, ot_isp_ae_route *ae_route_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_query_exposure_info(ot_vi_pipe vi_pipe, ot_isp_exp_info *exp_info)
{
    ot_isp_3a_alg_lib ae_lib;
    ae_lib.id = 4; /* 4: ae_lib_id */
    strncpy_s(ae_lib.lib_name, ALG_LIB_NAME_SIZE_MAX, "ot_ae_lib", sizeof("ot_ae_lib"));

    exp_info->a_gain = ot_ext_system_query_exposure_again_read((td_u8)ae_lib.id);

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_iris_attr(ot_vi_pipe vi_pipe, const ot_isp_iris_attr *iris_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_iris_attr(ot_vi_pipe vi_pipe, ot_isp_iris_attr *iris_attr)
{
    return TD_SUCCESS;
}
