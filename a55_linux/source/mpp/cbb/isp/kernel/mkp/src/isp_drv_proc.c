/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_spi.h"
#include "ot_osal.h"
#include "ot_common_isp.h"
#include "isp_drv_define.h"
#include "isp.h"

#define ISP_SHARE_PROC_BUF_LEN 10

static td_void isp_proc_module_param_show(ot_vi_pipe vi_pipe, osal_proc_entry *s)
{
    ot_isp_ctrl_param ctrl_param = { 0 };
    ot_isp_mod_param mod_param = { 0 };

    isp_get_mod_param(&mod_param);
    isp_get_ctrl_param(vi_pipe, &ctrl_param);
    call_sys_print_proc_title(s, "module/control param");
    osal_seq_printf(s->seqfile, " %10s" " %12s" " %12s" " %12s" " %12s" " %12s" " %12s" "\n", "proc_param",
                    "stat_intvl", "update_pos", "int_bothalf", "int_timeout", "pwm_number", "run_wakeup");
    osal_seq_printf(s->seqfile, " %10u" " %12u" " %12u" " %12u" " %12u" " %12u"  " %12u" "\n",
                    ctrl_param.proc_param, ctrl_param.stat_interval, ctrl_param.update_pos,
                    mod_param.interrupt_bottom_half, ctrl_param.interrupt_time_out, ctrl_param.pwm_num,
                    ctrl_param.isp_run_wakeup_select);
    osal_seq_printf(s->seqfile, "\n");

    osal_seq_printf(s->seqfile, " %14s" " %12s" " %15s" " %16s" " %12s" " %14s" " %12s""\n",
                    "port_int_delay", "quick_start", "ldci_tprflten", "long_frm_int_en",
                    "be_buf_num", "ob_update_pos", "alg_run_sel");
    osal_seq_printf(s->seqfile, " %14u" " %12u" " %15u" " %16u" " %12u" " %14s" " %12s""\n",
                    ctrl_param.port_interrupt_delay, ctrl_param.quick_start_en, ctrl_param.ldci_tpr_flt_en,
                    ctrl_param.long_frame_interrupt_en, ctrl_param.be_buf_num,
                    (ctrl_param.ob_stats_update_pos == OT_ISP_UPDATE_OB_STATS_FE_FRAME_END) ? "frame end"  :
                    (ctrl_param.ob_stats_update_pos == OT_ISP_UPDATE_OB_STATS_FE_FRAME_START) ? "frame start" : "butt",
                    (ctrl_param.alg_run_select == OT_ISP_ALG_RUN_NORM) ? "normal" :
                    (ctrl_param.alg_run_select == OT_ISP_ALG_RUN_FE_ONLY) ? "fe only" : "butt");
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_void isp_proc_detail_stats_show(const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    td_s32 i;
    if (drv_ctx->detail_stats_cfg.enable == TD_FALSE) {
        return;
    }

    call_sys_print_proc_title(s, "detail stats info");
    osal_seq_printf(s->seqfile, " %10s" " %10s" " %10s" "%10s" "%10s" "\n",
        "enable", "ctrl", "col", "row", "interval");
    osal_seq_printf(s->seqfile, " %10d" " %10s" " %10d" "%10d" "%10d" "\n",
                    drv_ctx->detail_stats_cfg.enable,
                    (drv_ctx->detail_stats_cfg.ctrl.key == 0x3) ? "ae awb" :
                    (drv_ctx->detail_stats_cfg.ctrl.key == 0x1) ? "ae" :
                    (drv_ctx->detail_stats_cfg.ctrl.key == 0x2) ? "awb" : "none",
                    drv_ctx->detail_stats_cfg.col,
                    drv_ctx->detail_stats_cfg.row,
                    drv_ctx->detail_stats_cfg.interval);
    osal_seq_printf(s->seqfile, "\n");
    for (i = 0; i < drv_ctx->detail_stats_cfg.col; i++) {
        osal_seq_printf(s->seqfile, "%10s%d ", "blk_w", i);
    }

    for (i = 0; i < drv_ctx->detail_stats_cfg.row; i++) {
        osal_seq_printf(s->seqfile, "%10s%d ", "blk_h", i);
    }
    osal_seq_printf(s->seqfile, "\n");
    for (i = 0; i < drv_ctx->detail_stats_cfg.col; i++) {
        osal_seq_printf(s->seqfile, "%11d ", drv_ctx->detail_size.split_width[i]);
    }
    for (i = 0; i < drv_ctx->detail_stats_cfg.row; i++) {
        osal_seq_printf(s->seqfile, "%11d ", drv_ctx->detail_size.split_height[i]);
    }
    osal_seq_printf(s->seqfile, "\n");
}
#endif
static td_void isp_proc_isp_mode_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    ot_unused(vi_pipe);
    call_sys_print_proc_title(s, "isp mode");
    osal_seq_printf(s->seqfile, " %11s" " %15s" " %15s" "%15s" "%12s" "\n",
        "stitch_mode", "running_mode", "block_num", "data_mode", "run_once");
    osal_seq_printf(s->seqfile, " %11s" " %15s" " %15d" "%15s" "%12d" "\n",
                    drv_ctx->stitch_attr.stitch_enable ? "stitch" : "normal",
                    (drv_ctx->work_mode.running_mode == ISP_MODE_RUNNING_OFFLINE) ? "offline"  :
                    (drv_ctx->work_mode.running_mode == ISP_MODE_RUNNING_PRE_ONLINE_POST_OFFLINE) ? "preon_postoff"  :
                    (drv_ctx->work_mode.running_mode == ISP_MODE_RUNNING_ONLINE) ? "online"   :
                    (drv_ctx->work_mode.running_mode == ISP_MODE_RUNNING_SIDEBYSIDE) ? "sbs"      :
                    (drv_ctx->work_mode.running_mode == ISP_MODE_RUNNING_STRIPING) ? "striping" : "butt",
                    drv_ctx->work_mode.block_num,
                    (drv_ctx->work_mode.data_input_mode == ISP_MODE_RAW) ? "raw" : "yuv",
                    drv_ctx->run_once_flag);
}

