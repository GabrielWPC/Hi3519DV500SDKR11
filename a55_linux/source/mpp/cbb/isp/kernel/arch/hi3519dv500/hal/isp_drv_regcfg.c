/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_drv.h"
#include "isp_drv_define.h"
#include "isp_reg_define.h"
#include "isp_stt_define.h"
#include "ot_common.h"
#include "ot_osal.h"
#include "ot_math.h"
#include "mkp_isp.h"
#include "isp.h"
#include "mm_ext.h"
#include "sys_ext.h"

#define DYNAMIC_BLC_FILTER_FIX_BIT 6

static td_u32 g_drc_cur_luma_lut[OT_ISP_DRC_SHP_LOG_CONFIG_NUM][OT_ISP_DRC_EXP_COMP_SAMPLE_NUM - 1] = {
    {1,     1,      5,      31,     180,    1023,   32767},
    {1,     3,      8,      52,     277,    1446,   38966},
    {2,     5,      15,     87,     427,    2044,   46337},
    {4,     9,      27,     144,    656,    2888,   55101},
    {7,     16,     48,     240,    1008,   4080,   65521},
    {12,    29,     85,     399,    1547,   5761,   77906},
    {23,    53,     151,    660,    2372,   8128,   92622},
    {42,    97,     267,    1090,   3628,   11458,  110100},
    {76,    175,    468,    1792,   5537,   16130,  130840},
    {138,   313,    816,    2933,   8423,   22664,  155417},
    {258,   555,    1412,   4770,   12758,  31760,  184476},
    {441,   977,    2420,   7699,   19215,  44338,  218711},
    {776,   1698,   4100,   12304,  28720,  61568,  258816},
    {1344,  2907,   6847,   19416,  42491,  84851,  305376},
    {2283,  4884,   11224,  30137,  62006,  115708, 358680},
    {3783,  8004,   17962,  45770,  88821,  155470, 418391},
};

static td_u32 g_drc_div_denom_log[OT_ISP_DRC_SHP_LOG_CONFIG_NUM] = {
    52429, 55188,  58254,  61681,  65536,  69905,  74898, 80659,
    87379, 95319, 104843, 116472, 130980, 149557, 174114, 207870
};

static td_u32 g_drc_denom_exp[OT_ISP_DRC_SHP_LOG_CONFIG_NUM] = {
    1310720, 1245184, 1179648, 1114113, 1048577, 983043, 917510, 851980,
    786455,  720942,  655452,  590008,  524657, 459488, 394682, 330589
};

static td_u8 g_drc_shp_log[OT_ISP_MAX_PIPE_NUM][OT_ISP_STRIPING_MAX_NUM] = {
    [0 ... OT_ISP_MAX_PIPE_NUM - 1] = { 8, 8, 8 }
};
static td_u8 g_drc_shp_exp[OT_ISP_MAX_PIPE_NUM][OT_ISP_STRIPING_MAX_NUM] = {
    [0 ... OT_ISP_MAX_PIPE_NUM - 1] = { 8, 8, 8 }
};

static td_s64 signed_right_shift(td_s64 value, td_u8 bit_shift)
{
    td_u64 value_pos;
    td_u64 tmp;
    value_pos = (td_u64)value;
    if (value < 0) {
        value_pos = value_pos >> bit_shift;
        tmp = (((0x1LL << (bit_shift)) - 1LL)) << (64 - bit_shift); /* left shift (64 - bit_shift)bits */
        value_pos = value_pos | tmp;
        return (td_s64)value_pos;
    } else {
        return (td_s64)(value_pos >> bit_shift);
    }
}

static td_u16 sqrt32(td_u32 arg)
{
    td_u32 mask = (td_u32)1 << 15; /* left shift 15 */
    td_u16 res = 0;
    td_u32 i;

    for (i = 0; i < 16; i++) {  /* max value 16 */
        if ((res + (mask >> i)) * (res + (mask >> i)) <= arg) {
            res = res + (mask >> i);
        }
    }

    /* rounding */
    if ((td_u32)(res * res + res) < arg) {
        ++res;
    }

    return res;
}

/* isp drv FHY regs define */
static td_void isp_drv_input_sel_write(isp_pre_be_reg_type *isp_be_regs, const td_u32 *input_sel)
{
    u_isp_be_input_mux o_isp_be_input_mux;
    isp_check_pointer_void_return(isp_be_regs);

    o_isp_be_input_mux.u32 = isp_be_regs->isp_be_input_mux.u32;
    o_isp_be_input_mux.bits.isp_input0_sel = input_sel[0]; /* array index 0 */
    o_isp_be_input_mux.bits.isp_input1_sel = input_sel[1]; /* array index 1 */
    o_isp_be_input_mux.bits.isp_input2_sel = input_sel[2]; /* array index 2 */
    o_isp_be_input_mux.bits.isp_input3_sel = input_sel[3]; /* array index 3 */
    o_isp_be_input_mux.bits.isp_input4_sel = input_sel[4]; /* array index 4 */
    isp_be_regs->isp_be_input_mux.u32 = o_isp_be_input_mux.u32;
}

static __inline td_void isp_drv_set_cc_coef00(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef00)
{
    u_isp_cc_coef0 o_isp_cc_coef0;
    o_isp_cc_coef0.u32 = post_be->isp_cc_coef0.u32;
    o_isp_cc_coef0.bits.isp_cc_coef00 = uisp_cc_coef00;
    post_be->isp_cc_coef0.u32 = o_isp_cc_coef0.u32;
}

static __inline td_void isp_drv_set_cc_coef01(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef01)
{
    u_isp_cc_coef0 o_isp_cc_coef0;
    o_isp_cc_coef0.u32 = post_be->isp_cc_coef0.u32;
    o_isp_cc_coef0.bits.isp_cc_coef01 = uisp_cc_coef01;
    post_be->isp_cc_coef0.u32 = o_isp_cc_coef0.u32;
}
static __inline td_void isp_drv_set_cc_coef02(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef02)
{
    u_isp_cc_coef1 o_isp_cc_coef1;
    o_isp_cc_coef1.u32 = post_be->isp_cc_coef1.u32;
    o_isp_cc_coef1.bits.isp_cc_coef02 = uisp_cc_coef02;
    post_be->isp_cc_coef1.u32 = o_isp_cc_coef1.u32;
}

static __inline td_void isp_drv_set_cc_coef10(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef10)
{
    u_isp_cc_coef1 o_isp_cc_coef1;
    o_isp_cc_coef1.u32 = post_be->isp_cc_coef1.u32;
    o_isp_cc_coef1.bits.isp_cc_coef10 = uisp_cc_coef10;
    post_be->isp_cc_coef1.u32 = o_isp_cc_coef1.u32;
}

static __inline td_void isp_drv_set_cc_coef11(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef11)
{
    u_isp_cc_coef2 o_isp_cc_coef2;
    o_isp_cc_coef2.u32 = post_be->isp_cc_coef2.u32;
    o_isp_cc_coef2.bits.isp_cc_coef11 = uisp_cc_coef11;
    post_be->isp_cc_coef2.u32 = o_isp_cc_coef2.u32;
}

static __inline td_void isp_drv_set_cc_coef12(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef12)
{
    u_isp_cc_coef2 o_isp_cc_coef2;
    o_isp_cc_coef2.u32 = post_be->isp_cc_coef2.u32;
    o_isp_cc_coef2.bits.isp_cc_coef12 = uisp_cc_coef12;
    post_be->isp_cc_coef2.u32 = o_isp_cc_coef2.u32;
}

static __inline td_void isp_drv_set_cc_coef20(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef20)
{
    u_isp_cc_coef3 o_isp_cc_coef3;
    o_isp_cc_coef3.u32 = post_be->isp_cc_coef3.u32;
    o_isp_cc_coef3.bits.isp_cc_coef20 = uisp_cc_coef20;
    post_be->isp_cc_coef3.u32 = o_isp_cc_coef3.u32;
}

static __inline td_void isp_drv_set_cc_coef21(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef21)
{
    u_isp_cc_coef3 o_isp_cc_coef3;
    o_isp_cc_coef3.u32 = post_be->isp_cc_coef3.u32;
    o_isp_cc_coef3.bits.isp_cc_coef21 = uisp_cc_coef21;
    post_be->isp_cc_coef3.u32 = o_isp_cc_coef3.u32;
}

static __inline td_void isp_drv_set_cc_coef22(isp_post_be_reg_type *post_be, td_u32 uisp_cc_coef22)
{
    u_isp_cc_coef4 o_isp_cc_coef4;
    o_isp_cc_coef4.u32 = post_be->isp_cc_coef4.u32;
    o_isp_cc_coef4.bits.isp_cc_coef22 = uisp_cc_coef22;
    post_be->isp_cc_coef4.u32 = o_isp_cc_coef4.u32;
}

static td_void isp_drv_set_ccm(isp_post_be_reg_type *isp_be_regs, td_u16 *be_cc)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        if ((be_cc[i] >> 12) < 0x8) { /* valid bit 12, sign bit 0x8 */
            be_cc[i] = MIN2(be_cc[i], 0xFFF); /* max positive value 0xFFF */
        } else { /* valid bit 12, sign bit 0x8 */
            be_cc[i] = MIN2(be_cc[i], 0x8FFF); /* max negative value 0x8FFF */
        }
    }

    isp_drv_set_cc_coef00(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[0]))); /* array index 0 */
    isp_drv_set_cc_coef01(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[1]))); /* array index 1 */
    isp_drv_set_cc_coef02(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[2]))); /* array index 2 */
    isp_drv_set_cc_coef10(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[3]))); /* array index 3 */
    isp_drv_set_cc_coef11(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[4]))); /* array index 4 */
    isp_drv_set_cc_coef12(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[5]))); /* array index 5 */
    isp_drv_set_cc_coef20(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[6]))); /* array index 6 */
    isp_drv_set_cc_coef21(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[7]))); /* array index 7 */
    isp_drv_set_cc_coef22(isp_be_regs, ccm_convert(ccm_convert_pre(be_cc[8]))); /* array index 8 */
}

static __inline td_void isp_drv_set_awb_gain(isp_pre_be_reg_type *isp_be_regs, const td_u32 *wb_gain)
{
    isp_be_regs->isp_wb_gain1.u32 = (wb_gain[OT_ISP_CHN_R] << 16) + wb_gain[OT_ISP_CHN_GR]; /* left shift 16 */
    isp_be_regs->isp_wb_gain2.u32 = (wb_gain[OT_ISP_CHN_B] << 16) + wb_gain[OT_ISP_CHN_GB]; /* left shift 16 */
}
static __inline td_void isp_drv_set_be_dgain(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_dgain)
{
    isp_be_regs->isp_dg_gain1.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
    isp_be_regs->isp_dg_gain2.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
}

static __inline td_void isp_drv_set_fe_dgain(isp_fe_reg_type *isp_fe_regs, td_u32 isp_dgain)
{
    u_isp_dg2_gain1 o_isp_dg_gain1;
    u_isp_dg2_gain2 o_isp_dg_gain2;

    isp_check_pointer_void_return(isp_fe_regs);

    o_isp_dg_gain1.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
    isp_fe_regs->isp_dg2_gain1.u32 = o_isp_dg_gain1.u32;

    o_isp_dg_gain2.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
    isp_fe_regs->isp_dg2_gain2.u32 = o_isp_dg_gain2.u32;
}

static td_void isp_drv_set_fe_nr_dgain(ot_vi_pipe vi_pipe, isp_fe_reg_type *isp_fe_regs, td_u32 isp_dgain)
{
    td_u8  fe_nr_dg_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };
    if (fe_nr_dg_en[vi_pipe] == 0) {
        return;
    }

    isp_fe_regs->isp_dg1_gain1.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
    isp_fe_regs->isp_dg1_gain2.u32 = (isp_dgain << 16) + isp_dgain; /* left shift 16 */
}

static __inline td_void isp_drv_set_isp_4dgain0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp4_dgain0)
{
    isp_be_regs->isp_4dg_0_gain1.u32 = (isp4_dgain0 << 16) + isp4_dgain0; /* left shift 16 */
    isp_be_regs->isp_4dg_0_gain2.u32 = (isp4_dgain0 << 16) + isp4_dgain0; /* left shift 16 */
}

static __inline td_void isp_drv_set_isp_4dgain1(isp_pre_be_reg_type *isp_be_regs, td_u32 isp4_dgain1)
{
    isp_be_regs->isp_4dg_1_gain1.u32 = (isp4_dgain1 << 16) + isp4_dgain1; /* left shift 16 */
    isp_be_regs->isp_4dg_1_gain2.u32 = (isp4_dgain1 << 16) + isp4_dgain1; /* left shift 16 */
}

static __inline td_void isp_drv_set_isp_4dgain2(isp_pre_be_reg_type *isp_be_regs, td_u32 isp4_dgain2)
{
    isp_be_regs->isp_4dg_2_gain1.u32 = (isp4_dgain2 << 16) + isp4_dgain2; /* left shift 16 */
    isp_be_regs->isp_4dg_2_gain2.u32 = (isp4_dgain2 << 16) + isp4_dgain2; /* left shift 16 */
}

static __inline td_void isp_drv_set_isp_4dgain3(isp_pre_be_reg_type *isp_be_regs, td_u32 isp4_dgain3)
{
    isp_be_regs->isp_4dg_3_gain1.u32 = (isp4_dgain3 << 16) + isp4_dgain3; /* left shift 16 */
    isp_be_regs->isp_4dg_3_gain2.u32 = (isp4_dgain3 << 16) + isp4_dgain3; /* left shift 16 */
}

static __inline td_void isp_drv_set_wdr_exporratio0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_exporatio0)
{
    u_isp_wdr_exporratio o_isp_wdr_exporatio;

    o_isp_wdr_exporatio.u32 = isp_be_regs->isp_wdr_exporratio.u32;
    o_isp_wdr_exporatio.bits.isp_wdr_exporratio0 = isp_wdr_exporatio0;
    isp_be_regs->isp_wdr_exporratio.u32 = o_isp_wdr_exporatio.u32;
}

static __inline td_void isp_drv_set_wdr_expo_value0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_expovalue0)
{
    u_isp_wdr_expovalue  o_isp_wdr_expovalue0;

    o_isp_wdr_expovalue0.u32 = isp_be_regs->isp_wdr_expovalue.u32;
    o_isp_wdr_expovalue0.bits.isp_wdr_expovalue0 = isp_wdr_expovalue0;
    isp_be_regs->isp_wdr_expovalue.u32 = o_isp_wdr_expovalue0.u32;
}

static __inline td_void isp_drv_set_wdr_expo_value1(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_expovalue1)
{
    u_isp_wdr_expovalue o_isp_wdr_expovalue1;

    o_isp_wdr_expovalue1.u32 = isp_be_regs->isp_wdr_expovalue.u32;
    o_isp_wdr_expovalue1.bits.isp_wdr_expovalue1 = isp_wdr_expovalue1;
    isp_be_regs->isp_wdr_expovalue.u32 = o_isp_wdr_expovalue1.u32;
}

static __inline td_void isp_drv_set_flick_exporatio0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_flick_exporatio)
{
    u_isp_flick_exporatio o_isp_flick_exporatio0;

    o_isp_flick_exporatio0.u32 = isp_be_regs->isp_flick_exporatio.u32;
    o_isp_flick_exporatio0.bits.isp_flick_exporatio = isp_flick_exporatio;
    isp_be_regs->isp_flick_exporatio.u32 = o_isp_flick_exporatio0.u32;
}

static __inline td_void isp_drv_set_wdr_max_ratio(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_maxratio)
{
    u_isp_wdr_maxratio o_isp_wdr_maxratio;

    o_isp_wdr_maxratio.u32 = isp_be_regs->isp_wdr_maxratio.u32;
    o_isp_wdr_maxratio.bits.isp_wdr_maxratio = isp_wdr_maxratio;
    isp_be_regs->isp_wdr_maxratio.u32 = o_isp_wdr_maxratio.u32;
}

static __inline td_void isp_drv_set_wdr_long_thr0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_long_thr)
{
    u_isp_wdr_wgtidx_thr o_isp_wdr_wgtidx_thr0;

    o_isp_wdr_wgtidx_thr0.u32 = isp_be_regs->isp_wdr_wgtidx_thr.u32;
    o_isp_wdr_wgtidx_thr0.bits.isp_wdr_long_thr  = isp_wdr_long_thr;
    isp_be_regs->isp_wdr_wgtidx_thr.u32 = o_isp_wdr_wgtidx_thr0.u32;
}

static __inline td_void isp_drv_set_wdr_short_thr0(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_short_thr)
{
    u_isp_wdr_wgtidx_thr o_isp_wdr_wgtidx_thr0;

    o_isp_wdr_wgtidx_thr0.u32 = isp_be_regs->isp_wdr_wgtidx_thr.u32;
    o_isp_wdr_wgtidx_thr0.bits.isp_wdr_short_thr  = isp_wdr_short_thr;
    isp_be_regs->isp_wdr_wgtidx_thr.u32 = o_isp_wdr_wgtidx_thr0.u32;
}

static __inline td_void isp_drv_set_wdr_mdt_en(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_mdt_en)
{
    u_isp_wdr_ctrl o_isp_wdr_ctrl;

    o_isp_wdr_ctrl.u32 = isp_be_regs->isp_wdr_ctrl.u32;
    o_isp_wdr_ctrl.bits.isp_wdr_mdt_en = isp_wdr_mdt_en;
    isp_be_regs->isp_wdr_ctrl.u32      = o_isp_wdr_ctrl.u32;
}

static __inline td_void isp_drv_set_wdr_fusion_mode(isp_pre_be_reg_type *isp_be_regs, td_u32 isp_wdr_fusion_mode)
{
    u_isp_wdr_ctrl o_isp_wdr_ctrl;

    o_isp_wdr_ctrl.u32 = isp_be_regs->isp_wdr_ctrl.u32;
    o_isp_wdr_ctrl.bits.isp_wdr_fusionmode = isp_wdr_fusion_mode;
    isp_be_regs->isp_wdr_ctrl.u32     = o_isp_wdr_ctrl.u32;
}

static __inline td_void isp_drv_set_fpn_offset(isp_pre_be_reg_type *isp_be_regs, td_u32 offset)
{
    u_isp_fpn_corr0 isp_fpn_corr0;

    isp_fpn_corr0.u32 = isp_be_regs->isp_fpn_corr0.u32;
    isp_fpn_corr0.bits.isp_fpn_offset0 = offset;
    isp_be_regs->isp_fpn_corr0.u32 = isp_fpn_corr0.u32;
}

static __inline td_void isp_drv_set_fpn_correct_en(isp_pre_be_reg_type *isp_be_regs, td_bool enable)
{
    u_isp_fpn_corr_cfg isp_fpn_corr_cfg;

    isp_fpn_corr_cfg.u32 = isp_be_regs->isp_fpn_corr_cfg.u32;
    isp_fpn_corr_cfg.bits.isp_fpn_correct0_en = enable;
    isp_be_regs->isp_fpn_corr_cfg.u32 = isp_fpn_corr_cfg.u32;
}

