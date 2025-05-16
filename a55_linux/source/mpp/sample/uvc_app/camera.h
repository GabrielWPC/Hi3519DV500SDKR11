/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <signal.h>

typedef struct ot_camera {
    int (*init)();
    int (*open)();
    int (*close)();
    int (*run)();
} ot_camera;

ot_camera *get_ot_camera(void);
void release_ot_camera(ot_camera *camera);

void sample_venc_config(void);
void sample_audio_config(void);

sig_atomic_t sample_uvc_get_quit_flag(void);

unsigned int get_g_uac_val(void);
#endif // __CAMERA_H__
