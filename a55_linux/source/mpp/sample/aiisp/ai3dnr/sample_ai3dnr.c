/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_ai3dnr.h"
#include "sample_aiisp_common.h"
#include "sample_aiisp_mcf_common.h"
#include "ot_common_mcf.h"
#include "ss_mpi_mcf.h"
#include "ss_mpi_ai3dnr.h"

static ot_vb_pool g_ai3dnr_vb_pool = OT_VB_INVALID_POOL_ID;

#define AI3DNR_COMMON_VB_CNT 8
#define AI3DNR_YUV_NET_VB_CNT 6
#define AI3DNR_UV_NET_VB_CNT 3
#define AI3DNR_MCF_VB_CNT 20
static td_char *g_model_file = "./ai3dnr/model/ai3dnr_model.bin";

static td_s32 sample_ai3dnr_load_model(ot_size in_size, ot_ai3dnr_model *model_info, td_s32 *model_id)
{
    td_s32 ret;

    ot_aiisp_model *model = &model_info->model;

    ret = sample_aiisp_load_mem((ot_aiisp_mem_info *)&(model->mem_info), g_model_file);
    if (ret != TD_SUCCESS) {
        sample_print("sample_aiisp_load_mem error\n");
        return ret;
    }

    ret = ss_mpi_ai3dnr_load_model(model_info, model_id);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_load_model error(%#x)\n", ret);
        goto unload_mem;
    }

    return ret;

unload_mem:
    sample_aiisp_unload_mem((ot_aiisp_mem_info *)&(model->mem_info));
    return ret;
}

static td_s32 sample_ai3dnr_set_attr(ot_vi_pipe vi_pipe, sample_ai3dnr_param *param)
{
    td_s32 ret;
    ot_ai3dnr_attr ai3dnr_attr = {0};
    ot_3dnr_attr nr_attr = {0};
    ot_3dnr_param nr_param = {0};
    ot_3dnr_pos_type pos = (param->net_type == OT_AI3DNR_NET_TYPE_YUV) ? OT_3DNR_POS_VI : OT_3DNR_POS_VPSS;

    if (pos == OT_3DNR_POS_VI) {
        ret = ss_mpi_vi_get_pipe_3dnr_attr(vi_pipe, &nr_attr);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_get_pipe_3dnr_attr error\n");
            return ret;
        }

        nr_attr.enable = TD_TRUE;

        ret = ss_mpi_vi_set_pipe_3dnr_attr(vi_pipe, &nr_attr);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_set_pipe_3dnr_attr error\n");
            return ret;
        }
    }

    ret = ss_mpi_ai3dnr_get_attr(vi_pipe, &ai3dnr_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_get_attr error\n");
        return ret;
    }

    ret = ss_mpi_ai3dnr_set_attr(vi_pipe, &ai3dnr_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_set_attr error\n");
        return ret;
    }

    if (pos == OT_3DNR_POS_VI) {
        ret = ss_mpi_vi_get_pipe_3dnr_param(vi_pipe, &nr_param);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_get_pipe_3dnr_param error\n");
            return ret;
        }

        ret = ss_mpi_vi_set_pipe_3dnr_param(vi_pipe, &nr_param);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_set_pipe_3dnr_param error\n");
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_s32 sample_ai3dnr_start(ot_vi_pipe vi_pipe, ot_size in_size, ot_ai3dnr_model *model_info, td_s32 *model_id,
    sample_ai3dnr_param *param)
{
    td_s32 ret;
    ot_ai3dnr_cfg cfg = {0};

    ret = ss_mpi_ai3dnr_init();
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_init error(%#x)\n", ret);
        return ret;
    }

    model_info->model.preempted_en = TD_FALSE;
    model_info->model.image_size.width  = in_size.width;
    model_info->model.image_size.height = in_size.height;
    model_info->net_type = param->net_type;
    ret = sample_ai3dnr_load_model(in_size, model_info, model_id);
    if (ret != TD_SUCCESS) {
        goto task_exit;
    }

    cfg.net_type = param->net_type;
    ret = ss_mpi_ai3dnr_set_cfg(vi_pipe, &cfg);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_set_alg_cfg error(%#x)\n", ret);
        goto unload_cfg;
    }

    sample_get_char("enable ai3dnr");

    ret = ss_mpi_ai3dnr_enable(vi_pipe);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_ai3dnr_enable error(%#x)\n", ret);
        goto unload_cfg;
    }

    ret = sample_ai3dnr_set_attr(vi_pipe, param);
    if (ret != TD_SUCCESS) {
        sample_print("sample_ai3dnr_set_attr error(%#x)\n", ret);
        goto ai3dnr_disable;
    }

    return ret;

ai3dnr_disable:
    ss_mpi_ai3dnr_disable(vi_pipe);
unload_cfg:
    if (*model_id != -1) {
        (td_void)ss_mpi_ai3dnr_unload_model(*model_id);
    }
    sample_aiisp_unload_mem((ot_aiisp_mem_info *)&(model_info->model.mem_info));
task_exit:
    ss_mpi_ai3dnr_exit();

    return ret;
}

