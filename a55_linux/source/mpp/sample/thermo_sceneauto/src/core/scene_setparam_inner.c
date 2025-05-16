/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <unistd.h>
#include <string.h>
#include "securec.h"
#include "ss_mpi_vpss.h"
#include "ss_mpi_vi.h"
#include "ot_scenecomm.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_aidrc.h"
#include "ss_mpi_ai3dnr.h"
#include "scene_setparam_inner.h"

#ifdef __cplusplus
extern "C" {
#endif

ot_scene_pipe_param g_scene_pipe_param[OT_SCENE_PIPETYPE_NUM];
td_bool g_isp_pause = TD_FALSE;

ot_scene_pipe_param *get_pipe_params(td_void)
{
    return g_scene_pipe_param;
}

td_bool *get_isp_pause(td_void)
{
    return &g_isp_pause;
}

td_u32 get_level_ltoh(td_u32 value, td_u32 level, td_u32 count, const td_u32 *thresh)
{
    for (level = 0; level < count; level++) {
        if (value <= thresh[level]) {
            break;
        }
    }

    level = (level == count) ? (count - 1) : level;
    return level;
}

td_u32 scene_get_level_ltoh_u32(td_u32 value, td_u32 count, const td_u32 *thresh)
{
    td_u32 level;
    for (level = 0; level < count; level++) {
        if (value <= thresh[level]) {
            break;
        }
    }

    if (level == count) {
        level = count - 1;
    }
    return level;
}

td_u32 scene_interpulate(td_u64 middle, td_u64 left, td_u64 left_value, td_u64 right, td_u64 right_value)
{
    td_u32 value_tmp, k;

    if (middle <= left) {
        value_tmp = (td_u32)left_value;
        return value_tmp;
    }
    if (middle >= right) {
        value_tmp = (td_u32)right_value;
        return value_tmp;
    }

    k = (td_u32)(right - left);
    value_tmp = (td_u32)(((right - middle) * left_value + (middle - left) * right_value + (k >> 1)) / k);
    return value_tmp;
}

td_u32 scene_time_filter(td_u32 param0, td_u32 param1, td_u32 time_cnt, td_u32 index)
{
    td_u64 temp;
    td_u32 value;

    if (param0 > param1) {
        temp = (td_u64)(param0 - param1) << 8; /* 8 is param to time_cnt */
        temp = ((temp * (index + 1)) / scene_div_0to1(time_cnt)) >> 8; /* 8 is time_cnt to param */
        value = (param0 > ((td_u32)temp + 1)) ? (param0 - (td_u32)temp - 1) : 0;
    } else if (param0 < param1) {
        temp = (td_u64)(param1 - param0) << 8; /* 8 is param to time_cnt */
        temp = ((temp * (index + 1)) / scene_div_0to1(time_cnt)) >> 8; /* 8 is time_cnt to param */
        value = (param0 + (td_u32)temp + 1) > param1 ? param1 : (param0 + (td_u32)temp + 1);
    } else {
        value = param0;
    }
    return value;
}


static void print_para_n(const ot_scene_3dnr *p_x, const t_v200_vpss_adv_iey *pai, const t_v200_vpss_adv_sfy *pas,
    const t_v200_vpss_tfy *pt, const t_v200_vpss_sfy *ps, const t_v200_vpss_iey *pi)
{
    printf("\n\n                             sizeof( X ) = %zu ", sizeof(*p_x));
    printf("\n**************N0**********|***********N1***********|*******N2*********|**************N3******************"
        "*****");
    printf("\n -en          %3u:        |              %3u:      |       %3u:       |               %3u:               "
        "     |", p_x->nry0_en, p_x->nry1_en, p_x->nry2_en, p_x->nry3_en);
    printf("\n -nXsf1       %3u:        |          %3u:    %3u:  |    %3u:    %3u:  |            %3u:    %3u:          "
        "     |", pas[0].sfs1, ps[1].sfs1, ps[1].sbr1,
        ps[2].sfs1, ps[2].sbr1, ps[3].sfs1, ps[3].sbr1); /* 2,3 is index */
    printf("\n -nXsf2       %3u:        |          %3u:%3u:%3u:  |    %3u:%3u:%3u:  |            %3u:%3u:%3u:          "
        "     |", pai[0].aux_ies, _tmprt3x(ps, sfs2, sft2, sbr2));
    printf("\n -nXsf4                   |          %3u:%3u:%3u:  |    %3u:%3u:%3u:  |            %3u:%3u:%3u:          "
        "     |", _tmprt3x(ps, sfs4, sft4, sbr4));
    printf("\n -nXsfs5     %3u:%3u:%3u: |                        |                  |            %3u:%3u:%3u:          "
        "     |", pas[0].sfs5, pas[0].sft5, pas[0].sbr5, ps[4].sfs2, ps[4].sft2, ps[4].sbr2); /* 4 is index */
    printf("\n -nXsf5  %3u:%3u:%3u:%3u: |        %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u: | "
        " %3u:%3u:%3u:%3u: |", pai[0].ies0, pai[0].ies1, pai[0].ies2,
        pai[0].ies3, _tmprt4_4(pi, ies0, ies1, ies2, ies3));
    printf("\n -nXsht5         %3u:%3u: |                        |                  | -nXsht5  %3u:%3u: | "
        "         %3u:%3u: |", pai[0].o_sht, pai[0].u_sht, pi[3].o_sht, /* 3 is index */
        pi[3].u_sht, pi[4].o_sht, pi[4].u_sht); /* 3, 4 is index */
    printf("\n                          |                        |-kmode      %3u:  |              %3u: |              "
        "     |", ps[2].k_mode, ps[3].k_mode); /* 2, 3 is index */

    printf("\n -nXsf3      %3u:%3u:%3u: |                        |                  |                   |              "
        "     |", pas[0].spn3, pas[0].sbn3, pas[0].pbr3);

    printf("\n -nXsf4      %3u:%3u:%3u: |                        |                  |                   |              "
        "     |", pas[0].spn4, pas[0].sbn4, pas[0].pbr4);
    printf("\n -nXsf6  %3u:%3u:%3u:%3u: |        %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u: | "
        " %3u:%3u:%3u:%3u: |", pas[0].spn6, pas[0].sbn6, pas[0].pbr6,
        pas[0].j_mode6, _tmprt4_4(ps, spn, sbn, pbr, j_mode));
    printf("\n -nXsfr6 %3u:%3u:%3u:%3u: |        %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u: | "
        " %3u:%3u:%3u:%3u: |", pas[0].sfr6[0], pas[0].sfr6[1], pas[0].sfr6[2], pas[0].sfr6[3], /* 2.3 is index */
        _tmprt4_4(ps, sfr6[0], sfr6[1], sfr6[2], sfr6[3])); /* 2, 3 is index */
    printf("\n -nXsbr6         %3u:%3u: |             %3u:%3u:   |          %3u:%3u:|          %3u:%3u: | "
        "         %3u:%3u: |", pas[0].sbr6[0], pas[0].sbr6[1], _tmprt2_4x(ps, sbr6[0], sbr6[1]));
    printf("\n                          |                        |                  |                   |              "
        "     |");
    printf("\n -nXsfn  %3u:%3u:%3u:%3u: |        %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u: | "
        " %3u:%3u:%3u:%3u: |",
        pas[0].sfn0, pas[0].sfn1, pas[0].sfn2, pas[0].sfn3, _tmprt4_4(ps, sfn0_0, sfn1_0, sfn2_0, sfn3_0));

    printf("\n -nXsth      %3u:%3u:%3u: |            %3u:%3u:%3u:|      %3u:%3u:%3u:|      %3u:%3u:%3u: | "
        "     %3u:%3u:%3u: |", pas[0].sth1, pas[0].sth2, pas[0].sth3, _tmprt3_4(ps, sth1_0, sth2_0, sth3_0));
    printf("\n *************************|-nX2sfn %3u:%3u:%3u:%3u:|  %3u:%3u:%3u:%3u:|                                  "
        "     |", _tmprt4_2(ps, sfn0_1, sfn1_1, sfn2_1, sfn3_1));
    printf("\n -nX2sf1      %3u:        |-nX2sth     %3u:%3u:%3u:|      %3u:%3u:%3u:|                                  "
        "     |", pas[1].sfs1, _tmprt3z(ps, 1, 2, sth1_1, sth2_1, sth3_1)); /* 2, is index */
    printf("\n -nX2sf2      %3u:        |                        |                  |-nXbIdx         %3u:%3u:          "
        "     |", pai[1].aux_ies, ps[3].bri_idx0, ps[3].bri_idx1); /* 2, 3 is index */
}

static void print_nc0(const ot_scene_3dnr *p_x, const t_v200_vpss_tfy *pt, const t_v200_vpss_adv_iey *pai,
    const t_v200_vpss_adv_sfy *pas, const t_v200_vpss_mdy *pm)
{
    printf("\n                          |-ref          %3u:      |                  "
        "|********************nrc0****************", pt[1].ref_en);
    printf("\n -nX2sf5 %3u:%3u:%3u:%3u: |-preMath      %3u:      |                  |  -nC0en             %3u:       "
        "       |", pai[1].ies0, pai[1].ies1, pai[1].ies2, pai[1].ies3, pt[0].auto_math, p_x->nrc0_en);
    printf("\n -nX2sht5        %3u:%3u: |-preTFS       %3u:      |                  |  -presfc            %3u:       "
        "       |", pai[1].o_sht, pai[1].u_sht, pt[0].tfs0, p_x->nrc0.pre_sfc); /* 1 is index */
    printf("\n                          |-dciW         %3u:      |                  |  -sfc               %3u:       "
        "       |", pm[0].dci_w, p_x->nrc0.sfc); /* 0 is index */
    printf("\n -nX2sf3     %3u:%3u:%3u: |-nXtss        %3u:      |     %3u:%3u:     |  -sfcExt          %3u:%3u:     "
        "       |", pas[1].spn3, pas[1].sbn3, pas[1].pbr3, pt[1].tss0, pt[2].tss0,    /* 2 index */
        pt[2].tss1, p_x->nrc0.sfc_enhance, p_x->nrc0.sfc_ext); /* 2 index */
    printf("\n -nX2sf4     %3u:%3u:%3u: |-nXtfs        %3u:      |     %3u:%3u:     |  -tpc               %3u:       "
        "       |", pas[1].spn4, pas[1].sbn4, pas[1].pbr4, pt[1].tfs0, pt[2].tfs0, /* 2 index */
        pt[2].tfs1, p_x->nrc0.tpc); /* 2 index */
    printf("\n -nX2sf6 %3u:%3u:%3u:%3u: |-nXtfr0   %3u:%3u:%3u:  |   %3u:%3u:%3u:   |  -tfc               %3u:       "
        "       |", pas[1].spn6, pas[1].sbn6, pas[1].pbr6, pas[1].j_mode6,
        _tmprt3z(pt, 1, 2, tfr0[0], tfr0[1], tfr0[2]), p_x->nrc0.tfc); /* 2 index  */
    printf("\n -nX2sfr6%3u:%3u:%3u:%3u: |          %3u:%3u:%3u:  |   %3u:%3u:%3u:   |  -trc               %3u:       "
        "       |", pas[1].sfr6[0], pas[1].sfr6[1], pas[1].sfr6[2], pas[1].sfr6[3], /* 2, 3 index  */
        _t3_(pt, 1, tfr0[3], tfr0[4], tfr0[5]), /*  3,4,5, index */
        _t3_(pt, 2, tfr0[3], tfr0[4], tfr0[5]), p_x->nrc0.trc); /*  2,3,4,5, index */
}

static void print_nc1(const t_v200_vpss_nrc1 *pc, const t_v200_vpss_tfy *pt,
    const t_v200_vpss_adv_sfy *pas, const t_v200_vpss_mdy *pm, const ot_scene_3dnr *p_x)
{
    printf("\n -nX2sbr6        %3u:%3u: |-nXtfr1  %3u:%3u:%3u:   |   %3u:%3u:%3u:   "
        "|********************nrc1***************|",
        pas[1].sbr6[0], pas[1].sbr6[1], _tmprt3z(pt, 1, 2, tfr1[0], tfr1[1], tfr1[2])); /*  2 index */
    printf("\n                          |         %3u:%3u:%3u:   |   %3u:%3u:%3u:   |                                  "
        "     |", _tmprt3z(pt, 1, 2, tfr1[3], tfr1[4], tfr1[5])); /*  2,3,4,5, index */
    printf("\n -nX2sfn %3u:%3u:%3u:%3u: |-maMode     %3u:%3u:    |                  | -nC1en              %3u:         "
        "     |", pas[1].sfn0, pas[1].sfn1, pas[1].sfn2, pas[1].sfn3,  pt[0].math_mode, pt[1].math_mode, p_x->nrc1_en);
    printf("\n -nX2sth     %3u:%3u:%3u: |                        |                  | -nCsf2          %3u:%3u:%3u:     "
        "     |", pas[1].sth1, pas[1].sth2, pas[1].sth3, pc->sfs2, pc->sft2, pc->sbr2);
    printf("\n                          |-advMath    %3u:%3u:    |-mcth       %3u:  |                                  "
        "     |", pm[0].adv_math, pm[1].adv_math, pm[0].mcth);

    printf("\n -mXmath             %3u: |            %3u:%3u:    |     %3u:%3u:     | -nCstr           %3u:| %3u:      "
        "     |", pas[0].bld1,  pm[0].math0, pm[0].math1, pm[1].math0, pm[1].math1, pc->pbr0, pc->pbr1);

    printf("\n -advMode            %3u: |-mXmate     %3u:%3u:    |     %3u:%3u:     |                                  "
        "     |", p_x->adv_mode, pm[0].mate0, pm[0].mate1, pm[1].mate0, pm[1].mate1);
    printf("\n -nXbIdx         %3u:%3u: |-mXmabw     %3u:%3u:    |     %3u:%3u:     |                                  "
        "     |", pas[0].bri_idx, pas[1].bri_idx, pm[0].mabw0, pm[0].mabw1, pm[1].mabw0, pm[1].mabw1);
}

static void print_nrx(const ot_scene_3dnr *p_x)
{
    const t_v200_vpss_adv_iey *pai = p_x->adv_ie_y;
    const t_v200_vpss_adv_sfy *pas = p_x->adv_sf_y;
    const t_v200_vpss_nrc1    *pc  = &p_x->nrc1;
    const t_v200_vpss_tfy     *pt  = p_x->tfy;
    const t_v200_vpss_sfy     *ps  = p_x->sfy;
    const t_v200_vpss_iey     *pi  = p_x->iey;
    const t_v200_vpss_mdy     *pm  = p_x->mdy;

    print_nc0(p_x, pt, pai, pas, pm);
    print_nc1(pc, pt, pas, pm, p_x);
    print_para_n(p_x, pai, pas, pt, ps, pi);

    printf("\n*********************************************************************************************************"
        "******");
    printf("\n -n3bri0    %6u: %6u: %6u: %6u:  -n3bri4  %6u: %6u: %6u: %6u:  ",
        _t4a_0_(ps, 3, bri_str), _t4a_4_(ps, 3, bri_str)); /* 3 index */
    printf("\n -n3bri8    %6u: %6u: %6u: %6u:  -n3bri12 %6u: %6u: %6u: %6u: %6u:  ",
        _t4a_8_(ps, 3, bri_str), _t4a_12_(ps, 3, bri_str), ps[3].bri_str[0x10]); /* 3, 0x10 index */
    printf("\n -n2sbs0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n2sbs8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:",
        _t4a_0_(ps, 2, sbs_k), _t4a_4_(ps, 2, sbs_k), _t4a_8_(ps, 2, sbs_k), _t4a_12_(ps, 2, sbs_k)); /* 2 index */
    printf("\n -n2sbs0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n2sbs8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:%3u:",
        _t4a_10_(ps, 2, sbs_k), _t4a_14_(ps, 2, sbs_k), _t4a_18_(ps, 2, sbs_k), /* 2 index */
        _t4a_1c_(ps, 2, sbs_k), ps[2].sbs_k[0x20]); /* 2  0x20  index */

    printf("\n -n2sds0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n2sds8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:",
        _t4a_0_(ps, 2, sds_k), _t4a_4_(ps, 2, sds_k), _t4a_8_(ps, 2, sds_k), _t4a_12_(ps, 2, sds_k)); /* 2 index */
    printf("\n -n2sds0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n2sds8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:%3u:",
        _t4a_10_(ps, 2, sds_k), _t4a_14_(ps, 2, sds_k), _t4a_18_(ps, 2, sds_k), /* 2 index */
        _t4a_1c_(ps, 2, sds_k), ps[2].sds_k[0x20]); /* 2  0x20  index */

    printf("\n -n3sbs0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n3sbs8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:",
        _t4a_0_(ps, 3, sbs_k), _t4a_4_(ps, 3, sbs_k), _t4a_8_(ps, 3, sbs_k), _t4a_12_(ps, 3, sbs_k)); /* 3 index */
    printf("\n -n3sbs0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n3sbs8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:%3u:",
        _t4a_10_(ps, 3, sbs_k), _t4a_14_(ps, 3, sbs_k), _t4a_18_(ps, 3, sbs_k), /* 3 index */
        _t4a_1c_(ps, 3, sbs_k), ps[3].sbs_k[0x20]); /* 3  0x20 index */

    printf("\n -n3sds0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n3sds8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:",
        _t4a_0_(ps, 3, sds_k), _t4a_4_(ps, 3, sds_k), _t4a_8_(ps, 3, sds_k), _t4a_12_(ps, 3, sds_k)); /* 3 index */
    printf("\n -n3sds0   %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:    -n3sds8 %3u:%3u:%3u:%3u: %3u:%3u:%3u:%3u:%3u:",
        _t4a_10_(ps, 3, sds_k), _t4a_14_(ps, 3, sds_k), _t4a_18_(ps, 3, sds_k), /* 3 index */
        _t4a_1c_(ps, 3, sds_k), ps[3].sds_k[0x20]); /* 3  0x20 index */
}

static td_void scene_set_adv_iey(ot_ai3dnr_attr *ai3dnr_attr, const ot_scene_3dnr *_3dnr)
{
    ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_mode =  _3dnr->adv_mode;
    for (td_u32 i = 0; i < scene_array_size(_3dnr->adv_ie_y); i++) {
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].ies0 = _3dnr->adv_ie_y[i].ies0;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].ies1 = _3dnr->adv_ie_y[i].ies1;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].ies2 = _3dnr->adv_ie_y[i].ies2;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].ies3 = _3dnr->adv_ie_y[i].ies3;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].aux_ies
            = _3dnr->adv_ie_y[i].aux_ies;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].u_sht = _3dnr->adv_ie_y[i].u_sht;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_ie_y[i].o_sht = _3dnr->adv_ie_y[i].o_sht;
    }
}

