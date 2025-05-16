/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "securec.h"
#include "ot_scene_inner.h"
#include "ot_scenecomm.h"
#include "ot_scene_setparam.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_sys.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* notes scene mode */
static ot_scene_mode g_scene_mode;

/* notes scene state */
static scene_state g_scene_state;

static ot_scene_fps g_scene_auto_fps = {0};

static td_s32 g_init_iso = -1;
static td_s64 g_init_exp = -1;

/* scene lock */
pthread_mutex_t g_scene_lock = PTHREAD_MUTEX_INITIALIZER;


/* -------------------------internal function interface------------------------- */
static td_void scene_get_last_exposure(scene_thread_type thread_type, td_s32 i, td_u64 *last_exposure)
{
    switch (thread_type) {
        case SCENE_THREAD_TYPE_NORMAL:
            *last_exposure = g_scene_state.main_pipe[i].last_normal_exposure;
            break;
        case SCENE_THREAD_TYPE_LUMINANCE:
            *last_exposure = g_scene_state.main_pipe[i].last_luminance_exposure;
            break;
        case SCENE_THREAD_TYPE_NOTLINEAR:
            *last_exposure = g_scene_state.main_pipe[i].last_not_linear_exposure;
            break;
        default:
            scene_loge("Error ThreadType");
            break;
    }
}

static td_void scene_get_last_iso(scene_thread_type thread_type, td_s32 i, td_u32 *last_iso)
{
    switch (thread_type) {
        case SCENE_THREAD_TYPE_NORMAL:
            *last_iso = g_scene_state.main_pipe[i].last_normal_iso;
            break;
        case SCENE_THREAD_TYPE_LUMINANCE:
            *last_iso = g_scene_state.main_pipe[i].last_luminance_iso;
            break;
        case SCENE_THREAD_TYPE_NOTLINEAR:
            *last_iso = g_scene_state.main_pipe[i].last_not_linear_iso;
            break;
        default:
            scene_loge("Error ThreadType");
            break;
    }
}

td_void mcf_pipe_change(ot_vi_pipe *vi_pipe, td_u8 index)
{
    if (g_scene_mode.pipe_attr[index].pipe_type == OT_SCENE_PIPE_TYPE_MCF_3DNR) {
        *vi_pipe = 1;
    } else {
    }
}


