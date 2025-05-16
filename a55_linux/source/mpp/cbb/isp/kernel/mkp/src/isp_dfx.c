/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_dfx.h"

#define ISP_DFX_EXTRA             2

typedef union {
    td_u32  enable;
    struct {
        td_u32  sns_cfg_sync_verify_en   : 1;    /* [0] */
        td_u32  reserved                 : 31;   /* [31:1] */
    } bits;
} isp_dfx_enable;

typedef struct {
    td_s32 verify_frm_cnt;
    td_s32 expect_change_cnt;
} isp_dfx_sns_sync_verify_attr;

typedef struct {
    isp_dfx_sns_sync_verify_attr sns_sync_verify_attr;
} isp_dfx_context;

static isp_dfx_enable g_isp_dfx_en = {0};
static isp_dfx_context *g_isp_dfx_ctx[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = TD_NULL};

static td_bool is_isp_dfx_sns_cfg_update(const isp_drv_ctx *drv_ctx, td_u8 cur_node_idx, td_u8 stay_cnt)
{
    td_u32 i;
    td_u8 cfg_node_id;
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;
    isp_sync_cfg_buf_node *cur_node = drv_ctx->sync_cfg.node[cur_node_idx];
    ot_isp_i2c_data *i2c_data = TD_NULL;
    if (cur_node == TD_NULL) {
        isp_err_trace("isp[%d] cur_node:%u is null\n", isp_drv_get_pipe_id(drv_ctx), cur_node_idx);
        return TD_FALSE;
    }

    for (i = 0; i < cur_node->sns_regs_info.reg_num; i++) {
        if (is_sensor_data_has_been_config(cur_node, i, stay_cnt) == TD_TRUE) {
            continue;
        }

        cfg_node_id = cur_node->sns_regs_info.i2c_data[i].delay_frame_num - stay_cnt;
        cfg_node = isp_drv_get_sns_cfg_node(drv_ctx, cur_node_idx, cfg_node_id);
        if (cfg_node == TD_NULL) {
            continue;
        }
        i2c_data = &cfg_node->sns_regs_info.i2c_data[i];
        if (i2c_data->update == TD_TRUE) {
            return TD_TRUE;
        }
    }

    return TD_FALSE;
}

td_void isp_dfx_global_enable_init(td_void)
{
    g_isp_dfx_en.enable = 0;

    g_isp_dfx_en.bits.sns_cfg_sync_verify_en = 0;
}

td_void isp_dfx_ctx_init(ot_vi_pipe vi_pipe)
{
    if (g_isp_dfx_en.enable == 0) {
        return;
    }

    isp_check_pipe_void_return(vi_pipe);

    if (g_isp_dfx_ctx[vi_pipe] == TD_NULL) {
        g_isp_dfx_ctx[vi_pipe] = (isp_dfx_context *)osal_vmalloc(sizeof(isp_dfx_context));
        if (g_isp_dfx_ctx[vi_pipe] == TD_NULL) {
            isp_err_trace("pipe %d vmalloc dfx context size %llu failed!\n", vi_pipe, (td_u64)sizeof(isp_dfx_context));
            return;
        }
    }

    (td_void)memset_s(g_isp_dfx_ctx[vi_pipe], sizeof(*g_isp_dfx_ctx[vi_pipe]), 0, sizeof(isp_dfx_context));

    return;
}

td_void isp_dfx_ctx_deinit(ot_vi_pipe vi_pipe)
{
    if (g_isp_dfx_en.enable == 0) {
        return;
    }

    isp_check_pipe_void_return(vi_pipe);

    if (g_isp_dfx_ctx[vi_pipe] == TD_NULL) {
        return;
    }

    osal_vfree(g_isp_dfx_ctx[vi_pipe]);
    g_isp_dfx_ctx[vi_pipe] = TD_NULL;
}

