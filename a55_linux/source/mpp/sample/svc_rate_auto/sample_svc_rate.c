/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "ot_bitrate_auto.h"

#include "sample_comm.h"

#ifdef OT_FPGA
    #define PIC_SIZE   PIC_1080P
#else
    #define PIC_SIZE   PIC_1080P
#endif
static td_s32 g_sample_venc_exit = 0;

static td_void sample_rate_auto_print(rate_auto_param *param)
{
    printf("avbr_rate_control:%u\n", param->avbr_rate_control);
    printf("svc_fg_qpmap_val_p:%u %u %u %u %u\n", param->svc_fg_qpmap_val_p[FG_TYPE_0],
        param->svc_fg_qpmap_val_p[FG_TYPE_1], param->svc_fg_qpmap_val_p[FG_TYPE_2],
        param->svc_fg_qpmap_val_p[FG_TYPE_3], param->svc_fg_qpmap_val_p[FG_TYPE_4]);
    printf("svc_fg_qpmap_val_i:%u %u %u %u %u\n", param->svc_fg_qpmap_val_i[FG_TYPE_0],
        param->svc_fg_qpmap_val_i[FG_TYPE_1],
        param->svc_fg_qpmap_val_i[FG_TYPE_2],
        param->svc_fg_qpmap_val_i[FG_TYPE_3],
        param->svc_fg_qpmap_val_i[FG_TYPE_4]);
    printf("svc_fg_skipmap_val:%u %u %u %u %u\n", param->svc_fg_skipmap_val[FG_TYPE_0],
        param->svc_fg_skipmap_val[FG_TYPE_1],
        param->svc_fg_skipmap_val[FG_TYPE_2],
        param->svc_fg_skipmap_val[FG_TYPE_3],
        param->svc_fg_skipmap_val[FG_TYPE_4]);
    printf("svc_bg_qpmap_val_p:%u\n", param->svc_bg_qpmap_val_p);
    printf("svc_bg_qpmap_val_i:%u\n", param->svc_bg_qpmap_val_i);
    printf("svc_bg_skipmap_val:%u\n", param->svc_bg_skipmap_val);
    printf("svc_roi_qpmap_val_p:%u\n", param->svc_roi_qpmap_val_p);
    printf("svc_roi_qpmap_val_i:%u\n", param->svc_roi_qpmap_val_i);
    printf("svc_roi_skipmap_val:%u\n", param->svc_roi_skipmap_val);
    printf("max_bg_qp:%u\n", param->max_bg_qp);
    printf("min_fg_qp:%u\n", param->min_fg_qp);
    printf("max_fg_qp:%u\n", param->max_fg_qp);
    printf("fg_protect_adjust:%u\n", param->fg_protect_adjust);
}

/******************************************************************************
* function : show usage
******************************************************************************/
static td_void sample_rate_auto_usage(td_char *s_prg_nm)
{
    printf("Usage : %s <inidir>\n\t\tfor example :./sample_svc_rate param/config_rate_auto_base_param.ini\n",
        s_prg_nm);
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
    td_char *ini_file_name = NULL;
    rate_auto_param param;

    if (argc != 2) { /* 2:arg num */
        sample_rate_auto_usage(argv[0]);
        return TD_FAILURE;
    }
    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_rate_auto_usage(argv[0]);
        return TD_FAILURE;
    }

    ini_file_name = argv[1];

#ifndef __LITEOS__
    sample_sys_signal(sample_rate_auto_handle_sig);
#endif

    rate_auto_prt("rate auto load param\n");
    ret = ot_rate_auto_load_param(ini_file_name, &param);
    if (ret != TD_SUCCESS) {
        rate_auto_prt("ot_rate_auto_load_param fail,ret:0x%x\n", ret);
        return TD_FAILURE;
    }

    sample_rate_auto_print(&param);

    rate_auto_prt("rate auto start\n");
    ret = ot_rate_auto_init(&param);
    if (ret != TD_SUCCESS) {
        rate_auto_prt("ot_rate_auto_init fail, ret:0x%x\n", ret);
        goto EXIT;
    }
    rate_auto_prt("rate auto start success, press any key to exit.....\n");
    (td_void)getchar();

EXIT:
    rate_auto_prt("rate auto stop\n");
    ret = ot_rate_auto_deinit();
    if (ret != TD_SUCCESS) {
        rate_auto_prt("ot_rate_auto_deinit fail, ret:0x%x\n", ret);
    }
    return ret;
}

