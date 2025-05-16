/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <signal.h>
#include "sample_comm.h"
#include "sample_ipc.h"
#include "sample_aibnr.h"
#include "sample_aidrc.h"
#include "sample_ai3dnr.h"

static td_void sample_aiisp_usage(const char *prg_name)
{
    printf("usage : %s <index> \n", prg_name);
    printf("index:\n");
    printf("    (0) aibnr line mode : base mode, has reference frame\n");
    printf("    (1) aibnr wdr mode  : wdr and long frame mode\n");
    printf("    (2) not support\n");
    printf("    (3) aibnr line mode and blend\n");
    printf("    (4) not support\n");
    printf("    (5) aibnr line switch to wdr\n");
    printf("    (6) aidrc normal mode\n");
    printf("    (7) not support\n");
    printf("    (8) not support\n");
    printf("    (9) ai3dnr uv net\n");
    printf("    (10) ai3dnr uv net + mcf: combine with ai3dnr uv net and mcf\n");
}

static td_void sample_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

static td_s32 sample_aiisp_msg_proc_vb_pool_share(td_s32 pid)
{
    td_s32 ret;
    td_u32 i;
    td_bool isp_states[OT_VI_MAX_PIPE_NUM];
#ifndef SAMPLE_MEM_SHARE_ENABLE
    ot_vb_common_pools_id pools_id = {0};

    if (ss_mpi_vb_get_common_pool_id(&pools_id) != TD_SUCCESS) {
        sample_print("get common pool_id failed!\n");
        return TD_FAILURE;
    }

    for (i = 0; i < pools_id.pool_cnt; ++i) {
        if (ss_mpi_vb_pool_share(pools_id.pool[i], pid) != TD_SUCCESS) {
            sample_print("vb pool share failed!\n");
            return TD_FAILURE;
        }
    }
#endif
    ret = sample_comm_vi_get_isp_run_state(isp_states, OT_VI_MAX_PIPE_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("get isp states fail\n");
        return TD_FAILURE;
    }

    for (i = 0; i < OT_VI_MAX_PIPE_NUM; i++) {
        if (!isp_states[i]) {
            continue;
        }
        ret = ss_mpi_isp_mem_share(i, pid);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_isp_mem_share vi_pipe %u, pid %d fail\n", i, pid);
        }
    }

    return TD_SUCCESS;
}

static td_void sample_aiisp_msg_proc_vb_pool_unshare(td_s32 pid)
{
    td_s32 ret;
    td_u32 i;
    td_bool isp_states[OT_VI_MAX_PIPE_NUM];
#ifndef SAMPLE_MEM_SHARE_ENABLE
    ot_vb_common_pools_id pools_id = {0};
    if (ss_mpi_vb_get_common_pool_id(&pools_id) == TD_SUCCESS) {
        for (i = 0; i < pools_id.pool_cnt; ++i) {
            ret = ss_mpi_vb_pool_unshare(pools_id.pool[i], pid);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_vb_pool_unshare vi_pipe %u, pid %d fail\n", pools_id.pool[i], pid);
            }
        }
    }
#endif
    ret = sample_comm_vi_get_isp_run_state(isp_states, OT_VI_MAX_PIPE_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("get isp states fail\n");
        return;
    }

    for (i = 0; i < OT_VI_MAX_PIPE_NUM; i++) {
        if (!isp_states[i]) {
            continue;
        }
        ret = ss_mpi_isp_mem_unshare(i, pid);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_isp_mem_unshare vi_pipe %u, pid %d fail\n", i, pid);
        }
    }
}

