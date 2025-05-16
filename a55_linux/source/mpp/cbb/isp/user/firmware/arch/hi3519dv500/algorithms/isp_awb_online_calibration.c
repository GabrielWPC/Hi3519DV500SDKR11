/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "ot_common.h"
#include "ot_common_isp.h"
#include "ot_isp_debug.h"
#include "ot_type.h"
#include "isp_math_utils.h"
#include "ot_mpi_isp.h"

#define AWB_CENTER_ROW_MIN  4
#define AWB_CENTER_COL_MIN  4

static td_void isp_awb_calc_center_gain(ot_isp_stats_cfg *stat_cfg, ot_isp_wb_stats *wb_stat,
                                        td_u16 *avg_r_gain, td_u16 *avg_b_gain)
{
    td_u32 i, j;
    td_u32 rsum = 0;
    td_u32 bsum = 0;
    td_u32 gsum = 0;
    td_u32 zone_col, zone_row;
    td_u32 zone_col_start, zone_col_end, zone_row_start, zone_row_end;

    zone_col = stat_cfg->wb_cfg.zone_col;
    zone_row = stat_cfg->wb_cfg.zone_row;
    zone_col_start = (zone_col >> 1) - (AWB_CENTER_COL_MIN >> 1);
    zone_col_end   = (zone_col >> 1) + (AWB_CENTER_COL_MIN >> 1);
    zone_row_start = (zone_row >> 1) - (AWB_CENTER_ROW_MIN >> 1);
    zone_row_end   = (zone_row >> 1) + (AWB_CENTER_ROW_MIN >> 1);

    /* get_statistics */
    for (j = zone_row_start; j < zone_row_end; j++) {
        for (i = j * zone_col + zone_col_start; i < j * zone_col + zone_col_end; i++) {
            rsum += wb_stat->zone_avg_r[i];
            bsum += wb_stat->zone_avg_b[i];
            gsum += wb_stat->zone_avg_g[i];
        }
    }

    *avg_r_gain = (td_u16)((gsum * 256) / div_0_to_1(rsum)); /* (G * 256 / R) */
    *avg_b_gain = (td_u16)((gsum * 256) / div_0_to_1(bsum)); /* (G * 256 / B) */
}

td_s32 isp_get_lightbox_gain(ot_vi_pipe vi_pipe, ot_isp_awb_calibration_gain *awb_calibration_gain)
{
    td_s32 ret;
    ot_isp_wb_stats  *wb_stat  = TD_NULL;
    ot_isp_stats_cfg *stat_cfg = TD_NULL;

    wb_stat = (ot_isp_wb_stats *)isp_malloc(sizeof(ot_isp_wb_stats));
    if (wb_stat == TD_NULL) {
        isp_err_trace("wb_stat malloc failure !\n");
        return OT_ERR_ISP_NOMEM;
    }
    stat_cfg = (ot_isp_stats_cfg *)isp_malloc(sizeof(ot_isp_stats_cfg));
    if (stat_cfg == TD_NULL) {
        isp_err_trace("wb_stat malloc failure !\n");
        isp_free(wb_stat);

        return OT_ERR_ISP_NOMEM;
    }
    ret = ot_mpi_isp_get_stats_cfg(vi_pipe, stat_cfg);
    if (ret != TD_SUCCESS) {
        isp_free(wb_stat);
        isp_free(stat_cfg);

        return ret;
    }

    if ((stat_cfg->wb_cfg.zone_col < AWB_CENTER_COL_MIN) || (stat_cfg->wb_cfg.zone_row < AWB_CENTER_ROW_MIN)) {
        isp_err_trace("Not support zone number less than 4 x 4 !\n");
        isp_free(wb_stat);
        isp_free(stat_cfg);

        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    ret = ot_mpi_isp_get_wb_stats(vi_pipe, wb_stat);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Get WB statics failed!\n");
        isp_free(wb_stat);
        isp_free(stat_cfg);

        return ret;
    }

    isp_awb_calc_center_gain(stat_cfg, wb_stat, &awb_calibration_gain->avg_r_gain, &awb_calibration_gain->avg_b_gain);

    isp_free(wb_stat);
    isp_free(stat_cfg);

    return TD_SUCCESS;
}