td_void isp_dfx_sns_sync_cfg_show(const isp_drv_ctx *drv_ctx, td_u8 cur_node_idx, td_u8 stay_cnt, td_u32 update_pos)
{
    td_s32 i;
    ot_vi_pipe vi_pipe;
    td_u8 cfg_node_id;
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;
    isp_sync_cfg_buf_node *cur_node = drv_ctx->sync_cfg.node[cur_node_idx];
    ot_isp_i2c_data *i2c_data = TD_NULL;
    isp_dfx_sns_sync_verify_attr *dfx_verify_attr = TD_NULL;
    isp_check_pointer_void_return(cur_node);

    if (g_isp_dfx_en.bits.sns_cfg_sync_verify_en == TD_FALSE) {
        return;
    }

    vi_pipe = isp_drv_get_pipe_id(drv_ctx);
    dfx_verify_attr = &g_isp_dfx_ctx[vi_pipe]->sns_sync_verify_attr;

    if (is_isp_dfx_sns_cfg_update(drv_ctx, cur_node_idx, stay_cnt) == TD_TRUE) {
        dfx_verify_attr->verify_frm_cnt = drv_ctx->sync_cfg.cfg2_vld_dly_max + ISP_DFX_EXTRA;
    }

    for (i = 0; i < cur_node->sns_regs_info.reg_num; i++) {
        if (is_sensor_data_has_been_config(cur_node, i, stay_cnt) == TD_TRUE) {
            continue;
        }

        cfg_node_id = cur_node->sns_regs_info.i2c_data[i].delay_frame_num - stay_cnt;
        cfg_node = isp_drv_get_sns_cfg_node(drv_ctx, cur_node_idx, cfg_node_id);
        if (cfg_node == TD_NULL) {
            continue;
        }
        i2c_data = &cfg_node->sns_regs_info.i2c_data[i];

        if ((update_pos == i2c_data->interrupt_pos) && (dfx_verify_attr->verify_frm_cnt != 0)) {
            isp_err_trace("sns data[%d] = 0x%x, update = %d, cfg_dly_max %d, delay_frame_num = %d\n",
                i, i2c_data->data, i2c_data->update, drv_ctx->sync_cfg.cfg2_vld_dly_max, i2c_data->delay_frame_num);
            if (i2c_data->update == TD_TRUE && dfx_verify_attr->expect_change_cnt <= 0) {
                dfx_verify_attr->expect_change_cnt = drv_ctx->sync_cfg.cfg2_vld_dly_max - i2c_data->delay_frame_num + 1;
            }
        }
    }
}

td_void isp_dfx_sns_sync_verify_show(const isp_drv_ctx *drv_ctx, isp_stat_info *stat_info)
{
    ot_vi_pipe vi_pipe;
    isp_dfx_sns_sync_verify_attr *dfx_verify_attr = TD_NULL;
    isp_fe_stat *fe_stat = (isp_fe_stat*)stat_info->virt_addr;

    if (g_isp_dfx_en.bits.sns_cfg_sync_verify_en == TD_FALSE) {
        return;
    }

    vi_pipe = isp_drv_get_pipe_id(drv_ctx);
    dfx_verify_attr = &g_isp_dfx_ctx[vi_pipe]->sns_sync_verify_attr;

    if (dfx_verify_attr->verify_frm_cnt <= 0) {
        return;
    }

    if (drv_ctx->wdr_attr.pipe_num == 1) {
        if (dfx_verify_attr->expect_change_cnt == 0) {
            isp_err_trace("fe_statistics: global_avg_gb/global_avg_gr/global_avg_r/global_avg_b = %d %d %d %d \
            ===> expect change!\n",
                fe_stat->fe_ae_stat2.global_avg_gb[0], fe_stat->fe_ae_stat2.global_avg_gr[0],
                fe_stat->fe_ae_stat2.global_avg_r[0], fe_stat->fe_ae_stat2.global_avg_b[0]);
        } else {
            isp_err_trace("fe_statistics: global_avg_gb/global_avg_gr/global_avg_r/global_avg_b = %d %d %d %d\n",
                fe_stat->fe_ae_stat2.global_avg_gb[0], fe_stat->fe_ae_stat2.global_avg_gr[0],
                fe_stat->fe_ae_stat2.global_avg_r[0], fe_stat->fe_ae_stat2.global_avg_b[0]);
        }
        isp_err_trace("\n");
    } else {
        if (dfx_verify_attr->expect_change_cnt == 0) {
            isp_err_trace("fe_statistics: global_avg_gb = %d %d ===> expect change!\n",
                fe_stat->fe_ae_stat2.global_avg_gb[0], fe_stat->fe_ae_stat2.global_avg_gb[1]);
        } else {
            isp_err_trace("fe_statistics: global_avg_gb = %d %d\n",
                fe_stat->fe_ae_stat2.global_avg_gb[0], fe_stat->fe_ae_stat2.global_avg_gb[1]);
        }

        isp_err_trace("\n");
    }

    dfx_verify_attr->verify_frm_cnt--;
    dfx_verify_attr->expect_change_cnt--;
}

