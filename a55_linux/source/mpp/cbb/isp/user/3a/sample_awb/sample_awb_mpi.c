/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>

#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "sample_awb_ext_config.h"
#include "sample_awb_adp.h"

td_s32 sample_ot_mpi_isp_set_awb_attr(ot_vi_pipe vi_pipe, const ot_isp_wb_attr *wb_attr)
{
    ot_isp_3a_alg_lib awb_lib;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(wb_attr);

    awb_lib.id = 0;
    strncpy_s(awb_lib.lib_name, ALG_LIB_NAME_SIZE_MAX, "ot_awb_lib", sizeof("ot_awb_lib"));

    if (wb_attr->op_type >= OT_OP_MODE_BUTT) {
        printf("Invalid input of parameter enOpType!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_wb_type_write((td_u8)awb_lib.id, wb_attr->op_type);

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_wb_attr(ot_vi_pipe vi_pipe, ot_isp_wb_attr *wb_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_ccm_attr(ot_vi_pipe vi_pipe, const ot_isp_color_matrix_attr *ccm_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_ccm_attr(ot_vi_pipe vi_pipe, ot_isp_color_matrix_attr *ccm_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_saturation_attr(ot_vi_pipe vi_pipe, const ot_isp_saturation_attr *sat_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_saturation_attr(ot_vi_pipe vi_pipe, ot_isp_color_matrix_attr *sat_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_set_color_tone_attr(ot_vi_pipe vi_pipe, const ot_isp_color_tone_attr *color_tone_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_get_color_tone_attr(ot_vi_pipe vi_pipe, ot_isp_color_tone_attr *color_tone_attr)
{
    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_isp_query_wb_info(ot_vi_pipe vi_pipe, ot_isp_wb_info *wb_info)
{
    ot_isp_3a_alg_lib awb_lib;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(wb_info);

    awb_lib.id = 0;
    strncpy_s(awb_lib.lib_name, ALG_LIB_NAME_SIZE_MAX, "ot_awb_lib", sizeof("ot_awb_lib"));

    wb_info->color_temp = ot_ext_system_wb_detect_temp_read((td_u8)awb_lib.id);

    return TD_SUCCESS;
}