static td_void scene_set_adv_sfy(ot_ai3dnr_attr *ai3dnr_attr, const ot_scene_3dnr *_3dnr)
{
    td_u32 i, j;
    for (i = 0; i < scene_array_size(_3dnr->adv_sf_y); i++) {
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sth1 = _3dnr->adv_sf_y[i].sth1;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sth2 = _3dnr->adv_sf_y[i].sth2;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sth3 = _3dnr->adv_sf_y[i].sth3;

        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].spn4 = _3dnr->adv_sf_y[i].spn4;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sbn4 = _3dnr->adv_sf_y[i].sbn4;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].pbr4 = _3dnr->adv_sf_y[i].pbr4;

        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].spn6 = _3dnr->adv_sf_y[i].spn6;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sbn6 = _3dnr->adv_sf_y[i].sbn6;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].pbr6 = _3dnr->adv_sf_y[i].pbr6;

        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].spn3 = _3dnr->adv_sf_y[i].spn3;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sbn3 = _3dnr->adv_sf_y[i].sbn3;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].pbr3 = _3dnr->adv_sf_y[i].pbr3;

        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfn0 = _3dnr->adv_sf_y[i].sfn0;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfn1 = _3dnr->adv_sf_y[i].sfn1;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfn2 = _3dnr->adv_sf_y[i].sfn2;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfn3 = _3dnr->adv_sf_y[i].sfn3;

        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfs1 = _3dnr->adv_sf_y[i].sfs1;
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].j_mode6
            = _3dnr->adv_sf_y[i].j_mode6;
        for (j = 0; j < 4; j++) { /* 4  is index */
            ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sfr6[j]
                = _3dnr->adv_sf_y[i].sfr6[j];
        }
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sbr6[0]
            = _3dnr->adv_sf_y[i].sbr6[0];
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].sbr6[1]
            = _3dnr->adv_sf_y[i].sbr6[1];
        ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[i].bri_idx
            = _3dnr->adv_sf_y[i].bri_idx;
    }

    ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[0].bld1 = _3dnr->adv_sf_y[0].bld1;
    ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[0].sfs5 = _3dnr->adv_sf_y[0].sfs5;
    ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[0].sft5 = _3dnr->adv_sf_y[0].sft5;
    ai3dnr_attr->adv_param.nr_adv_param_v1.manual_adv_param.adv_param.adv_sf_y[0].sbr5 = _3dnr->adv_sf_y[0].sbr5;
}