static td_s32 sample_aiisp_ipc_msg_proc(const sample_ipc_msg_req_buf *msg_req_buf,
    td_bool *is_need_fb, sample_ipc_msg_res_buf *msg_res_buf)
{
    td_s32 ret;

    if (msg_req_buf == TD_NULL || is_need_fb == TD_NULL) {
        return TD_FAILURE;
    }

    /* need feedback default */
    *is_need_fb = TD_TRUE;

    switch ((sample_msg_type)msg_req_buf->msg_type) {
        case SAMPLE_MSG_TYPE_VB_POOL_SHARE_REQ: {
            if (msg_res_buf == TD_NULL) {
                return TD_FAILURE;
            }
            ret = sample_aiisp_msg_proc_vb_pool_share(msg_req_buf->msg_data.pid);
            msg_res_buf->msg_type = SAMPLE_MSG_TYPE_VB_POOL_SHARE_RES;
            msg_res_buf->msg_data.is_req_success = (ret == TD_SUCCESS) ? TD_TRUE : TD_FALSE;
            break;
        }
        case SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_REQ: {
            if (msg_res_buf == TD_NULL) {
                return TD_FAILURE;
            }
            sample_aiisp_msg_proc_vb_pool_unshare(msg_req_buf->msg_data.pid);
            msg_res_buf->msg_type = SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_RES;
            msg_res_buf->msg_data.is_req_success = TD_TRUE;
            break;
        }
        default: {
            printf("unsupported msg type(%ld)!\n", msg_req_buf->msg_type);
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

static td_s32 sample_aiisp_execute_case(td_u32 case_index)
{
	RtspStart("config.xml", 554, 0, "video", "audio");
    td_s32 ret = TD_FAILURE;
    sample_aibnr_param aibnr_param = {0};
    sample_aidrc_param aidrc_param = {0};
    sample_ai3dnr_param ai3dnr_param = {0};

    switch (case_index) {
        case 0: /* 0 aibnr line mode */
            aibnr_param.is_wdr_mode = TD_FALSE;
            aibnr_param.ref_mode = OT_AIBNR_REF_MODE_NORM;
            aibnr_param.is_blend = TD_FALSE;
            ret = sample_aibnr(&aibnr_param);
            break;
        case 1: /* 1 aibnr wdr mode */
            aibnr_param.is_wdr_mode = TD_TRUE;
            aibnr_param.ref_mode = OT_AIBNR_REF_MODE_NORM;
            aibnr_param.is_blend = TD_FALSE;
            ret = sample_aibnr(&aibnr_param);
            break;
        case 2: /* 2 aibnr line mode, snap */
            printf("not support！\n");
            break;
        case 3: /* 3 aibnr line mode, blend */
            aibnr_param.is_wdr_mode = TD_FALSE;
            aibnr_param.ref_mode = OT_AIBNR_REF_MODE_NORM;
            aibnr_param.is_blend = TD_TRUE;
            ret = sample_aibnr(&aibnr_param);
            break;
        case 4: /* 4 aibnr snap + runonce */
            printf("not support！\n");
            break;
        case 5: /* 5 aibnr line switch to wdr */
            ret = sample_aibnr_line_switch_to_wdr();
            break;

        case 6: /* 6 aidrc normal mode */
            aidrc_param.is_wdr_mode = TD_TRUE;
            aidrc_param.mode = OT_AIDRC_MODE_NORMAL;
            ret = sample_aidrc(&aidrc_param);
            break;
        case 7: /* 7 aidrc advanced mode */
            printf("not support！\n");
            break;

        case 8: /* 8 ai3dnr yuv net */
            printf("not support！\n");
            break;
        case 9: /* 9 ai3dnr uv net */
            ai3dnr_param.net_type = OT_AI3DNR_NET_TYPE_UV;
            ret = sample_ai3dnr(&ai3dnr_param);
            break;
        case 10: /* 10 ai3dnr uv net + mcf */
            ret = sample_ai3dnr_and_mcf();
            break;
        default:
            break;
    }

    return ret;
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret = TD_FAILURE;
    td_u32 index;
    td_char *para_stop;

    if (argc != 2) { /* 2:arg num */
        sample_aiisp_usage(argv[0]);
        goto exit;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_aiisp_usage(argv[0]);
        goto exit;
    }

    if (strlen(argv[1]) > 2 || strlen(argv[1]) == 0 || !check_digit(argv[1][0]) || /* 2:arg len */
        (strlen(argv[1]) == 2 && (!check_digit(argv[1][1]) || argv[1][0] == '0'))) { /* 2:arg len */
        sample_aiisp_usage(argv[0]);
        goto exit;
    }

#ifndef __LITEOS__
    sample_register_sig_handler(sample_aiisp_handle_sig);
#endif

    if (sample_ipc_server_init(sample_aiisp_ipc_msg_proc) != TD_SUCCESS) {
        printf("sample_ipc_server_init failed!!!\n");
    }
    index = (td_u32)strtol(argv[1], &para_stop, 10); /* 1 idx , 10 dec */
    if (index > 10) { /* 10:arg len */
        sample_aiisp_usage(argv[0]);
        goto server_deinit;
    }

    ret = sample_aiisp_execute_case(index);
    if ((ret == TD_SUCCESS) && (sample_aiisp_get_sig() == 0)) {
        sample_print("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        sample_print("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

server_deinit:
    sample_ipc_server_deinit();

exit:
#ifdef __LITEOS__
    return ret;
#else
    exit(ret);
#endif
}