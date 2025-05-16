/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ot_uvc.h"
#include "ot_uac.h"
#include "ot_stream.h"
#include "ot_audio.h"
#include "ot_type.h"
#include "camera.h"
/* -------------------------------------------------------------------------- */

td_bool g_uac = TD_TRUE;
static pthread_t g_st_uvc_pid;
static pthread_t g_st_uac_pid;

static int camera_init(void)
{
    sample_venc_config();

    if (get_ot_uvc()->init() != 0 || ot_stream_init() != 0) {
        return -1;
    }

    if (g_uac) {
#ifdef OT_UAC_COMPILE
        sample_audio_config();
#endif

        if (get_ot_uac()->init() != 0 || ot_audio_init() != 0) {
            return -1;
        }
    }

    return 0;
}

static int camera_open(void)
{
#ifdef OT_UVC_COMPILE
    if (get_ot_uvc()->open() != 0) {
        return -1;
    }
#endif

    if (g_uac) {
        if (get_ot_uac()->open() != 0) {
            return -1;
        }
    }

    return 0;
}

static int camera_close(void)
{
#ifdef OT_UVC_COMPILE
    get_ot_uvc()->close();
#endif
    ot_stream_shutdown();
    if (g_uac) {
        get_ot_uac()->close();
        ot_audio_shutdown();
    }
    ot_stream_deinit();
    return 0;
}

static void *uvc_thread(void *p)
{
#ifdef OT_UVC_COMPILE
    get_ot_uvc()->run();
#endif
    return NULL;
}

static void *uac_thread(void *p)
{
    get_ot_uac()->run();
    return NULL;
}

static int camera_run(void)
{
    pthread_create(&g_st_uvc_pid, 0, uvc_thread, NULL);
    pthread_create(&g_st_uac_pid, 0, uac_thread, NULL);

#ifndef OT_UVC_COMPILE
    while (TD_TRUE) {
        if (sample_uvc_get_quit_flag() != 0) {
            break;
        }
        usleep(10 * 1000); /* 10 * 1000us sleep */
    }
#endif

    pthread_join(g_st_uvc_pid, NULL);
    pthread_join(g_st_uac_pid, NULL);
    return 0;
}

/* -------------------------------------------------------------------------- */

static ot_camera g_ot_camera = {
    .init = camera_init,
    .open = camera_open,
    .close = camera_close,
    .run = camera_run,
};

ot_camera *get_ot_camera(void)
{
    return &g_ot_camera;
}

void release_ot_camera(ot_camera *camera) {}

unsigned int get_g_uac_val(void)
{
    return g_uac;
}