static td_void isp_proc_sensor_info_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    ot_isp_sns_type sns_type;
    td_s32 sns_dev, ret;
    td_char sns_type_name[MAX_MMZ_NAME_LEN] = { 0 };
    isp_sync_cfg_buf_node *cur_node = drv_ctx->sync_cfg.node[0];
    if (cur_node == TD_NULL) {
        return;
    }
    ot_unused(vi_pipe);

    sns_type = cur_node->sns_regs_info.sns_type;
    if (sns_type == OT_ISP_SNS_TYPE_I2C) {
        sns_dev = cur_node->sns_regs_info.com_bus.i2c_dev;
        ret =  snprintf_s(sns_type_name, sizeof(sns_type_name), sizeof(sns_type_name) - 1, "i2c");
        if (ret < 0) {
            return;
        }
    } else if (sns_type == OT_ISP_SNS_TYPE_SSP) {
        sns_dev = cur_node->sns_regs_info.com_bus.ssp_dev.bit4_ssp_dev;
        ret =  snprintf_s(sns_type_name, sizeof(sns_type_name), sizeof(sns_type_name) - 1, "ssp");
        if (ret < 0) {
            return;
        }
    } else {
        sns_dev = -1;
        ret =  snprintf_s(sns_type_name, sizeof(sns_type_name), sizeof(sns_type_name) - 1, "butt");
        if (ret < 0) {
            return;
        }
    }
    call_sys_print_proc_title(s, "sensor info");
    osal_seq_printf(s->seqfile, "%12s" "%10s\n", "sensor_type", "dev");
    osal_seq_printf(s->seqfile, "%12s" "%10d\n", sns_type_name, sns_dev);
}

