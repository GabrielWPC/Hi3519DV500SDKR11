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
#include <fcntl.h>
#include <sys/ioctl.h>

#include "securec.h"
#include "sample_svp_npu_process.h"

#define SAMPLE_SVP_NPU_ARG_MAX_NUM  2
#define SAMPLE_SVP_NPU_CMP_STR_NUM 2

static char **g_npu_cmd_argv = TD_NULL;

/*
 * function : to process abnormal case
 */
#ifndef __LITEOS__
static td_void sample_svp_npu_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        sample_svp_npu_acl_handle_sig();
    }
}
#endif

/*
 * function : show usage
 */
static td_void sample_svp_npu_usage(const td_char *prg_name)
{
    printf("Usage : %s <index>\n", prg_name);
    printf("index:\n");
    printf("\t 0) svp_acl_resnet50.\n");
    printf("\t 1) svp_acl_resnet50_multi_thread.\n");
    printf("\t 2) svp_acl_resnet50_dynamic_batch_with_mem_cached.\n");
    printf("\t 3) svp_acl_lstm.\n");
    printf("\t 4) svp_acl_event.\n");
    printf("\t 5) svp_acl_aipp.\n");
    printf("\t 6) svp_acl_preemption.\n");
}

static td_s32 sample_svp_npu_case(int argc, char *argv[])
{
    td_s32 ret = TD_SUCCESS;

    if (argc != SAMPLE_SVP_NPU_ARG_MAX_NUM) {
        return TD_FAILURE;
    }

    switch (*argv[1]) {
        case '0':
            sample_svp_npu_acl_resnet50();
            break;
        case '1':
            sample_svp_npu_acl_resnet50_multi_thread();
            break;
        case '2':
            sample_svp_npu_acl_resnet50_dynamic_batch();
            break;
        case '3':
            sample_svp_npu_acl_lstm();
            break;
        case '4':
            sample_svp_npu_acl_event();
            break;
        case '5':
            sample_svp_npu_acl_aipp();
            break;
        case '6':
            sample_svp_npu_acl_preemption();
            break;
        default:
            ret = TD_FAILURE;
            break;
    }
    return ret;
}

/*
 * function : svp npu sample
 */
#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    size_t idx_len;
#ifndef __LITEOS__
    struct sigaction sa;
#endif
    if (argc != SAMPLE_SVP_NPU_ARG_MAX_NUM) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", SAMPLE_SVP_NPU_CMP_STR_NUM)) {
        sample_svp_npu_usage(argv[0]);
        return TD_SUCCESS;
    }

    g_npu_cmd_argv = argv;
#ifndef __LITEOS__
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_svp_npu_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif

    idx_len = (td_s32)strlen(argv[1]);
    if (idx_len != 1) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    ret = sample_svp_npu_case(argc, argv);
    if (ret != TD_SUCCESS) {
        sample_svp_npu_usage(argv[0]);
        return TD_FAILURE;
    }

    return 0;
}
