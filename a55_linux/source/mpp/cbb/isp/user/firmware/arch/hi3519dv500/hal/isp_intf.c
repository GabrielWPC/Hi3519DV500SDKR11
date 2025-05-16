/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_intf.h"
#include "ot_mpi_sys_mem.h"
#include "ot_common_vi.h"
#include "ot_common_isp.h"
#include "ot_common_3a.h"
#include "ot_common_ae.h"
#include "ot_common_awb.h"
#include "isp_main.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "ot_math.h"

static td_void isp_calc_grid_info(td_u16 wdith, td_u16 start_pox, td_u16 block_num, td_u16 *grid_info)
{
    td_u16 i;
    td_u16 integer;
    td_u16 remainder;

    integer   = wdith / div_0_to_1(block_num);
    remainder = wdith % div_0_to_1(block_num);
    grid_info[0] = start_pox;
    for (i = 1; i < block_num; i++) {
        if (remainder > 0) {
            grid_info[i] = grid_info[i - 1] + integer + 1;
            remainder = remainder - 1;
        } else {
            grid_info[i] = grid_info[i - 1] + integer;
        }
    }

    return;
}
static td_u32 isp_get_striping_active_img_start(td_u8 block_index, isp_working_mode *isp_work_mode)
{
    td_u32 over_lap;
    td_u32 block_start;

    over_lap = isp_work_mode->over_lap;
    if (block_index == 0) {
        block_start = isp_work_mode->block_rect[block_index].x;
    } else {
        block_start = isp_work_mode->block_rect[block_index].x + over_lap;
    }

    return block_start;
}

static td_u32 isp_get_striping_active_img_width(td_u8 block_index, isp_working_mode *isp_work_mode)
{
    td_u32 block_width;
    td_u32 over_lap;
    td_u8  block_num;

    over_lap    = isp_work_mode->over_lap;
    block_width = isp_work_mode->block_rect[block_index].width;
    block_num   = isp_work_mode->block_num;

    if ((block_index == 0) || (block_index == (block_num - 1))) { /* first block and last block */
        block_width = block_width - over_lap;
    } else {
        block_width = block_width - over_lap * 2; /*  overlap * 2 */
    }
    return block_width;
}

static td_u32 isp_get_striping_grid_x_info(td_u16 *grid_pos, td_u16 grid_num, isp_working_mode *isp_work_mode)
{
    td_u8  i;
    td_u16 start;
    td_u16 width;
    td_u16 div_num;
    td_u16 index = 0;

    for (i = 0; i < isp_work_mode->block_num; i++) {
        start = isp_get_striping_active_img_start(i, isp_work_mode);
        width = isp_get_striping_active_img_width(i, isp_work_mode);

        if (i < grid_num % div_0_to_1(isp_work_mode->block_num)) {
            div_num = grid_num / div_0_to_1(isp_work_mode->block_num) + 1;
        } else {
            div_num = grid_num / div_0_to_1(isp_work_mode->block_num);
        }

        isp_calc_grid_info(width, start, div_num, &(grid_pos[index]));
        index = index + div_num;
    }
    return TD_SUCCESS;
}

static td_void isp_calc_ae_fe_grid_info(ot_vi_pipe vi_pipe, ot_isp_ae_grid_info *fe_grid_info)
{
    td_bool crop_en;
    td_u16  img_total_width, img_total_height;
    td_u16  img_start_x, img_start_y;

    crop_en = ot_ext_system_ae_fe_crop_en_read(vi_pipe);
    if (crop_en == TD_TRUE) {
        img_start_x      = ot_ext_system_ae_fe_crop_x_read(vi_pipe);
        img_start_y      = ot_ext_system_ae_fe_crop_y_read(vi_pipe);
        img_total_width  = ot_ext_system_ae_fe_crop_width_read(vi_pipe);
        img_total_height = ot_ext_system_ae_fe_crop_height_read(vi_pipe);
    } else {
        img_start_x      = 0;
        img_start_y      = 0;
        img_total_width  = ot_ext_sync_total_width_read(vi_pipe);
        img_total_height = ot_ext_sync_total_height_read(vi_pipe);
    }

    isp_calc_grid_info(img_total_width,  img_start_x, OT_ISP_AE_ZONE_COLUMN, fe_grid_info->grid_x_pos);
    isp_calc_grid_info(img_total_height, img_start_y, OT_ISP_AE_ZONE_ROW, fe_grid_info->grid_y_pos);

    fe_grid_info->grid_x_pos[OT_ISP_AE_ZONE_COLUMN] = img_start_x + img_total_width - 1;
    fe_grid_info->grid_y_pos[OT_ISP_AE_ZONE_ROW]    = img_start_y + img_total_height - 1;
    fe_grid_info->status = 1;
}