static __inline td_void isp_drv_set_ldci_stat_evratio(isp_post_be_reg_type *isp_be_regs, td_u32 isp_ldci_stat_evratio)
{
    u_isp_ldci_stat_evratio o_isp_ldci_stat_evratio;

    o_isp_ldci_stat_evratio.u32 = isp_be_regs->isp_ldci_stat_evratio.u32;
    o_isp_ldci_stat_evratio.bits.isp_ldci_stat_evratio = isp_ldci_stat_evratio;
    isp_be_regs->isp_ldci_stat_evratio.u32 = o_isp_ldci_stat_evratio.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma0(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_0)
{
    u_isp_adrc_prev_luma_0 o_isp_drc_prev_luma_0;

    o_isp_drc_prev_luma_0.u32 = isp_be_regs->isp_adrc_prev_luma_0.u32;
    o_isp_drc_prev_luma_0.bits.isp_adrc_prev_luma_0 = isp_drc_prev_luma_0;
    isp_be_regs->isp_adrc_prev_luma_0.u32 = o_isp_drc_prev_luma_0.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma1(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_1)
{
    u_isp_adrc_prev_luma_1 o_isp_drc_prev_luma_1;

    o_isp_drc_prev_luma_1.u32 = isp_be_regs->isp_adrc_prev_luma_1.u32;
    o_isp_drc_prev_luma_1.bits.isp_adrc_prev_luma_1 = isp_drc_prev_luma_1;
    isp_be_regs->isp_adrc_prev_luma_1.u32 = o_isp_drc_prev_luma_1.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma2(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_2)
{
    u_isp_adrc_prev_luma_2 o_isp_drc_prev_luma_2;

    o_isp_drc_prev_luma_2.u32 = isp_be_regs->isp_adrc_prev_luma_2.u32;
    o_isp_drc_prev_luma_2.bits.isp_adrc_prev_luma_2 = isp_drc_prev_luma_2;
    isp_be_regs->isp_adrc_prev_luma_2.u32 = o_isp_drc_prev_luma_2.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma3(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_3)
{
    u_isp_adrc_prev_luma_3 o_isp_drc_prev_luma_3;

    o_isp_drc_prev_luma_3.u32 = isp_be_regs->isp_adrc_prev_luma_3.u32;
    o_isp_drc_prev_luma_3.bits.isp_adrc_prev_luma_3 = isp_drc_prev_luma_3;
    isp_be_regs->isp_adrc_prev_luma_3.u32 = o_isp_drc_prev_luma_3.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma4(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_4)
{
    u_isp_adrc_prev_luma_4 o_isp_drc_prev_luma_4;

    o_isp_drc_prev_luma_4.u32 = isp_be_regs->isp_adrc_prev_luma_4.u32;
    o_isp_drc_prev_luma_4.bits.isp_adrc_prev_luma_4 = isp_drc_prev_luma_4;
    isp_be_regs->isp_adrc_prev_luma_4.u32 = o_isp_drc_prev_luma_4.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma5(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_5)
{
    u_isp_adrc_prev_luma_5 o_isp_drc_prev_luma_5;

    o_isp_drc_prev_luma_5.u32 = isp_be_regs->isp_adrc_prev_luma_5.u32;
    o_isp_drc_prev_luma_5.bits.isp_adrc_prev_luma_5 = isp_drc_prev_luma_5;
    isp_be_regs->isp_adrc_prev_luma_5.u32 = o_isp_drc_prev_luma_5.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma6(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_6)
{
    u_isp_adrc_prev_luma_6 o_isp_drc_prev_luma_6;

    o_isp_drc_prev_luma_6.u32 = isp_be_regs->isp_adrc_prev_luma_6.u32;
    o_isp_drc_prev_luma_6.bits.isp_adrc_prev_luma_6 = isp_drc_prev_luma_6;
    isp_be_regs->isp_adrc_prev_luma_6.u32 = o_isp_drc_prev_luma_6.u32;
}

static __inline td_void isp_drv_set_drc_prev_luma7(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_prev_luma_7)
{
    u_isp_adrc_prev_luma_7 o_isp_drc_prev_luma_7;

    o_isp_drc_prev_luma_7.u32 = isp_be_regs->isp_adrc_prev_luma_7.u32;
    o_isp_drc_prev_luma_7.bits.isp_adrc_prev_luma_7 = isp_drc_prev_luma_7;
    isp_be_regs->isp_adrc_prev_luma_7.u32 = o_isp_drc_prev_luma_7.u32;
}

static __inline td_void isp_drv_set_drc_shp_cfg(isp_post_be_reg_type *isp_be_regs,
                                                td_u32 isp_drc_shp_log_luma, td_u32 isp_drc_shp_exp_luma)
{
    u_isp_adrc_shp_cfg_luma o_isp_adrc_shp_cfg_luma;

    o_isp_adrc_shp_cfg_luma.u32 = isp_be_regs->isp_adrc_shp_cfg_luma.u32;
    o_isp_adrc_shp_cfg_luma.bits.isp_adrc_shp_log_luma = isp_drc_shp_log_luma;
    o_isp_adrc_shp_cfg_luma.bits.isp_adrc_shp_exp_luma = isp_drc_shp_exp_luma;
    isp_be_regs->isp_adrc_shp_cfg_luma.u32 = o_isp_adrc_shp_cfg_luma.u32;
}

static __inline td_void isp_drv_set_drc_div_denom_log(isp_post_be_reg_type *isp_be_regs,
                                                      td_u32 isp_drc_div_denom_log_luma)
{
    u_isp_adrc_div_denom_log_luma o_isp_adrc_div_denom_log_luma;

    o_isp_adrc_div_denom_log_luma.u32 = isp_be_regs->isp_adrc_div_denom_log_luma.u32;
    o_isp_adrc_div_denom_log_luma.bits.isp_adrc_div_denom_log_luma = isp_drc_div_denom_log_luma;
    isp_be_regs->isp_adrc_div_denom_log_luma.u32 = o_isp_adrc_div_denom_log_luma.u32;
}

static __inline td_void isp_drv_set_drc_denom_exp(isp_post_be_reg_type *isp_be_regs, td_u32 isp_drc_denom_exp)
{
    u_isp_adrc_denom_exp_luma o_isp_adrc_denom_exp_luma;

    o_isp_adrc_denom_exp_luma.u32 = isp_be_regs->isp_adrc_denom_exp_luma.u32;
    o_isp_adrc_denom_exp_luma.bits.isp_adrc_denom_exp_luma = isp_drc_denom_exp;
    isp_be_regs->isp_adrc_denom_exp_luma.u32 = o_isp_adrc_denom_exp_luma.u32;
}

static __inline td_void isp_drv_set_drc_detail_rd_en(isp_post_be_reg_type *be_reg, td_u32 isp_drc_detail_rd_en)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_detail_rd_en = isp_drc_detail_rd_en;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_statformat(isp_post_be_reg_type *be_reg, td_u32 isp_drc_statformat)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_statformat = isp_drc_statformat;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_statflttype(isp_post_be_reg_type *be_reg, td_u32 isp_drc_statflttype)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_statflttype = isp_drc_statflttype;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_inadapt_mode(isp_post_be_reg_type *be_reg, td_u32 isp_drc_inadapt_mode)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_inadapt_mode = isp_drc_inadapt_mode;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_darkminl(isp_post_be_reg_type *be_reg, td_u32 isp_drc_darkminl)
{
    u_isp_adrc_bld_darkl o_isp_adrc_bld_darkl;
    o_isp_adrc_bld_darkl.u32 = be_reg->isp_adrc_bld_darkl.u32;
    o_isp_adrc_bld_darkl.bits.isp_adrc_darkminl = isp_drc_darkminl;
    be_reg->isp_adrc_bld_darkl.u32 = o_isp_adrc_bld_darkl.u32;
}

static __inline td_void isp_drv_set_drc_darkmaxl(isp_post_be_reg_type *be_reg, td_u32 isp_drc_darkmaxl)
{
    u_isp_adrc_bld_darkl o_isp_adrc_bld_darkl;
    o_isp_adrc_bld_darkl.u32 = be_reg->isp_adrc_bld_darkl.u32;
    o_isp_adrc_bld_darkl.bits.isp_adrc_darkmaxl = isp_drc_darkmaxl;
    be_reg->isp_adrc_bld_darkl.u32 = o_isp_adrc_bld_darkl.u32;
}

static __inline td_void isp_drv_set_drc_brightminl(isp_post_be_reg_type *be_reg, td_u32 isp_drc_brightminl)
{
    u_isp_adrc_bld_brightl o_isp_adrc_bld_brightl;
    o_isp_adrc_bld_brightl.u32 = be_reg->isp_adrc_bld_brightl.u32;
    o_isp_adrc_bld_brightl.bits.isp_adrc_brightminl = isp_drc_brightminl;
    be_reg->isp_adrc_bld_brightl.u32 = o_isp_adrc_bld_brightl.u32;
}

static __inline td_void isp_drv_set_drc_brightmaxl(isp_post_be_reg_type *be_reg, td_u32 isp_drc_brightmaxl)
{
    u_isp_adrc_bld_brightl o_isp_adrc_bld_brightl;
    o_isp_adrc_bld_brightl.u32 = be_reg->isp_adrc_bld_brightl.u32;
    o_isp_adrc_bld_brightl.bits.isp_adrc_brightmaxl = isp_drc_brightmaxl;
    be_reg->isp_adrc_bld_brightl.u32 = o_isp_adrc_bld_brightl.u32;
}

static __inline td_void isp_drv_set_drc_darkmind(isp_post_be_reg_type *be_reg, td_u32 isp_drc_darkmind)
{
    u_isp_adrc_bld_darkd o_isp_adrc_bld_darkd;
    o_isp_adrc_bld_darkd.u32 = be_reg->isp_adrc_bld_darkd.u32;
    o_isp_adrc_bld_darkd.bits.isp_adrc_darkmind = isp_drc_darkmind;
    be_reg->isp_adrc_bld_darkd.u32 = o_isp_adrc_bld_darkd.u32;
}

static __inline td_void isp_drv_set_drc_darkmaxd(isp_post_be_reg_type *be_reg, td_u32 isp_drc_darkmaxd)
{
    u_isp_adrc_bld_darkd o_isp_adrc_bld_darkd;
    o_isp_adrc_bld_darkd.u32 = be_reg->isp_adrc_bld_darkd.u32;
    o_isp_adrc_bld_darkd.bits.isp_adrc_darkmaxd = isp_drc_darkmaxd;
    be_reg->isp_adrc_bld_darkd.u32 = o_isp_adrc_bld_darkd.u32;
}

static __inline td_void isp_drv_set_drc_brightmind(isp_post_be_reg_type *be_reg, td_u32 isp_drc_brightmind)
{
    u_isp_adrc_bld_brightd o_isp_adrc_bld_brightd;
    o_isp_adrc_bld_brightd.u32 = be_reg->isp_adrc_bld_brightd.u32;
    o_isp_adrc_bld_brightd.bits.isp_adrc_brightmind = isp_drc_brightmind;
    be_reg->isp_adrc_bld_brightd.u32 = o_isp_adrc_bld_brightd.u32;
}

static __inline td_void isp_drv_set_drc_brightmaxd(isp_post_be_reg_type *be_reg, td_u32 isp_drc_brightmaxd)
{
    u_isp_adrc_bld_brightd o_isp_adrc_bld_brightd;
    o_isp_adrc_bld_brightd.u32 = be_reg->isp_adrc_bld_brightd.u32;
    o_isp_adrc_bld_brightd.bits.isp_adrc_brightmaxd = isp_drc_brightmaxd;
    be_reg->isp_adrc_bld_brightd.u32 = o_isp_adrc_bld_brightd.u32;
}

static __inline td_void isp_drv_set_drc_ltm_gain_coef(isp_post_be_reg_type *be_reg, td_u32 isp_drc_ltm_gain_coef)
{
    u_isp_adrc_gain_coef o_isp_adrc_gain_coef;
    o_isp_adrc_gain_coef.u32 = be_reg->isp_adrc_gain_coef.u32;
    o_isp_adrc_gain_coef.bits.isp_adrc_ltm_gain_coef = isp_drc_ltm_gain_coef;
    be_reg->isp_adrc_gain_coef.u32 = o_isp_adrc_gain_coef.u32;
}

static __inline td_void isp_drv_set_drc_gtm_gain_coef(isp_post_be_reg_type *be_reg, td_u32 isp_drc_gtm_gain_coef)
{
    u_isp_adrc_gain_coef o_isp_adrc_gain_coef;
    o_isp_adrc_gain_coef.u32 = be_reg->isp_adrc_gain_coef.u32;
    o_isp_adrc_gain_coef.bits.isp_adrc_gtm_gain_coef = isp_drc_gtm_gain_coef;
    be_reg->isp_adrc_gain_coef.u32 = o_isp_adrc_gain_coef.u32;
}

static __inline td_void isp_drv_set_drc_hnum(isp_post_be_reg_type *be_reg, td_u32 isp_drc_hnum)
{
    u_isp_adrc_zone o_isp_adrc_zone;
    o_isp_adrc_zone.u32 = be_reg->isp_adrc_zone.u32;
    o_isp_adrc_zone.bits.isp_adrc_hnum = isp_drc_hnum;
    be_reg->isp_adrc_zone.u32 = o_isp_adrc_zone.u32;
}

static __inline td_void isp_drv_set_drc_vnum(isp_post_be_reg_type *be_reg, td_u32 isp_drc_vnum)
{
    u_isp_adrc_zone o_isp_adrc_zone;
    o_isp_adrc_zone.u32 = be_reg->isp_adrc_zone.u32;
    o_isp_adrc_zone.bits.isp_adrc_vnum = isp_drc_vnum;
    be_reg->isp_adrc_zone.u32 = o_isp_adrc_zone.u32;
}

static __inline td_void isp_drv_set_drc_zone_hsize(isp_post_be_reg_type *be_reg, td_u32 isp_drc_zone_hsize)
{
    u_isp_adrc_zone_size o_isp_adrc_zone_size;
    o_isp_adrc_zone_size.u32 = be_reg->isp_adrc_zone_size.u32;
    o_isp_adrc_zone_size.bits.isp_adrc_zone_hsize = isp_drc_zone_hsize;
    be_reg->isp_adrc_zone_size.u32 = o_isp_adrc_zone_size.u32;
}

static __inline td_void isp_drv_set_drc_chk_x(isp_post_be_reg_type *be_reg, td_u32 isp_drc_chk_x)
{
    u_isp_adrc_zone_size o_isp_adrc_zone_size;
    o_isp_adrc_zone_size.u32 = be_reg->isp_adrc_zone_size.u32;
    o_isp_adrc_zone_size.bits.isp_adrc_chk_x = isp_drc_chk_x;
    be_reg->isp_adrc_zone_size.u32 = o_isp_adrc_zone_size.u32;
}

static __inline td_void isp_drv_set_drc_zone_vsize(isp_post_be_reg_type *be_reg, td_u32 isp_drc_zone_vsize)
{
    u_isp_adrc_zone_size o_isp_adrc_zone_size;
    o_isp_adrc_zone_size.u32 = be_reg->isp_adrc_zone_size.u32;
    o_isp_adrc_zone_size.bits.isp_adrc_zone_vsize = isp_drc_zone_vsize;
    be_reg->isp_adrc_zone_size.u32 = o_isp_adrc_zone_size.u32;
}

static __inline td_void isp_drv_set_drc_chk_y(isp_post_be_reg_type *be_reg, td_u32 isp_drc_chk_y)
{
    u_isp_adrc_zone_size o_isp_adrc_zone_size;
    o_isp_adrc_zone_size.u32 = be_reg->isp_adrc_zone_size.u32;
    o_isp_adrc_zone_size.bits.isp_adrc_chk_y = isp_drc_chk_y;
    be_reg->isp_adrc_zone_size.u32 = o_isp_adrc_zone_size.u32;
}

static __inline td_void isp_drv_set_drc_div_x0(isp_post_be_reg_type *be_reg, td_u32 isp_drc_div_x0)
{
    u_isp_adrc_zone_div0 o_isp_adrc_zone_div0;
    o_isp_adrc_zone_div0.u32 = be_reg->isp_adrc_zone_div0.u32;
    o_isp_adrc_zone_div0.bits.isp_adrc_div_x0 = isp_drc_div_x0;
    be_reg->isp_adrc_zone_div0.u32 = o_isp_adrc_zone_div0.u32;
}

static __inline td_void isp_drv_set_drc_div_y0(isp_post_be_reg_type *be_reg, td_u32 isp_drc_div_y0)
{
    u_isp_adrc_zone_div0 o_isp_adrc_zone_div0;
    o_isp_adrc_zone_div0.u32 = be_reg->isp_adrc_zone_div0.u32;
    o_isp_adrc_zone_div0.bits.isp_adrc_div_y0 = isp_drc_div_y0;
    be_reg->isp_adrc_zone_div0.u32 = o_isp_adrc_zone_div0.u32;
}

static __inline td_void isp_drv_set_drc_div_x1(isp_post_be_reg_type *be_reg, td_u32 isp_drc_div_x1)
{
    u_isp_adrc_zone_div1 o_isp_adrc_zone_div1;
    o_isp_adrc_zone_div1.u32 = be_reg->isp_adrc_zone_div1.u32;
    o_isp_adrc_zone_div1.bits.isp_adrc_div_x1 = isp_drc_div_x1;
    be_reg->isp_adrc_zone_div1.u32 = o_isp_adrc_zone_div1.u32;
}

static __inline td_void isp_drv_set_drc_div_y1(isp_post_be_reg_type *be_reg, td_u32 isp_drc_div_y1)
{
    u_isp_adrc_zone_div1 o_isp_adrc_zone_div1;
    o_isp_adrc_zone_div1.u32 = be_reg->isp_adrc_zone_div1.u32;
    o_isp_adrc_zone_div1.bits.isp_adrc_div_y1 = isp_drc_div_y1;
    be_reg->isp_adrc_zone_div1.u32 = o_isp_adrc_zone_div1.u32;
}

static __inline td_void isp_drv_set_drc_wrstat_en(isp_post_be_reg_type *be_reg, td_u32 isp_drc_wrstat_en)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_wrstat_en = isp_drc_wrstat_en;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_rdstat_en(isp_post_be_reg_type *be_reg, td_u32 isp_drc_rdstat_en)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_rdstat_en = isp_drc_rdstat_en;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_drc_vbiflt_en(isp_post_be_reg_type *be_reg, td_u32 isp_drc_vbiflt_en)
{
    u_isp_adrc_cfg o_isp_adrc_cfg;
    o_isp_adrc_cfg.u32 = be_reg->isp_adrc_cfg.u32;
    o_isp_adrc_cfg.bits.isp_adrc_vbiflt_en = isp_drc_vbiflt_en;
    be_reg->isp_adrc_cfg.u32 = o_isp_adrc_cfg.u32;
}

static __inline td_void isp_drv_set_be_regup(isp_viproc_reg_type *viproc_reg, td_u32 reg_up)
{
    u_isp_viproc_ispbe_regup o_viproc_regup;
    o_viproc_regup.u32 = viproc_reg->viproc_ispbe_regup.u32;
    o_viproc_regup.bits.ispbe_reg_up = reg_up;
    viproc_reg->viproc_ispbe_regup.u32 = o_viproc_regup.u32;
}

/* Fe Blc */
static __inline td_void isp_drv_fe_global_blc_mode_write(isp_fe_reg_type *fe_reg, td_u32 uisp_glb_blc_mode)
{
    u_isp_glb_blc_cfg o_isp_glb_blc_cfg;
    o_isp_glb_blc_cfg.u32 = fe_reg->isp_glb_blc_cfg.u32;
    o_isp_glb_blc_cfg.bits.isp_glb_blc_mode = uisp_glb_blc_mode;
    fe_reg->isp_glb_blc_cfg.u32 = o_isp_glb_blc_cfg.u32;
}
static __inline td_void isp_drv_fe_global_target_mode_write(isp_fe_reg_type *fe_reg, td_u32 uisp_glb_tg_mode)
{
    u_isp_glb_blc_cfg o_isp_glb_blc_cfg;
    o_isp_glb_blc_cfg.u32 = fe_reg->isp_glb_blc_cfg.u32;
    o_isp_glb_blc_cfg.bits.isp_glb_tg_mode = uisp_glb_tg_mode;
    fe_reg->isp_glb_blc_cfg.u32 = o_isp_glb_blc_cfg.u32;
}

static __inline td_void isp_drv_fe_global_blc_write(isp_fe_reg_type *fe_reg, const td_u16 *blc)
{
    fe_reg->isp_blc_cfg10.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_blc_cfg11.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_target_blc_write(isp_fe_reg_type *fe_reg, const td_u16 *blc)
{
    fe_reg->isp_blc_cfg20.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_blc_cfg21.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_dg_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *dg_blc)
{
    fe_reg->isp_dg2_blc_offset1.u32 = ((td_u32)dg_blc[OT_ISP_CHN_R] << 16) + dg_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_dg2_blc_offset2.u32 = ((td_u32)dg_blc[OT_ISP_CHN_B] << 16) + dg_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_dg_real_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *blc)
{
    fe_reg->isp_dg2_real_blc_offset1.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_dg2_real_blc_offset2.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_nr_dg_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *dg_blc)
{
    fe_reg->isp_dg1_blc_offset1.u32 = ((td_u32)dg_blc[OT_ISP_CHN_R] << 16) + dg_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_dg1_blc_offset2.u32 = ((td_u32)dg_blc[OT_ISP_CHN_B] << 16) + dg_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_nr_dg_real_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *blc)
{
    fe_reg->isp_dg1_real_blc_offset1.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_dg1_real_blc_offset2.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_wb_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *wb_blc)
{
    fe_reg->isp_wb1_blc_offset1.u32 = ((td_u32)wb_blc[OT_ISP_CHN_R] << 16) + wb_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    fe_reg->isp_wb1_blc_offset2.u32 = ((td_u32)wb_blc[OT_ISP_CHN_B] << 16) + wb_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_ae_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *ae_blc)
{
    fe_reg->isp_ae_offset_r_gr.u32 = ((td_u32)ae_blc[OT_ISP_CHN_GR] << 16) + ae_blc[OT_ISP_CHN_R]; /* bit16~30: gr */
    fe_reg->isp_ae_offset_gb_b.u32 = ((td_u32)ae_blc[OT_ISP_CHN_B] << 16) + ae_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_fe_af_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *af_blc)
{
    fe_reg->isp_af_offset.u32 = ((td_u32)af_blc[OT_ISP_CHN_GB] << 16) + af_blc[OT_ISP_CHN_GR]; /* bit16~30: gb */
}

static __inline td_void isp_drv_fe_blc1_en_write(isp_fe_reg_type *fe_reg, td_u32 uisp_blc1_en)
{
    u_isp_fe_ctrl o_isp_fe_ctrl;
    o_isp_fe_ctrl.u32 = fe_reg->isp_fe_ctrl.u32;
    o_isp_fe_ctrl.bits.isp_blc1_en = uisp_blc1_en;
    fe_reg->isp_fe_ctrl.u32 = o_isp_fe_ctrl.u32;
}
static __inline td_void isp_drv_fe_blc_offset_write(isp_fe_reg_type *fe_reg, const td_u16 *fe_blc)
{
    fe_reg->isp_blc1_offset1.u32 = ((td_u32)fe_blc[OT_ISP_CHN_R] << 16) + fe_blc[OT_ISP_CHN_GR]; /* bit16~30: r_blc */
    fe_reg->isp_blc1_offset2.u32 = ((td_u32)fe_blc[OT_ISP_CHN_B] << 16) + fe_blc[OT_ISP_CHN_GB]; /* bit16~30: b_blc */
}

static __inline td_void isp_drv_be_global_blc_mode_write(isp_be_reg_type *be_reg, const td_u32 uisp_glb_blc_mode)
{
    u_isp_glb_blc_cfg o_isp_glb_blc_cfg;
    o_isp_glb_blc_cfg.u32 = be_reg->isp_glb_blc_cfg.u32;
    o_isp_glb_blc_cfg.bits.isp_glb_blc_mode = uisp_glb_blc_mode;
    be_reg->isp_glb_blc_cfg.u32 = o_isp_glb_blc_cfg.u32;
}

static __inline td_void isp_drv_be_global_target_mode_write(isp_be_reg_type *be_reg, const td_u32 uisp_glb_tg_mode)
{
    u_isp_glb_blc_cfg o_isp_glb_blc_cfg;
    o_isp_glb_blc_cfg.u32 = be_reg->isp_glb_blc_cfg.u32;
    o_isp_glb_blc_cfg.bits.isp_glb_tg_mode = uisp_glb_tg_mode;
    be_reg->isp_glb_blc_cfg.u32 = o_isp_glb_blc_cfg.u32;
}

static __inline td_void isp_drv_be_global_blc0_write(isp_pre_be_reg_type *be_reg, const td_u16 *blc)
{
    be_reg->isp_blc_cfg00.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r_blc */
    be_reg->isp_blc_cfg01.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b_blc */
}

static __inline td_void isp_drv_be_global_blc1_write(isp_be_reg_type *be_reg, const td_u16 *blc)
{
    be_reg->isp_blc_cfg10.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    be_reg->isp_blc_cfg11.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_be_target_black_level(isp_be_reg_type *be_reg, const td_u16 *tg_blc)
{
    be_reg->isp_blc_cfg20.u32 = ((td_u32)tg_blc[OT_ISP_CHN_R] << 16) + tg_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    be_reg->isp_blc_cfg21.u32 = ((td_u32)tg_blc[OT_ISP_CHN_B] << 16) + tg_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_be_f0_4dg_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f0_4dg_blc)
{
    be_reg->isp_4dg_0_blc_offset1.u32 = ((td_u32)f0_4dg_blc[OT_ISP_CHN_R] << 16) + /* bit16~30: r_blc */
                                        f0_4dg_blc[OT_ISP_CHN_GR];

    be_reg->isp_4dg_0_blc_offset2.u32 = ((td_u32)f0_4dg_blc[OT_ISP_CHN_B] << 16) + /* bit16~30: b_blc */
                                        f0_4dg_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_f1_4dg_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f1_4dg_blc)
{
    be_reg->isp_4dg_1_blc_offset1.u32 = ((td_u32)f1_4dg_blc[OT_ISP_CHN_R] << 16) + /* bit16~30: r_blc */
                                        f1_4dg_blc[OT_ISP_CHN_GR];

    be_reg->isp_4dg_1_blc_offset2.u32 = ((td_u32)f1_4dg_blc[OT_ISP_CHN_B] << 16) + /* bit16~30: b_blc */
                                        f1_4dg_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_f2_4dg_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f2_4dg_blc)
{
    be_reg->isp_4dg_2_blc_offset1.u32 = ((td_u32)f2_4dg_blc[OT_ISP_CHN_R] << 16) + /* bit16~30: r_blc */
                                        f2_4dg_blc[OT_ISP_CHN_GR];

    be_reg->isp_4dg_2_blc_offset2.u32 = ((td_u32)f2_4dg_blc[OT_ISP_CHN_B] << 16) + /* bit16~30: b_blc */
                                        f2_4dg_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_f3_4dg_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f3_4dg_blc)
{
    be_reg->isp_4dg_3_blc_offset1.u32 = ((td_u32)f3_4dg_blc[OT_ISP_CHN_R] << 16) + /* bit16~30: r_blc */
                                        f3_4dg_blc[OT_ISP_CHN_GR];

    be_reg->isp_4dg_3_blc_offset2.u32 = ((td_u32)f3_4dg_blc[OT_ISP_CHN_B] << 16) + /* bit16~30: b_blc */
                                        f3_4dg_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_ge0_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *ge0_blc)
{
    be_reg->isp_ge0_blc_offset.u32 = ((td_u32)ge0_blc[OT_ISP_CHN_GR] << 16) + /* bit16~30: gr_blc */
                                        ge0_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_ge1_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *ge1_blc)
{
    be_reg->isp_ge1_blc_offset.u32 = ((td_u32)ge1_blc[OT_ISP_CHN_GR] << 16) + /* bit16~30: gr_blc */
                                        ge1_blc[OT_ISP_CHN_GB];
}

static __inline td_void isp_drv_be_f0_wdr_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f0_wdr_blc)
{
    be_reg->isp_wdr_f0_inblc0.u32 = ((td_u32)f0_wdr_blc[OT_ISP_CHN_R] << 16) +  /* bit16~30: r_blc */
                                    f0_wdr_blc[OT_ISP_CHN_GR];

    be_reg->isp_wdr_f0_inblc1.u32 = ((td_u32)f0_wdr_blc[OT_ISP_CHN_GB] << 16) + /* bit16~30: gb_blc */
                                    f0_wdr_blc[OT_ISP_CHN_B];
}

static __inline td_void isp_drv_be_f1_wdr_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f1_wdr_blc)
{
    be_reg->isp_wdr_f1_inblc0.u32 = ((td_u32)f1_wdr_blc[OT_ISP_CHN_R] << 16) +  /* bit16~30: r_blc */
                                    f1_wdr_blc[OT_ISP_CHN_GR];

    be_reg->isp_wdr_f1_inblc1.u32 = ((td_u32)f1_wdr_blc[OT_ISP_CHN_GB] << 16) + /* bit16~30: gb_blc */
                                    f1_wdr_blc[OT_ISP_CHN_B];
}

static __inline td_void isp_drv_be_f0_flick_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f0_flick_blc)
{
    be_reg->isp_flick_f0_inblc0.u32 = ((td_u32)f0_flick_blc[OT_ISP_CHN_R] << 16) +  /* bit16~30: r_blc */
                                      f0_flick_blc[OT_ISP_CHN_GR];

    be_reg->isp_flick_f0_inblc1.u32 = ((td_u32)f0_flick_blc[OT_ISP_CHN_GB] << 16) + /* bit16~30: gb_blc */
                                      f0_flick_blc[OT_ISP_CHN_B];
}

static __inline td_void isp_drv_be_f1_flick_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *f1_flick_blc)
{
    be_reg->isp_flick_f1_inblc0.u32 = ((td_u32)f1_flick_blc[OT_ISP_CHN_R] << 16) +  /* bit16~30: r_blc */
                                      f1_flick_blc[OT_ISP_CHN_GR];

    be_reg->isp_flick_f1_inblc1.u32 = ((td_u32)f1_flick_blc[OT_ISP_CHN_GB] << 16) + /* bit16~30: gb_blc */
                                      f1_flick_blc[OT_ISP_CHN_B];
}

static __inline td_void isp_drv_be_lsc_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *lsc_blc)
{
    be_reg->isp_lsc_blc0.u32 = ((td_u32)lsc_blc[OT_ISP_CHN_GR] << 16) + lsc_blc[OT_ISP_CHN_R]; /* bit16~30: gr_blc */
    be_reg->isp_lsc_blc1.u32 = ((td_u32)lsc_blc[OT_ISP_CHN_GB] << 16) + lsc_blc[OT_ISP_CHN_B]; /* bit16~30: gb_blc */
}

static __inline td_void isp_drv_be_dgain_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *dg_blc)
{
    be_reg->isp_dg_blc_offset1.u32 = ((td_u32)dg_blc[OT_ISP_CHN_R] << 16) + dg_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    be_reg->isp_dg_blc_offset2.u32 = ((td_u32)dg_blc[OT_ISP_CHN_B] << 16) + dg_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_be_dgain_real_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *blc)
{
    be_reg->isp_dg_real_blc_offset1.u32 = ((td_u32)blc[OT_ISP_CHN_R] << 16) + blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    be_reg->isp_dg_real_blc_offset2.u32 = ((td_u32)blc[OT_ISP_CHN_B] << 16) + blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_be_wb_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *wb_blc)
{
    be_reg->isp_wb_blc_offset1.u32  = ((td_u32)wb_blc[OT_ISP_CHN_R] << 16) + wb_blc[OT_ISP_CHN_GR]; /* bit16~30: r */
    be_reg->isp_wb_blc_offset2.u32  = ((td_u32)wb_blc[OT_ISP_CHN_B] << 16) + wb_blc[OT_ISP_CHN_GB]; /* bit16~30: b */
}

static __inline td_void isp_drv_be_expander_offset_write(isp_pre_be_reg_type *be_reg, const td_u16 *expander_blc)
{
    be_reg->isp_expander_offsetr.u32  = expander_blc[OT_ISP_CHN_R];
    be_reg->isp_expander_offsetgr.u32 = expander_blc[OT_ISP_CHN_GR];
    be_reg->isp_expander_offsetgb.u32 = expander_blc[OT_ISP_CHN_GB];
    be_reg->isp_expander_offsetb.u32  = expander_blc[OT_ISP_CHN_B];
}

static __inline td_void isp_drv_be_format_write(isp_post_be_reg_type *be_reg, isp_be_format be_format)
{
    u_isp_be_format o_isp_be_format;
    o_isp_be_format.u32 = be_reg->isp_be_format.u32;
    o_isp_be_format.bits.isp_format = be_format;
    be_reg->isp_be_format.u32 = o_isp_be_format.u32;
}

/* just called by isp_drv_reg_config_chn_sel */
static td_void isp_drv_reg_config_chn_sel_subfunc0(td_u32 channel_sel, const isp_drv_ctx *drv_ctx,  td_u32 *chn_switch)
{
    switch (channel_sel & 0x3) {
        case ISP_CHN_SWITCH_NORMAL:
            chn_switch[0] = (drv_ctx->sync_cfg.vc_num_max - drv_ctx->sync_cfg.vc_num) %
                            div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
            chn_switch[1] = (chn_switch[0] + 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 1 */
            chn_switch[2] = (chn_switch[0] + 2) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 2 */
            chn_switch[3] = (chn_switch[0] + 3) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 3 */
            break;

        case ISP_CHN_SWITCH_2LANE:
            chn_switch[1] = (drv_ctx->sync_cfg.vc_num_max - drv_ctx->sync_cfg.vc_num) %
                            div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
            chn_switch[0] = (chn_switch[1] + 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 0 & 1 */
            chn_switch[2] = (chn_switch[1] + 2) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 2 & 1 */
            chn_switch[3] = (chn_switch[1] + 3) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 3 & 1 */
            break;

        case ISP_CHN_SWITCH_3LANE:
            chn_switch[2] = (drv_ctx->sync_cfg.vc_num_max - drv_ctx->sync_cfg.vc_num) %  /* index 2 */
                            div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
            chn_switch[1] = (chn_switch[2] + 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 1 & 2 */
            chn_switch[0] = (chn_switch[2] + 2) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 0 & 2 */
            chn_switch[3] = (chn_switch[2] + 3) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 3 & 2 */
            break;

        case ISP_CHN_SWITCH_4LANE:
            chn_switch[3] = (drv_ctx->sync_cfg.vc_num_max - drv_ctx->sync_cfg.vc_num) %  /* index 3 */
                            div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
            chn_switch[2] = (chn_switch[3] + 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 2 & 3+1 */
            chn_switch[1] = (chn_switch[3] + 2) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 1 & 3+2 */
            chn_switch[0] = (chn_switch[3] + 3) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1); /* index 0 & 3+3 */
            break;
        default:
            break;
    }
}

static td_void isp_drv_reg_config_chn_sel_subfunc1(td_u32 channel_sel, isp_drv_ctx *drv_ctx,
    td_u32 *chn_switch, td_u8 length)
{
    ot_unused(length);

    switch (channel_sel & 0x3) {
        case ISP_CHN_SWITCH_NORMAL:
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 0 */
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1]; /* array index 1 */
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 2 */
            chn_switch[3] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[3]; /* array index 3 */
            break;

        case ISP_CHN_SWITCH_2LANE:
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 1 */
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1]; /* array index 0 */
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 2 */
            chn_switch[3] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[3]; /* array index 3 */
            break;

        case ISP_CHN_SWITCH_3LANE:
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 2 */
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1]; /* array index 1 */
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 0 & 2 */
            chn_switch[3] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[3]; /* array index 3 */
            break;

        case ISP_CHN_SWITCH_4LANE:
            chn_switch[3] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[0]; /* array index 3 */
            chn_switch[2] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[1]; /* array index 2 */
            chn_switch[1] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[2]; /* array index 1 & 2 */
            chn_switch[0] = drv_ctx->chn_sel_attr[0].wdr_chn_sel[3]; /* array index 0 & 3 */
            break;
        default:
            break;
    }
}

static td_void isp_drv_set_pre_be_input_sel(ot_vi_pipe vi_pipe, const td_u32 *input_sel, td_u8 length)
{
    td_u8 k, blk_dev;
    td_s32 ret;
    isp_pre_be_reg_type *be_reg[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    isp_drv_ctx *drv_ctx = TD_NULL;
    ot_unused(length);

    ret = isp_drv_get_pre_be_reg_virt_addr(vi_pipe, be_reg);
    if (ret != TD_SUCCESS) {
        return;
    }
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_dev = drv_ctx->work_mode.block_dev;

    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        isp_drv_input_sel_write(be_reg[k + blk_dev], input_sel);
    }
    return;
}

td_void isp_drv_set_input_sel(ot_vi_pipe vi_pipe, td_u32 *input_sel, td_u8 length)
{
    isp_drv_set_pre_be_input_sel(vi_pipe, input_sel, length);

    return;
}

td_s32 isp_drv_reg_config_chn_sel(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u32  chn_switch[ISP_CHN_SWITCH_NUM]  = {0};
    td_u32  channel_sel;

    isp_check_pointer_return(drv_ctx);

    channel_sel = drv_ctx->chn_sel_attr[0].channel_sel;
    chn_switch[4] = (drv_ctx->yuv_mode == TD_TRUE) ? 1 : 0; /* array index 4 */

    if (is_full_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        isp_drv_reg_config_chn_sel_subfunc0(channel_sel, drv_ctx, &chn_switch[0]);
        isp_drv_set_input_sel(vi_pipe, &chn_switch[0], sizeof(chn_switch));
    } else if ((is_line_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) ||
               (is_half_wdr_mode(drv_ctx->sync_cfg.wdr_mode))) {
        isp_drv_reg_config_chn_sel_subfunc1(channel_sel, drv_ctx, &chn_switch[0], sizeof(chn_switch));

        /* offline mode: isp BE buffer poll, so chn switch need each frame refres */
        if (is_offline_mode(drv_ctx->work_mode.running_mode) ||
            (is_striping_mode(drv_ctx->work_mode.running_mode)) ||
            (is_pre_online_post_offline(drv_ctx->work_mode.running_mode))) {
            isp_drv_set_input_sel(vi_pipe, &chn_switch[0], sizeof(chn_switch));
        }
    } else {
    }

    return TD_SUCCESS;
}

static td_void isp_drv_set_pre_be_blc_reg(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_pre_be_reg_type *pre_be,
    const isp_be_blc_dyna_cfg *be_blc_reg_cfg)
{
    if (drv_ctx->dyna_blc_info.is_runbe_first_frame == TD_TRUE &&
        (isp_drv_get_run_wakeup_sel(vi_pipe) == OT_ISP_RUN_WAKEUP_BE_END)) {
        drv_ctx->dyna_blc_info.is_runbe_first_frame = TD_FALSE;
        return;
    }

    isp_drv_be_global_blc_mode_write(pre_be, be_blc_reg_cfg->global_blc.blc_mode);
    isp_drv_be_global_target_mode_write(pre_be, be_blc_reg_cfg->global_blc.target_mode);
    isp_drv_be_global_blc0_write(pre_be, be_blc_reg_cfg->global_blc.global_blc0[0]);
    isp_drv_be_global_blc1_write(pre_be, be_blc_reg_cfg->global_blc.global_blc1[0]);
    isp_drv_be_target_black_level(pre_be, be_blc_reg_cfg->global_blc.target_blc[0]);
    /* 4dg */
    isp_drv_be_f0_4dg_offset_write(pre_be, be_blc_reg_cfg->wdr_dg_blc[0].blc); /* wdr0 */
    isp_drv_be_f1_4dg_offset_write(pre_be, be_blc_reg_cfg->wdr_dg_blc[1].blc); /* wdr1 */
    isp_drv_be_f2_4dg_offset_write(pre_be, be_blc_reg_cfg->wdr_dg_blc[2].blc); /* wdr2 */
    isp_drv_be_f3_4dg_offset_write(pre_be, be_blc_reg_cfg->wdr_dg_blc[3].blc); /* wdr3 */

    /* ge */
    isp_drv_be_ge0_offset_write(pre_be, be_blc_reg_cfg->ge_blc[0].blc); /* wdr0 */
    isp_drv_be_ge1_offset_write(pre_be, be_blc_reg_cfg->ge_blc[1].blc); /* wdr1 */

    /* wdr */
    isp_drv_be_f0_wdr_offset_write(pre_be, be_blc_reg_cfg->wdr_blc[0].blc); /* wdr0 */
    isp_drv_be_f1_wdr_offset_write(pre_be, be_blc_reg_cfg->wdr_blc[1].blc); /* wdr1 */

    /* flicker */
    isp_drv_be_f0_flick_offset_write(pre_be, be_blc_reg_cfg->flicker_blc[0].blc); /* wdr0 */
    isp_drv_be_f1_flick_offset_write(pre_be, be_blc_reg_cfg->flicker_blc[1].blc); /* wdr1 */
    isp_drv_be_expander_offset_write(pre_be, be_blc_reg_cfg->expander_blc.blc); /* expander */

    isp_drv_be_lsc_offset_write(pre_be, be_blc_reg_cfg->lsc_blc.blc); /* lsc */
    isp_drv_be_dgain_offset_write(pre_be, be_blc_reg_cfg->dg_blc.blc); /* isp_dgain */
    isp_drv_be_dgain_real_offset_write(pre_be, be_blc_reg_cfg->dg_real_blc.blc);
    isp_drv_be_wb_offset_write(pre_be, be_blc_reg_cfg->wb_blc.blc); /* wb */
}

/* dynamic mode: read ob stats and cfg fe_blc in frame_delay or frame_end interrupt */
static td_void isp_drv_calc_noarmal_blc_delay(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u8 fe_cfg_node_idx)
{
    ot_isp_black_level_mode pre_black_level_mode, cur_black_level_mode;
    isp_sync_cfg_buf_node *node0 = TD_NULL;
    isp_sync_cfg_buf_node *node1 = TD_NULL;
    node0 = drv_ctx->sync_cfg.node[0];
    node1 = drv_ctx->sync_cfg.node[1];
    isp_check_pointer_void_return(node0);
    cur_black_level_mode = node0->dynamic_blc_cfg.black_level_mode;
    if (node1 != TD_NULL) {
        pre_black_level_mode = node1->dynamic_blc_cfg.black_level_mode;
    } else {
        pre_black_level_mode = cur_black_level_mode;
    }

    if ((pre_black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) &&
        (cur_black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC)) {
        drv_ctx->dyna_blc_info.fe_blc_mode_change      = TD_TRUE;
        drv_ctx->dyna_blc_info.pre_be_blc_mode_change  = TD_TRUE;
        drv_ctx->dyna_blc_info.post_be_blc_mode_change = TD_TRUE;
        drv_ctx->dyna_blc_info.fe_delay_cnt      = fe_cfg_node_idx;
        drv_ctx->dyna_blc_info.pre_be_delay_cnt  = MIN2(isp_drv_get_pre_be_sync_index(vi_pipe, drv_ctx),
                                                        CFG2VLD_DLY_LIMIT - 1);
        drv_ctx->dyna_blc_info.post_be_delay_cnt = MIN2(isp_drv_get_be_sync_index(vi_pipe, drv_ctx),
                                                        CFG2VLD_DLY_LIMIT - 1);
        drv_ctx->dyna_blc_info.sync_cfg.black_level_mode = cur_black_level_mode;
    }
}

static td_void isp_drv_update_dyna_blc_info(ot_vi_pipe vi_pipe, isp_dynamic_blc_info *dyna_blc_info,
    const isp_drv_ctx *drv_ctx)
{
    td_u8 delay_cnt, cur_idx;
    isp_sync_cfg_buf_node  *node0 = TD_NULL;
    isp_sync_cfg_buf_node  *cur_node = TD_NULL;

    if (isp_drv_get_ob_stats_update_pos(vi_pipe) == OT_ISP_UPDATE_OB_STATS_FE_FRAME_START) {
        delay_cnt = 2; /* delay cnt 2 */
    } else {
        delay_cnt = 1;
    }
    if (drv_ctx->run_once_flag == TD_TRUE) {
        delay_cnt = 3; /* runonce delay 3 */
    }
    node0 = drv_ctx->sync_cfg.node[0];
    isp_check_pointer_void_return(node0);

    (td_void)memcpy_s(&dyna_blc_info->sync_cfg, sizeof(isp_dynamic_blc_sync_cfg),
                      &node0->dynamic_blc_cfg, sizeof(isp_dynamic_blc_sync_cfg));
    (td_void)memcpy_s(&dyna_blc_info->fpn_cfg[0], sizeof(isp_fpn_sync_cfg), &node0->fpn_cfg, sizeof(isp_fpn_sync_cfg));

    cur_idx = delay_cnt - 1;
    cur_node = drv_ctx->sync_cfg.node[cur_idx];
    if (cur_node == TD_NULL) {
        dyna_blc_info->ob_stats_read_en = TD_FALSE;
        return;
    }

    dyna_blc_info->sync_cfg.black_level_mode = cur_node->dynamic_blc_cfg.black_level_mode;

    if (dyna_blc_info->sync_cfg.black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        dyna_blc_info->ob_stats_read_en = TD_TRUE;

        if (dyna_blc_info->pre_black_level_mode == OT_ISP_BLACK_LEVEL_MODE_BUTT) {
            dyna_blc_info->is_first_frame = TD_TRUE;
        } else if (dyna_blc_info->pre_black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
            dyna_blc_info->is_first_frame = TD_TRUE;
            dyna_blc_info->is_runbe_first_frame = TD_TRUE;
        } else {
            dyna_blc_info->is_first_frame = TD_FALSE;
        }
    }

    dyna_blc_info->pre_black_level_mode = dyna_blc_info->sync_cfg.black_level_mode;
}

static td_bool isp_drv_calc_cfg_fe_blc_en(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u8 fe_cfg_node_idx)
{
    td_bool cfg_blc_en = TD_TRUE;
    isp_dynamic_blc_info *dyna_blc_info = TD_NULL;

    dyna_blc_info = &drv_ctx->dyna_blc_info;

    isp_drv_update_dyna_blc_info(vi_pipe, dyna_blc_info, drv_ctx);
    isp_drv_calc_noarmal_blc_delay(vi_pipe, drv_ctx, fe_cfg_node_idx);

    if (dyna_blc_info->sync_cfg.black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_FALSE;
    }

    if (dyna_blc_info->fe_blc_mode_change != TD_TRUE) {
        return TD_TRUE;
    }

    if (dyna_blc_info->fe_delay_cnt != 0) {
        cfg_blc_en = TD_FALSE;
        dyna_blc_info->fe_delay_cnt -= 1;
    }

    if (dyna_blc_info->fe_delay_cnt == 0) {
        dyna_blc_info->fe_blc_mode_change = TD_FALSE;
    }

    return cfg_blc_en;
}

static td_bool isp_drv_calc_cfg_post_be_blc_en(isp_drv_ctx *drv_ctx)
{
    td_bool cfg_blc_en = TD_TRUE;
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;

    if (dyna_blc_info->sync_cfg.black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_FALSE;
    }

    if (dyna_blc_info->post_be_blc_mode_change != TD_TRUE) {
        return TD_TRUE;
    }

    if (dyna_blc_info->post_be_delay_cnt != 0) {
        cfg_blc_en = TD_FALSE;
        dyna_blc_info->post_be_delay_cnt -= 1;
    }

    if (dyna_blc_info->post_be_delay_cnt == 0) {
        dyna_blc_info->post_be_blc_mode_change = TD_FALSE;
    }

    return cfg_blc_en;
}

static td_bool isp_drv_calc_cfg_pre_be_blc_en(isp_drv_ctx *drv_ctx)
{
    td_bool cfg_blc_en = TD_TRUE;
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;

    if (dyna_blc_info->sync_cfg.black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_FALSE;
    }

    if (dyna_blc_info->pre_be_blc_mode_change != TD_TRUE) {
        return TD_TRUE;
    }

    if (dyna_blc_info->pre_be_delay_cnt != 0) {
        cfg_blc_en = TD_FALSE;
        dyna_blc_info->pre_be_delay_cnt -= 1;
    }

    if (dyna_blc_info->pre_be_delay_cnt == 0) {
        dyna_blc_info->pre_be_blc_mode_change = TD_FALSE;
    }

    return cfg_blc_en;
}

static td_void isp_drv_offline_be_blc_reg(isp_drv_ctx *drv_ctx, const isp_sync_cfg_buf_node *cfg_node)
{
    td_u8 stitch_idx;
    ot_vi_pipe stitch_pipe;
    isp_drv_ctx *drv_ctx_stitch_pipe = TD_NULL;
    td_bool cfg_blc_en = TD_TRUE;

    (td_void)memcpy_s(&drv_ctx->be_sync_para.fpn_cfg, sizeof(isp_fpn_sync_cfg),
                      &cfg_node->fpn_cfg, sizeof(isp_fpn_sync_cfg));
    if (drv_ctx->stitch_attr.stitch_enable == TD_FALSE) {
        cfg_blc_en = isp_drv_calc_cfg_post_be_blc_en(drv_ctx);
        if (cfg_blc_en == TD_FALSE) {
            return;
        }
        (td_void)memcpy_s(&drv_ctx->be_sync_para.be_blc, sizeof(isp_be_blc_dyna_cfg),
                          &cfg_node->be_blc_reg_cfg, sizeof(isp_be_blc_dyna_cfg));
        return;
    }

    if (drv_ctx->stitch_attr.main_pipe != TD_TRUE) {
        return;
    }

    for (stitch_idx = 0; stitch_idx < drv_ctx->stitch_attr.stitch_pipe_num; stitch_idx++) {
        stitch_pipe = drv_ctx->stitch_attr.stitch_bind_id[stitch_idx];
        drv_ctx_stitch_pipe = isp_drv_get_ctx(stitch_pipe);

        cfg_blc_en = isp_drv_calc_cfg_post_be_blc_en(drv_ctx_stitch_pipe);
        if (cfg_blc_en == TD_FALSE) {
            continue;
        }

        (td_void)memcpy_s(&drv_ctx_stitch_pipe->be_sync_para.be_blc, sizeof(isp_be_blc_dyna_cfg),
                          &cfg_node->be_blc_reg_cfg_stitch[stitch_idx], sizeof(isp_be_blc_dyna_cfg));
        /* for main pipe */
        (td_void)memcpy_s(&drv_ctx->be_sync_para_stitch[stitch_idx].be_blc, sizeof(isp_be_blc_dyna_cfg),
                          &cfg_node->be_blc_reg_cfg_stitch[stitch_idx], sizeof(isp_be_blc_dyna_cfg));
    }

    return;
}

static td_void isp_drv_get_be_blc_online(isp_drv_ctx *drv_ctx, const isp_sync_cfg_buf_node *cfg_node)
{
    td_bool cfg_blc_en;

    cfg_blc_en = isp_drv_calc_cfg_post_be_blc_en(drv_ctx);
    if (cfg_blc_en == TD_FALSE) {
        return;
    }
    (td_void)memcpy_s(&drv_ctx->be_sync_para.be_blc, sizeof(isp_be_blc_dyna_cfg),
                      &cfg_node->be_blc_reg_cfg, sizeof(isp_be_blc_dyna_cfg));
    (td_void)memcpy_s(&drv_ctx->be_sync_para.fpn_cfg, sizeof(isp_fpn_sync_cfg),
                      &cfg_node->fpn_cfg, sizeof(isp_fpn_sync_cfg));
}

static td_void isp_drv_get_post_be_blc_pre_online_post_offline(isp_drv_ctx *drv_ctx,
    const isp_sync_cfg_buf_node *cfg_node)
{
}

static td_void isp_drv_get_pre_be_blc_pre_online_post_offline(isp_drv_ctx *drv_ctx,
    isp_sync_cfg_buf_node *pre_be_cfg_node)
{
    td_bool cfg_blc_en;
    td_u16  blc_dyna_size = sizeof(isp_blc_dyna_cfg);
    isp_be_blc_dyna_cfg *be_blc_sync = TD_NULL;
    isp_be_blc_dyna_cfg *be_blc_cfg = TD_NULL;

    cfg_blc_en = isp_drv_calc_cfg_pre_be_blc_en(drv_ctx);
    if (cfg_blc_en == TD_FALSE) {
        return;
    }
    be_blc_sync = &drv_ctx->be_sync_para.be_blc;
    be_blc_cfg  = &pre_be_cfg_node->be_blc_reg_cfg;

    (td_void)memcpy_s(&be_blc_sync->ge_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM,
                      &be_blc_cfg->ge_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM);
    (td_void)memcpy_s(&be_blc_sync->wdr_dg_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM,
                      &be_blc_cfg->wdr_dg_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM);
    (td_void)memcpy_s(&be_blc_sync->wdr_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM,
                      &be_blc_cfg->wdr_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM);
    (td_void)memcpy_s(&be_blc_sync->flicker_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM,
                      &be_blc_cfg->flicker_blc[0], blc_dyna_size * OT_ISP_WDR_MAX_FRAME_NUM);
    (td_void)memcpy_s(&be_blc_sync->expander_blc, blc_dyna_size, &be_blc_cfg->expander_blc, blc_dyna_size);
    (td_void)memcpy_s(&drv_ctx->be_sync_para.fpn_cfg, sizeof(isp_fpn_sync_cfg),
                      &pre_be_cfg_node->fpn_cfg, sizeof(isp_fpn_sync_cfg));
    (td_void)memcpy_s(&be_blc_sync->lsc_blc, blc_dyna_size, &be_blc_cfg->lsc_blc, blc_dyna_size);
    (td_void)memcpy_s(&be_blc_sync->dg_blc, blc_dyna_size, &be_blc_cfg->dg_blc, blc_dyna_size);
    (td_void)memcpy_s(&be_blc_sync->wb_blc, blc_dyna_size, &be_blc_cfg->wb_blc, blc_dyna_size);
}

static td_void isp_drv_get_be_blc_pre_online_post_offline(isp_drv_ctx *drv_ctx, const isp_sync_cfg_buf_node *cfg_node,
    isp_sync_cfg_buf_node *pre_be_cfg_node)
{
    isp_drv_get_post_be_blc_pre_online_post_offline(drv_ctx, cfg_node);
    isp_drv_get_pre_be_blc_pre_online_post_offline(drv_ctx, pre_be_cfg_node);
}

td_s32 isp_drv_reg_config_be_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_sync_cfg_buf_node *cfg_node,
    isp_sync_cfg_buf_node *pre_be_cfg_node)
{
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(drv_ctx);
    isp_check_pointer_return(cfg_node);

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        isp_drv_get_be_blc_online(drv_ctx, cfg_node);
    } else if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        isp_drv_get_be_blc_pre_online_post_offline(drv_ctx, cfg_node, pre_be_cfg_node);
    } else {
        isp_drv_offline_be_blc_reg(drv_ctx, cfg_node);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_set_fe_blc_reg(ot_vi_pipe vi_pipe_bind, td_u8 k, isp_fe_reg_type *fe_reg,
    const isp_fe_blc_dyna_cfg *fe_blc_reg_cfg)
{
    td_u8  af_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };
    td_u8  fe_nr_dg_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };
    isp_check_pointer_void_return(fe_reg);
    isp_check_pointer_void_return(fe_blc_reg_cfg);

    if (fe_blc_reg_cfg->resh_dyna == TD_TRUE) {
        isp_drv_fe_global_blc_mode_write(fe_reg, fe_blc_reg_cfg->global_blc.blc_mode);
        isp_drv_fe_global_target_mode_write(fe_reg, fe_blc_reg_cfg->global_blc.target_mode);
        isp_drv_fe_global_blc_write(fe_reg, fe_blc_reg_cfg->global_blc.global_blc1[k]);
        isp_drv_fe_target_blc_write(fe_reg, fe_blc_reg_cfg->global_blc.target_blc[k]);
        isp_drv_fe_dg_offset_write(fe_reg, fe_blc_reg_cfg->fe_dg_blc[k].blc);
        isp_drv_fe_dg_real_offset_write(fe_reg, fe_blc_reg_cfg->fe_dg_real_blc[k].blc);
        isp_drv_fe_wb_offset_write(fe_reg, fe_blc_reg_cfg->fe_wb_blc[k].blc);
        isp_drv_fe_ae_offset_write(fe_reg, fe_blc_reg_cfg->fe_ae_blc[k].blc);
        isp_drv_fe_blc1_en_write(fe_reg, fe_blc_reg_cfg->fe_blc1_en);
        isp_drv_fe_blc_offset_write(fe_reg, fe_blc_reg_cfg->fe_blc[k].blc);
        if (af_en[vi_pipe_bind] == 1) {
            isp_drv_fe_af_offset_write(fe_reg, fe_blc_reg_cfg->fe_af_blc[k].blc);
        }
        if (fe_nr_dg_en[vi_pipe_bind] == 1) {
            isp_drv_fe_nr_dg_offset_write(fe_reg, fe_blc_reg_cfg->fe_nr_dg_blc[k].blc);
            isp_drv_fe_nr_dg_real_offset_write(fe_reg, fe_blc_reg_cfg->fe_nr_dg_real_blc[k].blc);
        }
    }
}