static td_void scene_set_nry_iey(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    /* ie_y */
    for (td_u32 i = 1; i < scene_array_size(_3dnr->iey); i++) {
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].ies0 = _3dnr->iey[i].ies0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].ies1 = _3dnr->iey[i].ies1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].ies2 = _3dnr->iey[i].ies2;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].ies3 = _3dnr->iey[i].ies3;
    }

    for (td_u32 i = 3; i <= 4; i++) { /* 4  is index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].o_sht = _3dnr->iey[i].o_sht;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.iey[i].u_sht = _3dnr->iey[i].u_sht;
    }
}

static td_void scene_set_nry_en(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nry0_en = _3dnr->nry0_en;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nry1_en = _3dnr->nry1_en;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nry2_en = _3dnr->nry2_en;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nry3_en = _3dnr->nry3_en;
}

static td_void scene_set_nry_sfy_1to4(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    /* sf_y */
    td_u32 i;
    for (i = 1; i < 4; i++) { /* array sfy have 4 valid element */
        /* sfs1 */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfs1 = _3dnr->sfy[i].sfs1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbr1 = _3dnr->sfy[i].sbr1;

        /* sfs4 */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfs4 = _3dnr->sfy[i].sfs4;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sft4 = _3dnr->sfy[i].sft4;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbr4 = _3dnr->sfy[i].sbr4;

        /* sfs2 */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfs2 = _3dnr->sfy[i].sfs2;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sft2 = _3dnr->sfy[i].sft2;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbr2 = _3dnr->sfy[i].sbr2;
    }

    for (i = 0; i < OT_SCENE_3DNR_IDX_LEN; i++) { /*  3 sfy index */
        /* 3 index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].bri_str[i] = _3dnr->sfy[3].bri_str[i];
    }

    /* sfs4 */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[4].sfs2 = _3dnr->sfy[4].sfs2; /* 4  is index */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[4].sft2 = _3dnr->sfy[4].sft2; /* 4  is index */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[4].sbr2 = _3dnr->sfy[4].sbr2; /* 4  is index */

    for (i = 0; i < 33; i++) { /*  33 sfy index */
        /* 3 index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].sbs_k[i] = _3dnr->sfy[3].sbs_k[i]; /* 3 is index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].sds_k[i] = _3dnr->sfy[3].sds_k[i]; /* 3 is index */
        /* 2 index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[2].sbs_k[i] = _3dnr->sfy[2].sbs_k[i]; /* 2 is index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[2].sds_k[i] = _3dnr->sfy[2].sds_k[i]; /* 2 is index */
    }
}

static td_void scene_set_nry_sfy_6to8(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    td_u32 i, j;
    for (i = 1; i < scene_array_size(_3dnr->sfy); i++) {
        /* sfs7 */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].spn = _3dnr->sfy[i].spn;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbn = _3dnr->sfy[i].sbn;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].pbr = _3dnr->sfy[i].pbr;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].j_mode = _3dnr->sfy[i].j_mode;

        /* sfr6 */
        for (j = 0; j < scene_array_size(_3dnr->sfy[i].sfr6); j++) {
            nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfr6[j] = _3dnr->sfy[i].sfr6[j];
        }
        /* sth */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth1_0 = _3dnr->sfy[i].sth1_0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth2_0 = _3dnr->sfy[i].sth2_0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth3_0 = _3dnr->sfy[i].sth3_0;

        /* sbr6 */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbr6[0] = _3dnr->sfy[i].sbr6[0];
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sbr6[1] = _3dnr->sfy[i].sbr6[1];

        /* sfn */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn0_0 = _3dnr->sfy[i].sfn0_0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn1_0 = _3dnr->sfy[i].sfn1_0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn2_0 = _3dnr->sfy[i].sfn2_0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn3_0 = _3dnr->sfy[i].sfn3_0;
    }

    for (i = 1; i <= 2; i++) { /* 2 is index */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth1_1 = _3dnr->sfy[i].sth1_1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth2_1 = _3dnr->sfy[i].sth2_1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sth3_1 = _3dnr->sfy[i].sth3_1;

        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn0_1 = _3dnr->sfy[i].sfn0_1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn1_1 = _3dnr->sfy[i].sfn1_1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn2_1 = _3dnr->sfy[i].sfn2_1;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[i].sfn3_1 = _3dnr->sfy[i].sfn3_1;
    }

    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].bri_idx0 = _3dnr->sfy[3].bri_idx0; /* 3 is index */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].bri_idx1 = _3dnr->sfy[3].bri_idx1; /* 3 is index */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[2].k_mode  = _3dnr->sfy[2].k_mode; /* 2 is index */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.sfy[3].k_mode  = _3dnr->sfy[3].k_mode; /* 3 is index */
}