static td_void isp_proc_int_status_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    call_sys_print_proc_title(s, "drv info");

    osal_seq_printf(s->seqfile, "%10s" "%15s" "%15s" "%15s" "%15s" "%15s" "%15s" "%15s\n",
                    "vi_pipe", "int_cnt", "int_t", "max_int_t", "avg_int_t", "int_gap_t", "max_gap_t", "int_rat");

    osal_seq_printf(s->seqfile, "%10d" "%15d" "%15d" "%15d" "%15d" "%15d" "%15d" "%15d\n", vi_pipe,
                    drv_ctx->drv_dbg_info.isp_int_cnt, drv_ctx->drv_dbg_info.isp_int_time,
                    drv_ctx->drv_dbg_info.isp_int_time_max, drv_ctx->drv_dbg_info.isp_int_avg_time,
                    drv_ctx->drv_dbg_info.isp_int_gap_time, drv_ctx->drv_dbg_info.isp_int_gap_time_max,
                    drv_ctx->drv_dbg_info.isp_rate);

    osal_seq_printf(s->seqfile, "\n");

    osal_seq_printf(s->seqfile, "%10s" "%15s" "%15s" "%15s" "%15s" "%15s" "%15s" "%15s\n",
                    "int_type", "pt_int_cnt", "pt_int_t",
                    "pt_max_int_t", "pt_int_gap_t", "pt_max_gap_t",
                    "pt_int_rat", "reset_cnt");

    osal_seq_printf(s->seqfile, "%10s" "%15u" "%15u" "%15u" "%15u" "%15u" "%15u" "%15u\n",
                    (drv_ctx->frame_int_attr.interrupt_type == OT_FRAME_INTERRUPT_START) ? "start" : "other",
                    drv_ctx->drv_dbg_info.pt_int_cnt, drv_ctx->drv_dbg_info.pt_int_time,
                    drv_ctx->drv_dbg_info.pt_int_time_max, drv_ctx->drv_dbg_info.pt_int_gap_time,
                    drv_ctx->drv_dbg_info.pt_int_gap_time_max, drv_ctx->drv_dbg_info.pt_rate,
                    drv_ctx->drv_dbg_info.isp_reset_cnt);

    osal_seq_printf(s->seqfile, "\n");

    osal_seq_printf(s->seqfile, "%10s" "%15s" "%15s" "%15s" "%15s" "%15s" "%15s\n",
                    "fe_stat_t", "fe_stat_max_t", "cp_stat_t", "cp_stat_max_t", "be_stat_t",
                    "be_stat_max_t", "be_stat_lost");

    osal_seq_printf(s->seqfile, "%10u" "%15u" "%15u" "%15u" "%15u" "%15u" "%15u\n",
                    drv_ctx->drv_dbg_info.isp_fe_stat_time, drv_ctx->drv_dbg_info.isp_fe_stat_time_max,
                    drv_ctx->drv_dbg_info.isp_cp_fe_stat_time, drv_ctx->drv_dbg_info.isp_cp_fe_stat_time_max,
                    drv_ctx->drv_dbg_info.isp_be_stat_time, drv_ctx->drv_dbg_info.isp_be_stat_time_max,
                    drv_ctx->drv_dbg_info.isp_be_sta_lost);

    osal_seq_printf(s->seqfile, "\n");

    osal_seq_printf(s->seqfile, "%10s" "%15s" "%15s" "%15s" "%15s\n",
                    "be_end_t", "be_end_max_t", "sensor_cfg_t", "sensor_max_t", "sensor_avg_t");

    osal_seq_printf(s->seqfile, "%10u" "%15u" "%15u" "%15u" "%15u\n",
                    drv_ctx->drv_dbg_info.isp_int_be_end_time, drv_ctx->drv_dbg_info.isp_int_be_end_time_max,
                    drv_ctx->drv_dbg_info.sensor_cfg_time, drv_ctx->drv_dbg_info.sensor_cfg_time_max,
                    drv_ctx->drv_dbg_info.sensor_cfg_time_avg);

    osal_seq_printf(s->seqfile, "\n");

    osal_seq_printf(s->seqfile, "%13s" "%20s" "%20s" "%20s" "%20s" "%20s\n",
                    "sync_cfg_gap", "sync_cfg_gap_max", "sync_cfg_gap_min", "ldci_comp_err_cnt", "stitch_pts_cnt",
                    "sync_id_err_cnt");

    osal_seq_printf(s->seqfile, "%13u" "%20u" "%20u" "%20u" "%20u" "%20u\n",
                    drv_ctx->drv_dbg_info.sync_cfg_gap, drv_ctx->drv_dbg_info.sync_cfg_gap_max,
                    drv_ctx->drv_dbg_info.sync_cfg_gap_min, drv_ctx->drv_dbg_info.ldci_comp_err_cnt,
                    drv_ctx->drv_dbg_info.stitch_pts_cnt, drv_ctx->drv_dbg_info.sync_id_err_cnt);
}

