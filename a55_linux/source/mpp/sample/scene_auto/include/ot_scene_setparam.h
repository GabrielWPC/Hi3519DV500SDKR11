/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_SCENE_SETPARAM_H
#define OT_SCENE_SETPARAM_H

#include "ot_common.h"
#include "ot_common_isp.h"
#include "ot_common_vi.h"
#include "ot_common_vpss.h"
#include "ot_common_venc.h"
#include "ot_common_mcf.h"
#include "ss_mpi_aidrc.h"
#include "ss_mpi_ai3dnr.h"

#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(4)

#define OT_SCENE_PIPETYPE_NUM 4

#define OT_SCENE_FPS_ISO_MAX_COUNT 10
#define OT_SCENE_AE_EXPOSURE_MAX_COUNT 12
#define OT_SCENE_DEHAZE_LUT_NUM 256
#define OT_SCENE_ISO_STRENGTH_NUM 16
#define OT_SCENE_RATIO_STRENGTH_NUM 11
#define OT_SCENE_SHADING_EXPOSURE_MAX_COUNT 10
#define OT_SCENE_GAMMA_EXPOSURE_MAX_COUNT 10
#define OT_SCENE_DRC_ISO_MAX_COUNT 16
#define OT_SCENE_DRC_REF_RATIO_MAX_COUNT 12
#define OT_SCENE_DRC_RATIO_MAX_COUNT 12
#define OT_SCENE_REF_EXP_RATIO_FRM 16
#define OT_SCENE_NR_LUT_LENGTH 33
#define OT_SCENE_NR_LUT_LENGTH1 32
#define OT_SCENE_NR_RATIO_MAX_COUNT 16
#define OT_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT 10
#define OT_SCENE_DEMOSAIC_EXPOSURE_MAX_COUNT 8
#define OT_SCENE_DIS_ISO_MAX_COUNT 3
#define OT_SCENE_LDCI_EXPOSURE_MAX_COUNT 6
#define OT_SCENE_FSWDR_ISO_MAX_COUNT 3
#define OT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT 16
#define OT_SCENE_3DNR_MAX_COUNT 16
#define OT_SCENE_3DNR_IDX_LEN 17
#define ISP_AUTO_ISO_CA_NUM 16
#define ISP_SCENE_CA_MAX_BLEND_WEIGHT 256
#define ISP_SCENE_CA_SHIFT 8
#define ISP_AUTO_ISO_BLC_NUM 16
#define OT_SCENE_MCF_CC_UV_GAIN_LUT_NUM 256
#define OT_SCENE_MCF_WEIGHT_LUT_NUM  33
#define ISP_AUTO_ISO_DPC_NUM 16
#define FPN_GAIN_ISO 100
#define FPN_GAIN_SHIFT 20
#define FPN_CORRECTION_STR 256
#define DIR_NAME_BUFF 200
#define ISP_AUTO_RATIO_FSWDR_NUM 9
#define AIBNR_ISO_NUM 2
#define VENC_MODE_ISO_NUM 2
#define OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM      16
#define OT_SCENE_BSHP_CURVE_NUM    8
#define OT_SCENE_ACAC_THR_NUM             2

#undef  _tmprt1_5
#define _tmprt1_5(ps,    X)  ps[0].X, ps[1].X, ps[2].X, ps[3].X, ps[4].X

#undef  _tmprt1_4
#define _tmprt1_4(ps,    X)  ps[0].X, ps[1].X, ps[2].X, ps[3].X

#undef  _tmprt1y
#define _tmprt1y(ps,    X)  ps[1].X, ps[2].X
#undef  _tmprt1z
#define _tmprt1z(ps, a, b, X)  ps[a].X, ps[b].X

#undef  _tmprt2_3
#define _tmprt2_3(ps, X, Y)  ps[0].X, ps[0].Y,  ps[1].X, ps[1].Y,  ps[2].X, ps[2].Y

#undef  _tmprt2_4
#define _tmprt2_4(ps, X, Y)  ps[0].X, ps[0].Y,  ps[1].X, ps[1].Y,  ps[2].X, ps[2].Y,  ps[3].X, ps[3].Y

#undef  _tmprt2_4x
#define _tmprt2_4x(ps, X, Y) ps[1].X, ps[1].Y,  ps[2].X, ps[2].Y,  ps[3].X, ps[3].Y,  ps[4].X, ps[4].Y


#undef  _tmprt2_5
#define _tmprt2_5(ps, X, Y)  ps[0].X, ps[0].Y, ps[1].X, ps[1].Y,  ps[2].X, ps[2].Y, ps[3].X, ps[3].Y, ps[4].X, ps[4].Y

#undef  _tmprt2y
#define _tmprt2y(ps, X, Y)  ps[0].X, ps[0].Y,  ps[1].X, ps[1].Y,  ps[2].X, ps[2].Y

#undef  _tmprt2z
#define _tmprt2z(ps, a, b, X, Y)  ps[a].X, ps[a].Y,  ps[b].X, ps[b].Y

#undef  _t3_
#define _t3_(ie, n, X, Y, Z)            ie[n].X, ie[n].Y, ie[n].Z
#undef  _t4_
#define _t4_(ie, n, K, X, Y, Z)   ie[n].K, ie[n].X, ie[n].Y, ie[n].Z
#undef  _t5_
#define _t5_(ie, n, J, K, X, Y, Z)  ie[n].J, ie[n].K, ie[n].X, ie[n].Y, ie[n].Z

#undef  _t4_0_
#define _t4_0_(pc, X)   (pc)->X[0x00], (pc)->X[0x01], (pc)->X[0x02], (pc)->X[0x03]
#undef  _t4_4_
#define _t4_4_(pc, X)   (pc)->X[0x04], (pc)->X[0x05], (pc)->X[0x06], (pc)->X[0x07]
#undef  _t4_8_
#define _t4_8_(pc, X)   (pc)->X[0x08], (pc)->X[0x09], (pc)->X[0x0a], (pc)->X[0x0b]
#undef  _t4_12_
#define _t4_12_(pc, X)  (pc)->X[0x0c], (pc)->X[0x0d], (pc)->X[0x0e], (pc)->X[0x0f]

#undef  _t4a_0_
#define _t4a_0_(ps, a, X)   ps[a].X[0x00],  ps[a].X[0x01], ps[a].X[0x02], ps[a].X[0x03]
#undef  _t4a_4_
#define _t4a_4_(ps, a, X)   ps[a].X[0x04],  ps[a].X[0x05], ps[a].X[0x06], ps[a].X[0x07]
#undef  _t4a_8_
#define _t4a_8_(ps, a, X)   ps[a].X[0x08],  ps[a].X[0x09], ps[a].X[0x0a], ps[a].X[0x0b]
#undef  _t4a_12_
#define _t4a_12_(ps, a, X)   ps[a].X[0x0c],  ps[a].X[0x0d], ps[a].X[0x0e], ps[a].X[0x0f]

#undef  _t4a_10_
#define _t4a_10_(ps, a, X)   ps[a].X[0x10],  ps[a].X[0x11], ps[a].X[0x12], ps[a].X[0x13]
#undef  _t4a_14_
#define _t4a_14_(ps, a, X)   ps[a].X[0x14],  ps[a].X[0x15], ps[a].X[0x16], ps[a].X[0x17]
#undef  _t4a_18_
#define _t4a_18_(ps, a, X)   ps[a].X[0x18],  ps[a].X[0x19], ps[a].X[0x1a], ps[a].X[0x1b]
#undef  _t4a_1c_
#define _t4a_1c_(ps, a, X)   ps[a].X[0x1c],  ps[a].X[0x1d], ps[a].X[0x1e], ps[a].X[0x1f]


#undef  _tmprt3z
#define _tmprt3z(ps, a, b, X, Y, Z)  _t3_(ps, a, X, Y, Z), _t3_(ps, b, X, Y, Z)
#undef  _tmprt3y
#define _tmprt3y(ps, X, Y, Z)  _t3_(ps, 0, X, Y, Z), _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z)
#undef  _tmprt3x
#define _tmprt3x(ps, X, Y, Z)  _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z), _t3_(ps, 3, X, Y, Z)
#undef  _tmprt3_
#define _tmprt3_(ps, X, Y, Z)  _t3_(ps, 0, X, Y, Z), _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z), _t3_(ps, 3, X, Y, Z)

#undef  _tmprt3_5
#define _tmprt3_5(ps, X, Y, Z) \
    _t3_(ps, 0, X, Y, Z), _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z), _t3_(ps, 3, X, Y, Z), _t3_(ps, 4, X, Y, Z)

#undef  _tmprt3_4
#define _tmprt3_4(ps, X, Y, Z)  _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z), _t3_(ps, 3, X, Y, Z), _t3_(ps, 4, X, Y, Z)


#undef  _tmprt3x_
#define _tmprt3x_(ps, X, Y, Z) \
    _t3_(ps, 0, X, Y, Z), _t3_(ps, 1, X, Y, Z), _t3_(ps, 2, X, Y, Z), _t3_(ps, 3, X, Y, Z)

#undef  _tmprt4_5
#define _tmprt4_5(ps, K, X, Y, Z) \
    _t4_(ps, 0, K, X, Y, Z), _t4_(ps, 1, K, X, Y, Z), _t4_(ps, 2, K, X, Y, Z), \
    _t4_(ps, 3, K, X, Y, Z), _t4_(ps, 4, K, X, Y, Z)

#undef  _tmprt4_4
#define _tmprt4_4(ps, K, X, Y, Z) \
    _t4_(ps, 1, K, X, Y, Z), _t4_(ps, 2, K, X, Y, Z), _t4_(ps, 3, K, X, Y, Z), _t4_(ps, 4, K, X, Y, Z)

#undef  _tmprt4_2
#define _tmprt4_2(ps, K, X, Y, Z)  _t4_(ps, 1, K, X, Y, Z), _t4_(ps, 2, K, X, Y, Z)