static td_void scene_set_nry_tfy(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    td_u32 i, j;
    /* ref */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[0].ref_en = _3dnr->tfy[0].ref_en;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[1].ref_en = _3dnr->tfy[1].ref_en;

    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[0].math_mode = _3dnr->tfy[0].math_mode;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[0].auto_math = _3dnr->tfy[0].auto_math;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[0].tfs0      = _3dnr->tfy[0].tfs0;

    /* tf_y */
    for (i = 1; i < scene_array_size(_3dnr->tfy); i++) {
        /* tss */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[i].tss0 = _3dnr->tfy[i].tss0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[i].tss1 = _3dnr->tfy[i].tss1;

        /* tfs */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[i].tfs0 = _3dnr->tfy[i].tfs0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[i].tfs1 = _3dnr->tfy[i].tfs1;

        /* tfr0 */
        for (j = 0; j < scene_array_size(_3dnr->tfy[i].tfr0); j++) {
            nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[i].tfr0[j] = _3dnr->tfy[i].tfr0[j];
             /* tfr1 */
            nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[1].tfr1[j] = _3dnr->tfy[1].tfr1[j];
            nrx_attr->nr_norm_param_v1.nr_manual.nr_param.tfy[2].tfr1[j] = _3dnr->tfy[2].tfr1[j]; /* 2 is index */
        }
    }
}