static td_void isp_proc_be_cfg_phy_addr_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    td_u8  i, j, max_loop, remainder, loop, index;
    td_u64 be_buf_size;
    ot_isp_ctrl_param ctrl_param = { 0 };
    const td_u32 num_in_line = 4; /* const 4 */
    if (is_online_mode(drv_ctx->work_mode.running_mode) ||
        (drv_ctx->be_buf_info.init == TD_FALSE)) {
        return;
    }
    isp_get_ctrl_param(vi_pipe, &ctrl_param);

    call_sys_print_proc_title(s, "be_cfg phy addr");

    remainder = ctrl_param.be_buf_num % num_in_line;
    max_loop  = ctrl_param.be_buf_num / num_in_line;
    if (remainder != 0) {
        max_loop += 1;
    }

    loop = num_in_line;
    be_buf_size = drv_ctx->be_buf_info.be_buf_haddr.size / ctrl_param.be_buf_num;

    for (j = 0; j < max_loop; j++) {
        if ((j + 1) * num_in_line > ctrl_param.be_buf_num) {
            loop = remainder;
        }

        index = j * num_in_line;
        osal_seq_printf(s->seqfile, "%8s""%2d""%2s""%2d""%2s", "be_cfg[", index, ", ", index + loop - 1, "]:");

        for (i = 0; i < loop; i++) {
            index = j * num_in_line + i;
            osal_seq_printf(s->seqfile, "%#20llx", drv_ctx->be_buf_info.be_buf_haddr.phy_addr +
                            index * be_buf_size); /* 4 */
        }

        osal_seq_printf(s->seqfile, "\n");
    }
}

static td_char* isp_proc_get_wdr(const isp_drv_ctx *drv_ctx)
{
    switch (drv_ctx->proc_pub_info.wdr_mode) {
        case OT_WDR_MODE_NONE:
            return "linear";
        case OT_WDR_MODE_BUILT_IN:
            return "built_in";
        case OT_WDR_MODE_QUADRA:
            return "quadra";
        case OT_WDR_MODE_2To1_LINE:
            return "2to1_line";
        case OT_WDR_MODE_2To1_FRAME:
            return "2to1_frame";
        case OT_WDR_MODE_3To1_LINE:
            return "3to1_line";
        case OT_WDR_MODE_3To1_FRAME:
            return "3to1_frame";
        case OT_WDR_MODE_4To1_LINE:
            return "4to1_line";
        case OT_WDR_MODE_4To1_FRAME:
            return "4to1_frame";
        default :
            return "Unknown";
    }
    return "Unknown";
}