static td_s32 scene_set_dynamic_param_by_param(scene_set_dynamic_by_param_cb func_param,
    scene_thread_type thread_type, scene_dynamic_change_type dynamic_change_type)
{
    ot_scenecomm_check_pointer_return(func_param, TD_FAILURE);
    td_u32 i, j;
    td_s32 ret;
    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        for (j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            if (dynamic_change_type == SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE) {
                td_u64 exposure = g_scene_state.main_pipe[i].exposure;
                td_u64 last_exposure = 0;
                scene_get_last_exposure(thread_type, i, &last_exposure);
                td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
                ot_scenecomm_expr_true_return(index >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
                td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;
                ret = func_param(index, exposure, last_exposure, pipe_index);
            } else {
                td_u32 iso = g_scene_state.main_pipe[i].iso;
                td_u32 last_iso = 0;
                scene_get_last_iso(thread_type, i, &last_iso);
                td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
                ot_scenecomm_expr_true_return(index >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
                td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;
                ret = func_param(index, iso, last_iso, pipe_index);
            }
            ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
        }
    }
    return TD_SUCCESS;
}

static td_void scene_get_mainpipe_array(td_u32 *pipe_cnt)
{
    td_u32 main_pipe_cnt = 0;
    td_u32 j;

    /* get mainpipe array */
    for (td_s32 i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        if (g_scene_mode.pipe_attr[i].enable != TD_TRUE) {
            continue;
        }

        if (main_pipe_cnt == 0) {
            g_scene_state.main_pipe[main_pipe_cnt].main_pipe_hdl = g_scene_mode.pipe_attr[i].main_pipe_hdl;
            main_pipe_cnt++;
            continue;
        }

        for (j = 0; j < main_pipe_cnt; j++) {
            if (g_scene_state.main_pipe[j].main_pipe_hdl == g_scene_mode.pipe_attr[i].main_pipe_hdl) {
                break;
            }
        }

        if (main_pipe_cnt == j) {
            g_scene_state.main_pipe[main_pipe_cnt].main_pipe_hdl = g_scene_mode.pipe_attr[i].main_pipe_hdl;
            main_pipe_cnt++;
        }
    }
    *pipe_cnt = main_pipe_cnt;
}

static td_s32 scene_set_main_pipe_state(const ot_scene_mode *scene_mode)
{
    td_u32 i, j;
    td_u32 main_pipe_cnt = 0;
    /* if not use, set to 0 */
    (td_void)memset_s(g_scene_state.main_pipe, sizeof(g_scene_state.main_pipe), 0, sizeof(g_scene_state.main_pipe));

    for (i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        g_scene_state.main_pipe[i].long_exp = TD_FALSE;
        g_scene_state.main_pipe[i].metry_fixed = TD_FALSE;
    }

    /* get mainpipe array */
    scene_get_mainpipe_array(&main_pipe_cnt);

    /* set subpipe in certain mainpipe */
    for (i = 0; i < main_pipe_cnt; i++) {
        td_u32 sub_pipe_cnt = 0;

        for (j = 0; j < OT_SCENE_PIPE_MAX_NUM; j++) {
            if (g_scene_mode.pipe_attr[j].enable != TD_TRUE) {
                continue;
            }

            ot_scenecomm_expr_true_return(sub_pipe_cnt >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
            if (g_scene_state.main_pipe[i].main_pipe_hdl == g_scene_mode.pipe_attr[j].main_pipe_hdl) {
                g_scene_state.main_pipe[i].sub_pipe_hdl[sub_pipe_cnt] = g_scene_mode.pipe_attr[j].vcap_pipe_hdl;
                sub_pipe_cnt++;
            }
        }

        g_scene_state.main_pipe[i].sub_pipe_num = sub_pipe_cnt;
    }

    g_scene_state.main_pipe_num = main_pipe_cnt;

    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        scene_logd("The mainpipe is %u.", g_scene_state.main_pipe[i].main_pipe_hdl);
        for (j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            scene_logd("The subpipe in mainpipe %d is %u.", g_scene_state.main_pipe[i].main_pipe_hdl,
                g_scene_state.main_pipe[i].sub_pipe_hdl[j]);
        }
        scene_logd("\n");
    }
    return TD_SUCCESS;
}

static td_s32 scene_calculate_exp(ot_vi_pipe vi_pipe, td_u32 *out_iso, td_u64 *out_exposure)
{
    td_s32 ret;
    td_u64 exposure;
    td_u32 iso;

    ot_isp_exp_info isp_exp_info;
    ot_isp_pub_attr pub_attr;

    if (g_scene_state.pause == TD_TRUE) {
        return TD_SUCCESS;
    }

    ret = ss_mpi_isp_query_exposure_info(vi_pipe, &isp_exp_info);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ss_mpi_isp_get_pub_attr(vi_pipe, &pub_attr);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    iso = isp_exp_info.iso;

    if (pub_attr.wdr_mode == OT_WDR_MODE_4To1_LINE) {
        exposure = ((td_u64)iso * isp_exp_info.long_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_3To1_LINE) {
        exposure = ((td_u64)iso * isp_exp_info.median_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_2To1_LINE) {
        exposure = ((td_u64)iso * isp_exp_info.short_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_2To1_FRAME) {
        exposure = ((td_u64)iso * isp_exp_info.short_exp_time) / 100; /* 100 as AE ISO ratio */
    } else {
        exposure = ((td_u64)iso * isp_exp_info.exp_time) / 100; /* 100 as AE ISO ratio */
    }
    *out_iso = iso;
    *out_exposure = exposure;

    return TD_SUCCESS;
}

static td_s32 scene_calculate_wdr_param(ot_vi_pipe vi_pipe, td_u32 *wdr_ratio)
{
    td_s32 ret;

    ot_isp_inner_state_info inner_state_info;

    if (g_scene_state.pause == TD_TRUE) {
        return TD_SUCCESS;
    }

    ret = ss_mpi_isp_query_inner_state_info(vi_pipe, &inner_state_info);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    *wdr_ratio = inner_state_info.wdr_exp_ratio_actual[0];

    return TD_SUCCESS;
}

static td_s32 scene_set_main_pipe_special_param(ot_vi_pipe vi_pipe, td_bool metry_fixed)
{
    td_s32 ret;

    ret = ot_scene_set_static_ae(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_awb(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_awbex(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_saturation(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_wdr_exposure(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_sharpen(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_demosaic(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_bayershp(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_ca(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_venc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_crosstalk(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = ot_scene_set_static_clut(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    ret = ot_scene_set_static_fswdr(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
#ifdef CONFIG_SCENEAUTO_AIDRC_SUPPORT
    ret = ot_scene_set_static_aidrc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
#endif
    ret = ot_scene_set_static_drc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    ret = ot_scene_set_static_shading(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    ret = ot_scene_set_static_pregamma(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    ret = ot_scene_set_static_blc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    return TD_SUCCESS;
}

static td_s32 scene_set_pipe_param(ot_scene_set_static_param func)
{
    ot_scenecomm_check_pointer_return(func, TD_FAILURE);
    td_s32 ret;
    for (td_s32 i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        if (g_scene_mode.pipe_attr[i].enable != TD_TRUE) {
            continue;
        }
        ret = func(g_scene_mode.pipe_attr[i].vcap_pipe_hdl, g_scene_mode.pipe_attr[i].pipe_param_index);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }
    return TD_SUCCESS;
}

static td_s32 scene_set_pipe_static_param(td_void)
{
    td_s32 ret;

    ret = scene_set_pipe_param(ot_scene_set_static_ccm);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_dehaze);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_nr);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_csc);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_ldci);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_cac);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_dpc);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ret = scene_set_pipe_param(ot_scene_set_static_pipe_diff);
    ot_scenecomm_check_return(ret, TD_FAILURE);
    return TD_SUCCESS;
}

static td_s32 scene_set_pipe_dynamic_param_part1(td_void)
{
    scene_set_dynamic_by_param_cb func_false_color = ot_scene_set_dynamic_false_color;
    td_s32 ret = scene_set_dynamic_param_by_param(func_false_color, SCENE_THREAD_TYPE_NORMAL,
                                                  SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_set_dynamic_by_param_cb func_shading = ot_scene_set_dynamic_shading;
    ret = scene_set_dynamic_param_by_param(func_shading, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    scene_set_dynamic_by_param_cb func_dpc = ot_scene_set_dynamic_dpc;
    ret = scene_set_dynamic_param_by_param(func_dpc, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_set_dynamic_by_param_cb func_blc = ot_scene_set_dynamic_blc;
    ret = scene_set_dynamic_param_by_param(func_blc, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    scene_set_dynamic_by_param_cb func_clut = ot_scene_set_dynamic_clut;
    ret = scene_set_dynamic_param_by_param(func_clut, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_set_dynamic_by_param_cb func_fps = ot_scene_set_dynamic_fps;
    ret = scene_set_dynamic_param_by_param(func_fps, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    scene_set_dynamic_by_param_cb func_linear_ca = ot_scene_set_dynamic_linear_ca;
    ret = scene_set_dynamic_param_by_param(func_linear_ca, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_set_dynamic_by_param_cb func_ldci = ot_scene_set_dynamic_ldci;
    ret = scene_set_dynamic_param_by_param(func_ldci, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    scene_set_dynamic_by_param_cb func_dehaze = ot_scene_set_dynamic_dehaze;
    ret = scene_set_dynamic_param_by_param(func_dehaze, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_set_dynamic_by_param_cb func_venc_bitrate = ot_scene_set_dynamic_venc_bitrate;
    ret = scene_set_dynamic_param_by_param(func_venc_bitrate, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    scene_set_dynamic_by_param_cb func_venc_mode = ot_scene_set_dynamic_venc_mode;
    ret = scene_set_dynamic_param_by_param(func_venc_mode, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    return TD_SUCCESS;
}

static td_s32 scene_set_pipe_dynamic_param(td_void)
{
    td_s32 ret;
    ot_3dnr_pos_type pos = OT_3DNR_POS_VI;

    ret = ss_mpi_sys_get_3dnr_pos(&pos);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    ret = scene_set_pipe_dynamic_param_part1();
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

#ifdef CONFIG_SCENEAUTO_AI3DNR_SUPPORT
    scene_set_dynamic_by_param_cb func_mcf = ot_scene_set_dynamic_mcf;
    ret = scene_set_dynamic_param_by_param(func_mcf, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
#endif

#ifdef CONFIG_SCENEAUTO_AIBNR_SUPPORT
    scene_set_dynamic_by_param_cb func_aibnr = ot_scene_set_dynamic_aibnr;
    ret = scene_set_dynamic_param_by_param(func_aibnr, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
#endif

#ifdef CONFIG_SCENEAUTO_AI3DNR_SUPPORT
    scene_set_dynamic_by_param_cb func_ai3dnr = ot_scene_set_dynamic_ai3dnr;
    ret = scene_set_dynamic_param_by_param(func_ai3dnr, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
#endif

    scene_set_dynamic_by_param_cb func_fpn = ot_scene_set_dynamic_fpn;
    ret = scene_set_dynamic_param_by_param(func_fpn, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    if (g_scene_mode.pipe_mode == OT_SCENE_PIPE_MODE_LINEAR) {
        scene_set_dynamic_by_param_cb func_nr = ot_scene_set_dynamic_linear_drc;
        ret = scene_set_dynamic_param_by_param(func_nr, SCENE_THREAD_TYPE_NORMAL, SCENE_DYNAMIC_CHANGE_TYPE_ISO);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }
    for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
        for (td_u32 j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            td_u32 iso;
            td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
            td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;
            iso = g_scene_state.main_pipe[i].iso;
            if (pos == OT_3DNR_POS_VI) {
                ret = ot_scene_set_dynamic_3dnr(g_scene_mode.pipe_attr[index].vcap_pipe_hdl, iso, pipe_index, pos);
            } else if (pos == OT_3DNR_POS_VPSS) {
                ret = ot_scene_set_dynamic_3dnr(g_scene_mode.pipe_attr[index].vpss_hdl, iso, pipe_index, pos);
            }
            ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
        }
    }

    return TD_SUCCESS;
}

static td_s32 scene_set_vi_pipe_param(td_void)
{
    td_s32 ret;

    /* set mainIsp param */
    for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
        td_bool metry_fixed;

        metry_fixed = g_scene_state.main_pipe[i].metry_fixed;

        ret = scene_set_main_pipe_special_param(vi_pipe, metry_fixed);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

        mcf_pipe_change(&vi_pipe, i);
        if ((g_scene_state.thread_normal.thread_flag == 0) && (g_init_iso != -1) && (g_init_exp != -1)) {
            scene_logd("init iso: %d exp: %lld\n", g_init_iso, g_init_exp);
            g_scene_state.main_pipe[i].iso = g_init_iso;
            g_scene_state.main_pipe[i].exposure = g_init_exp;
        } else {
            ret =
                scene_calculate_exp(vi_pipe, &(g_scene_state.main_pipe[i].iso), &(g_scene_state.main_pipe[i].exposure));
        }
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    ret = scene_set_pipe_static_param();
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    return TD_SUCCESS;
}

static td_s32 scene_set_vpss_param(td_void)
{
    return TD_SUCCESS;
}

static td_s32 scene_set_venc_param(td_void)
{
    return TD_SUCCESS;
}

td_void *scene_normal_auto_thread(td_void *arg)
{
    td_s32 ret;
    td_u32 i, j;
    static td_bool first_loop = TD_TRUE;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"OT_SCENE_NormalThread", 0, 0, 0);

    while (g_scene_state.thread_normal.thread_flag == TD_TRUE) {
        if (g_scene_state.pause == TD_TRUE) {
            usleep(10000); /* 10000 10 ms */
            continue;
        }
        if (first_loop == 0) {
            for (i = 0; i < g_scene_state.main_pipe_num; i++) {
                ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;

                mcf_pipe_change(&vi_pipe, i);

                ret = scene_calculate_exp(vi_pipe, &(g_scene_state.main_pipe[i].iso),
                    &(g_scene_state.main_pipe[i].exposure));
                ot_scenecomm_check(ret, OT_SCENE_EINTER);
            }
        } else {
            /* iso&exposure init val already set in scene_set_vi_pipe_param */
            first_loop = TD_FALSE;
        }

        ret = scene_set_pipe_dynamic_param();
        ot_scenecomm_check(ret, OT_SCENE_EINTER);

        scene_set_dynamic_by_param_cb func_ae = ot_scene_set_dynamic_ae;
        ret = scene_set_dynamic_param_by_param(func_ae, SCENE_THREAD_TYPE_LUMINANCE,
            SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);

        for (i = 0; i < g_scene_state.main_pipe_num; i++) {
            g_scene_state.main_pipe[i].last_normal_exposure = g_scene_state.main_pipe[i].exposure;
            g_scene_state.main_pipe[i].last_normal_iso = g_scene_state.main_pipe[i].iso;
        }

        usleep(200000); /* 200000 200 ms */
    }

    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        for (j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num &&
             (g_scene_state.main_pipe[i].sub_pipe_hdl[j] < OT_SCENE_PIPE_MAX_NUM); j++) {
            td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
            td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;
            ret = ot_scene_release_fpn(index, pipe_index);
            ot_scenecomm_check(ret, OT_SCENE_EINTER);
        }
    }

    return TD_NULL;
}

static td_s32 scene_set_auto_luminance_gamma(td_void)
{
    td_s32 ret;
    for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
        for (td_u32 j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            td_u64 exposure = g_scene_state.main_pipe[i].exposure;
            td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
            td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;

            if (g_scene_mode.pipe_attr[index].pipe_type == OT_SCENE_PIPE_TYPE_SNAP) {
                ret = ot_scene_set_dynamic_photo_gamma(index, exposure,
                    g_scene_state.main_pipe[i].last_luminance_exposure, pipe_index);
                ot_scenecomm_check(ret, OT_SCENE_EINTER);
            } else if (g_scene_mode.pipe_attr[index].pipe_type == OT_SCENE_PIPE_TYPE_VIDEO ||
                       g_scene_mode.pipe_attr[index].pipe_type == OT_SCENE_PIPE_TYPE_MCF_3DNR) {
                ret = ot_scene_set_dynamic_video_gamma(index, exposure,
                    g_scene_state.main_pipe[i].last_luminance_exposure, pipe_index);
                ot_scenecomm_check(ret, OT_SCENE_EINTER);
            }
        }
    }
    return TD_SUCCESS;
}

td_void *scene_luminance_auto_thread(td_void *arg)
{
    td_s32 ret;
    td_u32 i;
    static td_bool first_loop = TD_TRUE;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"OT_SCENE_LuminanceThread", 0, 0, 0);

    while (g_scene_state.thread_luminance.thread_flag == TD_TRUE) {
        if (g_scene_state.pause == TD_TRUE) {
            usleep(10000); /* 10000 10 ms */
            continue;
        }
        if (first_loop == 0) {
            /* set AE compesation with exposure calculated by effective ISP dev */
            for (i = 0; i < g_scene_state.main_pipe_num; i++) {
                ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;

                mcf_pipe_change(&vi_pipe, i);

                ret = scene_calculate_exp(vi_pipe, &(g_scene_state.main_pipe[i].iso),
                    &(g_scene_state.main_pipe[i].exposure));
                ot_scenecomm_check(ret, OT_SCENE_EINTER);
            }
        } else {
            /* iso&exposure init val already set in scene_set_vi_pipe_param */
            first_loop = TD_FALSE;
        }

        scene_set_dynamic_by_param_cb func_fps = ot_scene_set_dynamic_fps;
        ret = scene_set_dynamic_param_by_param(func_fps, SCENE_THREAD_TYPE_LUMINANCE,
            SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);

        ot_scenecomm_check(ret, OT_SCENE_EINTER);

        ret = scene_set_auto_luminance_gamma();
        ot_scenecomm_check(ret, OT_SCENE_EINTER);

        for (i = 0; i < g_scene_state.main_pipe_num; i++) {
            g_scene_state.main_pipe[i].last_luminance_exposure = g_scene_state.main_pipe[i].exposure;
            g_scene_state.main_pipe[i].last_luminance_iso = g_scene_state.main_pipe[i].iso;
        }

        usleep(100000); /* 100000 100ms */
    }

    return TD_NULL;
}

static td_s32 scene_not_linear_auto_part1(td_void)
{
    td_u32 i;
    td_s32 ret;
    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
        ot_scenecomm_expr_true_return(vi_pipe >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
        ret = ot_scene_set_dynamic_fswdr(vi_pipe, g_scene_state.main_pipe[i].iso,
            g_scene_state.main_pipe[i].last_not_linear_iso, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index,
            g_scene_state.main_pipe[i].wdr_ratio);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
    }
    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
        ot_scenecomm_expr_true_return(vi_pipe >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
#ifdef CONFIG_SCENEAUTO_AIDRC_SUPPORT
        ret = ot_scene_set_dynamic_aidrc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index,
            g_scene_state.main_pipe[i].wdr_ratio, g_scene_state.main_pipe[i].iso);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
#endif
        ret = ot_scene_set_dynamic_drc(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index,
            g_scene_state.main_pipe[i].wdr_ratio, g_scene_state.main_pipe[i].iso);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
    }
    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
        ot_scenecomm_expr_true_return(vi_pipe >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
        ret = ot_scene_set_dynamic_nr(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index,
            g_scene_state.main_pipe[i].wdr_ratio, g_scene_state.main_pipe[i].iso);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
    }
    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
        ot_scenecomm_expr_true_return(vi_pipe >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
        ret = ot_scene_set_dynamic_ca(vi_pipe, g_scene_mode.pipe_attr[vi_pipe].pipe_param_index,
            g_scene_state.main_pipe[i].wdr_ratio, g_scene_state.main_pipe[i].iso);
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
    }
    return TD_SUCCESS;
}

td_void *scene_not_linear_auto_thread(td_void *arg)
{
    td_s32 ret;
    td_u32 i;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"OT_SCENE_NotLinearThread", 0, 0, 0);

    while (g_scene_state.thread_not_linear.thread_flag == TD_TRUE) {
        if (g_scene_state.pause == TD_TRUE) {
            usleep(10000); /* 10000 10 ms */
            continue;
        }
        if ((g_scene_mode.pipe_mode != OT_SCENE_PIPE_MODE_WDR) && (g_scene_mode.pipe_mode != OT_SCENE_PIPE_MODE_HDR)) {
            usleep(1000000); /* 1000000 1000ms */
            continue;
        }

        for (i = 0; i < g_scene_state.main_pipe_num; i++) {
            ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;

            mcf_pipe_change(&vi_pipe, i);
            ret =
                scene_calculate_exp(vi_pipe, &(g_scene_state.main_pipe[i].iso), &(g_scene_state.main_pipe[i].exposure));
            ot_scenecomm_check(ret, OT_SCENE_EINTER);
            ret = scene_calculate_wdr_param(vi_pipe, &(g_scene_state.main_pipe[i].wdr_ratio));
            ot_scenecomm_check(ret, OT_SCENE_EINTER);
        }

        ret = scene_not_linear_auto_part1();
        ot_scenecomm_check(ret, OT_SCENE_EINTER);

        for (i = 0; i < g_scene_state.main_pipe_num; i++) {
            g_scene_state.main_pipe[i].last_not_linear_exposure = g_scene_state.main_pipe[i].exposure;
            g_scene_state.main_pipe[i].last_not_linear_iso = g_scene_state.main_pipe[i].iso;
        }

        usleep(100000); /* 100000 100ms */
    }
    return TD_NULL;
}

void *scene_venc_auto_thread(td_void *arg)
{
    td_s32 ret;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"ot_AutoVenc", 0, 0, 0);
    while (g_scene_state.thread_venc.thread_flag == TD_TRUE) {
        if (g_scene_state.pause == TD_TRUE) {
            usleep(10000); /* 10000 10 ms */
            continue;
        }
        /* Set VENC RC PARAM */
        for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
            ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;
            td_s32 index = g_scene_mode.pipe_attr[i].pipe_param_index;

            ret = ot_scene_set_rc_param(vi_pipe, index);
            ot_scenecomm_check(ret, OT_SCENE_EINTER);
        }

        usleep(200000); /* 200000 200ms */
    }
    return NULL;
}


#define SCENE_THREAD_STACK_SIZE 0x20000
static td_s32 scene_start_auto_thread(td_void)
{
    td_s32 ret;

    pthread_attr_init(&(g_scene_state.thread_normal_attr));
    pthread_attr_setdetachstate(&(g_scene_state.thread_normal_attr), PTHREAD_CREATE_DETACHED);
#ifdef __LITEOS__
    pthread_attr_setstacksize(&(g_scene_state.thread_normal_attr), SCENE_THREAD_STACK_SIZE);
#endif

    pthread_attr_init(&(g_scene_state.thread_luminance_attr));
    pthread_attr_setdetachstate(&(g_scene_state.thread_luminance_attr), PTHREAD_CREATE_DETACHED);
#ifdef __LITEOS__
    pthread_attr_setstacksize(&(g_scene_state.thread_luminance_attr), SCENE_THREAD_STACK_SIZE);
#endif
    pthread_attr_init(&(g_scene_state.thread_not_linear_attr));
    pthread_attr_setdetachstate(&(g_scene_state.thread_not_linear_attr), PTHREAD_CREATE_DETACHED);
#ifdef __LITEOS__
    pthread_attr_setstacksize(&(g_scene_state.thread_not_linear_attr), SCENE_THREAD_STACK_SIZE);
#endif
    pthread_attr_init(&(g_scene_state.thread_venc_attr));
    pthread_attr_setdetachstate(&(g_scene_state.thread_venc_attr), PTHREAD_CREATE_DETACHED);
#ifdef __LITEOS__
    pthread_attr_setstacksize(&(g_scene_state.thread_venc_attr), SCENE_THREAD_STACK_SIZE);
#endif
    if (g_scene_state.thread_normal.thread_flag == TD_FALSE) {
        g_scene_state.thread_normal.thread_flag = TD_TRUE;
        ret = pthread_create(&g_scene_state.thread_normal.thread, &(g_scene_state.thread_normal_attr),
            scene_normal_auto_thread, NULL);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }
    if (g_scene_state.thread_luminance.thread_flag == TD_FALSE) {
        g_scene_state.thread_luminance.thread_flag = TD_TRUE;
        ret = pthread_create(&g_scene_state.thread_luminance.thread, &(g_scene_state.thread_luminance_attr),
            scene_luminance_auto_thread, NULL);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    if ((g_scene_state.thread_not_linear.thread_flag == TD_FALSE) &&
        ((g_scene_mode.pipe_mode == OT_SCENE_PIPE_MODE_WDR) || (g_scene_mode.pipe_mode == OT_SCENE_PIPE_MODE_HDR))) {
        g_scene_state.thread_not_linear.thread_flag = TD_TRUE;
        ret = pthread_create(&g_scene_state.thread_luminance.thread, &(g_scene_state.thread_not_linear_attr),
            scene_not_linear_auto_thread, NULL);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    if (g_scene_state.thread_venc.thread_flag == TD_FALSE) {
        g_scene_state.thread_venc.thread_flag = TD_TRUE;
        ret = pthread_create(&g_scene_state.thread_venc.thread, &(g_scene_state.thread_venc_attr),
            scene_venc_auto_thread, NULL);
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    return TD_SUCCESS;
}

static td_s32 scene_stop_auto_thread(td_void)
{
    if (g_scene_state.thread_normal.thread_flag == TD_TRUE) {
        g_scene_state.thread_normal.thread_flag = TD_FALSE;
        pthread_attr_destroy(&(g_scene_state.thread_normal_attr));
    }

    if (g_scene_state.thread_luminance.thread_flag == TD_TRUE) {
        g_scene_state.thread_luminance.thread_flag = TD_FALSE;
        pthread_attr_destroy(&(g_scene_state.thread_luminance_attr));
    }

    if (g_scene_state.thread_not_linear.thread_flag == TD_TRUE) {
        g_scene_state.thread_not_linear.thread_flag = TD_FALSE;
        pthread_attr_destroy(&(g_scene_state.thread_not_linear_attr));
    }
    if (g_scene_state.thread_venc.thread_flag == TD_TRUE) {
        g_scene_state.thread_venc.thread_flag = TD_FALSE;
        pthread_attr_destroy(&(g_scene_state.thread_venc_attr));
    }
    return TD_SUCCESS;
}

/* -------------------------external function interface------------------------- */
td_s32 ot_scene_init(const ot_scene_param *scene_param)
{
    ot_scenecomm_check_pointer_return(scene_param, OT_SCENE_ENONPTR);

    td_s32 ret;

    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_TRUE) {
        ot_mutex_unlock(g_scene_lock);
        scene_loge("SCENE module has already been inited.\n");
        return OT_SCENE_EINITIALIZED;
    }

    ret = ot_scene_set_pipe_param(scene_param->pipe_param, OT_SCENE_PIPETYPE_NUM);
    if (ret != TD_SUCCESS) {
        ot_mutex_unlock(g_scene_lock);
        scene_loge("SCENE module has already been inited.\n");
        return OT_SCENE_EINTER;
    }

    (td_void)memset_s(&g_scene_mode, sizeof(ot_scene_mode), 0, sizeof(ot_scene_mode));

    (td_void)memset_s(&g_scene_state, sizeof(scene_state), 0, sizeof(scene_state));

    g_scene_state.scene_init = TD_TRUE;
    g_scene_state.pause = TD_TRUE;
    ret = ot_scene_set_pause(TD_TRUE);
    ot_mutex_unlock(g_scene_lock);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    scene_logd("SCENE module has been inited successfully.\n");

    return TD_SUCCESS;
}

static td_s32 scene_check_pipe_attr(const ot_scene_mode *scene_mode)
{
    for (td_u32 i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        if (scene_mode->pipe_attr[i].enable != TD_TRUE) {
            continue;
        }

        if (scene_mode->pipe_attr[i].vcap_pipe_hdl != i) {
            scene_loge("The value of pipe in scene pipe array must be equal to the index of array!\n");
            return OT_SCENE_EINVAL;
        }

        if ((scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_SNAP) &&
            (scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_VIDEO) &&
            (scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_MCF_3DNR)) {
            scene_loge("Pipe Type is not video or snap!\n");
            return OT_SCENE_EOUTOFRANGE;
        }

        if (scene_mode->pipe_attr[i].pipe_param_index >= OT_SCENE_PIPETYPE_NUM) {
            scene_loge("Pipe param index is out of range!\n");
            return OT_SCENE_EOUTOFRANGE;
        }
    }
    return TD_SUCCESS;
}

td_s32 ot_scene_set_scene_mode(const ot_scene_mode *scene_mode)
{
    ot_scenecomm_check_pointer_return(scene_mode, OT_SCENE_ENONPTR);

    td_s32 ret;

    if (scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_LINEAR && scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_WDR &&
        scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_HDR) {
        scene_loge("The pipe mode must be LINEAR, WDR or HDR.\n");
        return OT_SCENE_EINVAL;
    }

    ret = scene_check_pipe_attr(scene_mode);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    ot_mutex_lock(g_scene_lock);
    (td_void)memcpy_s(&g_scene_mode, sizeof(ot_scene_mode), scene_mode, sizeof(ot_scene_mode));

    ret = scene_set_main_pipe_state(scene_mode);
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_SetMainIspState failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return TD_FAILURE;
    }

    ret = scene_set_vi_pipe_param();
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_SetIspParam failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return TD_FAILURE;
    }

    ret = scene_set_vpss_param();
    if (ret != TD_SUCCESS) {
        scene_loge("scene_set_vpss_param failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return TD_FAILURE;
    }

    ret = scene_set_venc_param();
    if (ret != TD_SUCCESS) {
        scene_loge("scene_set_venc_param failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return TD_FAILURE;
    }

    ret = scene_start_auto_thread();
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_StartThread failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return TD_FAILURE;
    }

    ot_mutex_unlock(g_scene_lock);

    return TD_SUCCESS;
}

td_s32 ot_scene_deinit(td_void)
{
    td_s32 ret;

    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    ret = scene_stop_auto_thread();
    if (ret != TD_SUCCESS) {
        scene_loge("scene_stop_auto_thread fail!\n");
    }
    g_scene_state.scene_init = TD_FALSE;
    g_init_iso = -1;
    g_init_exp = -1;
    ot_mutex_unlock(g_scene_lock);
    scene_logd("SCENE Module has been deinited successfully!\n");

    return TD_SUCCESS;
}

td_s32 ot_scene_get_scene_mode(ot_scene_mode *scene_mode)
{
    ot_scenecomm_check_pointer_return(scene_mode, OT_SCENE_ENONPTR);

    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    (td_void)memcpy_s(scene_mode, sizeof(ot_scene_mode), &g_scene_mode, sizeof(ot_scene_mode));
    ot_mutex_unlock(g_scene_lock);

    return TD_SUCCESS;
}

td_s32 ot_scene_set_scene_init_exp(td_s32 iso, td_s64 exp)
{
    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    if (g_scene_state.thread_venc.thread_flag != 0) {
        ot_mutex_unlock(g_scene_lock);
        scene_loge("scene auto thread in running, not support setinitExp \n");
        return TD_FAILURE;
    }
    g_init_iso = iso;
    g_init_exp = exp;
    ot_mutex_unlock(g_scene_lock);

    return TD_SUCCESS;
}

td_s32 ot_scene_set_scene_fps(const ot_scene_fps *scene_fps)
{
    td_s32 ret;
    ot_scenecomm_check_pointer_return(scene_fps, OT_SCENE_ENONPTR);
    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    (td_void)memcpy_s(&g_scene_auto_fps, sizeof(ot_scene_fps), scene_fps, sizeof(ot_scene_fps));
    scene_set_dynamic_by_param_cb func_fps = ot_scene_set_dynamic_fps;
    ret = scene_set_dynamic_param_by_param(func_fps, SCENE_THREAD_TYPE_LUMINANCE, SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE);
    ot_mutex_unlock(g_scene_lock);

    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    return TD_SUCCESS;
}

td_s32 ot_scene_get_scene_fps(ot_scene_fps *scene_fps)
{
    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    (td_void)memcpy_s(scene_fps, sizeof(ot_scene_fps), &g_scene_auto_fps, sizeof(ot_scene_fps));
    ot_mutex_unlock(g_scene_lock);

    return TD_SUCCESS;
}

td_s32 ot_scene_pause(td_bool enable)
{
    td_s32 ret;

    ot_mutex_lock(g_scene_lock);
    if (g_scene_state.scene_init == TD_FALSE) {
        scene_loge("sceneauto not init yet!\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_ENOTINIT;
    }
    g_scene_state.pause = enable;
    ret = ot_scene_set_pause(enable);
    ot_mutex_unlock(g_scene_lock);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    return TD_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
