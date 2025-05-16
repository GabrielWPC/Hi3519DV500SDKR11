/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_SCENE_INNER_H
#define OT_SCENE_INNER_H
#include <pthread.h>
#include "ot_scene.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* For HDR Ver.1, HDRDarkRatio = 64, as noticed */
#define SCENE_HDR_DARKRATIO 64

typedef enum {
    SCENE_THREAD_TYPE_NORMAL = 0,
    SCENE_THREAD_TYPE_LUMINANCE,
    SCENE_THREAD_TYPE_NOTLINEAR,
    SCENE_THREAD_PIPE_TYPE_BUTT
} scene_thread_type;

typedef enum {
    SCENE_DYNAMIC_CHANGE_TYPE_EXPOSURE = 0,
    SCENE_DYNAMIC_CHANGE_TYPE_ISO,
    SCENE_DYNAMIC_PIPE_TYPE_BUTT
} scene_dynamic_change_type;


typedef struct {
    td_bool thread_flag;
    pthread_t thread;
} scene_thread;

/* mainIsp state */
typedef struct {
    td_handle main_pipe_hdl;
    td_u32 sub_pipe_num;
    td_handle sub_pipe_hdl[OT_SCENE_PIPE_MAX_NUM];

    td_bool metry_fixed;
    td_bool long_exp;

    td_u32 long_exp_iso;
    td_u64 last_normal_exposure;
    td_u32 last_normal_iso;
    td_u64 last_luminance_exposure;
    td_u32 last_luminance_iso;
    td_u64 last_not_linear_exposure;
    td_u32 last_not_linear_iso;
    td_u32 iso;
    td_u64 exposure;
    td_u32 wdr_ratio;
} scene_main_pipe_state;

/* scene state */
typedef struct {
    td_bool scene_init;
    td_bool support_photo_processing;
    td_bool pause;
    scene_thread thread_normal;
    scene_thread thread_luminance;
    scene_thread thread_not_linear;
    scene_thread thread_venc;
    pthread_attr_t thread_normal_attr;
    pthread_attr_t thread_luminance_attr;
    pthread_attr_t thread_not_linear_attr;
    pthread_attr_t thread_venc_attr;
    td_u32 main_pipe_num;
    scene_main_pipe_state main_pipe[OT_SCENE_PIPE_MAX_NUM];
} scene_state;

typedef td_s32 (*scene_set_dynamic_by_param_cb)(ot_vi_pipe vi_pipe, td_u64 param, td_u64 last_param, td_u8 index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* End of #ifndef OT_SCENE_INNER_H */