static td_s32 isp_calc_ae_be_grid_info(ot_vi_pipe vi_pipe, ot_isp_ae_grid_info *be_grid_info)
{
    td_bool crop_en;
    td_u16 be_width, be_height;
    td_u16 be_start_x = 0;
    td_u16 be_start_y;
    isp_working_mode isp_work_mode;

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    crop_en = ot_ext_system_ae_crop_en_read(vi_pipe);
    if (is_striping_mode(isp_work_mode.running_mode)) {
        isp_get_striping_grid_x_info(be_grid_info->grid_x_pos, OT_ISP_AE_ZONE_COLUMN, &isp_work_mode);
        be_start_y = isp_work_mode.block_rect[0].y;
        be_height = isp_work_mode.frame_rect.height;
        isp_calc_grid_info(be_height, be_start_y, OT_ISP_AE_ZONE_ROW, be_grid_info->grid_y_pos);
        be_width = isp_work_mode.frame_rect.width;
    } else {
        if (crop_en == TD_TRUE) {
            be_start_x = ot_ext_system_ae_crop_x_read(vi_pipe);
            be_start_y = ot_ext_system_ae_crop_y_read(vi_pipe);
            be_width = ot_ext_system_ae_crop_width_read(vi_pipe);
            be_height = ot_ext_system_ae_crop_height_read(vi_pipe);
        } else {
            be_start_x = 0;
            be_start_y = 0;
            be_width = isp_work_mode.frame_rect.width;
            be_height = isp_work_mode.frame_rect.height;
        }

        isp_calc_grid_info(be_width, be_start_x, OT_ISP_AE_ZONE_COLUMN, be_grid_info->grid_x_pos);
        isp_calc_grid_info(be_height, be_start_y, OT_ISP_AE_ZONE_ROW, be_grid_info->grid_y_pos);
    }

    be_grid_info->grid_x_pos[OT_ISP_AE_ZONE_COLUMN] = be_start_x + be_width - 1;  /* last position */
    be_grid_info->grid_y_pos[OT_ISP_AE_ZONE_ROW] = be_start_y + be_height - 1;  /* last position */

    be_grid_info->status = 1;

    return TD_SUCCESS;
}

td_s32 isp_get_ae_grid_info(ot_vi_pipe vi_pipe, ot_isp_ae_grid_info *fe_grid_info, ot_isp_ae_grid_info *be_grid_info)
{
    td_s32 ret;
    (td_void)memset_s(fe_grid_info, sizeof(ot_isp_ae_grid_info), 0, sizeof(ot_isp_ae_grid_info));
    (td_void)memset_s(be_grid_info, sizeof(ot_isp_ae_grid_info), 0, sizeof(ot_isp_ae_grid_info));

    isp_calc_ae_fe_grid_info(vi_pipe, fe_grid_info);

    ret = isp_calc_ae_be_grid_info(vi_pipe, be_grid_info);

    return ret;
}

td_s32 isp_get_mg_grid_info(ot_vi_pipe vi_pipe, ot_isp_mg_grid_info *grid_info)
{
    td_bool crop_en;
    td_u16  be_width, be_height;
    td_u16  be_start_x = 0;
    td_u16  be_start_y;
    isp_working_mode isp_work_mode;

    (td_void)memset_s(grid_info, sizeof(ot_isp_mg_grid_info), 0, sizeof(ot_isp_mg_grid_info));

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    crop_en = ot_ext_system_ae_crop_en_read(vi_pipe);

    if (is_striping_mode(isp_work_mode.running_mode)) {
        isp_get_striping_grid_x_info(grid_info->grid_x_pos, OT_ISP_MG_ZONE_COLUMN, &isp_work_mode);
        be_start_y   = isp_work_mode.block_rect[0].y;
        be_height   = isp_work_mode.frame_rect.height;
        isp_calc_grid_info(be_height, be_start_y, OT_ISP_MG_ZONE_ROW, grid_info->grid_y_pos);
        be_width    = isp_work_mode.frame_rect.width;
    } else {
        if (crop_en == TD_TRUE) {
            be_start_x = ot_ext_system_ae_crop_x_read(vi_pipe);
            be_start_y = ot_ext_system_ae_crop_y_read(vi_pipe);
            be_width  = ot_ext_system_ae_crop_width_read(vi_pipe);
            be_height = ot_ext_system_ae_crop_height_read(vi_pipe);
        } else {
            be_start_x = 0;
            be_start_y = 0;
            be_width  = isp_work_mode.frame_rect.width;
            be_height = isp_work_mode.frame_rect.height;
        }

        isp_calc_grid_info(be_width,  be_start_x, OT_ISP_MG_ZONE_COLUMN, grid_info->grid_x_pos);
        isp_calc_grid_info(be_height, be_start_y, OT_ISP_MG_ZONE_ROW,    grid_info->grid_y_pos);
    }

    grid_info->grid_x_pos[OT_ISP_MG_ZONE_COLUMN] = be_start_x + be_width  - 1; /* last position */
    grid_info->grid_y_pos[OT_ISP_MG_ZONE_ROW]    = be_start_y + be_height - 1; /* last position */
    grid_info->status                     = 1;

    return TD_SUCCESS;
}

