/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "sample_ipc.h"
#include "securec.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_awb.h"
#include "ot_sns_ctrl.h"


#define MIPI_DEV_NAME "/dev/ot_mipi_rx"
#define ARGV_AE_EXP 1
#define ARGV_AE_AGAIN 2
#define ARGV_AE_DGAIN 3
#define ARGV_AE_ISP_DGAIN 4
#define ARGV_AE_EXPOSURE 5
#define ARGV_AE_LINE_PER500MS 6
#define ARGV_AWB_RGAIN 7
#define ARGV_AWB_GGAIN 8
#define ARGV_AWB_BGAIN 9

static volatile sig_atomic_t g_sig_flag = 0;
static td_u32 g_sns_exp, g_sns_again, g_sns_dgain, g_isp_dgain, g_exposure, g_lines_per500ms;
static td_u32 g_awb_rgain, g_awb_ggain, g_awb_bgain;


/* this configuration is used to adjust the size and number of buffer(VB).  */
static sample_vb_param g_vb_param = {
    .vb_size = {2688, 1520},
    .pixel_format = {OT_PIXEL_FORMAT_RGB_BAYER_12BPP, OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420},
    .compress_mode = {OT_COMPRESS_MODE_LINE, OT_COMPRESS_MODE_SEG},
    .video_format = {OT_VIDEO_FORMAT_LINEAR, OT_VIDEO_FORMAT_LINEAR},
    .blk_num = {4, 6}
};

static sampe_sys_cfg g_vio_sys_cfg = {
    .route_num = 1,
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

void sensor_os04a10_init(td_u32 sns_exp, td_u32 sns_again, td_u32 sns_dgain);
void sensor_os04a10_read_exp(td_u32 *sns_exp);
void sensor_os04a10_read_gain (td_u32 *sns_again, td_u32 *sns_dgain);

void sensor_os08a20_init(td_u32 sns_exp, td_u32 sns_again, td_u32 sns_dgain);
void sensor_os08a20_read_exp(td_u32 *sns_exp);
void sensor_os08a20_read_gain (td_u32 *sns_again, td_u32 *sns_dgain);

static td_void sample_get_char(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }

    sample_pause();
}

static td_s32 sample_comm_vi_mipi_ctrl_cmd(td_u32 devno, td_u32 cmd)
{
    td_s32 ret;
    td_s32 fd;

    fd = open(MIPI_DEV_NAME, O_RDWR);
    if (fd < 0) {
        sample_print("open %s failed!\n", MIPI_DEV_NAME);
        return TD_FAILURE;
    }

    ret = ioctl(fd, cmd, &devno);

    close(fd);

    return ret;
}

