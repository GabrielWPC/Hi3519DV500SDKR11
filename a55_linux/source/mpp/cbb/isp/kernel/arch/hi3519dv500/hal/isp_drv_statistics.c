/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_drv.h"
#include "isp_drv_define.h"
#include "isp_reg_define.h"
#include "isp_stt_define.h"
#include "ot_common.h"
#include "ot_osal.h"
#include "ot_math.h"
#include "mkp_isp.h"
#include "isp.h"
#include "mm_ext.h"
#include "sys_ext.h"

/* read FE statistics information */

td_s32 isp_drv_fe_ae_global_statistics_read(isp_fe_stat *stat,
    isp_fe_stat_reg_type *fe_stt_reg, isp_drv_ctx *drv_ctx, ot_vi_pipe vi_pipe_bind, td_u32 k)
{
    td_u16 hist_num[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_AE_HIST_NUM };
    td_u16 cur_pipe_hist_num, hist_multiple;
    td_s32 i, j, m, wdr_frame;
    td_u8 chn_num_max;
    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(stat);
    isp_check_pointer_return(fe_stt_reg);
    isp_check_pointer_return(drv_ctx);

    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);
    wdr_frame = k;
    if (is_half_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        wdr_frame = chn_num_max - 1 - k;
    }

    cur_pipe_hist_num = hist_num[vi_pipe_bind];
    hist_multiple     = OT_ISP_HIST_NUM / cur_pipe_hist_num;

    for (i = 0; i < cur_pipe_hist_num; i++) {
        j = i * hist_multiple;
        stat->fe_ae_stat1.histogram_mem_array[wdr_frame][j] = fe_stt_reg->isp_ae_hist[i].u32;
        for (m = 1; m < hist_multiple; m++) {
            stat->fe_ae_stat1.histogram_mem_array[wdr_frame][j + m] = 0;
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_fe_ae_local_statistics_read(isp_fe_stat *stat, isp_fe_stat_reg_type *fe_stt_reg,
    isp_drv_ctx *drv_ctx, ot_vi_pipe vi_pipe_bind, td_u32 k)
{
    td_bool avg_stat_support[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_AE_SUPPORT_AVG_STAT };
    td_s32 i, j, wdr_frame;
    td_u32 ave_mem;
    td_u8 chn_num_max;
    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(stat);
    isp_check_pointer_return(fe_stt_reg);
    isp_check_pointer_return(drv_ctx);

    if (avg_stat_support[vi_pipe_bind] == TD_FALSE) {
        return TD_SUCCESS;
    }

    wdr_frame = k;
    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);
    if (is_half_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        wdr_frame = chn_num_max - 1 - k;
    }

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ave_mem = fe_stt_reg->isp_ae_aver_r_gr[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
            stat->fe_ae_stat3.zone_avg[wdr_frame][i][j][0] = (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16bit */
            stat->fe_ae_stat3.zone_avg[wdr_frame][i][j][1] = (td_u16)((ave_mem & 0xFFFF)); /* index[1], low 16bit */
            ave_mem = fe_stt_reg->isp_ae_aver_gb_b[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
            stat->fe_ae_stat3.zone_avg[wdr_frame][i][j][2] = /* index[2] */
                (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16bit */
            stat->fe_ae_stat3.zone_avg[wdr_frame][i][j][3] = (td_u16)((ave_mem & 0xFFFF)); /* index[3], low 16bit */
        }
    }

    return TD_SUCCESS;
}

static td_void isp_drv_fe_apb_statistics_read(isp_stat_info *stat_info, isp_fe_reg_type *fe_reg, isp_drv_ctx *drv_ctx,
                                              td_u32 k)
{
    td_u32 wdr_frame;
    td_u8 chn_num_max;
    isp_fe_stat *stat = TD_NULL;
    isp_stat_key stat_key;
    isp_check_pointer_void_return(stat_info);
    isp_check_pointer_void_return(fe_reg);
    isp_check_pointer_void_return(drv_ctx);

    stat = (isp_fe_stat *)stat_info->virt_addr;
    isp_check_pointer_void_return(stat);
    stat_key.key = stat_info->stat_key.key;
    wdr_frame = k;
    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);
    if (is_half_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        wdr_frame = chn_num_max - 1 - k;
    }

    if (stat_key.bit1_fe_ae_global_stat) {
        stat->fe_ae_stat1.pixel_weight[wdr_frame]  = fe_reg->isp_ae_count_stat.u32;
        stat->fe_ae_stat1.pixel_count[wdr_frame]   = fe_reg->isp_ae_total_stat.u32;

        stat->fe_ae_stat2.global_avg_r[wdr_frame]  = fe_reg->isp_ae_total_r_aver.u32;
        stat->fe_ae_stat2.global_avg_gr[wdr_frame] = fe_reg->isp_ae_total_gr_aver.u32;
        stat->fe_ae_stat2.global_avg_gb[wdr_frame] = fe_reg->isp_ae_total_gb_aver.u32;
        stat->fe_ae_stat2.global_avg_b[wdr_frame]  = fe_reg->isp_ae_total_b_aver.u32;
    }

    return;
}

td_s32 isp_drv_fe_af_stt_statistics_read(isp_fe_stat *stat, isp_fe_reg_type *fe_reg, isp_fe_stat_reg_type *fe_stt_reg,
                                         ot_vi_pipe vi_pipe_bind, td_u32 k)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_u8 af_enbale[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };
    td_u32 i, j;
    td_u32 af_stat_data;
    td_u32 zone, col, row;

    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(stat);
    isp_check_pointer_return(fe_reg);
    isp_check_pointer_return(fe_stt_reg);

    if (af_enbale[vi_pipe_bind] != TD_TRUE) {
        return TD_SUCCESS;
    }

    zone = fe_reg->isp_af_zone.u32;
    col = MIN2((zone & 0x1F), OT_ISP_AF_ZONE_COLUMN);
    row = MIN2(((zone & 0x1F00) >> 8), OT_ISP_AF_ZONE_ROW); /* right shift 8 */

    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            af_stat_data = fe_stt_reg->isp_af_stat[(i * col + j) * 4].u32; /* array index(*4) */
            stat->fe_af_stat.zone_metrics[k][i][j].v1 = (td_u16)((0xFFFF0000 & af_stat_data) >> 16); /* 16 */
            stat->fe_af_stat.zone_metrics[k][i][j].h1 = (td_u16)(0xFFFF & af_stat_data);
            af_stat_data = fe_stt_reg->isp_af_stat[(i * col + j) * 4 + 1].u32; /* array index(*4)+1 */
            stat->fe_af_stat.zone_metrics[k][i][j].v2 = (td_u16)((0xFFFF0000 & af_stat_data) >> 16); /* 16 */
            stat->fe_af_stat.zone_metrics[k][i][j].h2 = (td_u16)(0xFFFF & af_stat_data);
            af_stat_data = fe_stt_reg->isp_af_stat[(i * col + j) * 4 + 2].u32; /* array index(*4)+2 */
            stat->fe_af_stat.zone_metrics[k][i][j].hl_cnt = (td_u16)((0xFFFF0000 & af_stat_data) >> 16); /* 16 */
            stat->fe_af_stat.zone_metrics[k][i][j].y = (td_u16)(0xFFFF & af_stat_data);
        }
    }
#endif
    return TD_SUCCESS;
}

static td_s32 isp_drv_fe_switch_stt_addr(ot_vi_pipe vi_pipe_bind, td_u8 wdr_index, isp_drv_ctx *drv_ctx)
{
    td_u32  cur_read_flag;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_u64 size;
    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(drv_ctx);

    if ((drv_ctx->fe_stt_attr.fe_stt[wdr_index].cur_write_flag != 0) &&
        (drv_ctx->fe_stt_attr.fe_stt[wdr_index].cur_write_flag != 1)) {
        isp_err_trace("Err FE cur_write_flag != 0/1 !!!\n");
        return TD_FAILURE;
    }
    cur_read_flag = drv_ctx->fe_stt_attr.fe_stt[wdr_index].cur_write_flag;
    if (drv_ctx->fe_stt_attr.fe_stt[wdr_index].first_frame == TD_TRUE) {
        cur_read_flag = 1;
        drv_ctx->fe_stt_attr.fe_stt[wdr_index].first_frame = TD_FALSE;
    }

    drv_ctx->fe_stt_attr.fe_stt[wdr_index].cur_write_flag  = 1 - cur_read_flag;
    isp_drv_set_fe_stt_addr(vi_pipe_bind, drv_ctx->fe_stt_attr.fe_stt[wdr_index].fe_stt_buf[cur_read_flag].phy_addr);

    vir_addr = drv_ctx->fe_stt_attr.fe_stt[wdr_index].fe_stt_buf[cur_read_flag].vir_addr;
    phy_addr = drv_ctx->fe_stt_attr.fe_stt[wdr_index].fe_stt_buf[cur_read_flag].phy_addr;
    size     = drv_ctx->fe_stt_attr.fe_stt[wdr_index].fe_stt_buf[cur_read_flag].size;

    cmpi_invalid_cache_byaddr(vir_addr, phy_addr, size);

    return TD_SUCCESS;
}

static td_void isp_drv_be_stats_calc_info_update(isp_be_stats_calc_info *be_stats_calc_info, isp_drv_ctx *drv_ctx)
{
    td_u8 i;

    be_stats_calc_info->fusion_mode = drv_ctx->be_sync_para.wdr.fusion_mode;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        be_stats_calc_info->blacklevel[i] = drv_ctx->be_sync_para.be_blc.wdr_blc[i].blc[OT_ISP_CHN_GR];
    }

    be_stats_calc_info->short_thr[0] = drv_ctx->be_sync_para.wdr.short_thr;
    be_stats_calc_info->long_thr[0]  = drv_ctx->be_sync_para.wdr.long_thr;
    be_stats_calc_info->exp_ratio[0] = isp_bitmask(10) * EXP_RATIO_MIN /   /* 10 */
        div_0_to_1(drv_ctx->be_sync_para.wdr.wdr_exp_ratio);
}

static td_s32 isp_drv_fe_stt_statistics_read(isp_stat_info *stat_info, isp_fe_reg_type *fe_reg,
                                             isp_drv_ctx *drv_ctx, ot_vi_pipe vi_pipe_bind, td_u32 k)
{
    td_s32 ret;
    td_u32 cur_read_flag;
    isp_fe_stat_reg_type *fe_stt_reg = TD_NULL;
    isp_fe_stat *stat = TD_NULL;
    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(stat_info);
    isp_check_pointer_return(fe_reg);

    stat = (isp_fe_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    ret = isp_drv_fe_switch_stt_addr(vi_pipe_bind, k, drv_ctx);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp_drv_fe_switch_stt_addr Failed !!!\n");
        return TD_FAILURE;
    }

    cur_read_flag = 1 - drv_ctx->fe_stt_attr.fe_stt[k].cur_write_flag;

    fe_stt_reg = (isp_fe_stat_reg_type *)drv_ctx->fe_stt_attr.fe_stt[k].fe_stt_buf[cur_read_flag].vir_addr;

    isp_drv_fe_ae_global_statistics_read(stat, fe_stt_reg, drv_ctx, vi_pipe_bind, k);

    isp_drv_fe_ae_local_statistics_read(stat, fe_stt_reg, drv_ctx, vi_pipe_bind, k);

    isp_drv_fe_af_stt_statistics_read(stat, fe_reg, fe_stt_reg, vi_pipe_bind, k);

    isp_drv_be_stats_calc_info_update(&stat->be_stats_calc_info, drv_ctx);

    return TD_SUCCESS;
}

/* ISP FE read sta */
td_s32 isp_drv_fe_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 chn_num_max;
    td_u32 k;
    td_s32 ret;
    ot_vi_pipe vi_pipe_bind;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_fe_stat *stat = TD_NULL;
    isp_fe_reg_type *fe_reg  = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    stat = (isp_fe_stat *)stat_info->virt_addr;
    if (stat == TD_NULL) {
        return TD_FAILURE;
    }
    stat->fe_frame_pts = drv_ctx->fe_frame_pts;

    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);

    for (k = 0; k < chn_num_max; k++) {
        /* get side statistics */
        vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);
        isp_drv_fe_apb_statistics_read(stat_info, fe_reg, drv_ctx, k);
        ret = isp_drv_fe_stt_statistics_read(stat_info, fe_reg, drv_ctx, vi_pipe_bind, k);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d] fe_stt_statistics_read failed!\n", vi_pipe);
        }
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_VI_STITCH_GRP
static td_s32 isp_drv_fe_switch_stit_stt_addr(ot_vi_pipe vi_pipe_bind, td_u8 stit_index,
                                              td_u8 wdr_index, isp_drv_ctx *drv_ctx)
{
    td_u32 cur_read_flag;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_u64 size;
    isp_check_no_fe_pipe_return(vi_pipe_bind);
    isp_check_pointer_return(drv_ctx);

    if ((drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].cur_write_flag != 0) &&
        (drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].cur_write_flag != 1)) {
        isp_err_trace("Err FE cur_write_flag != 0/1 !!!\n");
        return TD_FAILURE;
    }

    cur_read_flag = drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].cur_write_flag;
    if (drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].first_frame == TD_TRUE) {
        cur_read_flag = 1;
        drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].first_frame = TD_FALSE;
    }

    isp_drv_set_fe_stt_addr(vi_pipe_bind,
                            drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].fe_stt_buf[cur_read_flag].phy_addr);
    drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].cur_write_flag = 1 - cur_read_flag;
    vir_addr = drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].fe_stt_buf[cur_read_flag].vir_addr;
    phy_addr = drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].fe_stt_buf[cur_read_flag].phy_addr;
    size     = drv_ctx->fe_stit_stt_attr.fe_stt[stit_index][wdr_index].fe_stt_buf[cur_read_flag].size;

    cmpi_invalid_cache_byaddr(vir_addr, phy_addr, size);

    return TD_SUCCESS;
}