#undef  _tmprt5_5
#define _tmprt5_5(ps, J, K, X, Y, Z) \
    _t5_(ps, 0, J, K, X, Y, Z), _t5_(ps, 1, J, K, X, Y, Z), \
    _t5_(ps, 2, J, K, X, Y, Z), _t5_(ps, 3, J, K, X, Y, Z), _t5_(ps, 4, J, K, X, Y, Z)

#undef  _tmprt5_3
#define _tmprt5_3(ps, J, K, X, Y, Z)  _t5_(ps, 0, J, K, X, Y, Z), _t5_(ps, 1, J, K, X, Y, Z), _t5_(ps, 2, J, K, X, Y, Z)

#undef  _tmprt4_3
#define _tmprt4_3(ps, K, X, Y, Z)  _t4_(ps, 0, K, X, Y, Z), _t4_(ps, 1, K, X, Y, Z), _t4_(ps, 2, K, X, Y, Z)

#undef  _tmprt4x_
#define _tmprt4x_(ps, K, X, Y, Z) \
    _t4_(ps, 0, K, X, Y, Z), _t4_(ps, 1, K, X, Y, Z), _t4_(ps, 2, K, X, Y, Z), _t4_(ps, 3, K, X, Y, Z)

#define scene_set_dynamic_linear_drc(target, member) do {                                                        \
        if ((iso_level == 0) || (iso_level == (iso_count - 1))) {                                                \
            target = g_scene_pipe_param[index].dynamic_linear_drc.member[iso_level];                             \
        } else {                                                                                                 \
            target = scene_interpulate(iso, iso_level_thresh[iso_level - 1],                                     \
                g_scene_pipe_param[index].dynamic_linear_drc.member[iso_level - 1], iso_level_thresh[iso_level], \
                g_scene_pipe_param[index].dynamic_linear_drc.member[iso_level]);                                 \
        }                                                                                                        \
    } while (0)

#define scene_set_dynamic_drc_member_piso(member, temp_member) do {                                       \
        if ((iso_level == 0) || (iso_level == (iso_count - 1))) {                                         \
            temp_member = g_scene_pipe_param[index].dynamic_drc.member[iso_level];                        \
        } else {                                                                                          \
            temp_member = scene_interpulate(iso, iso_level_thresh[iso_level - 1],                         \
                g_scene_pipe_param[index].dynamic_drc.member[iso_level - 1], iso_level_thresh[iso_level], \
                g_scene_pipe_param[index].dynamic_drc.member[iso_level]);                                 \
        }                                                                                                 \
    } while (0)

#define scene_set_dynamic_aidrc_member_piso(member, temp_member) do {                                       \
        if ((iso_level == 0) || (iso_level == (iso_count - 1))) {                                         \
            temp_member = g_scene_pipe_param[index].dynamic_aidrc.member[iso_level];                        \
        } else {                                                                                          \
            temp_member = scene_interpulate(iso, iso_level_thresh[iso_level - 1],                         \
                g_scene_pipe_param[index].dynamic_aidrc.member[iso_level - 1], iso_level_thresh[iso_level], \
                g_scene_pipe_param[index].dynamic_aidrc.member[iso_level]);                                 \
        }                                                                                                 \
    } while (0)

#define scene_set_dynamic_drc_member_pratio(member, temp_member) do {                                           \
        if ((ratio_level == 0) || (ratio_level == (ratio_count - 1))) {                                         \
            temp_member = g_scene_pipe_param[index].dynamic_drc.member[ratio_level];                            \
        } else {                                                                                                \
            temp_member = scene_interpulate(wdr_ratio, ratio_level_thresh[ratio_level - 1],                     \
                g_scene_pipe_param[index].dynamic_drc.member[ratio_level - 1], ratio_level_thresh[ratio_level], \
                g_scene_pipe_param[index].dynamic_drc.member[ratio_level]);                                     \
        }                                                                                                       \
    } while (0)

#define scene_set_dynamic_aidrc_member_pratio(member, temp_member) do {                                           \
        if ((ratio_level == 0) || (ratio_level == (ratio_count - 1))) {                                         \
            temp_member = g_scene_pipe_param[index].dynamic_aidrc.member[ratio_level];                            \
        } else {                                                                                                \
            temp_member = scene_interpulate(wdr_ratio, ratio_level_thresh[ratio_level - 1],                     \
                g_scene_pipe_param[index].dynamic_aidrc.member[ratio_level - 1], ratio_level_thresh[ratio_level], \
                g_scene_pipe_param[index].dynamic_aidrc.member[ratio_level]);                                     \
        }                                                                                                       \
    } while (0)

#define scene_set_drc_member(target, member, temp_member, flag) do {                                        \
        if (flag == 0) {                                                                                    \
            scene_set_dynamic_drc_member_pratio(member, temp_member);                                       \
        } else {                                                                                            \
            scene_set_dynamic_drc_member_piso(member, temp_member);                                         \
        }                                                                                                   \
        target = scene_time_filter(target, temp_member, g_scene_pipe_param[index].dynamic_drc.interval, i); \
    } while (0)

#define scene_set_aidrc_member(target, member, temp_member, flag) do {                                        \
        if (flag == 0) {                                                                                    \
            scene_set_dynamic_aidrc_member_pratio(member, temp_member);                                       \
        } else {                                                                                            \
            scene_set_dynamic_aidrc_member_piso(member, temp_member);                                         \
        }                                                                                                   \
        (target) = scene_time_filter((target), temp_member, g_scene_pipe_param[index].dynamic_aidrc.interval, i); \
    } while (0)


#define scene_set_dynamic_drc_return(target, member, flag) do {                       \
        void *temp_member;                                                            \
        temp_member = (typeof(target) *)malloc(sizeof(target));                       \
        if (temp_member == NULL) {                                                    \
            return;                                                                   \
        }                                                                             \
        scene_set_drc_member(target, member, *((typeof(target) *)temp_member), flag); \
        free(temp_member);                                                            \
    } while (0)

#define scene_set_dynamic_aidrc_return(target, member, flag) do {                       \
        void *temp_member;                                                            \
        temp_member = (typeof(target) *)malloc(sizeof(target));                       \
        if (temp_member == NULL) {                                                    \
            return;                                                                   \
        }                                                                             \
        scene_set_aidrc_member(target, member, *((typeof(target) *)temp_member), flag); \
        free(temp_member);                                                            \
    } while (0)

#define scene_set_nrx_attr_interpulate(pack, member) do {                        \
        (pack)->nrx_attr->member = scene_interpulate((pack)->mid, (pack)->left,        \
            (pack)->nrx_param->threednr_value[(pack)->iso_level - 1].member, (pack)->right,  \
            (pack)->nrx_param->threednr_value[(pack)->iso_level].member);                  \
    } while (0)

typedef struct {
    td_u8 index;
    td_u32 iso_level;
    td_u32 iso_count;
    td_u32 ratio_level;
    td_u32 ratio_count;
    td_u32 interval;
    td_u32 wdr_ratio;
    td_u32 iso;
} set_isp_attr_param;

typedef enum {
    OT_SCENE_OP_TYPE_AUTO = 0,
    OT_SCENE_OP_TYPE_MANUAL = 1,
    OT_SCENE_OP_TYPE_BUTT
} ot_scene_op_type;

typedef struct {
    td_u32 fps_max;
} ot_scene_fps_auto;

typedef struct {
    td_u32 fps;
} ot_scene_fps_manual;

typedef struct {
    td_bool enable;
    ot_scene_op_type op_type;
    ot_scene_fps_auto auto_s;
    ot_scene_fps_manual manual;
} ot_scene_fps;

typedef struct {
    td_bool debug;
    td_bool ae_weight_tab;
    td_bool static_ae;
    td_bool static_cac;
    td_bool static_local_cac;
    td_bool static_dpc;
    td_bool static_wdr_exposure;
    td_bool static_fswdr;
    td_bool static_awb;
    td_bool static_awbex;
    td_bool static_pipe_diff;
    td_bool static_drc;
    td_bool static_aidrc;
    td_bool static_dehaze;
    td_bool static_ldci;
    td_bool static_saturation;
    td_bool static_ccm;
    td_bool static_dm;
    td_bool static_nr;
    td_bool static_shading;
    td_bool static_csc;
    td_bool static_bayeshp;
    td_bool static_sharpen;
    td_bool static_clut;
    td_bool static_ca;
    td_bool static_cross_talk;
    td_bool static_3dnr;
    td_bool static_venc;
    td_bool static_pre_gamma;
    td_bool static_blc;

    td_bool dynamic_fps;
    td_bool dynamic_ae;
    td_bool dynamic_wdr_exposure;
    td_bool dynamic_iso_venc;
    td_bool dynamic_fswdr;
    td_bool dynamic_drc;
    td_bool dynamic_linear_drc;
    td_bool dynamic_aidrc;
    td_bool dynamic_dehaze;
    td_bool dynamic_ldci;
    td_bool dynamic_nr;
    td_bool dynamic_dpc;
    td_bool dynamic_blc;
    td_bool dynamic_linear_ca;
    td_bool dynamic_ca;
    td_bool dynamic_shading;
    td_bool dynamic_clut;
    td_bool dynamic_gamma;
    td_bool dynamic_false_color;
    td_bool dynamic_mcf;
    td_bool dynamic_3dnr;
    td_bool dynamic_ai3dnr;
    td_bool dynamic_aibnr;
    td_bool dynamic_fpn;
    td_bool dynamic_venc_mode;
} ot_scene_module_state;

typedef struct {
    td_bool ae_route_ex_valid;
    td_u8 ae_run_interval;
    td_u32 auto_sys_gain_max;
    td_u8 auto_speed;
    td_u8 auto_tolerance;
    td_u16 auto_black_delay_frame;
    td_u16 auto_white_delay_frame;
    td_u32 auto_exp_time_max;
} ot_scene_static_ae;

typedef struct {
    td_u32 total_num;
    td_u32 int_time[OT_ISP_AE_ROUTE_MAX_NODES];
    td_u32 sys_gain[OT_ISP_AE_ROUTE_MAX_NODES];
} ot_scene_static_ae_route;

