/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_alg.h"
#include "isp_config.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"
#include "isp_param_check.h"
#include "isp_proc.h"
#include "isp_sensor.h"

#define OT_ISP_CA_RATIO_MAX         2047
#define OT_ISP_CA_CSC_FIXED_POINT   1024
#define OT_ISP_CA_CSC_DEC           1000
#define OT_ISP_CA_ISO_RATIO_DEFAULT 1024
#define OT_ISP_CA_FRAME_CNT         2
#define OT_ISP_CA_CP_Y_SHIFT        16
#define OT_ISP_CA_CP_U_SHIFT        8

static const td_u32 g_y_ratio_lut[OT_ISP_CA_YRATIO_LUT_LENGTH] = {
    511, 511, 512, 512, 512, 512, 512, 512, 512, 513, 513, 513, 513, 512, 512, 512,
    512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512,
    513, 513, 513, 514, 514, 515, 515, 516, 516, 517, 518, 519, 520, 521, 522, 523,
    524, 526, 527, 529, 530, 532, 534, 536, 538, 540, 542, 545, 547, 550, 553, 556,
    559, 562, 566, 570, 573, 577, 582, 586, 591, 595, 600, 605, 610, 616, 621, 626,
    632, 638, 644, 649, 655, 661, 668, 674, 680, 686, 693, 699, 705, 712, 718, 724,
    731, 737, 744, 750, 756, 762, 769, 775, 781, 787, 793, 799, 805, 810, 816, 822,
    827, 832, 837, 843, 847, 852, 857, 861, 865, 870, 873, 877, 881, 884, 887, 890,
    893, 895, 898, 900, 902, 904, 906, 907, 909, 910, 911, 912, 913, 914, 915, 915,
    916, 916, 917, 917, 917, 917, 917, 917, 917, 917, 917, 917, 916, 916, 916, 915,
    915, 914, 914, 913, 913, 912, 912, 911, 911, 910, 910, 909, 909, 909, 908, 908,
    908, 908, 907, 907, 907, 907, 907, 908, 908, 908, 909, 909, 910, 911, 912, 913,
    914, 915, 916, 917, 919, 920, 922, 923, 925, 926, 928, 930, 932, 933, 935, 937,
    939, 941, 943, 945, 947, 949, 951, 953, 955, 957, 960, 962, 964, 966, 968, 970,
    972, 975, 977, 979, 981, 983, 985, 987, 989, 991, 993, 995, 997, 999, 1001, 1003,
    1005, 1006, 1008, 1010, 1011, 1013, 1014, 1016, 1017, 1018, 1020, 1021, 1022, 1023, 1024, 1025
};
static const td_s16 g_iso_ratio[OT_ISP_AUTO_ISO_NUM] = {
    1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024
};
static const td_u32 g_y_sat_lut[OT_ISP_CA_YRATIO_LUT_LENGTH] = {
    811, 811, 812, 812, 812, 812, 812, 812, 812, 812, 812, 811, 811, 811, 810, 810,
    810, 809, 809, 809, 808, 808, 807, 807, 807, 806, 806, 806, 805, 805, 805, 804,
    804, 804, 804, 804, 804, 804, 804, 804, 804, 804, 805, 805, 806, 806, 807, 807,
    808, 809, 810, 811, 813, 814, 815, 817, 819, 820, 822, 824, 827, 829, 831, 834,
    837, 840, 843, 846, 850, 853, 857, 861, 865, 870, 874, 879, 883, 888, 893, 898,
    903, 908, 914, 919, 925, 930, 936, 942, 947, 953, 959, 965, 971, 977, 983, 989,
    995, 1001, 1007, 1012, 1018, 1024, 1030, 1036, 1042, 1048, 1053, 1059, 1065, 1070, 1075, 1081,
    1086, 1091, 1096, 1101, 1106, 1111, 1115, 1120, 1124, 1128, 1132, 1136, 1139, 1143, 1146, 1149,
    1152, 1155, 1158, 1161, 1163, 1166, 1168, 1170, 1172, 1174, 1176, 1178, 1180, 1182, 1183, 1185,
    1186, 1188, 1189, 1190, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1200, 1201, 1202,
    1202, 1203, 1204, 1204, 1205, 1205, 1206, 1206, 1207, 1207, 1208, 1208, 1209, 1209, 1210, 1210,
    1211, 1211, 1212, 1212, 1213, 1214, 1214, 1215, 1215, 1216, 1217, 1218, 1218, 1219, 1220, 1221,
    1222, 1223, 1224, 1225, 1226, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1232, 1233, 1234, 1235,
    1236, 1236, 1237, 1238, 1239, 1240, 1240, 1241, 1242, 1243, 1243, 1244, 1245, 1245, 1246, 1247,
    1248, 1248, 1249, 1250, 1250, 1251, 1252, 1253, 1253, 1254, 1255, 1255, 1256, 1257, 1258, 1258,
    1259, 1260, 1260, 1261, 1262, 1263, 1263, 1264, 1265, 1265, 1266, 1267, 1268, 1268, 1269, 1270
};
static const td_u32 g_ca_cp_lut[OT_ISP_CA_YRATIO_LUT_LENGTH] = {
    367502, 372351, 423958, 588475, 686838, 748143, 902962, 1001325, 1062630, 1212600, 1320662, 1475480, 1583542,
    1635149, 1785118, 1898029, 1996393, 2109304, 2207667, 2325428, 2423791, 2531853, 2593158, 2701220, 2799583,
    2865737, 2927042, 3035104, 3185074, 3293135, 3312533, 3420595, 3528657, 3580263, 3688325, 3754479, 3815784,
    4012511, 4073816, 4181877, 4248032, 4299638, 4407700, 4515762, 4577067, 4694828, 4793191, 4891554, 4999616,
    5097979, 5149585, 5215740, 5314103, 5412466, 5473771, 5572134, 5670497, 5783409, 5835015, 5933378, 5994683,
    6093046, 6210807, 6275225, 6354190, 6381550, 6474445, 6567341, 6594700, 6677897, 6770792, 6798152, 6904977,
    6997873, 6978475, 7113278, 7135788, 7233533, 7279672, 7307031, 7399927, 7543810, 7524412, 7617308, 7710203,
    7737563, 7830458, 7923354, 8029561, 8163627, 8311623, 8506376, 8645292, 8793288, 8861818, 8995884, 9195486,
    9353181, 9487247, 9635244, 9774159, 9889446, 10037442, 10171509, 10319505, 10519107, 10653173, 10810868,
    10870936, 11006121, 11089818, 11206223, 11280221, 11401476, 11550709, 11615508, 11755042, 11810761, 11959994,
    12010863, 12150397, 12285582, 12355349, 12485684, 12559682, 12680937, 12764634, 12894969, 13011375, 13048433,
    13029535, 13062244, 13043347, 13071206, 13108764, 13089867, 13070970, 13150435, 13131538, 13178176, 13079813,
    13005078, 12972251, 12892667, 12874388, 12785105, 12715220, 12630786, 12542122, 12532923, 12434560, 12425361,
    12289940, 12271042, 12182378, 12093095, 12069967, 11995232, 11962405, 11840913, 11817785, 11677514, 11542093,
    11383043, 11261551, 11172887, 10981628, 10906893, 10771472, 10626352, 10490930, 10393067, 10131423, 9855731,
    9645575, 9351104, 9173775, 8893852, 8678846, 8403154, 8207046, 7978111, 7730897, 7497112, 7305853, 7011382,
    6801226, 6535233, 6339125, 6063433, 5853276, 5572735, 5358348, 5325639, 5288581, 5298280, 5307978, 5270920,
    5270920, 5280619, 5178025, 5187723, 5150665, 5150665, 5160364, 5170063, 5133004, 5142703, 5315801, 5568983,
    5779639, 6027972, 6215618, 6445053, 6698235, 6881032, 7152374, 7335171, 7597433, 7831718, 8014515, 8267697,
    8431596, 8684778, 8904633, 9134068, 9387250, 9583976, 9804331, 10001057, 10249390, 10516502, 10703529, 10956711,
    11106681, 11359863, 11556589, 11804922, 12058104, 12254830, 12451557, 12648283, 12943373, 13149798, 13346524,
    13599706, 13749675, 14002857, 14236642, 14438218, 14691400, 14888126, 15094551, 15291278, 15544460, 15792792
};
static const td_u16 g_y_ch1_lut[OT_ISP_CA_CHLUT_LENGTH] = {
    0x14e, 0x14e, 0x111, 0xfe, 0x102, 0x105, 0x106, 0x108, 0x10c, 0x10c, 0xe3, 0xbf, 0xc4, 0xe2, 0x110, 0x154
};
static const td_u16 g_y_ch2_lut[OT_ISP_CA_CHLUT_LENGTH] = {
    0x0, 0x0, 0x5, 0x8, 0x7, 0x6, 0x6, 0x5, 0x3, 0x3, 0x15, 0x1e, 0x1f, 0x1c, 0x18, 0x15
};
static const td_u16 g_y_slu_lut[OT_ISP_CA_CHLUT_LENGTH] = {
    0xffe2, 0xffd3, 0xffbc, 0xffb0, 0xffab, 0xffa6, 0xffa1, 0xffa4, 0xffa8, 0xffae, 0xffb8, 0xffc0,
    0xffcc, 0xffd4, 0xffe0, 0xffea
};
static const td_u16 g_y_slv_lut[OT_ISP_CA_CHLUT_LENGTH] = {
    0x20, 0x2c, 0x40, 0x53, 0x5f, 0x67, 0x6d, 0x6e, 0x69, 0x64, 0x57, 0x4a, 0x3c, 0x32, 0x23, 0x17
};