td_s32 isp_drv_fe_switch_stit_stt_buf(ot_vi_pipe vi_pipe)
{
    td_u8 i, k;
    td_s32 ret;
    ot_vi_pipe stit_pipe_bind, wdr_pipe_bind;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_drv_ctx *drv_bind_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        stit_pipe_bind = drv_ctx->stitch_attr.stitch_bind_id[k];

        isp_check_pipe_return(stit_pipe_bind);

        drv_bind_ctx = isp_drv_get_ctx(stit_pipe_bind);

        for (i = 0; i < drv_ctx->wdr_attr.pipe_num; i++) {
            wdr_pipe_bind = drv_bind_ctx->wdr_attr.pipe_id[i];
            isp_check_no_fe_pipe_return(wdr_pipe_bind);

            ret = isp_drv_fe_switch_stit_stt_addr(wdr_pipe_bind, k, i, drv_ctx);
            if (ret != TD_SUCCESS) {
                return ret;
            }
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_fe_ae_sti_global_hist_statistics_read(isp_fe_stat *stat,
    isp_fe_stat_reg_type *fe_stt_reg, ot_vi_pipe vi_pipe_bind, td_u32 j)
{
    td_u16 hist_num[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_AE_HIST_NUM };
    td_u16 cur_pipe_hist_num, hist_multiple;
    td_u32 i, target_idx, m;

    cur_pipe_hist_num = hist_num[vi_pipe_bind];
    hist_multiple     = OT_ISP_HIST_NUM / cur_pipe_hist_num;

    for (i = 0; i < cur_pipe_hist_num; i++) {
        target_idx = i * hist_multiple;
        stat->stitch_stat.fe_ae_stat1.histogram_mem_array[j][target_idx] += fe_stt_reg->isp_ae_hist[i].u32;
        for (m = 1; m < hist_multiple; m++) {
            stat->stitch_stat.fe_ae_stat1.histogram_mem_array[j][target_idx + m] += 0;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_fe_ae_sti_global_statistics_read(isp_drv_ctx *drv_ctx, isp_fe_stat *stat)
{
    td_u64 global_avg_sum_r[ISP_WDR_CHN_MAX] = { 0 };
    td_u64 global_avg_sum_gr[ISP_WDR_CHN_MAX] = { 0 };
    td_u64 global_avg_sum_gb[ISP_WDR_CHN_MAX] = { 0 };
    td_u64 global_avg_sum_b[ISP_WDR_CHN_MAX] = { 0 };
    td_u32 pixel_weight_tmp[ISP_WDR_CHN_MAX] = { 0 };
    td_u32 j, k, cur_read_flag, pixel_weight;
    isp_drv_ctx *drv_bind_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_fe_stat_reg_type *fe_stt_reg = TD_NULL;

    (td_void)memset_s(&stat->stitch_stat.fe_ae_stat1, sizeof(ot_isp_fe_ae_stat_1), 0, sizeof(ot_isp_fe_ae_stat_1));

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        isp_check_pipe_return(drv_ctx->stitch_attr.stitch_bind_id[k]);
        drv_bind_ctx = isp_drv_get_ctx(drv_ctx->stitch_attr.stitch_bind_id[k]);

        for (j = 0; j < drv_ctx->wdr_attr.pipe_num; j++) {
            if ((drv_bind_ctx->wdr_attr.pipe_id[j] < OT_ISP_MAX_PIPE_NUM) &&
                (drv_bind_ctx->wdr_attr.pipe_id[j] >= OT_ISP_MAX_FE_PIPE_NUM)) {
                continue;
            }
            isp_drv_fereg_ctx(drv_bind_ctx->wdr_attr.pipe_id[j], fe_reg);

            cur_read_flag = 1 - drv_ctx->fe_stit_stt_attr.fe_stt[k][j].cur_write_flag;

            fe_stt_reg =
                (isp_fe_stat_reg_type *)drv_ctx->fe_stit_stt_attr.fe_stt[k][j].fe_stt_buf[cur_read_flag].vir_addr;
            isp_drv_fe_ae_sti_global_hist_statistics_read(stat, fe_stt_reg,
                                                          drv_bind_ctx->wdr_attr.pipe_id[j], j);

            pixel_weight_tmp[j] = fe_reg->isp_ae_count_stat.u32;
            stat->stitch_stat.fe_ae_stat1.pixel_count[j]  += pixel_weight_tmp[j];
            stat->stitch_stat.fe_ae_stat1.pixel_weight[j] += fe_reg->isp_ae_total_stat.u32;

            global_avg_sum_r[j]  += (td_u64)fe_reg->isp_ae_total_r_aver.u32  * pixel_weight_tmp[j];
            global_avg_sum_gr[j] += (td_u64)fe_reg->isp_ae_total_gr_aver.u32 * pixel_weight_tmp[j];
            global_avg_sum_gb[j] += (td_u64)fe_reg->isp_ae_total_gb_aver.u32 * pixel_weight_tmp[j];
            global_avg_sum_b[j]  += (td_u64)fe_reg->isp_ae_total_b_aver.u32  * pixel_weight_tmp[j];
        }
    }

    for (j = 0; j < drv_ctx->wdr_attr.pipe_num; j++) {
        pixel_weight = div_0_to_1(stat->stitch_stat.fe_ae_stat1.pixel_count[j]);

        stat->stitch_stat.fe_ae_stat2.global_avg_r[j]  = osal_div_u64(global_avg_sum_r[j],  pixel_weight);
        stat->stitch_stat.fe_ae_stat2.global_avg_gr[j] = osal_div_u64(global_avg_sum_gr[j], pixel_weight);
        stat->stitch_stat.fe_ae_stat2.global_avg_gb[j] = osal_div_u64(global_avg_sum_gb[j], pixel_weight);
        stat->stitch_stat.fe_ae_stat2.global_avg_b[j]  = osal_div_u64(global_avg_sum_b[j],  pixel_weight);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_fe_ae_sti_zone_avg_read(ot_vi_pipe stit_pipe_bind, td_u32 wdr_idx,
                                               isp_fe_stat_reg_type *fe_stt_reg, isp_fe_stat *stat)
{
    td_u32 i, l;
    td_u32 ave_mem;

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (l = 0; l < OT_ISP_AE_ZONE_COLUMN; l++) {
            ave_mem = fe_stt_reg->isp_ae_aver_r_gr[i * OT_ISP_AE_ZONE_COLUMN + l].u32;
            stat->stitch_stat.fe_ae_stat3.zone_avg[stit_pipe_bind][wdr_idx][i][l][0] = /* array index[0] */
                (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16 */
            stat->stitch_stat.fe_ae_stat3.zone_avg[stit_pipe_bind][wdr_idx][i][l][1] = /* array index[1] */
                (td_u16)((ave_mem & 0xFFFF));

            ave_mem = fe_stt_reg->isp_ae_aver_gb_b[i * OT_ISP_AE_ZONE_COLUMN + l].u32;
            stat->stitch_stat.fe_ae_stat3.zone_avg[stit_pipe_bind][wdr_idx][i][l][2] = /* array index[2] */
                (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16 */
            stat->stitch_stat.fe_ae_stat3.zone_avg[stit_pipe_bind][wdr_idx][i][l][3] = /* array index[3] */
                (td_u16)((ave_mem & 0xFFFF));
        }
    }
}

static td_s32 isp_drv_fe_ae_sti_local_statistics_read(isp_drv_ctx *drv_ctx, isp_fe_stat *stat)
{
    td_bool avg_stat_support[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_AE_SUPPORT_AVG_STAT };
    td_u32 j, k;
    ot_vi_pipe stit_pipe_bind, wdr_bind_pipe;
    td_u32 cur_read_flag;
    isp_fe_stat_reg_type *fe_stt_reg = TD_NULL;
    isp_drv_ctx *stitch_pipe_drv_ctx = TD_NULL;

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        stit_pipe_bind = drv_ctx->stitch_attr.stitch_bind_id[k];
        isp_check_no_fe_pipe_return(stit_pipe_bind);
        stitch_pipe_drv_ctx = isp_drv_get_ctx(stit_pipe_bind);

        for (j = 0; j < drv_ctx->wdr_attr.pipe_num; j++) {
            wdr_bind_pipe = stitch_pipe_drv_ctx->wdr_attr.pipe_id[j];
            isp_check_no_fe_pipe_return(wdr_bind_pipe);
            if (avg_stat_support[wdr_bind_pipe] == TD_FALSE) {
                continue;
            }
            cur_read_flag = 1 - drv_ctx->fe_stit_stt_attr.fe_stt[k][j].cur_write_flag;

            fe_stt_reg =
                (isp_fe_stat_reg_type *)drv_ctx->fe_stit_stt_attr.fe_stt[k][j].fe_stt_buf[cur_read_flag].vir_addr;

            isp_drv_fe_ae_sti_zone_avg_read(stit_pipe_bind, j, fe_stt_reg, stat);
        }
    }

    return TD_SUCCESS;
}
/* stitch main pipe, read all pipe's statistics to stat->stitch_stat,
 then copy all pipe's statistics to fe_stit_stt_attr.save_stt_stat;
 under all stitch pipe, read non_stitch statistics from drv_ctx_main_pipe->fe_stit_stt_attr.save_stt_stat */
td_s32 isp_drv_fe_stitch_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 stitch_num;
    td_u32 j, k;
    td_s32 ret;
    td_u32 cur_read_flag;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_fe_stat *stat = TD_NULL;
    isp_fe_stat_reg_type *fe_stt_reg = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    stat = (isp_fe_stat *)stat_info->virt_addr;
    if (stat == TD_NULL) {
        return TD_FAILURE;
    }

    if ((drv_ctx->stitch_attr.stitch_enable != TD_TRUE) || (drv_ctx->stitch_attr.main_pipe != TD_TRUE)) {
        return TD_SUCCESS;
    }

    stitch_num = drv_ctx->stitch_attr.stitch_pipe_num;
    /* switch ping pong buffer */
    ret = isp_drv_fe_switch_stit_stt_buf(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp_drv_fe_switch_stit_stt_buf failed !!!\n");
        return TD_FAILURE;
    }

    ret = isp_drv_fe_ae_sti_global_statistics_read(drv_ctx, stat);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    isp_drv_fe_ae_sti_local_statistics_read(drv_ctx, stat);
    for (k = 0; k < stitch_num; k++) {
        for (j = 0; j < drv_ctx->wdr_attr.pipe_num; j++) {
            cur_read_flag = 1 - drv_ctx->fe_stit_stt_attr.fe_stt[k][j].cur_write_flag;

            fe_stt_reg =
                (isp_fe_stat_reg_type *)drv_ctx->fe_stit_stt_attr.fe_stt[k][j].fe_stt_buf[cur_read_flag].vir_addr;

            (td_void)memcpy_s(drv_ctx->fe_stit_stt_attr.save_stt_stat[k][j], sizeof(isp_fe_stat_reg_type),
                              fe_stt_reg, sizeof(isp_fe_stat_reg_type));
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_fe_stitch_non_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 chn_num_max;
    td_s32 k, j;
    ot_vi_pipe vi_pipe_bind, stit_main_pipe;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_drv_ctx *drv_ctx_s = TD_NULL;
    isp_fe_stat *stat = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    stat = (isp_fe_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    stat->fe_frame_pts = drv_ctx->fe_frame_pts;

    chn_num_max = MIN2(drv_ctx->wdr_attr.pipe_num, ISP_WDR_CHN_MAX);

    stit_main_pipe = drv_ctx->stitch_attr.stitch_bind_id[0];
    isp_check_pipe_return(stit_main_pipe);

    drv_ctx_s = isp_drv_get_ctx(stit_main_pipe);

    for (j = 0; j < drv_ctx->stitch_attr.stitch_pipe_num; j++) {
        if (vi_pipe == drv_ctx->stitch_attr.stitch_bind_id[j]) {
            break;
        }
    }

    isp_check_pipe_return(j);

    for (k = 0; k < chn_num_max; k++) {
        /* get side statistics */
        vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);

        isp_drv_fe_apb_statistics_read(stat_info, fe_reg, drv_ctx, k);

        isp_drv_fe_ae_global_statistics_read(stat, drv_ctx_s->fe_stit_stt_attr.save_stt_stat[j][k], drv_ctx,
                                             vi_pipe_bind, k);

        isp_drv_fe_ae_local_statistics_read(stat, drv_ctx_s->fe_stit_stt_attr.save_stt_stat[j][k], drv_ctx,
                                            vi_pipe_bind, k);

        isp_drv_fe_af_stt_statistics_read(stat, fe_reg, drv_ctx_s->fe_stit_stt_attr.save_stt_stat[j][k],
                                          vi_pipe_bind, k);
    }

    return TD_SUCCESS;
}

#endif
/* read BE statistics information from phy:online */
td_s32 isp_drv_be_ae_stt_global_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                                td_u8 blk_num, td_u8 blk_dev)
{
    td_u32 i, k;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_online_stt[k + blk_dev]);

        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            if (k == 0) {
                stat->be_ae_stat1.histogram_mem_array[i]  = be_online_stt[k + blk_dev]->isp_ae_hist[i].u32;
            } else {
                stat->be_ae_stat1.histogram_mem_array[i] += be_online_stt[k + blk_dev]->isp_ae_hist[i].u32;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_be_ae_stt_local_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                               td_u8 blk_num, td_u8 blk_dev)
{
    td_u32 i, j, k;
    td_u8  block_offset    = 0;
    td_u8  block_zone_width;
    td_u32 ave_mem;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_online_stt[k + blk_dev]);

        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            if (k < (OT_ISP_AE_ZONE_COLUMN % div_0_to_1(blk_num))) {
                block_zone_width = (OT_ISP_AE_ZONE_COLUMN / div_0_to_1(blk_num)) + 1;
            } else {
                block_zone_width = OT_ISP_AE_ZONE_COLUMN / div_0_to_1(blk_num);
            }

            for (j = 0; j < block_zone_width; j++) {
                ave_mem = be_online_stt[k + blk_dev]->isp_ae_aver_r_gr[i * block_zone_width + j].u32;
                stat->be_ae_stat3.zone_avg[i][j + block_offset][0] =
                    (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16bit */
                stat->be_ae_stat3.zone_avg[i][j + block_offset][1] = /* array index 1 */
                    (td_u16)((ave_mem & 0xFFFF)); /* low 16bit */
                ave_mem = be_online_stt[k + blk_dev]->isp_ae_aver_gb_b[i * block_zone_width + j].u32;
                stat->be_ae_stat3.zone_avg[i][j + block_offset][2] = /* array index 2 */
                    (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16bit */
                stat->be_ae_stat3.zone_avg[i][j + block_offset][3] = /* array index 3 */
                    (td_u16)((ave_mem & 0xFFFF)); /* low 16bit */
            }
        }

        block_offset += block_zone_width;
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_be_mg_stt_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                         td_u8 blk_num, td_u8 blk_dev)
{
    td_u8  k;
    td_u8  block_offset     = 0;
    td_u8  block_zone_width;
    td_u32 i, j;
    td_u32 ave_mem;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_online_stt[k + blk_dev]);

        for (i = 0; i < OT_ISP_MG_ZONE_ROW; i++) {
            if (k < (OT_ISP_MG_ZONE_COLUMN % div_0_to_1(blk_num))) {
                block_zone_width = (OT_ISP_MG_ZONE_COLUMN / div_0_to_1(blk_num)) + 1;
            } else {
                block_zone_width = OT_ISP_MG_ZONE_COLUMN / div_0_to_1(blk_num);
            }

            for (j = 0; j < block_zone_width; j++) {
                ave_mem = be_online_stt[k + blk_dev]->isp_la_aver[i * block_zone_width + j].u32;
                stat->mg_stat.zone_avg[i][j + block_offset][0] =
                    (td_u16)((ave_mem & 0xFF000000) >> 24); /* Rshift 24bit */
                stat->mg_stat.zone_avg[i][j + block_offset][1] = /* array index 1 */
                    (td_u16)((ave_mem & 0xFF0000) >> 16); /* Rshift 16bit */
                stat->mg_stat.zone_avg[i][j + block_offset][2] = /* array index 2 */
                    (td_u16)((ave_mem & 0xFF00) >> 8); /* Rshift 8bit */
                stat->mg_stat.zone_avg[i][j + block_offset][3] = /* array index 3 */
                    (td_u16)((ave_mem & 0xFF)); /* low 8bit */
            }
        }

        block_offset += block_zone_width;
    }

    return TD_SUCCESS;
}

td_void isp_drv_cal_awb_gain_conv(isp_drv_ctx *drv_ctx, td_u16 *rgb_gain_conv, td_u8 length)
{
    td_s32 i;
    td_u32 wb_gain_bf_stat[OT_ISP_BAYER_CHN_NUM] = { 0x100, 0x100, 0x100, 0x100 };
    td_u32 min_gain, norm_gain;
    td_u16 gain_conv_r, gain_conv_g, gain_conv_b;
    isp_sync_cfg_buf_node *node = TD_NULL;
    ot_unused(length);

    node = &(drv_ctx->sync_cfg.sync_cfg_buf.sync_cfg_buf_node[0]);

    if (node->awb_reg_cfg.be_awb_switch == OT_ISP_AWB_AFTER_DRC) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            wb_gain_bf_stat[i] = node->awb_reg_cfg.be_white_balance_gain[i];
        }
    }

    min_gain = 0xFFFF;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        min_gain = MIN2(min_gain, wb_gain_bf_stat[i]);
    }

    norm_gain = 0x10000 / div_0_to_1(min_gain);
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        wb_gain_bf_stat[i] = (wb_gain_bf_stat[i] * norm_gain + 0x80) >> 8; /* Rshift 8bit */
    }

    gain_conv_r = (0x10000 + (wb_gain_bf_stat[0] >> 1)) / div_0_to_1(wb_gain_bf_stat[0]); /* array index 0 */
    gain_conv_g = (0x10000 + (wb_gain_bf_stat[1] >> 1)) / div_0_to_1(wb_gain_bf_stat[1]); /* array index 1 */
    gain_conv_b = (0x10000 + (wb_gain_bf_stat[3] >> 1)) / div_0_to_1(wb_gain_bf_stat[3]); /* array index 3 */

    rgb_gain_conv[0] = gain_conv_r; /* array index 0 */
    rgb_gain_conv[1] = gain_conv_g; /* array index 1 */
    rgb_gain_conv[2] = gain_conv_b; /* array index 2 */
    return;
}