static td_s32 sample_quick_start_vi_start_mipi_rx(const sample_sns_info *sns_info, const sample_mipi_info *mipi_info)
{
    td_s32 ret = 0;

    ret = sample_comm_vi_mipi_ctrl_cmd(sns_info->sns_clk_src, OT_MIPI_ENABLE_SENSOR_CLOCK);
    if (ret != TD_SUCCESS) {
        sample_print("devno %u enable sensor clock failed!\n", sns_info->sns_clk_src);
        return TD_FAILURE;
    }

    ret = sample_comm_vi_mipi_ctrl_cmd(sns_info->sns_rst_src, OT_MIPI_RESET_SENSOR);
    if (ret != TD_SUCCESS) {
        sample_print("devno %u reset sensor failed!\n", sns_info->sns_rst_src);
        return TD_FAILURE;
    }

    ret = sample_comm_vi_mipi_ctrl_cmd(sns_info->sns_rst_src, OT_MIPI_UNRESET_SENSOR);
    if (ret != TD_SUCCESS) {
        sample_print("devno %u unreset sensor failed!\n", sns_info->sns_rst_src);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}


static td_u32 sample_quick_start_get_fmu_wrap_num(ot_fmu_mode fmu_mode[], td_u32 len)
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

static td_s32 sample_quick_start_fmu_wrap_init(sampe_sys_cfg *fmu_cfg, ot_size *in_size)
{
    td_u32 cnt;
    ot_fmu_attr fmu_attr;

    cnt = sample_quick_start_get_fmu_wrap_num(fmu_cfg->vi_fmu, fmu_cfg->route_num);
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

    cnt = sample_quick_start_get_fmu_wrap_num(fmu_cfg->vpss_fmu, fmu_cfg->route_num);
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
td_void sample_quick_start_init_mem_share(td_void)
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

static td_s32 sample_quick_start_sys_init(td_void)
{
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK | OT_VB_SUPPLEMENT_MOTION_DATA_MASK;

    sample_comm_sys_get_default_vb_cfg(&g_vb_param, &vb_cfg);
    if (sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config) != TD_SUCCESS) {
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_quick_start_init_mem_share();
#endif

    if (sample_comm_vi_set_vi_vpss_mode(g_vio_sys_cfg.mode_type, OT_VI_AIISP_MODE_DEFAULT) != TD_SUCCESS) {
        goto sys_exit;
    }

    if (ss_mpi_sys_set_3dnr_pos(g_vio_sys_cfg.nr_pos) != TD_SUCCESS) {
        goto sys_exit;
    }

    if (sample_quick_start_fmu_wrap_init(&g_vio_sys_cfg, &g_vb_param.vb_size) != TD_SUCCESS) {
        goto sys_exit;
    }

    return TD_SUCCESS;
sys_exit:
    sample_comm_sys_exit();
    return TD_FAILURE;
}

static td_s32 sample_quick_start_start_vpss(ot_vpss_grp grp, sample_vpss_cfg *vpss_cfg)
{
    td_s32 ret = TD_SUCCESS;
    sample_vpss_chn_attr vpss_chn_attr = {0};

    ret = memcpy_s(&vpss_chn_attr.chn_attr[0], sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM,
        vpss_cfg->chn_attr, sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM);
    if (g_vio_sys_cfg.vpss_fmu[grp] == OT_FMU_MODE_WRAP) {
        vpss_chn_attr.chn0_wrap = TD_TRUE;
    }
    ret += memcpy_s(vpss_chn_attr.chn_enable, sizeof(vpss_chn_attr.chn_enable),
        vpss_cfg->chn_en, sizeof(vpss_chn_attr.chn_enable));
    if (ret != TD_SUCCESS) {
        return ret;
    }
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

static td_void sample_quick_start_stop_vpss(ot_vpss_grp grp)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE, TD_FALSE, TD_FALSE, TD_FALSE};

    sample_common_vpss_stop(grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

static td_s32 sample_quick_start_start_venc(ot_venc_chn venc_chn[], size_t size, td_u32 chn_num)
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

static td_void sample_quick_start_stop_venc(ot_venc_chn venc_chn[], size_t size, td_u32 chn_num)
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

static td_s32 sample_quick_start_start_vo(sample_vo_mode vo_mode)
{
    g_vo_cfg.vo_mode = vo_mode;

    return sample_comm_vo_start_vo(&g_vo_cfg);
}

static td_void sample_quick_start_stop_vo(td_void)
{
    sample_comm_vo_stop_vo(&g_vo_cfg);
}

static td_s32 sample_quick_start_start_venc_and_vo(ot_vpss_grp vpss_grp[], size_t size, td_u32 grp_num)
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

    ret = sample_quick_start_start_venc(venc_chn, size, grp_num);
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

    ret = sample_quick_start_start_vo(vo_mode);
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

start_venc_failed:
    sample_quick_start_stop_vo();
start_vo_failed:
    return TD_FAILURE;
}

static td_void sample_quick_start_stop_venc_and_vo(ot_vpss_grp vpss_grp[], size_t size, td_u32 grp_num)
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

    sample_quick_start_stop_venc(venc_chn, size, grp_num);
    sample_quick_start_stop_vo();
}

static td_s32 sample_quick_start_start_route(sample_vi_cfg *vi_cfg, sample_vpss_cfg *vpss_cfg, td_s32 route_num)
{
    td_s32 i, j, ret;
    ot_vpss_grp vpss_grp[SAMPLE_VIO_MAX_ROUTE_NUM] = {0, 1, 2, 3};

    sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &g_vb_param.vb_size);
    if (sample_quick_start_sys_init() != TD_SUCCESS) {
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
        ret = sample_quick_start_start_vpss(vpss_grp[i], vpss_cfg);
        if (ret != TD_SUCCESS) {
            goto start_vpss_failed;
        }
    }

    ret = sample_quick_start_start_venc_and_vo(vpss_grp, SAMPLE_VIO_MAX_ROUTE_NUM, route_num);
    if (ret != TD_SUCCESS) {
        goto start_venc_and_vo_failed;
    }

    return TD_SUCCESS;

start_venc_and_vo_failed:
start_vpss_failed:
    for (j = i - 1; j >= 0; j--) {
        sample_quick_start_stop_vpss(vpss_grp[j]);
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

static td_void sample_quick_start_stop_route(sample_vi_cfg *vi_cfg, td_s32 route_num)
{
    td_s32 i;
    ot_vpss_grp vpss_grp[SAMPLE_VIO_MAX_ROUTE_NUM] = {0, 1, 2, 3};

    sample_quick_start_stop_venc_and_vo(vpss_grp, SAMPLE_VIO_MAX_ROUTE_NUM, route_num);
    for (i = 0; i < route_num; i++) {
        sample_quick_start_stop_vpss(vpss_grp[i]);
        sample_comm_vi_un_bind_vpss(i, 0, vpss_grp[i], 0);
        sample_comm_vi_stop_vi(&vi_cfg[i]);
    }
    sample_comm_sys_exit();
}

static td_void sample_quick_start_get_vi_vpss_mode()
{
    g_vio_sys_cfg.mode_type = OT_VI_ONLINE_VPSS_ONLINE;
    g_vio_sys_cfg.vi_fmu[0] = OT_FMU_MODE_OFF;
    g_vb_param.blk_num[0] = 0; /* raw_vb num 0 */
}

static td_void sample_quick_start_read_exp(td_u32 *exposure, td_u32 *lines_per500ms, td_u32 *isp_dgain)
{
    td_s32 ret;
    ot_isp_exp_info exp_info;
    ret = ss_mpi_isp_query_exposure_info(0, &exp_info);
    if (ret == TD_SUCCESS) {
        *exposure = exp_info.exposure;
        *lines_per500ms = exp_info.lines_per500ms;
        *isp_dgain = exp_info.isp_d_gain;
    } else {
        *exposure = 0;
        *lines_per500ms = 0;
        *isp_dgain = 0;
    }
}

static td_void sample_quick_start_read_awb(td_u32 *awb_r, td_u32 *awb_g, td_u32 *awb_b)
{
    td_s32 ret;
    ot_isp_wb_info wb_info;
    ret = ss_mpi_isp_query_wb_info(0, &wb_info);
    if (ret == TD_SUCCESS) {
        *awb_r = wb_info.r_gain;
        *awb_g = wb_info.gr_gain;
        *awb_b = wb_info.b_gain;
    } else {
        *awb_r = 0;
        *awb_g = 0;
        *awb_b = 0;
    }
}

static td_void sample_quick_start_isp_init()
{
    sample_sns_type sns_type = SENSOR0_TYPE;
    ot_isp_sns_obj *sns_obj = TD_NULL;
    ot_isp_init_attr isp_init_attr = {0};
    if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sns_obj = &g_sns_os04a10_obj;
    } else if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sns_obj = &g_sns_os08a20_obj;
    }
    isp_init_attr.exp_time = g_sns_exp;
    isp_init_attr.a_gain = g_sns_again;
    isp_init_attr.d_gain = g_sns_dgain;
    isp_init_attr.ispd_gain = g_isp_dgain;
    isp_init_attr.wb_r_gain = g_awb_rgain;
    isp_init_attr.wb_g_gain = g_awb_ggain;
    isp_init_attr.wb_b_gain = g_awb_bgain;
    isp_init_attr.exposure = g_exposure;
    isp_init_attr.lines_per500ms = g_lines_per500ms;
    isp_init_attr.quick_start_en = TD_TRUE;
    if (sns_obj != TD_NULL) {
        sns_obj->pfn_set_init(0, &isp_init_attr);
    }
}

