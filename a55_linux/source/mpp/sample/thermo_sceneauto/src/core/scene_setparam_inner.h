/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SCENE_SETPARAM_INNER_H
#define SCENE_SETPARAM_INNER_H

#include "ot_scene_setparam.h"


#ifdef __cplusplus
extern "C" {
#endif


#define scene_div_0to1(a) (((a) == 0) ? 1 : (a))

ot_scene_pipe_param *get_pipe_params(td_void);
td_bool *get_isp_pause(td_void);

#define check_scene_ret(ret) do {                                                    \
        if ((ret) != TD_SUCCESS) {                                                   \
            printf("Failed at %s: LINE: %d with %#x!", __FUNCTION__, __LINE__, ret); \
        }                                                                            \
    } while (0)

#define check_scene_return_if_pause() do {   \
        if (*(get_isp_pause()) == TD_TRUE) { \
            return TD_SUCCESS;               \
        }                                    \
    } while (0)

td_u32 get_level_ltoh(td_u32 value, td_u32 level, td_u32 count, const td_u32 *thresh);
td_u32 scene_get_level_ltoh_u32(td_u32 value, td_u32 count, const td_u32 *thresh);
td_u32 scene_interpulate(td_u64 middle, td_u64 left, td_u64 left_value, td_u64 right, td_u64 right_value);
td_u32 scene_time_filter(td_u32 param0, td_u32 param1, td_u32 time_cnt, td_u32 index);
td_s32 scene_set_3dnr(ot_vi_pipe vi_pipe, const ot_scene_3dnr *_3dnr, td_u8 index, ot_3dnr_pos_type pos3Dnr);
td_void scene_set_3dnr_nrx_nry(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_iey(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_sfy(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_tfy(_3dnr_nrx_param param, ot_3dnr_pos_type pos3Dnr);
td_void scene_set_3dnr_nrx_mdy(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_nrc0(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_nrc1(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_adv_sfy(_3dnr_nrx_param param);
td_void scene_set_3dnr_nrx_adv_iey(_3dnr_nrx_param param);

#ifdef __cplusplus
}
#endif

#endif