static td_void isp_get_af_fe_grid_info(ot_vi_pipe vi_pipe, ot_isp_focus_grid_info *fe_grid_info)
{
    td_bool crop_en;
    td_u16 img_total_height;
    td_u16 img_total_width;
    td_u16 img_start_y;
    td_u16 img_start_x;
    td_u16 af_x_grid_num, af_y_grid_num;

    crop_en = ot_ext_af_fe_crop_enable_read(vi_pipe);
    af_y_grid_num = ot_ext_af_window_vnum_read(vi_pipe);
    af_x_grid_num = ot_ext_af_window_hnum_read(vi_pipe);

    if (crop_en == TD_TRUE) {
        img_start_x = ot_ext_af_fe_crop_pos_x_read(vi_pipe);
        img_start_y = ot_ext_af_fe_crop_pos_y_read(vi_pipe);
        img_total_width  = ot_ext_af_fe_crop_h_size_read(vi_pipe);
        img_total_height = ot_ext_af_fe_crop_v_size_read(vi_pipe);
    } else {
        img_start_x = 0;
        img_start_y = 0;
        img_total_width = ot_ext_sync_total_width_read(vi_pipe);
        img_total_height = ot_ext_sync_total_height_read(vi_pipe);
    }

    isp_calc_grid_info(img_total_width, img_start_x, af_x_grid_num, fe_grid_info->grid_x_pos);
    isp_calc_grid_info(img_total_height, img_start_y, af_y_grid_num, fe_grid_info->grid_y_pos);

    fe_grid_info->grid_x_pos[af_x_grid_num] = img_start_x + img_total_width - 1;
    fe_grid_info->grid_y_pos[af_y_grid_num] = img_start_y + img_total_height - 1;
    fe_grid_info->status = 1;
}

static td_s32 isp_get_af_be_grid_info(ot_vi_pipe vi_pipe, ot_isp_focus_grid_info *be_grid_info)
{
    td_bool crop_en;
    td_u16  be_width, be_height;
    td_u16  be_start_x = 0;
    td_u16  be_start_y;
    td_u16  af_x_grid_num, af_y_grid_num;
    isp_working_mode isp_work_mode;

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    crop_en = ot_ext_af_crop_enable_read(vi_pipe);
    af_y_grid_num = ot_ext_af_window_vnum_read(vi_pipe);
    af_x_grid_num = ot_ext_af_window_hnum_read(vi_pipe);

    if (is_striping_mode(isp_work_mode.running_mode)) {
        isp_get_striping_grid_x_info(be_grid_info->grid_x_pos, af_x_grid_num, &isp_work_mode);
        be_start_y = isp_work_mode.block_rect[0].y;
        be_height = isp_work_mode.frame_rect.height;
        isp_calc_grid_info(be_height, be_start_y, af_y_grid_num, be_grid_info->grid_y_pos);
        be_width  = isp_work_mode.frame_rect.width;
    } else {
        if (crop_en == TD_TRUE) {
            be_start_x = ot_ext_af_crop_pos_x_read(vi_pipe);
            be_start_y = ot_ext_af_crop_pos_y_read(vi_pipe);
            be_width  = ot_ext_af_crop_h_size_read(vi_pipe);
            be_height = ot_ext_af_crop_v_size_read(vi_pipe);
        } else {
            be_start_x = 0;
            be_start_y = 0;
            be_width  = isp_work_mode.frame_rect.width;
            be_height = isp_work_mode.frame_rect.height;
        }

        isp_calc_grid_info(be_width,  be_start_x, af_x_grid_num, be_grid_info->grid_x_pos);
        isp_calc_grid_info(be_height, be_start_y, af_y_grid_num,    be_grid_info->grid_y_pos);
    }

    be_grid_info->grid_x_pos[af_x_grid_num] = be_start_x + be_width  - 1; /* last position */
    be_grid_info->grid_y_pos[af_y_grid_num] = be_start_y + be_height - 1; /* last position */
    be_grid_info->status = 1;

    return TD_SUCCESS;
}

td_s32 isp_get_af_grid_info(ot_vi_pipe vi_pipe, ot_isp_focus_grid_info *fe_grid_info,
                            ot_isp_focus_grid_info *be_grid_info)
{
    (td_void)memset_s(fe_grid_info, sizeof(ot_isp_focus_grid_info), 0, sizeof(ot_isp_focus_grid_info));
    (td_void)memset_s(be_grid_info, sizeof(ot_isp_focus_grid_info), 0, sizeof(ot_isp_focus_grid_info));

    isp_get_af_fe_grid_info(vi_pipe, fe_grid_info);
    return isp_get_af_be_grid_info(vi_pipe, be_grid_info);
}