typedef struct {
    td_u32 total_num;
    td_u32 int_time[OT_ISP_AE_ROUTE_MAX_NODES];
    td_u32 again[OT_ISP_AE_ROUTE_MAX_NODES];
    td_u32 dgain[OT_ISP_AE_ROUTE_MAX_NODES];
    td_u32 isp_dgain[OT_ISP_AE_ROUTE_MAX_NODES];
} ot_scene_static_ae_route_ex;

typedef struct {
    td_u8  op_type;
    td_u16 manual_rgain;
    td_u16 manual_gbgain;
    td_u16 manual_grgain;
    td_u16 manual_bgain;
    td_u16 auto_static_wb[OT_ISP_BAYER_CHN_NUM];
    td_s32 auto_curve_para[OT_ISP_AWB_CURVE_PARA_NUM];
    td_u16 auto_speed;
    td_u16 auto_low_color_temp;
    td_u16 auto_cr_max[OT_ISP_AUTO_ISO_NUM];
    td_u16 auto_cr_min[OT_ISP_AUTO_ISO_NUM];
    td_u16 auto_cb_max[OT_ISP_AUTO_ISO_NUM];
    td_u16 auto_cb_min[OT_ISP_AUTO_ISO_NUM];
    td_u16 luma_hist_enable;
    td_u16 awb_switch;
    td_u16 black_level;
} ot_scene_static_awb;

typedef struct {
    td_bool bypass;
    td_u8 tolerance;
    td_u8 out_shift_limit;
    td_u32 out_thresh;
    td_u16 low_stop;
    td_u16 high_start;
    td_u16 high_stop;
    td_bool multi_light_source_en;
    td_u16 multi_ctwt[OT_ISP_AWB_MULTI_CT_NUM];
} ot_scene_static_awb_ex;

typedef struct {
    td_u32 gain[OT_ISP_BAYER_CHN_NUM];
} ot_scene_isp_pipe_diff;

typedef struct {
    td_bool enable;                     /* RW; range: [0, 1]; format:1.0; cac enable */
    td_u8   op_type;                 /* RW; range: [0, 1]; format:1.0; purple fringing correction working mode */
    td_u8   detect_mode;                /* RW; range: [0, 2]; format:2.0; cac edge detect mode */
    td_s16  purple_upper_limit;         /* RW; range: [-2048,2047]; format:s12.0; cac purple high limitation */

    td_s16  purple_lower_limit;         /* RW; range: [-2048,2047]; format:s12.0; cac purple low limitation */
    td_u16  edge_threshold[OT_SCENE_ACAC_THR_NUM][OT_ISP_AUTO_ISO_NUM];
    td_u16  edge_gain[OT_ISP_AUTO_ISO_NUM];
    td_u16  cac_rb_strength[OT_ISP_AUTO_ISO_NUM];        /* RW; range: [0, 31]; format:5.0; cac strength on rb */
    td_u16  purple_alpha[OT_ISP_AUTO_ISO_NUM];           /* RW; range: [0, 63]; format:6.0; cac purple alpha */
    td_u16  edge_alpha[OT_ISP_AUTO_ISO_NUM];             /* RW; range: [0, 63]; format:6.0; cac edge alpha */
    td_u16  satu_low_threshold[OT_ISP_AUTO_ISO_NUM];     /* RW; range: [0, 4095]; format:12.0;
                                                         cac purple saturation low threshold */
    td_u16  satu_high_threshold[OT_ISP_AUTO_ISO_NUM];    /* RW; range: [0, 16383]; format:14.0;
                                                         cac purple saturation high threshold */

    td_u8   de_purple_cr_strength[OT_ISP_LCAC_EXP_RATIO_NUM]; /* RW; range: [0, 8]; format:4.0;
                                                         correction strength of the R channel */
    td_u8   de_purple_cb_strength[OT_ISP_LCAC_EXP_RATIO_NUM]; /* RW; range: [0, 8]; format:4.0; */
    td_u16  purple_detect_range;        /* RW; range: [0, 410]; format:9.0; purple fringing detection scope */
    td_u16  var_threshold;              /* RW; range: [0, 4095]; format:12.0; edge detection threshold */

    td_u16 r_detect_threshold[OT_ISP_LCAC_DET_NUM]; /* RW;Range:[0,4095];Format:12.0;Component R detection threshold */
    td_u16 g_detect_threshold[OT_ISP_LCAC_DET_NUM]; /* RW;Range:[0,4095];Format:12.0;Component G detection threshold */
    td_u16 b_detect_threshold[OT_ISP_LCAC_DET_NUM]; /* RW;Range:[0,4095];Format:12.0;Component B detection threshold */
} ot_scene_static_cac;

