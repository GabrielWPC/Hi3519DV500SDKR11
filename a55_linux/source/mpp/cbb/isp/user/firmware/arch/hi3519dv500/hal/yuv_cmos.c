/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_sensor.h"
#include "yuv_cmos_ex.h"
#include "securec.h"

static td_void yuv_cmos_get_isp_dng_default(ot_isp_cmos_default *sns_dft)
{
    (td_void)memcpy_s(&sns_dft->dng_color_param, sizeof(ot_isp_cmos_dng_color_param),
                      &g_yuv_dng_color_param, sizeof(ot_isp_cmos_dng_color_param));

    sns_dft->sns_mode.dng_raw_format.bits_per_sample = 12; /* bit depth 12 */
    sns_dft->sns_mode.dng_raw_format.white_level = 4095; /* white level 4095 */

    sns_dft->sns_mode.dng_raw_format.default_scale.default_scale_hor.denominator = 1;
    sns_dft->sns_mode.dng_raw_format.default_scale.default_scale_hor.numerator   = 1;
    sns_dft->sns_mode.dng_raw_format.default_scale.default_scale_ver.denominator = 1;
    sns_dft->sns_mode.dng_raw_format.default_scale.default_scale_ver.numerator   = 1;
    sns_dft->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_row = 2; /* dim row 2 */
    sns_dft->sns_mode.dng_raw_format.cfa_repeat_pattern_dim.repeat_pattern_dim_col = 2; /* dim column 2 */
    sns_dft->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_row = 2; /* row 2 */
    sns_dft->sns_mode.dng_raw_format.black_level_repeat_dim.repeat_col = 2; /* column 2 */
    sns_dft->sns_mode.dng_raw_format.cfa_layout = OT_ISP_CFALAYOUT_TYPE_RECTANGULAR;
    sns_dft->sns_mode.dng_raw_format.cfa_plane_color[0] = 0; /* array index 0 assignment 0 */
    sns_dft->sns_mode.dng_raw_format.cfa_plane_color[1] = 1; /* array index 1 assignment 1 */
    sns_dft->sns_mode.dng_raw_format.cfa_plane_color[2] = 2; /* array index 2 assignment 2 */
    sns_dft->sns_mode.dng_raw_format.cfa_pattern[0] = 0; /* array index 0 assignment 0 */
    sns_dft->sns_mode.dng_raw_format.cfa_pattern[1] = 1; /* array index 1 assignment 1 */
    sns_dft->sns_mode.dng_raw_format.cfa_pattern[2] = 1; /* array index 2 assignment 1 */
    sns_dft->sns_mode.dng_raw_format.cfa_pattern[3] = 2; /* array index 3 assignment 2 */
    sns_dft->sns_mode.valid_dng_raw_format = TD_TRUE;
}

td_s32 isp_get_yuv_default(ot_isp_cmos_default *sns_dft)
{
    isp_check_pointer_return(sns_dft);

    sns_dft->key.bit1_ca        = 0;
    sns_dft->ca                 = &g_yuv_isp_ca;
    sns_dft->key.bit1_clut      = 0;
    sns_dft->clut               = TD_NULL;
    sns_dft->key.bit1_wdr       = 0;
    sns_dft->wdr                = TD_NULL;
    sns_dft->key.bit1_dpc       = 0;
    sns_dft->dpc                = TD_NULL;

    sns_dft->key.bit1_lsc       = 0;
    sns_dft->lsc                = TD_NULL;

    sns_dft->key.bit1_demosaic  = 0;
    sns_dft->demosaic           = TD_NULL;
    sns_dft->key.bit1_drc       = 0;
    sns_dft->drc                = TD_NULL;
    sns_dft->key.bit1_gamma     = 0;
    sns_dft->gamma              = TD_NULL;
    sns_dft->key.bit1_bayer_nr  = 0;
    sns_dft->bayer_nr           = TD_NULL;
    sns_dft->key.bit1_sharpen   = 0;
    sns_dft->sharpen            = &g_yuv_isp_yuv_sharpen;
    sns_dft->key.bit1_ge        = 0;
    sns_dft->ge                 = TD_NULL;
    sns_dft->key.bit1_anti_false_color = 0;
    sns_dft->anti_false_color      = TD_NULL;
    sns_dft->key.bit1_ldci         = 0;
    sns_dft->ldci                  = &g_yuv_isp_ldci;

    sns_dft->sns_max_resolution.max_width  = 4056; /* max width 4056 */
    sns_dft->sns_max_resolution.max_height = 3040; /* max width 3040 */
    sns_dft->sns_mode.sns_id = 477; /* sensor id 477 */
    sns_dft->sns_mode.sns_mode = OT_WDR_MODE_NONE;

    yuv_cmos_get_isp_dng_default(sns_dft);

    return TD_SUCCESS;
}