td_s32 isp_get_wb_grid_info(ot_vi_pipe vi_pipe, ot_isp_awb_grid_info *grid_info)
{
    td_bool crop_en;
    td_u16  be_width, be_height;
    td_u16  be_start_x = 0;
    td_u16  be_start_y;
    td_u16  u16awb_x_grid_num, u16awb_y_grid_num;
    isp_working_mode isp_work_mode;

    (td_void)memset_s(grid_info, sizeof(ot_isp_awb_grid_info), 0, sizeof(ot_isp_awb_grid_info));

    if (ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode) != TD_SUCCESS) {
        isp_err_trace("get work mode error!\n");
        return TD_FAILURE;
    }

    u16awb_y_grid_num = ot_ext_system_awb_vnum_read(vi_pipe);
    u16awb_x_grid_num = ot_ext_system_awb_hnum_read(vi_pipe);
    crop_en           = ot_ext_system_awb_crop_en_read(vi_pipe);

    if (is_striping_mode(isp_work_mode.running_mode)) {
        isp_get_striping_grid_x_info(grid_info->grid_x_pos, u16awb_x_grid_num, &isp_work_mode);
        be_start_y = isp_work_mode.block_rect[0].y;
        be_height = isp_work_mode.frame_rect.height;
        isp_calc_grid_info(be_height, be_start_y, u16awb_y_grid_num, grid_info->grid_y_pos);
        be_width    = isp_work_mode.frame_rect.width;
    } else {
        if (crop_en == TD_TRUE) {
            be_start_x = ot_ext_system_awb_crop_x_read(vi_pipe);
            be_start_y = ot_ext_system_awb_crop_y_read(vi_pipe);
            be_width   = ot_ext_system_awb_crop_width_read(vi_pipe);
            be_height  = ot_ext_system_awb_crop_height_read(vi_pipe);
        } else {
            be_start_x = 0;
            be_start_y = 0;
            be_width   = isp_work_mode.frame_rect.width;
            be_height  = isp_work_mode.frame_rect.height;
        }

        isp_calc_grid_info(be_width, be_start_x, u16awb_x_grid_num, grid_info->grid_x_pos);
        isp_calc_grid_info(be_height, be_start_y, u16awb_y_grid_num, grid_info->grid_y_pos);
    }

    grid_info->grid_x_pos[u16awb_x_grid_num] = be_start_x + be_width  - 1; /* last position */
    grid_info->grid_y_pos[u16awb_y_grid_num] = be_start_y + be_height - 1; /* last position */
    grid_info->status                     = 1;

    return TD_SUCCESS;
}

static td_void isp_get_fe_ae_stitch_global_stat(td_u32 pipe_num, isp_stitch_stat *stitch_stat,
                                                ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_u32  k;
    td_u16 i;

    (td_void)memset_s(ae_stitch_stat->fe_global_avg, OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16),
                      0, OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16));

    for (k = 0; k < pipe_num; k++) {
        for (i = 0; i < OT_ISP_HIST_NUM; i++) {
            ae_stitch_stat->fe_hist1024_value[k][i] = stitch_stat->fe_ae_stat1.histogram_mem_array[k][i];
        }

        ae_stitch_stat->fe_global_avg[k][OT_ISP_CHN_R]  = stitch_stat->fe_ae_stat2.global_avg_r[k];
        ae_stitch_stat->fe_global_avg[k][OT_ISP_CHN_GR] = stitch_stat->fe_ae_stat2.global_avg_gr[k];
        ae_stitch_stat->fe_global_avg[k][OT_ISP_CHN_GB] = stitch_stat->fe_ae_stat2.global_avg_gb[k];
        ae_stitch_stat->fe_global_avg[k][OT_ISP_CHN_B]  = stitch_stat->fe_ae_stat2.global_avg_b[k];
    }
}

static td_void isp_get_fe_ae_stitch_local_stat(td_u32 pipe_num, isp_stitch_stat *stitch_stat,
                                               ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_u32  k, l;
    size_t size = OT_ISP_MAX_STITCH_NUM * OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_AE_ZONE_ROW * OT_ISP_AE_ZONE_COLUMN *
                  OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);

    (td_void)memset_s(ae_stitch_stat->fe_zone_avg, size, 0, size);

    for (k = 0; k < pipe_num; k++) {
        for (l = 0; l < OT_ISP_MAX_STITCH_NUM; l++) {
            (td_void)memcpy_s(ae_stitch_stat->fe_zone_avg[l][k],
                              OT_ISP_AE_ZONE_ROW * OT_ISP_AE_ZONE_COLUMN * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16),
                              stitch_stat->fe_ae_stat3.zone_avg[l][k],
                              OT_ISP_AE_ZONE_ROW * OT_ISP_AE_ZONE_COLUMN * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16));
        }
    }
}

static td_void isp_get_be_ae_stitch_global_stat(isp_stitch_stat *stitch_stat, ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_u16 i;

    for (i = 0; i < OT_ISP_HIST_NUM; i++) {
        ae_stitch_stat->be_hist1024_value[i] = stitch_stat->be_ae_stat1.histogram_mem_array[i];
    }

    ae_stitch_stat->be_global_avg[OT_ISP_CHN_R]  = stitch_stat->be_ae_stat2.global_avg_r;
    ae_stitch_stat->be_global_avg[OT_ISP_CHN_GR] = stitch_stat->be_ae_stat2.global_avg_gr;
    ae_stitch_stat->be_global_avg[OT_ISP_CHN_GB] = stitch_stat->be_ae_stat2.global_avg_gb;
    ae_stitch_stat->be_global_avg[OT_ISP_CHN_B]  = stitch_stat->be_ae_stat2.global_avg_b;
}

static td_void isp_get_be_ae_stitch_local_stat(isp_stitch_stat *stitch_stat, ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_u8 i, j, l;

    for (i = 0; i < OT_ISP_AE_ZONE_ROW; i++) {
        for (j = 0; j < OT_ISP_AE_ZONE_COLUMN; j++) {
            for (l = 0; l < OT_ISP_MAX_STITCH_NUM; l++) {
                ae_stitch_stat->be_zone_avg[l][i][j][0] = stitch_stat->be_ae_stat3.zone_avg[l][i][j][0]; /* 0:R */
                ae_stitch_stat->be_zone_avg[l][i][j][1] = stitch_stat->be_ae_stat3.zone_avg[l][i][j][1]; /* 1:Gr */
                ae_stitch_stat->be_zone_avg[l][i][j][2] = stitch_stat->be_ae_stat3.zone_avg[l][i][j][2]; /* 2:Gb */
                ae_stitch_stat->be_zone_avg[l][i][j][3] = stitch_stat->be_ae_stat3.zone_avg[l][i][j][3]; /* 3:B */
            }
        }
    }
}