typedef struct {
    td_bool enable;
    ot_op_mode op_type[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u16 strength[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u16 blend_ratio[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_static_dpc;

typedef struct {
    td_u8 exp_ratio_type;
    td_u32 exp_ratio_max;
    td_u32 exp_ratio_min;
    td_u32 exp_ratio[OT_ISP_EXP_RATIO_NUM];
    td_u16 tolerance;
    td_u32 ref_ratio_up;
    td_u32 ref_ratio_dn;
    td_u32 exp_thr;
    td_u32 high_light_target;
    td_u32 exp_coef_min;
} ot_scene_static_wdr_exposure;

typedef struct {
    td_u32 iso_thresh_cnt;
    td_u32 iso_thresh_ltoh[OT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
    td_u16 manual_percent[OT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_venc_bitrate;

typedef struct {
    td_u8 ae_exposure_cnt;
    td_u64 exp_ltoh_thresh[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u64 exp_htol_thresh[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u32 exp_compensation[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u32 max_hist_offset[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_wdr_exposure;

typedef struct {
    ot_isp_wdr_merge_mode wdr_merge_mode;
    td_bool fusion_blend_en;
} ot_scene_static_fswdr;

typedef struct {
    td_bool enable;
    td_u8 ldci_op_type;
    td_u8 gauss_lpf_sigma;
    td_u16 tpr_incr_coef;
    td_u16 tpr_decr_coef;
    td_u8 auto_he_pos_wgt[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_he_pos_sigma[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_he_pos_mean[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_he_neg_wgt[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_he_neg_sigma[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_he_neg_mean[OT_ISP_AUTO_ISO_NUM];
    td_u8 auto_blc_ctrl[OT_ISP_AUTO_ISO_NUM];
} ot_scene_static_ldci;

typedef struct {
    td_bool enable;
    td_u8 dehaze_op_type;
    td_bool user_lut_enable;
    td_u16 tmprflt_incr_coef;
    td_u16 tmprflt_decr_coef;
    td_u8 dehaze_lut[OT_ISP_DEHAZE_LUT_SIZE];
} ot_scene_static_dehaze;

typedef struct {
    td_bool enable;
    ot_op_mode op_type;
    ot_isp_drc_curve_select curve_select; /* RW; Range:[0x0, 0x1]; Tone mapping curve selection */
    td_u8 purple_reduction_strength;      /* RW; Range:[0x0, 0x80]; Purple detection and reduction strength */
    td_u8 contrast_ctrl;  /* RW; Range:[0x0, 0xF]; Format:4.0; Contrast control, smaller value yields better contrast */
    td_u8 blend_luma_max; /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_bright_min;         /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_bright_threshold;   /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */

    td_u8 blend_luma_dark_min;           /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */
    td_u8 blend_luma_dark_threshold;     /* RW; Range:[0x0, 0xFF]; Luma-based filter blending weight control */

    td_u8 blend_detail_max;              /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_bright_min;       /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_bright_threshold; /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */

    td_u8 blend_detail_dark_min;         /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */
    td_u8 blend_detail_dark_threshold;   /* RW; Range:[0x0, 0xFF]; Detail-based filter blending weight control */

    td_u8  global_color_ctrl;
    td_u16 color_correction_lut[OT_ISP_DRC_CC_NODE_NUM];
} ot_scene_static_drc;

typedef struct {
    td_u8 tone_mapping_wgt_x; /* RW; Range:[0x0, 0x80]; Local TM gain scaling coef */
    td_u8 detail_adjust_coef_x; /* RW; Range:[0x0, 0xF]; X filter detail adjust coefficient */
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
} ot_scene_static_aidrc;

typedef struct {
    td_u8 fps_exposure_cnt;
    td_u64 exp_ltoh_thresh[OT_SCENE_FPS_ISO_MAX_COUNT];
    td_u64 exp_htol_thresh[OT_SCENE_FPS_ISO_MAX_COUNT];
    td_u32 fps[OT_SCENE_FPS_ISO_MAX_COUNT];
    td_u32 venc_gop[OT_SCENE_FPS_ISO_MAX_COUNT];
    td_u32 ae_max_time[OT_SCENE_FPS_ISO_MAX_COUNT];
} ot_scene_dynamic_fps;

typedef struct {
    td_u8 ae_exposure_cnt;
    td_u32 auto_exp_ratio_max[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u32 auto_exp_ratio_min[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u64 exp_ltoh_thresh[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u64 exp_htol_thresh[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u8 auto_compensation[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u8 auto_max_hist_offset[OT_SCENE_AE_EXPOSURE_MAX_COUNT];
    td_u32  wdr_ratio_threshold;
    td_bool l_advance_ae;
    td_bool h_advance_ae;
} ot_scene_dynamic_ae;

typedef struct {
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_bool enable;

    td_u8 local_mixing_bright[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 local_mixing_dark[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 spatial_filter_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 range_filter_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 detail_adjust_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 rim_reduction_strength[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 rim_reduction_threshold[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 bright_gain_limit[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 bright_gain_limit_step[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 dark_gain_limit_luma[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 dark_gain_limit_chroma[OT_SCENE_DRC_ISO_MAX_COUNT];

    td_u8 high_saturation_color_ctrl[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 asymmetry[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 second_pole[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 compress[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 stretch[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u16 strength[OT_SCENE_DRC_ISO_MAX_COUNT];
} ot_scene_dynamic_linear_drc;

typedef struct {
    td_bool enable;
    td_u32 ratio_count;
    td_u32 ratio_level[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u32 interval;

    td_u8 high_saturation_color_ctrl[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 bright_gain_limit[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 bright_gain_limit_step[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u16 drc_str_incre[OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u8 local_mixing_bright[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 local_mixing_dark[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u16 color_correction_lut[OT_ISP_DRC_CC_NODE_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u8 tone_mapping_wgt_x[OT_SCENE_DRC_ISO_MAX_COUNT];

    td_u8 spatial_filter_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 range_filter_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 detail_adjust_coef[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 rim_reduction_strength[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 rim_reduction_threshold[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 dark_gain_limit_luma[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 dark_gain_limit_chroma[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 global_color_ctrl[OT_SCENE_DRC_RATIO_MAX_COUNT]; /* global color control  for CSP curve */
    td_bool shoot_reduction_en[OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u16 manual_str[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u32 ref_ratio_count;
    td_u32 ref_ratio_ltoh[OT_SCENE_DRC_REF_RATIO_MAX_COUNT];
    td_u32 ref_ratio_alpha[OT_SCENE_DRC_REF_RATIO_MAX_COUNT];
    td_u32 tm_ratio_threshold;
    td_u16 tm_value_low[OT_ISP_DRC_TM_NODE_NUM];
    td_u16 tm_value_high[OT_ISP_DRC_TM_NODE_NUM];
    td_u16 tm_val_higher[OT_ISP_DRC_TM_NODE_NUM];
    td_u16 tm_val_highest[OT_ISP_DRC_TM_NODE_NUM];
} ot_scene_dynamic_drc;

typedef struct {
    td_bool enable;
    td_u32 ratio_count;
    td_u32 ratio_level[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u32 interval;

    td_u8 blend_luma_max[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 detail_adjust_coef_x[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 detail_adjust_coef_blend[OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u32 aidrc_strength[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u32 aidrc_threshold[OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u8 local_mixing_bright_x[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
    td_u8 local_mixing_dark_x[OT_ISP_DRC_LMIX_NODE_NUM][OT_SCENE_DRC_ISO_MAX_COUNT];
} ot_scene_dynamic_aidrc;

typedef struct {
    td_u32 ref_exp_ratio;
    td_u32 ref_ratio_alpha;
    td_u32 k;
} ot_scene_dynamic_drc_coef;

typedef struct {
    td_u32 enable_cnt;
    td_u8 enable[OT_SCENE_LDCI_EXPOSURE_MAX_COUNT];
    td_u64 enable_exp_thresh_ltoh[OT_SCENE_LDCI_EXPOSURE_MAX_COUNT];
    td_u32 exp_thresh_cnt;
    td_u8 manual_ldci_he_pos_wgt[OT_SCENE_LDCI_EXPOSURE_MAX_COUNT];
    td_u64 exp_thresh_ltoh[OT_SCENE_LDCI_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_ldci;

#define SDK_RELY
#define S_IDX_LEN   17

typedef struct {
    /* ies0~3 for different frequency response. */
    td_u32 ies0;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies1;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies2;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies3;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 iedz;    /* RW; Range: [0, 999]; The threshold to control the generated artifacts. */
    td_u32 o_sht;    /* RW; Range: [0, 255]; The gains to control the overshoot. */
    td_u32 u_sht;    /* RW; Range: [0, 255]; The gains to control the undershoot. */
} t_v200_vpss_iey;

typedef struct {
    td_u32 spn;  /* RW; Range: [0,   6]; The selection of filters to be mixed for NO.7 filter. */
    td_u32 sbn;  /* RW; Range: [0,   6]; The selection of filters to be mixed for NO.7 filter. */
    td_u32 pbr;  /* RW; Range: [0,  16]; The mix ratio between spn and sbr. */
    td_u32 j_mode;  /* RW; Range: [0,   2]; The selection modes for the blending of spatial filters. */
    td_u32 _rb_;  /* RW; Reserved */

    td_u32 sfr6[4];     /* RW; Range: [0, 31]; The relative NR strength for NO.6 filter. (Effective when jmode = 2). */
    td_u32 sbr6[2];     /* RW; Range: [0, 15]; The control of overshoot and undershoot. */

    td_u32 sfs1;        /* RW; Range: [0, 255]; The NR strength parameters for NO.1 filter. */
    td_u32 sbr1;        /* RW; Range: [0, 255]; The NR strength parameters for NO.1 filter. */
    td_u32 sfs2;        /* RW; Range: [0, 255]; The NR strength parameters for NO.2 filter. */
    td_u32 sft2;        /* RW; Range: [0, 255]; The NR strength parameters for NO.2 filter. */
    td_u32 sbr2;        /* RW; Range: [0, 255]; The NR strength parameters for NO.2 filter. */
    td_u32 sfs4;        /* RW; Range: [0, 255]; The NR strength parameters for NO.3 and NO.4 filters. */
    td_u32 sft4;        /* RW; Range: [0, 255]; The NR strength parameters for NO.3 and NO.4 filters. */
    td_u32 sbr4;        /* RW; Range: [0, 255]; The NR strength parameters for NO.3 and NO.4 filters. */
    /*
    * sth1_0, sth2_0, sth3_0; Range: [0, 511]; The thresholds for protection of edges on foreground from blurring.
    * sth1_1, sth2_1, sth3_1; Range: [0, 511]; The thresholds for protection of edges on background from blurring.
    * bri_idx0, bri_idx1; Range: [0, 7]; The filter selection based on brightness
    * for foreground and background, respectively.
    */
    td_u32 sth1_0;
    td_u32 sth2_0;
    td_u32 sth3_0;
    td_u32 sth1_1;
    td_u32 sth2_1;
    td_u32 sth3_1;

    /*
    * sfn0_0~sfn3_0; Range: [0, 6]; Filter selection for different foreground image areas based on sth1_0~sth3_0.
    * sfn0_1~sfn3_1; Range: [0, 6]; Filter selection for different background image areas based on sth1_1~sth3_1.
    */

    td_u32 sfn0_0;
    td_u32 sfn1_0;
    td_u32 sfn2_0;
    td_u32 sfn3_0;

    td_u32 sfn0_1;
    td_u32 sfn1_1;
    td_u32 sfn2_1;
    td_u32 sfn3_1;

    td_u32 bri_str[OT_NR_S_IDX_LEN]; /* RW ; Range: [0, 255];  Spatial NR strength based on brightness. */

    td_u32 bri_idx0;     /* RW; Range: [0, 6]; The filter selection based on brightness */
    td_u32 bri_idx1;     /* RW; Range: [0, 6]; The filter selection based on brightness */
    td_u32 reserved1;
    td_u32 k_mode;
    td_u32 sbs_k[33];
    td_u32 sds_k[33];
    td_u32 reserved2;
} t_v200_vpss_sfy;

typedef struct {
    /* ies0~3 for different frequency response. */
    td_u32 ies0;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies1;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies2;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 ies3;     /* RW; Range: [0, 255]; The gains of edge and texture enhancement. */
    td_u32 aux_ies;  /* RW; Range: [0, 255]; The gains of edge enhancement for NO.2 filter. */

    td_u32 iedz;    /* RW; Range: [0, 999]; The threshold to control the generated artifacts. */
    td_u32 _rb_;

    td_u32 o_sht;       /* RW; Range: [0, 255]; The gains to control the overshoot. */
    td_u32 u_sht;       /* RW; Range: [0, 255]; The gains to control the undershoot. */
    td_u32 aux_o_sht;   /* RW; Range: [0, 255]; The gains to control the overshoot. */
    td_u32 aux_u_sht;   /* RW; Range: [0, 255]; The gains to control the undershoot. */
} t_v200_vpss_adv_iey;

typedef struct {
    td_u32 spn6;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.6 filter. */
    td_u32 sbn6;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.6 filter. */
    td_u32 pbr6;  /* RW; Range: [0,  16]; The mix ratio between spn and sbr. */
    td_u32 j_mode6;  /* RW; Range: [0,   2]; The selection modes for the blending of spatial filters. */
    td_u32 _rb_;  /* RW; Reserved */

    td_u32 sfr6[4];     /* RW; Range: [0, 31]; The relative NR strength for NO.6 filter. (Effective when jmode = 2). */
    td_u32 sbr6[2];     /* RW; Range: [0, 15]; The control of overshoot and undershoot. */
    td_u32 sfs1;        /* RW; Range: [0, 15]; The NR strength parameters for NO.1 filter. */
    td_u32 sfs2;        /* RW; Range: [0, 255]; Currently no use, reserved. */
    td_u32 bld1;       /* RW; Range: [0, 999];  The NR strength parameters for NO.1 filter. */

    td_u32 spn3;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.3 filter. */
    td_u32 sbn3;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.3 filter. */
    td_u32 pbr3;  /* RW; Range: [0,  16]; The mix ratio between spn and sbr. */
    td_u32 _rb0_;

    td_u32 spn4;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.4 filter. */
    td_u32 sbn4;  /* RW; Range: [0,   5]; The selection of filters to be mixed for NO.4 filter. */
    td_u32 pbr4;  /* RW; Range: [0,  16]; The mix ratio between spn and sbr. */
    td_u32 _rb1_;

    td_u32 sfs5;           /* RW; Range: [0, 255]; The NR strength parameters for NO.5 filters. */
    td_u32 sft5;           /* RW; Range: [0, 255]; The NR strength parameters for NO.5 filters. */
    td_u32 sbr5;           /* RW; Range: [0, 255]; The NR strength parameters for NO.5 filters. */
    td_u32 reserved2;

    td_u32  sth1;        /* RW; Range: [0, 511]; The thresholds for protection of edges from blurring. */
    td_u32  sth2;        /* RW; Range: [0, 511]; The thresholds for protection of edges from blurring. */
    td_u32  sth3;        /* RW; Range: [0, 511]; The thresholds for protection of edges from blurring. */

    td_u32 sfn0; /* RW; Range: [0, 6]; Filter selection for different foreground image areas based on sth1. */
    td_u32 sfn1; /* RW; Range: [0, 6]; Filter selection for different foreground image areas based on sth1. */
    td_u32 sfn2; /* RW; Range: [0, 6]; Filter selection for different foreground image areas based on sth2. */
    td_u32 sfn3; /* RW; Range: [0, 6]; Filter selection for different foreground image areas based on sth3. */

    td_u32   bri_str[OT_NR_S_IDX_LEN];   /* RW; Range: [0, 255];  Spatial NR strength based on brightness. */
    td_u32 bri_idx;                       /* RW; Range: [0, 6]; The filter selection based on brightness */

    td_u32 k_mode;
    td_u32 sbs_k[33];
    td_u32 sds_k[33];
    td_u32 reserved3;
} t_v200_vpss_adv_sfy;

typedef struct {
    td_u32 tfs0;   /* RW; Range: [0,  15]; The NR strength for temporal filtering. */
    td_u32 tdz0;  /* RW; Range: [0, 999]; Protection of the weak texture area from temporal filtering. */
    td_u32 ref_en;   /* RW; Range: [0,  1];  The switch the reference frame. */
    td_u32 _rb0_;

    td_u32 tfs1;   /* RW; Range: [0,  15]; The NR strength for temporal filtering. */
    td_u32 tdz1;  /* RW; Range: [0, 999]; Protection of the weak texture area from temporal filtering. */
    td_u32 math_mode;   /* RW; Range: [0,   1]; The motion detection mode. */
    td_u32 _rb1_;

    td_u32   tss0;       /* RW; Range: [0,  15]; The ratio for blending spatial NR with the temporal NR results. */
    td_u32   tss1;       /* RW; Range: [0,  15]; The ratio for blending spatial NR with the temporal NR results. */

    td_u32 auto_math;   /* RW; Range: [0, 999]; The motion threshold for the Level 0 denoise. */
    td_u32 tfr0[6];      /* RW; Range: [0,  31]; The temporal NR strength control for background (static) area. */
    td_u32 tfr1[6];      /* RW; Range: [0,  31]; The temporal NR strength control for background (static) area. */
} t_v200_vpss_tfy;

typedef struct {
    td_u32 math0;  /* RW; Range: [0, 999]; The threshold for motion detection. */
    td_u32 mate0;   /* RW; Range: [0,   8]; The motion index for smooth image area. */
    td_u32 adv_math; /* RW; Range: [0,   2]; The mode selection for motion detection. */
    td_u32 math1;  /* RW; Range: [0, 999]; The threshold for motion detection. */
    td_u32 mate1;   /* RW; Range: [0,   8]; The motion index for smooth image area. */
    td_u32 _rb1_;
    td_u32   mabw0;          /* RW; Range: [5,   9]; The window size for motion detection. */
    td_u32   mabw1;          /* RW; Range: [5,   9]; The window size for motion detection. */

    td_u32 artc;           /* RW; Range: [0, 255]; The artifact control for motion */
    td_u32 mcth;           /* RW; Range: [0, 999]; The sensitivity for motion */
    td_u32 dci_w;          /* RW; Range: [0, 999]; The temporal gain for dci gain */
} t_v200_vpss_mdy;

typedef struct {
    td_u32 sfc_enhance;     /* RW; Range: [0, 255];  The chroma NR strength for sfc. */
    td_u32 sfc_ext;         /* RW; Range: [0, 255];  The extra chroma NR strength for sfc. */
    td_u32 trc;             /* RW; Range: [0, 255];  The temporal chroma NR strength. */
    td_u32 reserved;
    td_u32 sfc;   /* RW; Range: [0, 255];  The motion threshold for chroma. */
    td_u32 tfc;   /* RW; Range: [0, 31];   The control of temporal chroma NR strength. */
    td_u32 _rb0_;

    td_u32 tpc;              /* RW; Range: [0, 31];  Chroma denoise mode. */
    td_u32 pre_sfc;          /* RW; Range: [0, 31];  pre-chroma denoise. */
} t_v200_vpss_nrc0;

typedef struct {
    td_u32 sfs2;      /* RW; Range: [0, 255];  The NR strength parameters for NO.2 filter. */
    td_u32 sft2;      /* RW; Range: [0, 255];  The NR strength parameters for NO.2 filter. */
    td_u32 sbr2;      /* RW; Range: [0, 255];  The NR strength parameters for NO.2 filter. */
    td_u32 sfc3;      /* RW; Range: [0, 255];  The chroma NR strength for NO.3 filter. */
    td_u32 sfc_ext3;  /* RW; Range: [0, 255];  The extra chroma NR strength for NO.3 filter. */
    /* sfr3_u, sfr3_v; Range: [0, 15]; The chroma NR strength control for U and V, respectively, for NO.3 filter. */

    td_u32 sfr3_u;
    td_u32 sfr3_v;

    td_u32 spn0;    /* RW; Range: [0,  3];  The selection of filters to be mixed for NO.X filter. */
    td_u32 sbn0;    /* RW; Range: [0,  3];  The selection of filters to be mixed for NO.X filter. */
    td_u32 pbr0;    /* RW; Range: [0, 15];  The mix ratio for NO.7 filter. */
    td_u32 spn1;    /* RW; Range: [0,  3];  The selection of filters to be mixed for NO.X filter. */
    td_u32 sbn1;    /* RW; Range: [0,  3];  The selection of filters to be mixed for NO.X filter. */
    td_u32 pbr1;    /* RW; Range: [0,  15]; The mix ratio for NO.X filter. */
} t_v200_vpss_nrc1;

typedef struct {
    t_v200_vpss_adv_iey adv_ie_y[2];
    t_v200_vpss_adv_sfy adv_sf_y[2];

    t_v200_vpss_iey  iey[5];
    t_v200_vpss_sfy  sfy[5];
    t_v200_vpss_mdy  mdy[2];
    t_v200_vpss_tfy  tfy[3];
    t_v200_vpss_nrc0 nrc0;
    t_v200_vpss_nrc1 nrc1;

    td_u32 limit_range_en;  /* RW; Range: [0, 1]; The switch for limit range mode. */
    td_u32 nry0_en;  /* RW; Range: [0, 1]; The switch for luma denoise. */
    td_u32 nry1_en;  /* RW; Range: [0, 1]; The switch for luma denoise. */
    td_u32 nry2_en;  /* RW; Range: [0, 1]; The switch for luma denoise. */
    td_u32 nry3_en;  /* RW; Range: [0, 1]; The switch for luma denoise. */
    td_u32 nrc0_en;  /* RW; Range: [0, 1]; The switch for chroma denoise. */
    td_u32 nrc1_en;  /* RW; Range: [0, 1]; The switch for chroma denoise. */
    td_u32 _rb_;

    td_u32 adv_mode;                /* RW; Range: [0, 3]; */
} ot_scene_3dnr;


typedef struct {
    td_u32 threed_nr_count;
    td_u32 threed_nr_iso[OT_SCENE_3DNR_MAX_COUNT];
    ot_scene_3dnr threednr_value[OT_SCENE_3DNR_MAX_COUNT];
} ot_scene_static_3dnr;

typedef struct {
    td_u8 ae_weight[OT_ISP_AE_ZONE_ROW][OT_ISP_AE_ZONE_COLUMN];
} ot_scene_static_statisticscfg;

typedef struct {
    td_u8 auto_sat[OT_ISP_AUTO_ISO_NUM];
} ot_scene_static_saturation;

typedef struct {
    td_u8 ccm_op_type;
    td_u16 manual_ccm[OT_ISP_CCM_MATRIX_SIZE];
    td_bool auto_iso_act_en;
    td_bool auto_temp_act_en;
    td_u32 total_num;
    td_u16 auto_color_temp[OT_ISP_CCM_MATRIX_NUM];
    td_u16 auto_ccm[OT_ISP_CCM_MATRIX_NUM][OT_ISP_CCM_MATRIX_SIZE];
} ot_scene_static_ccm;

typedef struct {
    td_bool enable;
    td_bool limited_range_en;
    td_u8 hue;
    td_u8 luma;
    td_u8 contrast;
    td_u8 saturation;
    ot_color_gamut color_gamut;
} ot_scene_static_csc;

typedef struct {
    td_u8 nddm_strength[OT_ISP_AUTO_ISO_NUM];           /* RW; Range:[0x0, 0xFF]; Format:4.4; Non-direction strength */
    td_u8 nddm_mf_detail_strength[OT_ISP_AUTO_ISO_NUM]; /* RW; Range:SD3403V100 = [0x0, 0x7f];
                                                           Format:3.4; Non-direction medium frequent detail
                                                           enhance strength */
    td_u8 nddm_hf_detail_strength[OT_ISP_AUTO_ISO_NUM]; /* RW; Range:[0x0, 0x10]; Format:2.2; Non-direction
                                                           high frequent detail enhance strength */
    td_u8 detail_smooth_range[OT_ISP_AUTO_ISO_NUM];     /* RW; Range:SD3403V100 = [0x1, 0x7];
                                                           Format:4.0; Detail smooth range */
    td_u8 color_noise_f_threshold[OT_ISP_AUTO_ISO_NUM]; /* RW;Range:[0x0, 0xff]; Format:8.0;
                                                           Threshold of color noise cancel */
    td_u8 color_noise_f_strength[OT_ISP_AUTO_ISO_NUM];  /* RW;Range:[0x0, 0x8]; Format:4.0; Strength of color
                                                           noise cancel */
    td_u8 color_noise_y_threshold[OT_ISP_AUTO_ISO_NUM]; /* RW;Range:SD3403V100 = [0x0, 0xFF]; Format:8.0;Range of
	                                                       color denoise luma, related to luminance and saturation */
    td_u8 color_noise_y_strength[OT_ISP_AUTO_ISO_NUM];  /* RW;Range:SD3403V100 = [0x0, 0xFF]; Format:8.0;Format:8.0;
                                                           Strength of color denoise luma */
} ot_scene_static_dm_auto;

typedef struct {
    td_bool enable; /* RW; Range:[0, 1]; Format:1.0;Enable/Disable demosaic module */
    ot_op_mode op_type;
    ot_scene_static_dm_auto dm_auto_cfg;
} ot_scene_static_demosaic;

typedef struct {
    td_u16  dering_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8   dering_thresh[OT_ISP_AUTO_ISO_NUM];
    td_u8   dering_static_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8   dering_motion_strength[OT_ISP_AUTO_ISO_NUM];
} ot_scene_static_bnr_dering_snr;

typedef struct {
    td_u16 sfm0_coarse_strength[OT_ISP_BAYER_CHN_NUM][OT_ISP_AUTO_ISO_NUM];
    td_u8  sfm6_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8  sfm7_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8  sth[OT_ISP_AUTO_ISO_NUM];
    td_u8  sfm1_adp_strength[OT_ISP_AUTO_ISO_NUM];
    td_u16  sfm1_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8 sfm0_detail_prot[OT_ISP_AUTO_ISO_NUM];

    td_u8 fine_strength[OT_ISP_AUTO_ISO_NUM];
    td_u16 coring_wgt[OT_ISP_AUTO_ISO_NUM];
    td_u8 coring_mot_ratio[OT_ISP_AUTO_ISO_NUM];
} ot_scene_static_bnr_snr;

typedef struct {
    td_bool md_mode[OT_ISP_AUTO_ISO_NUM];
    td_u8 md_anti_flicker_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8 md_static_ratio[OT_ISP_AUTO_ISO_NUM];
    td_u8 md_motion_ratio[OT_ISP_AUTO_ISO_NUM];
    td_u8 md_static_fine_strength[OT_ISP_AUTO_ISO_NUM];
    td_u8 tfs[OT_ISP_AUTO_ISO_NUM];

    td_bool user_define_md[OT_ISP_AUTO_ISO_NUM];
    td_s16 user_define_slope[OT_ISP_AUTO_ISO_NUM];
    td_u16 user_define_dark_thresh[OT_ISP_AUTO_ISO_NUM];
    td_u8 user_define_color_thresh[OT_ISP_AUTO_ISO_NUM];

    td_u8 sfr_r[OT_ISP_AUTO_ISO_NUM];
    td_u8 sfr_g[OT_ISP_AUTO_ISO_NUM];
    td_u8 sfr_b[OT_ISP_AUTO_ISO_NUM];
} ot_scene_static_bnr_tnr;

typedef struct {
    td_bool enable;        /* RW; Range:[0,1];Format:1.0; bayernr enable  */
    ot_op_mode op_type;    /* RW; Range:[0,1];Format:1.0; bayernr operation mode  */
    td_bool tnr_enable;    /* RW; Range:[0,1];Format:1.0; bayernr tnr enable */
    td_bool lsc_nr_enable; /* RW; Range:[0,1];Format:1.0; lsc nr enable */
    td_u8 lsc_ratio1;      /* RW; Range:[0,15];Format:4.0; lsc nr ratio1 */

    td_u16 coring_ratio[OT_ISP_BAYERNR_LUT_LENGTH];
    td_u16 mix_gain[OT_ISP_BAYERNR_LUT_LENGTH1];
    ot_scene_static_bnr_snr snr_cfg;
    ot_scene_static_bnr_tnr tnr_cfg;
    ot_scene_static_bnr_dering_snr  dering_snr_cfg;
} ot_scene_static_nr;

typedef struct {
    td_u8 luma_wgt[OT_ISP_SHARPEN_LUMA_NUM][OT_ISP_AUTO_ISO_NUM]; /* RW; Range:SD3403V100 =[0, 31]; Format:0.5;
                                                                     Adjust the sharpen strength according to luma.
                                                                     Sharpen strength will be weaker when it decrease */
    td_u16 texture_strength[OT_ISP_SHARPEN_GAIN_NUM][OT_ISP_AUTO_ISO_NUM]; /* RW; Range: [0, 4095]; Format:7.5;
                                                                              Undirectional sharpen strength for texture
                                                                              and detail enhancement */
    td_u16 edge_strength[OT_ISP_SHARPEN_GAIN_NUM][OT_ISP_AUTO_ISO_NUM]; /* RW; Range: [0, 4095]; Format:7.5; Directional
                                                                           sharpen strength for edge enhancement */
    td_u16 texture_freq[OT_ISP_AUTO_ISO_NUM]; /* RW; Range: [0, 4095]; Format:6.6;Texture frequency adjustment.
                                                 Texture and detail will be finer when it increase */
    td_u16 edge_freq[OT_ISP_AUTO_ISO_NUM];    /* RW; Range: [0, 4095]; Format:6.6;Edge frequency adjustment.
                                                 Edge will be narrower and thiner when it increase */
    td_u8 over_shoot[OT_ISP_AUTO_ISO_NUM];    /* RW; Range: [0, 127];  Format:7.0 */
    td_u8 under_shoot[OT_ISP_AUTO_ISO_NUM];   /* RW; Range: [0, 127];  Format:7.0 */

    td_u8 shoot_sup_strength[OT_ISP_AUTO_ISO_NUM]; /* overshoot and undershoot suppression strength */
    td_u8 shoot_sup_adj[OT_ISP_AUTO_ISO_NUM]; /* RW; Range: [0, 15]; Format:4.0;overshoot and undershoot
                                                 suppression adjusting, adjust the edge shoot suppression strength */
    td_u8 detail_ctrl[OT_ISP_AUTO_ISO_NUM];           /* RW; Range: [0, 255]; Format:8.0;Different sharpen strength for
                                                         detail and edge. When it is bigger than 128, detail sharpen
                                                         strength will be stronger than edge. */
    td_u8 detail_ctrl_threshold[OT_ISP_AUTO_ISO_NUM]; /* RW; Range: [0, 255]; Format:8.0; The threshold of DetailCtrl,
                                                         it is used to distinguish detail and edge. */
    td_u8 edge_filt_strength[OT_ISP_AUTO_ISO_NUM];    /* RW; Range: [0, 63]; Format:6.0;
                                                         The strength of edge filtering. */
    td_u8 edge_filt_max_cap[OT_ISP_AUTO_ISO_NUM];     /* RW; Range: [0, 47]; Format:6.0;
                                                         The max capacity of edge filtering. */
    td_u8 r_gain[OT_ISP_AUTO_ISO_NUM];                /* RW; Range: [0, 31]; Format:5.0; Sharpen Gain for Red Area */
    td_u8 g_gain[OT_ISP_AUTO_ISO_NUM];                /* RW; Range: [0, 255]; Format:8.0; Sharpen Gain for Green Area */
    td_u8 b_gain[OT_ISP_AUTO_ISO_NUM];                /* RW; Range: [0, 31]; Format:5.0; Sharpen Gain for Blue Area */
    td_u8 skin_gain[OT_ISP_AUTO_ISO_NUM];             /* RW; Range: [0, 31]; Format:5.0;Sharpen Gain for Skin Area */
    td_u16 max_sharp_gain[OT_ISP_AUTO_ISO_NUM];       /* RW; Range: [0, 0x7FF]; Format:8.3; Maximum sharpen gain */
    td_u8 weak_detail_gain[OT_ISP_AUTO_ISO_NUM];      /* RW; Range: [0, 127]; Format:7.0; sharpen Gain for weakdetail */
} ot_scene_static_sharpen_auto;

typedef struct {
    td_bool enable;          /* RW; Range:[0, 1]; Format:1.0;Enable/Disable sharpen module */
    td_u8 skin_umin;         /* RW; Range: [0, 255];  Format:8.0; U min value of the range of skin area */
    td_u8 skin_vmin;         /* RW; Range: [0, 255];  Format:8.0; V min value of the range of skin area */
    td_u8 skin_umax;         /* RW; Range: [0, 255];  Format:8.0; U max value of the range of skin area */
    td_u8 skin_vmax;         /* RW; Range: [0, 255];  Format:8.0; V max value of the range of skin area */

    ot_scene_static_sharpen_auto sharpen_auto_cfg;
} ot_scene_static_sharpen;

typedef struct {
    td_u8  mf_strength[OT_SCENE_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM];   /* RW; range:[0, 63];mf_strength. */
    td_u8  hf_strength[OT_SCENE_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM];   /* RW; range:[0, 63];hf_strength. */
    td_u8  dark_strength[OT_SCENE_BSHP_CURVE_NUM][OT_ISP_AUTO_ISO_NUM]; /* RW; range:[0, 63];dark_strength. */
    td_u8  mf_gain[OT_ISP_AUTO_ISO_NUM];    /* RW; range:[0, 15];format:4.0; mf gain */
    td_u8  hf_gain[OT_ISP_AUTO_ISO_NUM];    /* RW; range:[0, 15];format:4.0; hf gain */
    td_u8  dark_gain[OT_ISP_AUTO_ISO_NUM];  /* RW; range:[0, 15];format:4.0; dark gain */
    td_u16 overshoot[OT_ISP_AUTO_ISO_NUM];  /* RW; range:[0, 255];format:8.0; over shoot */
    td_u16 undershoot[OT_ISP_AUTO_ISO_NUM]; /* RW; range:[0, 255];format:8.0; under shoot */
} ot_scene_static_bayershp_auto;

typedef struct {
    td_bool enable; /* RW; range:[0, 1]; format:1.0;en/disable bayershp module */
    td_u16  dark_threshold[OT_ISP_BSHP_THD_NUM];    /* RW; range:[0, 4095];format:12.0; dark threshold */
    td_u16  texture_threshold[OT_ISP_BSHP_THD_NUM]; /* RW; range:[0, 4095];format:12.0; texture threshold */
    ot_op_mode op_type;
    ot_scene_static_bayershp_auto bayershp_auto_attr;
} ot_scene_static_bayershp;

typedef struct {
    td_bool enable;
} ot_scene_static_clut;

typedef struct {
    td_bool enable;
} ot_scene_static_crosstalk;

typedef struct {
    td_bool enable;
    td_u32 y_ratio_lut[OT_ISP_CA_YRATIO_LUT_LENGTH];
    td_u32 y_sat_lut[OT_ISP_CA_YRATIO_LUT_LENGTH];
    td_u16 iso_ratio[ISP_AUTO_ISO_CA_NUM];
} ot_scene_static_ca;

/* The param of h265e vbr */
typedef struct {
    td_s32 change_pos;         /* RW; Range:[50, 100];Indicates the ratio of the current bit rate to the maximum
                                     bit rate when the QP value starts to be adjusted */
    td_u32 min_iprop;          /* RW; [1, 100]the min ratio of i frame and p frame */
    td_u32 max_iprop;          /* RW; [1, 100]the max ratio of i frame and p frame,can not be smaller than min_iprop */
    td_s32 max_reencode_times; /* RW; Range:[0, 3]; Range:max number of re-encode times. */

    td_s32 min_still_percent; /* RW; Range:[5, 100]; the min percent of target bitrate for still scene */
    td_u32 max_still_qp;      /* RW; Range:[0, 51]; the max QP value of I frame for still scene,
                                      can not be smaller than min_iqp and can not be larger than su32MaxIQp */
    td_u32 min_still_psnr;    /* RW; reserved */

    td_u32 max_qp;  /* RW; Range:[0, 51];the max P B qp */
    td_u32 min_qp;  /* RW; Range:[0, 51];the min P B qp,can not be larger than max_qp */
    td_u32 max_iqp; /* RW; Range:[0, 51];the max I qp */
    td_u32 min_iqp; /* RW; Range:[0, 51];the min I qp,can not be larger than max_iqp */

    td_u32 min_qp_delta;
    td_u32 motion_sensitivity; /* RW; Range:[0, 100]; Motion Sensitivity */

    td_bool qp_map_en;                 /* RW; Range:[0, 1]; enable qpmap. */
    ot_venc_rc_qpmap_mode qp_map_mode; /* RW; Qpmap Mode */
} ot_scene_static_venc_param_h265_avbr;

typedef struct {
    td_u32 min_iprop; /* RW; Range:[1, 100] ; the min ratio of i frame and p frame */
    td_u32 max_iprop; /* RW; Range:[1, 100] ; the max ratio of i frame and p frame,can not be smaller than min_iprop */
    td_s32 max_reencode_times; /* RW; Range:[0, 3]; max number of re-encode times */
    td_bool qp_map_en;
    ot_venc_rc_qpmap_mode qp_map_mode; /* RW; Qpmap Mode */

    td_u32 max_qp;  /* RW; Range:[0, 51]; the max P B qp */
    td_u32 min_qp;  /* RW; Range:[0, 51]; the min P B qp,can not be larger than max_qp */
    td_u32 max_iqp; /* RW; Range:[0, 51]; the max I qp */
    td_u32 min_iqp; /* RW; Range:[0, 51]; the min I qp,can not be larger than max_iqp */

    td_u32 min_qp_delta;
    td_u32 max_qp_delta;
    td_u32 extra_bit_percent;
    td_u32 long_term_static_time_unit;
} ot_scene_static_venc_param_h265_cvbr;

typedef struct {
    td_bool deblur_en;
    td_bool deblur_adaptive_en;
    ot_scene_static_venc_param_h265_avbr venc_param_h265_avbr;
    ot_scene_static_venc_param_h265_cvbr venc_param_h265_cvbr;
} ot_scene_static_venc_attr;

typedef struct {
    td_u32 ratio_count;
    td_u32 ratio_level[OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u16 mix_gain[OT_SCENE_NR_LUT_LENGTH1][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 md_fusion_frame_str_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 snr_sfm0_fusion_frame_str_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 md_wdr_frame_str_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 snr_sfm0_wdr_frame_str_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 snr_fusion_sfm6_sfm7_frame_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];
    td_u8 snr_wdr_sfm6_sfm7_frame_incr[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_DRC_RATIO_MAX_COUNT];

    td_u32 coring_ratio_count;
    td_u32 coring_ratio_iso[OT_SCENE_NR_RATIO_MAX_COUNT];
    td_u16 coring_ratio[OT_SCENE_NR_LUT_LENGTH][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_sfm0_wdr_frame_str[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_sfm0_fusion_frame_str[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 md_wdr_frame_str[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 md_fusion_frame_str[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_wdr_sfm6_strength[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_wdr_sfm7_strength[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_fusion_sfm6_strength[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 snr_fusion_sfm7_strength[OT_ISP_WDR_MAX_FRAME_NUM][OT_SCENE_ISO_STRENGTH_NUM];

    /* used for different exp ratio */
    td_u16 wdr_ratio_threshold;
    td_u8  fine_strength_l[OT_ISP_AUTO_ISO_NUM];
    td_u8  fine_strength_h[OT_ISP_AUTO_ISO_NUM];
    td_u16 coring_wgt_l[OT_ISP_AUTO_ISO_NUM];
    td_u16 coring_wgt_h[OT_ISP_AUTO_ISO_NUM];
} ot_scene_dynamic_nr;

typedef struct {
    td_u8 md_fusion_incr[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u8 sfm0_fusion_incr[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u8 md_wdr_incr[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u8 sfm0_wdr_incr[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u8 snr_fusion_sfm6_sfm7_incr[OT_ISP_WDR_MAX_FRAME_NUM];
    td_u8 snr_wdr_sfm6_sfm7_incr[OT_ISP_WDR_MAX_FRAME_NUM];
} ot_scene_nr_para;

typedef struct {
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 denoise_y_alpha[OT_SCENE_ISO_STRENGTH_NUM];        /* RW; Range:[0, 255];Format:8.0; */
    td_u8 denoise_y_mot_ratio[OT_SCENE_ISO_STRENGTH_NUM];    /* RW; Range:[0, 63];  Format:6.0; */
    td_u8 denoise_y_fg_str_lut[OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 denoise_y_bg_str_lut[OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 denoise_y_mot_map_lut[OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM][OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_ynet_para;

typedef struct {
    td_u32 iso_count;
    td_u32 ratio_count;
    td_u32 iso_level[OT_SCENE_ISO_STRENGTH_NUM];                                 /* iso threshold count */
    td_u32 ratio_level[OT_SCENE_RATIO_STRENGTH_NUM];                                 /* iso threshold count */
    td_u32 blend_weight[OT_SCENE_RATIO_STRENGTH_NUM];                                 /* iso threshold count */
    td_u32 ca_y_ratio_iso_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_ISP_CA_YRATIO_LUT_LENGTH]; /* cay_ratio_lut count */
    td_u32 ca_y_ratio_ratio_lut[OT_SCENE_RATIO_STRENGTH_NUM][OT_ISP_CA_YRATIO_LUT_LENGTH]; /* cay_ratio_lut count */
} ot_scene_dynamic_ca;

typedef struct {
    td_u8  black_level_mode;
    td_u32 blc_count;
    td_u32 iso_thresh[ISP_AUTO_ISO_BLC_NUM];
    td_u32 blc_r[ISP_AUTO_ISO_BLC_NUM];
    td_u32 blc_gr[ISP_AUTO_ISO_BLC_NUM];
    td_u32 blc_gb[ISP_AUTO_ISO_BLC_NUM];
    td_u32 blc_b[ISP_AUTO_ISO_BLC_NUM];
} ot_scene_dynamic_blc;

typedef struct {
    td_u8  iso_count;
    td_u32 iso_level[ISP_AUTO_ISO_DPC_NUM];
    td_u8  sup_twinkle_en[OT_ISP_WDR_MAX_FRAME_NUM][ISP_AUTO_ISO_DPC_NUM];
    td_u8  soft_thr[OT_ISP_WDR_MAX_FRAME_NUM][ISP_AUTO_ISO_DPC_NUM];
    td_u8  soft_slope[OT_ISP_WDR_MAX_FRAME_NUM][ISP_AUTO_ISO_DPC_NUM];
    td_u8  ratio_count;
    td_u32 ratio_level[ISP_AUTO_ISO_DPC_NUM];
    td_u8  bright_strength[OT_ISP_WDR_MAX_FRAME_NUM][ISP_AUTO_ISO_DPC_NUM];
    td_u8  dark_strength[OT_ISP_WDR_MAX_FRAME_NUM][ISP_AUTO_ISO_DPC_NUM];
} ot_scene_dynamic_dpc;

typedef struct {
    td_bool enable;
} ot_scene_static_shading;

typedef struct {
    td_u32 exp_thresh_cnt;
    td_u64 exp_thresh_ltoh[OT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
    td_u16 mesh_strength[OT_SCENE_SHADING_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_shading;

typedef struct {
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_ISO_STRENGTH_NUM];

    td_u32 gain_r[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 gain_g[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 gain_b[OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_clut;

typedef struct {
    td_u8 option;
    td_u32 interval;
    td_u32 total_num;
    td_u8 iso_count;
    td_u8 ratio_count;
    td_u64 exp_thresh_ltoh[OT_SCENE_GAMMA_EXPOSURE_MAX_COUNT];
    td_u64 exp_thresh_htol[OT_SCENE_GAMMA_EXPOSURE_MAX_COUNT];
    td_u16 table[OT_SCENE_GAMMA_EXPOSURE_MAX_COUNT][OT_ISP_GAMMA_NODE_NUM];
    td_u16 gamma_idx[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_RATIO_STRENGTH_NUM];
} ot_scene_dynamic_gamma;

typedef struct {
    td_bool enable;
    td_u32 table[OT_ISP_PREGAMMA_NODE_NUM];
} ot_scene_static_pregamma;

typedef enum {
    OT_ISP_BLC_NORMAL = 0,
    OT_ISP_BLC_GLOBAL,
    OT_ISP_BLC_REAL_TIME,
    OT_ISP_BLC_MODE_BUTT
} ot_isp_blc_process_sel;


typedef struct {
    td_bool enable;
    ot_isp_blc_process_sel blc_sel;
    td_u16 user_offset[OT_ISP_BAYER_CHN_NUM];
} ot_scene_static_blc;

typedef struct {
    td_u32 exp_thresh_cnt;
    td_u64 exp_thresh_ltoh[OT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];
    td_u8 manual_strength[OT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];
    td_u32 wdr_ratio_threshold;
    td_u8 manual_strengther[OT_SCENE_DEHAZE_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_dehaze;

typedef struct {
    td_u8 md_thr_low_gain[OT_SCENE_RATIO_STRENGTH_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 md_thr_high_gain[OT_SCENE_RATIO_STRENGTH_NUM][OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_wdr_combine_attr;

typedef struct {
    td_u8  ratio_count;
    td_u32 ratio_level[ISP_AUTO_RATIO_FSWDR_NUM];
    td_u8  wdr_merge_mode[ISP_AUTO_RATIO_FSWDR_NUM];
    td_u8  motion_comp[ISP_AUTO_RATIO_FSWDR_NUM];

    td_u8  iso_count;
    td_u32 ratio_thr;
    td_u32 iso_ltoh_thresh[OT_SCENE_ISO_STRENGTH_NUM];
    td_bool fusion_blend_en[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 fusion_blend_wgt[OT_SCENE_ISO_STRENGTH_NUM];
    td_bool fusion_blend_en_h[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 fusion_blend_wgt_h[OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_fswdr;

typedef struct {
    td_u32 total_num;
    td_u32 false_color_exp_thresh[OT_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT];
    td_u8 manual_strength[OT_SCENE_FALSECOLOR_EXPOSURE_MAX_COUNT];
} ot_scene_dynamic_false_color;

typedef struct {
    td_u32 iso_count;
    td_u32 iso_thresh[OT_SCENE_ISO_STRENGTH_NUM];
    td_bool fusion_mono_flat_en[OT_SCENE_ISO_STRENGTH_NUM];
    td_bool fusion_color_ratio_en[OT_SCENE_ISO_STRENGTH_NUM];
    td_bool fusion_mono_ratio_en[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  fusion_ratio_scale[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  lf_mono_flt_radius[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  mf_mono_flt_radius[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  hf_mono_flt_radius[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  lf_fusion_det_gain[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  mf_fusion_det_gain[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8  hf_fusion_det_gain[OT_SCENE_ISO_STRENGTH_NUM];
    td_u16 cc_uv_gain_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_CC_UV_GAIN_LUT_NUM];
    td_u8  lf_fusion_alpha_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  mf_fusion_alpha_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  hf_fusion_alpha_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  fusion_color_ratio_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  lf_fusion_mono_det_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  mf_fusion_mono_det_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
    td_u8  hf_fusion_mono_det_lut[OT_SCENE_ISO_STRENGTH_NUM][OT_SCENE_MCF_WEIGHT_LUT_NUM];
} ot_scene_dynamic_mcf;

typedef struct {
    td_u32 aibnr_iso_thresh[AIBNR_ISO_NUM];
    td_bool aibnr_chg_en;
    td_bool blend_en;
    ot_op_mode op_type;
    td_u8 manual_id; /* RW; Range:[0, 15]; Format:4.0 */
    td_u8 auto_id[OT_AIISP_AUTO_ISO_NUM]; /* RW; Range:[0, 15]; Format:4.0 */
    td_s32 pipe_id;
    td_u32 iso_count;
    td_u32 iso_level[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 sfs[OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_aibnr;

typedef struct {
    td_u32 iso_count;
    td_bool ai3dnr_en[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 iso_level[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 uv_denoise_fg[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 uv_denoise_bg[OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 denoise_y_fg_str_lut[OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM][OT_SCENE_ISO_STRENGTH_NUM];
    td_u8 denoise_y_bg_str_lut[OT_SCENE_BAYERNR_DENOISE_Y_LUT_NUM][OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_ai3dnr;

typedef struct {
    td_u32 deblur_mode_iso_thresh[VENC_MODE_ISO_NUM];
    td_bool deblur_en[VENC_MODE_ISO_NUM];
    td_bool deblur_adaptive_en[VENC_MODE_ISO_NUM];
} ot_scene_dynamic_venc_mode;

typedef struct {
    td_u32 iso_count;
    td_bool aibnr_mode;
    td_u32 fpn_iso_thresh;
    td_u32 iso_thresh[OT_SCENE_ISO_STRENGTH_NUM];
    td_u32 fpn_offset[OT_SCENE_ISO_STRENGTH_NUM];
} ot_scene_dynamic_fpn;

typedef struct {
    ot_scene_module_state module_state;
    ot_scene_static_ae static_ae;
    ot_scene_static_ae_route static_ae_route;
    ot_scene_static_ae_route_ex static_ae_route_ex;
    ot_scene_static_statisticscfg static_statistics;
    ot_scene_static_wdr_exposure static_wdr_exposure;
    ot_scene_static_fswdr static_fswdr;
    ot_scene_static_awb static_awb;
    ot_scene_static_awb_ex static_awb_ex;
    ot_scene_isp_pipe_diff static_pipe_diff;
    ot_scene_static_ccm static_ccm;
    ot_scene_static_saturation static_saturation;
    ot_scene_static_clut static_clut;
    ot_scene_static_ldci static_ldci;
    ot_scene_static_drc static_drc;
    ot_scene_static_aidrc static_aidrc;
    ot_scene_static_nr static_nr;
    ot_scene_static_ca static_ca;
    ot_scene_static_pregamma static_pregamma;
    ot_scene_static_blc static_blc;
    ot_scene_static_venc_attr static_venc_attr;
    ot_scene_static_cac static_cac;
    ot_scene_static_dpc static_dpc;
    ot_scene_static_dehaze static_dehaze;
    ot_scene_static_shading static_shading;
    ot_scene_static_csc staic_csc;
    ot_scene_static_crosstalk static_crosstalk;
    ot_scene_static_demosaic static_dm;
    ot_scene_static_sharpen static_sharpen;
    ot_scene_static_bayershp static_bayershp;
    ot_scene_static_3dnr static_threednr;

    ot_scene_dynamic_fps dynamic_fps;
    ot_scene_dynamic_ae dynamic_ae;
    ot_scene_dynamic_wdr_exposure dynamic_wdr_exposure;
    ot_scene_dynamic_fswdr dynamic_fswdr;
    ot_scene_dynamic_dehaze dynamic_dehaze;
    ot_scene_dynamic_drc dynamic_drc;
    ot_scene_dynamic_aidrc dynamic_aidrc;
    ot_scene_dynamic_linear_drc dynamic_linear_drc;
    ot_scene_dynamic_gamma dynamic_gamma;
    ot_scene_dynamic_nr dynamic_nr;
    ot_scene_dynamic_ca dynamic_ca;
    ot_scene_dynamic_blc dynamic_blc;
    ot_scene_dynamic_dpc dynamic_dpc;
    ot_scene_dynamic_shading dynamic_shading;
    ot_scene_dynamic_clut dynamic_clut;
    ot_scene_dynamic_venc_bitrate dynamic_venc_bitrate;
    ot_scene_dynamic_ldci dynamic_ldci;
    ot_scene_dynamic_false_color dynamic_false_color;
    ot_scene_dynamic_mcf dynamic_mcf;
    ot_scene_dynamic_aibnr dynamic_aibnr;
    ot_scene_dynamic_ai3dnr dynamic_ai3dnr;
    ot_scene_dynamic_fpn dynamic_fpn;
    ot_scene_dynamic_venc_mode dynamic_venc_mode;
} ot_scene_pipe_param;

typedef struct {
    ot_scene_3dnr *nrx_attr;
    const ot_scene_static_3dnr *nrx_param;
    td_u32 iso_level;
    td_u32 mid;
    td_u32 left;
    td_u32 right;
} _3dnr_nrx_pack;

td_s32 ot_scene_set_static_ae(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_dynamic_fps(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_ae(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_static_cac(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_dpc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_wdr_exposure(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_fswdr(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_drc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_aidrc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_dehaze(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_ldci(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_awb(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_awbex(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_pipe_diff(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_saturation(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_ccm(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_csc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_nr(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_shading(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_ae_weight_table(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_3dnr(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_pregamma(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_blc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_dynamic_3dnr(ot_vi_pipe vi_pipe, td_u32 iso, td_u8 index, ot_3dnr_pos_type pos_3dnr);

td_s32 ot_scene_set_static_demosaic(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_sharpen(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_bayershp(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_clut(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_ca(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_venc(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_static_crosstalk(ot_vi_pipe vi_pipe, td_u8 index);

td_s32 ot_scene_set_dynamic_photo_gamma(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_video_gamma(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_wdr_exposure(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_shading(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_clut(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_linear_ca(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_venc_bitrate(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_mcf(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_aibnr(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_ai3dnr(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_fpn(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_nr(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio, td_u32 iso);

td_s32 ot_scene_set_dynamic_ca(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio, td_u32 iso);

td_s32 ot_scene_set_dynamic_blc(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_dpc(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_dpc_ratio(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio);

td_s32 ot_scene_set_dynamic_ldci(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_black_level(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_false_color(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_dehaze(ot_vi_pipe vi_pipe, td_u64 exposure, td_u64 last_exposure, td_u8 index);

td_s32 ot_scene_set_dynamic_fswdr(ot_vi_pipe vi_pipe, td_u32 iso, td_u32 last_iso, td_u8 index, td_u32 wdr_ratio);

td_s32 ot_scene_set_dynamic_drc(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio, td_u32 iso);

td_s32 ot_scene_set_dynamic_aidrc(ot_vi_pipe vi_pipe, td_u8 index, td_u32 wdr_ratio, td_u32 iso);

td_s32 ot_scene_set_dynamic_linear_drc(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_dynamic_venc_mode(ot_vi_pipe vi_pipe, td_u64 iso, td_u64 last_iso, td_u8 index);

td_s32 ot_scene_set_pipe_param(const ot_scene_pipe_param *scene_pipe_param, td_u32 num);

td_s32 ot_scene_set_pause(td_bool pause);

td_s32 ot_scene_set_rc_param(ot_vi_pipe vi_pipe, td_u8 idx);

td_void set_dir_name(const td_char *dir_name);

td_s32 ot_scene_release_fpn(ot_vi_pipe vi_pipe, td_u8 index);

#pragma pack()
#ifdef __cplusplus
}
#endif

#endif