static td_s32 isp_drv_reg_config_fe_blc_all_wdr_pipe(td_u8 chn_num_max, const isp_drv_ctx *drv_ctx,
    const isp_fe_blc_dyna_cfg  *fe_blc_reg_cfg)
{
    td_u8 k;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_reg_type *fe_reg = TD_NULL;

    for (k = 0; k < chn_num_max; k++) {
        vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);
        isp_drv_set_fe_blc_reg(vi_pipe_bind, k, fe_reg, fe_blc_reg_cfg);
    }

    return TD_SUCCESS;
}

static td_s32 isp_drv_reg_config_fe_blc_stitch(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, td_u8 fe_cfg_node_idx,
    td_u8 chn_num_max)
{
    td_u8 i;
    td_u8 stitch_idx = 0;
    isp_drv_ctx *drv_ctx_main_pipe = TD_NULL;
    isp_sync_cfg_buf_node *cfg_node_main_pipe = TD_NULL;

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        if (vi_pipe == drv_ctx->stitch_attr.stitch_bind_id[i]) {
            stitch_idx = i;
            break;
        }
    }

    drv_ctx_main_pipe  = isp_drv_get_ctx(drv_ctx->stitch_attr.stitch_bind_id[0]);
    cfg_node_main_pipe = drv_ctx_main_pipe->sync_cfg.node[fe_cfg_node_idx];
    if (cfg_node_main_pipe == TD_NULL) {
        return TD_SUCCESS;
    }

    return isp_drv_reg_config_fe_blc_all_wdr_pipe(chn_num_max, drv_ctx,
                                                  &cfg_node_main_pipe->fe_blc_reg_cfg_stitch[stitch_idx]);
}

