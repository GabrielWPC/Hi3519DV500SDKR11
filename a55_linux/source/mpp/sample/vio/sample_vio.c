/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "sample_ipc.h"
#include "securec.h"
#include "ss_mpi_ae.h"

#define X_ALIGN 16
#define Y_ALIGN 2
#define out_ratio_1(x) ((x) / 3)
#define out_ratio_2(x) ((x) * 2 / 3)
#define out_ratio_3(x) ((x) / 2)

static volatile sig_atomic_t g_sig_flag = 0;

/* this configuration is used to adjust the size and number of buffer(VB).  */
static sample_vb_param g_vb_param = {
    .vb_size = {3840, 2160},
    .pixel_format = {OT_PIXEL_FORMAT_RGB_BAYER_12BPP, OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420},
    .compress_mode = {OT_COMPRESS_MODE_LINE, OT_COMPRESS_MODE_SEG},
    .video_format = {OT_VIDEO_FORMAT_LINEAR, OT_VIDEO_FORMAT_LINEAR},
    .blk_num = {8, 12}
};

static sampe_sys_cfg g_vio_sys_cfg = {
    .route_num = 2,
    .mode_type = OT_VI_OFFLINE_VPSS_OFFLINE,
    .nr_pos = OT_3DNR_POS_VI,
    .vi_fmu = {0},
    .vpss_fmu = {0},
};

static sample_vo_cfg g_vo_cfg = {
    .vo_dev            = SAMPLE_VO_DEV_UHD,
    .vo_layer          = SAMPLE_VO_LAYER_VHD0,
    .vo_intf_type      = OT_VO_INTF_BT1120,
    .intf_sync         = OT_VO_OUT_1080P60,
    .bg_color          = COLOR_RGB_BLACK,
    .pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect         = {0, 0, 1920, 1080},
    .image_size        = {1920, 1080},
    .vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len       = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode           = VO_MODE_1MUX,
    .compress_mode     = OT_COMPRESS_MODE_NONE,
};

static sample_comm_venc_chn_param g_venc_chn_param = {
    .frame_rate           = 30, /* 30 is a number */
    .stats_time           = 2,  /* 2 is a number */
    .gop                  = 60, /* 60 is a number */
    .venc_size            = {1920, 1080},
    .size                 = -1,
    .profile              = 0,
    .is_rcn_ref_share_buf = TD_FALSE,
    .gop_attr             = {
        .gop_mode = OT_VENC_GOP_MODE_NORMAL_P,
        .normal_p = {2},
    },
    .type                 = OT_PT_H265,
    .rc_mode              = SAMPLE_RC_CBR,
};

static sample_vi_fpn_calibration_cfg g_calibration_cfg = {
    .threshold     = 4095, /* 4095 is a number */
    .frame_num     = 16,   /* 16 is a number */
    .fpn_type      = OT_ISP_FPN_TYPE_FRAME,
    .pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP,
    .compress_mode = OT_COMPRESS_MODE_NONE,
};

static sample_vi_fpn_correction_cfg g_correction_cfg = {
    .op_mode       = OT_OP_MODE_AUTO,
    .fpn_type      = OT_ISP_FPN_TYPE_FRAME,
    .strength      = 0,
    .pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP,
    .compress_mode = OT_COMPRESS_MODE_NONE,
};

static td_void sample_get_char(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }

    sample_pause();
}

static td_u32 sample_vio_get_fmu_wrap_num(ot_fmu_mode fmu_mode[], td_u32 len)
{
    td_u32 i;
    td_u32 cnt = 0;

    for (i = 0; i < len; i++) {
        if (fmu_mode[i] == OT_FMU_MODE_WRAP) {
            cnt++;
        }
    }
    return cnt;
}

