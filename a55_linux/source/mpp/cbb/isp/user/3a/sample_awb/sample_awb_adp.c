/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_awb_adp.h"
#include "sample_awb_ext_config.h"

#include <stdio.h>
#include <string.h>

#include "ot_mpi_isp.h"
#include "isp_vreg.h"


/* GLOBAL VARIABLES */
sample_awb_ctx_s g_sample_awb_ctx[OT_ISP_MAX_AWB_LIB_NUM] = {{0}};

/* we assumed that the different lib instance have different id,
 * use the id 0 & 1.
 */
static sample_awb_ctx_s *sample_awb_get_ctx(td_s32 handle)
{
    return (&g_sample_awb_ctx[handle]);
}

static td_u8 sample_awb_get_extreg_id(td_s32 handle)
{
    return ((handle == 0) ? 0x4 : 0x5);
}

td_s32 sample_awb_ext_regs_initialize(td_s32 handle)
{
    td_u8 id;
    id = sample_awb_get_extreg_id(handle);

    ot_ext_system_wb_type_write(id, OT_EXT_SYSTEM_WB_TYPE_DEFAULT);

    return TD_SUCCESS;
}

td_s32 sample_awb_read_ext_regs(td_s32 handle)
{
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    td_u8 id;
    td_u8 wb_type;

    awb_ctx = sample_awb_get_ctx(handle);
    id = sample_awb_get_extreg_id(handle);

    /* read the extregs to the global variables */
    wb_type = ot_ext_system_wb_type_read(id);

    awb_ctx->wb_type = wb_type;

    return TD_SUCCESS;
}

td_s32 sample_awb_update_ext_regs(td_s32 handle)
{
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    td_u8 id;

    awb_ctx = sample_awb_get_ctx(handle);
    id = sample_awb_get_extreg_id(handle);

    /* update the global variables to the extregs */
    ot_ext_system_wb_detect_temp_write(id, awb_ctx->detect_temp);

    return TD_SUCCESS;
}

td_s32 sample_awb_isp_regs_init(td_s32 handle)
{
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    ot_isp_awb_raw_stat_attr *raw_stat_attr = TD_NULL;
    td_s32 i;

    awb_ctx = sample_awb_get_ctx(handle);

    raw_stat_attr = &awb_ctx->awb_result.raw_stat_attr;
    raw_stat_attr->metering_white_level_awb = 0xFFFF;
    raw_stat_attr->metering_black_level_awb = 0x08;
    raw_stat_attr->metering_cr_ref_max_awb = 0x130;
    raw_stat_attr->metering_cr_ref_min_awb = 0x40;
    raw_stat_attr->metering_cb_ref_max_awb = 0x120;
    raw_stat_attr->metering_cb_ref_min_awb = 0x40;

    raw_stat_attr->stat_cfg_update  = TD_TRUE;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        awb_ctx->awb_result.white_balance_gain[i] =
            (td_u32)awb_ctx->sns_dft.gain_offset[i] << 8;  /* left shift 8 for result awb gain */
    }

    return TD_SUCCESS;
}

td_s32 sample_awb_calculate(td_s32 handle)
{
    /* user's AWB alg implementor */
    return TD_SUCCESS;
}

td_s32 sample_awb_init(td_s32 handle, const ot_isp_awb_param *awb_param, ot_isp_awb_result *awb_result)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe;
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    td_u8 id;

    sample_awb_check_handle_id_return(handle);
    awb_ctx = sample_awb_get_ctx(handle);
    id = sample_awb_get_extreg_id(handle);
    vi_pipe = awb_ctx->isp_bind_dev;

    sample_awb_check_pointer_return(awb_param);
    sample_awb_check_pointer_return(awb_result);

    /* do something ... like check the sensor id, init the global variables...
     * and so on
     */
    /* Commonly, create a virtual regs to communicate, also user can design...
     * the new commuincation style.
     */
    ret = vreg_init(vi_pipe, awb_lib_vreg_base(id), AWB_VREG_SIZE);
    if (ret != TD_SUCCESS) {
        sample_awb_err_trace("Awb lib(%d) vreg init failed!\n", handle);
        return ret;
    }

    ret = sample_awb_ext_regs_initialize(handle);

    /* maybe you need to init the virtual regs. */
    return ret;
}