typedef struct {
    td_bool init;
    td_bool ca_en;      /* u1.0 */
    td_bool ca_cp_en;  /* u1.0 */
    td_bool ca_coef_update_en;
    td_s16 saturation_ratio;
    td_u32 y_ratio_lut[OT_ISP_CA_YRATIO_LUT_LENGTH];
    td_u32 y_sat_lut[OT_ISP_CA_YRATIO_LUT_LENGTH];
    td_s16 ca_iso_ratio[OT_ISP_AUTO_ISO_NUM]; /* 16 */
    td_u32 ca_cp_lut[OT_ISP_CA_YRATIO_LUT_LENGTH];
    td_s16 actual_iso_ratio;
} isp_ca;

isp_ca *g_ca_ctx[OT_ISP_MAX_PIPE_NUM] = {TD_NULL};

#define ca_get_ctx(pipe, ctx)   (ctx) = g_ca_ctx[pipe]
#define ca_set_ctx(pipe, ctx)   g_ca_ctx[pipe] = (ctx)
#define ca_reset_ctx(pipe)      g_ca_ctx[pipe] = TD_NULL

static td_s32 ca_ctx_init(ot_vi_pipe vi_pipe)
{
    isp_ca *ca_ctx = TD_NULL;

    ca_get_ctx(vi_pipe, ca_ctx);

    if (ca_ctx == TD_NULL) {
        ca_ctx = (isp_ca *)isp_malloc(sizeof(isp_ca));
        if (ca_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] ca_ctx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(ca_ctx, sizeof(isp_ca), 0, sizeof(isp_ca));

    ca_set_ctx(vi_pipe, ca_ctx);

    return TD_SUCCESS;
}

static td_void ca_ctx_exit(ot_vi_pipe vi_pipe)
{
    isp_ca *ca_ctx = TD_NULL;

    ca_get_ctx(vi_pipe, ca_ctx);
    isp_free(ca_ctx);
    ca_reset_ctx(vi_pipe);
}

static td_void ca_ext_regs_initialize(ot_vi_pipe vi_pipe, const isp_ca *ca)
{
    td_u16 i;

    ot_ext_system_ca_en_write(vi_pipe, ca->ca_en);
    ot_ext_system_ca_cp_en_write(vi_pipe, ca->ca_cp_en);

    ot_ext_system_ca_saturation_ratio_write(vi_pipe, OT_ISP_EXT_CA_SATURATION_RATIO_DEFAULT);
    for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
        ot_ext_system_ca_y_ratio_lut_write(vi_pipe, i, ca->y_ratio_lut[i]);
        ot_ext_system_ca_y_sat_lut_write(vi_pipe, i, ca->y_sat_lut[i]);
        ot_ext_system_ca_cp_lut_write(vi_pipe, i, ca->ca_cp_lut[i]);
    }

    for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
        ot_ext_system_ca_iso_ratio_lut_write(vi_pipe, i, ca->ca_iso_ratio[i]);
    }

    ot_ext_system_ca_coef_update_en_write(vi_pipe, TD_TRUE);
}


