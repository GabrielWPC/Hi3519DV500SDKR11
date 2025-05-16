/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SCENE_SETPARAM_INNER_H
#define SCENE_SETPARAM_INNER_H

#include "ot_scene_setparam.h"


#ifdef __cplusplus
extern "C" {
#endif

#define PIC_WIDTH_1080P 1920
#define PIC_HEIGHT_1080P 1080
#define LCU_ALIGN_H265 64
#define MB_ALIGN_H264 16

#define scene_div_0to1(a) (((a) == 0) ? 1 : (a))

#define SCENE_SHADING_TRIGMODE_L2H 1

#define SCENE_SHADING_TRIGMODE_H2L 2


ot_scene_pipe_param *get_pipe_params(td_void);
td_bool *get_isp_pause(td_void);

#define check_scene_return_if_pause() do {   \
        if (*(get_isp_pause()) == TD_TRUE) { \
            return TD_SUCCESS;               \
        }                                    \
    } while (0)

#define check_scene_ret(ret) do {                                                    \
        if ((ret) != TD_SUCCESS) {                                                   \
            printf("Failed at %s: LINE: %d with %#x!", __FUNCTION__, __LINE__, ret); \
        }                                                                            \
    } while (0)

td_u32 scene_get_level_ltoh(td_u64 value, td_u32 count, const td_u64 *thresh);
td_u32 scene_get_level_ltoh_u32(td_u32 value, td_u32 count, const td_u32 *thresh);
td_u32 scene_interpulate(td_u64 middle, td_u64 left, td_u64 left_value, td_u64 right, td_u64 right_value);
td_u32 scene_time_filter(td_u32 param0, td_u32 param1, td_u32 time_cnt, td_u32 index);
td_void scene_set_static_h265_avbr(ot_venc_rc_param *rc_param, td_u8 index);
td_void scene_set_static_h265_cvbr(ot_venc_rc_param *rc_param, td_u8 index);
td_void scene_set_isp_attr(set_isp_attr_param param, const td_u32 *ratio_level_thresh,
    const td_u32 *iso_level_thresh, ot_isp_drc_attr *isp_drc_attr);
td_void scene_set_aidrc_attr(set_isp_attr_param param, const td_u32 *ratio_level_thresh,
    const td_u32 *iso_level_thresh, ot_aidrc_attr *aidrc_attr);
td_s32 scene_blend_tone_mapping(ot_vi_pipe vi_pipe, td_u8 index,
    ot_scene_dynamic_drc_coef *drc_coef, ot_isp_drc_attr *isp_drc_attr);
td_s32 scene_set_tone_mapping_value(ot_vi_pipe vi_pipe, td_u8 index, td_u32 k,
    const ot_isp_inner_state_info *inner_state_info, ot_isp_drc_attr *isp_drc_attr);
td_s32 scene_set_isp_drc_attr(td_u8 index, td_u64 iso, ot_isp_drc_attr *isp_drc_attr);
td_s32 scene_set_nr_attr_para(td_u8 index, td_u32 iso, ot_isp_nr_attr *nr_attr, const ot_scene_nr_para *nr_para);
td_s32 scene_set_nr_wdr_ratio_para(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio, td_u32 ratio_index,
    ot_isp_nr_attr *nr_attr);
td_s32 scene_set_3dnr(ot_vi_pipe vi_pipe, const ot_scene_3dnr *_3dnr, td_u8 index, ot_3dnr_pos_type pos_3dnr);
td_void scene_set_3dnr_nrx_nry(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_iey(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_sfy(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_tfy(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_mdy(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_nrc0(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_nrc1(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_adv_sfy(const _3dnr_nrx_pack *pack);
td_void scene_set_3dnr_nrx_adv_iey(const _3dnr_nrx_pack *pack);

td_s32 scene_set_qp(td_u32 pic_width, td_u32 pic_height, td_u32 max_bitrate, ot_payload_type type,
    ot_venc_rc_param *rc_param);
td_s32 get_iso_mean_qp_chn_attr(ot_vi_pipe vi_pipe, td_u32 *iso, td_u32 *mean_qp, ot_venc_chn_attr *venc_chn_attr);
td_void calculate_manual_percent_index(td_u8 pipe_param_index, td_u32 sum_iso, td_u32 *out_index);
td_void set_initial_percent(ot_payload_type type, td_u32 index, ot_venc_rc_param *rc_param,
    const ot_scene_pipe_param *param, td_s32 *percent);
td_void set_min_qp_delta_when_iso_larger(ot_payload_type type, td_u32 sum_mean_qp, td_s32 percent,
    ot_venc_rc_param *rc_param);
td_void set_min_qp_delta_when_iso_less(ot_payload_type type, td_u32 sum_mean_qp, ot_venc_rc_param *rc_param,
    ot_venc_chn_attr *venc_chn_attr);


#ifdef __cplusplus
}
#endif

#endif
