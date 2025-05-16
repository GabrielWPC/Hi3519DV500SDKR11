/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_COMMON_ISP_INNER_H
#define OT_COMMON_ISP_INNER_H

#include "ot_type.h"
#include "ot_common.h"
#include "ot_isp_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ISP_DRC_LMIX_NODE_NUM    33

typedef struct {
    ot_vi_pipe video_pipe;
    ot_vi_pipe pic_pipe;
} isp_snap_pipe;

typedef struct {
    td_u8 tone_mapping_wgt_x; /* RW; Range:[0x0, 0x80]; Local TM gain scaling coef */
    td_u8 detail_adjust_coef_x; /* RW; Range:[0x0, 0xF]; X filter detail adjust coefficient */
    td_u8 local_mixing_bright_x[ISP_DRC_LMIX_NODE_NUM]; /* RW; Range:[0x0, 0x80];
                                                               LUT of enhancement coefficients for positive details */
    td_u8 local_mixing_dark_x[ISP_DRC_LMIX_NODE_NUM]; /* RW; Range:[0x0, 0x80];
                                                             LUT of enhancement coefficients for positive details */
    td_u8 blend_luma_max; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_bright_min; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_bright_threshold; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_dark_min; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_dark_threshold; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_detail_max; /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_bright_min; /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_bright_threshold; /* RW; Range:[0x0, 0xF]; Detail-based filter blending weight control */
    td_u8 blend_detail_dark_min; /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_dark_threshold; /* RW; Range:[0x0, 0xF]; Detail-based filter blending weight control */
    td_u8 detail_adjust_coef_blend; /* RW; Range:[0x0, 0xF]; Extra detail gain of X filter details */
} isp_drc_blend;

typedef struct {
    td_u64 sum_y[OT_ISP_STRIPING_MAX_NUM];
    td_u64 pts;
} isp_check_sum_stat;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* OT_COMMON_ISP_INNER_H */
