/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "sample_comm.h"
#include "sample_ipc.h"
#define PIC_SIZE   PIC_1080P


#define BIG_STREAM_SIZE     PIC_1080P
#define SMALL_STREAM_SIZE   PIC_D1_NTSC
#define SAMPLE_CHN_NUM 2
#define SAMPLE_CHN_0    0
#define SAMPLE_CHN_1    1
#define SAMPLE_VPSS_0    0
#define SAMPLE_VPSS_1    1
static td_s32 g_sample_venc_exit = 0;


/******************************************************************************
* function : show usage
******************************************************************************/
static td_void sample_rate_auto_usage(td_char *s_prg_nm)
{
    printf("usage : %s [index] \n", s_prg_nm);
    printf("index:\n");
    printf("\t  0) rate_auto      : bitrate_auto sample.\n");
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
static td_void sample_rate_auto_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sample_venc_exit = 1;
    }
}
static td_void sample_rate_getchar()
{
    if (g_sample_venc_exit == 1) {
        sample_comm_sys_exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        exit(-1);
    }
    (td_void)getchar();

    if (g_sample_venc_exit == 1) {
        sample_comm_sys_exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        exit(-1);
    }
}

/******************************************************************************
* function :  H.265e@1080P@30fps + h264e@D1@30fps
******************************************************************************/
static td_s32 sample_rate_auto_normal(td_void)
{
    td_s32 ret;
    const ot_vpss_grp vpss_grp = 0;
    const td_s32 chn_num = SAMPLE_CHN_NUM;
    ot_venc_chn venc_chn[SAMPLE_CHN_NUM] = {SAMPLE_CHN_0, SAMPLE_CHN_1};
    ot_vpss_chn vpss_chn[SAMPLE_CHN_NUM] = {SAMPLE_VPSS_0, SAMPLE_VPSS_1};

    /******************************************
     stream save process
    ******************************************/
    ret = sample_comm_venc_rateauto_start(venc_chn, chn_num, vpss_grp, vpss_chn);
    if (ret != TD_SUCCESS) {
        sample_print("Start Venc failed!\n");
    }

    printf("please press ENTER to exit this sample\n");
    sample_rate_getchar();
    /******************************************
     exit process
    ******************************************/
    sample_comm_venc_stop_rateauto(venc_chn, chn_num);

    return ret;
}

static td_void sample_rate_notify_server_mem_share(td_bool is_share)
{
    sample_ipc_msg_req_buf req_buf = {0};
    sample_ipc_msg_res_buf res_buf = {0};

    if (is_share == TD_TRUE) {
        req_buf.msg_type = SAMPLE_MSG_TYPE_VB_POOL_SHARE_REQ;
    } else {
        req_buf.msg_type = SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_REQ;
    }
    req_buf.msg_data.pid = getpid();
    if (sample_ipc_client_send_data(&req_buf, TD_TRUE, &res_buf) != TD_SUCCESS) {
        printf("sample_ipc_client_send_data failed!!!\n");
    }
    if (res_buf.msg_data.is_req_success != TD_TRUE) {
        printf("sample request failed!!!\n");
    }
}

/******************************************************************************
* function    : main()
* description : video venc sample
******************************************************************************/
#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;
    td_u32 index;
    td_char *end_ptr = TD_NULL;

    if ((argc != 2) || !strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_rate_auto_usage(argv[0]);
        return TD_FAILURE;
    }
    if (strlen(argv[1]) != 1) {
        sample_rate_auto_usage(argv[0]);
        return TD_FAILURE;
    }
    if (argv[1][0] != '0') {
        sample_rate_auto_usage(argv[0]);
        return TD_FAILURE;
    }

    index = strtoul(argv[1], &end_ptr, 10); /* 10: base number */
    if (end_ptr == argv[1] || *end_ptr != '\0') {
        sample_rate_auto_usage(argv[1]);
        return TD_FAILURE;
    }
#ifndef __LITEOS__
    sample_sys_signal(sample_rate_auto_handle_sig);
#endif
    switch (index) {
        case 0: // 0 : case 0
            if (sample_ipc_client_init() != TD_SUCCESS) {
                printf("sample_ipc_client_init failed!!!\n");
            }
            sample_rate_notify_server_mem_share(TD_TRUE);
            ret = sample_rate_auto_normal();
            break;
        default:
            printf("the index is invalid!\n");
            sample_rate_auto_usage(argv[0]);
            return TD_FAILURE;
    }
    sample_rate_notify_server_mem_share(TD_FALSE);
    sample_ipc_client_deinit();

    if (ret == TD_SUCCESS) {
        printf("program exit normally!\n");
    } else {
        printf("program exit abnormally!\n");
    }
#ifdef __LITEOS__
    return ret;
#else
    exit(ret);
#endif
}