static td_s32 sample_vio_fmu_wrap_init(sampe_sys_cfg *fmu_cfg, ot_size *in_size)
{
    td_u32 cnt;
    ot_fmu_attr fmu_attr;

    cnt = sample_vio_get_fmu_wrap_num(fmu_cfg->vi_fmu, fmu_cfg->route_num);
    if (cnt > 0) {
        fmu_attr.wrap_en = TD_TRUE;
        fmu_attr.page_num = MIN2(ot_common_get_fmu_wrap_page_num(OT_FMU_ID_VI,
            in_size->width, in_size->height) + (cnt - 1) * 3, /* 3: for multi pipe */
            OT_FMU_MAX_Y_PAGE_NUM);
    } else {
        fmu_attr.wrap_en = TD_FALSE;
    }
    if (ss_mpi_sys_set_fmu_attr(OT_FMU_ID_VI, &fmu_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    cnt = sample_vio_get_fmu_wrap_num(fmu_cfg->vpss_fmu, fmu_cfg->route_num);
    if (cnt > 0) {
        fmu_attr.wrap_en = TD_TRUE;
        fmu_attr.page_num = MIN2(ot_common_get_fmu_wrap_page_num(OT_FMU_ID_VPSS,
            in_size->width, in_size->height) + (cnt - 1) * 3, /* 3: for multi pipe */
            OT_FMU_MAX_Y_PAGE_NUM + OT_FMU_MAX_C_PAGE_NUM);
    } else {
        fmu_attr.wrap_en = TD_FALSE;
    }
    if (ss_mpi_sys_set_fmu_attr(OT_FMU_ID_VPSS, &fmu_attr) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

/* define SAMPLE_MEM_SHARE_ENABLE, when use tools to dump YUV/RAW. */
#ifdef SAMPLE_MEM_SHARE_ENABLE
td_void sample_vio_init_mem_share(td_void)
{
    td_u32 i;
    ot_vb_common_pools_id pools_id = {0};

    if (ss_mpi_vb_get_common_pool_id(&pools_id) != TD_SUCCESS) {
        sample_print("get common pool_id failed!\n");
        return;
    }
    for (i = 0; i < pools_id.pool_cnt; ++i) {
        ss_mpi_vb_pool_share_all(pools_id.pool[i]);
    }
}
#endif

static td_s32 sample_vio_sys_init(td_void)
{
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK | OT_VB_SUPPLEMENT_MOTION_DATA_MASK;

    sample_comm_sys_get_default_vb_cfg(&g_vb_param, &vb_cfg);
    if (sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config) != TD_SUCCESS) {
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_vio_init_mem_share();
#endif

    if (sample_comm_vi_set_vi_vpss_mode(g_vio_sys_cfg.mode_type, OT_VI_AIISP_MODE_DEFAULT) != TD_SUCCESS) {
        goto sys_exit;
    }

    if (ss_mpi_sys_set_3dnr_pos(g_vio_sys_cfg.nr_pos) != TD_SUCCESS) {
        goto sys_exit;
    }

    if (sample_vio_fmu_wrap_init(&g_vio_sys_cfg, &g_vb_param.vb_size) != TD_SUCCESS) {
        goto sys_exit;
    }

    return TD_SUCCESS;
sys_exit:
    sample_comm_sys_exit();
    return TD_FAILURE;
}

static td_s32 sample_vio_start_vpss(ot_vpss_grp grp, sample_vpss_cfg *vpss_cfg)
{
    td_s32 ret;
    sample_vpss_chn_attr vpss_chn_attr = {0};

    (td_void)memcpy_s(&vpss_chn_attr.chn_attr[0], sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM,
        vpss_cfg->chn_attr, sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM);
    if (g_vio_sys_cfg.vpss_fmu[grp] == OT_FMU_MODE_WRAP) {
        vpss_chn_attr.chn0_wrap = TD_TRUE;
    }
    (td_void)memcpy_s(vpss_chn_attr.chn_enable, sizeof(vpss_chn_attr.chn_enable),
        vpss_cfg->chn_en, sizeof(vpss_chn_attr.chn_enable));
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    ret = sample_common_vpss_start(grp, &vpss_cfg->grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (vpss_cfg->nr_attr.enable == TD_TRUE) {
        if (ss_mpi_vpss_set_grp_3dnr_attr(grp, &vpss_cfg->nr_attr) != TD_SUCCESS) {
            goto stop_vpss;
        }
    }
    /* OT_FMU_MODE_WRAP is set in sample_common_vpss_start() */
    if (g_vio_sys_cfg.vpss_fmu[grp] == OT_FMU_MODE_OFF) {
        const ot_low_delay_info low_delay_info = { TD_TRUE, 200, TD_FALSE }; /* 200: lowdelay line */
        if (ss_mpi_vpss_set_chn_low_delay(grp, 0, &low_delay_info) != TD_SUCCESS) {
            goto stop_vpss;
        }
    } else if (g_vio_sys_cfg.vpss_fmu[grp] == OT_FMU_MODE_DIRECT) {
        if (ss_mpi_vpss_set_chn_fmu_mode(grp, OT_VPSS_DIRECT_CHN, g_vio_sys_cfg.vpss_fmu[grp]) != TD_SUCCESS) {
            goto stop_vpss;
        }
        if (ss_mpi_vpss_enable_chn(grp, OT_VPSS_DIRECT_CHN) != TD_SUCCESS) {
            goto stop_vpss;
        }
    }

    if (g_vio_sys_cfg.mode_type != OT_VI_ONLINE_VPSS_ONLINE) {
        ot_gdc_param gdc_param = {0};
        gdc_param.in_size.width  = g_vb_param.vb_size.width;
        gdc_param.in_size.height = g_vb_param.vb_size.height;
        gdc_param.cell_size = OT_LUT_CELL_SIZE_16;
        if (ss_mpi_vpss_set_grp_gdc_param(grp, &gdc_param) != TD_SUCCESS) {
            goto stop_vpss;
        }
    }

    return TD_SUCCESS;
stop_vpss:
    sample_common_vpss_stop(grp, vpss_cfg->chn_en, OT_VPSS_MAX_PHYS_CHN_NUM);
    return TD_FAILURE;
}

static td_void sample_vio_stop_vpss(ot_vpss_grp grp)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_FALSE, TD_FALSE, TD_FALSE};

    sample_common_vpss_stop(grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

static td_s32 sample_vio_start_venc(ot_venc_chn venc_chn[], size_t size, td_u32 chn_num)
{
    td_s32 i;
    td_s32 ret;

    if (chn_num > size) {
        return TD_FAILURE;
    }

    sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &g_venc_chn_param.venc_size);
    for (i = 0; i < (td_s32)chn_num; i++) {
        ret = sample_comm_venc_start(venc_chn[i], &g_venc_chn_param);
        if (ret != TD_SUCCESS) {
            goto exit;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, chn_num);
    if (ret != TD_SUCCESS) {
        goto exit;
    }

    return TD_SUCCESS;

exit:
    for (i = i - 1; i >= 0; i--) {
        sample_comm_venc_stop(venc_chn[i]);
    }
    return TD_FAILURE;
}

static td_void sample_vio_stop_venc(ot_venc_chn venc_chn[], size_t size, td_u32 chn_num)
{
    td_u32 i;

    if (chn_num > size) {
        return;
    }

    sample_comm_venc_stop_get_stream(chn_num);

    for (i = 0; i < chn_num; i++) {
        sample_comm_venc_stop(venc_chn[i]);
    }
}

static td_s32 sample_vio_start_vo(sample_vo_mode vo_mode)
{
    g_vo_cfg.vo_mode = vo_mode;

    return sample_comm_vo_start_vo(&g_vo_cfg);
}

static td_void sample_vio_stop_vo(td_void)
{
    sample_comm_vo_stop_vo(&g_vo_cfg);
}

static td_s32 sample_vio_start_venc_and_vo(ot_vpss_grp vpss_grp[], size_t size, td_u32 grp_num)
{
    td_u32 i;
    td_s32 ret;
    sample_vo_mode vo_mode = VO_MODE_1MUX;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (grp_num > size) {
        return TD_FAILURE;
    }

    if (grp_num > 1) {
        vo_mode = VO_MODE_4MUX;
    }

    ret = sample_vio_start_venc(venc_chn, sizeof(venc_chn) / sizeof(venc_chn[0]), grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_failed;
    }
    for (i = 0; i < grp_num; i++) {
        if (g_vio_sys_cfg.vpss_fmu[i] == OT_FMU_MODE_DIRECT) {
            sample_comm_vpss_bind_venc(vpss_grp[i], OT_VPSS_DIRECT_CHN, venc_chn[i]);
        } else {
            sample_comm_vpss_bind_venc(vpss_grp[i], OT_VPSS_CHN0, venc_chn[i]);
        }
    }

    ret = sample_vio_start_vo(vo_mode);
    if (ret != TD_SUCCESS) {
        goto start_vo_failed;
    }
    for (i = 0; i < grp_num; i++) {
        if (g_vio_sys_cfg.vpss_fmu[i] == OT_FMU_MODE_WRAP) {
            sample_comm_vpss_bind_vo(vpss_grp[i], OT_VPSS_CHN1, vo_layer, vo_chn[i]);
        } else {
            sample_comm_vpss_bind_vo(vpss_grp[i], OT_VPSS_CHN0, vo_layer, vo_chn[i]);
        }
    }

    return TD_SUCCESS;

start_vo_failed:
    for (i = 0; i < grp_num; i++) {
        if (g_vio_sys_cfg.vpss_fmu[i] == OT_FMU_MODE_DIRECT) {
            sample_comm_vpss_un_bind_venc(vpss_grp[i], OT_VPSS_DIRECT_CHN, venc_chn[i]);
        } else {
            sample_comm_vpss_un_bind_venc(vpss_grp[i], OT_VPSS_CHN0, venc_chn[i]);
        }
    }
    sample_vio_stop_venc(venc_chn, sizeof(venc_chn) / sizeof(venc_chn[0]), grp_num);
start_venc_failed:
    return TD_FAILURE;
}

static td_void sample_vio_stop_venc_and_vo(ot_vpss_grp vpss_grp[], size_t size, td_u32 grp_num)
{
    td_u32 i;
    const ot_vo_layer vo_layer = 0;
    ot_vo_chn vo_chn[4] = {0, 1, 2, 3};     /* 4: max chn num, 0/1/2/3 chn id */
    ot_venc_chn venc_chn[4] = {0, 1, 2, 3}; /* 4: max chn num, 0/1/2/3 chn id */

    if (grp_num > size) {
        return;
    }

    for (i = 0; i < grp_num; i++) {
        if (g_vio_sys_cfg.vpss_fmu[i] == OT_FMU_MODE_WRAP) {
            sample_comm_vpss_un_bind_vo(vpss_grp[i], OT_VPSS_CHN1, vo_layer, vo_chn[i]);
        } else {
            sample_comm_vpss_un_bind_vo(vpss_grp[i], OT_VPSS_CHN0, vo_layer, vo_chn[i]);
        }
        if (g_vio_sys_cfg.vpss_fmu[i] == OT_FMU_MODE_DIRECT) {
            sample_comm_vpss_un_bind_venc(vpss_grp[i], OT_VPSS_DIRECT_CHN, venc_chn[i]);
        } else {
            sample_comm_vpss_un_bind_venc(vpss_grp[i], OT_VPSS_CHN0, venc_chn[i]);
        }
    }

    sample_vio_stop_venc(venc_chn, sizeof(venc_chn) / sizeof(venc_chn[0]), grp_num);
    sample_vio_stop_vo();
}

static td_s32 sample_vio_start_route(sample_vi_cfg *vi_cfg, sample_vpss_cfg *vpss_cfg, td_s32 route_num)
{
    td_s32 i, j, ret;
    ot_vpss_grp vpss_grp[SAMPLE_VIO_MAX_ROUTE_NUM] = {0, 1, 2, 3};

   // sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &g_vb_param.vb_size);
    if (sample_vio_sys_init() != TD_SUCCESS) {
        return TD_FAILURE;
    }

    for (i = 0; i < route_num; i++) {
        ret = sample_comm_vi_start_vi(&vi_cfg[i]);
            if (ret != TD_SUCCESS) {
            goto start_vi_failed;
        }
    }

    for (i = 0; i < route_num; i++) {
        sample_comm_vi_bind_vpss(i, 0, vpss_grp[i], 0);
    }

    for (i = 0; i < route_num; i++) {
        ret = sample_vio_start_vpss(vpss_grp[i], vpss_cfg);
        if (ret != TD_SUCCESS) {
            goto start_vpss_failed;
        }
    }

    ret = sample_vio_start_venc_and_vo(vpss_grp, SAMPLE_VIO_MAX_ROUTE_NUM, route_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    return TD_SUCCESS;

start_venc_and_vo_failed:
start_vpss_failed:
    for (j = i - 1; j >= 0; j--) {
        sample_vio_stop_vpss(vpss_grp[j]);
    }
    for (i = 0; i < route_num; i++) {
        sample_comm_vi_un_bind_vpss(i, 0, vpss_grp[i], 0);
    }
start_vi_failed:
    for (j = i - 1; j >= 0; j--) {
        sample_comm_vi_stop_vi(&vi_cfg[j]);
    }
    sample_comm_sys_exit();
    return TD_FAILURE;
}

static td_void sample_vio_stop_route(sample_vi_cfg *vi_cfg, td_s32 route_num)
{
    td_s32 i;
    ot_vpss_grp vpss_grp[SAMPLE_VIO_MAX_ROUTE_NUM] = {0, 1, 2, 3};

    sample_vio_stop_venc_and_vo(vpss_grp, SAMPLE_VIO_MAX_ROUTE_NUM, route_num);
    for (i = 0; i < route_num; i++) {
        sample_vio_stop_vpss(vpss_grp[i]);
        sample_comm_vi_un_bind_vpss(i, 0, vpss_grp[i], 0);
        sample_comm_vi_stop_vi(&vi_cfg[i]);
    }
    sample_comm_sys_exit();
}

static td_void sample_vio_print_vi_mode_list(td_bool is_wdr_mode)
{
    printf("vi vpss mode list: \n");
    if (is_wdr_mode == TD_TRUE) {
        printf("    (0) VI_ONLINE_VPSS_ONLINE\n");
        printf("    (1) VI_ONLINE_VPSS_OFFLINE\n");
        printf("    (2) VI_OFFLINE_VPSS_OFFLINE\n");
        printf("please select mode:\n");
        return;
    }
    printf("    (0) VI_ONLINE_VPSS_ONLINE, FMU OFF\n");
    printf("    (1) VI_ONLINE_VPSS_OFFLINE, FMU OFF\n");
    printf("    (2) VI_OFFLINE_VPSS_OFFLINE, FMU DIRECT\n");
    printf("    (3) VI_OFFLINE_VPSS_OFFLINE, FMU OFF\n");
    printf("please select mode:\n");
}

static td_void sample_vio_get_vi_vpss_mode_by_char(td_char ch, td_bool is_wdr)
{
    switch (ch) {
        case '0':
            g_vio_sys_cfg.mode_type = OT_VI_ONLINE_VPSS_ONLINE;
            g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
            g_vb_param.blk_num[0] = 0; /* raw_vb num 0 */
            break;
        case '1':
            g_vio_sys_cfg.mode_type = OT_VI_ONLINE_VPSS_OFFLINE;
            g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
            g_vb_param.blk_num[0] = 0; /* raw_vb num 0 */
            g_vb_param.blk_num[1] = 8; /* yuv_vb num 8 */
            break;
        case '2':
            g_vio_sys_cfg.mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
            g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_DIRECT;
            g_vb_param.blk_num[0] = is_wdr ? 6 : 3; /* raw_vb num 6 or 3 */
            break;
        case '3':
            g_vio_sys_cfg.mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
            g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
            g_vb_param.blk_num[0] = is_wdr ? 6 : 3; /* raw_vb num 6 or 3 */
            break;
        default:
            g_vio_sys_cfg.mode_type = OT_VI_ONLINE_VPSS_ONLINE;
            g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
            g_vb_param.blk_num[0] = 0; /* raw_vb num 0 */
            break;
    }
}

static td_void sample_vio_get_vi_vpss_mode(td_bool is_wdr_mode)
{
    td_char ch = '0';
    td_char end_ch;
    td_char input[3] = {0}; /* max_len: 3 */
    td_s32 max_len = 3; /* max_len: 3 */

    if (is_wdr_mode == TD_TRUE) {
        end_ch = '2';
    } else {
        end_ch = '4';
    }

    sample_vio_print_vi_mode_list(is_wdr_mode);

    while (g_sig_flag == 0) {
        if (gets_s(input, max_len) != TD_NULL && strlen(input) == 1 && input[0] >= ch && input[0] <= end_ch) {
            break;
        } else {
            printf("\nInvalid param, please enter again!\n\n");
            sample_vio_print_vi_mode_list(is_wdr_mode);
        }
        (td_void)fflush(stdin);
    }

    sample_vio_get_vi_vpss_mode_by_char(input[0], is_wdr_mode);
}

static td_s32 sample_vio_all_mode(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;

    sample_vio_get_vi_vpss_mode(TD_FALSE);
    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_s32 sample_vio_wdr(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;

    sample_vio_get_vi_vpss_mode(TD_TRUE);

    if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sns_type = OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sns_type = OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1;
    } else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT) {
        sns_type = SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC850SL_MIPI_8M_30FPS_12BIT) {
        sns_type = SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1;
    }

    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_s32 sample_vio_switch_first_route(sample_sns_type sns_type, td_bool is_run_be)
{
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;
    ot_vpss_grp vpss_grp[1] = {0};
    const td_u32 grp_num = 1;
    const ot_vpss_chn vpss_chn = 0;
    sample_vi_cfg vi_cfg;
    sample_vpss_cfg vpss_cfg;
    ot_size in_size;
    sample_run_be_bind_pipe bind_pipe = {
        .wdr_mode = OT_WDR_MODE_NONE,
        .pipe_id = {0},
        .pipe_num = 1
    };
    td_s32 ret;

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    sample_comm_vi_get_default_vi_cfg(sns_type, &vi_cfg);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);
    vi_cfg.pipe_info[0].isp_be_end_trigger = is_run_be;
    ret = sample_comm_vi_start_vi(&vi_cfg);
    if (ret != TD_SUCCESS) {
        goto start_vi_failed;
    }
    sample_comm_vi_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    ret = sample_vio_start_vpss(vpss_grp[0], &vpss_cfg);
    if (ret != TD_SUCCESS) {
        goto start_vpss_failed;
    }
    ret = sample_vio_start_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    if (is_run_be) {
        sample_comm_vi_send_run_be_frame(&bind_pipe);
    } else {
        sample_get_char();
    }

    sample_vio_stop_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
start_venc_and_vo_failed:
    sample_vio_stop_vpss(vpss_grp[0]);
start_vpss_failed:
    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    if (ret == TD_SUCCESS) {
        sample_comm_vi_mode_switch_stop_vi(&vi_cfg);
    } else {
        sample_comm_vi_stop_vi(&vi_cfg);
    }
start_vi_failed:
    return ret;
}

static td_s32 sample_vio_switch_second_route(sample_sns_type sns_type, td_bool is_run_be)
{
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;
    ot_vpss_grp vpss_grp[1] = {0};
    const td_u32 grp_num = 1;
    const ot_vpss_chn vpss_chn = 0;
    ot_size in_size;
    sample_vi_cfg vi_cfg;
    sample_vpss_cfg vpss_cfg;
    sample_run_be_bind_pipe bind_pipe = {
        .wdr_mode = OT_WDR_MODE_2To1_LINE,
        .pipe_id = {0, 1},
        .pipe_num = 2
    };
    td_s32 ret;

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    sample_comm_vi_get_default_vi_cfg(sns_type, &vi_cfg);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    ret = sample_comm_vi_mode_switch_start_vi(&vi_cfg, TD_FALSE, &in_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_comm_vi_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    ret = sample_vio_start_vpss(vpss_grp[0], &vpss_cfg);
    if (ret != TD_SUCCESS) {
        goto start_vpss_failed;
    }

    ret = sample_vio_start_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    if (is_run_be) {
        sample_comm_vi_send_run_be_frame(&bind_pipe);
    } else {
        sample_get_char();
    }

    sample_vio_stop_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
start_venc_and_vo_failed:
    sample_vio_stop_vpss(vpss_grp[0]);
start_vpss_failed:
    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    sample_comm_vi_stop_vi(&vi_cfg);

    return ret;
}

static td_void sample_vio_vpss_get_cfg_by_size(sample_sns_type sns_type,sample_vpss_cfg *vpss_cfg, ot_size *in_size)
{
    ot_vpss_chn chn;

    sample_comm_vpss_get_default_vpss_cfg(sns_type,vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);
    vpss_cfg->grp_attr.max_width = in_size->width;
    vpss_cfg->grp_attr.max_height = in_size->height;
    for (chn = 0; chn < OT_VPSS_MAX_PHYS_CHN_NUM; chn++) {
        vpss_cfg->chn_attr[chn].width = in_size->width;
        vpss_cfg->chn_attr[chn].height = in_size->height;
    }
}

static td_void sample_vio_venc_change_cfg_by_size(ot_size *in_size)
{
    ot_pic_size venc_size;
    if (in_size->width != g_venc_chn_param.venc_size.width || in_size->height != g_venc_chn_param.venc_size.height) {
        venc_size = sample_comm_sys_get_pic_enum(in_size);
        g_venc_chn_param.venc_size.width = in_size->width;
        g_venc_chn_param.venc_size.height = in_size->height;
        g_venc_chn_param.size = venc_size;
    }
    return;
}

static td_s32 sample_vio_switch_resolution_route(sample_sns_type sns_type)
{
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;
    ot_vpss_grp vpss_grp[1] = {0};
    const td_u32 grp_num = 1;
    const ot_vpss_chn vpss_chn = 0;
    ot_size in_size;
    sample_vi_cfg vi_cfg;
    sample_vpss_cfg vpss_cfg;
    td_s32 ret;

    in_size.width = 1920;  //  switch to 1920
    in_size.height = 1080; //  switch to 1080

    sample_comm_vi_init_vi_cfg(sns_type, &in_size, &vi_cfg);
    sample_vio_vpss_get_cfg_by_size(sns_type,&vpss_cfg, &in_size);
    sample_vio_venc_change_cfg_by_size(&in_size);

    ret = sample_comm_vi_mode_switch_start_vi(&vi_cfg, TD_TRUE, &in_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_comm_vi_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    ret = sample_vio_start_vpss(vpss_grp[0], &vpss_cfg);
    if (ret != TD_SUCCESS) {
        goto start_vpss_failed;
    }

    ret = sample_vio_start_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    sample_get_char();

    sample_vio_stop_venc_and_vo(vpss_grp, sizeof(vpss_grp) / sizeof(vpss_grp[0]), grp_num);
start_venc_and_vo_failed:
    sample_vio_stop_vpss(vpss_grp[0]);
start_vpss_failed:
    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp[0], vpss_chn);
    sample_comm_vi_stop_vi(&vi_cfg);

    return ret;
}

static td_s32 sample_vio_switch_mode(td_void)
{
    td_s32 ret;
    sample_sns_type sns_type = SENSOR0_TYPE;
    td_bool is_run_be = TD_FALSE;

    sample_vio_get_vi_vpss_mode(TD_TRUE);
    sample_comm_vi_get_size_by_sns_type(sns_type, &g_vb_param.vb_size);
    ret = sample_vio_sys_init();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_vio_switch_first_route(sns_type, is_run_be);
    if (ret != TD_SUCCESS) {
        sample_comm_sys_exit();
        return ret;
    }

    if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sns_type = OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1) {
        sns_type = OV_OS08A20_MIPI_8M_30FPS_12BIT;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sns_type = OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1) {
        sns_type = OV_OS04A10_MIPI_4M_30FPS_12BIT;
    }  else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT) {
        sns_type = SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sns_type = SC450AI_MIPI_4M_30FPS_10BIT;
    }  else if (sns_type == SC850SL_MIPI_8M_30FPS_12BIT) {
        sns_type = SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1) {
        sns_type = SC850SL_MIPI_8M_30FPS_12BIT;
    } else {
        printf("not support other sensor:%d switch wdr_mode\n", sns_type);
        return TD_FAILURE;
    }

    ret = sample_vio_switch_second_route(sns_type, is_run_be);
    sample_comm_sys_exit();
    return ret;
}

static td_s32 sample_vio_switch_resolution(td_void)
{
    td_s32 ret;

    sample_sns_type sns_type = SENSOR0_TYPE;
    td_bool is_run_be = TD_FALSE;

    sample_vio_get_vi_vpss_mode(TD_TRUE);
    sample_comm_vi_get_size_by_sns_type(sns_type, &g_vb_param.vb_size);
    ret = sample_vio_sys_init();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_vio_switch_first_route(sns_type, is_run_be);
    if (ret != TD_SUCCESS) {
        sample_comm_sys_exit();
        return ret;
    }

    ret = sample_vio_switch_resolution_route(sns_type);
    sample_comm_sys_exit();
    return ret;
}

static td_s32 sample_vio_run_be_switch_mode(td_void)
{
    td_s32 ret;
    sample_sns_type sns_type = SENSOR0_TYPE;
    td_bool is_run_be = TD_TRUE;

    sample_comm_vi_get_size_by_sns_type(sns_type, &g_vb_param.vb_size);
    g_vb_param.blk_num[0] = 5; /* raw_vb num 5 */
    ret = sample_vio_sys_init();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_vio_switch_first_route(sns_type, is_run_be);
    if (ret != TD_SUCCESS) {
        sample_comm_sys_exit();
        return ret;
    }

    if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sns_type = OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1) {
        sns_type = OV_OS08A20_MIPI_8M_30FPS_12BIT;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sns_type = OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1;
    } else if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1) {
        sns_type = OV_OS04A10_MIPI_4M_30FPS_12BIT;
    } else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT) {
        sns_type = SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC450AI_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sns_type = SC450AI_MIPI_4M_30FPS_10BIT;
    } else if (sns_type == SC850SL_MIPI_8M_30FPS_12BIT) {
        sns_type = SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1;
    } else if (sns_type == SC850SL_MIPI_8M_30FPS_10BIT_WDR2TO1) {
        sns_type = SC850SL_MIPI_8M_30FPS_12BIT;
    } else {
        printf("not support other sensor:%d switch wdr_mode\n", sns_type);
        sample_comm_sys_exit();
        return TD_FAILURE;
    }

    ret = sample_vio_switch_second_route(sns_type, is_run_be);
    sample_comm_sys_exit();
    return ret;
}

static td_void sample_vio_restart_get_venc_stream(ot_venc_chn venc_chn[], td_u32 chn_num)
{
    td_u32 i;
    td_s32 ret;
    ot_venc_start_param start_param;

    for (i = 0; i < chn_num; i++) {
        start_param.recv_pic_num = -1;
        if ((ret = ss_mpi_venc_start_chn(venc_chn[i], &start_param)) != TD_SUCCESS) {
            sample_print("ss_mpi_venc_start_recv_pic failed with%#x! \n", ret);
            return;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, chn_num);
    if (ret != TD_SUCCESS) {
        for (i = 0; i < chn_num; i++) {
            if ((ret = ss_mpi_venc_stop_chn(venc_chn[i])) != TD_SUCCESS) {
                sample_print("ss_mpi_venc_stop_recv_pic failed with%#x! \n", ret);
            }
        }
    }
}

static td_void sample_vio_do_fpn_calibrate_and_correction(ot_vi_pipe vi_pipe)
{
    ot_venc_chn venc_chn[1] = {0};
    const td_u32 chn_num = 1;

    sample_comm_venc_stop_get_stream(chn_num);
    sample_comm_vi_fpn_calibrate(vi_pipe, &g_calibration_cfg);

    printf("please enter any key to enable fpn correction!\n");
    sample_get_char();

    sample_vio_restart_get_venc_stream(venc_chn, chn_num);
    sample_comm_vi_enable_fpn_correction(vi_pipe, &g_correction_cfg);
}

static td_s32 sample_vio_fpn(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    const ot_vi_pipe vi_pipe = 0;

    g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_DIRECT;
    g_vb_param.blk_num[0] = 5; /* raw_vb num 5 */
    sample_comm_vi_get_vi_cfg_by_fmu_mode(SENSOR0_TYPE, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(SENSOR0_TYPE,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_vio_do_fpn_calibrate_and_correction(vi_pipe);

    sample_get_char();

    sample_comm_vi_disable_fpn_correction(vi_pipe, &g_correction_cfg);

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_vio_set_dis_en(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_bool enable)
{
    td_s32 ret;
    ot_dis_cfg dis_cfg = {0};
    ot_dis_attr dis_attr = {0};

    dis_cfg.motion_level  = OT_DIS_MOTION_LEVEL_NORM;
    dis_cfg.crop_ratio    = 80; /* 80 sample crop  ratio */
    dis_cfg.buf_num       = 10; /* 10 sample buf   num   */
    dis_cfg.frame_rate    = 30; /* 30 sample frame rate  */
    dis_cfg.camera_steady = TD_FALSE;
    dis_cfg.scale         = TD_TRUE;
    dis_cfg.pdt_type      = OT_DIS_PDT_TYPE_RECORDER;
    dis_cfg.mode          = OT_DIS_MODE_6_DOF_GME;
    ret = ss_mpi_vi_set_chn_dis_cfg(vi_pipe, vi_chn, &dis_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("set dis config failed.ret:0x%x !\n", ret);
    }

    dis_attr.enable               = enable;
    dis_attr.moving_subject_level = 0;
    dis_attr.rolling_shutter_coef = 0;
    dis_attr.timelag              = 1000;     /* 1000: timelag */
    dis_attr.still_crop           = TD_FALSE;
    dis_attr.hor_limit            = 512;      /* 512  sample hor_limit */
    dis_attr.ver_limit            = 512;      /* 512  sample ver_limit */
    dis_attr.gdc_bypass           = TD_FALSE;
    dis_attr.strength             = 1024;     /* 1024 sample strength  */
    ret = ss_mpi_vi_set_chn_dis_attr(vi_pipe, vi_chn, &dis_attr);
    if (ret != TD_SUCCESS) {
        sample_print("set dis attr failed.ret:0x%x !\n", ret);
    }
}

static td_void sample_vio_set_ldc_en(ot_vpss_grp grp, td_bool enable)
{
    td_s32 ret;
    ot_ldc_attr ldc_attr;

    ldc_attr.enable                       = enable;
    ldc_attr.ldc_version                  = OT_LDC_V1;
    ldc_attr.ldc_v1_attr.aspect           = 0;
    ldc_attr.ldc_v1_attr.x_ratio          = 100; /* 100: x ratio */
    ldc_attr.ldc_v1_attr.y_ratio          = 100; /* 100: y ratio */
    ldc_attr.ldc_v1_attr.xy_ratio         = 100; /* 100: x y ratio */
    ldc_attr.ldc_v1_attr.center_x_offset  = 0;
    ldc_attr.ldc_v1_attr.center_y_offset  = 0;
    ldc_attr.ldc_v1_attr.distortion_ratio = 500; /* 500: distortion ratio */

    ret = ss_mpi_vpss_set_grp_ldc(grp, &ldc_attr);
    if (ret != TD_SUCCESS) {
        sample_print("set ldc attr failed.ret:0x%x !\n", ret);
    }
}

static td_void sample_vio_switch_ldc_dis_en(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn, td_u32 width, td_u32 height)
{
    printf("please enter any key to enable dis!\n");
    sample_get_char();

    sample_vio_set_dis_en(vi_pipe, vi_chn, TD_TRUE);

    printf("please enter any key to disable dis!\n");
    sample_get_char();

    sample_vio_set_dis_en(vi_pipe, vi_chn, TD_FALSE);
}

static td_s32 sample_vio_dis_3dnr(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;

    g_vio_sys_cfg.nr_pos = OT_3DNR_POS_VPSS;
    g_vb_param.blk_num[0] = 4; /* raw_vb num 4 */
    sample_comm_vi_get_vi_cfg_by_fmu_mode(SENSOR0_TYPE, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(SENSOR0_TYPE,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);
    vi_cfg[0].pipe_info[0].nr_attr.enable = TD_FALSE;
    vi_cfg[0].pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_TILE;
    vi_cfg[0].pipe_info[0].chn_info[0].chn_attr.video_format = OT_VIDEO_FORMAT_TILE_32x4;
    vpss_cfg.nr_attr.enable = TD_TRUE;

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_vio_switch_ldc_dis_en(vi_pipe, vi_chn, g_vb_param.vb_size.width, g_vb_param.vb_size.height);
    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_vio_switch_3dnr_ldc_en(ot_vi_pipe vi_pipe)
{
    printf("please enter any key to enable ldc!\n");
    sample_get_char();

    sample_vio_set_ldc_en(vi_pipe, TD_TRUE);

    printf("please enter any key to disable ldc!\n");
    sample_get_char();

    sample_vio_set_ldc_en(vi_pipe, TD_FALSE);
}

static td_s32 sample_vio_3dnr_ldc(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    const ot_vi_pipe vi_pipe = 0;

    g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
    g_vb_param.blk_num[0] = 4; /* raw_vb num 4 */
    sample_comm_vi_get_vi_cfg_by_fmu_mode(SENSOR0_TYPE, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(SENSOR0_TYPE,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);
    vi_cfg[0].pipe_info[0].chn_info[0].chn_attr.compress_mode = OT_COMPRESS_MODE_TILE;
    vi_cfg[0].pipe_info[0].chn_info[0].chn_attr.video_format = OT_VIDEO_FORMAT_TILE_32x4;

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_vio_switch_3dnr_ldc_en(vi_pipe);
    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_vio_switch_low_delay(ot_vi_pipe vi_pipe, ot_vi_chn vi_chn)
{
    td_s32 ret;
    ot_low_delay_info low_delay_info;

    low_delay_info.enable = TD_TRUE;
    low_delay_info.line_cnt = 300; /* 300: low delay line cnt */
    low_delay_info.one_buf_en = TD_FALSE;

    printf("please enter any key to enable pipe low delay!\n");
    sample_get_char();

    ret = ss_mpi_vi_set_pipe_low_delay(vi_pipe, &low_delay_info);
    if (ret != TD_SUCCESS) {
        sample_print("enable pipe low delay failed!\n");
    }

    printf("please enter any key to disable pipe low delay!\n");
    sample_get_char();

    low_delay_info.enable = TD_FALSE;
    ret = ss_mpi_vi_set_pipe_low_delay(vi_pipe, &low_delay_info);
    if (ret != TD_SUCCESS) {
        sample_print("disable pipe low delay failed!\n");
    }

    printf("please enter any key to enable chn low delay!\n");
    sample_get_char();

    low_delay_info.enable = TD_TRUE;
    ret = ss_mpi_vi_set_chn_low_delay(vi_pipe, vi_chn, &low_delay_info);
    if (ret != TD_SUCCESS) {
        sample_print("enable chn low delay failed!\n");
    }

    printf("please enter any key to disable chn low delay!\n");
    sample_get_char();

    low_delay_info.enable = TD_FALSE;
    ret = ss_mpi_vi_set_chn_low_delay(vi_pipe, vi_chn, &low_delay_info);
    if (ret != TD_SUCCESS) {
        sample_print("disable chn low delay failed!\n");
    }
}

static td_s32 sample_vio_lowdelay(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;

    g_vb_param.blk_num[0] = 4; /* raw_vb num 4 */
    sample_comm_vi_get_default_vi_cfg(sns_type, &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_vio_switch_low_delay(vi_pipe, vi_chn);
    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_switch_user_pic(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    sample_vi_user_pic_type user_pic_type;
    sample_vi_user_frame_info user_frame_info = {0};

    for (user_pic_type = VI_USER_PIC_FRAME; user_pic_type <= VI_USER_PIC_BGCOLOR; user_pic_type++) {
        ret = sample_common_vi_load_user_pic(vi_pipe, user_pic_type, &user_frame_info);
        if (ret != TD_SUCCESS) {
            sample_print("load user pic failed!\n");
            return;
        }

        ret = ss_mpi_vi_set_pipe_user_pic(vi_pipe, &user_frame_info.frame_info);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_set_pipe_user_pic failed!\n");
        }

        printf("Enter any key to enable user pic!\n");
        sample_get_char();
        ret = ss_mpi_vi_enable_pipe_user_pic(vi_pipe);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_enable_pipe_user_pic failed!\n");
        }

        printf("Enter any key to disable user pic!\n");
        sample_get_char();
        ret = ss_mpi_vi_disable_pipe_user_pic(vi_pipe);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_vi_disable_pipe_user_pic failed!\n");
        }

        sleep(1);
        sample_common_vi_unload_user_pic(&user_frame_info);
    }
}

static td_s32 sample_vio_user_pic(td_void)
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;
    const ot_vi_pipe vi_pipe = 0;

    g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_DIRECT;
    g_vb_param.blk_num[0] = 4; /* raw_vb num 4 */
    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    if (sample_vio_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_switch_user_pic(vi_pipe);
    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_vi_get_two_sensor_vi_cfg(sample_sns_type *sns_type, sample_vi_cfg vi_cfg[], size_t size)
{
    const ot_vi_dev vi_dev = 1; /* dev2 for sensor1 */
    const ot_vi_pipe vi_pipe = 1; /* dev2 bind pipe1 */

    if (size < 2) { /* need 2 sensor cfg */
        return;
    }

    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type[0], g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type[1], g_vio_sys_cfg.vi_fmu[1], &vi_cfg[1]);

    vi_cfg[0].mipi_info.divide_mode = LANE_DIVIDE_MODE_3;
#ifdef OT_FPGA
    vi_cfg[1].sns_info.bus_id = 1; /* i2c1 */
#else
    vi_cfg[1].sns_info.bus_id = 4; /* i2c5 */
#endif
    vi_cfg[1].sns_info.sns_clk_src = 1;
    vi_cfg[1].sns_info.sns_rst_src = 1;

    sample_comm_vi_get_mipi_info_by_dev_id(sns_type[1], vi_dev, &vi_cfg[1].mipi_info);
    vi_cfg[1].dev_info.vi_dev = vi_dev;
    vi_cfg[1].bind_pipe.pipe_id[0] = vi_pipe;
    vi_cfg[1].grp_info.grp_num = 1;
    vi_cfg[1].grp_info.fusion_grp[0] = 1;
    vi_cfg[1].grp_info.fusion_grp_attr[0].pipe_id[0] = vi_pipe;

    /* total performance does not support 4K@60. */
    if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT || sns_type == SONY_IMX515_MIPI_8M_30FPS_12BIT) {
        vi_cfg[0].pipe_info[0].pipe_attr.frame_rate_ctrl.src_frame_rate = 30; /* 30: src_rate */
        vi_cfg[0].pipe_info[0].pipe_attr.frame_rate_ctrl.dst_frame_rate = 20; /* 20: dst_rate */
        vi_cfg[1].pipe_info[0].pipe_attr.frame_rate_ctrl.src_frame_rate = 30; /* 30: src_rate */
        vi_cfg[1].pipe_info[0].pipe_attr.frame_rate_ctrl.dst_frame_rate = 20; /* 20: dst_rate */
    }
}

/* Set pin_mux i2c4 & i2c5 & sensor0 & sensor1 & MIPI0 & MIPI1 before using this sample! */
static td_s32 sample_vio_two_sensor(td_void)
{
    sample_vi_cfg vi_cfg[2]={0};
    sample_vpss_cfg vpss_cfg[2];
    sample_sns_type sns_type[] = {SENSOR0_TYPE,SENSOR1_TYPE};
	//sample_sns_type sns_type1 = SENSOR1_TYPE;
    g_vio_sys_cfg.route_num = 2; /* 2: route_num */
    g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_DIRECT;
    g_vio_sys_cfg.vi_fmu[1] = OT_FMU_MODE_DIRECT;
    g_vb_param.blk_num[0] = 8; /* raw_vb num 8 */
    g_vb_param.blk_num[1] = 8; /* raw_vb num 8 */
    sample_vi_get_two_sensor_vi_cfg(sns_type, vi_cfg, sizeof(vi_cfg) / sizeof(vi_cfg[0]));
    sample_comm_vpss_get_default_vpss_cfg(sns_type[0],&vpss_cfg[0], g_vio_sys_cfg.vpss_fmu[0]);
	sample_comm_vpss_get_default_vpss_cfg(sns_type[1],&vpss_cfg[1], g_vio_sys_cfg.vpss_fmu[1]);
    if (sample_vio_start_route(vi_cfg, vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_get_char();

    sample_vio_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_vio_usage(const char *prg_name)
{
    printf("usage : %s <index> \n", prg_name);
    printf("index:\n");
    printf("    (0) all mode route          :vi linear(Online/Offline) -> vpss(Online/Offline) -> venc && vo.\n");
    printf("    (1) wdr route               :vi wdr(Online) -> vpss(Offline) -> venc && vo.\n");
    printf("    (2) fpn calibrate & correct :vi fpn calibrate & correct -> vpss -> venc && vo.\n");
    printf("    (3) dis & 3dnr(VPSS)        :vi dis -> vpss 3dnr -> venc && vo.\n");
    printf("    (4) 3dnr(VI) & ldc          :vi 3dnr -> vpss ldc -> venc && vo.\n");
    printf("    (5) low delay               :vi(pipe & chn lowdelay) -> vpss(lowdelay) -> venc && vo.\n");
    printf("    (6) user pic                :vi user pic (offline) -> vpss -> venc && vo.\n");
    printf("    (7) two sensor              :vi two sensor (offline) -> vpss -> venc && vo.\n");
    printf("    (8) switch mode             :vi linear switch to wdr -> vpss -> venc && vo.\n");
    printf("    (9) switch resolution       :vi FHD switch to 720P or 4K switch to FHD -> vpss -> venc && vo.\n");
    printf("    (10) run be switch mode     :vi linear switch to wdr -> vpss -> venc && vo.\n");
}

static td_void sample_vio_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sig_flag = 1;
    }
}

static td_void sample_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

static td_s32 sample_vio_execute_case(td_u32 case_index)
{
    td_s32 ret;

    switch (case_index) {
        case 0: /* 0 all mode route */
            ret = sample_vio_all_mode();
            break;
        case 1: /* 1 wdr route */
            ret = sample_vio_wdr();
            break;
        case 2: /* 2 fpn calibrate and correct */
            ret = sample_vio_fpn();
            break;
        case 3: /* 3 ldc and dis */
            ret = sample_vio_dis_3dnr();
            break;
        case 4: /* 4 3dnr and ldc */
            ret = sample_vio_3dnr_ldc();
            break;
        case 5: /* 5 low delay */
            ret = sample_vio_lowdelay();
            break;
        case 6: /* 6 user pic */
            ret = sample_vio_user_pic();
            break;
        case 7: /* 7 two sensor */
            ret = sample_vio_two_sensor();
            break;
        case 8: /* 8 switch mode */
            ret = sample_vio_switch_mode();
            break;
        case 9: /* 9 switch resolution */
            ret = sample_vio_switch_resolution();
            break;
        case 10: /* 10 run be switch mode */
            ret = sample_vio_run_be_switch_mode();
            break;
        default:
            ret = TD_FAILURE;
            break;
    }

    return ret;
}

static td_s32 sample_vio_msg_proc_vb_pool_share(td_s32 pid)
{
    td_s32 ret;
    td_u32 i;
    td_bool isp_states[OT_VI_MAX_PIPE_NUM];
#ifndef SAMPLE_MEM_SHARE_ENABLE
    ot_vb_common_pools_id pools_id = {0};

    if (ss_mpi_vb_get_common_pool_id(&pools_id) != TD_SUCCESS) {
        sample_print("get common pool_id failed!\n");
        return TD_FAILURE;
    }

    for (i = 0; i < pools_id.pool_cnt; ++i) {
        if (ss_mpi_vb_pool_share(pools_id.pool[i], pid) != TD_SUCCESS) {
            sample_print("vb pool share failed!\n");
            return TD_FAILURE;
        }
    }
#endif
    ret = sample_comm_vi_get_isp_run_state(isp_states, OT_VI_MAX_PIPE_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("get isp states fail\n");
        return TD_FAILURE;
    }

    for (i = 0; i < OT_VI_MAX_PIPE_NUM; i++) {
        if (!isp_states[i]) {
            continue;
        }
        ret = ss_mpi_isp_mem_share(i, pid);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_isp_mem_share vi_pipe %u, pid %d fail\n", i, pid);
        }
    }

    return TD_SUCCESS;
}

static td_void sample_vio_msg_proc_vb_pool_unshare(td_s32 pid)
{
    td_s32 ret;
    td_u32 i;
    td_bool isp_states[OT_VI_MAX_PIPE_NUM];
#ifndef SAMPLE_MEM_SHARE_ENABLE
    ot_vb_common_pools_id pools_id = {0};
    if (ss_mpi_vb_get_common_pool_id(&pools_id) == TD_SUCCESS) {
        for (i = 0; i < pools_id.pool_cnt; ++i) {
            ret = ss_mpi_vb_pool_unshare(pools_id.pool[i], pid);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_vb_pool_unshare vi_pipe %u, pid %d fail\n", pools_id.pool[i], pid);
            }
        }
    }
#endif
    ret = sample_comm_vi_get_isp_run_state(isp_states, OT_VI_MAX_PIPE_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("get isp states fail\n");
        return;
    }

    for (i = 0; i < OT_VI_MAX_PIPE_NUM; i++) {
        if (!isp_states[i]) {
            continue;
        }
        ret = ss_mpi_isp_mem_unshare(i, pid);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_isp_mem_unshare vi_pipe %u, pid %d fail\n", i, pid);
        }
    }
}

static td_s32 sample_vio_ipc_msg_proc(const sample_ipc_msg_req_buf *msg_req_buf,
    td_bool *is_need_fb, sample_ipc_msg_res_buf *msg_res_buf)
{
    td_s32 ret;

    if (msg_req_buf == TD_NULL || is_need_fb == TD_NULL) {
        return TD_FAILURE;
    }

    /* need feedback default */
    *is_need_fb = TD_TRUE;

    switch ((sample_msg_type)msg_req_buf->msg_type) {
        case SAMPLE_MSG_TYPE_VB_POOL_SHARE_REQ: {
            if (msg_res_buf == TD_NULL) {
                return TD_FAILURE;
            }
            ret = sample_vio_msg_proc_vb_pool_share(msg_req_buf->msg_data.pid);
            msg_res_buf->msg_type = SAMPLE_MSG_TYPE_VB_POOL_SHARE_RES;
            msg_res_buf->msg_data.is_req_success = (ret == TD_SUCCESS) ? TD_TRUE : TD_FALSE;
            break;
        }
        case SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_REQ: {
            if (msg_res_buf == TD_NULL) {
                return TD_FAILURE;
            }
            sample_vio_msg_proc_vb_pool_unshare(msg_req_buf->msg_data.pid);
            msg_res_buf->msg_type = SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_RES;
            msg_res_buf->msg_data.is_req_success = TD_TRUE;
            break;
        }
        default: {
            printf("unsupported msg type(%ld)!\n", msg_req_buf->msg_type);
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;
    td_u32 index;
    td_char *end_ptr = TD_NULL;

    if (argc != 2) { /* 2:arg num */
        sample_vio_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_vio_usage(argv[0]);
        return TD_FAILURE;
    }

    if (strlen(argv[1]) > 2 || strlen(argv[1]) == 0 || !check_digit(argv[1][0]) || /* 2:arg len */
        (strlen(argv[1]) == 2 && (!check_digit(argv[1][1]) || argv[1][0] == '0'))) { /* 2:arg len */
        sample_vio_usage(argv[0]);
        return TD_FAILURE;
    }

    if (strlen(argv[1]) == 2 && argv[1][1] != '0') { /* 2:arg len, max: 10 */
        sample_vio_usage(argv[0]);
        return TD_FAILURE;
    }

    index = (td_u32)strtol(argv[1], &end_ptr, 10); /* base 10, argv[1] has been check between [0, 10] */
    if ((end_ptr == argv[1]) || (*end_ptr) != '\0') {
        sample_vio_usage(argv[0]);
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    sample_register_sig_handler(sample_vio_handle_sig);
#endif

    if (sample_ipc_server_init(sample_vio_ipc_msg_proc) != TD_SUCCESS) {
        printf("sample_ipc_server_init failed!!!\n");
    }

    ret = sample_vio_execute_case(index);
    if ((ret == TD_SUCCESS) && (g_sig_flag == 0)) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    sample_ipc_server_deinit();
    return ret;
}