td_s32 isp_drv_reg_config_fe_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_bool cfg_blc_en;
    td_u8 fe_cfg_node_idx, cfg_node_vc, chn_num_max;
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(drv_ctx);

    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    fe_cfg_node_idx = isp_drv_get_fe_sync_index(vi_pipe, drv_ctx, TD_FALSE);
    fe_cfg_node_idx = MIN2(fe_cfg_node_idx, CFG2VLD_DLY_LIMIT - 1);

    if (is_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        cfg_node_vc  = (drv_ctx->sync_cfg.cfg2_vld_dly_max - 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
    } else {
        cfg_node_vc  = 0;
    }

    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);
    cfg_node    = drv_ctx->sync_cfg.node[fe_cfg_node_idx];
    if (cfg_node == TD_NULL) {
        return TD_SUCCESS;
    }
    if (drv_ctx->sync_cfg.vc_cfg_num != cfg_node_vc) {
        return TD_SUCCESS;
    }

    cfg_blc_en = isp_drv_calc_cfg_fe_blc_en(vi_pipe, drv_ctx, fe_cfg_node_idx);
    if (cfg_blc_en != TD_TRUE) {
        return TD_SUCCESS;
    }
    drv_ctx->dyna_blc_info.actual_info.is_ready = TD_FALSE;
    if (drv_ctx->stitch_attr.stitch_enable == TD_FALSE) {
        return isp_drv_reg_config_fe_blc_all_wdr_pipe(chn_num_max, drv_ctx, &cfg_node->fe_blc_reg_cfg);
    }

    return isp_drv_reg_config_fe_blc_stitch(vi_pipe, drv_ctx, fe_cfg_node_idx, chn_num_max);
}

td_s32 isp_drv_reg_config_fe_dgain(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 k, fe_cfg_node_idx, cfg_node_vc, chn_num_max;
    ot_vi_pipe vi_pipe_bind;
    td_u32 wdr_gain;
    isp_fe_reg_type     *fe_reg   = TD_NULL;
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;

    isp_check_no_fe_pipe_return(vi_pipe);
    isp_check_pointer_return(drv_ctx);

    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    fe_cfg_node_idx = isp_drv_get_fe_sync_index(vi_pipe, drv_ctx, TD_FALSE);
    fe_cfg_node_idx = MIN2(fe_cfg_node_idx, CFG2VLD_DLY_LIMIT - 1);

    if (is_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        cfg_node_vc  = (drv_ctx->sync_cfg.cfg2_vld_dly_max - 1) % div_0_to_1(drv_ctx->sync_cfg.vc_num_max + 1);
    } else {
        cfg_node_vc  = 0;
    }

    chn_num_max = clip3(drv_ctx->wdr_attr.pipe_num, 1, ISP_WDR_CHN_MAX);
    cfg_node    = drv_ctx->sync_cfg.node[fe_cfg_node_idx];
    if (cfg_node == TD_NULL) {
        return TD_SUCCESS;
    }

    if (drv_ctx->sync_cfg.vc_cfg_num == cfg_node_vc) {
        for (k = 0; k < chn_num_max; k++) {
            vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
            isp_check_no_fe_pipe_return(vi_pipe_bind);
            isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);
            if (cfg_node->ae_reg_cfg.thermo_enable == TD_TRUE) {
                isp_drv_set_fe_dgain(fe_reg, 0x100);
                isp_drv_set_fe_nr_dgain(vi_pipe_bind, fe_reg, 0x100);
            } else {
                wdr_gain = drv_ctx->sync_cfg.wdr_gain[k][fe_cfg_node_idx];
                wdr_gain = (cfg_node->ae_reg_cfg.isp_dgain * wdr_gain) >> 0x8;
                wdr_gain = clip3(wdr_gain, ISP_DIGITAL_GAIN_MIN, ISP_DIGITAL_GAIN_MAX);
                isp_drv_set_fe_dgain(fe_reg, wdr_gain);
                isp_drv_set_fe_nr_dgain(vi_pipe_bind, fe_reg, cfg_node->ae_reg_cfg.fe_nr_dgain[k]);
            }
        }
    }

    return TD_SUCCESS;
}

static td_void isp_drv_set_wdr_sync_cfg(isp_pre_be_reg_type *pre_be, const isp_be_wdr_sync_param *wdr_sync_para)
{
    isp_drv_set_wdr_exporratio0(pre_be, wdr_sync_para->wdr_exp_ratio);
    isp_drv_set_flick_exporatio0(pre_be, wdr_sync_para->flick_exp_ratio);
    isp_drv_set_wdr_expo_value0(pre_be, wdr_sync_para->wdr_exp_val[0]); /* index 0 */
    isp_drv_set_wdr_expo_value1(pre_be, wdr_sync_para->wdr_exp_val[1]); /* index 1 */
    isp_drv_set_wdr_mdt_en(pre_be, wdr_sync_para->wdr_mdt_en);
    isp_drv_set_wdr_fusion_mode(pre_be, wdr_sync_para->fusion_mode);
    isp_drv_set_wdr_short_thr0(pre_be, wdr_sync_para->short_thr);
    isp_drv_set_wdr_long_thr0(pre_be, wdr_sync_para->long_thr);
    isp_drv_set_wdr_max_ratio(pre_be, wdr_sync_para->wdr_max_ratio);
}

static td_void isp_drv_reg_config_wdr_2to1(isp_drv_ctx *drv_ctx, const isp_fswdr_sync_cfg *wdr_reg_cfg,
    td_u32 *ratio)
{
    td_u8  i;
    td_u32 expo_value[OT_ISP_WDR_MAX_FRAME_NUM] = { 0 }; /* exposure max 4 */
    isp_be_wdr_sync_param *wdr_sync_para = &drv_ctx->be_sync_para.wdr;

    wdr_sync_para->wdr_exp_ratio = MIN2((isp_bitmask(10) * EXP_RATIO_MIN / div_0_to_1(ratio[0])), 0x3FF); /* 10 */
    wdr_sync_para->flick_exp_ratio = MIN2(ratio[0], 0X3FFF);

    if (wdr_reg_cfg->fusion_mode == 0) {
        expo_value[0] = MIN2(ratio[0], isp_bitmask(14)); /* const value 14 */
        expo_value[1] = MIN2(EXP_RATIO_MIN, isp_bitmask(14)); /* const value 14 */
    } else {
        expo_value[0] = MIN2((ratio[0] + EXP_RATIO_MIN), isp_bitmask(14)); /* const value 14 */
        expo_value[1] = MIN2(EXP_RATIO_MIN, isp_bitmask(14)); /* const value 14 */
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        wdr_sync_para->wdr_exp_val[i]    = expo_value[i];
    }
}

static td_void isp_drv_reg_config_wdr_thr(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_fswdr_sync_cfg *wdr_reg_cfg)
{
    td_bool wdr_mdt_en;
    td_u8   cfg_dly_max, lf_mode;
    td_u16  long_thr = 0xFFF;
    td_u16  short_thr = 0xFFF;

    cfg_dly_max = isp_drv_get_pre_be_sync_index(vi_pipe, drv_ctx);
    cfg_dly_max = MIN2(cfg_dly_max, CFG2VLD_DLY_LIMIT - 1);
    lf_mode = drv_ctx->sync_cfg.lf_mode[cfg_dly_max];

    long_thr  = wdr_reg_cfg->long_thr;
    short_thr = wdr_reg_cfg->short_thr;

    wdr_mdt_en = wdr_reg_cfg->wdr_mdt_en;
    if ((lf_mode != 0) && (drv_ctx->be_sync_para.wdr.wdr_exp_val[0] < 0x44)) {
        long_thr  = 0xFFF;
        short_thr = 0xFFF;
        wdr_mdt_en   = 0;
    }

    drv_ctx->be_sync_para.wdr.long_thr  = long_thr;
    drv_ctx->be_sync_para.wdr.short_thr = short_thr;
    drv_ctx->be_sync_para.wdr.wdr_mdt_en  = wdr_mdt_en;
}

td_s32 isp_drv_reg_config_wdr(ot_vi_pipe vi_pipe, isp_fswdr_sync_cfg *wdr_reg_cfg, td_u32 *ratio)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_wdr_sync_param *wdr_sync_para = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(wdr_reg_cfg);
    isp_check_pointer_return(ratio);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (is_linear_mode(drv_ctx->sync_cfg.wdr_mode) || is_built_in_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        return TD_SUCCESS;
    }

    if (is_2to1_wdr_mode(drv_ctx->sync_cfg.wdr_mode)) {
        isp_drv_reg_config_wdr_2to1(drv_ctx, wdr_reg_cfg, ratio);
    }

    wdr_sync_para = &drv_ctx->be_sync_para.wdr;
    wdr_sync_para->wdr_max_ratio = ((1 << 22) - 1) / div_0_to_1(wdr_sync_para->wdr_exp_val[0]); /* 22 */
    wdr_sync_para->fusion_max_ratio = wdr_sync_para->wdr_max_ratio;
    wdr_sync_para->fusion_mode      = wdr_reg_cfg->fusion_mode;
    isp_drv_reg_config_wdr_thr(vi_pipe, drv_ctx, wdr_reg_cfg);

    return TD_SUCCESS;
}

static td_void isp_drv_reg_config_pq_ai_ldci(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u32 ldci_comp)
{
    isp_drv_ctx *ai_drv_ctx = TD_NULL;

    if (vi_pipe == drv_ctx->ai_info.base_pipe_id) {
        if (drv_ctx->ai_info.ai_pipe_id < 0 || drv_ctx->ai_info.ai_pipe_id >= OT_ISP_MAX_PIPE_NUM) {
            isp_err_trace("Err ai pipe %d!\n", drv_ctx->ai_info.ai_pipe_id);
            return;
        }

        ai_drv_ctx = isp_drv_get_ctx(drv_ctx->ai_info.ai_pipe_id);
        ai_drv_ctx->be_sync_para.ldci.ldci_comp = ldci_comp;
    }
}

td_s32 isp_drv_reg_config_ldci(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u32  ldci_comp;
    td_u32  ldci_comp_index;

    isp_check_pointer_return(drv_ctx);
    if ((drv_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == drv_ctx->ai_info.ai_pipe_id)) {
        return TD_SUCCESS;
    }

    if (isp_drv_get_run_wakeup_sel(vi_pipe) == OT_ISP_RUN_WAKEUP_BE_END) {
        ldci_comp = drv_ctx->sync_cfg.drc_comp_next;
    } else {
        ldci_comp_index = isp_get_drc_comp_sync_index(vi_pipe, drv_ctx);
        if (ldci_comp_index >= 1) {
            ldci_comp_index = ldci_comp_index - 1; /* ldci compensate is earlier tham drc one frame */
        } else {
            ldci_comp_index = 0;
        }
        ldci_comp_index = MIN2(ldci_comp_index, CFG2VLD_DLY_LIMIT - 1);

        ldci_comp = drv_ctx->sync_cfg.drc_comp[ldci_comp_index];
    }

    ldci_comp = sqrt32(ldci_comp << DRC_COMP_SHIFT);
    ldci_comp = MIN2(ldci_comp, 0xFFFF);

    drv_ctx->be_sync_para.ldci.ldci_comp = ldci_comp;
    if (drv_ctx->ai_info.pq_ai_en == TD_TRUE) {
        isp_drv_reg_config_pq_ai_ldci(vi_pipe, drv_ctx, ldci_comp);
    }

    return TD_SUCCESS;
}

static td_bool isp_drv_get_bnr_is_initial(const isp_drv_ctx *drv_ctx)
{
    td_bool is_init = TD_FALSE;
    td_bool pre_tnr_en, cur_tnr_en;

    cur_tnr_en = drv_ctx->bnr_tpr_filt.cur.tnr_en && drv_ctx->bnr_tpr_filt.cur.nr_en;
    pre_tnr_en = drv_ctx->bnr_tpr_filt.pre.tnr_en && drv_ctx->bnr_tpr_filt.pre.nr_en;
    if ((cur_tnr_en == TD_TRUE) && (pre_tnr_en == TD_FALSE)) {
        is_init = TD_TRUE;
    }

    return is_init;
}

static td_void isp_drv_bnr_entmpnr_write(isp_post_be_reg_type *be_reg, td_bool tnr_en, td_bool is_init)
{
    u_isp_bnr_cfg isp_bnr_cfg;
    isp_bnr_cfg.u32 = be_reg->isp_bnr_cfg.u32;
    isp_bnr_cfg.bits.isp_bnr_entmpnr   = tnr_en;
    isp_bnr_cfg.bits.isp_bnr_isinitial = is_init;
    be_reg->isp_bnr_cfg.u32 = isp_bnr_cfg.u32;
}

td_s32 isp_drv_disable_bnr_wmot(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);
    if (ckfn_vi_set_pipe_disable_bnr_wmot() != TD_NULL) {
        ret = call_vi_set_pipe_disable_bnr_wmot(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] call_vi_set_pipe_bnr_mot_off Failed!\n", vi_pipe);
            return ret;
        }
    } else {
        isp_err_trace("ISP[%d] ckfn_vi_set_pipe_bnr_mot_off is TD_NULL\n", vi_pipe);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void isp_drv_reg_config_bnr_online(ot_vi_pipe vi_pipe, isp_post_be_reg_type *be_reg[],
    isp_viproc_reg_type *viproc_reg[], isp_drv_ctx *drv_ctx)
{
    td_bool is_init, tnr_en;
    td_u8 k, blk_num, blk_dev;
    isp_kernel_tpr_filt_reg  *bnr_tpr_filt = TD_NULL;
    u_isp_viproc_ispbe_ctrl0 viproc_ispbe_ctrl0;

    if (is_offline_mode(drv_ctx->work_mode.running_mode) ||
        is_striping_mode(drv_ctx->work_mode.running_mode)) {
        return;
    }

    blk_num = drv_ctx->work_mode.block_num;
    blk_dev = drv_ctx->work_mode.block_dev;

    bnr_tpr_filt = &drv_ctx->bnr_tpr_filt;

    is_init = isp_drv_get_bnr_is_initial(drv_ctx);
    tnr_en = bnr_tpr_filt->cur.tnr_en && bnr_tpr_filt->cur.nr_en;
    for (k = 0; k < blk_num; k++) {
        if (tnr_en == TD_FALSE) {
            isp_drv_disable_bnr_wmot(vi_pipe);
        }

        isp_drv_bnr_entmpnr_write(be_reg[k + blk_dev], tnr_en, is_init);

        /* bnr enable */
        viproc_ispbe_ctrl0.u32 = viproc_reg[k + blk_dev]->viproc_ispbe_ctrl0.u32;
        viproc_ispbe_ctrl0.bits.isp_bnr_en = bnr_tpr_filt->cur.nr_en;
        viproc_reg[k + blk_dev]->viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
    }

    (td_void)memcpy_s(&bnr_tpr_filt->pre, sizeof(isp_bnr_temporal_filt),
                      &bnr_tpr_filt->cur, sizeof(isp_bnr_temporal_filt));
}

td_void isp_drv_reg_config_bnr_offline(isp_be_wo_reg_cfg *be_cfg, isp_drv_ctx *drv_ctx)
{
    td_bool is_init, tnr_en;
    td_u8 k;
    u_isp_viproc_ispbe_ctrl0 viproc_ispbe_ctrl0;

    is_init = isp_drv_get_bnr_is_initial(drv_ctx);

    tnr_en = drv_ctx->bnr_tpr_filt.cur.tnr_en && drv_ctx->bnr_tpr_filt.cur.nr_en;
    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        isp_drv_bnr_entmpnr_write(&be_cfg->be_reg_cfg[k].be_reg, tnr_en, is_init);
         /* bnr enable */
        viproc_ispbe_ctrl0.u32 = be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32;
        viproc_ispbe_ctrl0.bits.isp_bnr_en = drv_ctx->bnr_tpr_filt.cur.nr_en;
        be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
    }

    (td_void)memcpy_s(&drv_ctx->bnr_tpr_filt.pre, sizeof(isp_bnr_temporal_filt),
                      &drv_ctx->bnr_tpr_filt.cur, sizeof(isp_bnr_temporal_filt));
}

static td_void isp_drv_calc_drc_exp_ratio(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u32 *exp_ratio)
{
    td_u8 cfg_dly_max;
    td_u32 drc_exp_ratio;
    isp_drv_ctx *base_drv_ctx = TD_NULL;

    cfg_dly_max = isp_get_drc_comp_sync_index(vi_pipe, drv_ctx);
    cfg_dly_max = MIN2(cfg_dly_max, CFG2VLD_DLY_LIMIT - 1);

    if ((drv_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == drv_ctx->ai_info.ai_pipe_id)) {
        if (drv_ctx->ai_info.base_pipe_id < 0 || drv_ctx->ai_info.base_pipe_id >= OT_ISP_MAX_PIPE_NUM) {
            isp_err_trace("Err base pipe %d!\n", drv_ctx->ai_info.base_pipe_id);
            return;
        }
        base_drv_ctx = isp_drv_get_ctx(drv_ctx->ai_info.base_pipe_id);
        drc_exp_ratio = base_drv_ctx->sync_cfg.drc_comp[cfg_dly_max];
    } else {
        drc_exp_ratio = drv_ctx->sync_cfg.drc_comp[cfg_dly_max];
    }

    if (drc_exp_ratio != 0x1000) { /* do division only when drc_exp_ratio != 4096 */
        drc_exp_ratio = div_0_to_1(drc_exp_ratio);
        drc_exp_ratio = osal_div_u64((1 << (DRC_COMP_SHIFT + DRC_COMP_SHIFT)), drc_exp_ratio);
        drc_exp_ratio = MIN2(drc_exp_ratio, (15 << DRC_COMP_SHIFT)); /* Maximum supported ratio is 15 */
    }
    *exp_ratio = drc_exp_ratio;
}

td_void isp_drv_calc_drc_prev_luma(isp_drc_sync_cfg *drc_reg_cfg, td_bool update_log_param,
                                   td_u8 drc_shp_log, td_u32 drc_exp_ratio, td_u32 *drc_prev_luma)
{
    td_u32 i;

    /* Compensate on PrevLuma when ShpLog/ShpExp is modified, but no compensation under offline repeat mode */
    if (update_log_param) {
        for (i = 0; i < OT_ISP_DRC_EXP_COMP_SAMPLE_NUM - 1; i++) {
            drc_prev_luma[i] =
                (td_u32)((td_s32)g_drc_cur_luma_lut[drc_shp_log][i] + drc_reg_cfg->prev_luma_delta[i]);
        }
    } else {
        for (i = 0; i < OT_ISP_DRC_EXP_COMP_SAMPLE_NUM - 1; i++) {
            drc_prev_luma[i] = g_drc_cur_luma_lut[drc_shp_log][i];
        }
    }
    drc_prev_luma[OT_ISP_DRC_EXP_COMP_SAMPLE_NUM - 1] = (1 << 20); /* left shift 20 */

    if (drc_exp_ratio != 0x1000) {
        for (i = 0; i < OT_ISP_DRC_EXP_COMP_SAMPLE_NUM; i++) {
            drc_prev_luma[i] = (td_u32)(((td_u64)drc_exp_ratio * drc_prev_luma[i]) >> DRC_COMP_SHIFT);
        }
    }
}

