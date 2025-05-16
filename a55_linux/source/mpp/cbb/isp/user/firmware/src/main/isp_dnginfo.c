/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_dnginfo.h"
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "ot_mpi_sys_mem.h"
#include "isp_sensor.h"
#include "isp_main.h"
#include "isp_alg.h"
#include "isp_ext_config.h"

td_s32 isp_dng_info_init(ot_vi_pipe vi_pipe)
{
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    phy_addr = isp_ctx->isp_trans_info.dng_info.phy_addr;

    isp_ctx->dng_info_ctrl.isp_dng = ot_mpi_sys_mmap(phy_addr, sizeof(ot_isp_dng_image_static_info));

    if (isp_ctx->dng_info_ctrl.isp_dng == TD_NULL) {
        isp_err_trace("isp[%d] mmap Dng info buf failed!\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    return TD_SUCCESS;
}

td_void isp_dng_info_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->dng_info_ctrl.isp_dng != TD_NULL) {
        ot_mpi_sys_munmap(isp_ctx->dng_info_ctrl.isp_dng, sizeof(ot_isp_dng_image_static_info));
        isp_ctx->dng_info_ctrl.isp_dng = TD_NULL;
    }
}

static td_bool isp_dng_color_param_check(isp_dng_ccm *dng_ccm, isp_dng_ccm *pre_dng_ccm)
{
    td_bool changed = TD_FALSE;
    td_u8 i;

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        if (dng_ccm->low_ccm[i] != pre_dng_ccm->low_ccm[i]) {
            changed = TD_TRUE;
            return changed;
        }
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        if (dng_ccm->high_ccm[i] != pre_dng_ccm->high_ccm[i]) {
            changed = TD_TRUE;
            return changed;
        }
    }

    if (dng_ccm->high_color_temp != pre_dng_ccm->high_color_temp) {
        changed = TD_TRUE;
        return changed;
    }

    if (dng_ccm->low_color_temp != pre_dng_ccm->low_color_temp) {
        changed = TD_TRUE;
        return changed;
    }

    return changed;
}

static void isp_ccm_data_format(td_u16 *ccm_in, td_double *pd_ccm_out, td_u8 array_length)
{
    td_u8 i;
    td_s16 tmp;
    for (i = 0; i < array_length; i++) {
        tmp = (td_s16)ccm_in[i];
        pd_ccm_out[i] = (td_double)tmp / 256; /* const value 256 */
    }
}

static const double g_k_near_zero = 1.0E-10;

static const double g_srgb_to_xyzd50[OT_ISP_CCM_MATRIX_SIZE] = {
    0.4361, 0.3851, 0.1431, 0.2225, 0.7169, 0.0606, 0.0139, 0.0971, 0.7141
};
static const double g_srgb_to_xyza[OT_ISP_CCM_MATRIX_SIZE] = {
    0.4969, 0.4388, 0.1630, 0.2225, 0.7169, 0.0606, 0.0060, 0.0419, 0.3080
};

static const double g_xyzd50_to_srgb[OT_ISP_CCM_MATRIX_SIZE] = {
    3.1340, -1.6169, -0.4907, -0.9784, 1.9159, 0.0334, 0.0720, -0.2290, 1.4049
};

static const td_double g_ad_noise_profile[OT_DNG_NP_SIZE] = { 2.0E-5, 4.5E-7, 2.0E-5, 4.5E-7, 2.0E-5, 4.5E-7 };

static double abs_double(double x)
{
    return ((x < 0.0) ? -x : x);
}