td_s32 sample_awb_run(td_s32 handle, const ot_isp_awb_info *awb_info,
                      ot_isp_awb_result *awb_result, td_s32 rsv)
{
    td_s32  i;
    sample_awb_ctx_s *awb_ctx = TD_NULL;

    sample_awb_check_handle_id_return(handle);
    awb_ctx = sample_awb_get_ctx(handle);

    sample_awb_check_pointer_return(awb_info);
    sample_awb_check_pointer_return(awb_result);

    sample_awb_check_pointer_return(awb_info->awb_stat1);

    awb_ctx->frame_cnt = awb_info->frame_cnt;

    /* init isp regs in the first frame. the regs in stStatAttr may need to be configured a few times. */
    if (awb_ctx->frame_cnt == 1) {
        sample_awb_isp_regs_init(handle);
    }

    /* do something ... */
    if (awb_ctx->frame_cnt % 2 == 0) { /* 2 is awb run interval */
        /*
        record the statistics in awb_info, and then use the statistics to calculate,
           no need to call any other api
        */
        (td_void)memcpy_s(&awb_ctx->awb_info, sizeof(ot_isp_awb_info), awb_info, sizeof(ot_isp_awb_info));

        /* maybe need to read the virtual regs to check whether someone changes the configs. */
        sample_awb_read_ext_regs(handle);

        sample_awb_calculate(handle);

        /* maybe need to write some configs to the virtual regs. */
        sample_awb_update_ext_regs(handle);

        /* pls fill the result after calculate, the firmware will config the regs for awb algorithm. */
        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            awb_ctx->awb_result.color_matrix[i] = 0x0;
        }

        /* the unit of the result is not fixed, just map with the isp_awb.c, modify the unit together. */
        awb_ctx->awb_result.color_matrix[0] = 0x100;  /* 0 for RR */
        awb_ctx->awb_result.color_matrix[4] = 0x100;  /* 4 for GG */
        awb_ctx->awb_result.color_matrix[8] = 0x100;  /* 8 for BB */

        awb_ctx->awb_result.white_balance_gain[0] = 0x100 << 8;  /* (8 + 8) fractional precision. 0 for chn r */
        awb_ctx->awb_result.white_balance_gain[1] = 0x100 << 8; /* (8 + 8) fractional precision. 1 for chn gr */
        awb_ctx->awb_result.white_balance_gain[2] = 0x100 << 8; /* (8 + 8) fractional precision. 2 for chn gb */
        awb_ctx->awb_result.white_balance_gain[3] = 0x100 << 8;  /* (8 + 8) fractional precision. 3 for chn b */
    }

    /* record result */
    (td_void)memcpy_s(awb_result, sizeof(ot_isp_awb_result), &awb_ctx->awb_result, sizeof(ot_isp_awb_result));

    return TD_SUCCESS;
}

td_s32 sample_awb_ctrl(td_s32 handle, td_u32 cmd, td_void *value)
{
    sample_awb_check_handle_id_return(handle);

    sample_awb_check_pointer_return(value);

    switch (cmd) {
            /* system ctrl */
        case OT_ISP_WDR_MODE_SET:
            /* do something ... */
            break;
        case OT_ISP_AWB_ISO_SET:
            /* do something ... */
            break;
            /* awb ctrl, define the customer's ctrl cmd, if needed ... */
        default:
            break;
    }

    return TD_SUCCESS;
}