static td_s32 sample_ai3dnr_stop(ot_vi_pipe vi_pipe, ot_ai3dnr_model *model, td_s32 model_id)
{
    td_s32 return_value = TD_SUCCESS;
    td_s32 ret;

    ret = ss_mpi_ai3dnr_disable(vi_pipe);
    if (ret != TD_SUCCESS) {
        return_value = TD_FAILURE;
        sample_print("ss_mpi_ai3dnr_disable false error(%#x)\n", ret);
    }

    if (model_id != -1) {
        ret = ss_mpi_ai3dnr_unload_model(model_id);
        if (ret != TD_SUCCESS) {
            return_value = TD_FAILURE;
            sample_print("ss_mpi_ai3dnr_unload_cfg error(%#x)\n", ret);
        }
    }

    sample_aiisp_unload_mem((ot_aiisp_mem_info *)&(model->model.mem_info));

    ss_mpi_ai3dnr_exit();

    return return_value;
}

static td_s32 sample_ai3dnr_attach_vb(ot_vi_pipe vi_pipe, ot_ai3dnr_net_type net_type,
    ot_size *in_size, td_u32 vb_cnt)
{
    td_s32 ret;
    td_s32 blk_size;
    ot_vb_pool vb_pool;
    ot_aiisp_pool pool_attr = {0};
    ot_vb_pool_cfg vb_pool_cfg = {0};

    blk_size = ot_ai3dnr_get_pic_buf_size(in_size->width, in_size->height);
    vb_pool_cfg.blk_size = blk_size;
    vb_pool_cfg.blk_cnt = vb_cnt;
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    vb_pool = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (vb_pool == OT_VB_INVALID_POOL_ID) {
        sample_print("create user pool failed!\n");
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    ss_mpi_vb_pool_share_all(vb_pool);
#endif

    pool_attr.aiisp_type = OT_AIISP_TYPE_AI3DNR;
    pool_attr.ai3dnr_pool.vb_pool = vb_pool;
    pool_attr.ai3dnr_pool.net_type = net_type;

    ret = ss_mpi_vi_attach_aiisp_vb_pool(vi_pipe, &pool_attr);
    if (ret != TD_SUCCESS) {
        sample_print("vi aiisp attach vb pool failed.\n");
        return ret;
    }

    g_ai3dnr_vb_pool = vb_pool;

    return TD_SUCCESS;
}

static td_s32 sample_ai3dnr_detach_vb(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = ss_mpi_vi_detach_aiisp_vb_pool(vi_pipe, OT_AIISP_TYPE_AI3DNR);
    if (ret != TD_SUCCESS) {
        sample_print("vi aiisp detach vb pool failed.\n");
    }

    if (g_ai3dnr_vb_pool != OT_VB_INVALID_POOL_ID) {
        ss_mpi_vb_destroy_pool(g_ai3dnr_vb_pool);
        g_ai3dnr_vb_pool = OT_VB_INVALID_POOL_ID;
    }

    return TD_SUCCESS;
}

static td_void sample_ai3dnr_stop_vi(ot_vi_pipe vi_pipe, const sample_vi_cfg *vi_cfg)
{
    sample_ai3dnr_detach_vb(vi_pipe);
    sample_comm_vi_stop_vi(vi_cfg);
}

static td_s32 sample_ai3dnr_start_vi(ot_vi_pipe vi_pipe, const sample_vi_cfg *vi_cfg, ot_size *in_size,
    ot_ai3dnr_net_type net_type)
{
    td_s32 ret;
    ot_3dnr_pos_type pos = (net_type == OT_AI3DNR_NET_TYPE_YUV) ? OT_3DNR_POS_VI : OT_3DNR_POS_VPSS;
    td_u32 vb_cnt = (net_type == OT_AI3DNR_NET_TYPE_YUV) ? AI3DNR_YUV_NET_VB_CNT : AI3DNR_UV_NET_VB_CNT;

    if (sample_aiisp_sys_init(in_size, AI3DNR_COMMON_VB_CNT) != TD_SUCCESS) {
        sample_print("aiisp sys init failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_sys_set_3dnr_pos(pos);
    if (ret != TD_SUCCESS) {
        sample_print("set 3dnr pos failed! ret:0x%x\n", ret);
        goto sys_exit;
    }

    ret = sample_comm_vi_start_vi(vi_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vi failed.\n");
        goto sys_exit;
    }

    ret = sample_ai3dnr_attach_vb(vi_pipe, net_type, in_size, vb_cnt);
    if (ret != TD_SUCCESS) {
        sample_print("sample_ai3dnr_attach_vb failed.\n");
        goto stop_vi;
    }

    return TD_SUCCESS;

stop_vi:
    sample_comm_vi_stop_vi(vi_cfg);
sys_exit:
    sample_comm_sys_exit();

    return TD_FAILURE;
}

static td_void sample_ai3dnr_set_vi_cfg(ot_vi_pipe vi_pipe, ot_ai3dnr_net_type net_type, sample_vi_cfg *vi_cfg)
{
    vi_cfg->pipe_info[vi_pipe].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    vi_cfg->pipe_info[vi_pipe].nr_attr.enable = TD_FALSE;

    if (net_type == OT_AI3DNR_NET_TYPE_YUV) {
        vi_cfg->pipe_info[vi_pipe].nr_attr.enable = TD_TRUE;
    }

    vi_cfg->pipe_info[vi_pipe].bnr_bnf_num = 3; /* 3: ai3dnr should set bnr buf_num 3 */
}

td_s32 sample_ai3dnr(sample_ai3dnr_param *ai3dnr_param)
{
    td_s32 ret = TD_FAILURE;
    sample_vi_cfg vi_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_pipe master_pipe = 0;
    const ot_vi_chn vi_chn = 0;
    ot_vpss_grp vpss_grp[1] = {0};
    ot_size in_size = {0};
    ot_ai3dnr_model model_info = {0};
    td_s32 model_id = -1;

    if (ai3dnr_param->net_type != OT_AI3DNR_NET_TYPE_UV) {
        sample_print("sample_ai3dnr only support uv_net!\n");
        return TD_FAILURE;
    }

    sample_aiisp_get_default_cfg(sns_type, vi_pipe, &in_size, &vi_cfg);

    sample_ai3dnr_set_vi_cfg(vi_pipe, ai3dnr_param->net_type, &vi_cfg);

    if (sample_ai3dnr_start_vi(vi_pipe, &vi_cfg, &in_size, ai3dnr_param->net_type) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_aiisp_sensor_cfg(vi_pipe, sns_type);

    sample_comm_vi_bind_vpss(master_pipe, vi_chn, vpss_grp[0], 0);
    if (sample_aiisp_start_vpss(vpss_grp[0], &in_size) != TD_SUCCESS) {
        goto stop_vi;
    }

    if (sample_aiisp_start_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), &in_size) != TD_SUCCESS) {
        goto stop_vpss;
    }

    if (sample_ai3dnr_start(vi_pipe, in_size, &model_info, &model_id, ai3dnr_param) != TD_SUCCESS) {
        goto stop_venc_and_vo;
    }

    sample_get_char("disable");

    ret = sample_ai3dnr_stop(vi_pipe, &model_info, model_id);

    sample_get_char("exit");

stop_venc_and_vo:
    sample_aiisp_stop_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]));