static td_s32 isp_get_fe_ae_stitch_stats(ot_vi_pipe vi_pipe, isp_stitch_stat *stitch_stat,
                                         ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_s32 ret;
    td_u32 pipe_num;
    td_u32 ctrl;
    ot_isp_stats_ctrl stat_key;
    vi_pipe_wdr_attr wdr_attr;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_WDR_ATTR, &wdr_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get WDR attr failed\n", vi_pipe);
        return ret;
    }
    ctrl = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl;

    pipe_num = MIN2(wdr_attr.pipe_num, OT_ISP_WDR_MAX_FRAME_NUM);
    if (stat_key.bit1_fe_ae_stitch_global_stat && wdr_attr.is_mast_pipe) {
        isp_get_fe_ae_stitch_global_stat(pipe_num, stitch_stat, ae_stitch_stat);
    }

    if (stat_key.bit1_fe_ae_stitch_local_stat && wdr_attr.is_mast_pipe) {
        isp_get_fe_ae_stitch_local_stat(pipe_num, stitch_stat, ae_stitch_stat);
    }

    return TD_SUCCESS;
}

static td_void isp_get_be_ae_stitch_stats(ot_vi_pipe vi_pipe, isp_stitch_stat *stitch_stat,
                                          ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_u32 ctrl;
    ot_isp_stats_ctrl stat_key;

    ctrl = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl; /* shfit to MSB 32 bits */

    /* AE BE stat */
    if (stat_key.bit1_be_ae_stitch_global_stat) {
        isp_get_be_ae_stitch_global_stat(stitch_stat, ae_stitch_stat);
    }

    if (stat_key.bit1_be_ae_stitch_local_stat) {
        isp_get_be_ae_stitch_local_stat(stitch_stat, ae_stitch_stat);
    }
    return;
}