td_void isp_drv_be_awb_stt_stat2_read(isp_stat *stat, isp_post_be_reg_type *be_reg_offset,
    isp_be_online_stat_reg_type *be_online_stt_offset, td_u32 whole_col, const td_u16 *rgb_gain_conv)
{
    td_u32 i, j;
    td_u32 value, zone, col, row;
    td_u8  block_offset = 0;
    td_u16 read_r_avg, read_g_avg, read_b_avg;
    td_u16 w_start_addr, r_start_addr;

    zone = be_reg_offset->isp_awb_zone.u32;
    col  = MIN2((zone & 0x3F), OT_ISP_AWB_ZONE_ORIG_COLUMN);
    row  = MIN2(((zone & 0x3F00) >> 8), OT_ISP_AWB_ZONE_ORIG_ROW); /* Rshift 8bit */

    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            w_start_addr = i * whole_col + j + block_offset;
            r_start_addr = (i * whole_col + j) * 2;  /* get array index(*2) */

            value = be_online_stt_offset->isp_awb_stat[r_start_addr + 0].u32; /* index(*2) */
            read_r_avg = (value & 0xFFFF);
            read_g_avg = ((value >> 16) & 0xFFFF); /* shift 16bit, get high 16bit */
            value = be_online_stt_offset->isp_awb_stat[r_start_addr + 1].u32; /* index(*2) */
            read_b_avg = (value & 0xFFFF);

            stat->awb_stat2.metering_mem_array_count_all[w_start_addr] =
                ((value >> 16) & 0xFFFF); /* shift 16bit */
            stat->awb_stat2.metering_mem_array_avg_r[w_start_addr] =
                (read_r_avg * rgb_gain_conv[0] + 0x80) >> 8; /* shift 8bit, index[0] */
            stat->awb_stat2.metering_mem_array_avg_g[w_start_addr] =
                (read_g_avg * rgb_gain_conv[1] + 0x80) >> 8; /* shift 8bit, index[1] */
            stat->awb_stat2.metering_mem_array_avg_b[w_start_addr] =
                (read_b_avg * rgb_gain_conv[2] + 0x80) >> 8; /* shift 8bit, index[2] */
        }
    }
    block_offset += col;
    return;
}

td_s32 isp_drv_be_awb_stt_statistics_read(isp_drv_ctx *drv_ctx, isp_stat *stat, isp_post_be_reg_type *be_reg[],
                                          isp_be_online_stat_reg_type *be_online_stt[])
{
    td_s32 k;
    td_u32 zone;
    td_u8  blk_dev, blk_num;
    td_u32 whole_col  = 0;
    td_u16 rgb_gain_conv[3] = {0x100, 0x100, 0x100}; /* array max 3 for R,G,B channel */
    isp_post_be_reg_type *be_reg_offset = TD_NULL;
    isp_be_online_stat_reg_type *be_online_stt_offset = TD_NULL;

    isp_check_pointer_return(stat);
    isp_check_pointer_return(drv_ctx);

    blk_dev = drv_ctx->work_mode.block_dev;
    blk_num = drv_ctx->work_mode.block_num;

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_reg[k + blk_dev]);
        zone = be_reg[k + blk_dev]->isp_awb_zone.u32;
        whole_col  += MIN2((zone & 0x3F), OT_ISP_AWB_ZONE_ORIG_COLUMN);
    }

    isp_drv_cal_awb_gain_conv(drv_ctx, rgb_gain_conv, sizeof(rgb_gain_conv));

    for (k = 0; k < blk_num; k++) {
        be_online_stt_offset = be_online_stt[k + blk_dev];
        isp_check_pointer_return(be_online_stt_offset);

        be_reg_offset = be_reg[k + blk_dev];
        isp_check_pointer_return(be_reg_offset);

        isp_drv_be_awb_stt_stat2_read(stat, be_reg_offset, be_online_stt_offset, whole_col, rgb_gain_conv);
    }

    stat->awb_stat1.metering_awb_avg_r =
        (stat->awb_stat1.metering_awb_avg_r * rgb_gain_conv[0] + 0x80) >> 8; /* index[0] shift 8 */
    stat->awb_stat1.metering_awb_avg_g =
        (stat->awb_stat1.metering_awb_avg_g * rgb_gain_conv[1] + 0x80) >> 8; /* index[1] shift 8 */
    stat->awb_stat1.metering_awb_avg_b =
        (stat->awb_stat1.metering_awb_avg_b * rgb_gain_conv[2] + 0x80) >> 8; /* index[2] shift 8 */

    return TD_SUCCESS;
}

td_s32 isp_drv_be_af_stt_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                         td_u8 blk_dev, td_u32 zone)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_u32 i, j;
    td_u32 be_af_stat_data;
    td_u32 col = (zone & 0x1F);
    td_u32 row = ((zone & 0x1F00) >> 8); /* shift 8bit */

    isp_check_pointer_return(stat);
    isp_check_pointer_return(be_online_stt[blk_dev]);

    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            be_af_stat_data = be_online_stt[blk_dev]->isp_af_stat[(i * col + j) * 4].u32; /* array index(*4) */
            stat->be_af_stat.zone_metrics[i][j].v1    =
                (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* shift 16bit, get high 16bit */
            stat->be_af_stat.zone_metrics[i][j].h1    =
                (td_u16)(0xFFFF & be_af_stat_data);
            be_af_stat_data = be_online_stt[blk_dev]->isp_af_stat[(i * col + j) * 4 + 1].u32; /* array index(*4)+1 */
            stat->be_af_stat.zone_metrics[i][j].v2    =
                (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* shift 16bit, get high 16bit */
            stat->be_af_stat.zone_metrics[i][j].h2    =
                (td_u16)(0xFFFF & be_af_stat_data);
            be_af_stat_data = be_online_stt[blk_dev]->isp_af_stat[(i * col + j) * 4 + 2].u32; /* array index(*4)+2 */
            stat->be_af_stat.zone_metrics[i][j].hl_cnt =
                (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* shift 16bit, get high 16bit */
            stat->be_af_stat.zone_metrics[i][j].y     =
                (td_u16)(0xFFFF & be_af_stat_data);
        }
    }
#endif
    return TD_SUCCESS;
}