static void invert_3by3(const double *matrix_a, double *inv_matrix, td_u8  array_length)
{
    double a00 = matrix_a[0];   /* array index 0 */
    double a01 = matrix_a[1];   /* array index 1 */
    double a02 = matrix_a[2];   /* array index 2 */
    double a10 = matrix_a[3];   /* array index 3 */
    double a11 = matrix_a[4];   /* array index 4 */
    double a12 = matrix_a[5];   /* array index 5 */
    double a20 = matrix_a[6];   /* array index 6 */
    double a21 = matrix_a[7];   /* array index 7 */
    double a22 = matrix_a[8];   /* array index 8 */
    double temp[OT_ISP_CCM_MATRIX_SIZE];
    double det;
    int i;

    temp[0] = a11 * a22 - a21 * a12;    /* array index 0 */
    temp[1] = a21 * a02 - a01 * a22;    /* array index 1 */
    temp[2] = a01 * a12 - a11 * a02;    /* array index 2 */
    temp[3] = a20 * a12 - a10 * a22;    /* array index 3 */
    temp[4] = a00 * a22 - a20 * a02;    /* array index 4 */
    temp[5] = a10 * a02 - a00 * a12;    /* array index 5 */
    temp[6] = a10 * a21 - a20 * a11;    /* array index 6 */
    temp[7] = a20 * a01 - a00 * a21;    /* array index 7 */
    temp[8] = a00 * a11 - a10 * a01;    /* array index 8 */

    det = (a00 * temp[0] +      /* array index 0 */
           a01 * temp[3] +      /* array index 3 */
           a02 * temp[6]);      /* array index 6 */

    if (abs_double(det) < g_k_near_zero) {
        return;
    }

    for (i = 0; i < array_length; i++) {
        inv_matrix[i] = temp[i] / det;
    }

    return;
}

static void multi_matrix_3x3(const double *matrix_a, const double *matrix_b, double *mut_matrix, td_u8 array_length)
{
    int i, j, k;
    td_u8 row, column;
    row = 3;  /* 3: row num */
    column = array_length / row;

    for (i = 0; i < row; ++i) {       /* 3: row num */
        for (j = 0; j < column; ++j) {   /* 3: column num */
            double temp = 0;

            for (k = 0; k < 3; ++k) {   /* 3: column num */
                temp += matrix_a[i * 3 + k] * matrix_b[k * 3 + j]; /* 3: column num */
            }
            mut_matrix[i * 3 + j] = temp;   /* 3: column num */
        }
    }
}

static void isp_dng_light_source_check(td_u16 color_tmep, td_u8 *light_source)
{
    /*  light source, actually this means white balance setting.
     *  '0' means unknown, '1' daylight, '2' fluorescent, '3' tungsten, '10' flash, '17' standard light A,
     *  '18' standard light B, '19' standard light C, '20' D55, '21' D65, '22' D75, '255' other
     */
    if (color_tmep >= (7500 - 500)) { /* D75 7500 */
        *light_source = 22; /* '22' D75 */
    } else if ((color_tmep < (6500 + 500)) && (color_tmep >= (6500 - 500))) { /* [6500 - 500, 6500 + 500) */
        *light_source = 21; /* '21' D65 */
    } else if ((color_tmep < (5500 + 500)) && (color_tmep >= (5500 - 250))) { /* [5500 - 250, 5500 + 500) */
        *light_source = 20; /* '20' D55 */
    } else if ((color_tmep < (5000 + 250)) && (color_tmep >= (5000 - 100))) { /* [5000 - 100, 5000 + 250) */
        *light_source = 23; /* '23' D50(daylight 5000) */
    } else if ((color_tmep < (4800 + 100)) && (color_tmep >= (4800 - 550))) { /* [4800 - 550, 4800 + 100)  */
        *light_source = 18; /* '18' standard light B(4800) */
    } else if ((color_tmep < (4000 + 250)) && (color_tmep >= (4000 - 800))) { /* [4000 - 800, 4000 + 250) */
        *light_source = 2;  /* '2' fluorescent 4000 */
    } else if (color_tmep < (2800 + 400)) { /* [0, 2800 + 400) */
        *light_source = 17; /* '17' standard light A 2800 */
    }
}

