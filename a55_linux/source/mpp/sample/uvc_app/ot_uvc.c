/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <pthread.h>
#include <unistd.h>
#include "log.h"
#include "uvc.h"
#include "camera.h"
#include "ot_uvc.h"

static pthread_t g_stuvc_send_data_pid;

static int uvc_init(void)
{
    return 0;
}

static int uvc_open(void)
{
    const char *devpath = "/dev/video0";

    return open_uvc_device(devpath);
}

static int uvc_close(void)
{
    return close_uvc_device();
}

static void *uvc_send_data_thread(void *p)
{
    int status = 0;
    const td_bool running = TD_TRUE;

    while (running) {
        if (sample_uvc_get_quit_flag() != 0) {
            return NULL;
        }
        status = run_uvc_data();
    }

    rlog("uvc_send_data_thread exit, status: %d.\n", status);

    return NULL;
}

static int uvc_run(void)
{
    const td_bool running = TD_TRUE;

    pthread_create(&g_stuvc_send_data_pid, 0, uvc_send_data_thread, NULL);

    while (running) {
        if (sample_uvc_get_quit_flag() != 0) {
            break;
        }

        int status = run_uvc_device();
        if (status < 0) {
            break;
        }
    }

    pthread_join(g_stuvc_send_data_pid, NULL);

    return 0;
}

/* ---------------------------------------------------------------------- */

static ot_uvc g_ot_uvc = {
    .init = uvc_init,
    .open = uvc_open,
    .close = uvc_close,
    .run = uvc_run,
};

ot_uvc *get_ot_uvc(void)
{
    return &g_ot_uvc;
}

void release_ot_uvc(ot_uvc *uvc) {}