td_s32 isp_drv_be_dehaze_stt_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                             td_u8 blk_num, td_u8 blk_dev)
{
    td_u32 i, j, m;

    isp_check_pointer_return(stat);

    j = DEFOG_ZONE_NUM >> 1;

    for (i = 0; i < blk_num; i++) {
        isp_check_pointer_return(be_online_stt[i + blk_dev]);

        for (m = 0; m < j; m++) {
            stat->dehaze_stat.min_dout[i][m] = be_online_stt[i + blk_dev]->isp_dehaze_minstat[m].u32;
        }

        for (m = 0; m < DEFOG_ZONE_NUM; m++) {
            stat->dehaze_stat.max_stat_dout[i][m] = be_online_stt[i + blk_dev]->isp_dehaze_maxstat[m].u32;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_be_flicker_stt_statistics_read(isp_stat *stat, isp_be_online_stat_reg_type *be_online_stt[],
                                                     td_u8 blk_num, td_u8 blk_dev)
{
    td_u16  k, j;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_online_stt[k + blk_dev]);

        for (j = 0; j < ISP_FLICKER_G_MEAN_LENGTH; j++) {
            stat->flicker_stat.g_mean[k][j] = be_online_stt[k + blk_dev]->isp_flick_gmean[j].u32;
        }
    }

    return TD_SUCCESS;
}


td_void isp_drv_be_apb_comm_statistics_read(isp_stat *stat, isp_post_be_reg_type *be_reg[], td_u8 blk_dev)
{
    stat->comm_stat.white_balance_gain[0] = be_reg[blk_dev]->isp_wb_gain1.bits.isp_wb_rgain;  /* array index[0] */
    stat->comm_stat.white_balance_gain[1] = be_reg[blk_dev]->isp_wb_gain1.bits.isp_wb_grgain; /* array index[1] */
    stat->comm_stat.white_balance_gain[2] = be_reg[blk_dev]->isp_wb_gain2.bits.isp_wb_gbgain; /* array index[2] */
    stat->comm_stat.white_balance_gain[3] = be_reg[blk_dev]->isp_wb_gain2.bits.isp_wb_bgain;  /* array index[3] */
}

td_void isp_drv_be_apb_flicker_statistics_read(isp_stat *stat, isp_pre_be_reg_type *be_reg[],
                                               td_u8 blk_num, td_u8 blk_dev)
{
    td_u8  k;

    for (k = 0; k < blk_num; k++) {
        stat->flicker_stat.cur_diff_gr[k] = be_reg[k + blk_dev]->isp_flick_gr_diff_cur.bits.isp_flick_gr_diff_cur;
        stat->flicker_stat.cur_diff_gb[k] = be_reg[k + blk_dev]->isp_flick_gb_diff_cur.bits.isp_flick_gb_diff_cur;
        stat->flicker_stat.cur_abs_gr[k]  = be_reg[k + blk_dev]->isp_flick_gr_abs_cur.bits.isp_flick_gr_abs_cur;
        stat->flicker_stat.cur_abs_gb[k]  = be_reg[k + blk_dev]->isp_flick_gb_abs_cur.bits.isp_flick_gb_abs_cur;
    }
}

td_void isp_drv_post_be_apb_ae_statistics_read(isp_stat *stat, isp_post_be_reg_type *be_reg[],
                                               td_u8 blk_num, td_u8 blk_dev)
{
    td_u8  k, be_ae_sel;
    td_u32 pixel_weight;
    td_u32 pixel_weight_tmp;
    td_u64 global_avg_sum_r  = 0;
    td_u64 global_avg_sum_gr = 0;
    td_u64 global_avg_sum_gb = 0;
    td_u64 global_avg_sum_b  = 0;

    be_ae_sel = be_reg[0]->isp_be_module_pos.bits.isp_ae_sel;
    if ((be_ae_sel != OT_ISP_AE_AFTER_DG) && (be_ae_sel != OT_ISP_AE_AFTER_WB) &&
        (be_ae_sel != OT_ISP_AE_AFTER_DRC)) {
        return;
    }

    for (k = 0; k < blk_num; k++) {
        if (k == 0) {
            pixel_weight_tmp = be_reg[k + blk_dev]->isp_ae_count_stat.bits.isp_ae_count_pixels;
            stat->be_ae_stat1.pixel_weight = pixel_weight_tmp;
            stat->be_ae_stat1.pixel_count  = be_reg[k + blk_dev]->isp_ae_total_stat.bits.isp_ae_total_pixels;
        } else {
            pixel_weight_tmp = be_reg[k + blk_dev]->isp_ae_count_stat.bits.isp_ae_count_pixels;
            stat->be_ae_stat1.pixel_weight += pixel_weight_tmp;
            stat->be_ae_stat1.pixel_count  += be_reg[k + blk_dev]->isp_ae_total_stat.bits.isp_ae_total_pixels;
        }

        global_avg_sum_r  += ((td_u64)be_reg[k + blk_dev]->isp_ae_total_r_aver.u32)  * ((td_u64)pixel_weight_tmp);
        global_avg_sum_gr += ((td_u64)be_reg[k + blk_dev]->isp_ae_total_gr_aver.u32) * ((td_u64)pixel_weight_tmp);
        global_avg_sum_gb += ((td_u64)be_reg[k + blk_dev]->isp_ae_total_gb_aver.u32) * ((td_u64)pixel_weight_tmp);
        global_avg_sum_b  += ((td_u64)be_reg[k + blk_dev]->isp_ae_total_b_aver.u32)  * ((td_u64)pixel_weight_tmp);
    }

    pixel_weight = div_0_to_1(stat->be_ae_stat1.pixel_weight);

    stat->be_ae_stat2.global_avg_r  = osal_div_u64(global_avg_sum_r,  pixel_weight);
    stat->be_ae_stat2.global_avg_gr = osal_div_u64(global_avg_sum_gr, pixel_weight);
    stat->be_ae_stat2.global_avg_gb = osal_div_u64(global_avg_sum_gb, pixel_weight);
    stat->be_ae_stat2.global_avg_b  = osal_div_u64(global_avg_sum_b,  pixel_weight);
}

td_void isp_drv_be_apb_awb_statistics_read(isp_stat *stat, isp_post_be_reg_type *be_reg[],
                                           td_u8 blk_num, td_u8 blk_dev)
{
    td_u8  k;
    td_u64 metering_awb_avg_r = 0;
    td_u64 metering_awb_avg_g = 0;
    td_u64 metering_awb_avg_b = 0;
    td_u64 metering_awb_count_all = 0;

    for (k = 0; k < blk_num; k++) {
        metering_awb_avg_r += (td_u64)be_reg[k + blk_dev]->isp_awb_avg_r.bits.isp_awb_avg_r *
                              be_reg[k + blk_dev]->isp_awb_cnt_all.bits.isp_awb_count_all;
        metering_awb_avg_g += (td_u64)be_reg[k + blk_dev]->isp_awb_avg_g.bits.isp_awb_avg_g *
                              be_reg[k + blk_dev]->isp_awb_cnt_all.bits.isp_awb_count_all;
        metering_awb_avg_b += (td_u64)be_reg[k + blk_dev]->isp_awb_avg_b.bits.isp_awb_avg_b *
                              be_reg[k + blk_dev]->isp_awb_cnt_all.bits.isp_awb_count_all;
        metering_awb_count_all += be_reg[k + blk_dev]->isp_awb_cnt_all.bits.isp_awb_count_all;
    }

    stat->awb_stat1.metering_awb_avg_r =
        (td_u16)(osal_div_u64(metering_awb_avg_r, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_avg_g =
        (td_u16)(osal_div_u64(metering_awb_avg_g, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_avg_b =
        (td_u16)(osal_div_u64(metering_awb_avg_b, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_count_all =
        (td_u16)(osal_div_u64(metering_awb_count_all, div_0_to_1(blk_num)));
}

static td_s32 isp_drv_pre_be_apb_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key)
{
    td_u8  k;
    td_s32 ret;
    td_u8  blk_dev, blk_num;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_pre_be_reg_type *pre_be[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    ret = isp_drv_get_pre_be_reg_virt_addr(vi_pipe, pre_be);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get_pre_be_reg_virt_addr Err!\n", vi_pipe);
        return ret;
    }
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_dev = drv_ctx->work_mode.block_dev;
    blk_num = drv_ctx->work_mode.block_num;

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(pre_be[k + blk_dev]);
    }

    /* Flicker */
    if (stat_key.bit1_flicker) {
        isp_drv_be_apb_flicker_statistics_read(stat, pre_be, blk_num, blk_dev);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_be_apb_check_sum_read(isp_stat *stat, isp_post_be_reg_type *be_reg[],
    td_u8 blk_num, td_u8 blk_dev)
{
    td_u32 k, sum_y_low32;
    td_u64 sum_y_high32;
    for (k = 0; k < blk_num; k++) {
        sum_y_low32 = be_reg[k + blk_dev]->isp_y_sum0.u32;
        sum_y_high32 = be_reg[k + blk_dev]->isp_y_sum1.u32;
        stat->sum_stat.sum_y[k] = ((sum_y_high32) << 32) + sum_y_low32; // left shift 32
    }
}

static td_s32 isp_drv_post_be_apb_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key)
{
    td_u8  k;
    td_u8  blk_dev, blk_num;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_post_be_reg_type *post_be[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    ret = isp_drv_get_post_be_reg_virt_addr(vi_pipe, post_be);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get_post_be_reg_virt_addr Err!\n", vi_pipe);
        return ret;
    }
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_dev = drv_ctx->work_mode.block_dev;
    blk_num = drv_ctx->work_mode.block_num;

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(post_be[k + blk_dev]);
    }

    if (stat_key.bit1_comm_stat) {
        isp_drv_be_apb_comm_statistics_read(stat, post_be, blk_dev);
    }

    /* AE */
    if (stat_key.bit1_be_ae_global_stat) {
        isp_drv_post_be_apb_ae_statistics_read(stat, post_be, blk_num, blk_dev);
    }

    /* awb */
    if (stat_key.bit1_awb_stat1) {
        isp_drv_be_apb_awb_statistics_read(stat, post_be, blk_num, blk_dev);
    }

    if (stat_key.bit1_check_sum) {
        isp_drv_be_apb_check_sum_read(stat, post_be, blk_num, blk_dev);
    }
    return TD_SUCCESS;
}

td_s32 isp_drv_be_apb_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key)
{
    td_s32 ret;
    isp_check_pointer_return(stat);

    ret = isp_drv_pre_be_apb_statistics_read(vi_pipe, stat, stat_key);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = isp_drv_post_be_apb_statistics_read(vi_pipe, stat, stat_key);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void isp_drv_be_stt_get_virt_addr(ot_vi_pipe vi_pipe, td_void **stt_buf_addr)
{
    td_u32 cur_read_flag;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_u64  size;
    isp_drv_ctx *drv_ctx = TD_NULL;

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if ((drv_ctx->be_online_stt_buf.cur_write_flag != 0) && (drv_ctx->be_online_stt_buf.cur_write_flag != 1)) {
        isp_err_trace("Err cur_write_flag != 0/1 !!!\n");
        drv_ctx->be_online_stt_buf.cur_write_flag = 0;
    }

    /* switch ping pong buffer */
    cur_read_flag = drv_ctx->be_online_stt_buf.cur_write_flag;
    if (drv_ctx->be_online_stt_buf.get_first_stat_info_flag == TD_FALSE) {
        (td_void)memcpy_s(drv_ctx->be_online_stt_buf.be_stt_buf[1].vir_addr, sizeof(isp_be_rw_online_stt_reg),
                          drv_ctx->be_online_stt_buf.be_stt_buf[0].vir_addr, sizeof(isp_be_rw_online_stt_reg));
        cur_read_flag = 1;
        drv_ctx->be_online_stt_buf.get_first_stat_info_flag = TD_TRUE;
    }

    drv_ctx->be_online_stt_buf.cur_write_flag = 1 - cur_read_flag;

    vir_addr = drv_ctx->be_online_stt_buf.be_stt_buf[cur_read_flag].vir_addr;
    phy_addr = drv_ctx->be_online_stt_buf.be_stt_buf[cur_read_flag].phy_addr;
    size     = drv_ctx->be_online_stt_buf.be_stt_buf[cur_read_flag].size;

    cmpi_invalid_cache_byaddr(vir_addr, phy_addr, size);

    *stt_buf_addr = vir_addr;
}

td_s32 isp_drv_be_stt_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key)
{
    td_u8  k, idx;
    td_u8  blk_dev, blk_num;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_rw_online_stt_reg *be_rw_online_stt_reg = TD_NULL;
    isp_be_online_stat_reg_type *be_online_stt[ISP_MAX_BE_NUM] = { TD_NULL };
    isp_post_be_reg_type *post_be[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *vir_addr = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat);

    ret = isp_drv_get_post_be_reg_virt_addr(vi_pipe, post_be);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get_post_be_reg_virt_addr Err!\n", vi_pipe);
        return ret;
    }
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_dev = drv_ctx->work_mode.block_dev;
    blk_num = drv_ctx->work_mode.block_num;

    isp_drv_be_stt_get_virt_addr(vi_pipe, &vir_addr);

    be_rw_online_stt_reg = (isp_be_rw_online_stt_reg *)vir_addr;

    for (k = 0; k < blk_num; k++) {
        idx = MIN2(ISP_MAX_BE_NUM - 1, k + blk_dev);
        be_online_stt[idx] = &be_rw_online_stt_reg->be_online_stt_reg[idx];
    }

    /* BE AE statistics */
    if (stat_key.bit1_be_ae_global_stat) {
        isp_drv_be_ae_stt_global_statistics_read(stat, be_online_stt, blk_num, blk_dev);
    }

    if (stat_key.bit1_be_ae_local_stat) {
        isp_drv_be_ae_stt_local_statistics_read(stat, be_online_stt, blk_num, blk_dev);
    }

    if (stat_key.bit1_mg_stat) {
        isp_drv_be_mg_stt_statistics_read(stat, be_online_stt, blk_num, blk_dev);
    }

    /* BE AWB statistics */
    if (stat_key.bit1_awb_stat2) {
        isp_drv_be_awb_stt_statistics_read(drv_ctx, stat, post_be, be_online_stt);
    }

    /* BE AF statistics */
    if (stat_key.bit1_be_af_stat) {
        isp_drv_be_af_stt_statistics_read(stat, be_online_stt, blk_dev, post_be[blk_dev]->isp_af_zone.u32);
    }

    /* BE dehaze statistics */
    if (stat_key.bit1_dehaze) {
        isp_drv_be_dehaze_stt_statistics_read(stat, be_online_stt, blk_num, blk_dev);
    }

    if (stat_key.bit1_flicker) {
        isp_drv_be_flicker_stt_statistics_read(stat, be_online_stt, blk_num, blk_dev);
    }

    isp_drv_ldci_online_attr_update(vi_pipe, stat);

    return TD_SUCCESS;
}

static td_void isp_drv_be_ae_global_offline_stat_get(isp_stat *stat, isp_be_offline_ae_stat_type *ae_atat,
                                                     isp_ae_global_avg_sum *global_avg_sum, td_u32 k)
{
    td_u32 i;
    td_u32 pixel_weight_tmp;

    if (k == 0) {
        (td_void)memcpy_s(stat->be_ae_stat1.histogram_mem_array, sizeof(stat->be_ae_stat1.histogram_mem_array),
            (const u_isp_ae_hist*)(ae_atat->isp_ae_hist), sizeof(ae_atat->isp_ae_hist));
        pixel_weight_tmp = ae_atat->isp_ae_count_stat_rstt.bits.isp_ae_count_pixels_stt;
        stat->be_ae_stat1.pixel_weight = ae_atat->isp_ae_count_stat_rstt.bits.isp_ae_count_pixels_stt;
        stat->be_ae_stat1.pixel_count = ae_atat->isp_ae_total_stat_rstt.bits.isp_ae_total_pixels_stt;
    } else {
        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            stat->be_ae_stat1.histogram_mem_array[i] += ae_atat->isp_ae_hist[i].u32;
        }
        pixel_weight_tmp = ae_atat->isp_ae_count_stat_rstt.bits.isp_ae_count_pixels_stt;
        stat->be_ae_stat1.pixel_weight += pixel_weight_tmp;
        stat->be_ae_stat1.pixel_count += ae_atat->isp_ae_total_stat_rstt.bits.isp_ae_total_pixels_stt;
    }

    global_avg_sum->global_avg_sum_r  += ((td_u64)ae_atat->isp_ae_total_r_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
    global_avg_sum->global_avg_sum_gr += ((td_u64)ae_atat->isp_ae_total_gr_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
    global_avg_sum->global_avg_sum_gb += ((td_u64)ae_atat->isp_ae_total_gb_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
    global_avg_sum->global_avg_sum_b  += ((td_u64)ae_atat->isp_ae_total_b_aver_rstt.u32)  * ((td_u64)pixel_weight_tmp);
}

static td_void isp_drv_be_ae_global_avg_calc(isp_stat *stat, const isp_ae_global_avg_sum *global_avg_sum)
{
    td_u32 pixel_weight;

    pixel_weight = div_0_to_1(stat->be_ae_stat1.pixel_weight);

    stat->be_ae_stat2.global_avg_r  = osal_div_u64(global_avg_sum->global_avg_sum_r,  pixel_weight);
    stat->be_ae_stat2.global_avg_gr = osal_div_u64(global_avg_sum->global_avg_sum_gr, pixel_weight);
    stat->be_ae_stat2.global_avg_gb = osal_div_u64(global_avg_sum->global_avg_sum_gb, pixel_weight);
    stat->be_ae_stat2.global_avg_b  = osal_div_u64(global_avg_sum->global_avg_sum_b,  pixel_weight);
}

static td_void isp_drv_be_ae_local_offline_stat_get(isp_stat *stat, isp_be_offline_ae_stat_type *ae_atat,
                                                    td_u8 blk_num, td_u32 k, td_u8 *block_offset)
{
    td_u8  block_zone_width;
    td_u32 i, j;
    td_u32 ave_mem;

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        if (k < (OT_ISP_AE_ZONE_COLUMN % div_0_to_1(blk_num))) {
            block_zone_width = (OT_ISP_AE_ZONE_COLUMN / div_0_to_1(blk_num)) + 1;
        } else {
            block_zone_width = OT_ISP_AE_ZONE_COLUMN / div_0_to_1(blk_num);
        }

        for (j = 0; j < block_zone_width; j++) {
            ave_mem = ae_atat->isp_ae_aver_r_gr[i * block_zone_width + j].u32;
            stat->be_ae_stat3.zone_avg[i][j + *block_offset][0] = (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* 16bit */
            stat->be_ae_stat3.zone_avg[i][j + *block_offset][1] = (td_u16)((ave_mem & 0xFFFF)); /* array index 1 */
            ave_mem = ae_atat->isp_ae_aver_gb_b[i * block_zone_width + j].u32;
            stat->be_ae_stat3.zone_avg[i][j + *block_offset][2] = (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* 2/16 */
            stat->be_ae_stat3.zone_avg[i][j + *block_offset][3] = (td_u16)((ave_mem & 0xFFFF)); /* array index 3 */
        }
    }
    *block_offset += block_zone_width;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
td_void isp_drv_read_be_ae_detail_stats(const isp_be_offline_detail_ae_awb_stat *ae_awb_stat, td_s32 r, td_s32 c,
    ot_isp_detail_ae_stats *stat)
{
    td_s32 i, j;
    td_u32 ave_mem;
    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            ave_mem = ae_awb_stat->isp_detail_ae_stat.isp_ae_aver_r_gr[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
            stat->be_zone_avg[r * OT_ISP_AE_ZONE_ROW + i][c * OT_ISP_AE_ZONE_COLUMN + j][0] =
                (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* 16bit */
            stat->be_zone_avg[r * OT_ISP_AE_ZONE_ROW + i][c * OT_ISP_AE_ZONE_COLUMN + j][1] =
                (td_u16)((ave_mem & 0xFFFF)); /* array index 1 */
            ave_mem = ae_awb_stat->isp_detail_ae_stat.isp_ae_aver_gb_b[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
            stat->be_zone_avg[r * OT_ISP_AE_ZONE_ROW + i][c * OT_ISP_AE_ZONE_COLUMN + j][2] = // 2 bayer gr
                (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* 2/16 */
            stat->be_zone_avg[r * OT_ISP_AE_ZONE_ROW + i][c * OT_ISP_AE_ZONE_COLUMN + j][3] = // 3 bayer b
                (td_u16)((ave_mem & 0xFFFF)); /* array index 3 */
        }
    }
}
#endif
td_s32 isp_drv_be_ae_offline_global_stat_read(isp_stat *stat, isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_u32 k;
    isp_ae_global_avg_sum global_avg_sum = { 0 };

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);
        isp_drv_be_ae_global_offline_stat_get(stat, &be_stt[k]->isp_ae_atat, &global_avg_sum, k);
    }

    isp_drv_be_ae_global_avg_calc(stat, &global_avg_sum);

    return TD_SUCCESS;
}

td_s32 isp_drv_be_ae_offline_local_stat_read(isp_stat *stat, isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_u32 k;
    td_u8  block_offset = 0;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);

        isp_drv_be_ae_local_offline_stat_get(stat, &be_stt[k]->isp_ae_atat, blk_num, k, &block_offset);
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_be_mg_offline_statistics_read(isp_stat *stat, isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_u8  k;
    td_u8  block_offset    = 0;
    td_u8  block_zone_width;
    td_u32 i, j;
    td_u32 ave_mem;

    isp_check_pointer_return(stat);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);

        for (i = 0; i < OT_ISP_MG_ZONE_ROW; i++) {
            if (k < (OT_ISP_MG_ZONE_COLUMN % div_0_to_1(blk_num))) {
                block_zone_width = (OT_ISP_MG_ZONE_COLUMN / div_0_to_1(blk_num)) + 1;
            } else {
                block_zone_width = OT_ISP_MG_ZONE_COLUMN / div_0_to_1(blk_num);
            }

            for (j = 0; j < block_zone_width; j++) {
                ave_mem = be_stt[k]->isp_la_aver[i * block_zone_width + j].u32;
                stat->mg_stat.zone_avg[i][j + block_offset][0] =
                    (td_u16)((ave_mem & 0xFF000000) >> 24); /* Rshift 24bit */
                stat->mg_stat.zone_avg[i][j + block_offset][1] = /* array index 1 */
                    (td_u16)((ave_mem & 0xFF0000) >> 16); /* Rshift 16bit */
                stat->mg_stat.zone_avg[i][j + block_offset][2] = /* array index 2 */
                    (td_u16)((ave_mem & 0xFF00) >> 8); /* Rshift 8bit */
                stat->mg_stat.zone_avg[i][j + block_offset][3] = /* array index 3 */
                    (td_u16)((ave_mem & 0xFF)); /* low 8bit */
            }
        }

        block_offset += block_zone_width;
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_be_awb_stat1_read(isp_be_offline_stat *be_stt[], td_u8 blk_num,
    isp_stat *stat, const td_u16 *rgb_gain_conv, td_u8 length)
{
    td_s32 k;
    td_u64 metering_awb_avg_r = 0;
    td_u64 metering_awb_avg_g = 0;
    td_u64 metering_awb_avg_b = 0;
    td_u64 metering_awb_count_all = 0;
    ot_unused(length);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);

        metering_awb_avg_r += (td_u64)be_stt[k]->isp_awb_avg_r_rstt.bits.isp_awb_avg_r_stt *
                              be_stt[k]->isp_awb_cnt_all_rstt.bits.isp_awb_count_all_stt;
        metering_awb_avg_g += (td_u64)be_stt[k]->isp_awb_avg_g_rstt.bits.isp_awb_avg_g_stt *
                              be_stt[k]->isp_awb_cnt_all_rstt.bits.isp_awb_count_all_stt;
        metering_awb_avg_b += (td_u64)be_stt[k]->isp_awb_avg_b_rstt.bits.isp_awb_avg_b_stt *
                              be_stt[k]->isp_awb_cnt_all_rstt.bits.isp_awb_count_all_stt;
        metering_awb_count_all += be_stt[k]->isp_awb_cnt_all_rstt.bits.isp_awb_count_all_stt;
    }

    stat->awb_stat1.metering_awb_avg_r =
        (td_u16)(osal_div_u64(metering_awb_avg_r, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_avg_g =
        (td_u16)(osal_div_u64(metering_awb_avg_g, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_avg_b =
        (td_u16)(osal_div_u64(metering_awb_avg_b, div_0_to_1(metering_awb_count_all)));
    stat->awb_stat1.metering_awb_count_all =
        (td_u16)(osal_div_u64(metering_awb_count_all, div_0_to_1(blk_num)));
    stat->awb_stat1.metering_awb_avg_r =
        (stat->awb_stat1.metering_awb_avg_r * rgb_gain_conv[0] + 0x80) >> 8; /* Rshift 8bit, index[0] */
    stat->awb_stat1.metering_awb_avg_g =
        (stat->awb_stat1.metering_awb_avg_g * rgb_gain_conv[1] + 0x80) >> 8; /* Rshift 8bit, index[1] */
    stat->awb_stat1.metering_awb_avg_b =
        (stat->awb_stat1.metering_awb_avg_b * rgb_gain_conv[2] + 0x80) >> 8; /* Rshift 8bit, index[2] */
    return TD_SUCCESS;
}

td_s32 isp_drv_be_awb_stat2_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_be_offline_stat *be_stt[],
                                 const td_u16 *rgb_gain_conv, td_u8 blk_num)
{
    td_u32 i, j, k;
    td_u8  block_offset     = 0;
    td_u8  block_zone_width = 0;
    td_u32 col, row, value;
    td_u16 read_r_avg, read_g_avg, read_avg;
    isp_drv_ctx *drv_ctx = TD_NULL;

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);

        col  = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[k].be_awb_zone_cfg.column;
        row  = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[k].be_awb_zone_cfg.row;
        col  = MIN2(col, OT_ISP_AWB_ZONE_ORIG_COLUMN);
        row  = MIN2(row, OT_ISP_AWB_ZONE_ORIG_ROW);

        for (i = 0; i < row; i++) {
            if (k < (col % div_0_to_1(blk_num))) {
                block_zone_width = (col / div_0_to_1(blk_num)) + 1;
            } else {
                block_zone_width = col / div_0_to_1(blk_num);
            }

            for (j = 0; j < block_zone_width; j++) {
                value = be_stt[k]->isp_awb_stat[(i * block_zone_width + j) * 2].u32; /*  index(*2) */
                read_r_avg = (value & 0xFFFF);
                read_g_avg = ((value >> 16) & 0xFFFF); /* Rshift 16bit */
                value = be_stt[k]->isp_awb_stat[(i * block_zone_width + j) * 2 + 1].u32; /* index(*2) */
                read_avg = (value & 0xFFFF);

                stat->awb_stat2.metering_mem_array_count_all[i * col + j + block_offset] =
                    ((value >> 16) & 0xFFFF); /* Rshift 16bit */
                stat->awb_stat2.metering_mem_array_avg_r[i * col + j + block_offset] =
                    (read_r_avg * rgb_gain_conv[0] + 0x80) >> 8; /* index[0] Rshift 8bit */
                stat->awb_stat2.metering_mem_array_avg_g[i * col + j + block_offset] =
                    (read_g_avg * rgb_gain_conv[1] + 0x80) >> 8; /* index[1] Rshift 8bit */
                stat->awb_stat2.metering_mem_array_avg_b[i * col + j + block_offset] =
                    (read_avg   * rgb_gain_conv[2] + 0x80) >> 8; /* index[2] Rshift 8bit */
            }
        }
        block_offset += block_zone_width;
    }

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
td_void isp_drv_read_be_awb_detail_stats(const isp_be_offline_detail_ae_awb_stat *ae_awb_stat, td_s32 r,
    td_s32 c, td_s32 col, ot_isp_detail_awb_stats *stat)
{
    td_s32 i, j, col_temp, row_temp, idx;
    td_u32 value, zone_col;
    td_u16 read_r_avg, read_g_avg, read_avg;
    for (i = 0; i < OT_ISP_AWB_ZONE_ORIG_ROW; i++) {
        for (j = 0; j < OT_ISP_AWB_ZONE_ORIG_COLUMN; j++) {
            zone_col = OT_ISP_AWB_ZONE_ORIG_COLUMN;
            value = ae_awb_stat->isp_detail_awb_stat.isp_awb_stat[(i * zone_col + j)  * 2].u32; /*  index(*2) */
            read_r_avg = (value & 0xFFFF);
            read_g_avg = ((value >> 16) & 0xFFFF); /* Rshift 16bit */
            value = ae_awb_stat->isp_detail_awb_stat.isp_awb_stat[(i * zone_col + j) * 2 + 1].u32; /* index(*2) */
            read_avg = (value & 0xFFFF);

            col_temp = c * OT_ISP_AWB_ZONE_ORIG_COLUMN + j;
            row_temp = r * OT_ISP_AWB_ZONE_ORIG_ROW + i;
            idx = col_temp + row_temp * (OT_ISP_AWB_ZONE_ORIG_COLUMN * col);
            // rgb_gain_conv is 0x100, faceAE not support after drc, no need to div wb_gain
            stat->zone_count_all[idx] = ((value >> 16) & 0xFFFF); /* Rshift 16bit */
            stat->zone_avg_r[idx] = (read_r_avg * 0x100 + 0x80) >> 8; /* index[0] Rshift 8bit */
            stat->zone_avg_g[idx] = (read_g_avg * 0x100 + 0x80) >> 8; /* index[1] Rshift 8bit */
            stat->zone_avg_b[idx] = (read_avg   * 0x100 + 0x80) >> 8; /* index[2] Rshift 8bit */
        }
    }
    return;
}

td_void isp_drv_read_be_detail_stats(isp_drv_ctx *drv_ctx, isp_be_offline_detail_stat *stt_addr,
    ot_isp_detail_stats *detail_stats)
{
    td_u8 i, j, col, row, idx;

    col = drv_ctx->detail_stats_cfg.col;
    row = drv_ctx->detail_stats_cfg.row;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            idx = j + i * col;
            if (drv_ctx->detail_stats_cfg.ctrl.bit1_ae) {
                isp_drv_read_be_ae_detail_stats(&stt_addr->isp_ae_awb_stat[idx], i, j,
                    &detail_stats->ae_stats);
            }
            if (drv_ctx->detail_stats_cfg.ctrl.bit1_awb) {
                isp_drv_read_be_awb_detail_stats(&stt_addr->isp_ae_awb_stat[idx], i, j, col,
                    &detail_stats->awb_stats);
            }
        }
    }
    return;
}
#endif

td_s32 isp_drv_be_awb_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat,
    isp_be_offline_stat *be_stt[], td_u8 blk_num, isp_stat_key stat_key)
{
    td_s32 ret;
    td_u16 rgb_gain_conv[3] = {0}; /* 3 : max index for R,G,B */
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat);

    drv_ctx = isp_drv_get_ctx(vi_pipe);

    isp_drv_cal_awb_gain_conv(drv_ctx, rgb_gain_conv, sizeof(rgb_gain_conv));
    if (stat_key.bit1_awb_stat1) {
        ret  = isp_drv_be_awb_stat1_read(be_stt, blk_num, stat, rgb_gain_conv, sizeof(rgb_gain_conv));
        isp_check_return(vi_pipe, ret, "isp_drv_be_awb_stat1_read");
    }

    if (stat_key.bit1_awb_stat2) {
        ret = isp_drv_be_awb_stat2_read(vi_pipe, stat, be_stt, rgb_gain_conv, blk_num);
        isp_check_return(vi_pipe, ret, "isp_drv_be_awb_stat2_read");
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_be_af_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat,
                                             isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_u8  k;
    td_u8  col_index;
    td_u8  block_offset = 0;
    td_u32 i, j;
    td_u32 col;
    td_u32 row;
    td_u32 be_af_stat_data;
    isp_drv_ctx *drv_ctx   = TD_NULL;
    isp_check_pointer_return(stat);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_return(be_stt[k]);

        col = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[k].be_af_zone_cfg.column;
        row = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[k].be_af_zone_cfg.row;
        col = MIN2(col, OT_ISP_AF_ZONE_COLUMN);
        row = MIN2(row, OT_ISP_AF_ZONE_ROW);

        for (i = 0; i < row; i++) {
            for (j = 0; j < col; j++) {
                col_index = MIN2(j + block_offset, OT_ISP_AF_ZONE_COLUMN);

                be_af_stat_data = be_stt[k]->isp_af_stat[(i * col + j) * 4].u32; /* get array index(*4) */
                stat->be_af_stat.zone_metrics[i][col_index].v1 =
                    (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* Rshift 16bit, get high 16bit */
                stat->be_af_stat.zone_metrics[i][col_index].h1 = (td_u16)(0xFFFF & be_af_stat_data);
                be_af_stat_data = be_stt[k]->isp_af_stat[(i * col + j) * 4 + 1].u32; /* get array index(*4)+1 */
                stat->be_af_stat.zone_metrics[i][col_index].v2 =
                    (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* Rshift 16bit, get high 16bit */
                stat->be_af_stat.zone_metrics[i][col_index].h2 = (td_u16)(0xFFFF & be_af_stat_data);
                be_af_stat_data = be_stt[k]->isp_af_stat[(i * col + j) * 4 + 2].u32; /* get array index(*4)+2 */
                stat->be_af_stat.zone_metrics[i][col_index].hl_cnt =
                    (td_u16)((0xFFFF0000 & be_af_stat_data) >> 16); /* Rshift 16bit, get high 16bit */
                stat->be_af_stat.zone_metrics[i][col_index].y     = (td_u16)(0xFFFF & be_af_stat_data);
            }
        }

        block_offset += col;
    }
#endif
    return TD_SUCCESS;
}

td_s32 isp_drv_be_dehaze_offline_statistics_read(isp_stat *stat, isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_u32 i;
    isp_check_pointer_return(stat);

    for (i = 0; i < blk_num; i++) {
        isp_check_pointer_return(be_stt[i]);

        (td_void)memcpy_s(stat->dehaze_stat.min_dout[i], sizeof(stat->dehaze_stat.min_dout[i]),
            (const u_isp_dehaze_minstat*)be_stt[i]->isp_dehaze_minstat, sizeof(be_stt[i]->isp_dehaze_minstat));

        (td_void)memcpy_s(stat->dehaze_stat.max_stat_dout[i], sizeof(stat->dehaze_stat.max_stat_dout[i]),
            (const u_isp_dehaze_maxstat*)be_stt[i]->isp_dehaze_maxstat, sizeof(be_stt[i]->isp_dehaze_maxstat));
    }

    return TD_SUCCESS;
}

static td_void isp_drv_dpc_offline_calib_info_read(isp_stat *stat, isp_be_offline_stat *be_stt[],
                                                   td_u8 blk_num, td_u64 bit1_dp_stat)
{
    td_u8  k;
    td_u16 j;
    isp_check_pointer_void_return(stat);

    if (bit1_dp_stat != 1) {
        return;
    }

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_void_return(be_stt[k]);

        stat->dp_stat.defect_pixel_count[k] =
            be_stt[k]->isp_dpc_bpt_calib_number_rstt.bits.isp_dpc_bpt_calib_number_rstt;

        for (j = 0; j < stat->dp_stat.defect_pixel_count[k]; j++) {
            stat->dp_stat.defect_pixel_lut[k][j] = be_stt[k]->isp_dpc_bpt_rstt[j].u32;
        }
    }

    return;
}

static td_void isp_drv_flicker_offline_statistics_read(isp_stat *stat, isp_be_offline_stat *be_stt[],
                                                       td_u8 blk_num, td_u64 bit1_flicker)
{
    td_u8  k;
    isp_check_pointer_void_return(stat);

    if (bit1_flicker != 1) {
        return;
    }

    for (k = 0; k < blk_num; k++) {
        isp_check_pointer_void_return(be_stt[k]);
        stat->flicker_stat.cur_diff_gr[k] = be_stt[k]->isp_flick_gr_diff_cur_rstt.bits.isp_flick_gr_diff_cur_rstt;
        stat->flicker_stat.cur_diff_gb[k] = be_stt[k]->isp_flick_gb_diff_cur_rstt.bits.isp_flick_gb_diff_cur_rstt;
        stat->flicker_stat.cur_abs_gr[k]  = be_stt[k]->isp_flick_gr_abs_cur_rstt.bits.isp_flick_gr_abs_cur_rstt;
        stat->flicker_stat.cur_abs_gb[k]  = be_stt[k]->isp_flick_gb_abs_cur_rstt.bits.isp_flick_gb_abs_cur_rstt;

        (td_void)memcpy_s(stat->flicker_stat.g_mean[k], sizeof(stat->flicker_stat.g_mean[k]),
            (const u_isp_flick_gmean*)be_stt[k]->isp_flick_gmean, sizeof(be_stt[k]->isp_flick_gmean));
    }

    return;
}

td_s32 isp_drv_ldci_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat)
{
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    (td_void)memcpy_s(&stat->ldci_stat, sizeof(isp_ldci_stat),
                      &drv_ctx->ldci_stt_addr.ldci_stat, sizeof(isp_ldci_stat));

    return TD_SUCCESS;
}

td_void isp_drv_pre_be_offline_stats_read(isp_stat_key stat_key, isp_stat *stat,
    isp_be_offline_stat *pre_stt[], td_u8 blk_num)
{
    isp_drv_dpc_offline_calib_info_read(stat, pre_stt, blk_num, stat_key.bit1_dp_stat);
    isp_drv_flicker_offline_statistics_read(stat, pre_stt, blk_num, stat_key.bit1_flicker);
}

static td_void isp_drv_be_check_sum_offline_read(isp_stat *stat,
    isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_u32 k, sum_y_low32;
    td_u64 sum_y_high32;
    for (k = 0; k < blk_num; k++) {
        sum_y_low32 = be_stt[k]->isp_y_sum0_rstt.u32;
        sum_y_high32 = be_stt[k]->isp_y_sum1_rstt.u32;
        stat->sum_stat.sum_y[k] = ((sum_y_high32 << 32)) + sum_y_low32; // left shift 32
    }
}


static td_void isp_drv_post_be_offline_stats_read(ot_vi_pipe vi_pipe, isp_stat_key stat_key, isp_stat *stat,
    isp_be_offline_stat *be_stt[], td_u8 blk_num)
{
    td_s32 ret;
    if (stat_key.bit1_be_ae_global_stat) {
        isp_drv_be_ae_offline_global_stat_read(stat, be_stt, blk_num);
    }

    if (stat_key.bit1_be_ae_local_stat) {
        isp_drv_be_ae_offline_local_stat_read(stat, be_stt, blk_num);
    }
    if (stat_key.bit1_mg_stat) {
        isp_drv_be_mg_offline_statistics_read(stat, be_stt, blk_num);
    }

    /* BE AWB statistics */
    ret = isp_drv_be_awb_offline_statistics_read(vi_pipe, stat,  be_stt, blk_num, stat_key);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp[%d] be_awb_offline_statistics_read failed!\n", vi_pipe);
    }

    /* BE AF statistics */
    if (stat_key.bit1_be_af_stat) {
        isp_drv_be_af_offline_statistics_read(vi_pipe, stat, be_stt, blk_num);
    }

    if (stat_key.bit1_dehaze) {
        isp_drv_be_dehaze_offline_statistics_read(stat, be_stt, blk_num);
    }

    if (isp_drv_get_ldci_tpr_flt_en(vi_pipe) == TD_TRUE) {
        isp_drv_ldci_offline_statistics_read(vi_pipe, stat);
    }

    if (stat_key.bit1_check_sum) {
        isp_drv_be_check_sum_offline_read(stat, be_stt, blk_num);
    }
}

static td_s32 isp_drv_be_offline_pre_stt_get(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    isp_be_offline_stat *pre_stt[], td_u8 blk_num)
{
    td_u8  k;
    td_u32 pre_ready_idx;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_u64  size;
    isp_be_offline_stat_reg_type *stat_tmp = TD_NULL;
    ot_unused(vi_pipe);

    pre_ready_idx = drv_ctx->be_off_stt_buf.last_ready_be_stt_idx[PRE_VIPORC_ID];
    if (pre_ready_idx != 0 && pre_ready_idx != 1) {
        isp_err_trace("pre_be_stt not ready!\n");
        return TD_FAILURE;
    }

    for (k = 0; k < blk_num; k++) {
        vir_addr = drv_ctx->be_off_stt_buf.be_stt_buf[pre_ready_idx].virt_addr[k];
        phy_addr = drv_ctx->be_off_stt_buf.be_stt_buf[pre_ready_idx].phys_addr[k];
        size     =  sizeof(isp_be_offline_stat_reg_type);

        cmpi_invalid_cache_byaddr(vir_addr, phy_addr, size);

        stat_tmp = (isp_be_offline_stat_reg_type *)drv_ctx->be_off_stt_buf.be_stt_buf[pre_ready_idx].virt_addr[k];
        pre_stt[k] = &stat_tmp->be_stat;
        if (pre_stt[k] == TD_NULL) {
            isp_err_trace("pre_be_stt is TD_NULL point\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 isp_drv_be_offline_post_stt_get(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    isp_be_offline_stat *post_stt[], td_u8 blk_num)
{
    td_u8  k;
    td_u32 post_ready_idx;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_u64  size;
    isp_be_offline_stat_reg_type *stat_tmp = TD_NULL;
    ot_unused(vi_pipe);
    post_ready_idx = drv_ctx->be_off_stt_buf.last_ready_be_stt_idx[POST_VIPORC_ID];
    if (post_ready_idx != 0 && post_ready_idx != 1) {
        isp_err_trace("post_be_stt not ready!\n");
        return TD_FAILURE;
    }

    for (k = 0; k < blk_num; k++) {
        vir_addr = drv_ctx->be_off_stt_buf.be_stt_buf[post_ready_idx].virt_addr[k];
        phy_addr = drv_ctx->be_off_stt_buf.be_stt_buf[post_ready_idx].phys_addr[k];
        size     =  sizeof(isp_be_offline_stat_reg_type);

        cmpi_invalid_cache_byaddr(vir_addr, phy_addr, size);

        stat_tmp = (isp_be_offline_stat_reg_type *)drv_ctx->be_off_stt_buf.be_stt_buf[post_ready_idx].virt_addr[k];
        post_stt[k] = &stat_tmp->be_stat;
        if (post_stt[k] == TD_NULL) {
            isp_err_trace("post_be_stt is TD_NULL point\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 isp_drv_pre_online_post_offline_post_stt_get(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    isp_be_offline_stat *post_stt[], td_u8 blk_num)
{
    td_u8  k;
    td_u32 post_ready_idx;
    td_void *vir_addr = TD_NULL;
    td_phys_addr_t  phy_addr;
    td_ulong size;
    isp_be_offline_stt_attr *pre_on_post_off_stt_buf = TD_NULL;
    ot_unused(vi_pipe);

    post_ready_idx = drv_ctx->be_pre_on_post_off_stt_buf.last_ready_be_stt_idx[POST_VIPORC_ID];
    if (post_ready_idx != 0 && post_ready_idx != 1) {
        isp_err_trace("post_be_stt not ready!\n");
        return TD_FAILURE;
    }
    pre_on_post_off_stt_buf = &drv_ctx->be_pre_on_post_off_stt_buf;
    for (k = 0; k < blk_num; k++) {
        vir_addr = pre_on_post_off_stt_buf->be_stt_buf[post_ready_idx].virt_addr[k];
        phy_addr = pre_on_post_off_stt_buf->be_stt_buf[post_ready_idx].phys_addr[k];
        size     =  sizeof(isp_be_offline_stat);

        cmpi_invalid_cache_byaddr(vir_addr, (td_ulong)phy_addr, size);

        post_stt[k] = (isp_be_offline_stat *)pre_on_post_off_stt_buf->be_stt_buf[post_ready_idx].virt_addr[k];
        if (post_stt[k] == TD_NULL) {
            isp_err_trace("post_be_stt is TD_NULL point\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 isp_drv_pre_online_post_offline_pre_stats_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_s32 ret;
    isp_stat *stat = TD_NULL;
    isp_stat_key stat_key;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    stat = (isp_stat *)stat_info->virt_addr;
    if (stat == TD_NULL) {
        return TD_FAILURE;
    }

    stat_key.key = stat_info->stat_key.key;

    ret = isp_drv_pre_be_apb_statistics_read(vi_pipe, stat, stat_key);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_s32 isp_drv_pre_online_post_offline_post_stats_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 blk_num;
    td_s32 ret;
    isp_stat_key stat_key;
    isp_stat *stat = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_offline_stat *be_stt[OT_ISP_STRIPING_MAX_NUM] = { [0 ...(OT_ISP_STRIPING_MAX_NUM - 1)] = TD_NULL };

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);
    stat = (isp_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_num = isp_drv_get_block_num(vi_pipe);
    blk_num = div_0_to_1(blk_num);

    stat->be_update = TD_FALSE;
    ret = isp_drv_pre_online_post_offline_post_stt_get(vi_pipe, drv_ctx, be_stt, blk_num);
    isp_check_return(vi_pipe, ret, "pre_online_post_offline_post_stt_get");
    stat->be_update = TD_TRUE;

    if (stat->be_update == TD_FALSE) {
        drv_ctx->drv_dbg_info.isp_be_sta_lost++;
    }

    stat_key.key = stat_info->stat_key.key;

    isp_drv_post_be_offline_stats_read(vi_pipe, stat_key, stat, be_stt, blk_num);
    return TD_SUCCESS;
}
static td_void isp_drv_pre_be_offline_stat_key(isp_stat_key *stat_key, isp_be_stat_valid stat_valid)
{
    stat_key->bit1_dp_stat &= stat_valid.bits.bit_dp_stat;
    stat_key->bit1_flicker &= stat_valid.bits.bit_flicker;
}

static td_s32 isp_drv_be_offline_pre_stats_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 blk_num;
    td_s32 ret;
    isp_stat_key stat_key;
    isp_stat *stat = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_offline_stat *pre_stt[OT_ISP_STRIPING_MAX_NUM] = { [0 ...(OT_ISP_STRIPING_MAX_NUM - 1)] = TD_NULL };

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    stat = (isp_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_num = isp_drv_get_block_num(vi_pipe);
    blk_num = div_0_to_1(blk_num);

    stat->be_update = TD_FALSE;
    ret = isp_drv_be_offline_pre_stt_get(vi_pipe, drv_ctx, pre_stt, blk_num);
    isp_check_return(vi_pipe, ret, "isp_drv_be_offline_pre_stt_get");
    stat->be_update = TD_TRUE;

    if (stat->be_update == TD_FALSE) {
        drv_ctx->drv_dbg_info.isp_be_sta_lost++;
    }

    stat_key.key = stat_info->stat_key.key;
    /* set temp stat_key. the invalid stats is not right, don't read the stats */
    isp_drv_pre_be_offline_stat_key(&stat_key, drv_ctx->be_off_stt_buf.stat_valid);
    isp_drv_pre_be_offline_stats_read(stat_key, stat, pre_stt, blk_num);

    return TD_SUCCESS;
}
static td_void isp_drv_post_be_offline_stat_key(isp_stat_key *stat_key, isp_be_stat_valid stat_valid)
{
    stat_key->bit1_be_ae_global_stat &= stat_valid.bits.bit_be_ae_stat;
    stat_key->bit1_be_ae_local_stat &= stat_valid.bits.bit_be_ae_stat;
    stat_key->bit1_mg_stat &= stat_valid.bits.bit_mg_stat;
    stat_key->bit1_awb_stat1 &= stat_valid.bits.bit_awb_stat;
    stat_key->bit1_awb_stat2 &= stat_valid.bits.bit_awb_stat;
    stat_key->bit1_be_af_stat &= stat_valid.bits.bit_be_af_stat;
    stat_key->bit1_dehaze &= stat_valid.bits.bit_dehaze_stat;
}

static td_s32 isp_drv_be_offline_post_stats_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_u8 blk_num;
    td_s32 ret;
    isp_stat_key stat_key;
    isp_stat *stat = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_offline_stat *be_stt[OT_ISP_STRIPING_MAX_NUM] = { [0 ...(OT_ISP_STRIPING_MAX_NUM - 1)] = TD_NULL };

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);
    stat = (isp_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_num = isp_drv_get_block_num(vi_pipe);
    blk_num = div_0_to_1(blk_num);

    stat->be_update = TD_FALSE;
    ret = isp_drv_be_offline_post_stt_get(vi_pipe, drv_ctx, be_stt, blk_num);
    isp_check_return(vi_pipe, ret, "isp_drv_be_offline_post_stt_get");
    stat->be_update = TD_TRUE;

    if (stat->be_update == TD_FALSE) {
        drv_ctx->drv_dbg_info.isp_be_sta_lost++;
    }

    stat_key.key = stat_info->stat_key.key;
    // set temp stat_key, would't read the invalid stats
    isp_drv_post_be_offline_stat_key(&stat_key, drv_ctx->be_off_stt_buf.stat_valid);
    isp_drv_post_be_offline_stats_read(vi_pipe, stat_key, stat, be_stt, blk_num);

    return TD_SUCCESS;
}

td_s32 isp_drv_be_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_pre_online_post_offline_pre_stats_read(vi_pipe, stat_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        ret = isp_drv_pre_online_post_offline_post_stats_read(vi_pipe, stat_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else {
        ret = isp_drv_be_offline_pre_stats_read(vi_pipe, stat_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        ret = isp_drv_be_offline_post_stats_read(vi_pipe, stat_info);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_drv_read_af_offline_other_stats_end_int(isp_drv_ctx *drv_ctx, isp_stat_info *stat_info,
    isp_stat_key stat_key, isp_be_stat_valid stat_valid)
{
    td_u32 i;
    isp_stat *other_stat;
    isp_stat *stat = (isp_stat *)stat_info->virt_addr;

    for (i = 0; i < MAX_ISP_STAT_BUF_NUM; i++) { /* only 2 bufs, find other buf */
        if (drv_ctx->statistics_buf.node[i].stat_info.phy_addr != stat_info->phy_addr) {
            break;
        }
    }
    if (i >= MAX_ISP_STAT_BUF_NUM) {
        isp_err_trace("err, no other buf\n");
        return;
    }

    other_stat = (isp_stat *)drv_ctx->statistics_buf.node[i].stat_info.virt_addr;
    if (stat_key.bit1_be_af_stat == 1 && stat_valid.bits.bit_be_af_stat == 0) {
        (td_void)memcpy_s(&stat->be_af_stat, sizeof(stat->be_af_stat),
            &other_stat->be_af_stat, sizeof(other_stat->be_af_stat));
    }
    return;
}


td_void isp_drv_read_af_offline_stats_end_int(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u8  blk_num,
    isp_stat_info *stat_info, isp_stat_key stat_key)
{
    td_s32 ret;
    isp_stat_key stat_key_temp;
    isp_be_offline_stat *be_stt[OT_ISP_STRIPING_MAX_NUM] = { [0 ...(OT_ISP_STRIPING_MAX_NUM - 1)] = TD_NULL };
    isp_stat *stat = (isp_stat *)stat_info->virt_addr;
    if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_pre_online_post_offline_post_stt_get(vi_pipe, drv_ctx, be_stt, blk_num);
        if (ret != TD_SUCCESS) {
            return;
        }
    } else {
        ret = isp_drv_be_offline_post_stt_get(vi_pipe, drv_ctx, be_stt, blk_num);
        if (ret != TD_SUCCESS) {
            return;
        }
    }
    stat_key_temp.key = stat_key.key;
    stat_key_temp.bit1_be_af_stat &= drv_ctx->be_off_stt_buf.stat_valid.bits.bit_be_af_stat;

    if (stat_key_temp.bit1_be_af_stat) {
        isp_drv_be_af_offline_statistics_read(vi_pipe, stat, be_stt, blk_num);
    }
    isp_drv_read_af_offline_other_stats_end_int(drv_ctx, stat_info, stat_key, drv_ctx->be_off_stt_buf.stat_valid);
}

static td_s32 isp_drv_be_offline_ae_stitch_global_stats_read(isp_stat *stat, const isp_drv_ctx *drv_ctx,
                                                             isp_stitch_sync_be_stats *be_stitch_stat)
{
    td_s32 i;
    td_s32 k;
    td_u64 global_avg_sum_r = 0;
    td_u64 global_avg_sum_gr = 0;
    td_u64 global_avg_sum_gb = 0;
    td_u64 global_avg_sum_b = 0;
    td_u32 pixel_weight_tmp;
    td_u32 pixel_weight;
    isp_stitch_stat_reg_type *ptmp = TD_NULL;
    isp_be_offline_ae_stat_type *ae_atat = TD_NULL;
    isp_check_pointer_return(stat);
    isp_check_pointer_return(drv_ctx);

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        ptmp = be_stitch_stat->sync_be_stt[k];
        ae_atat = &ptmp->isp_ae_atat;

        if (k == 0) {
            for (i = 0; i < OT_ISP_HIST_NUM; i++) {
                stat->stitch_stat.be_ae_stat1.histogram_mem_array[i] = ae_atat->isp_ae_hist[i].u32;
            }

            pixel_weight_tmp = ae_atat->isp_ae_count_stat_rstt.bits.isp_ae_count_pixels_stt;
            stat->stitch_stat.be_ae_stat1.pixel_weight = pixel_weight_tmp;
            stat->stitch_stat.be_ae_stat1.pixel_count = ae_atat->isp_ae_total_stat_rstt.bits.isp_ae_total_pixels_stt;
        } else {
            for (i = 0; i < OT_ISP_HIST_NUM; i++) {
                stat->stitch_stat.be_ae_stat1.histogram_mem_array[i] += ae_atat->isp_ae_hist[i].u32;
            }

            pixel_weight_tmp = ae_atat->isp_ae_count_stat_rstt.bits.isp_ae_count_pixels_stt;
            stat->stitch_stat.be_ae_stat1.pixel_weight += pixel_weight_tmp;
            stat->stitch_stat.be_ae_stat1.pixel_count += ae_atat->isp_ae_total_stat_rstt.bits.isp_ae_total_pixels_stt;
        }

        global_avg_sum_r += ((td_u64)ae_atat->isp_ae_total_r_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
        global_avg_sum_gr += ((td_u64)ae_atat->isp_ae_total_gr_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
        global_avg_sum_gb += ((td_u64)ae_atat->isp_ae_total_gb_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
        global_avg_sum_b += ((td_u64)ae_atat->isp_ae_total_b_aver_rstt.u32) * ((td_u64)pixel_weight_tmp);
    }

    pixel_weight = div_0_to_1(stat->stitch_stat.be_ae_stat1.pixel_weight);

    stat->stitch_stat.be_ae_stat2.global_avg_r  = osal_div_u64(global_avg_sum_r, pixel_weight);
    stat->stitch_stat.be_ae_stat2.global_avg_gr = osal_div_u64(global_avg_sum_gr, pixel_weight);
    stat->stitch_stat.be_ae_stat2.global_avg_gb = osal_div_u64(global_avg_sum_gb, pixel_weight);
    stat->stitch_stat.be_ae_stat2.global_avg_b  = osal_div_u64(global_avg_sum_b, pixel_weight);

    return TD_SUCCESS;
}

static td_s32 isp_drv_be_offline_ae_stitch_local_stats_read(isp_stat *stat, isp_drv_ctx *drv_ctx,
                                                            isp_stitch_sync_be_stats *be_stitch_stat)
{
    td_s32 i, j, k;
    td_u32 ave_mem;
    ot_vi_pipe vi_pipe_bind;

    isp_stitch_stat_reg_type *ptmp = TD_NULL;
    isp_be_offline_ae_stat_type *ae_atat = TD_NULL;
    isp_check_pointer_return(stat);
    isp_check_pointer_return(drv_ctx);

    for (k = 0; k < drv_ctx->stitch_attr.stitch_pipe_num; k++) {
        ptmp    = be_stitch_stat->sync_be_stt[k];
        ae_atat = &ptmp->isp_ae_atat;
        vi_pipe_bind = drv_ctx->stitch_attr.stitch_bind_id[k];

        if (vi_pipe_bind >= OT_ISP_MAX_PIPE_NUM) {
            continue;
        }

        for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
                ave_mem = ae_atat->isp_ae_aver_r_gr[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
                stat->stitch_stat.be_ae_stat3.zone_avg[vi_pipe_bind][i][j][0] = /* array index[0] */
                    (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16 */
                stat->stitch_stat.be_ae_stat3.zone_avg[vi_pipe_bind][i][j][1] = /* array index[1] */
                    (td_u16)((ave_mem & 0xFFFF));

                ave_mem = ae_atat->isp_ae_aver_gb_b[i * OT_ISP_AE_ZONE_COLUMN + j].u32;
                stat->stitch_stat.be_ae_stat3.zone_avg[vi_pipe_bind][i][j][2] = /* array index[2] */
                    (td_u16)((ave_mem & 0xFFFF0000) >> 16); /* Rshift 16 */
                stat->stitch_stat.be_ae_stat3.zone_avg[vi_pipe_bind][i][j][3] = /* array index[3] */
                    (td_u16)((ave_mem & 0xFFFF));
            }
        }
    }

    return TD_SUCCESS;
}

static td_void isp_drv_be_offline_awb_stitch_statistics_sub_read(isp_stat *stat, isp_stitch_stat_reg_type *ptmp,
                                                                 isp_drv_ctx *drv_ctx, td_s32 k)
{
    td_u8 stitch_num;
    td_u16 block_zone_width;
    td_u16 stitch_width;
    td_u32 i, j;
    td_u32 col, row, zone_bin, m;
    td_u32 stat_gr, stat_count_b;
    td_u16 w_start_addr, r_start_addr;

    stitch_num = drv_ctx->stitch_attr.stitch_pipe_num;
    col = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[0].be_awb_zone_cfg.column;
    row = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[0].be_awb_zone_cfg.row;
    col = MIN2(col, OT_ISP_AWB_ZONE_ORIG_COLUMN);
    row = MIN2(row, OT_ISP_AWB_ZONE_ORIG_ROW);
    zone_bin = drv_ctx->kernel_cfg[drv_ctx->reg_cfg_info_flag].alg_kernel_cfg[0].be_awb_zone_cfg.zone_bin;

    block_zone_width = col;
    stitch_width = block_zone_width * stitch_num;

    stat->stitch_stat.awb_stat2.zone_row = row;
    stat->stitch_stat.awb_stat2.zone_col = stitch_width;

    for (i = 0; i < row; i++) {
        for (j = 0; j < block_zone_width; j++) {
            w_start_addr = (stitch_width * i + block_zone_width * k + j) * zone_bin;
            r_start_addr = (block_zone_width * i + j) * 2 * zone_bin;  /* get array index(*2) */

            for (m = 0; m < zone_bin; m++) {
                stat_gr = ptmp->isp_awb_stat[r_start_addr + (m * 2) + 0].u32;  /* get array index(*2) */
                stat_count_b = ptmp->isp_awb_stat[r_start_addr + (m * 2) + 1].u32;  /* get array index(*2) */

                stat->stitch_stat.awb_stat2.metering_mem_array_avg_r[w_start_addr + m] =
                    (stat_gr & 0xFFFF);
                stat->stitch_stat.awb_stat2.metering_mem_array_avg_g[w_start_addr + m] =
                    ((stat_gr >> 16) & 0xFFFF); /* Rshift 16 */
                stat->stitch_stat.awb_stat2.metering_mem_array_avg_b[w_start_addr + m] =
                    (stat_count_b & 0xFFFF);
                stat->stitch_stat.awb_stat2.metering_mem_array_count_all[w_start_addr + m] =
                    ((stat_count_b >> 16) & 0xFFFF); /* Rshift 16 */
            }
        }
    }
}

static td_s32 isp_drv_be_offline_awb_stitch_stats_read(isp_stat *stat, isp_drv_ctx *drv_ctx,
    isp_stat_key un_statkey, isp_stitch_sync_be_stats *be_stitch_stat)
{
    td_u8  stitch_num;
    td_s32 k;
    isp_stitch_stat_reg_type *ptmp = TD_NULL;

    isp_check_pointer_return(stat);
    isp_check_pointer_return(drv_ctx);

    if (!un_statkey.bit1_awb_stat2) {
        return TD_SUCCESS;
    }

    stitch_num = drv_ctx->stitch_attr.stitch_pipe_num;

    for (k = 0; k < stitch_num; k++) {
        ptmp = be_stitch_stat->sync_be_stt[k];
        isp_drv_be_offline_awb_stitch_statistics_sub_read(stat, ptmp, drv_ctx, k);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_be_offline_stitch_stt_get(ot_vi_pipe stitch_main_pipe, isp_stat *stat,
                                                isp_stitch_sync_be_stats *be_stitch)
{
    td_u32 i, sync_ok_idx;
    td_s32 working_idx, ready_idx;
    isp_drv_ctx *drv_ctx = TD_NULL;

    drv_ctx = isp_drv_get_ctx(stitch_main_pipe);
    stat->be_update = TD_FALSE;
    working_idx = isp_drv_stitch_sync_get_working_index(drv_ctx);
    ready_idx   = 1 - working_idx;
    if (drv_ctx->be_off_stitch_stt_buf.is_ready[ready_idx] == TD_FALSE) {
        isp_debug_trace("ISP[%d] stitch sync be stt is not ready\n", stitch_main_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        sync_ok_idx = drv_ctx->be_off_stitch_stt_buf.sync_ok_index[ready_idx][i];
        be_stitch->sync_be_stt[i] = drv_ctx->be_off_stitch_stt_buf.sync_be_buf[i].stitch_stt_reg[sync_ok_idx];
    }

    stat->be_update = TD_TRUE;
    return TD_SUCCESS;
}

td_s32 isp_drv_be_offline_stitch_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info)
{
    td_s32 ret, ready_idx;
    isp_stat_key un_statkey;
    isp_stat *stat = TD_NULL;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_stitch_sync_be_stats be_stitch_stat;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stat_info);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if ((drv_ctx->stitch_attr.main_pipe != TD_TRUE) || (drv_ctx->stitch_attr.stitch_enable != TD_TRUE)) {
        return TD_SUCCESS;
    }
    un_statkey.key = stat_info->stat_key.key;

    stat = (isp_stat *)stat_info->virt_addr;
    isp_check_pointer_return(stat);

    ret = isp_drv_be_offline_stitch_stt_get(vi_pipe, stat, &be_stitch_stat);
    if (stat->be_update == TD_FALSE) {
        drv_ctx->drv_dbg_info.isp_be_sta_lost++;
    }
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (un_statkey.bit1_be_ae_stitch_global_stat) {
        ret = isp_drv_be_offline_ae_stitch_global_stats_read(stat, drv_ctx, &be_stitch_stat);
        isp_check_return(vi_pipe, ret, "isp_drv_be_offline_ae_stitch_global_stats_read");
    }

    if (un_statkey.bit1_be_ae_stitch_local_stat) {
        ret = isp_drv_be_offline_ae_stitch_local_stats_read(stat, drv_ctx, &be_stitch_stat);
        isp_check_return(vi_pipe, ret, "isp_drv_be_offline_ae_stitch_local_stats_read");
    }

    /* BE AWB statistics */
    ret = isp_drv_be_offline_awb_stitch_stats_read(stat, drv_ctx, un_statkey, &be_stitch_stat);
    isp_check_return(vi_pipe, ret, "isp_drv_be_offline_awb_stitch_stats_read");

    ready_idx = 1 - isp_drv_stitch_sync_get_working_index(drv_ctx);
    isp_drv_stitch_sync_info_ready(drv_ctx, ready_idx, TD_FALSE);
    return TD_SUCCESS;
}

static td_void isp_drv_get_fe_ae_glo_statistics(td_u32 pipe_num, ot_isp_ae_stats *ae_stat,
    isp_stat *isp_act_stat)
{
    td_u32 k, i;
    td_s32 ret;

    for (k = 0; k < pipe_num; k++) {
        ret = memcpy_s(ae_stat->fe_hist1024_value[k], sizeof(ae_stat->fe_hist1024_value[k]),
            isp_act_stat->fe_ae_stat1.histogram_mem_array[k], sizeof(isp_act_stat->fe_ae_stat1.histogram_mem_array[k]));
        isp_check_eok_void(ret);

        ae_stat->fe_global_avg[k][OT_ISP_CHN_R]  = isp_act_stat->fe_ae_stat2.global_avg_r[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_GR] = isp_act_stat->fe_ae_stat2.global_avg_gr[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_GB] = isp_act_stat->fe_ae_stat2.global_avg_gb[k];
        ae_stat->fe_global_avg[k][OT_ISP_CHN_B]  = isp_act_stat->fe_ae_stat2.global_avg_b[k];
    }

    for (; k < OT_ISP_WDR_MAX_FRAME_NUM; k++) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            ae_stat->fe_global_avg[k][i] = 0;
        }
        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            ae_stat->fe_hist1024_value[k][i] = 0;
        }
    }
}

static td_void isp_drv_get_fe_ae_loc_statistics(td_u32 pipe_num, ot_isp_ae_stats *ae_stat, isp_stat *act_stat)
{
    size_t size;
    td_s32 ret;

    size = OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_AE_ZONE_ROW *
        OT_ISP_AE_ZONE_COLUMN * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);
    (td_void)memset_s(ae_stat->fe_zone_avg, size, 0, size);

    ret = memcpy_s(ae_stat->fe_zone_avg, size, act_stat->fe_ae_stat3.zone_avg, size);
    isp_check_eok_void(ret);
}

static td_void isp_drv_get_be_ae_glo_statistics(ot_isp_ae_stats *ae_stat, isp_stat *isp_act_stat)
{
    td_s32 ret;
    ret = memcpy_s(ae_stat->be_hist1024_value, sizeof(ae_stat->be_hist1024_value),
        isp_act_stat->be_ae_stat1.histogram_mem_array, sizeof(isp_act_stat->be_ae_stat1.histogram_mem_array));
    isp_check_eok_void(ret);

    ae_stat->be_global_avg[OT_ISP_CHN_R]  = isp_act_stat->be_ae_stat2.global_avg_r;
    ae_stat->be_global_avg[OT_ISP_CHN_GR] = isp_act_stat->be_ae_stat2.global_avg_gr;
    ae_stat->be_global_avg[OT_ISP_CHN_GB] = isp_act_stat->be_ae_stat2.global_avg_gb;
    ae_stat->be_global_avg[OT_ISP_CHN_B]  = isp_act_stat->be_ae_stat2.global_avg_b;
}

static td_void isp_drv_get_be_ae_loc_statistics(ot_isp_ae_stats *ae_stat, isp_stat *isp_act_stat)
{
    td_s32 ret;
    ret = memcpy_s(ae_stat->be_zone_avg, sizeof(ae_stat->be_zone_avg), isp_act_stat->be_ae_stat3.zone_avg,
        sizeof(isp_act_stat->be_ae_stat3.zone_avg));
    isp_check_eok_void(ret);
}

td_s32 isp_drv_ae_stats_kernel_get(ot_vi_pipe vi_pipe, ot_isp_ae_stats *ae_stat)
{
    unsigned long flags;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_stat *stat = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    isp_stat_key stat_key;

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    if (ae_stat == TD_NULL) {
        isp_err_trace("get statistic active buffer err, focus_stat is TD_NULL!\n");
        return TD_FAILURE;
    }

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    if (!drv_ctx->statistics_buf.act_stat) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_info_trace("get statistic active buffer err, stat not ready!\n");
        return TD_FAILURE;
    }

    if (!drv_ctx->statistics_buf.act_stat->virt_addr) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_err_trace("get statistic active buffer err, virt_addr is TD_NULL!\n");
        return TD_FAILURE;
    }

    stat = (isp_stat *)drv_ctx->statistics_buf.act_stat->virt_addr;
    stat_key = drv_ctx->statistics_buf.act_stat->stat_key;

    if (stat_key.bit1_fe_ae_global_stat && drv_ctx->wdr_attr.is_mast_pipe) {
        isp_drv_get_fe_ae_glo_statistics(drv_ctx->wdr_attr.pipe_num, ae_stat, stat);
    }
    if (stat_key.bit1_fe_ae_local_stat && drv_ctx->wdr_attr.is_mast_pipe) {
        isp_drv_get_fe_ae_loc_statistics(drv_ctx->wdr_attr.pipe_num, ae_stat, stat);
    }
    if (stat_key.bit1_be_ae_global_stat) {
        isp_drv_get_be_ae_glo_statistics(ae_stat, stat);
    }
    if (stat_key.bit1_be_ae_local_stat) {
        isp_drv_get_be_ae_loc_statistics(ae_stat, stat);
    }
    ae_stat->pts = stat->frame_pts;

    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    return TD_SUCCESS;
}

td_s32 isp_drv_af_stats_kernel_get(ot_vi_pipe vi_pipe, ot_isp_drv_af_statistics *focus_stat)
{
    unsigned long flags;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_stat *stat = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    drv_ctx = isp_drv_get_ctx(vi_pipe);

    if (focus_stat == TD_NULL) {
        isp_err_trace("get statistic active buffer err, focus_stat is TD_NULL!\n");
        return TD_FAILURE;
    }

    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    if (!drv_ctx->statistics_buf.act_stat) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_info_trace("get statistic active buffer err, stat not ready!\n");
        return TD_FAILURE;
    }

    if (!drv_ctx->statistics_buf.act_stat->virt_addr) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_err_trace("get statistic active buffer err, virt_addr is TD_NULL!\n");
        return TD_FAILURE;
    }

    stat = (isp_stat *)drv_ctx->statistics_buf.act_stat->virt_addr;
    (td_void)memcpy_s(&focus_stat->fe_af_stat, sizeof(ot_isp_drv_fe_focus_statistics), &stat->fe_af_stat,
        sizeof(ot_isp_drv_fe_focus_statistics));
    (td_void)memcpy_s(&focus_stat->be_af_stat, sizeof(ot_isp_drv_be_focus_statistics), &stat->be_af_stat,
        sizeof(ot_isp_drv_be_focus_statistics));
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
}