td_s32 isp_drv_reg_config_drc(ot_vi_pipe vi_pipe, isp_drc_sync_cfg *drc_reg_cfg)
{
    td_u8  i;
    td_u8  blk_dev;
    td_u32 drc_div_denom_log, drc_denom_exp;
    td_u32 drc_exp_ratio = 0x1000;
    td_u32 drc_prev_luma[OT_ISP_DRC_EXP_COMP_SAMPLE_NUM] = {0};
    td_bool update_log_param;
    isp_drv_ctx *drv_ctx = TD_NULL;

    isp_check_pointer_return(drc_reg_cfg);

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    blk_dev = drv_ctx->work_mode.block_dev;

    isp_drv_calc_drc_exp_ratio(vi_pipe, drv_ctx, &drc_exp_ratio);

    update_log_param = (g_drc_shp_log[vi_pipe][blk_dev] != drc_reg_cfg->shp_log ||
                        g_drc_shp_exp[vi_pipe][blk_dev] != drc_reg_cfg->shp_exp) ? TD_TRUE : TD_FALSE;

    g_drc_shp_log[vi_pipe][blk_dev] = drc_reg_cfg->shp_log;
    g_drc_shp_exp[vi_pipe][blk_dev] = drc_reg_cfg->shp_exp;

    isp_drv_calc_drc_prev_luma(drc_reg_cfg, update_log_param,
                               g_drc_shp_log[vi_pipe][blk_dev], drc_exp_ratio, drc_prev_luma);

    drc_div_denom_log = g_drc_div_denom_log[g_drc_shp_log[vi_pipe][blk_dev]];
    drc_denom_exp    = g_drc_denom_exp[g_drc_shp_exp[vi_pipe][blk_dev]];

    for (i = 0; i < OT_ISP_DRC_EXP_COMP_SAMPLE_NUM; i++) {
        drv_ctx->be_sync_para.drc.prev_luma[i] = drc_prev_luma[i];
    }

    drv_ctx->be_sync_para.drc.shp_log       = g_drc_shp_log[vi_pipe][blk_dev];
    drv_ctx->be_sync_para.drc.div_denom_log = drc_div_denom_log;
    drv_ctx->be_sync_para.drc.denom_exp     = drc_denom_exp;

    return TD_SUCCESS;
}

td_void isp_drv_reg_config_sync_ccm(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u16 *color_matrix)
{
    td_s32 i;
    td_u8  stitch_idx = 0;
    ot_vi_pipe stitch_main_pipe;
    isp_drv_ctx *drv_ctx_main_pipe = TD_NULL;
    td_u16  ccm[OT_ISP_CCM_MATRIX_SIZE] = { 0 };

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        ccm[i] = color_matrix[i];
    }

    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        drv_ctx->be_sync_para.ccm[i] = ccm[i];
    }
    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        stitch_main_pipe = drv_ctx->stitch_attr.stitch_bind_id[0];
        drv_ctx_main_pipe = isp_drv_get_ctx(stitch_main_pipe);

        for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
            if (vi_pipe == drv_ctx->stitch_attr.stitch_bind_id[i]) {
                stitch_idx = i;
                break;
            }
        }

        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            drv_ctx_main_pipe->be_sync_para_stitch[stitch_idx].ccm[i] = ccm[i];
        }
    }
}

static td_void isp_drv_reg_config_pq_ai_wb_gain(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u32 *wb_dgain)
{
    td_u8 i;
    isp_drv_ctx *ai_drv_ctx = TD_NULL;
    if ((drv_ctx->ai_info.pq_ai_en == TD_TRUE) && (vi_pipe == drv_ctx->ai_info.ai_pipe_id)) {
        return;
    }

    if (vi_pipe == drv_ctx->ai_info.base_pipe_id) {
        if (drv_ctx->ai_info.ai_pipe_id < 0 || drv_ctx->ai_info.ai_pipe_id >= OT_ISP_MAX_PIPE_NUM) {
            isp_err_trace("Err ai pipe %d!\n", drv_ctx->ai_info.ai_pipe_id);
            return;
        }

        ai_drv_ctx = isp_drv_get_ctx(drv_ctx->ai_info.ai_pipe_id);
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            ai_drv_ctx->be_sync_para.wb_gain[i] = wb_dgain[i];
        }
    }
}

static td_s32 isp_drv_reg_config_awb_gain(ot_vi_pipe vi_pipe,
    isp_drv_ctx *drv_ctx, const isp_awb_reg_cfg_2 *awb_reg_cfg)
{
    td_u8  i;
    td_u8  stitch_idx = 0;
    ot_vi_pipe stitch_main_pipe;
    td_u32 wb_gain[OT_ISP_BAYER_CHN_NUM] = {0};
    isp_drv_ctx *drv_ctx_main_pipe = TD_NULL;

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        wb_gain[i] = awb_reg_cfg->be_white_balance_gain[i];
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        drv_ctx->be_sync_para.wb_gain[i] = wb_gain[i];
    }
    isp_drv_reg_config_pq_ai_wb_gain(vi_pipe, drv_ctx, wb_gain);

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        stitch_main_pipe = drv_ctx->stitch_attr.stitch_bind_id[0];
        drv_ctx_main_pipe = isp_drv_get_ctx(stitch_main_pipe);

        for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
            if (vi_pipe == drv_ctx->stitch_attr.stitch_bind_id[i]) {
                stitch_idx = i;
                break;
            }
        }

        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            drv_ctx_main_pipe->be_sync_para_stitch[stitch_idx].wb_gain[i] = wb_gain[i];
        }
    }

    return TD_SUCCESS;
}

static td_void isp_drv_reg_config_sync_awb_ccm_stitch(const isp_drv_ctx *drv_ctx, td_u8 cfg_node_idx, td_u8 cfg_node_vc)
{
    td_u8 i, j, pipe_id;
    ot_vi_pipe stitch_pipe;
    isp_drv_ctx *stitch_drv_ctx = TD_NULL;
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;
    isp_sync_cfg_buf_node *ccm_cfg_node = TD_NULL;

    if (drv_ctx->stitch_attr.main_pipe != TD_TRUE) {
        return;
    }

    cfg_node     = drv_ctx->sync_cfg.node[cfg_node_idx];
    ccm_cfg_node = drv_ctx->sync_cfg.node[0];
    isp_check_pointer_void_return(cfg_node);
    isp_check_pointer_void_return(ccm_cfg_node);

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = drv_ctx->stitch_attr.stitch_bind_id[i];
        stitch_drv_ctx = isp_drv_get_ctx(stitch_pipe);

        isp_drv_reg_config_sync_ccm(stitch_pipe, stitch_drv_ctx, ccm_cfg_node->awb_reg_cfg_stitch[i].color_matrix);
        isp_drv_reg_config_awb_gain(stitch_pipe, stitch_drv_ctx, &cfg_node->awb_reg_cfg_stitch[i]);
    }

    for (i = 1; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = drv_ctx->stitch_attr.stitch_bind_id[i];
        stitch_drv_ctx = isp_drv_get_ctx(stitch_pipe);

        for (pipe_id = 0; pipe_id < OT_ISP_MAX_PIPE_NUM; pipe_id++) {
            for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
                stitch_drv_ctx->be_sync_para_stitch[pipe_id].wb_gain[j] =
                    drv_ctx->be_sync_para_stitch[pipe_id].wb_gain[j];
            }

            for (j = 0; j < OT_ISP_CCM_MATRIX_SIZE; j++) {
                stitch_drv_ctx->be_sync_para_stitch[pipe_id].ccm[j] = drv_ctx->be_sync_para_stitch[pipe_id].ccm[j];
            }
        }
    }
}

static td_void isp_drv_reg_config_sync_awb_ccm_normal(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    td_u8 cfg_node_idx, td_u8 cfg_node_vc)
{
    isp_sync_cfg_buf_node *cfg_node = TD_NULL;
    isp_sync_cfg_buf_node *ccm_cfg_node = TD_NULL;
    cfg_node     = drv_ctx->sync_cfg.node[cfg_node_idx];
    ccm_cfg_node = drv_ctx->sync_cfg.node[0];
    isp_check_pointer_void_return(cfg_node);
    isp_check_pointer_void_return(ccm_cfg_node);
    if (drv_ctx->sync_cfg.vc_cfg_num == cfg_node_vc) {
        isp_drv_reg_config_sync_ccm(vi_pipe, drv_ctx, ccm_cfg_node->awb_reg_cfg.color_matrix);
        isp_drv_reg_config_awb_gain(vi_pipe, drv_ctx, &cfg_node->awb_reg_cfg);
    }

    return;
}

td_void isp_drv_reg_config_sync_awb_ccm(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    td_u8 cfg_node_idx, td_u8 cfg_node_vc)
{
    isp_check_pointer_void_return(drv_ctx);
    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        return isp_drv_reg_config_sync_awb_ccm_stitch(drv_ctx, cfg_node_idx, cfg_node_vc);
    }

    return isp_drv_reg_config_sync_awb_ccm_normal(vi_pipe, drv_ctx, cfg_node_idx, cfg_node_vc);
}


td_s32 isp_drv_reg_config_4dgain(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_sync_4dgain_cfg *sync_4dgain_cfg)
{
    td_u32 *wdr_gain = TD_NULL;

    wdr_gain = sync_4dgain_cfg->wdr_gain;

    drv_ctx->be_sync_para.wdr_gain[0] = wdr_gain[0];
    drv_ctx->be_sync_para.wdr_gain[1] = wdr_gain[1];
    drv_ctx->be_sync_para.wdr_gain[2] = wdr_gain[2]; /* 2: config channel 2 dgain value */
    drv_ctx->be_sync_para.wdr_gain[3] = wdr_gain[3]; /* 3: config channel 3 dgain value */

    return TD_SUCCESS;
}

td_void isp_drv_reg_config_vi_fpn_offline(isp_be_wo_reg_cfg *be_cfg, const isp_drv_ctx *drv_ctx)
{
    td_bool fpn_cor_en;
    td_u8 k;
    u_isp_viproc_ispbe_ctrl0 viproc_ispbe_ctrl0;
    isp_check_pointer_void_return(be_cfg);
    isp_check_pointer_void_return(drv_ctx);

    /* reset isp_fpn_en */
    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        viproc_ispbe_ctrl0.u32 = be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32;
        viproc_ispbe_ctrl0.bits.isp_fpn_en = 0;
        be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
    }

    if (drv_ctx->fpn_work_mode != FPN_MODE_CORRECTION) {
        return;
    }
    fpn_cor_en = drv_ctx->be_sync_para.fpn_cfg.fpn_cor_en;

    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        viproc_ispbe_ctrl0.u32 = be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32;
        viproc_ispbe_ctrl0.bits.isp_fpn_en   = fpn_cor_en;
        if (fpn_cor_en == TD_TRUE) {
            viproc_ispbe_ctrl0.bits.isp_fpn_mode = 0; /* 0: cor mode */
        }
        be_cfg->be_reg_cfg[k].viproc_reg.viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
    }
}

static td_s32 isp_drv_set_vi_fpn_correction_en_online(ot_vi_pipe vi_pipe, td_bool fpn_cor_en,
                                                      isp_drv_ctx *drv_ctx)
{
    td_u8  k, blk_dev;
    td_s32 ret;
    isp_viproc_reg_type *pre_viproc[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    u_isp_viproc_ispbe_ctrl0 viproc_ispbe_ctrl0;

    if (drv_ctx->pre_fpn_cor_en == fpn_cor_en) {
        return TD_SUCCESS;
    }

    if (ckfn_vi_set_fpn_correct_en() != TD_NULL) {
        ret = call_vi_set_fpn_correct_en(vi_pipe, fpn_cor_en);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    } else {
        return TD_FAILURE;
    }

    ret = isp_drv_get_pre_viproc_reg_virt_addr(vi_pipe, pre_viproc);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    blk_dev = drv_ctx->work_mode.block_dev;
    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        viproc_ispbe_ctrl0.u32 = pre_viproc[k + blk_dev]->viproc_ispbe_ctrl0.u32;
        viproc_ispbe_ctrl0.bits.isp_fpn_en = fpn_cor_en;
        if (fpn_cor_en == TD_TRUE) {
            viproc_ispbe_ctrl0.bits.isp_fpn_mode = 0; /* 0: cor mode */
        }
        pre_viproc[k + blk_dev]->viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
    }

    drv_ctx->pre_fpn_cor_en = fpn_cor_en;

    return TD_SUCCESS;
}

static td_void isp_drv_set_fpn_cfg(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    isp_pre_be_reg_type *pre_be, const isp_fpn_sync_cfg *fpn_cfg)
{
    td_s32 ret;

    if (drv_ctx->fpn_work_mode == FPN_MODE_CALIBRATE) {
        return;
    }

    if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode) ||
        is_online_mode(drv_ctx->work_mode.running_mode)) {
        ret = isp_drv_set_vi_fpn_correction_en_online(vi_pipe, fpn_cfg->fpn_cor_en, drv_ctx);
        if (ret != TD_SUCCESS) {
            return;
        }
    }

    isp_drv_set_fpn_offset(pre_be, fpn_cfg->add_offset[0]);
    isp_drv_set_fpn_correct_en(pre_be, fpn_cfg->fpn_cor_en);
    if (is_2to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
    }
}

static td_void isp_drv_set_drc_sync_cfg(isp_be_reg_type *be_reg, const isp_be_drc_sync_param *drc_sync_para)
{
    isp_drv_set_drc_shp_cfg(be_reg, drc_sync_para->shp_log, drc_sync_para->shp_log);
    isp_drv_set_drc_div_denom_log(be_reg, drc_sync_para->div_denom_log);
    isp_drv_set_drc_denom_exp(be_reg, drc_sync_para->denom_exp);
    isp_drv_set_drc_prev_luma0(be_reg, drc_sync_para->prev_luma[0]); /* isp_drc_prev_luma_0 */
    isp_drv_set_drc_prev_luma1(be_reg, drc_sync_para->prev_luma[1]); /* isp_drc_prev_luma_1 */
    isp_drv_set_drc_prev_luma2(be_reg, drc_sync_para->prev_luma[2]); /* isp_drc_prev_luma_2 */
    isp_drv_set_drc_prev_luma3(be_reg, drc_sync_para->prev_luma[3]); /* isp_drc_prev_luma_3 */
    isp_drv_set_drc_prev_luma4(be_reg, drc_sync_para->prev_luma[4]); /* isp_drc_prev_luma_4 */
    isp_drv_set_drc_prev_luma5(be_reg, drc_sync_para->prev_luma[5]); /* isp_drc_prev_luma_5 */
    isp_drv_set_drc_prev_luma6(be_reg, drc_sync_para->prev_luma[6]); /* isp_drc_prev_luma_6 */
    isp_drv_set_drc_prev_luma7(be_reg, drc_sync_para->prev_luma[7]); /* isp_drc_prev_luma_7 */
}

static td_void isp_drv_set_aibnr_wdr_gain_cfg(isp_be_sync_para *be_sync_para, isp_aibnr_cfg *ainr_cfg)
{
#if defined(CONFIG_OT_AIISP_SUPPORT) && defined(CONFIG_OT_AIBNR_SUPPORT)
    if (ainr_cfg->dgain_bypass == TD_FALSE) {
        return;
    }

    if (ainr_cfg->blend_en == TD_FALSE) {
        be_sync_para->wdr_gain[0] = 0x100;
        return;
    }

    // if blend on
    be_sync_para->wdr_gain[0] = (be_sync_para->isp_dgain[0] * be_sync_para->wdr_gain[0]) >> 0x8;
    be_sync_para->isp_dgain[0] = 0x100;
#endif
}

static td_void isp_drv_set_aibnr_dgain_cfg(isp_post_be_reg_type *pre_be, td_bool dgain_bypass)
{
#if defined(CONFIG_OT_AIISP_SUPPORT) && defined(CONFIG_OT_AIBNR_SUPPORT)
    td_u16 isp_dgain = 0x100;

    if (dgain_bypass == TD_FALSE) {
        return;
    }

    isp_drv_set_be_dgain(pre_be, isp_dgain);
#endif
}

static td_void isp_drv_set_pre_be_sync_para(ot_vi_pipe vi_pipe, isp_pre_be_reg_type *pre_be, isp_drv_ctx *drv_ctx,
    isp_be_sync_para *be_sync_para, isp_aibnr_cfg *ainr_cfg)
{
    td_bool cfg_blc_normal = TD_TRUE;
    if ((drv_ctx->dyna_blc_info.sync_cfg.black_level_mode == OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) &&
        (isp_drv_get_ob_stats_update_pos(vi_pipe) == OT_ISP_UPDATE_OB_STATS_FE_FRAME_END)) {
        if (is_online_mode(drv_ctx->work_mode.running_mode) ||
            is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
            cfg_blc_normal = TD_FALSE;
        }
    }

    isp_drv_set_aibnr_wdr_gain_cfg(be_sync_para, ainr_cfg);

    /* isp 4dgain */
    isp_drv_set_isp_4dgain0(pre_be, be_sync_para->wdr_gain[0]);
    isp_drv_set_isp_4dgain1(pre_be, be_sync_para->wdr_gain[1]);
    isp_drv_set_isp_4dgain2(pre_be, be_sync_para->wdr_gain[2]); /* 2: config channel 2 dgain value */
    isp_drv_set_isp_4dgain3(pre_be, be_sync_para->wdr_gain[3]); /* 3: config channel 3 dgain value */

    /* wdr */
    isp_drv_set_wdr_sync_cfg(pre_be, &be_sync_para->wdr);
    if (cfg_blc_normal == TD_TRUE) {
        isp_drv_set_pre_be_blc_reg(vi_pipe, drv_ctx, pre_be, &be_sync_para->be_blc);
        isp_drv_set_fpn_cfg(vi_pipe, drv_ctx, pre_be, &be_sync_para->fpn_cfg);
        /* isp dgain */
        isp_drv_set_be_dgain(pre_be, be_sync_para->isp_dgain[0]);
        isp_drv_set_aibnr_dgain_cfg(pre_be, ainr_cfg->dgain_bypass);
    }

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        isp_drv_set_awb_gain(pre_be, be_sync_para->wb_gain);
    }
}

static td_void isp_drv_set_post_be_sync_para(ot_vi_pipe vi_pipe, isp_post_be_reg_type *post_be,
    const isp_drv_ctx *drv_ctx, isp_be_sync_para *be_sync_para)
{
    /* ldci */
    isp_drv_set_ldci_stat_evratio(post_be, be_sync_para->ldci.ldci_comp);

    /* drc */
    isp_drv_set_drc_sync_cfg(post_be, &be_sync_para->drc);

    /* awb */
    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        isp_drv_set_ccm(post_be, be_sync_para->ccm);
    }
}

td_s32 isp_drv_set_be_sync_para_offline(ot_vi_pipe vi_pipe, td_void *be_node, isp_be_sync_para *be_sync_para,
    isp_aibnr_cfg *ainr_cfg)
{
    td_u8 i;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_be_wo_reg_cfg *be_reg = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_check_pointer_return(be_node);
    isp_check_pointer_return(be_sync_para);
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if ((isp_drv_get_alg_run_select(vi_pipe) == OT_ISP_ALG_RUN_FE_ONLY) && (drv_ctx->yuv_mode != TD_TRUE)) {
        return TD_SUCCESS;
    }

    if (drv_ctx->isp_init != TD_TRUE) {
        return TD_FAILURE;
    }
    be_reg  = (isp_be_wo_reg_cfg *)be_node;

    for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
        isp_drv_set_pre_be_sync_para(vi_pipe, &be_reg->be_reg_cfg[i].be_reg, drv_ctx, be_sync_para, ainr_cfg);
        isp_drv_set_post_be_sync_para(vi_pipe, &be_reg->be_reg_cfg[i].be_reg, drv_ctx, be_sync_para);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_set_be_format_offline(isp_be_wo_reg_cfg *be_node, const isp_drv_ctx *drv_ctx,
    isp_be_format be_format)
{
    td_u8 i;
    for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
        isp_drv_be_format_write(&be_node->be_reg_cfg[i].be_reg, be_format);
    }
}

static td_void isp_drv_set_be_format_online(ot_vi_pipe vi_pipe, isp_be_format be_format)
{
    td_u8 i, block_dev;
    td_s32 ret;
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_post_be_reg_type *post_be_reg[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    ret = isp_drv_get_post_be_reg_virt_addr(vi_pipe, post_be_reg);
    if (ret != TD_SUCCESS) {
        return;
    }
    drv_ctx = isp_drv_get_ctx(vi_pipe);
    block_dev = drv_ctx->work_mode.block_dev;
    for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
        isp_drv_be_format_write(post_be_reg[i + block_dev], be_format);
    }
}