static td_s32 sample_quick_start()
{
    sample_vi_cfg vi_cfg[1];
    sample_vpss_cfg vpss_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;

    if (sns_type != OV_OS04A10_MIPI_4M_30FPS_12BIT && sns_type != OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        printf("quick start only support os04a10 and os08a20!\n");
        return TD_SUCCESS;
    }

    sample_quick_start_get_vi_vpss_mode();
    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type, g_vio_sys_cfg.vi_fmu[0], &vi_cfg[0]);
    sample_comm_vpss_get_default_vpss_cfg(sns_type,&vpss_cfg, g_vio_sys_cfg.vpss_fmu[0]);

    sample_quick_start_vi_start_mipi_rx(&vi_cfg[0].sns_info, &vi_cfg[0].mipi_info);
    vi_cfg[0].pipe_info[0].isp_quick_start = TD_TRUE;
    vi_cfg[0].sns_info.sns_clk_rst_en = TD_FALSE;
    if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sensor_os04a10_init(g_sns_exp, g_sns_again, g_sns_dgain);
    } else if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sensor_os08a20_init(g_sns_exp, g_sns_again, g_sns_dgain);
    }
    sample_quick_start_isp_init();

    if (sample_quick_start_start_route(vi_cfg, &vpss_cfg, g_vio_sys_cfg.route_num) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_get_char();
    if (sns_type == OV_OS04A10_MIPI_4M_30FPS_12BIT) {
        sensor_os04a10_read_exp(&g_sns_exp);
        sensor_os04a10_read_gain(&g_sns_again, &g_sns_dgain);
    } else if (sns_type == OV_OS08A20_MIPI_8M_30FPS_12BIT) {
        sensor_os08a20_read_exp(&g_sns_exp);
        sensor_os08a20_read_gain(&g_sns_again, &g_sns_dgain);
    }
    sample_quick_start_read_exp(&g_exposure, &g_lines_per500ms, &g_isp_dgain);
    sample_quick_start_read_awb(&g_awb_rgain, &g_awb_ggain, &g_awb_bgain);
    printf("sns_exp = %u again = %u dgain = %u isp_dgain = %u exposure = %u lines_per500ms = %u "
        "awb_rgain = %u, awb_ggain = %u, awb_bgain = %u\n",
        g_sns_exp, g_sns_again, g_sns_dgain, g_isp_dgain, g_exposure, g_lines_per500ms,
        g_awb_rgain, g_awb_ggain, g_awb_bgain);

    sample_quick_start_stop_route(vi_cfg, g_vio_sys_cfg.route_num);
    return TD_SUCCESS;
}

