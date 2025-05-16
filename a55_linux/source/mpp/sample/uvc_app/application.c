/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "camera.h"
#include "frame_cache.h"


#ifndef __LITEOS__
volatile sig_atomic_t g_need_quit_flag = 0;
sig_atomic_t sample_uvc_get_quit_flag(void)
{
    return g_need_quit_flag;
}
#endif

static void sample_uvc_usage(const char *s_prg_nm)
{
    printf("Usage : %s <param>\n", s_prg_nm);
    printf("param:\n");
    printf("\t -h        --for help.\n");
    printf("\t -bulkmode --use bulkmode.\n");
    return;
}

#ifndef __LITEOS__
static void sample_uvc_handle_signal(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_need_quit_flag = 1;
    }
}
#endif

int main(int argc, char *argv[])
{
    int i = argc;

    while (i > 1) {
        if (strcmp(argv[i - 1], "-h") == 0) {
            sample_uvc_usage(argv[0]);
        }

        i--;
    }

#ifndef __LITEOS__
    struct sigaction act = {0};
    act.sa_handler = sample_uvc_handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
#endif

    if (get_g_uac_val() == 1) {
        if (create_uac_cache() != 0) {
            goto err;
        }
    }

    if (get_ot_camera()->init() != 0 || get_ot_camera()->open() != 0 || get_ot_camera()->run() != 0) {
        goto err;
    }

    printf("UVC sample exit!\n");

err:
    get_ot_camera()->close();
    destroy_uvc_cache();
    if (get_g_uac_val() == 1) {
        destroy_uac_cache();
    }

    printf("exit app normally\n");
    return 0;
}
