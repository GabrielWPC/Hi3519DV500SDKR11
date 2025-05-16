/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_sensor.h"
#include "ot_common_sns.h"
#include "isp_math_utils.h"
#include "ot_osal.h"
#include "ot_debug.h"
#include "securec.h"

typedef struct {
    ot_isp_sns_attr_info    sns_attr_info;
    ot_isp_sns_register  sns_register;
    ot_isp_cmos_default     sns_dft;
    ot_isp_cmos_black_level sns_black_level;    /* some sensors's black level will be changed with iso */
    ot_isp_sns_regs_info    sns_reg_info;
    ot_isp_cmos_sns_image_mode sns_image_mode;
} isp_sensor_ctx;

isp_sensor_ctx *g_sensor_ctx[OT_ISP_MAX_PIPE_NUM] = { TD_NULL };

#define sensor_get_ctx(pipe, ctx) ((ctx) = g_sensor_ctx[pipe])
#define sensor_set_ctx(pipe, ctx) (g_sensor_ctx[pipe] = (ctx))
#define sensor_reset_ctx(pipe)    (g_sensor_ctx[pipe] = TD_NULL)

td_s32 isp_sensor_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);

    if (sensor_ctx == TD_NULL) {
        sensor_ctx = (isp_sensor_ctx *)isp_malloc(sizeof(isp_sensor_ctx));
        if (sensor_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] sensor_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(sensor_ctx, sizeof(isp_sensor_ctx), 0, sizeof(isp_sensor_ctx));

    sensor_set_ctx(vi_pipe, sensor_ctx);

    return TD_SUCCESS;
}

td_s32 isp_sensor_reg_callback(ot_vi_pipe vi_pipe, ot_isp_sns_attr_info *sns_attr_info,
                               const ot_isp_sns_register *sns_register)
{
    td_s32 ret;
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    ret = isp_sensor_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    (td_void)memcpy_s(&sensor_ctx->sns_attr_info, sizeof(ot_isp_sns_attr_info),
                      sns_attr_info, sizeof(ot_isp_sns_attr_info));
    (td_void)memcpy_s(&sensor_ctx->sns_register, sizeof(ot_isp_sns_register),
                      sns_register, sizeof(ot_isp_sns_register));

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_global_init != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_global_init(vi_pipe);
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_unreg_callback(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_free(sensor_ctx);
    sensor_reset_ctx(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_sensor_update_all(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;
    ot_isp_sns_exp_func *sns_exp = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);
    sns_exp = &sensor_ctx->sns_register.sns_exp;
    if (sns_exp->pfn_cmos_get_isp_default != TD_NULL) {
        sns_exp->pfn_cmos_get_isp_default(vi_pipe, &sensor_ctx->sns_dft);
    } else {
        isp_err_trace("Get isp[%d] default value error!\n", vi_pipe);
        return TD_FAILURE;
    }

    if (sns_exp->pfn_cmos_get_isp_black_level != TD_NULL) {
        sns_exp->pfn_cmos_get_isp_black_level(vi_pipe, &sensor_ctx->sns_black_level);
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_update_all_yuv(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    ret = isp_get_yuv_default(&sensor_ctx->sns_dft);

    sensor_ctx->sns_black_level.auto_attr.black_level[0][OT_ISP_CHN_R]  = 0x101;
    sensor_ctx->sns_black_level.auto_attr.black_level[0][OT_ISP_CHN_GR] = 0x101;
    sensor_ctx->sns_black_level.auto_attr.black_level[0][OT_ISP_CHN_GB] = 0x101;
    sensor_ctx->sns_black_level.auto_attr.black_level[0][OT_ISP_CHN_B]  = 0x101;
    sensor_ctx->sns_black_level.auto_attr.update = TD_TRUE;

    return ret;
}

td_s32 isp_sensor_get_id(ot_vi_pipe vi_pipe, ot_sensor_id *sns_id)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    *sns_id = sensor_ctx->sns_attr_info.sns_id;

    return TD_SUCCESS;
}

td_s32 isp_sensor_get_blc(ot_vi_pipe vi_pipe, ot_isp_cmos_black_level **sns_black_level)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    *sns_black_level = &sensor_ctx->sns_black_level;

    return TD_SUCCESS;
}

td_s32 isp_sensor_get_default(ot_vi_pipe vi_pipe, ot_isp_cmos_default **sns_dft)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    *sns_dft = &sensor_ctx->sns_dft;

    return TD_SUCCESS;
}

td_s32 isp_sensor_get_max_resolution(ot_vi_pipe vi_pipe, ot_isp_cmos_sns_max_resolution **sns_max_resolution)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    *sns_max_resolution = &sensor_ctx->sns_dft.sns_max_resolution;

    return TD_SUCCESS;
}

td_s32 isp_sensor_get_sns_reg(ot_vi_pipe vi_pipe, ot_isp_sns_regs_info **sns_reg_info)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    *sns_reg_info = &sensor_ctx->sns_reg_info;

    return TD_SUCCESS;
}