td_s32 isp_drv_set_be_format(ot_vi_pipe vi_pipe, td_void *be_node, isp_be_format be_format)
{
    isp_drv_ctx *drv_ctx = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    if (be_format >= ISP_BE_FORMAT_BUTT) {
        isp_err_trace("Err be_format!\n");
        return TD_FAILURE;
    }

    drv_ctx = isp_drv_get_ctx(vi_pipe);
    if (drv_ctx->isp_init != TD_TRUE) {
        return TD_FAILURE;
    }
    if (is_offline_mode(drv_ctx->work_mode.running_mode) ||
        is_striping_mode(drv_ctx->work_mode.running_mode) ||
        is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        isp_check_pointer_return(be_node);
        isp_drv_set_be_format_offline((isp_be_wo_reg_cfg *)be_node, drv_ctx, be_format);
    } else if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        isp_drv_set_be_format_online(vi_pipe, be_format);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_set_isp_pre_be_sync_param_online(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 k, blk_dev;
    td_s32 ret;
    isp_pre_be_reg_type *be_reg[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    isp_viproc_reg_type *pre_viproc[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    isp_aibnr_cfg ainr_cfg = {0};

    ret = isp_drv_get_pre_be_reg_virt_addr(vi_pipe, be_reg);
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = isp_drv_get_pre_viproc_reg_virt_addr(vi_pipe, pre_viproc);
    if (ret != TD_SUCCESS) {
        return;
    }

    isp_drv_reg_config_bnr_online(vi_pipe, be_reg, pre_viproc, drv_ctx);

    blk_dev = drv_ctx->work_mode.block_dev;
    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        isp_drv_set_pre_be_sync_para(vi_pipe, be_reg[k + blk_dev], drv_ctx, &drv_ctx->be_sync_para, &ainr_cfg);
    }
}

static td_void isp_drv_set_isp_post_be_sync_param_online(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8   i, block_dev;
    td_s32  ret;
    isp_viproc_reg_type *post_viproc[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    isp_post_be_reg_type *post_be_reg[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    ret = isp_drv_get_post_be_reg_virt_addr(vi_pipe, post_be_reg);
    if (ret != TD_SUCCESS) {
        return;
    }
    ret = isp_drv_get_post_viproc_reg_virt_addr(vi_pipe, post_viproc);
    if (ret != TD_SUCCESS) {
        return;
    }

    block_dev = drv_ctx->work_mode.block_dev;
    for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
        isp_drv_set_post_be_sync_para(vi_pipe, post_be_reg[i + block_dev], drv_ctx, &drv_ctx->be_sync_para);
        isp_drv_set_be_regup(post_viproc[i + block_dev], TD_TRUE);
    }
}

td_s32 isp_drv_set_isp_be_sync_param_online(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 cfg_node_idx;
    isp_check_pointer_return(drv_ctx);
    cfg_node_idx = MIN2(isp_drv_get_be_sync_index(vi_pipe, drv_ctx), CFG2VLD_DLY_LIMIT - 1);
    if (drv_ctx->sync_cfg.node[cfg_node_idx] == TD_NULL) {
        return TD_SUCCESS;
    }

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        isp_drv_set_isp_pre_be_sync_param_online(vi_pipe, drv_ctx);
        isp_drv_set_isp_post_be_sync_param_online(vi_pipe, drv_ctx);
    } else if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        isp_drv_set_isp_pre_be_sync_param_online(vi_pipe, drv_ctx);
    }

    return TD_SUCCESS;
}

#if defined(CONFIG_OT_AIISP_SUPPORT) && defined(CONFIG_OT_AIBNR_SUPPORT)
#define ISP_SWITCH_AIBNR_SPECIAL_PROCESS_CNT 3
#define ISP_AIBNR_OFF_BNR_SPECIAL_PROCESS_CNT 5
static td_void isp_drv_set_bnr_en_ai_md(isp_post_be_reg_type *post_be, td_bool ai_mode)
{
    u_isp_bnr_cfg15 isp_bnr_cfg15;
    isp_bnr_cfg15.u32 = post_be->isp_bnr_cfg15.u32;
    if (ai_mode == TD_TRUE) {
        isp_bnr_cfg15.bits.isp_bnr_en_aimd = 1;
    } else {
        isp_bnr_cfg15.bits.isp_bnr_en_aimd = 0;
    }
    post_be->isp_bnr_cfg15.u32 = isp_bnr_cfg15.u32;
}

static td_void isp_drv_aibnr_set_bnr_bypass(isp_be_all_reg_type *be_reg_cfg, td_bool bnr_bypass)
{
    u_isp_bnr_cfg isp_bnr_cfg;
    td_bool bnr_en = bnr_bypass ? TD_FALSE : TD_TRUE;
    u_isp_viproc_ispbe_ctrl0 viproc_ispbe_ctrl0;

    if (bnr_bypass == TD_FALSE) {
        return;
    }

    isp_bnr_cfg.u32 = be_reg_cfg->be_reg.isp_bnr_cfg.u32;
    isp_bnr_cfg.bits.isp_bnr_entmpnr = bnr_en;
    be_reg_cfg->be_reg.isp_bnr_cfg.u32 = isp_bnr_cfg.u32;
    /* bnr enable */
    viproc_ispbe_ctrl0.u32 = be_reg_cfg->viproc_reg.viproc_ispbe_ctrl0.u32;
    viproc_ispbe_ctrl0.bits.isp_bnr_en = bnr_en;
    be_reg_cfg->viproc_reg.viproc_ispbe_ctrl0.u32 = viproc_ispbe_ctrl0.u32;
}

static td_void isp_drv_aibnr_set_sptnr(isp_post_be_reg_type *post_be, isp_drv_ctx *drv_ctx)
{
    td_bool aibnr_en = drv_ctx->aibnr_info.aibnr_en;
    td_bool blend_en = drv_ctx->aibnr_info.blend_en;
    td_bool bnr_ensptnr = TD_FALSE;
    u_isp_bnr_cfg o_isp_bnr_cfg;

    td_u8 bnr_mode = (aibnr_en == TD_FALSE) ? 0 : (blend_en == TD_TRUE) ? 1 : 2; // mode 0 1 2

    td_bool ai_md = (bnr_mode == 1) ? 1 : 0;

    if (drv_ctx->work_mode.is_ia_nr_enable == 1) {
        ai_md = 0;
    }

    bnr_ensptnr = (ai_md == 1) ? 0 : (drv_ctx->bnr_tpr_filt.cur.nr_en == TD_TRUE) ? 1 : 0;

    o_isp_bnr_cfg.u32 = post_be->isp_bnr_cfg.u32;
    o_isp_bnr_cfg.bits.isp_bnr_ensptnr = bnr_ensptnr;
    post_be->isp_bnr_cfg.u32 = o_isp_bnr_cfg.u32;
}

static td_void isp_drv_set_all_snr(isp_post_be_reg_type *post_be, isp_drv_ctx *drv_ctx)
{
    u_isp_bnr_cfg10 o_isp_bnr_cfg10;
    u_isp_bnr_cfg16 o_isp_bnr_cfg16;
    u_isp_bnr_cfg13 o_isp_bnr_cfg13;
    o_isp_bnr_cfg10.u32 = post_be->isp_bnr_cfg10.u32;
    o_isp_bnr_cfg10.bits.isp_bnr_target_noise_ratio = 0;
    post_be->isp_bnr_cfg10.u32 = o_isp_bnr_cfg10.u32;

    o_isp_bnr_cfg16.u32 = post_be->isp_bnr_cfg16.u32;
    o_isp_bnr_cfg16.bits.isp_bnr_jnlm_gain = 128; // 128: max nr strength
    o_isp_bnr_cfg16.bits.isp_bnr_coring_enable = 0;
    post_be->isp_bnr_cfg16.u32 = o_isp_bnr_cfg16.u32;

    o_isp_bnr_cfg13.u32 = post_be->isp_bnr_cfg13.u32;
    o_isp_bnr_cfg13.bits.isp_bnr_tnr1_ratio = 0;
    post_be->isp_bnr_cfg13.u32 = o_isp_bnr_cfg13.u32;
}

static td_void isp_drv_set_bnr_lut_cfg(isp_pre_be_lut_wstt_type *lut2stt)
{
    td_s32 i;

    for (i = 0; i < 65; i++) { /* [0, 65) */
        lut2stt->isp_bnr_lut_wstt[4 * i + 2].u32 = 300; /* 4 * i + 2, 300 */
    }

    for (i = 0; i < 64; i++) { /* [0, 64) */
        lut2stt->isp_bnr_lut_wstt[4 * i + 3].u32 = 300; /* 4 * i + 3, 300 */
    }
}

td_void isp_drv_set_aibnr_bnr_cfg(isp_be_wo_reg_cfg *be_reg, isp_drv_ctx *drv_ctx, td_bool bnr_bypass)
{
    td_u8 i;
    td_bool ai_mode = TD_FALSE;
    isp_be_sync_para *be_sync_para = &drv_ctx->be_sync_para;

    if (drv_ctx->aibnr_info.aibnr_en == TD_TRUE && drv_ctx->aibnr_info.blend_en == TD_TRUE) {
        ai_mode = TD_TRUE;
    }

    if (drv_ctx->work_mode.aiisp_mode == OT_VI_AIISP_MODE_DEFAULT) {
        for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
            isp_drv_aibnr_set_bnr_bypass(&be_reg->be_reg_cfg[i], bnr_bypass);
            isp_drv_set_bnr_en_ai_md(&be_reg->be_reg_cfg[i].be_reg, ai_mode);
            isp_drv_aibnr_set_sptnr(&be_reg->be_reg_cfg[i].be_reg, drv_ctx);
        }
    }

    if ((drv_ctx->aibnr_info.aibnr_en != drv_ctx->aibnr_info.pre_aibnr_en) &&
        ((be_sync_para->wdr_gain[0] != 0x100) || (be_sync_para->isp_dgain[0] != 0x100) ||
        (be_sync_para->isp_dgain[0] != drv_ctx->aibnr_info.pre_isp_dgain))) {
        drv_ctx->aibnr_info.switch_aibnr_cnt = ISP_SWITCH_AIBNR_SPECIAL_PROCESS_CNT;
    }

    if (drv_ctx->aibnr_info.switch_aibnr_cnt > 0) {
        for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
            isp_drv_set_all_snr(&be_reg->be_reg_cfg[i].be_reg, drv_ctx);
        }

        drv_ctx->aibnr_info.switch_aibnr_cnt--;
    }

    // aibnr -> bnr and isp dgain is 1
    if ((drv_ctx->aibnr_info.aibnr_en == TD_FALSE) && (drv_ctx->aibnr_info.pre_aibnr_en == TD_TRUE) &&
        (be_sync_para->wdr_gain[0] == 0x100 && be_sync_para->isp_dgain[0] == 0x100)) {
        drv_ctx->aibnr_info.off_switch_cnt = ISP_AIBNR_OFF_BNR_SPECIAL_PROCESS_CNT;
    }

    if (drv_ctx->aibnr_info.off_switch_cnt > 0) {
        for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
            /* set strong stength for tnr */
            isp_drv_set_bnr_lut_cfg(&be_reg->be_reg_cfg[i].be_reg.be_lut.be_lut2stt.pre_be_lut2stt);
        }

        drv_ctx->aibnr_info.off_switch_cnt--;
    }

    drv_ctx->aibnr_info.pre_aibnr_en = drv_ctx->aibnr_info.aibnr_en;
    drv_ctx->aibnr_info.pre_isp_dgain = be_sync_para->isp_dgain[0];

    return;
}
#endif

#if defined(CONFIG_OT_AIISP_SUPPORT) && defined(CONFIG_OT_AIDRC_SUPPORT)

static td_void isp_drv_aidrc_set_drc_comm_cfg(isp_post_be_reg_type *post_be, isp_drc_sync_cfg *drc_reg_cfg,
                                              td_bool aidrc_en, isp_aidrc_mode aidrc_mode, td_u32 frame_idx)
{
    td_u32 stat_format = 0, stat_flttype = 0;
    td_u32 detail_rd_en, in_adapt_mode;
    td_u32 adapt_mode_on = 2;
    td_u32 flttype_normal = 2;

    if (!aidrc_en && frame_idx == 0) {
        stat_format = 0;
        stat_flttype = 0;
        in_adapt_mode = drc_reg_cfg->shoot_reduction_en ? adapt_mode_on : 0;
        detail_rd_en = TD_FALSE;
    } else {
        detail_rd_en = TD_TRUE;
        in_adapt_mode = adapt_mode_on;
        if (aidrc_mode == ISP_AIDRC_MODE_NORMAL) {
            stat_format = 0;
            stat_flttype = 0;
        } else if (aidrc_mode == ISP_AIDRC_MODE_ADVANCED) {
            stat_format = 1;
            stat_flttype = flttype_normal;
        }
    }

    isp_drv_set_drc_inadapt_mode(post_be, in_adapt_mode);
    isp_drv_set_drc_statformat(post_be, stat_format);
    isp_drv_set_drc_statflttype(post_be, stat_flttype);
    isp_drv_set_drc_detail_rd_en(post_be, detail_rd_en);
}

static td_u8 isp_drv_aidrc_update_blend_coef(td_u8 curr_coef, td_u8 target_coef, td_u8 frame_idx)
{
    td_u8 switch_interval = 4;
    td_s16 step = ((td_s16)target_coef - (td_s16)curr_coef) / div_0_to_1(switch_interval);
    return (td_u8)clip3((td_s16)target_coef - step * frame_idx, 0x0, 0xFF);
}

static td_void isp_drv_aidrc_set_drc_blend_cfg(isp_post_be_reg_type *post_be, isp_drc_sync_cfg *drc_reg_cfg,
    td_bool aidrc_en, isp_aidrc_mode aidrc_mode, td_u32 frame_idx)
{
    td_u8 bld_none = 0xFF;
    td_u8 bld_l_max, bld_l_bright_min, bld_l_dark_min;
    td_u8 bld_d_max, bld_d_bright_min, bld_d_dark_min;

    if (!aidrc_en) {
        if (frame_idx > 0) {
            bld_l_max = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_l_max, bld_none, frame_idx);
            bld_l_bright_min = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_l_bright_min, bld_none, frame_idx);
            bld_l_dark_min = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_l_dark_min, bld_none, frame_idx);
            bld_d_max = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_d_max, bld_none, frame_idx);
            bld_d_bright_min = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_d_bright_min, bld_none, frame_idx);
            bld_d_dark_min = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->bld_d_dark_min, bld_none, frame_idx);
        } else {
            bld_l_max = bld_none;
            bld_l_bright_min = bld_none;
            bld_l_dark_min = bld_none;
            bld_d_max = bld_none;
            bld_d_bright_min = bld_none;
            bld_d_dark_min = bld_none;
        }
    } else {
        if (frame_idx > 0) {
            bld_l_max = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_l_max, frame_idx);
            bld_l_bright_min = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_l_bright_min, frame_idx);
            bld_l_dark_min = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_l_dark_min, frame_idx);
            bld_d_max = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_d_max, frame_idx);
            bld_d_bright_min = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_d_bright_min, frame_idx);
            bld_d_dark_min = isp_drv_aidrc_update_blend_coef(bld_none, drc_reg_cfg->bld_d_dark_min, frame_idx);
        } else {
            bld_l_max = drc_reg_cfg->bld_l_max;
            bld_l_bright_min = drc_reg_cfg->bld_l_bright_min;
            bld_l_dark_min = drc_reg_cfg->bld_l_dark_min;
            bld_d_max = drc_reg_cfg->bld_d_max;
            bld_d_bright_min = drc_reg_cfg->bld_d_bright_min;
            bld_d_dark_min = drc_reg_cfg->bld_d_dark_min;
        }
    }

    isp_drv_set_drc_brightmaxl(post_be, bld_l_max);
    isp_drv_set_drc_brightminl(post_be, bld_l_bright_min);
    isp_drv_set_drc_darkmaxl(post_be, bld_l_max);
    isp_drv_set_drc_darkminl(post_be, bld_l_dark_min);
    isp_drv_set_drc_brightmaxd(post_be, bld_d_max);
    isp_drv_set_drc_brightmind(post_be, bld_d_bright_min);
    isp_drv_set_drc_darkmaxd(post_be, bld_d_max);
    isp_drv_set_drc_darkmind(post_be, bld_d_dark_min);
}

static td_void isp_drv_aidrc_set_drc_gain_cfg(isp_post_be_reg_type *post_be, isp_drc_sync_cfg *drc_reg_cfg,
                                              td_bool aidrc_en, isp_aidrc_mode aidrc_mode, td_u32 frame_idx)
{
    td_u8 gain_coef_max = 0x80;
    td_u8 ltm_gain_coef, gtm_gain_coef;

    if (!aidrc_en) {
        if (frame_idx > 0) {
            ltm_gain_coef = isp_drv_aidrc_update_blend_coef(drc_reg_cfg->tone_mapping_wgt_x, 0, frame_idx);
        } else {
            ltm_gain_coef = 0;
        }
    } else {
        if (frame_idx > 0) {
            ltm_gain_coef = isp_drv_aidrc_update_blend_coef(0, drc_reg_cfg->tone_mapping_wgt_x, frame_idx);
        } else {
            ltm_gain_coef = drc_reg_cfg->tone_mapping_wgt_x;
        }
    }

    gtm_gain_coef = gain_coef_max - ltm_gain_coef;

    isp_drv_set_drc_ltm_gain_coef(post_be, ltm_gain_coef);
    isp_drv_set_drc_gtm_gain_coef(post_be, gtm_gain_coef);
}

static td_void isp_drv_aidrc_set_drc_stat_cfg(isp_post_be_reg_type *post_be, isp_drv_ctx *drv_ctx,
                                              td_bool aidrc_en, isp_aidrc_mode aidrc_mode, td_u32 frame_idx)
{
    td_u32 height = drv_ctx->work_mode.frame_rect.height;
    td_u32 total_width = drv_ctx->work_mode.frame_rect.width;
    td_u32 h_blk_size, v_blk_size, h_blk_num, v_blk_num, slice_height, slice_width;
    td_u32 ds_scale = 0x20;
    td_u32 div_x_fix = 0x400, div_y_fix =  0x400;

    if (aidrc_mode == ISP_AIDRC_MODE_ADVANCED && (aidrc_en || frame_idx > 0)) {
        slice_height = (height % ds_scale) ? (height / ds_scale + 1) * ds_scale : height;
        slice_width  = (total_width % ds_scale) ? (total_width / ds_scale + 1) * ds_scale : total_width;

        h_blk_num = slice_width / ds_scale;
        v_blk_num = slice_height / ds_scale;

        h_blk_size = ds_scale;
        v_blk_size = ds_scale;

        isp_drv_set_drc_chk_y(post_be, 0);
        isp_drv_set_drc_chk_x(post_be, 0);
        isp_drv_set_drc_vnum(post_be, v_blk_num);
        isp_drv_set_drc_hnum(post_be, h_blk_num);
        isp_drv_set_drc_zone_vsize(post_be, v_blk_size - 1);
        isp_drv_set_drc_zone_hsize(post_be, h_blk_size - 1);
        isp_drv_set_drc_div_x0(post_be, div_x_fix);
        isp_drv_set_drc_div_x1(post_be, div_x_fix);
        isp_drv_set_drc_div_y0(post_be, div_y_fix);
        isp_drv_set_drc_div_y1(post_be, div_y_fix);
        isp_drv_set_drc_rdstat_en(post_be, TD_TRUE);
        isp_drv_set_drc_wrstat_en(post_be, TD_FALSE);
        isp_drv_set_drc_vbiflt_en(post_be, TD_FALSE);
    }
}

static td_void isp_drv_aidrc_set_drc_log_en(isp_be_all_reg_type *be_reg_cfg, td_bool drc_log_en)
{
    td_bool drc_en;
    u_isp_viproc_ispbe_ctrl0 o_viproc_ispbe_ctrl0;
    u_isp_viproc_ispbe_ctrl1 o_viproc_ispbe_ctrl1;

    o_viproc_ispbe_ctrl0.u32 = be_reg_cfg->viproc_reg.viproc_ispbe_ctrl0.u32;
    o_viproc_ispbe_ctrl1.u32 = be_reg_cfg->viproc_reg.viproc_ispbe_ctrl1.u32;

    drc_en = (td_bool)o_viproc_ispbe_ctrl1.bits.isp_drc_en;
    o_viproc_ispbe_ctrl0.bits.isp_drclog_en = drc_log_en && drc_en;
    be_reg_cfg->viproc_reg.viproc_ispbe_ctrl0.u32 = o_viproc_ispbe_ctrl0.u32;
}

td_void isp_drv_set_aidrc_drc_cfg(isp_be_wo_reg_cfg *be_reg, isp_drv_ctx *drv_ctx, isp_drc_sync_cfg *drc_reg_cfg)
{
    td_u8 i;
    td_bool drc_log_en;
    td_bool aidrc_true_en;
    isp_aidrc_mode mode;
    td_u32 frame_idx;
    isp_aiisp_pending_stat pending_stat;

    isp_check_pointer_void_return(drv_ctx);
    isp_check_pointer_void_return(drc_reg_cfg);

    mode = drv_ctx->aidrc_info.mode;
    frame_idx = drv_ctx->aidrc_info.pending_idx;
    pending_stat = drv_ctx->aidrc_info.pending_stat;
    if (pending_stat == ISP_AIISP_PENDING_STAT_ENABLE) {
        aidrc_true_en = TD_TRUE;
    } else if (pending_stat == ISP_AIISP_PENDING_STAT_DISABLE) {
        aidrc_true_en = TD_FALSE;
    } else {
        aidrc_true_en = drv_ctx->aidrc_info.aidrc_en;
    }

    isp_debug_trace("aidrc_en = %d, pending_stat = %d, aidrc_true_en = %d, frame_idx = %d\n",
        drv_ctx->aidrc_info.aidrc_en, pending_stat, aidrc_true_en, frame_idx);

    if ((!aidrc_true_en) && (frame_idx == 0)) {
        drc_log_en = drc_reg_cfg->shoot_reduction_en;
    } else {
        drc_log_en = TD_TRUE;
    }

    if (drv_ctx->work_mode.aiisp_mode == OT_VI_AIISP_MODE_DEFAULT ||
        drv_ctx->work_mode.aiisp_mode == OT_VI_AIISP_MODE_AIDRC) {
        for (i = 0; i < drv_ctx->work_mode.block_num; i++) {
            isp_drv_aidrc_set_drc_log_en(&be_reg->be_reg_cfg[i], drc_log_en);
            isp_drv_aidrc_set_drc_comm_cfg(&be_reg->be_reg_cfg[i].be_reg, drc_reg_cfg, aidrc_true_en, mode, frame_idx);
            isp_drv_aidrc_set_drc_blend_cfg(&be_reg->be_reg_cfg[i].be_reg, drc_reg_cfg, aidrc_true_en, mode, frame_idx);
            isp_drv_aidrc_set_drc_gain_cfg(&be_reg->be_reg_cfg[i].be_reg, drc_reg_cfg, aidrc_true_en, mode, frame_idx);
            isp_drv_aidrc_set_drc_stat_cfg(&be_reg->be_reg_cfg[i].be_reg, drv_ctx, aidrc_true_en, mode, frame_idx);
        }
    }
}

#endif

static td_void isp_drv_dynamic_blc_post_sync_info_init(isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    for (i = 1; i < DYNAMIC_BLC_BUF_NUM; i++) {
        (td_void)memcpy_s(&drv_ctx->dyna_blc_info.be_blc_sync[i], sizeof(isp_be_blc_dyna_cfg),
                          &drv_ctx->be_sync_para.be_blc, sizeof(isp_be_blc_dyna_cfg));
    }
}

static td_void isp_drv_fpn_sync_info_init(isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    td_s32 ret;
    for (i = 0; i < DYNAMIC_BLC_BUF_NUM; i++) {
        ret = memcpy_s(&drv_ctx->dyna_blc_info.fpn_cfg[i], sizeof(isp_fpn_sync_cfg),
                       &drv_ctx->be_sync_para.fpn_cfg, sizeof(isp_fpn_sync_cfg));
        isp_check_eok_void(ret);
    }
}

static td_void isp_drv_dynamic_be_blc_post_sync_info_update(isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    for (i = DYNAMIC_BLC_BUF_NUM - 1; i >= 1; i--) {
        (td_void)memcpy_s(&drv_ctx->dyna_blc_info.be_blc_sync[i], sizeof(isp_be_blc_dyna_cfg),
                          &drv_ctx->dyna_blc_info.be_blc_sync[i - 1], sizeof(isp_be_blc_dyna_cfg));
    }
}

static td_void isp_drv_fpn_sync_info_update(isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    for (i = DYNAMIC_BLC_BUF_NUM - 1; i >= 1; i--) {
        (td_void)memcpy_s(&drv_ctx->dyna_blc_info.fpn_cfg[i], sizeof(isp_fpn_sync_cfg),
                          &drv_ctx->dyna_blc_info.fpn_cfg[i - 1], sizeof(isp_fpn_sync_cfg));
    }
}

static td_s32 isp_drv_dynamic_blc_get_index(ot_vi_pipe vi_pipe, td_u8 wdr_idx, td_s16 *ag_blc_offset,
    isp_drv_ctx *drv_ctx)
{
    td_u8 blc_cfg_node_idx;

    isp_sync_cfg_buf_node *cfg_node = TD_NULL;
    if (wdr_idx >= OT_ISP_WDR_MAX_FRAME_NUM) {
        return TD_FALSE;
    }

    blc_cfg_node_idx = isp_drv_get_fe_sync_index(vi_pipe, drv_ctx, TD_FALSE);

    blc_cfg_node_idx = MIN2(blc_cfg_node_idx, CFG2VLD_DLY_LIMIT - 1);

    cfg_node    = drv_ctx->sync_cfg.node[blc_cfg_node_idx];

    if (cfg_node == TD_NULL) {
        return TD_FALSE;
    }

    *ag_blc_offset = cfg_node->dynamic_blc_cfg.ag_offset;

    return TD_SUCCESS;
}

static td_void isp_drv_dynamic_blc_calc_separate(td_u8 wdr_idx, const isp_fe_reg_type *fe_reg, isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    td_s32 cur_read[OT_ISP_BAYER_CHN_NUM];
    td_s32 filt_res[OT_ISP_BAYER_CHN_NUM];
    td_s32 wgt_last, wgt_cur;
    td_s32 offset, blc_res, blc_diff;
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;

    cur_read[OT_ISP_CHN_R]  = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value0;
    cur_read[OT_ISP_CHN_GR] = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value1;
    cur_read[OT_ISP_CHN_GB] = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value2;
    cur_read[OT_ISP_CHN_B]  = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value3;

    if (wdr_idx >= OT_ISP_WDR_MAX_FRAME_NUM) {
        return;
    }
    if (dyna_blc_info->is_first_frame == TD_TRUE) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            dyna_blc_info->last_filt_res[wdr_idx][i] = cur_read[i];
        }
        isp_drv_dynamic_blc_post_sync_info_init(drv_ctx);
        isp_drv_fpn_sync_info_init(drv_ctx);
    }

    wgt_last = dyna_blc_info->sync_cfg.filter_strength;
    wgt_cur = (1 << DYNAMIC_BLC_FILTER_FIX_BIT) - wgt_last;
    offset  = dyna_blc_info->sync_cfg.offset;
    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        blc_diff = cur_read[i] - dyna_blc_info->last_filt_res[wdr_idx][i];
        if (ABS(blc_diff) > dyna_blc_info->sync_cfg.filter_thr) {
            dyna_blc_info->last_filt_res[wdr_idx][i] = cur_read[i];
        }
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        filt_res[i] = (td_s32)signed_right_shift(
            (dyna_blc_info->last_filt_res[wdr_idx][i] * wgt_last + cur_read[i] * wgt_cur), DYNAMIC_BLC_FILTER_FIX_BIT);
        dyna_blc_info->last_filt_res[wdr_idx][i] = filt_res[i];

        blc_res = clip3(filt_res[i] + offset, 0x0, 0x3FFF);
        blc_diff = blc_res - dyna_blc_info->sns_black_level[wdr_idx][i];
        if (ABS(blc_diff) > dyna_blc_info->sync_cfg.tolerance) {
            dyna_blc_info->sns_black_level[wdr_idx][i] = (td_u16)blc_res;
        }
    }
}