static td_void scene_set_nry_mdy(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    td_u32 i;
    /* md_y */
    for (i = 0; i < scene_array_size(_3dnr->mdy); i++) {
        /* adv_math */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].adv_math = _3dnr->mdy[i].adv_math;

        /* mcth */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[0].mcth = _3dnr->mdy[0].mcth;

        /* dci_w */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[0].dci_w = _3dnr->mdy[0].dci_w;

        /* math */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].math0 = _3dnr->mdy[i].math0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].math1 = _3dnr->mdy[i].math1;

        /* mate */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].mate0 = _3dnr->mdy[i].mate0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].mate1 = _3dnr->mdy[i].mate1;

        /* mabw */
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].mabw0 = _3dnr->mdy[i].mabw0;
        nrx_attr->nr_norm_param_v1.nr_manual.nr_param.mdy[i].mabw1 = _3dnr->mdy[i].mabw1;
    }
}

static td_void scene_set_nry_nc0(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    /* sfc */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.sfc = _3dnr->nrc0.sfc;

    /* tfc */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.tfc = _3dnr->nrc0.tfc;

    /* tpc */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.tpc = _3dnr->nrc0.tpc;

    /* pre_sfc */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.pre_sfc = _3dnr->nrc0.pre_sfc;

    /* nrc0_en */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0_en = _3dnr->nrc0_en;

    /* sfc_enhance */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.sfc_enhance = _3dnr->nrc0.sfc_enhance;

    /* sfc_ext */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.sfc_ext = _3dnr->nrc0.sfc_ext;

    /* trc */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc0.trc = _3dnr->nrc0.trc;
}