td_s32 isp_sensor_init(ot_vi_pipe vi_pipe)
{
    td_s8 ssp_dev;
    td_s32 ret;
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    /* if I2C or SSP Dev is -1, don't init sensor */
    {
        ot_isp_sns_regs_info *sns_regs_info = TD_NULL;

        ret = isp_sensor_update_sns_reg(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        ret = isp_sensor_get_sns_reg(vi_pipe, &sns_regs_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        ssp_dev = sns_regs_info->com_bus.ssp_dev.bit4_ssp_dev;

        if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_I2C) &&
            (sns_regs_info->com_bus.i2c_dev == -1)) {
            return TD_SUCCESS;
        }

        if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_SSP) &&
            (ssp_dev == -1)) {
            return TD_SUCCESS;
        }
    }

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_init != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_init(vi_pipe);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_switch(ot_vi_pipe vi_pipe)
{
    td_s8  ssp_dev;
    td_s32 ret;
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    /* if I2C or SSP Dev is -1, don't init sensor */
    {
        ot_isp_sns_regs_info *sns_regs_info = TD_NULL;

        ret = isp_sensor_update_sns_reg(vi_pipe);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        ret = isp_sensor_get_sns_reg(vi_pipe, &sns_regs_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        ssp_dev = sns_regs_info->com_bus.ssp_dev.bit4_ssp_dev;

        if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_I2C) &&
            (sns_regs_info->com_bus.i2c_dev == -1)) {
            return TD_SUCCESS;
        }

        if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_SSP) &&
            (ssp_dev == -1)) {
            return TD_SUCCESS;
        }
    }

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_init != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_init(vi_pipe);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void isp_sensor_exit(ot_vi_pipe vi_pipe)
{
    td_s8  ssp_dev;
    td_s32 ret;
    isp_sensor_ctx       *sensor_ctx    = TD_NULL;
    ot_isp_sns_regs_info *sns_regs_info = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_void_return(sensor_ctx);

    ret = isp_sensor_update_sns_reg(vi_pipe);
    if (ret != TD_SUCCESS) {
        return;
    }
    ret = isp_sensor_get_sns_reg(vi_pipe, &sns_regs_info);
    if (ret != TD_SUCCESS) {
        return;
    }
    ssp_dev = sns_regs_info->com_bus.ssp_dev.bit4_ssp_dev;

    if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_I2C) &&
        (sns_regs_info->com_bus.i2c_dev == -1)) {
        return;
    }

    if ((sns_regs_info->sns_type == OT_ISP_SNS_TYPE_SSP) &&
        (ssp_dev == -1)) {
        return;
    }

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_exit != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_sns_exit(vi_pipe);
    } else {
        isp_err_trace("pfn_cmos_sns_exit is null!\n");
    }
}

td_s32 isp_sensor_update_blc(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;
    ot_isp_sns_exp_func *sns_exp = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);
    sns_exp = &sensor_ctx->sns_register.sns_exp;

    if (sns_exp->pfn_cmos_get_isp_black_level != TD_NULL) {
        /* sensor should record the present iso, and calculate new black level. */
        sns_exp->pfn_cmos_get_isp_black_level(vi_pipe, &sensor_ctx->sns_black_level);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_update_default(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;
    ot_isp_sns_exp_func *sns_exp = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    sns_exp = &sensor_ctx->sns_register.sns_exp;

    if (sns_exp->pfn_cmos_get_isp_default != TD_NULL) {
        sns_exp->pfn_cmos_get_isp_default(vi_pipe, &sensor_ctx->sns_dft);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_set_wdr_mode(ot_vi_pipe vi_pipe, td_u8 mode)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_set_wdr_mode != TD_NULL) {
        if (sensor_ctx->sns_register.sns_exp.pfn_cmos_set_wdr_mode(vi_pipe, mode) != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_set_image_mode(ot_vi_pipe vi_pipe, ot_isp_cmos_sns_image_mode *sns_image_mode)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;
    ot_isp_sns_exp_func *sns_exp = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    sns_exp = &sensor_ctx->sns_register.sns_exp;

    if (sns_exp->pfn_cmos_set_image_mode != TD_NULL) {
        return sns_exp->pfn_cmos_set_image_mode(vi_pipe, sns_image_mode);
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_set_pixel_detect(ot_vi_pipe vi_pipe, td_bool enable)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_set_pixel_detect != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_set_pixel_detect(vi_pipe, enable);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_get_awb_gain(ot_vi_pipe vi_pipe, td_u32 *sns_awb_gain)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_get_awb_gains != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_get_awb_gains(vi_pipe, sns_awb_gain);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_update_blc_clamp_info(ot_vi_pipe vi_pipe, td_bool *clamp_en)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    if (sensor_ctx->sns_register.sns_exp.pfn_cmos_get_blc_clamp_info != TD_NULL) {
        sensor_ctx->sns_register.sns_exp.pfn_cmos_get_blc_clamp_info(vi_pipe, clamp_en);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_sensor_update_sns_reg(ot_vi_pipe vi_pipe)
{
    isp_sensor_ctx *sensor_ctx = TD_NULL;
    ot_isp_sns_exp_func *sns_exp = TD_NULL;

    sensor_get_ctx(vi_pipe, sensor_ctx);
    isp_check_pointer_return(sensor_ctx);

    sns_exp = &sensor_ctx->sns_register.sns_exp;

    if (sns_exp->pfn_cmos_get_sns_reg_info != TD_NULL) {
        sns_exp->pfn_cmos_get_sns_reg_info(vi_pipe, &sensor_ctx->sns_reg_info);
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}