static td_void isp_proc_pub_attr_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    td_char *wdr_mode = isp_proc_get_wdr(drv_ctx);
    ot_unused(vi_pipe);
    /*  show isp attribute here. width/height/bayer_format, etc..
              Read parameter from memory directly. */
    call_sys_print_proc_title(s, "pubattr info");

    osal_seq_printf(s->seqfile, "%10s" "%12s" "%12s" "%12s" "%12s" "%12s" "%12s" "%10s" "%12s" "%12s\n",
                    "wnd_x", "wnd_y", "wnd_w", "wnd_h", "sns_w", "sns_h", "sns_mode", "flip", "mirror", "bayer");

    osal_seq_printf(s->seqfile, "%10d" "%12d" "%12d" "%12d" "%12d" "%12d" "%12u" "%10d" "%12d" "%12s\n\n",
                    drv_ctx->proc_pub_info.wnd_rect.x, drv_ctx->proc_pub_info.wnd_rect.y,
                    drv_ctx->proc_pub_info.wnd_rect.width, drv_ctx->proc_pub_info.wnd_rect.height,
                    drv_ctx->proc_pub_info.sns_size.width, drv_ctx->proc_pub_info.sns_size.height,
                    drv_ctx->proc_pub_info.sns_mode,
                    drv_ctx->proc_pub_info.sns_flip_en, drv_ctx->proc_pub_info.sns_mirror_en,
                    (drv_ctx->proc_pub_info.bayer_format == OT_ISP_BAYER_RGGB) ? "rggb" :
                    (drv_ctx->proc_pub_info.bayer_format == OT_ISP_BAYER_GRBG) ? "grbg" :
                    (drv_ctx->proc_pub_info.bayer_format == OT_ISP_BAYER_GBRG) ? "gbrg" :
                    (drv_ctx->proc_pub_info.bayer_format == OT_ISP_BAYER_BGGR) ? "bggr" : "butt");

    osal_seq_printf(s->seqfile, "%10s" "%12s" "%12s" "%12s" "%12s" "%12s\n",
                    "crop_en", "crop_x", "crop_y", "crop_w", "crop_h", "wdr_mode");

    osal_seq_printf(s->seqfile, "%10d" "%12d" "%12d" "%12d" "%12d" "%12s\n",
                    drv_ctx->proc_pub_info.mipi_crop_attr.mipi_crop_en,
                    drv_ctx->proc_pub_info.mipi_crop_attr.mipi_crop_offset.x,
                    drv_ctx->proc_pub_info.mipi_crop_attr.mipi_crop_offset.y,
                    drv_ctx->proc_pub_info.mipi_crop_attr.mipi_crop_offset.width,
                    drv_ctx->proc_pub_info.mipi_crop_attr.mipi_crop_offset.height,
                    wdr_mode);
}

static td_void isp_proc_fe_roi_show(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_check_no_fe_pipe_void_return(vi_pipe);
    isp_drv_fereg_ctx(vi_pipe, fe_reg);
    if (fe_reg == TD_NULL) {
        return;
    }
    /*  show isp fe roi here. Read parameter from reg. */
    call_sys_print_proc_title(s, "fe roi info");

    osal_seq_printf(s->seqfile, "%10s" "%12s" "%12s" "%12s" "%12s\n",
                    "fe_roi_en", "fe_roi_x", "fe_roi_y", "fe_roi_w", "fe_roi_h");

    osal_seq_printf(s->seqfile, "%10d" "%12d" "%12d" "%12d" "%12d\n",
                    fe_reg->isp_blc1_roi_cfg0.bits.isp_blc1_roi_en, fe_reg->isp_blc1_roi_cfg0.bits.isp_blc1_roi_x,
                    fe_reg->isp_blc1_roi_cfg0.bits.isp_blc1_roi_y, fe_reg->isp_blc1_roi_cfg1.bits.isp_blc1_roi_width,
                    fe_reg->isp_blc1_roi_cfg1.bits.isp_blc1_roi_height);
}