static td_void ca_static_regs_initialize(isp_ca_static_cfg *static_reg_cfg)
{
    static_reg_cfg->ca_llhc_proc_en  = TD_TRUE;
    static_reg_cfg->ca_skin_proc_en  = TD_TRUE;
    static_reg_cfg->ca_satvssat_en   = TD_TRUE;
    static_reg_cfg->ca_satu_adj_en   = TD_TRUE;
    static_reg_cfg->ca_des_point     = 0x400;
    static_reg_cfg->ca_des_mix       = 0xff;
    static_reg_cfg->ca_sat_c2        = 0x0;
    static_reg_cfg->ca_sat_c1        = 0x20;
    static_reg_cfg->static_resh = TD_TRUE;
}

static td_void ca_usr_regs_initialize(isp_ca_usr_cfg *usr_reg_cfg, const isp_ca *ca)
{
    usr_reg_cfg->ca_cp_en = ca->ca_cp_en;

    if (!ca->ca_cp_en) {
        (td_void)memcpy_s(usr_reg_cfg->y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          ca->y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        (td_void)memcpy_s(usr_reg_cfg->y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          ca->y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        (td_void)memcpy_s(usr_reg_cfg->y_ch1_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16),
                          g_y_ch1_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16));
        (td_void)memcpy_s(usr_reg_cfg->y_ch2_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16),
                          g_y_ch2_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16));
        (td_void)memcpy_s(usr_reg_cfg->y_slu_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16),
                          g_y_slu_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16));
        (td_void)memcpy_s(usr_reg_cfg->y_slv_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16),
                          g_y_slv_lut, OT_ISP_CA_CHLUT_LENGTH * sizeof(td_u16));
    } else {
        (td_void)memcpy_s(usr_reg_cfg->y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          ca->ca_cp_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
    }
    usr_reg_cfg->ca_lut_update_en = TD_TRUE;
    usr_reg_cfg->resh          = TD_TRUE;
    usr_reg_cfg->update_index = 1;
    usr_reg_cfg->buf_id        = 0;
}