static td_void isp_dng_ext_read(ot_vi_pipe vi_pipe, ot_isp_dng_color_param *dng_color_param)
{
    td_u32 i;
    dng_color_param->wb_gain1.r_gain = ot_ext_system_dng_low_wb_gain_r_read(vi_pipe);
    dng_color_param->wb_gain1.g_gain = ot_ext_system_dng_low_wb_gain_g_read(vi_pipe);
    dng_color_param->wb_gain1.b_gain = ot_ext_system_dng_low_wb_gain_b_read(vi_pipe);

    dng_color_param->wb_gain2.r_gain = ot_ext_system_dng_high_wb_gain_r_read(vi_pipe);
    dng_color_param->wb_gain2.g_gain = ot_ext_system_dng_high_wb_gain_g_read(vi_pipe);
    dng_color_param->wb_gain2.b_gain = ot_ext_system_dng_high_wb_gain_b_read(vi_pipe);

    dng_color_param->ccm_tab1.color_temp = ot_ext_system_dng_low_temp_read(vi_pipe);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        dng_color_param->ccm_tab1.ccm[i] = ot_ext_system_dng_low_ccm_read(vi_pipe, i);
    }

    dng_color_param->ccm_tab2.color_temp = ot_ext_system_dng_high_temp_read(vi_pipe);
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        dng_color_param->ccm_tab2.ccm[i] = ot_ext_system_dng_high_ccm_read(vi_pipe, i);
    }
}

