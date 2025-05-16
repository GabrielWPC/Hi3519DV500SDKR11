/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_ae_adp.h"
#include <stdio.h>
#include <string.h>
#include "ot_mpi_isp.h"
#include "isp_vreg.h"
#include "sample_ae_ext_config.h"
#include "sample_ae_mpi.h"

/* GLOBAL VARIABLES */
sample_ae_ctx_s g_ae_ctx_sample[OT_ISP_MAX_AE_LIB_NUM] = {{0}};
#define ae_get_ctx(handle)           (&g_ae_ctx_sample[handle])

td_s32 sample_ae_ext_regs_initialize(td_s32 handle)
{
    td_u8 id = ae_get_extreg_id(handle);

    /* set ext registers as a default value */
    ot_ext_system_ae_mode_write(id, OT_EXT_SYSTEM_AE_MODE_DEFAULT);

    return TD_SUCCESS;
}

td_s32 sample_ae_read_ext_regs(td_s32 handle)
{
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_u8 id = ae_get_extreg_id(handle);

    sample_ae_ctx = ae_get_ctx(handle);

    /* read the extregs to the global variables */
    sample_ae_ctx->ae_mode = ot_ext_system_ae_mode_read(id);

    return TD_SUCCESS;
}

td_s32 sample_ae_update_ext_regs(td_s32 handle)
{
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_u8 id = ae_get_extreg_id(handle);

    sample_ae_ctx = ae_get_ctx(handle);

    /* update the global variables to the extregs */
    td_u32 iso;

    iso = sample_ae_ctx->ae_result.iso;

    ot_ext_system_query_exposure_again_write(id, iso);

    return TD_SUCCESS;
}

td_s32 sample_ae_calculate(td_s32 handle)
{
    /* user's AE alg implementor */
    return TD_SUCCESS;
}

td_s32 sample_ae_isp_regs_init(td_s32 handle)
{
    ot_vi_pipe vi_pipe;
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    ot_isp_ae_stat_attr *sample_stat_attr = TD_NULL;

    td_u8 weight_table[OT_ISP_AE_ZONE_ROW][OT_ISP_AE_ZONE_COLUMN] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},
        {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };

    sample_ae_ctx = ae_get_ctx(handle);
    sample_stat_attr = &sample_ae_ctx->ae_result.stat_attr;

    vi_pipe = sample_ae_ctx->isp_bind_dev;

    /* change the ae zone's weight table and histogram thresh, although there is
     * default value in isp.
     */
    (td_void)memcpy_s(sample_stat_attr->weight_table[vi_pipe],
        OT_ISP_AE_ZONE_ROW * OT_ISP_AE_ZONE_COLUMN * sizeof(td_u8),
                      weight_table, OT_ISP_AE_ZONE_ROW * OT_ISP_AE_ZONE_COLUMN * sizeof(td_u8));

    sample_stat_attr->change = TD_TRUE;

    return TD_SUCCESS;
}

