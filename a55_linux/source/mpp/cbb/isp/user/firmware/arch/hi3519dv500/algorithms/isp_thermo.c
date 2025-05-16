/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "isp_alg.h"
#include "isp_ext_config.h"
#include "ot_common_3a.h"
#include "isp_thermo.h"

#define OT_THERMO_WEIGHT_TABLE_WIDTH  OT_ISP_AE_ZONE_COLUMN
#define OT_THERMO_WEIGHT_TABLE_HEIGHT OT_ISP_AE_ZONE_ROW
#define OT_THERMO_ISO_MIN  100
#define OT_THERMO_ISPDGAIN_MAX 0x10000
#define OT_THERMO_ISPDGAIN_MIN 0x100
#define OT_THERMO_DC_MIN  0
#define OT_THERMO_DC_MAX  0x3FFF
#define OT_THERMO_WDR_EXP_RATIO_MIN  0x40   /* min expratio 1X */

static td_void thermo_result_check(isp_usr_ctx *isp_ctx, ot_isp_thermo_result *thermo_result)
{
    if (is_linear_mode(isp_ctx->sns_wdr_mode)) {
        if (thermo_result->int_time <= 0) {
            isp_err_trace("int_time 0 should not be less than 1, current is %u\n", thermo_result->int_time);
        }
    } else {
        isp_err_trace("thermo do not support wdr\n");
    }
    if (thermo_result->isp_dgain < OT_THERMO_ISPDGAIN_MIN || thermo_result->isp_dgain > OT_THERMO_ISPDGAIN_MAX) {
        isp_err_trace("isp_dgain within [%d %d] current is %u\n",
                      OT_THERMO_ISPDGAIN_MIN, OT_THERMO_ISPDGAIN_MAX, thermo_result->isp_dgain);
    }
    if (thermo_result->run_interval < 1) {
        isp_err_trace("thermo_run_interval should be bigger than 0 current is %u\n", thermo_result->run_interval);
    }
    if (thermo_result->iso < OT_THERMO_ISO_MIN) {
        isp_err_trace("thermo iso should be bigger than %d current is %u\n", OT_THERMO_ISO_MIN, thermo_result->iso);
    }
    if (thermo_result->dc > OT_THERMO_DC_MAX) {
        isp_err_trace("thermo dc should be within [%d %d],the current value is %u\n",
                      OT_THERMO_DC_MIN, OT_THERMO_DC_MAX, thermo_result->dc);
    }
}

td_void thermo_update_linkage(ot_isp_thermo_result *thermo_result,
    isp_linkage *linkage)
{
    linkage->blc_fix.enable = TD_TRUE;
    linkage->blc_fix.blc = thermo_result->dc;
    return;
}

static td_void thermo_update_stat_attr(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result)
{
    td_u32 i, j;
    td_u8 sensor_pattern_type;
    sensor_pattern_type = ot_ext_system_rggb_cfg_read(vi_pipe);
    ae_result->stat_attr.ae_be_sel = 1;
    ae_result->stat_attr.change = TD_FALSE;
    ae_result->stat_attr.hist_adjust = TD_FALSE;
    ae_result->stat_attr.mode_update = TD_FALSE;
    ae_result->stat_attr.wight_table_update = TD_FALSE;
    ae_result->stat_attr.four_plane_mode = 0;
    ae_result->stat_attr.hist_skip_x = 1;
    ae_result->stat_attr.hist_skip_y = 1;
    ae_result->stat_attr.hist_offset_x = ((sensor_pattern_type == OT_ISP_TOP_RGGB_START_R_GR_GB_B) ||
        (sensor_pattern_type == OT_ISP_TOP_RGGB_START_B_GB_GR_R)) ? 1 : 0;
    ae_result->stat_attr.hist_offset_y   = 0;
    ae_result->stat_attr.hist_mode = 0;
    ae_result->stat_attr.aver_mode = 0;
    ae_result->stat_attr.max_gain_mode = 0;
    for (i = 0; i < OT_THERMO_WEIGHT_TABLE_HEIGHT; i++) {
        for (j = 0; j < OT_THERMO_WEIGHT_TABLE_WIDTH; j++) {
            ae_result->stat_attr.weight_table[vi_pipe][i][j] = 1;
        }
    }
    return;
}

static td_void thermo_update_result(ot_vi_pipe vi_pipe, ot_isp_ae_result *ae_result,
    ot_isp_thermo_result *thermo_result, isp_linkage *linkage)
{
    td_s32 i;
    isp_usr_ctx *isp_ctx    = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    ae_result->ae_run_interval = thermo_result->run_interval;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        ae_result->wdr_gain[i] = 0x100;
    }
    for (i = 0; i < AE_INT_TIME_NUM; i++) {
        ae_result->int_time[i] = thermo_result->int_time;
    }
    ae_result->iso = thermo_result->iso;
    ae_result->again = 1024; // default 1024
    ae_result->dgain = 1024; // default 1024
    ae_result->isp_dgain = thermo_result->isp_dgain;
    ae_result->isp_dgain_sf = 0; /* isp dgain shift 8 */
    ae_result->again_sf = 0;
    ae_result->dgain_sf = 0;
    ae_result->iso_sf = 0;
    ae_result->piris_valid = 0;
    ae_result->piris_pos = 0;
    ae_result->piris_gain = 0;
    ae_result->fswdr_mode = OT_ISP_FSWDR_NORMAL_MODE;
    ae_result->sns_lhcg_exp_ratio = OT_THERMO_WDR_EXP_RATIO_MIN;
    ae_result->hmax_times = isp_ctx->sys_rect.width ;
    ae_result->vmax       = isp_ctx->sys_rect.height;

    // ae_stat_attr
    thermo_update_stat_attr(vi_pipe, ae_result);

    // update dcf
    ae_result->update_info.exposure_bias_value = 0;
    ae_result->update_info.white_balance = 0;
    ae_result->update_info.iso_speed_ratings   = 0;
    ae_result->update_info.exposure_program    = 0;
    ae_result->update_info.exposure_mode       = 0;
    ae_result->update_info.exposure_time       = 0;
    ae_result->update_info.max_aperture_value  = 0;
    ae_result->update_info.f_number            = 0;

    thermo_update_linkage(thermo_result, linkage);
    return;
}

td_s32  isp_thermo_run_callback(ot_vi_pipe vi_pipe,   const isp_lib_node *lib, const ot_isp_ae_info *ae_info,
    ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx    = TD_NULL;
    ot_isp_thermo_result thermo_result  = {0};
    isp_get_ctx(vi_pipe, isp_ctx);
    isp_linkage *linkage = &isp_ctx->linkage;
    isp_check_pointer_return(ae_result);
    isp_check_pointer_return(reg_cfg);
    isp_check_pointer_return(lib);
    isp_check_pointer_return(ae_info);

    ret = lib->ae_regsiter.ae_exp_func.pfn_thermo_run(lib->alg_lib.id, ae_info, &thermo_result, 0);
    thermo_result_check(isp_ctx, &thermo_result);
    if (ret != TD_SUCCESS) {
        isp_err_trace("WARNING!! ISP[%d] run thermo lib err!\n", vi_pipe);
    }
    thermo_update_result(vi_pipe, ae_result, &thermo_result, linkage);
    reg_cfg->alg_reg_cfg[0].ae_reg_cfg2.thermo_enable = TD_TRUE;

    return TD_SUCCESS;
}