static td_void sample_quick_start_handle_sig(td_s32 signo)
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

static td_s32 sample_quick_start_msg_proc_vb_pool_share(td_s32 pid)
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

static td_void sample_quick_start_msg_proc_vb_pool_unshare(td_s32 pid)
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

static td_s32 sample_quick_start_ipc_msg_proc(const sample_ipc_msg_req_buf *msg_req_buf,
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
            ret = sample_quick_start_msg_proc_vb_pool_share(msg_req_buf->msg_data.pid);
            msg_res_buf->msg_type = SAMPLE_MSG_TYPE_VB_POOL_SHARE_RES;
            msg_res_buf->msg_data.is_req_success = (ret == TD_SUCCESS) ? TD_TRUE : TD_FALSE;
            break;
        }
        case SAMPLE_MSG_TYPE_VB_POOL_UNSHARE_REQ: {
            if (msg_res_buf == TD_NULL) {
                return TD_FAILURE;
            }
            sample_quick_start_msg_proc_vb_pool_unshare(msg_req_buf->msg_data.pid);
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

static td_void sample_quick_start_usage(const char *prg_name)
{
    printf("usage : %s <sns_exp> <sns_again> <sns_dgain> <isp_dgain> <exposure> "
        "<line_per500ms> <r_gain> <g_gain> <b_gain>\n", prg_name);
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret = TD_TRUE;
    td_char *para_stop;
    if (argc != 10) { /* 10 parameter. */
        sample_quick_start_usage(argv[0]);
        return TD_FAILURE;
    }
    g_sns_exp = (td_u32)strtol(argv[ARGV_AE_EXP], &para_stop, 10); /* 10 dec */
    g_sns_again = (td_u32)strtol(argv[ARGV_AE_AGAIN], &para_stop, 10); /* 10 dec */
    g_sns_dgain = (td_u32)strtol(argv[ARGV_AE_DGAIN], &para_stop, 10); /* 10 dec */
    g_isp_dgain = (td_u32)strtol(argv[ARGV_AE_ISP_DGAIN], &para_stop, 10); /* 10 dec */
    g_exposure = (td_u32)strtol(argv[ARGV_AE_EXPOSURE], &para_stop, 10); /* 10 dec */
    g_lines_per500ms = (td_u32)strtol(argv[ARGV_AE_LINE_PER500MS], &para_stop, 10); /* 10 dec */

    g_awb_rgain = (td_u32)strtol(argv[ARGV_AWB_RGAIN], &para_stop, 10); /* 10 dec */
    g_awb_ggain = (td_u32)strtol(argv[ARGV_AWB_GGAIN], &para_stop, 10); /* 10 dec */
    g_awb_bgain = (td_u32)strtol(argv[ARGV_AWB_BGAIN], &para_stop, 10); /* 10 dec */

#ifndef __LITEOS__
    sample_register_sig_handler(sample_quick_start_handle_sig);
#endif

    if (sample_ipc_server_init(sample_quick_start_ipc_msg_proc) != TD_SUCCESS) {
        printf("sample_ipc_server_init failed!!!\n");
    }

    ret = sample_quick_start();
    if ((ret == TD_SUCCESS) && (g_sig_flag == 0)) {
        printf("\033[0;32mprogram exit normally!\033[0;39m\n");
    } else {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    sample_ipc_server_deinit();
    return ret;
}