stop_vpss:
    sample_aiisp_stop_vpss(vpss_grp[0]);
stop_vi:
    sample_comm_vi_un_bind_vpss(master_pipe, vi_chn, vpss_grp[0], 0);
    sample_ai3dnr_stop_vi(vi_pipe, &vi_cfg);
    sample_comm_sys_exit();
    return ret;
}

td_s32 sample_ai3dnr_and_mcf(td_void)
{
    td_s32 ret;
    const ot_mcf_grp mcf_grp = 0;
    sample_vi_cfg vi_cfg0, vi_cfg1;
    ot_mcf_grp_attr mcf_grp_attr;
    ot_size mono_size, color_size, large_size;
    ot_mcf_crop_info grp_crop = { 0 }; /* do not use mcf grp crop function */
    ot_vi_pipe vi_pipe;
    td_s32 model_id = -1;
    ot_ai3dnr_model model_info = {0};
    sample_ai3dnr_param ai3dnr_param = {OT_AI3DNR_NET_TYPE_UV};

    sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &mono_size);
    sample_comm_vi_get_size_by_sns_type(SENSOR1_TYPE, &color_size);

    sample_aiisp_mcf_get_large_size(mono_size, color_size, &large_size);
    if (sample_aiisp_sys_init(&large_size, AI3DNR_MCF_VB_CNT) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_aiisp_mcf_get_default_grp_attr(&mcf_grp_attr, &mono_size, &color_size);
    sample_aiisp_mcf_set_chn_defualt_outsize(large_size);

    ret = sample_aiisp_mcf_start_vi(&mcf_grp_attr, &vi_cfg1, &vi_cfg0, &color_size);
    if (ret != TD_SUCCESS) {
        goto sys_exit;
    }

    vi_pipe = mcf_grp_attr.color_pipe_attr.pipe_id;
    sample_aiisp_sensor_cfg(vi_pipe, SENSOR1_TYPE);
    ret = sample_aiisp_start_mcf_data_flow(mcf_grp, &mcf_grp_attr, &large_size, &grp_crop);
    if (ret != TD_SUCCESS) {
        goto stop_vi;
    }

    ret = sample_ai3dnr_start(vi_pipe, color_size, &model_info, &model_id, &ai3dnr_param);
    if (ret != TD_SUCCESS) {
        goto stop_mcf;
    }

    sample_get_char("exit");

    sample_ai3dnr_stop(vi_pipe, &model_info, model_id);

stop_mcf:
    sample_aiisp_stop_mcf_data_flow(mcf_grp, &mcf_grp_attr);
stop_vi:
    sample_aiisp_mcf_stop_vi(mcf_grp_attr.color_pipe_attr.pipe_id, &vi_cfg1, &vi_cfg0);
sys_exit:
    sample_comm_sys_exit();
    return ret;
}