static td_void isp_drv_dynamic_blc_calc_not_separate(td_u8 wdr_idx, const isp_fe_reg_type *fe_reg, isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    td_u16 blc_r, blc_gr, blc_gb, blc_b;
    td_s32 cur_read, filt_res;
    td_s32 wgt_last, wgt_cur;
    td_s32 offset, blc_res, blc_diff;

    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;
    if (wdr_idx >= OT_ISP_WDR_MAX_FRAME_NUM) {
        return;
    }

    blc_r  = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value0;
    blc_gr = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value1;
    blc_gb = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value2;
    blc_b  = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value3;

    cur_read = (blc_r + blc_gr + blc_gb + blc_b) >> 2; /* right shift 2 */

    if (dyna_blc_info->is_first_frame == TD_TRUE) {
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            dyna_blc_info->last_filt_res[wdr_idx][i] = cur_read;
        }
        isp_drv_dynamic_blc_post_sync_info_init(drv_ctx);
        isp_drv_fpn_sync_info_init(drv_ctx);
    }

    wgt_last = dyna_blc_info->sync_cfg.filter_strength;
    wgt_cur  = (1 << DYNAMIC_BLC_FILTER_FIX_BIT) - wgt_last;
    offset   = dyna_blc_info->sync_cfg.offset;

    blc_diff = cur_read - dyna_blc_info->last_filt_res[wdr_idx][0];

    if (ABS(blc_diff) > dyna_blc_info->sync_cfg.filter_thr) {
        dyna_blc_info->last_filt_res[wdr_idx][0] = cur_read;
    }

    filt_res = signed_right_shift(dyna_blc_info->last_filt_res[wdr_idx][0] * wgt_last + cur_read * wgt_cur,
                                  DYNAMIC_BLC_FILTER_FIX_BIT);
    dyna_blc_info->last_filt_res[wdr_idx][0] = filt_res;

    blc_res = clip3(filt_res + offset, 0x0, 0x3FFF);
    blc_diff = blc_res - dyna_blc_info->sns_black_level[wdr_idx][0];
    if (ABS(blc_diff) > dyna_blc_info->sync_cfg.tolerance) {
        dyna_blc_info->sns_black_level[wdr_idx][OT_ISP_CHN_R] = (td_u16)blc_res;
        dyna_blc_info->sns_black_level[wdr_idx][OT_ISP_CHN_GR] = (td_u16)blc_res;
        dyna_blc_info->sns_black_level[wdr_idx][OT_ISP_CHN_GB] = (td_u16)blc_res;
        dyna_blc_info->sns_black_level[wdr_idx][OT_ISP_CHN_B] = (td_u16)blc_res;
    }
}

static td_void isp_drv_dynamic_blc_add_offset(ot_vi_pipe vi_pipe, td_u8 wdr_idx, isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;
    td_s32 ret;
    td_s16 ag_blc_offset;

    ag_blc_offset = 0;

    ret = isp_drv_dynamic_blc_get_index(vi_pipe, wdr_idx, &ag_blc_offset, drv_ctx);
    if (ret != TD_SUCCESS) {
        return;
    }

    for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
        dyna_blc_info->sns_black_level[wdr_idx][i] = clip3(dyna_blc_info->sns_black_level[wdr_idx][i] +
                                                           ag_blc_offset, 0x0, 0x3FFF);
    }
}

static td_s32 isp_drv_dynamic_blc_calc(isp_drv_ctx *drv_ctx)
{
    td_u8 k;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;

    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    if (dyna_blc_info->is_first_frame != TD_TRUE) {
        isp_drv_dynamic_be_blc_post_sync_info_update(drv_ctx);
        isp_drv_fpn_sync_info_update(drv_ctx);
    }

    for (k = 0; k < drv_ctx->wdr_attr.pipe_num; k++) {
        vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);

        if (drv_ctx->dyna_blc_info.sync_cfg.separate_en == TD_TRUE) {
            isp_drv_dynamic_blc_calc_separate(k, fe_reg, drv_ctx);
        } else {
            isp_drv_dynamic_blc_calc_not_separate(k, fe_reg, drv_ctx);
        }

        isp_drv_dynamic_blc_add_offset(vi_pipe_bind, k, drv_ctx);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_dynamic_update_isp_black_level(isp_drv_ctx *drv_ctx)
{
    td_u16 blc_size = OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_BAYER_CHN_NUM * sizeof(td_u16);
    isp_dynamic_blc_info *dyna_blc_info = &drv_ctx->dyna_blc_info;
    if (drv_ctx->dyna_blc_info.sync_cfg.user_black_level_en == TD_TRUE) { /* user black level */
        (td_void)memcpy_s(dyna_blc_info->isp_black_level, blc_size, dyna_blc_info->sync_cfg.user_black_level, blc_size);
    } else {  /* sns black level */
        (td_void)memcpy_s(dyna_blc_info->isp_black_level, blc_size, dyna_blc_info->sns_black_level, blc_size);
    }
}

static td_void isp_drv_dynamic_fe_global_blc_reg(isp_gloal_blc_cfg *global_blc, isp_dynamic_blc_info *dyna_blc_info)
{
    td_u8 i, j;
    global_blc->blc_mode = dyna_blc_info->sync_cfg.blc_mode;
    if (dyna_blc_info->sync_cfg.blc_mode == 0) {
        global_blc->target_mode = 0;
    } else {
        global_blc->target_mode = dyna_blc_info->sync_cfg.user_black_level_en;
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            global_blc->global_blc1[i][j] = dyna_blc_info->sns_black_level[i][j];
            global_blc->target_blc[i][j]  = dyna_blc_info->isp_black_level[i][j];
        }
    }
}

static td_void isp_drv_dynamic_be_global_blc_calc(isp_be_blc_dyna_cfg *dyna_blc, isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    dyna_blc->global_blc.blc_mode = drv_ctx->dyna_blc_info.sync_cfg.blc_mode;
    if ((is_offline_mode(drv_ctx->work_mode.running_mode) ||
         is_striping_mode(drv_ctx->work_mode.running_mode)) &&
        (dyna_blc->global_blc.blc_mode == ISP_BLC_REAL_TIME)) {
        dyna_blc->global_blc.blc_mode = ISP_BLC_GLOBAL;
    }
    if (dyna_blc->global_blc.blc_mode == 0) {
        dyna_blc->global_blc.target_mode = 0;
    } else {
        dyna_blc->global_blc.target_mode = drv_ctx->dyna_blc_info.sync_cfg.user_black_level_en;
    }

    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            dyna_blc->global_blc.global_blc0[i][j] = drv_ctx->dyna_blc_info.isp_black_level[i][j];
            dyna_blc->global_blc.global_blc1[i][j] = drv_ctx->dyna_blc_info.sns_black_level[i][j];
            dyna_blc->global_blc.target_blc[i][j]  = drv_ctx->dyna_blc_info.isp_black_level[i][j];
        }
    }
}

static td_void isp_drv_dynamic_be_blc_calc(isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    isp_be_blc_dyna_cfg *dyna_blc = &drv_ctx->dyna_blc_info.be_blc_sync[0];

    if ((dyna_blc->blc_id == 0) || (dyna_blc->blc_id >= RUN_BE_DYN_BLC_ID_MAX)) {
        dyna_blc->blc_id = RUN_BE_DYN_BLC_ID_MIN; // [RUN_BE_DYN_BLC_ID_MIN, RUN_BE_DYN_BLC_ID_MAX]
    } else {
        dyna_blc->blc_id++;
    }

    isp_drv_dynamic_be_global_blc_calc(dyna_blc, drv_ctx);

    for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
        for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
            dyna_blc->ge_blc[i].blc[j]      = dyna_blc->global_blc.global_blc0[i][j]; /* GE */
            dyna_blc->wdr_dg_blc[i].blc[j]  = dyna_blc->global_blc.global_blc0[i][j]; /* 4DG */
            dyna_blc->wdr_blc[i].blc[j]     = dyna_blc->global_blc.global_blc0[i][j]; /* WDR */
            dyna_blc->flicker_blc[i].blc[j] = dyna_blc->global_blc.global_blc0[i][j]; /* flicker */
        }

        dyna_blc->expander_blc.blc[j] = 0;
        dyna_blc->lsc_blc.blc[j]      = dyna_blc->global_blc.target_blc[0][j];  /* lsc */
        dyna_blc->dg_blc.blc[j]       = dyna_blc->global_blc.target_blc[0][j];  /* Dg */
        dyna_blc->dg_real_blc.blc[j]  = dyna_blc->global_blc.global_blc1[0][j]; /* Dg */
        dyna_blc->wb_blc.blc[j]       = dyna_blc->global_blc.target_blc[0][j];  /* WB */
    }
    dyna_blc->bnr_blc.blc[0] = dyna_blc->global_blc.target_blc[0][0] >> 2; /* shift 2 */
}

static td_s32 isp_drv_dynamic_fe_blc_calc_reg(isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    td_s16 global_blc1, target_blc, diff;
    isp_fe_blc_dyna_cfg dyna_blc;

    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }
    isp_drv_dynamic_update_isp_black_level(drv_ctx);

    isp_drv_dynamic_fe_global_blc_reg(&dyna_blc.global_blc, &drv_ctx->dyna_blc_info);

    dyna_blc.fe_blc1_en = drv_ctx->dyna_blc_info.sync_cfg.user_black_level_en;
    for (i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            global_blc1 = dyna_blc.global_blc.global_blc1[i][j];
            target_blc  = dyna_blc.global_blc.target_blc[i][j];
            diff = clip3(global_blc1 - target_blc, -16383, 16383); /* range[-16383, 16383] */
            dyna_blc.fe_blc[i].blc[j]    = diff;
            dyna_blc.fe_nr_dg_blc[i].blc[j]      = target_blc;  /* Fe Dg1 */
            dyna_blc.fe_nr_dg_real_blc[i].blc[j] = global_blc1; /* Fe Dg1 */
            dyna_blc.fe_dg_blc[i].blc[j]         = target_blc;  /* Fe Dg2 */
            dyna_blc.fe_dg_real_blc[i].blc[j]    = global_blc1; /* Fe Dg2 */
            dyna_blc.fe_wb_blc[i].blc[j]         = target_blc; /* Fe WB */
            dyna_blc.fe_ae_blc[i].blc[j]         = target_blc; /* Fe AE */
            dyna_blc.fe_af_blc[i].blc[j]         = target_blc; /* Fe AF */
        }
    }

    dyna_blc.resh_dyna = TD_TRUE;
    isp_drv_reg_config_fe_blc_all_wdr_pipe(drv_ctx->wdr_attr.pipe_num, drv_ctx, &dyna_blc);
    isp_drv_dynamic_be_blc_calc(drv_ctx);

    return TD_SUCCESS;
}

static td_void isp_drv_dynamic_pre_be_blc_reg(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_be_blc_dyna_cfg *be_blc)
{
    td_u8 k, blk_dev;
    td_s32 ret;
    isp_pre_be_reg_type *be_reg[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    ret = isp_drv_get_pre_be_reg_virt_addr(vi_pipe, be_reg);
    if (ret != TD_SUCCESS) {
        return;
    }

    blk_dev = drv_ctx->work_mode.block_dev;
    for (k = 0; k < drv_ctx->work_mode.block_num; k++) {
        isp_drv_set_pre_be_blc_reg(vi_pipe, drv_ctx, be_reg[k + blk_dev], be_blc);
        isp_drv_set_fpn_cfg(vi_pipe, drv_ctx, be_reg[k + blk_dev], &drv_ctx->be_sync_para.fpn_cfg);
        isp_drv_set_be_dgain(be_reg[k + blk_dev], drv_ctx->be_sync_para.isp_dgain[0]);
    }
}

static td_void isp_drv_dynamic_blc_update_stitch_sync(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx)
{
    td_u8 i;
    td_u8 stitch_idx = 0;
    isp_drv_ctx *drv_ctx_main_pipe = TD_NULL;

    for (i = 0; i < drv_ctx->stitch_attr.stitch_pipe_num; i++) {
        if (vi_pipe == drv_ctx->stitch_attr.stitch_bind_id[i]) {
            stitch_idx = i;
            break;
        }
    }

    drv_ctx_main_pipe = isp_drv_get_ctx(drv_ctx->stitch_attr.stitch_bind_id[0]);

    (td_void)memcpy_s(&drv_ctx_main_pipe->be_sync_para_stitch[stitch_idx].be_blc, sizeof(isp_be_blc_dyna_cfg),
                      &drv_ctx->be_sync_para.be_blc, sizeof(isp_be_blc_dyna_cfg));
}

static td_void isp_drv_dynamic_be_blc_get_idx(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx,
    td_u32 *pre_be_idx, td_u32 *post_be_idx)
{
    td_u8 i;
    td_u8 blc_idx = 0;
    td_bool blc_flag = TD_FALSE;

    if (drv_ctx->dyna_blc_info.sync_cfg.blc_mode == ISP_BLC_NORMAL ||
        drv_ctx->dyna_blc_info.sync_cfg.blc_mode == ISP_BLC_GLOBAL) {
        for (i = 0; i < DYNAMIC_BLC_BUF_NUM; i++) {
            if (drv_ctx->dyna_blc_info.blc_sync_id == drv_ctx->dyna_blc_info.be_blc_sync[i].blc_id) {
                blc_idx = i;
                blc_flag = TD_TRUE;
                break;
            }
        }
    }

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        *pre_be_idx = 0;
        *post_be_idx = 0;
        return;
    }

    /* 19dv500 not support, calculate index when pre_online_post_offline support */
    if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        *pre_be_idx = 0;
        if (isp_drv_get_ob_stats_update_pos(vi_pipe) == OT_ISP_UPDATE_OB_STATS_FE_FRAME_START) {
            *post_be_idx = 2; /* delay 2 */
        } else {
            *post_be_idx = 1;
        }

        if ((isp_drv_get_run_wakeup_sel(vi_pipe) == OT_ISP_RUN_WAKEUP_BE_END) && blc_flag == TD_TRUE) {
            *post_be_idx = blc_idx;
        }
        return;
    }

    if (is_offline_mode(drv_ctx->work_mode.running_mode) ||
        is_striping_mode(drv_ctx->work_mode.running_mode)) {
        /* The FE interrupt and the ob_stats interrupt are reported at the same time.
           It need to take FE_BLC effect at next frame, but the BE_BLC effect at this frame.
           No matter whether ob_pos is in Frame_START or in Frame_END, delay num is equal to 2.
        */
        *post_be_idx = 2; /* delay 2 */
        *pre_be_idx = 2; /* delay 2 */

        if ((isp_drv_get_run_wakeup_sel(vi_pipe) == OT_ISP_RUN_WAKEUP_BE_END) && blc_flag == TD_TRUE) {
            *post_be_idx = blc_idx;
            *pre_be_idx = blc_idx;
        }
    }

    return;
}

static td_void isp_drv_update_be_blc_sync_param(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
    isp_be_blc_dyna_cfg *pre_be_blc, isp_be_blc_dyna_cfg *post_be_blc)
{
    isp_be_blc_dyna_cfg *be_blc_sync = &drv_ctx->be_sync_para.be_blc;
    (td_void)memcpy_s(be_blc_sync, sizeof(isp_be_blc_dyna_cfg), pre_be_blc, sizeof(isp_be_blc_dyna_cfg));

    if (drv_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        isp_drv_dynamic_blc_update_stitch_sync(vi_pipe, drv_ctx);
    }
}

static td_s32 isp_drv_dynamic_be_blc_calc_reg(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    isp_be_blc_dyna_cfg *pre_be_blc = TD_NULL;
    isp_be_blc_dyna_cfg *post_be_blc = TD_NULL;
    td_u32 pre_be_idx = 0;
    td_u32 post_be_idx = 0;
    isp_check_no_fe_pipe_return(vi_pipe);
    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    isp_drv_dynamic_be_blc_get_idx(vi_pipe, drv_ctx, &pre_be_idx, &post_be_idx);

    pre_be_blc = &drv_ctx->dyna_blc_info.be_blc_sync[pre_be_idx];
    post_be_blc = &drv_ctx->dyna_blc_info.be_blc_sync[post_be_idx];

    isp_drv_update_be_blc_sync_param(vi_pipe, drv_ctx, pre_be_blc, post_be_blc);

    (td_void)memcpy_s(&drv_ctx->be_sync_para.fpn_cfg, sizeof(isp_fpn_sync_cfg),
                      &drv_ctx->dyna_blc_info.fpn_cfg[pre_be_idx], sizeof(isp_fpn_sync_cfg));

    if (isp_drv_get_ob_stats_update_pos(vi_pipe) != OT_ISP_UPDATE_OB_STATS_FE_FRAME_END) {
        return TD_SUCCESS;
    }

    if (is_online_mode(drv_ctx->work_mode.running_mode)) {
        isp_drv_dynamic_pre_be_blc_reg(vi_pipe, drv_ctx, pre_be_blc);
    } else if (is_pre_online_post_offline(drv_ctx->work_mode.running_mode)) {
        isp_drv_dynamic_pre_be_blc_reg(vi_pipe, drv_ctx, pre_be_blc);
    }

    return TD_SUCCESS;
}

static td_void isp_drv_dynamic_blc_update_actucal_info(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    td_u8 wdr_merge_frame = 1;
    isp_blc_actual_info *actual_info = &drv_ctx->dyna_blc_info.actual_info;

    if (is_2to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        wdr_merge_frame = 2; /* 2to1 wdr */
    } else if (is_3to1_wdr_mode(drv_ctx->wdr_attr.wdr_mode)) {
        wdr_merge_frame = 3; /* 3to1 wdr */
    }

    for (i = 0; i < wdr_merge_frame; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            actual_info->isp_black_level[i][j] = drv_ctx->dyna_blc_info.isp_black_level[i][j];
            actual_info->sns_black_level[i][j] = drv_ctx->dyna_blc_info.sns_black_level[i][j];
        }
    }

    for (i = wdr_merge_frame; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            actual_info->isp_black_level[i][j] = 0;
            actual_info->sns_black_level[i][j] = 0;
        }
    }

    actual_info->is_ready = TD_TRUE;
}

static td_s32 isp_drv_dynamic_blc_update_actucal_ori_info(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    td_u8 i, j;
    td_u8 wdr_merge_frame = 1;
    isp_blc_actual_info *actual_info = &drv_ctx->dyna_blc_info.actual_info;

    td_u8 k;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_reg_type *fe_reg = TD_NULL;

    if (drv_ctx->wdr_attr.is_mast_pipe == TD_FALSE) {
        return TD_SUCCESS;
    }

    wdr_merge_frame = drv_ctx->wdr_attr.pipe_num;

    for (k = 0; k < drv_ctx->wdr_attr.pipe_num; k++) {
        vi_pipe_bind = drv_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        isp_drv_fereg_ctx(vi_pipe_bind, fe_reg);
        actual_info->ori_black_level[k][OT_ISP_CHN_R]  = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value0;
        actual_info->ori_black_level[k][OT_ISP_CHN_GR] = fe_reg->isp_blc_dynamic_reg5.bits.isp_blc_dynamic_value1;
        actual_info->ori_black_level[k][OT_ISP_CHN_GB] = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value2;
        actual_info->ori_black_level[k][OT_ISP_CHN_B]  = fe_reg->isp_blc_dynamic_reg6.bits.isp_blc_dynamic_value3;
    }

    for (i = wdr_merge_frame; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
            actual_info->ori_black_level[i][j] = 0;
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_drv_reg_config_fe_dynamic_blc_single_pipe(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    isp_check_pointer_return(drv_ctx);
    if (drv_ctx->dyna_blc_info.sync_cfg.black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_SUCCESS;
    }

    isp_drv_dynamic_blc_update_actucal_ori_info(vi_pipe, drv_ctx);

    isp_drv_dynamic_blc_calc(drv_ctx);

    isp_drv_dynamic_fe_blc_calc_reg(drv_ctx);
    return TD_SUCCESS;
}

td_s32 isp_drv_reg_config_be_dynamic_blc_single_pipe(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    isp_check_pointer_return(drv_ctx);

    if (drv_ctx->dyna_blc_info.sync_cfg.black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_SUCCESS;
    }

    isp_drv_dynamic_be_blc_calc_reg(vi_pipe, drv_ctx);

    isp_drv_dynamic_blc_update_actucal_info(vi_pipe, drv_ctx);
    return TD_SUCCESS;
}

td_s32 isp_drv_reg_config_dynamic_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx)
{
    isp_check_pointer_return(drv_ctx);

    if (drv_ctx->dyna_blc_info.sync_cfg.black_level_mode != OT_ISP_BLACK_LEVEL_MODE_DYNAMIC) {
        return TD_SUCCESS;
    }

    isp_drv_reg_config_fe_dynamic_blc_single_pipe(vi_pipe, drv_ctx);
    isp_drv_reg_config_be_dynamic_blc_single_pipe(vi_pipe, drv_ctx);

    return TD_SUCCESS;
}

static __inline td_void isp_drv_aibnr_fpn_en_write(isp_fe_reg_type *fe_reg, td_bool fpn_en)
{
    u_isp_fe_ctrl o_isp_fe_ctrl;
    o_isp_fe_ctrl.u32 = fe_reg->isp_fe_ctrl.u32;
    o_isp_fe_ctrl.bits.isp_fpn_en = fpn_en;
    fe_reg->isp_fe_ctrl.u32 = o_isp_fe_ctrl.u32;
}

static __inline td_void isp_drv_fe_line_dly_en_write(isp_fe_reg_type *fe_reg, td_bool fpn_en)
{
    u_isp_fe_ctrl o_isp_fe_ctrl;
    o_isp_fe_ctrl.u32 = fe_reg->isp_fe_ctrl.u32;
    o_isp_fe_ctrl.bits.isp_line_dly_en = fpn_en;
    fe_reg->isp_fe_ctrl.u32 = o_isp_fe_ctrl.u32;
}

static __inline td_void isp_drv_aibnr_fpn_correct0_en_write(isp_fe_reg_type *fe_reg,
    td_u32 uisp_fpn_correct0_en)
{
    u_isp_fpn_corr_cfg o_isp_fpn_corr_cfg;
    o_isp_fpn_corr_cfg.u32 = fe_reg->isp_fpn_corr_cfg.u32;
    o_isp_fpn_corr_cfg.bits.isp_fpn_correct0_en = uisp_fpn_correct0_en;
    fe_reg->isp_fpn_corr_cfg.u32 = o_isp_fpn_corr_cfg.u32;
}
static __inline td_void isp_drv_aibnr_fpn_offset0_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_offset0)
{
    u_isp_fpn_corr0 o_isp_fpn_corr0;
    o_isp_fpn_corr0.u32 = fe_reg->isp_fpn_corr0.u32;
    o_isp_fpn_corr0.bits.isp_fpn_offset0 = uisp_fpn_offset0;
    fe_reg->isp_fpn_corr0.u32 = o_isp_fpn_corr0.u32;
}

static __inline td_void isp_drv_fpn_frame_calib_shift_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_frame_calib_shift)
{
    u_isp_fpn_shift o_isp_fpn_shift;
    o_isp_fpn_shift.u32 = fe_reg->isp_fpn_shift.u32;
    o_isp_fpn_shift.bits.isp_fpn_frame_calib_shift = uisp_fpn_frame_calib_shift;
    fe_reg->isp_fpn_shift.u32 = o_isp_fpn_shift.u32;
}

