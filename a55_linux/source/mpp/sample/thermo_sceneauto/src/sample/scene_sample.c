/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>
#include <securec.h>

#include "ot_scenecomm.h"
#include "scene_loadparam.h"
#include "ot_scene_setparam.h"
#include "sample_ipc.h"

ot_scene_param g_scene_param;

static td_bool g_scene_sample_exit = TD_FALSE;

#ifndef __LITEOS__
static td_void sample_scene_handle_signal(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_scene_sample_exit = TD_TRUE;
    }
}

static td_void sample_register_handler(td_void)
{
    struct sigaction action = { 0 };
    action.sa_flags = 0;
    action.sa_handler = sample_scene_handle_signal;
    sigaction(SIGINT, &action, TD_NULL);
    sigaction(SIGTERM, &action, TD_NULL);
}
#endif

static td_s32 sample_prompt_and_input_para(td_char *input, td_u32 input_len, td_s32 *choice)
{
    printf("1.scene start\n");
    printf("2.scene pause and reset media route\n");
    printf("3.scene resume and set a new media type\n");
    printf("4.exit the sample\n");

    *choice = -1;
    (td_void)fgets(input, input_len, stdin);
    td_s32 ret = sscanf_s(input, "%d", choice);
    if (ret != 1) {
        printf("choice error\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_create_para_and_set_mode(td_char *input, td_u32 input_len, const td_char *dir_name,
    ot_scene_video_mode *video_mode)
{
    printf("When We Start SceneAuto, we neen to set video Pipe mode\n");
    printf("Please input videomode index, without which we couldn't work  effectively.\n");
    td_s32 choice = -1;

    (td_void)fgets(input, input_len, stdin);
    td_s32 ret = sscanf_s(input, "%d", &choice);
    if (ret != 1) {
        printf("videomode error\n");
        return TD_FAILURE;
    }
    printf("videomode type has already been input.\n");

    ret = ot_scene_create_param(dir_name, &g_scene_param, video_mode);
    if (ret != TD_SUCCESS) {
        printf("SCENETOOL_CreateParam failed\n");
        return TD_FAILURE;
    }

    ret = ot_scene_init(&g_scene_param);
    if (ret != TD_SUCCESS) {
        printf("ot_scene_init failed\n");
        return TD_FAILURE;
    }

    ot_scenecomm_expr_true_return(choice >= SCENE_MAX_VIDEOMODE, TD_FAILURE);
    ret = ot_scene_set_scene_mode(&(video_mode->video_mode[choice]));
    if (ret != TD_SUCCESS) {
        printf("OT_SRDK_SCENEAUTO_Start failed\n");
        return TD_FAILURE;
    }
    printf("The sceneauto is started already.\n");
    return TD_SUCCESS;
}

static td_s32 scene_sample_scene_pause(td_void)
{
    td_s32 ret = ot_scene_pause(TD_TRUE);
    if (ret != TD_SUCCESS) {
        printf("ot_scene_pause failed\n");
        return TD_FAILURE;
    }
    printf("The sceneauto is pause.\n");
    printf("If you want to change videomode, please enter 3.\n");
    return TD_SUCCESS;
}

static td_s32 sample_scene_set_video_mode(td_char *input, td_u32 input_len, const ot_scene_video_mode *video_mode)
{
    printf("Please input videomode index, without which we couldn't work  effectively.\n");
    td_s32 choice = -1;
    (td_void)fgets(input, input_len, stdin);
    td_s32 ret = sscanf_s(input, "%d", &choice);
    if (ret != 1) {
        printf("videomode error\n");
        return TD_FAILURE;
    }
    printf("videomode type has already been input.\n");

    ot_scenecomm_expr_true_return(choice >= SCENE_MAX_VIDEOMODE, TD_FAILURE);
    ret = ot_scene_set_scene_mode(&(video_mode->video_mode[choice]));
    if (ret != TD_SUCCESS) {
        printf("OT_SRDK_SCENEAUTO_Start failed\n");
        return TD_FAILURE;
    }

    ret = ot_scene_pause(TD_FALSE);
    if (ret != TD_SUCCESS) {
        printf("OT_SCENE_Resume failed\n");
        return TD_FAILURE;
    }
    printf("The sceneauto is started already.\n");

    return TD_SUCCESS;
}

static td_void sample_scene_deinit(td_void)
{
    td_s32 ret = ot_scene_deinit();
    if (ret != TD_SUCCESS) {
        printf("ot_scene_deinit failed\n");
        return;
    }
    printf("The scene sample is end.\n");
}

static td_void sample_scene_notify_server_mem_share(td_bool is_share)
{
    sample_ipc_msg_req_buf req_buf = {0};
    sample_ipc_msg_res_buf res_buf = {0};

    if (is_share) {
        if (sample_ipc_client_init() != TD_SUCCESS) {
            printf("sample_ipc_client_init failed!!!\n");
            return;
        }
    }

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

    if (!is_share) {
        sample_ipc_client_deinit();
    }
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret, choice;
    td_char input[10]; /* 10 max input char */
    ot_scene_video_mode video_mode;

    ot_scenecomm_check_pointer_return(argv, TD_FAILURE);
    if (argc < 2 || strncmp(argv[1], "-h", 3) == 0) { /* need 2 args, compare max 3 bytes with -h */
        printf("Usage : %s <inidir>\n\t\tfor example :./sample_thermo_occ_fpn param/gst412c\n", argv[0]);
        return TD_SUCCESS;
    }

    set_dir_name(argv[1]);

#ifndef __LITEOS__
    sample_register_handler();
#endif
    sample_scene_notify_server_mem_share(TD_TRUE);
    printf("init success\n");
    
    while (g_scene_sample_exit == TD_FALSE) {
        ret = sample_prompt_and_input_para(input, sizeof(input), &choice);
        if (ret != TD_SUCCESS) {
            continue;
        }

        switch (choice) {
            case 1: /* user input 1 */
                ret = sample_create_para_and_set_mode(input, sizeof(input), argv[1], &video_mode);
                break;
            case 2: /* user input 2 */
                ret = scene_sample_scene_pause();
                break;
            case 3: /* user input 3 */
                ret = sample_scene_set_video_mode(input, sizeof(input), &video_mode);
                break;
            case 4: /* user input 4 */
                sample_scene_deinit();
                sleep(1);
                goto deinit_ipc;
            default:
                ret = TD_SUCCESS;
                printf("unknown input\n");
                break;
        }
        ot_scenecomm_check_return(ret, TD_FAILURE);
        
        sleep(1);
    }

    sample_scene_deinit();
    g_scene_sample_exit = TD_FALSE;
deinit_ipc:
    sample_scene_notify_server_mem_share(TD_FALSE);

    return TD_SUCCESS;
}