static td_void isp_proc_wdr_mode_show(isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    static ot_isp_fswdr_mode fs_wdr_mode = 0;
    fs_wdr_mode = drv_ctx->frame_info.fs_wdr_mode;

    osal_seq_printf(s->seqfile, "%16s\n", "fswdr mode");

    if (is_fs_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        if (fs_wdr_mode == OT_ISP_FSWDR_NORMAL_MODE) {
            osal_seq_printf(s->seqfile, "%16s\n\n", "normal");
        } else if (fs_wdr_mode == OT_ISP_FSWDR_LONG_FRAME_MODE) {
            osal_seq_printf(s->seqfile, "%16s\n\n", "long frame");
        } else if (fs_wdr_mode == OT_ISP_FSWDR_AUTO_LONG_FRAME_MODE) {
            osal_seq_printf(s->seqfile, "%16s\n\n", "auto long frame");
        }
    }
}

static td_void isp_proc_frame_info_show(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    td_u8       i = 0;
    td_u8       loop = 0;

    ot_unused(vi_pipe);

    if (drv_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return;
    }

    /*  show isp frame info here. fe_id/fs_wdr_mode/bayer_format, etc..
              Read parameter from memory directly. */
    call_sys_print_proc_title(s, "send raw isp frame info");

    if (drv_ctx->proc_frame_info.print_en != TD_TRUE) {
        return;
    }

    osal_seq_printf(s->seqfile, "%16s" "%16s" "%16s" "%16s" "%16s" "%26s\n",
                    "fe_id", "exp_time", "isp_dg", "again", "dgain", "vi_send_raw_cnt");

    if (is_linear_mode(drv_ctx->wdr_attr.wdr_mode) || is_built_in_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        loop = 1;
    } else if (is_2to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        loop = 2; /* 2 */
    } else if (is_3to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        loop = 3; /* 3 */
    } else if (is_4to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        loop = 4; /* 4 */
    }

    for (i = 0; i < loop; i++) {
        osal_seq_printf(s->seqfile, "%16u" "%16u" "%16u" "%16u" "%16u" "%26llu\n\n",
                        drv_ctx->proc_frame_info.fe_id[i], drv_ctx->proc_frame_info.exposure_time[i],
                        drv_ctx->proc_frame_info.isp_dgain[i], drv_ctx->proc_frame_info.again[i],
                        drv_ctx->proc_frame_info.dgain[i], drv_ctx->proc_frame_info.vi_send_raw_cnt);
    }

    isp_proc_wdr_mode_show(drv_ctx, s);
}

static td_void isp_proc_share_mem_show(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, osal_proc_entry *s)
{
    td_u32 i = 0;
    char buf[ISP_SHARE_PROC_BUF_LEN] = {0};
    td_s32 new_len;
    ot_unused(vi_pipe);

    call_sys_print_proc_title(s, "share mem info");
    osal_seq_printf(s->seqfile, "%15s", "share_all_en");
    for (i = 0; i < ISP_SHARE_ID_LEN; i++) {
        new_len = snprintf_s(buf, ISP_SHARE_PROC_BUF_LEN, ISP_SHARE_PROC_BUF_LEN - 1, "share[%d]", i);
        if (new_len < 0) {
            isp_err_trace("log write snprintf_s err!!\n");
        }
        osal_seq_printf(s->seqfile, "%12s", buf);
    }

    osal_seq_printf(s->seqfile, "\n%15d", drv_ctx->share_all_en);
    for (i = 0; i < ISP_SHARE_ID_LEN; i++) {
        if (drv_ctx->share_pid[i] == ISP_INVALID_PID) {
            continue;
        }
        osal_seq_printf(s->seqfile, "%12d", drv_ctx->share_pid[i]);
    }
    osal_seq_printf(s->seqfile, "\n\n");
}