static td_void ca_dyna_regs_initialize(isp_ca_dyna_cfg *dyna_reg_cfg)
{
    dyna_reg_cfg->ca_iso_ratio  = OT_ISP_CA_ISO_RATIO_DEFAULT;
    dyna_reg_cfg->resh          = TD_TRUE;

    return;
}

static td_void ca_regs_initialize(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_ca *ca)
{
    td_bool offline_mode;
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode = (is_offline_mode(isp_ctx->block_attr.running_mode) ||
                    is_striping_mode(isp_ctx->block_attr.running_mode) ||
                    is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    for (i = 0; i < reg_cfg->cfg_num; i++) {
        reg_cfg->alg_reg_cfg[i].ca_reg_cfg.ca_en       = offline_mode ? (ca->ca_en) : (TD_FALSE);
        reg_cfg->alg_reg_cfg[i].ca_reg_cfg.lut2_stt_en = TD_TRUE;

        ca_static_regs_initialize(&reg_cfg->alg_reg_cfg[i].ca_reg_cfg.static_reg_cfg);
        ca_usr_regs_initialize(&reg_cfg->alg_reg_cfg[i].ca_reg_cfg.usr_reg_cfg, ca);
        ca_dyna_regs_initialize(&reg_cfg->alg_reg_cfg[i].ca_reg_cfg.dyna_reg_cfg);
    }

    reg_cfg->cfg_key.bit1_ca_cfg = 1;
}

static td_s32 ca_check_cmos_param(ot_vi_pipe vi_pipe, const ot_isp_ca_attr *ca_attr)
{
    td_s32 ret;
    ot_unused(vi_pipe);
    ret = isp_ca_attr_check("cmos", ca_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return TD_SUCCESS;
}

static td_s32 ca_in_initialize(ot_vi_pipe vi_pipe, isp_ca *ca)
{
    td_u16 i;
    td_s32 ret;
    td_u32 lut_y, lut_u, lut_v;
    ot_isp_cmos_default *sns_dft = TD_NULL;

    isp_sensor_get_default(vi_pipe, &sns_dft);

    if (sns_dft->key.bit1_ca) {
        isp_check_pointer_return(sns_dft->ca);

        ret = ca_check_cmos_param(vi_pipe, sns_dft->ca);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        ca->ca_en  = sns_dft->ca->enable;
        ca->ca_cp_en = (sns_dft->ca->ca_cp_en == OT_ISP_CP_ENABLE) ? TD_TRUE : TD_FALSE;
        (td_void)memcpy_s(ca->y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          sns_dft->ca->ca.y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        (td_void)memcpy_s(ca->y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          sns_dft->ca->ca.y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
            ca->ca_iso_ratio[i] = (td_s16)sns_dft->ca->ca.iso_ratio[i];
        }
        for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
            lut_y = sns_dft->ca->cp.cp_lut_y[i];
            lut_u = sns_dft->ca->cp.cp_lut_u[i];
            lut_v = sns_dft->ca->cp.cp_lut_v[i];
            ca->ca_cp_lut[i] = (lut_y << OT_ISP_CA_CP_Y_SHIFT) + (lut_u << OT_ISP_CA_CP_U_SHIFT) + lut_v;
        }
    } else {
        ca->ca_en  = TD_TRUE;
        ca->ca_cp_en = TD_FALSE;
        (td_void)memcpy_s(ca->y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          g_y_ratio_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        (td_void)memcpy_s(ca->ca_iso_ratio, OT_ISP_AUTO_ISO_NUM * sizeof(td_s16),
                          g_iso_ratio, OT_ISP_AUTO_ISO_NUM * sizeof(td_s16));
        (td_void)memcpy_s(ca->y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          g_y_sat_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
        (td_void)memcpy_s(ca->ca_cp_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32),
                          g_ca_cp_lut, OT_ISP_CA_YRATIO_LUT_LENGTH * sizeof(td_u32));
    }

    return TD_SUCCESS;
}

static td_s32 ca_read_extregs(ot_vi_pipe vi_pipe)
{
    td_u16 i;
    isp_ca *ca = TD_NULL;

    ca_get_ctx(vi_pipe, ca);

    ca->ca_coef_update_en = ot_ext_system_ca_coef_update_en_read(vi_pipe);
    if (ca->ca_coef_update_en) {
        ot_ext_system_ca_coef_update_en_write(vi_pipe, TD_FALSE);
        ca->ca_cp_en = ot_ext_system_ca_cp_en_read(vi_pipe);
        if (ca->ca_cp_en) {
            for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
                ca->y_ratio_lut[i] = ot_ext_system_ca_cp_lut_read(vi_pipe, i);
            }
        } else {
            for (i = 0; i < OT_ISP_CA_YRATIO_LUT_LENGTH; i++) {
                ca->y_ratio_lut[i] = ot_ext_system_ca_y_ratio_lut_read(vi_pipe, i);
                ca->y_sat_lut[i] = ot_ext_system_ca_y_sat_lut_read(vi_pipe, i);
            }
        }

        for (i = 0; i < OT_ISP_AUTO_ISO_NUM; i++) {
            ca->ca_iso_ratio[i] = (td_s16)ot_ext_system_ca_iso_ratio_lut_read(vi_pipe, i);
        }

        ca->saturation_ratio = (td_s16)ot_ext_system_ca_saturation_ratio_read(vi_pipe);
    }

    return TD_SUCCESS;
}

static td_s32 ca_get_value_from_lut(td_s32 x, const td_u32 *lut_x, const td_s16 *lut_y, td_s32 length)
{
    td_s32 n;

    if (x <= (td_s32)lut_x[0]) {
        return lut_y[0];
    }

    for (n = 1; n < length; n++) {
        if (x <= (td_s32)lut_x[n]) {
            return (lut_y[n - 1] + (td_s64)(lut_y[n] - lut_y[n - 1]) *
                   (td_s64)(x - (td_s32)lut_x[n - 1]) / div_0_to_1((td_s32)lut_x[n] - (td_s32)lut_x[n - 1]));
        }
    }

    return lut_y[length - 1];
}

static td_bool check_ca_open(const isp_ca *ca)
{
    return (ca->ca_en == TD_TRUE);
}

static td_void isp_ca_usr_fw(const isp_ca *ca, isp_ca_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    usr_reg_cfg->ca_cp_en = ca->ca_cp_en;

    if (ca->ca_cp_en == 0) {
        for (j = 0; j < OT_ISP_CA_YRATIO_LUT_LENGTH; j++) {
            usr_reg_cfg->y_ratio_lut[j] =
            MIN2(ca->y_ratio_lut[j] * ca->saturation_ratio / OT_ISP_CA_CSC_DEC, OT_ISP_CA_RATIO_MAX);
            usr_reg_cfg->y_sat_lut[j] = ca->y_sat_lut[j];
        }
    } else {
        for (j = 0; j < OT_ISP_CA_YRATIO_LUT_LENGTH; j++) {
            usr_reg_cfg->y_ratio_lut[j] = ca->y_ratio_lut[j];
        }
    }

    usr_reg_cfg->ca_lut_update_en = TD_TRUE;
    usr_reg_cfg->resh             = TD_TRUE;
    usr_reg_cfg->update_index += 1;

    return;
}

static td_void isp_ca_dyna_fw(td_s32 iso, isp_ca_dyna_cfg *dyna_reg_cfg, isp_ca *ca)
{
    td_s32 iso_ratio;

    iso_ratio = ca_get_value_from_lut(iso, get_iso_lut(), ca->ca_iso_ratio, OT_ISP_AUTO_ISO_NUM);

    dyna_reg_cfg->ca_iso_ratio = clip3(iso_ratio, 0, OT_ISP_CA_RATIO_MAX);
    dyna_reg_cfg->resh         = TD_TRUE;
    ca->actual_iso_ratio       = dyna_reg_cfg->ca_iso_ratio;
}

static td_s32 isp_ca_init(ot_vi_pipe vi_pipe, td_void *reg_cfg_info)
{
    td_s32 ret;
    isp_ca *ca_ctx = TD_NULL;
    isp_reg_cfg *reg_cfg = (isp_reg_cfg *)reg_cfg_info;

    ot_ext_system_isp_ca_init_status_write(vi_pipe, TD_FALSE);
    ret = ca_ctx_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ca_get_ctx(vi_pipe, ca_ctx);
    isp_check_pointer_return(ca_ctx);

    ca_ctx->init = TD_FALSE;
    ret = ca_in_initialize(vi_pipe, ca_ctx);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ca_regs_initialize(vi_pipe, reg_cfg, ca_ctx);
    ca_ext_regs_initialize(vi_pipe, ca_ctx);
    ca_ctx->init = TD_TRUE;
    ot_ext_system_isp_ca_init_status_write(vi_pipe, ca_ctx->init);

    return TD_SUCCESS;
}

static td_s32 isp_ca_run(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv)
{
    td_u8  i;
    ot_isp_alg_mod alg_mod = OT_ISP_ALG_CA;
    isp_ca  *ca = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg *reg = (isp_reg_cfg *)reg_cfg;
    td_s32 ret;
    ot_unused(stat_info);
    ot_unused(rsv);
    isp_get_ctx(vi_pipe, isp_ctx);
    ca_get_ctx(vi_pipe, ca);
    isp_check_pointer_return(ca);

    ot_ext_system_isp_ca_init_status_write(vi_pipe, ca->init);
    if (ca->init != TD_TRUE) {
        ioctl(isp_get_fd(vi_pipe), ISP_ALG_INIT_ERR_INFO_PRINT, &alg_mod);
        return TD_SUCCESS;
    }

    /* calculate every two interrupts */
    if ((isp_ctx->frame_cnt % OT_ISP_CA_FRAME_CNT != 0) && (isp_ctx->linkage.snap_state != TD_TRUE)) {
        return TD_SUCCESS;
    }

    ca->ca_en = ot_ext_system_ca_en_read(vi_pipe);

    for (i = 0; i < reg->cfg_num; i++) {
        reg->alg_reg_cfg[i].ca_reg_cfg.ca_en = ca->ca_en;
    }

    reg->cfg_key.bit1_ca_cfg = 1;

    /* check hardware setting */
    if (!check_ca_open(ca)) {
        return TD_SUCCESS;
    }

    ret = ca_read_extregs(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (ca->ca_coef_update_en) {
        for (i = 0; i < reg->cfg_num; i++) {
            isp_ca_usr_fw(ca, &reg->alg_reg_cfg[i].ca_reg_cfg.usr_reg_cfg);
        }
    }

    for (i = 0; i < reg->cfg_num; i++) {
        isp_ca_dyna_fw((td_s32)isp_ctx->linkage.iso, &reg->alg_reg_cfg[i].ca_reg_cfg.dyna_reg_cfg, ca);
    }

    return TD_SUCCESS;
}

static td_void ca_proc_write(ot_vi_pipe vi_pipe, ot_isp_ctrl_proc_write *proc)
{
    ot_isp_ctrl_proc_write proc_tmp;
    isp_ca  *ca_ctx = TD_NULL;

    ca_get_ctx(vi_pipe, ca_ctx);
    isp_check_pointer_void_return(ca_ctx);

    if ((proc->proc_buff == TD_NULL) || (proc->buff_len == 0)) {
        return;
    }

    proc_tmp.proc_buff = proc->proc_buff;
    proc_tmp.buff_len  = proc->buff_len;

    isp_proc_print_title(&proc_tmp, &proc->write_len, "ca info");
    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%7s" "%12s\n", "enable", "iso_ratio");

    isp_proc_printf(&proc_tmp, proc->write_len,
                    "%7d" "%12d\n", ca_ctx->ca_en, ca_ctx->actual_iso_ratio);

    proc->write_len += 1;
}

static td_s32 isp_ca_ctrl(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value)
{
    switch (cmd) {
        case OT_ISP_PROC_WRITE:
            ca_proc_write(vi_pipe, (ot_isp_ctrl_proc_write *)value);
            break;
        default:
            break;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ca_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    ot_ext_system_isp_ca_init_status_write(vi_pipe, TD_FALSE);
    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        reg_cfg->reg_cfg.alg_reg_cfg[i].ca_reg_cfg.ca_en = TD_FALSE;
    }

    reg_cfg->reg_cfg.cfg_key.bit1_ca_cfg = 1;

    ca_ctx_exit(vi_pipe);

    return TD_SUCCESS;
}

td_s32 isp_alg_register_ca(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_alg_node *algs = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_alg_check_return(isp_ctx->alg_key.bit1_ca);
    algs = isp_search_alg(isp_ctx->algs);
    isp_check_pointer_return(algs);

    algs->alg_type = OT_ISP_ALG_CA;
    algs->alg_func.pfn_alg_init = isp_ca_init;
    algs->alg_func.pfn_alg_run  = isp_ca_run;
    algs->alg_func.pfn_alg_ctrl = isp_ca_ctrl;
    algs->alg_func.pfn_alg_exit = isp_ca_exit;
    algs->used = TD_TRUE;

    return TD_SUCCESS;
}
