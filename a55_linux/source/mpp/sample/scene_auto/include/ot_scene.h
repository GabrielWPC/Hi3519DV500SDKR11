/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_SCENE_H
#define OT_SCENE_H

#include "ot_scene_setparam.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* * \addtogroup     SCENE */
/* * @{ */ /* * <!-- [SCENE] */

/* macro define */
#define OT_SCENE_PIPE_MAX_NUM OT_VI_MAX_PIPE_NUM            /* *< pipe max count */
#define OT_SCENE_LONGEXP_AEROUTE_MAX_NUM 4 /* *< LongExp AeRoute Number */
#define OT_SCENE_HDRFRAME_MAX_COUNT 3      /* *< HDR max frame count */

/* error code define */
#define OT_SCENE_EINVAL OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 1)       /* Invalid argument */
#define OT_SCENE_ENOTINIT OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 2)     /* Not inited */
#define OT_SCENE_ENONPTR OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 3)      /* null pointer error */
#define OT_SCENE_EOUTOFRANGE OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 4)  /* out of range */
#define OT_SCENE_EINTER OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 5)       /* Internal error */
#define OT_SCENE_EINITIALIZED OT_SCENECOMM_ERR_ID(OT_SCENE_MOD_SCENE, 6) /* Already initialized */

typedef td_s32 (*ot_scene_set_static_param)(ot_vi_pipe vi_pipe, td_u8 index);

/* pipe type */
typedef enum {
    OT_SCENE_PIPE_TYPE_SNAP = 0,
    OT_SCENE_PIPE_TYPE_VIDEO,
    OT_SCENE_PIPE_TYPE_MCF_3DNR,
    OT_SCENE_PIPE_TYPE_BUTT
} ot_scene_pipe_type;

/* pipe attr */
typedef struct {
    td_bool enable;
    td_handle main_pipe_hdl;
    td_handle vcap_pipe_hdl;
    td_handle pipe_chn_hdl;
    td_handle venc_hdl;
    td_handle vpss_hdl;  /* vpss group hdl */
    td_handle vport_hdl; /* vpss chn hdl */
    td_u8 pipe_param_index;
    ot_scene_pipe_type pipe_type;
} ot_scene_pipe_attr;

typedef enum {
    OT_SCENE_PIPE_MODE_LINEAR = 0,
    OT_SCENE_PIPE_MODE_WDR,
    OT_SCENE_PIPE_MODE_HDR, /* HDR means , WDR existed at same time */
    OT_SCENE_PIPE_MODE_BUTT
} ot_scene_pipe_mode;

/* scene mode, an encapsulation of pipe attr array */
typedef struct {
    ot_scene_pipe_mode pipe_mode;
    ot_scene_pipe_attr pipe_attr[OT_SCENE_PIPE_MAX_NUM]; /* Pipe Attr */
} ot_scene_mode;

typedef struct {
    ot_scene_pipe_param pipe_param[OT_SCENE_PIPETYPE_NUM];
} ot_scene_param;

/* *
 * @brief     Init SCENE module.
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_init(const ot_scene_param *scene_param);
/* *
 * @brief     Deinit SCENE module.
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_deinit(td_void);

/* *
 * @brief     SetScenemode, start sceneauto thread.
 * @param[in] scene_mode : pipeCfg
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_set_scene_mode(const ot_scene_mode *scene_mode);

/* *
 * @brief GetSceneMode.
 * @param[out]  scene_mode : pipeCfg
 * @return   0 success, non-zero error code.
 * @exception    None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_get_scene_mode(ot_scene_mode *scene_mode);

/* *
 * @brief     Set fps control.
 * @param[in] scene_fps : scenefps config
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_set_scene_fps(const ot_scene_fps *scene_fps);

/* *
 * @brief Get fps control.
 * @param[out]  scene_fps : scenefps config
 * @return   0 success, non-zero error code.
 * @exception    None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_get_scene_fps(ot_scene_fps *scene_fps);

/* *
 * @brief     Set init iso and exposure.
 * @param[in] iso : iso
 * @param[in] exp : exposure
 * @return    0 success, non-zero error code.
 * @exception None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_set_scene_init_exp(td_s32 iso, td_s64 exp);

/* *
 * @brief Pause sceneauto when isp stopped(in playback state or when reset mediaRoute)
 * Note: when Pause is true, the sceneauto thread can's set param executed
 * @param[in]  enable: pause enable
 * @return   0 success, non-zero error code.
 * @exception    None
 * @author    MobileCam Reference Develop Team
 * @date      2017/12/11
 */
td_s32 ot_scene_pause(td_bool enable);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* End of #ifndef OT_SCENE_H */