static td_s32 isp_drv_proc_printf(ot_vi_pipe vi_pipe, osal_proc_entry *s)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    td_u32 proc_buf_len;
    const td_char *psz_str = TD_NULL;
    td_char *psz_buf = TD_NULL;
    ot_isp_ctrl_param ctrl_param = { 0 };

    isp_check_pipe_return(vi_pipe);

    isp_get_ctrl_param(vi_pipe, &ctrl_param);
    if (ctrl_param.proc_param == 0) {
        return TD_SUCCESS;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (osal_sem_down(&drv_ctx->proc_sem)) {
        return -ERESTARTSYS;
    }

    if (drv_ctx->porc_mem.virt_addr != TD_NULL) {
        psz_buf = osal_kmalloc((PROC_PRT_SLICE_SIZE + 1), OSAL_GFP_ATOMIC);
        if (psz_buf == TD_NULL) {
            isp_err_trace("isp_drv_proc_printf malloc slice buf err\n");
            osal_sem_up(&drv_ctx->proc_sem);
            return OT_ERR_ISP_NULL_PTR;
        }

        psz_buf[PROC_PRT_SLICE_SIZE] = '\0';
        psz_str = (td_char *)drv_ctx->porc_mem.virt_addr;
        proc_buf_len = osal_strlen((td_char *)drv_ctx->porc_mem.virt_addr);

        while (proc_buf_len) {
            (td_void)strncpy_s(psz_buf, PROC_PRT_SLICE_SIZE + 1, psz_str, PROC_PRT_SLICE_SIZE);
            osal_seq_printf(s->seqfile, "%s", psz_buf);
            psz_str += PROC_PRT_SLICE_SIZE;

            if (proc_buf_len < PROC_PRT_SLICE_SIZE) {
                proc_buf_len = 0;
            } else {
                proc_buf_len -= PROC_PRT_SLICE_SIZE;
            }
        }

        osal_kfree((td_void *)psz_buf);
    }

    osal_sem_up(&drv_ctx->proc_sem);

    return TD_SUCCESS;
}

td_s32 isp_proc_show(osal_proc_entry *s)
{
    td_s32 ret;
    ot_vi_pipe  vi_pipe = 0;
    isp_drv_ctx *drv_ctx = TD_NULL;

    osal_seq_printf(s->seqfile, "\n[ISP] Version: ["OT_MPP_VERSION"], Build Time["__DATE__", "__TIME__"]\n\n");
    osal_seq_printf(s->seqfile, "\n");

    do {
        drv_ctx = isp_drv_get_ctx(vi_pipe);
        if (!drv_ctx->mem_init) {
            continue;
        }

        call_sys_print_proc_title(s, "isp proc pipe[%d]", vi_pipe);
        osal_seq_printf(s->seqfile, "\n");
        isp_proc_module_param_show(vi_pipe, s);
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
        isp_proc_detail_stats_show(drv_ctx, s);
#endif
        isp_proc_isp_mode_show(vi_pipe, drv_ctx, s);
        isp_proc_sensor_info_show(vi_pipe, drv_ctx, s);
        isp_proc_int_status_show(vi_pipe, drv_ctx, s);
        isp_proc_be_cfg_phy_addr_show(vi_pipe, drv_ctx, s);
        isp_proc_pub_attr_show(vi_pipe, drv_ctx, s);
        isp_proc_frame_info_show(vi_pipe, drv_ctx, s);
        isp_proc_fe_roi_show(vi_pipe, drv_ctx, s);
        isp_proc_share_mem_show(vi_pipe, drv_ctx, s);
        osal_seq_printf(s->seqfile, "\n");

        ret = isp_drv_proc_printf(vi_pipe, s);
        if (ret != TD_SUCCESS) {
        }

        call_sys_print_proc_title(s, "isp proc end[%d]", vi_pipe);
        osal_seq_printf(s->seqfile, "\n");
    } while (++vi_pipe < OT_ISP_MAX_PIPE_NUM);

    return 0;
}