td_s32 sample_awb_exit(td_s32 handle)
{
    td_s32 ret;
    td_u8 id;
    ot_vi_pipe vi_pipe;
    sample_awb_ctx_s *awb_ctx = TD_NULL;

    sample_awb_check_handle_id_return(handle);
    awb_ctx = sample_awb_get_ctx(handle);
    id = sample_awb_get_extreg_id(handle);
    vi_pipe = awb_ctx->isp_bind_dev;

    /* do something ... */
    /* if created the virtual regs, need to destroy virtual regs. */
    ret = vreg_exit(vi_pipe, awb_lib_vreg_base(id), AWB_VREG_SIZE);
    if (ret != TD_SUCCESS) {
        sample_awb_err_trace("Awb lib(%d) vreg exit failed!\n", handle);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_awb_register(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib)
{
    ot_isp_awb_register awb_register;
    td_s32 ret;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(awb_lib);
    sample_awb_check_handle_id_return(awb_lib->id);
    sample_awb_check_lib_name_return(awb_lib->lib_name);

    awb_register.awb_exp_func.pfn_awb_init = sample_awb_init;
    awb_register.awb_exp_func.pfn_awb_run  = sample_awb_run;
    awb_register.awb_exp_func.pfn_awb_ctrl = sample_awb_ctrl;
    awb_register.awb_exp_func.pfn_awb_exit = sample_awb_exit;
    ret = ot_mpi_isp_awb_lib_reg_callback(vi_pipe, awb_lib, &awb_register);
    if (ret != TD_SUCCESS) {
        sample_awb_err_trace("OT_awb register failed!\n");
    }

    return ret;
}

td_s32 sample_ot_mpi_awb_unregister(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib)
{
    td_s32 ret;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(awb_lib);
    sample_awb_check_handle_id_return(awb_lib->id);
    sample_awb_check_lib_name_return(awb_lib->lib_name);

    ret = ot_mpi_isp_awb_lib_unreg_callback(vi_pipe, awb_lib);
    if (ret != TD_SUCCESS) {
        sample_awb_err_trace("OT_awb unregister failed!\n");
    }

    return ret;
}

td_s32 sample_ot_mpi_awb_sensor_reg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib,
    ot_isp_sns_attr_info *sns_attr_info, ot_isp_awb_sensor_register *awb_register)
{
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    td_s32  handle;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(awb_lib);
    sample_awb_check_pointer_return(awb_register);

    handle = awb_lib->id;
    sample_awb_check_handle_id_return(handle);
    sample_awb_check_lib_name_return(awb_lib->lib_name);

    awb_ctx = sample_awb_get_ctx(handle);

    if (awb_register->sns_exp.pfn_cmos_get_awb_default != TD_NULL) {
        awb_register->sns_exp.pfn_cmos_get_awb_default(vi_pipe, &awb_ctx->sns_dft);
    }

    (td_void)memcpy_s(&awb_ctx->sns_register, sizeof(ot_isp_awb_sensor_register),
                      awb_register, sizeof(ot_isp_awb_sensor_register));
    (td_void)memcpy_s(&awb_ctx->sns_attr_info, sizeof(ot_isp_sns_attr_info),
                      sns_attr_info, sizeof(ot_isp_sns_attr_info));

    awb_ctx->sns_register_flag = TD_TRUE;

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_awb_sensor_unreg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *awb_lib, ot_sensor_id sns_id)
{
    sample_awb_ctx_s *awb_ctx = TD_NULL;
    td_s32  handle;

    sample_awb_check_pipe_return(vi_pipe);
    sample_awb_check_pointer_return(awb_lib);

    handle = awb_lib->id;
    sample_awb_check_handle_id_return(handle);
    sample_awb_check_lib_name_return(awb_lib->lib_name);

    awb_ctx = sample_awb_get_ctx(handle);

    (td_void)memset_s(&awb_ctx->sns_dft, sizeof(ot_isp_awb_sensor_default), 0, sizeof(ot_isp_awb_sensor_default));
    (td_void)memset_s(&awb_ctx->sns_register, sizeof(ot_isp_awb_sensor_register),
                      0, sizeof(ot_isp_awb_sensor_register));
    awb_ctx->sns_attr_info.sns_id = 0;
    awb_ctx->sns_register_flag = TD_FALSE;

    return TD_SUCCESS;
}