static td_void scene_set_nry_nc1(ot_3dnr_param *nrx_attr, const ot_scene_3dnr *_3dnr)
{
    /* sfs2 */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1.sfs2 = _3dnr->nrc1.sfs2;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1.sft2 = _3dnr->nrc1.sft2;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1.sbr2 = _3dnr->nrc1.sbr2;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1.pbr0 = _3dnr->nrc1.pbr0;
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1.pbr1 = _3dnr->nrc1.pbr1;

    /* nrc1_en */
    nrx_attr->nr_norm_param_v1.nr_manual.nr_param.nrc1_en = _3dnr->nrc1_en;
}

td_s32 scene_set_ai3dnr(ot_vi_pipe vi_pipe, const ot_scene_3dnr *_3dnr, td_u8 index)
{
    td_s32 ret = TD_SUCCESS;
    ot_ai3dnr_attr ai3dnr_attr;

    ai3dnr_attr.adv_param.nr_version = OT_NR_V1;
    ai3dnr_attr.adv_param.nr_adv_param_v1.op_mode = OT_OP_MODE_MANUAL;
    ret = ss_mpi_ai3dnr_get_attr(vi_pipe, &ai3dnr_attr);
    check_scene_ret(ret);

    scene_set_adv_sfy(&ai3dnr_attr, _3dnr);
    scene_set_adv_iey(&ai3dnr_attr, _3dnr);

    ai3dnr_attr.adv_param.nr_version = OT_NR_V1;
    ai3dnr_attr.adv_param.nr_adv_param_v1.op_mode = OT_OP_MODE_MANUAL;
    ret = ss_mpi_ai3dnr_set_attr(vi_pipe, &ai3dnr_attr);
    check_scene_ret(ret);
    return ret;
}

td_s32 scene_set_3dnr(ot_vi_pipe vi_pipe, const ot_scene_3dnr *_3dnr, td_u8 index, ot_3dnr_pos_type pos3dnr)
{
    td_s32 ret = TD_SUCCESS;

    ot_3dnr_param nrx_attr, nrx_get_attr;

    nrx_attr.nr_version = OT_NR_V1;
    nrx_attr.nr_norm_param_v1.op_mode = OT_OP_MODE_MANUAL;
    if (pos3dnr == OT_3DNR_POS_VI) {
        ret = ss_mpi_vi_get_pipe_3dnr_param(vi_pipe,  &nrx_attr);
    } else {
        ret = ss_mpi_vpss_get_grp_3dnr_param(vi_pipe,  &nrx_attr);
    }
    check_scene_ret(ret);

    scene_set_nry_en(&nrx_attr, _3dnr);
    scene_set_nry_iey(&nrx_attr, _3dnr);
    scene_set_nry_sfy_1to4(&nrx_attr, _3dnr);
    scene_set_nry_sfy_6to8(&nrx_attr, _3dnr);

    scene_set_nry_tfy(&nrx_attr, _3dnr);
    scene_set_nry_mdy(&nrx_attr, _3dnr);

    scene_set_nry_nc0(&nrx_attr, _3dnr);
    scene_set_nry_nc1(&nrx_attr, _3dnr);

    if (get_pipe_params()[index].module_state.dynamic_ai3dnr == TD_TRUE) {
        ret = scene_set_ai3dnr(vi_pipe, _3dnr, index);
        check_scene_ret(ret);
    }

    nrx_attr.nr_version = OT_NR_V1;
    nrx_attr.nr_norm_param_v1.op_mode = OT_OP_MODE_MANUAL;
    if (pos3dnr == OT_3DNR_POS_VI) {
        ret = ss_mpi_vi_set_pipe_3dnr_param(vi_pipe, &nrx_attr);
    } else {
        ret = ss_mpi_vpss_set_grp_3dnr_param(vi_pipe,  &nrx_attr);
    }
    check_scene_ret(ret);

    nrx_get_attr.nr_version = OT_NR_V1;
    nrx_get_attr.nr_norm_param_v1.op_mode = OT_OP_MODE_MANUAL;
    if (pos3dnr == OT_3DNR_POS_VI) {
        ret = ss_mpi_vi_get_pipe_3dnr_param(vi_pipe,  &nrx_get_attr);
    } else {
        ret = ss_mpi_vpss_get_grp_3dnr_param(vi_pipe,  &nrx_get_attr);
    }
    check_scene_ret(ret);

    static td_u32 count[OT_VI_MAX_PIPE_NUM] = {0};
    if ((g_scene_pipe_param[index].module_state.debug == 1) && ((count[vi_pipe] % 15) == 0)) { /* 15 max count */
        print_nrx(_3dnr);
    }
    count[vi_pipe]++;
    return ret;
}

td_void scene_set_3dnr_nrx_nry(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    scene_set_nrx_attr_interpulate(nry0_en);
    scene_set_nrx_attr_interpulate(nry1_en);
    scene_set_nrx_attr_interpulate(nry2_en);
    scene_set_nrx_attr_interpulate(nry3_en);
    scene_set_nrx_attr_interpulate(adv_mode);
}