static __inline td_void isp_drv_fpn_out_shift_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_out_shift)
{
    u_isp_fpn_shift o_isp_fpn_shift;
    o_isp_fpn_shift.u32 = fe_reg->isp_fpn_shift.u32;
    o_isp_fpn_shift.bits.isp_fpn_out_shift = uisp_fpn_out_shift;
    fe_reg->isp_fpn_shift.u32 = o_isp_fpn_shift.u32;
}

static __inline td_void isp_drv_fpn_in_shift_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_in_shift)
{
    u_isp_fpn_shift o_isp_fpn_shift;
    o_isp_fpn_shift.u32 = fe_reg->isp_fpn_shift.u32;
    o_isp_fpn_shift.bits.isp_fpn_in_shift = uisp_fpn_in_shift;
    fe_reg->isp_fpn_shift.u32 = o_isp_fpn_shift.u32;
}

static __inline td_void isp_drv_fpn_shift_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_shift)
{
    u_isp_fpn_shift o_isp_fpn_shift;
    o_isp_fpn_shift.u32 = fe_reg->isp_fpn_shift.u32;
    o_isp_fpn_shift.bits.isp_fpn_shift = uisp_fpn_shift;
    fe_reg->isp_fpn_shift.u32 = o_isp_fpn_shift.u32;
}

static __inline td_void isp_drv_fpn_max_o_write(isp_fe_reg_type *fe_reg, td_u32 uisp_fpn_max_o)
{
    u_isp_fpn_max_o o_isp_fpn_max_o;
    o_isp_fpn_max_o.u32 = fe_reg->isp_fpn_max_o.u32;
    o_isp_fpn_max_o.bits.isp_fpn_max_o = uisp_fpn_max_o;
    fe_reg->isp_fpn_max_o.u32 = o_isp_fpn_max_o.u32;
}

td_s32 isp_drv_set_aibnr_fpn_cor_cfg(ot_vi_pipe vi_pipe, isp_fpn_cor_reg_cfg *fpn_cor_cfg)
{
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_check_pointer_return(fpn_cor_cfg);
    isp_check_pipe_return(vi_pipe);
    isp_check_no_fe_pipe_return(vi_pipe);
    if (vi_pipe != 0) {
        return TD_FAILURE;
    }

    isp_drv_fereg_ctx(vi_pipe, fe_reg);
    isp_check_pointer_return(fe_reg);

    isp_drv_aibnr_fpn_en_write(fe_reg, fpn_cor_cfg->correct_en);
    isp_drv_fe_line_dly_en_write(fe_reg, fpn_cor_cfg->correct_en);
    isp_drv_aibnr_fpn_correct0_en_write(fe_reg, fpn_cor_cfg->correct_en);
    isp_drv_aibnr_fpn_offset0_write(fe_reg, fpn_cor_cfg->offset);
    isp_drv_fpn_frame_calib_shift_write(fe_reg, fpn_cor_cfg->isp_fpn_frame_calibrate_shift);
    isp_drv_fpn_out_shift_write(fe_reg, fpn_cor_cfg->isp_fpn_out_shift);
    isp_drv_fpn_in_shift_write(fe_reg, fpn_cor_cfg->isp_fpn_in_shift);

    isp_drv_fpn_shift_write(fe_reg, fpn_cor_cfg->isp_fpn_shift);
    isp_drv_fpn_max_o_write(fe_reg, fpn_cor_cfg->isp_fpn_max);

    return TD_SUCCESS;
}
static td_void isp_drv_set_pre_stt_en(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_be_reg_type *be_reg)
{
    td_u32 stt_en = 0;
    u_isp_be_sys_ctrl isp_be_sys_ctrl;
    stt_en |= (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_PRE) ? alg_cfg->be_ctrl0.bits.isp_ae_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_PRE) ? alg_cfg->be_ctrl0.bits.isp_awb_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_PRE) ? alg_cfg->be_ctrl0.bits.isp_af_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_PRE) ? alg_cfg->be_ctrl1.bits.isp_la_en : 0;
    stt_en |= (alg_cfg->be_ctrl0.bits.isp_fpn_en && alg_cfg->be_ctrl0.bits.isp_fpn_mode == 1);
    stt_en |= (alg_cfg->be_ctrl0.bits.isp_dpc_en && alg_cfg->be_ctrl0.bits.isp_dpc_stat_en == 1);
    stt_en |= alg_cfg->be_ctrl0.bits.isp_flicker_en;

    isp_be_sys_ctrl.u32 = be_reg->isp_be_sys_ctrl.u32;
    isp_be_sys_ctrl.bits.isp_stt_en = stt_en;
    be_reg->isp_be_sys_ctrl.u32 = isp_be_sys_ctrl.u32;
}

static td_void isp_drv_set_post_stt_en(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_be_reg_type *be_reg)
{
    td_u32 stt_en = 0;
    u_isp_be_sys_ctrl isp_be_sys_ctrl;
    stt_en |= (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_POST) ? alg_cfg->be_ctrl0.bits.isp_ae_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_POST) ? alg_cfg->be_ctrl0.bits.isp_awb_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_POST) ? alg_cfg->be_ctrl0.bits.isp_af_en : 0;
    stt_en |= (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_POST) ? alg_cfg->be_ctrl1.bits.isp_la_en : 0;
    stt_en |= alg_cfg->be_ctrl1.bits.isp_ldci_en;
    stt_en |= alg_cfg->be_ctrl1.bits.isp_dehaze_en;
    stt_en |= alg_cfg->be_ctrl1.bits.isp_sumy_en;

    isp_be_sys_ctrl.u32 = be_reg->isp_be_sys_ctrl.u32;
    isp_be_sys_ctrl.bits.isp_stt_en = stt_en;
    be_reg->isp_be_sys_ctrl.u32 = isp_be_sys_ctrl.u32;
}

static td_void isp_drv_set_pre_be_ctrl(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_viproc_reg_type *viproc_reg)
{
    u_isp_viproc_ispbe_ctrl0 ctrl0;
    u_isp_viproc_ispbe_ctrl1 ctrl1;
    u_isp_viproc_ispbe_ctrl1 ctrl1_temp;

    ctrl0.u32 = viproc_reg->viproc_ispbe_ctrl0.u32;
    ctrl1.u32 = viproc_reg->viproc_ispbe_ctrl1.u32;
    // if in post, set alg_en ctrl = 0
    // ctrl0
    ctrl0.bits.isp_ae_en = (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_POST) ? 0 : ctrl0.bits.isp_ae_en;
    ctrl0.bits.isp_awb_en = (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_POST) ? 0 : ctrl0.bits.isp_awb_en;
    ctrl0.bits.isp_af_en = (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_POST) ? 0 : ctrl0.bits.isp_af_en;
    // ctrl1
    ctrl1_temp.u32 = ctrl1.u32;
    ctrl1.u32 = 0;
    ctrl1.bits.isp_drc_en = (alg_cfg->alg_pos.bits.bit_drc == ISP_ALG_IN_POST) ? 0: ctrl1_temp.bits.isp_drc_en;
    ctrl1.bits.isp_la_en = (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_POST) ? 0 : ctrl1_temp.bits.isp_la_en;

    alg_cfg->be_ctrl0.u32 = ctrl0.u32;
    alg_cfg->be_ctrl1.u32 = ctrl1.u32;
}

static td_void isp_drv_set_post_be_ctrl(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_viproc_reg_type *viproc_reg)
{
    u_isp_viproc_ispbe_ctrl0 ctrl0;
    u_isp_viproc_ispbe_ctrl1 ctrl1;
    u_isp_viproc_ispbe_ctrl0 ctrl0_temp;

    ctrl0.u32 = viproc_reg->viproc_ispbe_ctrl0.u32;
    ctrl1.u32 = viproc_reg->viproc_ispbe_ctrl1.u32;
    // if in pre, set alg_en ctrl = 0
    // ctrl0
    ctrl0_temp.u32 = ctrl0.u32;
    ctrl0.u32 = 0;
    ctrl0.bits.isp_ae_en = (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_POST) ? ctrl0_temp.bits.isp_ae_en : 0;
    ctrl0.bits.isp_awb_en = (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_POST) ? ctrl0_temp.bits.isp_awb_en : 0;
    ctrl0.bits.isp_af_en = (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_POST) ? ctrl0_temp.bits.isp_af_en : 0;
    // ctrl1
    ctrl1.bits.isp_drc_en = (alg_cfg->alg_pos.bits.bit_drc == ISP_ALG_IN_POST) ? ctrl1.bits.isp_drc_en : 0;
    ctrl1.bits.isp_la_en = (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_POST) ? ctrl1.bits.isp_la_en : 0;

    alg_cfg->be_ctrl0.u32 = ctrl0.u32;
    alg_cfg->be_ctrl1.u32 = ctrl1.u32;
}

static td_void isp_drv_set_pre_post_be_ctrl(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_viproc_reg_type *viproc_reg_pre, isp_viproc_reg_type *viproc_reg_post)
{
    u_isp_viproc_ispbe_ctrl0 ctrl0_pre;
    u_isp_viproc_ispbe_ctrl1 ctrl1_pre;
    u_isp_viproc_ispbe_ctrl0 ctrl0_post;
    u_isp_viproc_ispbe_ctrl1 ctrl1_post;
    u_isp_viproc_ispbe_ctrl0 ctrl0;
    u_isp_viproc_ispbe_ctrl1 ctrl1;

    ctrl0_pre.u32 = viproc_reg_pre->viproc_ispbe_ctrl0.u32;
    ctrl1_pre.u32 = viproc_reg_pre->viproc_ispbe_ctrl1.u32;
    ctrl0_post.u32 = viproc_reg_post->viproc_ispbe_ctrl0.u32;
    ctrl1_post.u32 = viproc_reg_post->viproc_ispbe_ctrl1.u32;
    // ctrl0
    ctrl0.u32 = ctrl0_pre.u32;
    ctrl0.bits.isp_ae_en =
        (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_POST) ? ctrl0_post.bits.isp_ae_en : ctrl0_pre.bits.isp_ae_en;
    ctrl0.bits.isp_awb_en =
        (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_POST) ? ctrl0_post.bits.isp_awb_en : ctrl0_pre.bits.isp_awb_en;
    ctrl0.bits.isp_af_en =
        (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_POST) ? ctrl0_post.bits.isp_af_en : ctrl0_pre.bits.isp_af_en;

    // ctrl1
    ctrl1.u32 = ctrl1_post.u32;
    ctrl1.bits.isp_drc_en =
        (alg_cfg->alg_pos.bits.bit_drc == ISP_ALG_IN_POST) ? ctrl1_post.bits.isp_drc_en : ctrl1_pre.bits.isp_drc_en;
    ctrl1.bits.isp_la_en =
        (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_POST) ? ctrl1_post.bits.isp_la_en : ctrl1_pre.bits.isp_la_en;

    alg_cfg->be_ctrl0.u32 = ctrl0.u32;
    alg_cfg->be_ctrl1.u32 = ctrl1.u32;
}

static td_void isp_drv_set_alg_pos(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_be_reg_type *be_reg)
{
    td_u32 ae_sel = be_reg->isp_be_module_pos.bits.isp_ae_sel;
    td_u32 awb_sel = be_reg->isp_be_module_pos.bits.isp_awb_sel;
    td_u32 af_sel = be_reg->isp_be_module_pos.bits.isp_af_sel;
    alg_cfg->alg_pos.pos = 0;
    if (work_param->aiisp_mode == OT_VI_AIISP_MODE_AIDRC) {
        alg_cfg->alg_pos.bits.bit_drc = ISP_ALG_IN_POST; // post, drc:1
        alg_cfg->alg_pos.bits.bit_la = ISP_ALG_IN_POST; // post, la:1
        alg_cfg->alg_pos.bits.bit_ae = (ae_sel == OT_ISP_AE_AFTER_DRC) ? ISP_ALG_IN_POST : ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_awb = (awb_sel == OT_ISP_AWB_AFTER_DRC) ? ISP_ALG_IN_POST : ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_af = (af_sel != OT_ISP_AF_STATS_AFTER_DGAIN) ? ISP_ALG_IN_POST : ISP_ALG_IN_PRE;
    } else { // AI_DM
        alg_cfg->alg_pos.bits.bit_drc = ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_la = ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_ae = ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_awb = ISP_ALG_IN_PRE;
        alg_cfg->alg_pos.bits.bit_af = (af_sel == OT_ISP_AF_STATS_AFTER_CSC) ? ISP_ALG_IN_POST : ISP_ALG_IN_PRE;
    }
}

static td_void isp_drv_set_pre_post_com_cfg(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, isp_be_reg_type *be_reg_pre, isp_be_reg_type *be_reg_post)
{
    /* use pre's cfg, 3a position cfg take effect immediately */
    be_reg_post->isp_be_module_pos.u32 = be_reg_pre->isp_be_module_pos.u32;
    /* blc is in pre, use pre's cfg */
    be_reg_post->isp_glb_blc_cfg.u32 = be_reg_pre->isp_glb_blc_cfg.u32;
    be_reg_post->isp_blc_cfg00.u32 = be_reg_pre->isp_blc_cfg00.u32;
    be_reg_post->isp_blc_cfg01.u32 = be_reg_pre->isp_blc_cfg01.u32;
    be_reg_post->isp_blc_cfg10.u32 = be_reg_pre->isp_blc_cfg10.u32;
    be_reg_post->isp_blc_cfg11.u32 = be_reg_pre->isp_blc_cfg11.u32;
    be_reg_post->isp_blc_cfg20.u32 = be_reg_pre->isp_blc_cfg20.u32;
    be_reg_post->isp_blc_cfg21.u32 = be_reg_pre->isp_blc_cfg21.u32;
    if (alg_cfg->alg_pos.bits.bit_drc == ISP_ALG_IN_PRE) {
        /* if drc is in pre, drc_dither is also in pre, use pre'cfg */
        be_reg_post->isp_drc_dither.u32 = be_reg_pre->isp_drc_dither.u32;
    }
}
static td_void isp_drv_set_stat_valid(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg, td_bool pre_valid, td_bool post_valid)
{
    isp_be_stat_valid stat_valid;
    stat_valid.key = 0xffffffff;
    /* in pre, use pre_valid, in post, use post_valid, if vi run pre_only, post_valid=0, the stat in post is wrong. */
    stat_valid.bits.bit_be_ae_stat = (alg_cfg->alg_pos.bits.bit_ae == ISP_ALG_IN_PRE) ? pre_valid : post_valid;
    stat_valid.bits.bit_awb_stat = (alg_cfg->alg_pos.bits.bit_awb == ISP_ALG_IN_PRE) ? pre_valid : post_valid;
    stat_valid.bits.bit_be_af_stat = (alg_cfg->alg_pos.bits.bit_af == ISP_ALG_IN_PRE) ? pre_valid : post_valid;
    stat_valid.bits.bit_mg_stat  = (alg_cfg->alg_pos.bits.bit_la == ISP_ALG_IN_PRE) ? pre_valid : post_valid;
    stat_valid.bits.bit_flicker = pre_valid;
    stat_valid.bits.bit_dp_stat = pre_valid;
    stat_valid.bits.bit_fpn_stat = pre_valid;
    stat_valid.bits.bit_ldci_stat = post_valid;
    stat_valid.bits.bit_dehaze_stat = post_valid;
    stat_valid.bits.bit_sumy_stat = post_valid;
    /* if ctrl_en is false, the stat is invalid. */
    /* for example, if stt_en is 1 and ae_en is 0, then the ae_stat is invalid, would not read and use the stat */
    stat_valid.bits.bit_be_ae_stat &= alg_cfg->be_ctrl0.bits.isp_ae_en;
    stat_valid.bits.bit_awb_stat &= alg_cfg->be_ctrl0.bits.isp_awb_en;
    stat_valid.bits.bit_be_af_stat &= alg_cfg->be_ctrl0.bits.isp_af_en;
    stat_valid.bits.bit_mg_stat &= alg_cfg->be_ctrl1.bits.isp_la_en;
    stat_valid.bits.bit_flicker &= alg_cfg->be_ctrl0.bits.isp_flicker_en;
    stat_valid.bits.bit_dp_stat &= (alg_cfg->be_ctrl0.bits.isp_dpc_en && alg_cfg->be_ctrl0.bits.isp_dpc_stat_en == 1);
    stat_valid.bits.bit_fpn_stat &= (alg_cfg->be_ctrl0.bits.isp_fpn_en && alg_cfg->be_ctrl0.bits.isp_fpn_mode == 1);
    stat_valid.bits.bit_ldci_stat &= alg_cfg->be_ctrl1.bits.isp_ldci_en;
    stat_valid.bits.bit_dehaze_stat &= alg_cfg->be_ctrl1.bits.isp_dehaze_en;
    stat_valid.bits.bit_sumy_stat &= alg_cfg->be_ctrl1.bits.isp_sumy_en;
    alg_cfg->stat_valid = stat_valid.key;
}
static td_void isp_drv_process_be_pre_cfg(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg)
{
    td_u32 split_index = work_param->split_index;
    isp_be_wo_cfg_buf *be_cfg_pre = work_param->be_cfg_buf_pre;
    isp_be_reg_type *be_reg = (isp_be_reg_type *)be_cfg_pre->be_vir_addr[split_index];
    isp_viproc_reg_type *viproc_reg = (isp_viproc_reg_type *)be_cfg_pre->viproc_vir_addr[split_index];

    isp_debug_trace("before ctrl[0-1]:0x%x, 0x%x, 3a_pos:0x%x\n", viproc_reg->viproc_ispbe_ctrl0.u32,
        viproc_reg->viproc_ispbe_ctrl1.u32, be_reg->isp_be_module_pos.u32);

    /* set drc 3a la position */
    isp_drv_set_alg_pos(work_param, alg_cfg, be_reg);

    /* if in post, need to close */
    isp_drv_set_pre_be_ctrl(work_param, alg_cfg, viproc_reg);

    /* if in post, check stt_en == 0; */
    isp_drv_set_pre_stt_en(work_param, alg_cfg, be_reg);

    /* pre_valid:true, post_valid:false */
    isp_drv_set_stat_valid(work_param, alg_cfg, TD_TRUE, TD_FALSE);

    isp_debug_trace("after ctrl[0-1]:0x%x, 0x%x, alg_pos:0x%x, stat_valid:0x%x, stt_en:%d\n",
        alg_cfg->be_ctrl0.u32, alg_cfg->be_ctrl1.u32, alg_cfg->alg_pos.pos, alg_cfg->stat_valid,
        be_reg->isp_be_sys_ctrl.bits.isp_stt_en);
    return;
}

static td_void isp_drv_process_be_post_cfg(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg)
{
    td_u32 split_index = work_param->split_index;
    isp_be_wo_cfg_buf *be_cfg_post = work_param->be_cfg_buf_post;
    isp_be_reg_type *be_reg = (isp_be_reg_type *)be_cfg_post->be_vir_addr[split_index];
    isp_viproc_reg_type *viproc_reg = (isp_viproc_reg_type *)be_cfg_post->viproc_vir_addr[split_index];

    isp_debug_trace("before ctrl[0-1]:0x%x, 0x%x, 3a_pos:0x%x\n", viproc_reg->viproc_ispbe_ctrl0.u32,
        viproc_reg->viproc_ispbe_ctrl1.u32, be_reg->isp_be_module_pos.u32);

    /* set drc 3a la position */
    isp_drv_set_alg_pos(work_param, alg_cfg, be_reg);

    /* if in post, need to close */
    isp_drv_set_post_be_ctrl(work_param, alg_cfg, viproc_reg);

    /* if in post, check stt_en == 0; */
    isp_drv_set_post_stt_en(work_param, alg_cfg, be_reg);

    /* pre_valid:false, post_valid:true */
    isp_drv_set_stat_valid(work_param, alg_cfg, TD_FALSE, TD_TRUE);

    isp_debug_trace("after ctrl[0-1]:0x%x, 0x%x, alg_pos:0x%x, stat_valid:0x%x, stt_en:%d\n",
        alg_cfg->be_ctrl0.u32, alg_cfg->be_ctrl1.u32, alg_cfg->alg_pos.pos, alg_cfg->stat_valid,
        be_reg->isp_be_sys_ctrl.bits.isp_stt_en);
}

static td_void isp_drv_process_be_pre_post_cfg(isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg)
{
    td_u32 split_index = work_param->split_index;
    isp_be_wo_cfg_buf *be_cfg_pre = work_param->be_cfg_buf_pre;
    isp_be_wo_cfg_buf *be_cfg_post = work_param->be_cfg_buf_post;
    isp_be_reg_type *be_reg_pre = (isp_be_reg_type *)be_cfg_pre->be_vir_addr[split_index];
    isp_be_reg_type *be_reg_post = (isp_be_reg_type *)be_cfg_post->be_vir_addr[split_index];
    isp_viproc_reg_type *viproc_reg_pre = (isp_viproc_reg_type *)be_cfg_pre->viproc_vir_addr[split_index];
    isp_viproc_reg_type *viproc_reg_post = (isp_viproc_reg_type *)be_cfg_post->viproc_vir_addr[split_index];
    u_isp_be_sys_ctrl isp_be_sys_ctrl;

    isp_debug_trace("before pre:ctrl[0-1]:0x%x, 0x%x, 3a_pos:0x%x, post:ctrl[0-1]:0x%x, 0x%x, 3a_pos:0x%x\n",
        viproc_reg_pre->viproc_ispbe_ctrl0.u32, viproc_reg_pre->viproc_ispbe_ctrl1.u32,
        be_reg_pre->isp_be_module_pos.u32, viproc_reg_post->viproc_ispbe_ctrl0.u32,
        viproc_reg_post->viproc_ispbe_ctrl1.u32, be_reg_post->isp_be_module_pos.u32);

    /* set drc 3a la position, use pre cfg, cfg take effect immediately. */
    isp_drv_set_alg_pos(work_param, alg_cfg, be_reg_pre);

    /* if in post, need to close */
    isp_drv_set_pre_post_be_ctrl(work_param, alg_cfg, viproc_reg_pre, viproc_reg_post);

    /* if in post, check stt_en == 0; */
    isp_drv_set_pre_stt_en(work_param, alg_cfg, be_reg_pre);
    isp_drv_set_post_stt_en(work_param, alg_cfg, be_reg_post);
    /* set to post's com_cfg, change stt_en */
    isp_be_sys_ctrl.u32 = be_reg_post->isp_be_sys_ctrl.u32;
    isp_be_sys_ctrl.bits.isp_stt_en |= be_reg_pre->isp_be_sys_ctrl.bits.isp_stt_en;
    be_reg_post->isp_be_sys_ctrl.u32 = isp_be_sys_ctrl.u32;

    /* set to post's com_cfg */
    isp_drv_set_pre_post_com_cfg(work_param, alg_cfg, be_reg_pre, be_reg_post);

    /* pre_valid:true, post_valid:true */
    isp_drv_set_stat_valid(work_param, alg_cfg, TD_TRUE, TD_TRUE);

    isp_debug_trace("after be_ctrl[0-1]:0x%x, 0x%x, 3a_pos:0x%x, alg_pos:0x%x, stat_valid:0x%x, stt_en:%d\n",
        alg_cfg->be_ctrl0.u32, alg_cfg->be_ctrl1.u32, be_reg_post->isp_be_module_pos.u32, alg_cfg->alg_pos.pos,
        alg_cfg->stat_valid, be_reg_post->isp_be_sys_ctrl.bits.isp_stt_en);
}

td_void isp_drv_get_ai_be_alg_pos(ot_vi_pipe vi_pipe, isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg)
{
    isp_ai_alg_cfg cfg;
    td_u32 be_mask = work_param->be_mask;
    if (be_mask == 0x1) { /* only run pre */
        isp_drv_process_be_pre_cfg(work_param, &cfg);
    } else if (be_mask == 0x2) { /* only run post */
        isp_drv_process_be_post_cfg(work_param, &cfg);
    } else { /* run pre and post */
        isp_drv_process_be_pre_post_cfg(work_param, &cfg);
    }
    alg_cfg->be_ctrl0.u32 = cfg.be_ctrl0.u32;
    alg_cfg->be_ctrl1.u32 = cfg.be_ctrl1.u32;
    alg_cfg->alg_pos.pos = cfg.alg_pos.pos;
    alg_cfg->stat_valid = cfg.stat_valid;

    return;
}