td_s32 isp_get_ae_stitch_stats(ot_vi_pipe vi_pipe, ot_isp_ae_stitch_stats *ae_stitch_stat)
{
    td_s32 ret;
    vi_stitch_attr stitch_attr;
    ot_vi_pipe vi_main_stitch_pipe;
    isp_stat_info stat_info;
    isp_stat *isp_act_stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(ae_stitch_stat);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_STITCH_ATTR, &stitch_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get stitch attr failed\n", vi_pipe);
        return ret;
    }

    vi_main_stitch_pipe = stitch_attr.stitch_bind_id[0];

    ret = ioctl(isp_get_fd(vi_main_stitch_pipe), ISP_STAT_ACT_GET, &stat_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get active stat buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    stat_info.virt_addr = ot_mpi_sys_mmap_cached(stat_info.phy_addr, sizeof(isp_stat));
    if (stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    isp_act_stat = (isp_stat *)stat_info.virt_addr;
    /* AE FE stat */
    ret = isp_get_fe_ae_stitch_stats(vi_pipe, &isp_act_stat->stitch_stat, ae_stitch_stat);
    if (ret != TD_SUCCESS) {
        ot_mpi_sys_munmap(stat_info.virt_addr, sizeof(isp_stat));
        return ret;
    }
    /* AE BE stat */
    isp_get_be_ae_stitch_stats(vi_pipe, &isp_act_stat->stitch_stat, ae_stitch_stat);
    ae_stitch_stat->pts = isp_act_stat->frame_pts;

    ot_mpi_sys_munmap((td_void *)isp_act_stat, sizeof(isp_stat));

    return TD_SUCCESS;
}

td_s32 isp_get_wb_stitch_stats(ot_vi_pipe vi_pipe, ot_isp_wb_stitch_stats *stitch_wb_stat)
{
    td_s32 i;
    ot_isp_stats_ctrl stat_key;
    isp_stat_info act_stat_info;
    isp_stat *isp_act_stat = TD_NULL;
    td_s32 ret;
    td_u32 ctrl;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(stitch_wb_stat);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    ctrl = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &act_stat_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get active stat buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    act_stat_info.virt_addr = ot_mpi_sys_mmap_cached(act_stat_info.phy_addr, sizeof(isp_stat));

    if (act_stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    isp_act_stat = (isp_stat *)act_stat_info.virt_addr;

    if (stat_key.bit1_awb_stat2) {
        for (i = 0; i < OT_ISP_AWB_ZONE_STITCH_MAX; i++) {
            stitch_wb_stat->zone_avg_r[i] = isp_act_stat->stitch_stat.awb_stat2.metering_mem_array_avg_r[i];
            stitch_wb_stat->zone_avg_g[i] = isp_act_stat->stitch_stat.awb_stat2.metering_mem_array_avg_g[i];
            stitch_wb_stat->zone_avg_b[i] = isp_act_stat->stitch_stat.awb_stat2.metering_mem_array_avg_b[i];
            stitch_wb_stat->zone_count_all[i] = isp_act_stat->stitch_stat.awb_stat2.metering_mem_array_count_all[i];
        }

        stitch_wb_stat->zone_row = isp_act_stat->stitch_stat.awb_stat2.zone_row;
        stitch_wb_stat->zone_col = isp_act_stat->stitch_stat.awb_stat2.zone_col;
    }

    stitch_wb_stat->pts = isp_act_stat->frame_pts;
    ot_mpi_sys_munmap(act_stat_info.virt_addr, sizeof(isp_stat));

    return TD_SUCCESS;
}

td_s32 isp_get_fe_focus_stats(ot_vi_pipe vi_pipe, ot_isp_fe_focus_stats *fe_af_stat,
                              isp_stat *isp_act_stat, td_u8 wdr_chn)
{
    td_u8 i, j;
    td_u8 col, row;
    td_u8 k = 0;

    col = clip_max(ot_ext_af_window_hnum_read(vi_pipe), OT_ISP_AF_ZONE_COLUMN);
    row = clip_max(ot_ext_af_window_vnum_read(vi_pipe), OT_ISP_AF_ZONE_ROW);

    for (; k < wdr_chn; k++) {
        for (i = 0; i < row; i++) {
            for (j = 0; j < col; j++) {
                fe_af_stat->zone_metrics[k][i][j].v1 = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].v1;
                fe_af_stat->zone_metrics[k][i][j].h1 = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].h1;
                fe_af_stat->zone_metrics[k][i][j].v2 = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].v2;
                fe_af_stat->zone_metrics[k][i][j].h2 = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].h2;
                fe_af_stat->zone_metrics[k][i][j].y  = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].y;
                fe_af_stat->zone_metrics[k][i][j].hl_cnt = isp_act_stat->fe_af_stat.zone_metrics[k][i][j].hl_cnt;
            }
        }
    }

    for (; k < OT_ISP_WDR_MAX_FRAME_NUM; k++) {
        for (i = 0; i < row; i++) {
            for (j = 0; j < col; j++) {
                fe_af_stat->zone_metrics[k][i][j].v1 = 0;
                fe_af_stat->zone_metrics[k][i][j].h1 = 0;
                fe_af_stat->zone_metrics[k][i][j].v2 = 0;
                fe_af_stat->zone_metrics[k][i][j].h2 = 0;
                fe_af_stat->zone_metrics[k][i][j].y = 0;
                fe_af_stat->zone_metrics[k][i][j].hl_cnt = 0;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_get_mg_stats(ot_vi_pipe vi_pipe, ot_isp_mg_stats *mg_stat)
{
    td_s32 i, j;
    td_s32 ret;
    td_u32 ctrl;

    ot_isp_stats_ctrl stat_key;
    isp_stat_info stat_info;
    isp_stat *isp_act_stat = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_pointer_return(mg_stat);
    isp_check_vreg_permission_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &stat_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get active stat buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    stat_info.virt_addr = ot_mpi_sys_mmap_cached(stat_info.phy_addr, sizeof(isp_stat));
    if (stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }
    isp_act_stat = (isp_stat *)stat_info.virt_addr;

    ctrl = ot_ext_system_statistics_ctrl_lowbit_read(vi_pipe);
    stat_key.key = ctrl;

    /* AE FE stat */
    if (stat_key.bit1_mg_stat) {
        for (i = 0; i < OT_ISP_MG_ZONE_ROW; i++) {
            for (j = 0; j < OT_ISP_MG_ZONE_COLUMN; j++) {
                mg_stat->zone_avg[i][j][OT_ISP_CHN_R]  = isp_act_stat->mg_stat.zone_avg[i][j][OT_ISP_CHN_R];
                mg_stat->zone_avg[i][j][OT_ISP_CHN_GR] = isp_act_stat->mg_stat.zone_avg[i][j][OT_ISP_CHN_GR];
                mg_stat->zone_avg[i][j][OT_ISP_CHN_GB] = isp_act_stat->mg_stat.zone_avg[i][j][OT_ISP_CHN_GB];
                mg_stat->zone_avg[i][j][OT_ISP_CHN_B]  = isp_act_stat->mg_stat.zone_avg[i][j][OT_ISP_CHN_B];
            }
        }
    }

    ot_mpi_sys_munmap((td_void *)isp_act_stat, sizeof(isp_stat));
    return isp_get_mg_grid_info(vi_pipe, &mg_stat->grid_info);
}

td_s32 isp_set_pipe_differ_attr(ot_vi_pipe vi_pipe, const ot_isp_pipe_diff_attr *pipe_differ)
{
    td_u32 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pipe_differ);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    if (pipe_differ->op_type >= OT_OP_MODE_BUTT) {
        printf("Invalid input of parameter enOpType!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if ((pipe_differ->overlap > 0x10) || (pipe_differ->overlap < 0x1)) {
        isp_err_trace("Invalid offset :%u!\n", pipe_differ->overlap);
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_isp_pipe_diff_type_write(vi_pipe, pipe_differ->op_type);
    ot_ext_system_isp_pipe_diff_overlap_write(vi_pipe, pipe_differ->overlap);

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        if ((pipe_differ->offset[i] > 0x3FFF) || (pipe_differ->offset[i] < -0x3FFF)) {
            isp_err_trace("Invalid offset :%d!\n", pipe_differ->offset[i]);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if ((pipe_differ->gain[i] < 0x80) || (pipe_differ->gain[i] > 0x400)) {
            isp_err_trace("Invalid gain :%#x!\n", pipe_differ->gain[i]);
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        ot_ext_system_isp_pipe_diff_offset_write(vi_pipe, i, pipe_differ->offset[i]);
        ot_ext_system_isp_pipe_diff_gain_write(vi_pipe, i, pipe_differ->gain[i]);
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ot_ext_system_isp_pipe_diff_ccm_write(vi_pipe, i, pipe_differ->color_matrix[i]);
    }

    ot_ext_system_black_level_change_write(vi_pipe, (td_u8)TD_TRUE);

    return TD_SUCCESS;
}

td_s32 isp_get_pipe_differ_attr(ot_vi_pipe vi_pipe, ot_isp_pipe_diff_attr *pipe_differ)
{
    td_u32 i;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(pipe_differ);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    pipe_differ->op_type = ot_ext_system_isp_pipe_diff_type_read(vi_pipe);
    pipe_differ->overlap = ot_ext_system_isp_pipe_diff_overlap_read(vi_pipe);

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        pipe_differ->offset[i] = ot_ext_system_isp_pipe_diff_offset_read(vi_pipe, i);
        pipe_differ->gain[i]   = ot_ext_system_isp_pipe_diff_gain_read(vi_pipe, i);
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        pipe_differ->color_matrix[i] = ot_ext_system_isp_pipe_diff_ccm_read(vi_pipe, i);
    }

    return TD_SUCCESS;
}

td_s32 isp_set_rc_attr(ot_vi_pipe vi_pipe, const ot_isp_rc_attr *rc_attr)
{
    td_u16 width, height, max;
    td_u32 sq_length;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(rc_attr);
    isp_check_bool_return(rc_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    width  = ot_ext_sync_total_width_read(vi_pipe);
    height = ot_ext_sync_total_height_read(vi_pipe);
    sq_length = (td_u32)(width * width + height * height);
    max      = sqrt32(sq_length);

    if (rc_attr->center_coord.x >= width || rc_attr->center_coord.x < 0) {
        isp_err_trace("Invalid center_coor.x!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (rc_attr->center_coord.y >= height || rc_attr->center_coord.y < 0) {
        isp_err_trace("Invalid center_coor.y!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    if (rc_attr->radius >= (td_u32)max) {
        isp_err_trace("Invalid radius!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_rc_en_write(vi_pipe, rc_attr->enable);
    ot_ext_system_rc_center_hor_coord_write(vi_pipe, rc_attr->center_coord.x);
    ot_ext_system_rc_center_ver_coord_write(vi_pipe, rc_attr->center_coord.y);
    ot_ext_system_rc_radius_write(vi_pipe, rc_attr->radius);
    ot_ext_system_rc_coef_update_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}

td_s32 isp_get_rc_attr(ot_vi_pipe vi_pipe, ot_isp_rc_attr *rc_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(rc_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);

    rc_attr->enable         = ot_ext_system_rc_en_read(vi_pipe);
    rc_attr->center_coord.x = ot_ext_system_rc_center_hor_coord_read(vi_pipe);
    rc_attr->center_coord.y = ot_ext_system_rc_center_ver_coord_read(vi_pipe);
    rc_attr->radius         = ot_ext_system_rc_radius_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_set_clut_coeff(ot_vi_pipe vi_pipe, const ot_isp_clut_lut *clut_lut)
{
    td_s32 ret;
    td_u32 *vir_addr = TD_NULL;
    isp_mmz_buf_ex clut_buf;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(clut_lut);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_clut_init_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_CLUT_BUF_GET, &clut_buf.phy_addr) != TD_SUCCESS) {
        isp_err_trace("get clut buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    clut_buf.vir_addr = ot_mpi_sys_mmap(clut_buf.phy_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));

    if (clut_buf.vir_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    vir_addr = (td_u32 *)clut_buf.vir_addr;

    ret = memcpy_s(vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                   clut_lut->lut, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    if (ret != EOK) {
        ot_mpi_sys_munmap(clut_buf.vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
        isp_err_trace("ISP[%d] set clut lut failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    ot_ext_system_clut_lut_update_en_write(vi_pipe, TD_TRUE);

    ot_mpi_sys_munmap(clut_buf.vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));

    return TD_SUCCESS;
}

td_s32 isp_get_clut_coeff(ot_vi_pipe vi_pipe, ot_isp_clut_lut *clut_lut)
{
    td_s32 ret;
    td_u32 *vir_addr = TD_NULL;
    isp_mmz_buf_ex clut_buf;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(clut_lut);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_clut_init_return(vi_pipe);

    if (ioctl(isp_get_fd(vi_pipe), ISP_CLUT_BUF_GET, &clut_buf.phy_addr) != TD_SUCCESS) {
        isp_err_trace("get clut buffer err\n");
        return OT_ERR_ISP_NOMEM;
    }

    clut_buf.vir_addr = ot_mpi_sys_mmap(clut_buf.phy_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));

    if (clut_buf.vir_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }

    vir_addr = (td_u32 *)clut_buf.vir_addr;

    ret = memcpy_s(clut_lut->lut, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32),
                   vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
    if (ret != EOK) {
        ot_mpi_sys_munmap(clut_buf.vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));
        isp_err_trace("ISP[%d] get clut lut failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    ot_mpi_sys_munmap(clut_buf.vir_addr, OT_ISP_CLUT_LUT_LENGTH * sizeof(td_u32));

    return TD_SUCCESS;
}

td_s32 isp_set_clut_attr(ot_vi_pipe vi_pipe, const ot_isp_clut_attr *clut_attr)
{
    const td_u32 max_gain = 4095;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(clut_attr);
    isp_check_bool_return(clut_attr->enable);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_clut_init_return(vi_pipe);

    if (clut_attr->gain_r > max_gain) {
        isp_err_trace("Invalid gain_r!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (clut_attr->gain_g > max_gain) {
        isp_err_trace("Invalid gain_g!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    if (clut_attr->gain_b > max_gain) {
        isp_err_trace("Invalid gain_b!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    ot_ext_system_clut_en_write(vi_pipe, clut_attr->enable);
    ot_ext_system_clut_gain_r_write(vi_pipe, clut_attr->gain_r);
    ot_ext_system_clut_gain_g_write(vi_pipe, clut_attr->gain_g);
    ot_ext_system_clut_gain_b_write(vi_pipe, clut_attr->gain_b);
    ot_ext_system_clut_ctrl_update_en_write(vi_pipe, TD_TRUE);

    return TD_SUCCESS;
}
td_s32 isp_get_clut_attr(ot_vi_pipe vi_pipe, ot_isp_clut_attr *clut_attr)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(clut_attr);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_vreg_permission_return(vi_pipe);
    isp_check_clut_init_return(vi_pipe);

    clut_attr->enable = ot_ext_system_clut_en_read(vi_pipe);
    clut_attr->gain_r = ot_ext_system_clut_gain_r_read(vi_pipe);
    clut_attr->gain_g = ot_ext_system_clut_gain_g_read(vi_pipe);
    clut_attr->gain_b = ot_ext_system_clut_gain_b_read(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_calc_flicker_type(ot_vi_pipe vi_pipe, ot_isp_calc_flicker_input *input_param,
                             ot_isp_calc_flicker_output *output_param, ot_video_frame_info frame[],
                             td_u32 array_size)
{
    td_u32 i;

    if (array_size != 3) { /* frame number cannot be 3 */
        isp_err_trace("Frame Number is not 3 Frame\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    isp_check_pipe_return(vi_pipe);
    isp_check_open_return(vi_pipe);
    isp_check_mem_init_return(vi_pipe);
    isp_check_pointer_return(input_param);
    isp_check_pointer_return(output_param);

    if (input_param->lines_per_second < 500) { /* Lines per second range [500, inf) */
        isp_err_trace("LinePerSecond is out of range\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }

    for (i = 0; i < array_size; i++) {
        isp_check_pointer_return(frame + i);

        if (frame[i].video_frame.phys_addr[0] == 0) {
            isp_err_trace("The Phy Address Error!!!\n");
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if ((frame[i].video_frame.width < OT_ISP_WIDTH_MIN) ||
            (frame[i].video_frame.width > ot_isp_res_width_max(vi_pipe))) {
            isp_err_trace("The Image width is out of range!!!\n");
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if ((frame[i].video_frame.height < OT_ISP_HEIGHT_MIN) ||
            (frame[i].video_frame.height > ot_isp_res_height_max(vi_pipe))) {
            isp_err_trace("The Image height is out of range!!!\n");
            return OT_ERR_ISP_ILLEGAL_PARAM;
        }

        if (i != 0) {
            if ((frame[i].video_frame.time_ref - frame[i - 1].video_frame.time_ref) != 2) { /* discontinue is 2frames */
                isp_err_trace("The Frames is not continuity!!!\n");
                return OT_ERR_ISP_ILLEGAL_PARAM;
            }
        }
    }

    return calc_flicker_type(input_param, output_param, frame, array_size);
}

td_s32 isp_get_check_sum_stats(ot_vi_pipe vi_pipe, isp_check_sum_stat *sum_stat)
{
    td_s32 ret;
    isp_stat_info stat_info;
    isp_stat *isp_act_stat = TD_NULL;

    if (ioctl(isp_get_fd(vi_pipe), ISP_STAT_ACT_GET, &stat_info) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get active stat buffer failed\n", vi_pipe);
        return OT_ERR_ISP_NOMEM;
    }

    stat_info.virt_addr = ot_mpi_sys_mmap_cached(stat_info.phy_addr, sizeof(isp_stat));
    if (stat_info.virt_addr == TD_NULL) {
        return OT_ERR_ISP_NULL_PTR;
    }
    isp_act_stat = (isp_stat *)stat_info.virt_addr;

    ret = memcpy_s(sum_stat->sum_y, sizeof(sum_stat->sum_y), isp_act_stat->sum_stat.sum_y,
        sizeof(isp_act_stat->sum_stat.sum_y));
    if ((ret) != EOK) {
        isp_err_trace("memcpy_s failed!\n");
        ot_mpi_sys_munmap((td_void *)isp_act_stat, sizeof(isp_stat));
        return TD_FAILURE;
    }

    sum_stat->pts = isp_act_stat->frame_pts;
    ot_mpi_sys_munmap((td_void *)isp_act_stat, sizeof(isp_stat));

    return TD_SUCCESS;
}