td_void scene_set_3dnr_nrx_iey(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* ie_y */
    for (td_u32 i = 1; i < 5; i++) { /* 5 iey array size */
        scene_set_nrx_attr_interpulate(iey[i].ies0);
        scene_set_nrx_attr_interpulate(iey[i].ies1);
        scene_set_nrx_attr_interpulate(iey[i].ies2);
        scene_set_nrx_attr_interpulate(iey[i].ies3);
    }
}

static td_void scene_set_nrx_sfy(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* sf_y */
    for (td_u32 i = 1; i < 4; i++) { /* 4 sfy index */
        /* sfs1 */
        scene_set_nrx_attr_interpulate(sfy[i].sfs1);
        scene_set_nrx_attr_interpulate(sfy[i].sbr1);

        /* sfs2 */
        scene_set_nrx_attr_interpulate(sfy[i].sfs2);
        scene_set_nrx_attr_interpulate(sfy[i].sft2);
        scene_set_nrx_attr_interpulate(sfy[i].sbr2);

        /* sfs4 */
        scene_set_nrx_attr_interpulate(sfy[i].sfs4);
        scene_set_nrx_attr_interpulate(sfy[i].sft4);
        scene_set_nrx_attr_interpulate(sfy[i].sbr4);
    }
    scene_set_nrx_attr_interpulate(sfy[4].sfs2); /* 4  is index */
    scene_set_nrx_attr_interpulate(sfy[4].sft2); /* 4  is index */

    scene_set_nrx_attr_interpulate(sfy[2].k_mode); /* 2 is index */
    scene_set_nrx_attr_interpulate(sfy[3].k_mode); /* 3 is index */
}

static td_void set_sfs7_param(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    for (td_u32 i = 1; i < scene_array_size(nrx_attr->sfy); i++) {
        /* sfs7 */
        nrx_attr->sfy[i].spn = nrx_param->threednr_value[iso_level - 1].sfy[i].spn;
        nrx_attr->sfy[i].sbn = nrx_param->threednr_value[iso_level - 1].sfy[i].sbn;
        nrx_attr->sfy[i].pbr = nrx_param->threednr_value[iso_level - 1].sfy[i].pbr;
        nrx_attr->sfy[i].j_mode = nrx_param->threednr_value[iso_level - 1].sfy[i].j_mode;

        /* sfr6 */
        for (td_u32 j = 0; j < scene_array_size(nrx_attr->sfy[i].sfr6); j++) {
            scene_set_nrx_attr_interpulate(sfy[i].sfr6[j]);
        }

        /* sbr6 */
        scene_set_nrx_attr_interpulate(sfy[i].sbr6[0]);
        scene_set_nrx_attr_interpulate(sfy[i].sbr6[1]);

        /* sfn */
        nrx_attr->sfy[i].sfn0_0 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn0_0;
        nrx_attr->sfy[i].sfn1_0 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn1_0;
        nrx_attr->sfy[i].sfn2_0 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn2_0;
        nrx_attr->sfy[i].sfn3_0 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn3_0;

        scene_set_nrx_attr_interpulate(sfy[i].sth1_0);
        scene_set_nrx_attr_interpulate(sfy[i].sth2_0);
        scene_set_nrx_attr_interpulate(sfy[i].sth3_0);
    }

    for (td_u32 i = 1; i <= 2; i++) { /* 2 is index */
        /* sth */
        scene_set_nrx_attr_interpulate(sfy[i].sth1_1);
        scene_set_nrx_attr_interpulate(sfy[i].sth2_1);
        scene_set_nrx_attr_interpulate(sfy[i].sth3_1);

        nrx_attr->sfy[i].sfn0_1 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn0_1;
        nrx_attr->sfy[i].sfn1_1 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn1_1;
        nrx_attr->sfy[i].sfn2_1 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn2_1;
        nrx_attr->sfy[i].sfn3_1 = nrx_param->threednr_value[iso_level - 1].sfy[i].sfn3_1;
    }
}

static td_void scene_set_nrx_sfy_bri(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    scene_set_nrx_attr_interpulate(sfy[3].bri_idx0); /* 3 is index */
    scene_set_nrx_attr_interpulate(sfy[3].bri_idx1); /* 3 is index */

    for (td_u32 i = 0; i < OT_SCENE_3DNR_IDX_LEN; i++) {
        scene_set_nrx_attr_interpulate(sfy[3].bri_str[i]); /* 3 index */
    }

    for (td_u32 i = 0; i < 33; i++) { /* 33 is index */
        scene_set_nrx_attr_interpulate(sfy[2].sbs_k[i]); /* 2 index */
        scene_set_nrx_attr_interpulate(sfy[2].sds_k[i]); /* 2 index */
        scene_set_nrx_attr_interpulate(sfy[3].sbs_k[i]); /* 3 index */
        scene_set_nrx_attr_interpulate(sfy[3].sds_k[i]); /* 3 index */
    }
}

td_void scene_set_3dnr_nrx_sfy(_3dnr_nrx_param param)
{
    /* sf_y */
    scene_set_nrx_sfy(param);

    set_sfs7_param(param);
    /* bri */
    scene_set_nrx_sfy_bri(param);
}