td_s32 sample_ae_init(td_s32 handle, const ot_isp_ae_param *ae_param)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe;
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_u8 id;

    sample_ae_check_handle_id_return(handle);
    sample_ae_ctx = ae_get_ctx(handle);
    id = ae_get_extreg_id(handle);
    vi_pipe = sample_ae_ctx->isp_bind_dev;

    sample_ae_check_pointer_return(ae_param);

    /* do something ... like check the sensor id, init the global variables, and so on */
    /* Commonly, create a virtual regs to communicate, also user can design the new commuincation style. */
    ret = vreg_init(vi_pipe, ae_lib_vreg_base(id), AE_VREG_SIZE);
    if (ret != TD_SUCCESS) {
        sample_ae_err_trace("Ae lib(%d) vreg init failed!\n", handle);
        return ret;
    }

    /* maybe you need to init the virtual regs. */
    ret = sample_ae_ext_regs_initialize(handle);
    if (ret != TD_SUCCESS) {
        sample_ae_err_trace("Ae lib(%d) ext regs init failed!\n", handle);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 sample_ae_run(td_s32 handle, const ot_isp_ae_info *ae_info,
                     ot_isp_ae_result *ae_result, td_s32 rsv)
{
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    ot_vi_pipe vi_pipe;

    sample_ae_check_handle_id_return(handle);
    sample_ae_ctx = ae_get_ctx(handle);
    vi_pipe = sample_ae_ctx->isp_bind_dev;

    sample_ae_check_pointer_return(ae_info);
    sample_ae_check_pointer_return(ae_result);

    sample_ae_check_pointer_return(ae_info->fe_ae_stat1);
    sample_ae_check_pointer_return(ae_info->fe_ae_stat2);
    sample_ae_check_pointer_return(ae_info->fe_ae_stat3);

    sample_ae_ctx->frame_cnt = ae_info->frame_cnt;

    /* init isp regs in the first frame. the regs in stStatAttr may need to be...
     * ...configured a few times.
     */
    if (sample_ae_ctx->frame_cnt == 1) {
        sample_ae_isp_regs_init(handle);
    }

    if (sample_ae_ctx->frame_cnt % 2 == 0) { /* do AE alg per 2 frame */
        /* record the statistics in ae_info, and then use the statistics...
         * ...to calculate, no need to call any other api
         */
        (td_void)memcpy_s(&sample_ae_ctx->ae_info, sizeof(ot_isp_ae_info), ae_info, sizeof(ot_isp_ae_info));

        /* maybe need to read the virtual regs to check whether...
         * ...someone changes the configs.
         */
        sample_ae_read_ext_regs(handle);

        sample_ae_calculate(handle);

        /* pls fill the result after calculate, the firmware will config...
         * ...the regs for awb algorithm.
         */
        sample_ae_ctx->ae_result.iso = 0x100;
        sample_ae_ctx->ae_result.isp_dgain = 0x100;

        /* maybe need to write some configs to the virtual regs. */
        sample_ae_update_ext_regs(handle);

        /* maybe need to call the sensor's register functions to config sensor */
        /* update sensor gains */ /* update sensor exposure time etc. */
        if (sample_ae_ctx->sns_register.sns_exp.pfn_cmos_gains_update != TD_NULL) {
            const td_u32 again = 0;
            const td_u32 dgain = 0;

            sample_ae_ctx->sns_register.sns_exp.pfn_cmos_gains_update(vi_pipe, again, dgain);
        }
    }

    /* record result */
    (td_void)memcpy_s(ae_result, sizeof(ot_isp_ae_result), &sample_ae_ctx->ae_result, sizeof(ot_isp_ae_result));

    sample_ae_ctx->ae_result.stat_attr.change = TD_FALSE;

    return TD_SUCCESS;
}

td_s32 sample_ae_ctrl(td_s32 handle, td_u32 cmd, td_void *value)
{
    sample_ae_check_handle_id_return(handle);

    sample_ae_check_pointer_return(value);

    switch (cmd) {
            /* system ctrl */
        case OT_ISP_WDR_MODE_SET:
            /* do something ... */
            break;
        case OT_ISP_AE_FPS_BASE_SET:
            /* do something ... */
            break;
        default:
            /* ae ctrl, define the customer's ctrl cmd, if needed ... */
            break;
    }

    return TD_SUCCESS;
}

td_s32 sample_ae_exit(td_s32 handle)
{
    td_s32 ret;
    ot_vi_pipe vi_pipe;
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_u8 id;

    sample_ae_check_handle_id_return(handle);
    sample_ae_ctx = ae_get_ctx(handle);
    id = ae_get_extreg_id(handle);
    vi_pipe = sample_ae_ctx->isp_bind_dev;

    /* do something ... */
    /* if created the virtual regs, need to destroy virtual regs. */
    ret = vreg_exit(vi_pipe, ae_lib_vreg_base(id), AE_VREG_SIZE);
    if (ret != TD_SUCCESS) {
        sample_ae_err_trace("Ae lib(%d) vreg exit failed!\n", handle);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_ae_register(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib)
{
    ot_isp_ae_register s_register = {0};
    td_s32 ret;
    td_s32 handle;
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;

    sample_ae_check_pipe_return(vi_pipe);
    sample_ae_check_pointer_return(ae_lib);
    sample_ae_check_handle_id_return(ae_lib->id);
    sample_ae_check_lib_name_return(ae_lib->lib_name);

    handle = ae_lib->id;
    sample_ae_ctx = ae_get_ctx(handle);
    sample_ae_ctx->isp_bind_dev = vi_pipe;

    s_register.ae_exp_func.pfn_ae_init  = sample_ae_init;
    s_register.ae_exp_func.pfn_ae_run   = sample_ae_run;
    s_register.ae_exp_func.pfn_ae_ctrl  = sample_ae_ctrl;
    s_register.ae_exp_func.pfn_ae_exit  = sample_ae_exit;
    s_register.ae_exp_func.pfn_thermo_run   = TD_NULL;
    ret = ot_mpi_isp_ae_lib_reg_callback(vi_pipe, ae_lib, &s_register);
    if (ret != TD_SUCCESS) {
        sample_ae_err_trace("OT_ae register failed!\n");
    }

    return ret;
}

td_s32 sample_ot_mpi_ae_unregister(ot_vi_pipe vi_pipe, const ot_isp_3a_alg_lib *ae_lib)
{
    td_s32 ret;

    sample_ae_check_pipe_return(vi_pipe);
    sample_ae_check_pointer_return(ae_lib);
    sample_ae_check_handle_id_return(ae_lib->id);
    sample_ae_check_lib_name_return(ae_lib->lib_name);

    ret = ot_mpi_isp_ae_lib_unreg_callback(vi_pipe, ae_lib);
    if (ret != TD_SUCCESS) {
        sample_ae_err_trace("OT_ae unregister failed!\n");
    }

    return ret;
}

td_s32 sample_ot_mpi_ae_sensor_reg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib,
    ot_isp_sns_attr_info *sns_attr_info, ot_isp_ae_sensor_register *s_register)
{
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_s32  handle;

    sample_ae_check_pipe_return(vi_pipe);
    sample_ae_check_pointer_return(ae_lib);
    sample_ae_check_pointer_return(s_register);
    sample_ae_check_pointer_return(sns_attr_info);

    handle = ae_lib->id;
    sample_ae_check_handle_id_return(handle);
    sample_ae_check_lib_name_return(ae_lib->lib_name);

    sample_ae_ctx = ae_get_ctx(handle);

    /* get default value */
    if (s_register->sns_exp.pfn_cmos_get_ae_default != TD_NULL) {
        s_register->sns_exp.pfn_cmos_get_ae_default(vi_pipe, &sample_ae_ctx->sns_def);
    }

    /* record information */
    (td_void)memcpy_s(&sample_ae_ctx->sns_register, sizeof(ot_isp_ae_sensor_register),
                      s_register, sizeof(ot_isp_ae_sensor_register));
    (td_void)memcpy_s(&sample_ae_ctx->sns_attr_info, sizeof(ot_isp_sns_attr_info),
                      sns_attr_info, sizeof(ot_isp_sns_attr_info));

    sample_ae_ctx->b_sns_register = TD_TRUE;

    return TD_SUCCESS;
}

td_s32 sample_ot_mpi_ae_sensor_unreg_callback(ot_vi_pipe vi_pipe, ot_isp_3a_alg_lib *ae_lib,
    ot_sensor_id sns_id)
{
    sample_ae_ctx_s *sample_ae_ctx = TD_NULL;
    td_s32  handle;

    sample_ae_check_pipe_return(vi_pipe);
    sample_ae_check_pointer_return(ae_lib);

    handle = ae_lib->id;
    sample_ae_check_handle_id_return(handle);
    sample_ae_check_lib_name_return(ae_lib->lib_name);

    sample_ae_ctx = ae_get_ctx(handle);

    (td_void)memset_s(&sample_ae_ctx->sns_def, sizeof(ot_isp_ae_sensor_default), 0, sizeof(ot_isp_ae_sensor_register));
    (td_void)memset_s(&sample_ae_ctx->sns_register, sizeof(ot_isp_ae_sensor_register),
                      0, sizeof(ot_isp_ae_sensor_register));
    sample_ae_ctx->sns_attr_info.sns_id = 0;
    sample_ae_ctx->b_sns_register = TD_FALSE;

    return TD_SUCCESS;
}