static td_void isp_dng_a_matrix_calc(ot_isp_dng_color_param *dng_color_param, isp_dng_ccm *dng_ccm,
    td_double *ad_a_color_matrix, td_double *ad_a_forward_matrix, td_u8 array_length)
{
    td_u16 ot_a_wbgain[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_ot_a_ccm[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_ot_a_wbgain[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_a_mult_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_inv_a_color_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };

    ot_a_wbgain[0] = dng_color_param->wb_gain1.r_gain;   /* array index 0 */
    ot_a_wbgain[4] = dng_color_param->wb_gain1.g_gain;   /* array index 4 */
    ot_a_wbgain[8] = dng_color_param->wb_gain1.b_gain;   /* array index 8 */

    isp_ccm_data_format(dng_ccm->low_ccm, ad_ot_a_ccm, array_length);
    isp_ccm_data_format(ot_a_wbgain, ad_ot_a_wbgain, array_length);

    /* calculate color_matrix1 */
    multi_matrix_3x3(ad_ot_a_ccm, ad_ot_a_wbgain, ad_a_mult_matrix, array_length);
    multi_matrix_3x3(g_srgb_to_xyza, ad_a_mult_matrix, ad_inv_a_color_matrix, array_length);
    invert_3by3(ad_inv_a_color_matrix, ad_a_color_matrix, array_length);

    /* calculate forward_matrix1 */
    invert_3by3(g_xyzd50_to_srgb, ad_inv_a_color_matrix, array_length);
    multi_matrix_3x3(ad_inv_a_color_matrix, ad_ot_a_ccm, ad_a_forward_matrix, array_length);
}

static td_void isp_dng_d50_matrix_calc(ot_isp_dng_color_param *dng_color_param, isp_dng_ccm *dng_ccm,
    td_double *ad_d50_color_matrix, td_double *ad_d50_forward_matrix, td_u8 array_length)
{
    td_u16 ot_d50_wbgain[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_ot_d50_ccm[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_ot_d50_wbgain[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_d50_mult_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_inv_d50_color_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };

    ot_d50_wbgain[0] = dng_color_param->wb_gain2.r_gain; /* array index 0 */
    ot_d50_wbgain[4] = dng_color_param->wb_gain2.g_gain; /* array index 4 */
    ot_d50_wbgain[8] = dng_color_param->wb_gain2.b_gain; /* array index 8 */

    isp_ccm_data_format(dng_ccm->high_ccm, ad_ot_d50_ccm, array_length);
    isp_ccm_data_format(ot_d50_wbgain, ad_ot_d50_wbgain, array_length);

    /* calculate color_matrix2 */
    multi_matrix_3x3(ad_ot_d50_ccm, ad_ot_d50_wbgain, ad_d50_mult_matrix, array_length);
    multi_matrix_3x3(g_srgb_to_xyzd50, ad_d50_mult_matrix, ad_inv_d50_color_matrix, array_length);
    invert_3by3(ad_inv_d50_color_matrix, ad_d50_color_matrix, array_length);

    /* calculate forward_matrix2 */
    invert_3by3(g_xyzd50_to_srgb, ad_inv_d50_color_matrix, array_length);
    multi_matrix_3x3(ad_inv_d50_color_matrix, ad_ot_d50_ccm, ad_d50_forward_matrix, array_length);
}

static td_void isp_update_dng_color_param(ot_isp_dng_image_static_info *isp_dng,
                                          ot_isp_dng_color_param *dng_color_param, isp_dng_ccm *dng_ccm)
{
    td_u8 i, array_length;
    td_double ad_a_color_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_d50_color_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_a_forward_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    td_double ad_d50_forward_matrix[OT_ISP_CCM_MATRIX_SIZE] = { 0 };
    array_length = OT_ISP_CCM_MATRIX_SIZE;
    isp_dng_a_matrix_calc(dng_color_param, dng_ccm, ad_a_color_matrix, ad_a_forward_matrix, array_length);
    isp_dng_d50_matrix_calc(dng_color_param, dng_ccm, ad_d50_color_matrix, ad_d50_forward_matrix, array_length);

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        isp_dng->color_matrix1[i].numerator = (td_s32)(ad_a_color_matrix[i] * 1000000); /* const value 1000000 */
        isp_dng->color_matrix1[i].denominator = 1000000;        /* const value 1000000 */

        isp_dng->color_matrix2[i].numerator = (td_s32)(ad_d50_color_matrix[i] * 1000000);   /* const value 1000000 */
        isp_dng->color_matrix2[i].denominator = 1000000;        /* const value 1000000 */

        isp_dng->forwad_matrix1[i].numerator = (td_s32)(ad_a_forward_matrix[i] * 1000000);  /* const value 1000000 */
        isp_dng->forwad_matrix1[i].denominator = 1000000;       /* const value 1000000 */

        isp_dng->forwad_matrix2[i].numerator = (td_s32)(ad_d50_forward_matrix[i] * 1000000);  /* const value 1000000 */
        isp_dng->forwad_matrix2[i].denominator = 1000000;       /* const value 1000000 */

        isp_dng->camera_calibration1[i].numerator = 0;
        isp_dng->camera_calibration1[i].denominator = 1000000;  /* const value 1000000 */

        isp_dng->camera_calibration2[i].numerator = 0;
        isp_dng->camera_calibration2[i].denominator = 1000000;  /* const value 1000000 */
    }
}

static td_void isp_dng_color_param_update(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx)
{
    isp_dng_ccm dng_ccm = { 0 };
    ot_isp_dng_color_param dng_color_param = { 0 };
    td_bool changed;

    isp_dng_ext_read(vi_pipe, &dng_color_param);

    /* DNG color param have not set */
    if (dng_color_param.wb_gain1.g_gain == 0) {
        return;
    }

    (td_void)memcpy_s(dng_ccm.low_ccm, sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE,
                      dng_color_param.ccm_tab1.ccm, sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE);
    (td_void)memcpy_s(dng_ccm.high_ccm, sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE,
                      dng_color_param.ccm_tab2.ccm, sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE);
    dng_ccm.high_color_temp = dng_color_param.ccm_tab2.color_temp;
    dng_ccm.low_color_temp  = dng_color_param.ccm_tab1.color_temp;

    /* if CCM or WB gain changed, recaculate color parameters */
    changed = isp_dng_color_param_check(&dng_ccm, &isp_ctx->pre_dng_ccm);
    if (!changed) {
        if ((isp_ctx->pre_dng_color_param.wb_gain1.r_gain != dng_color_param.wb_gain1.r_gain) ||
            (isp_ctx->pre_dng_color_param.wb_gain1.g_gain != dng_color_param.wb_gain1.g_gain) ||
            (isp_ctx->pre_dng_color_param.wb_gain1.b_gain != dng_color_param.wb_gain1.b_gain) ||
            (isp_ctx->pre_dng_color_param.wb_gain2.r_gain != dng_color_param.wb_gain2.r_gain) ||
            (isp_ctx->pre_dng_color_param.wb_gain2.g_gain != dng_color_param.wb_gain2.g_gain) ||
            (isp_ctx->pre_dng_color_param.wb_gain2.b_gain != dng_color_param.wb_gain2.b_gain)) {
            changed = TD_TRUE;
        }
    }
    /* save last CCM and WB gain */
    (td_void)memcpy_s(&isp_ctx->pre_dng_ccm, sizeof(isp_dng_ccm), &dng_ccm, sizeof(isp_dng_ccm));
    (td_void)memcpy_s(&isp_ctx->pre_dng_color_param, sizeof(ot_isp_dng_color_param),
                      &dng_color_param, sizeof(ot_isp_dng_color_param));

    if (changed == TD_TRUE) {
        isp_update_dng_color_param(isp_ctx->dng_info_ctrl.isp_dng, &dng_color_param, &dng_ccm);
        /* data format */
        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration1[0].numerator = 1000000; /* index0, const value 1000000 */
        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration1[4].numerator = 1000000; /* index4, const value 1000000 */
        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration1[8].numerator = 1000000; /* index8, const value 1000000 */

        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration2[0].numerator = 1000000; /* index0, const value 1000000 */
        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration2[4].numerator = 1000000; /* index4, const value 1000000 */
        isp_ctx->dng_info_ctrl.isp_dng->camera_calibration2[8].numerator = 1000000; /* index8, const value 1000000 */

        isp_dng_light_source_check(dng_ccm.low_color_temp, &isp_ctx->dng_info_ctrl.isp_dng->calibration_illuminant1);
        isp_dng_light_source_check(dng_ccm.high_color_temp, &isp_ctx->dng_info_ctrl.isp_dng->calibration_illuminant2);
    }
}

td_s32 isp_update_dng_image_dynamic_info(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    ot_isp_cmos_black_level *sns_black_level = TD_NULL;
    ot_dng_image_dynamic_info dng_dyna_info;
    td_u8 i;
    td_s32 ret;

    isp_get_ctx(vi_pipe, isp_ctx);

    isp_sensor_get_blc(vi_pipe, &sns_black_level);

    if (sns_black_level->auto_attr.update == TD_TRUE) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            dng_dyna_info.black_level[i] = sns_black_level->auto_attr.black_level[0][i];
        }
    } else {
        dng_dyna_info.black_level[OT_ISP_CHN_R]  = ot_ext_system_black_level_query_f0_r_read(vi_pipe);
        dng_dyna_info.black_level[OT_ISP_CHN_GR] = ot_ext_system_black_level_query_f0_gr_read(vi_pipe);
        dng_dyna_info.black_level[OT_ISP_CHN_GB] = ot_ext_system_black_level_query_f0_gb_read(vi_pipe);
        dng_dyna_info.black_level[OT_ISP_CHN_B]  = ot_ext_system_black_level_query_f0_b_read(vi_pipe);
    }

    dng_dyna_info.as_shot_neutral[0].denominator = MAX2(isp_ctx->linkage.white_balance_gain[0], 1);
    dng_dyna_info.as_shot_neutral[1].denominator = MAX2(isp_ctx->linkage.white_balance_gain[1], 1);
    dng_dyna_info.as_shot_neutral[2].denominator = MAX2(isp_ctx->linkage.white_balance_gain[3], 1); /* index 2&3 */

    for (i = 0; i < OT_CFACOLORPLANE; i++) {
        dng_dyna_info.as_shot_neutral[i].numerator = isp_ctx->linkage.white_balance_gain[OT_ISP_CHN_GR];
    }

    for (i = 0; i < OT_DNG_NP_SIZE; i++) {
        dng_dyna_info.ad_noise_profile[i] = g_ad_noise_profile[i];
    }

    isp_dng_color_param_update(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_DNG_INFO_SET, &dng_dyna_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}