td_void scene_set_3dnr_nrx_tfy(_3dnr_nrx_param param, ot_3dnr_pos_type pos3dnr)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* ref */
    scene_set_nrx_attr_interpulate(tfy[0].ref_en);
    scene_set_nrx_attr_interpulate(tfy[1].ref_en);

    scene_set_nrx_attr_interpulate(tfy[0].math_mode);
    scene_set_nrx_attr_interpulate(tfy[0].auto_math);

    if (pos3dnr == OT_3DNR_POS_VI) {
        scene_set_nrx_attr_interpulate(tfy[0].tfs0);
    } else {
        nrx_attr->tfy[0].tfs0 = 0;
    }
    /* tf_y */
    for (td_u32 i = 1; i < scene_array_size(nrx_attr->tfy); i++) {
        /* tss */
        scene_set_nrx_attr_interpulate(tfy[i].tss0);
        scene_set_nrx_attr_interpulate(tfy[i].tss1);

        /* tfs */
        scene_set_nrx_attr_interpulate(tfy[i].tfs0);
        scene_set_nrx_attr_interpulate(tfy[i].tfs1);

        /* tfr0 */
        for (td_u32 j = 0; j < scene_array_size(nrx_attr->tfy[i].tfr0); j++) {
            scene_set_nrx_attr_interpulate(tfy[i].tfr0[j]);
        }

        /* tfr1 */
        for (td_u32 j = 0; j < scene_array_size(nrx_attr->tfy[1].tfr1); j++) { /* tfr1 array size 6 */
            scene_set_nrx_attr_interpulate(tfy[1].tfr1[j]);
            scene_set_nrx_attr_interpulate(tfy[2].tfr1[j]); /* 2 is index */
        }
    }
}

td_void scene_set_3dnr_nrx_mdy(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* md_y */
    for (td_u32 i = 0; i < scene_array_size(nrx_attr->mdy); i++) {
        /* math */
        scene_set_nrx_attr_interpulate(mdy[i].math0);
        scene_set_nrx_attr_interpulate(mdy[i].math1);

        /* mate */
        scene_set_nrx_attr_interpulate(mdy[i].mate0);
        scene_set_nrx_attr_interpulate(mdy[i].mate1);

        /* mabw */
        scene_set_nrx_attr_interpulate(mdy[i].mabw0);
        scene_set_nrx_attr_interpulate(mdy[i].mabw1);

        scene_set_nrx_attr_interpulate(mdy[i].adv_math);
        /* mcth */
        scene_set_nrx_attr_interpulate(mdy[0].mcth);

        /* dci_w */
        scene_set_nrx_attr_interpulate(mdy[0].dci_w);
    }
}

td_void scene_set_3dnr_nrx_nrc0(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* nrc0_en */
    scene_set_nrx_attr_interpulate(nrc0_en);
    /* sfc */
    scene_set_nrx_attr_interpulate(nrc0.sfc);
    /* pre_sfc */
    scene_set_nrx_attr_interpulate(nrc0.pre_sfc);
    /* sfc_enhance */
    scene_set_nrx_attr_interpulate(nrc0.sfc_enhance);
    /* sfc_ext */
    scene_set_nrx_attr_interpulate(nrc0.sfc_ext);

    /* tfc */
    scene_set_nrx_attr_interpulate(nrc0.tfc);
    /* tpc */
    scene_set_nrx_attr_interpulate(nrc0.tpc);
    /* trc */
    scene_set_nrx_attr_interpulate(nrc0.trc);
}

td_void scene_set_3dnr_nrx_adv_iey(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    for (td_u32 i = 0; i < scene_array_size(nrx_attr->adv_ie_y); i++) {
        scene_set_nrx_attr_interpulate(adv_ie_y[i].ies0);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].ies1);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].ies2);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].ies3);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].aux_ies);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].o_sht);
        scene_set_nrx_attr_interpulate(adv_ie_y[i].u_sht);
    }
}

td_void scene_set_3dnr_nrx_adv_sfy(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    for (td_u32 i = 0; i < scene_array_size(nrx_attr->adv_sf_y); i++) {
        scene_set_nrx_attr_interpulate(adv_sf_y[i].spn6);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sbn6);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].pbr6);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].spn3);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sbn3);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].pbr3);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].spn4);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sbn4);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].pbr4);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].sfn0);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sfn1);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sfn2);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sfn3);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].sth1);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sth2);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sth3);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].sfs1);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].j_mode6);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].sbr6[0]);
        scene_set_nrx_attr_interpulate(adv_sf_y[i].sbr6[1]);

        scene_set_nrx_attr_interpulate(adv_sf_y[i].bri_idx);
        for (td_u32 j = 0; j < 4; j++) { /* 4  is index */
            scene_set_nrx_attr_interpulate(adv_sf_y[i].sfr6[j]);
        }
    }
    scene_set_nrx_attr_interpulate(adv_sf_y[0].sfs5);
    scene_set_nrx_attr_interpulate(adv_sf_y[0].sft5);
    scene_set_nrx_attr_interpulate(adv_sf_y[0].sbr5);
    scene_set_nrx_attr_interpulate(adv_sf_y[0].bld1);
}

td_void scene_set_3dnr_nrx_nrc1(_3dnr_nrx_param param)
{
    ot_scene_3dnr *nrx_attr = param.nrx_attr;
    const ot_scene_static_3dnr *nrx_param = param.nrx_param;
    td_u32 iso_level = param.iso_level;
    td_u32 mid = param.mid;
    td_u32 left = param.left;
    td_u32 right = param.right;

    /* nrc1_en */
    scene_set_nrx_attr_interpulate(nrc1_en);

    /* sfn */
    scene_set_nrx_attr_interpulate(nrc1.sfs2);
    scene_set_nrx_attr_interpulate(nrc1.sft2);
    scene_set_nrx_attr_interpulate(nrc1.sbr2);

    scene_set_nrx_attr_interpulate(nrc1.pbr0);
    scene_set_nrx_attr_interpulate(nrc1.pbr1);
}

#ifdef __cplusplus
}
#endif
