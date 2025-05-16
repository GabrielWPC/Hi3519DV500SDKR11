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
#ifdef OT_GYRODIS_SUPPORT
#include "sample_gyro_dis.h"
#include "sample_dis_two_pipe.h"
#endif

#include "sample_comm.h"
#include "ss_mpi_isp.h"

typedef enum {
    SAMPLE_DIS_GME_TYPE_4DOF = 0,
    SAMPLE_DIS_GME_TYPE_6DOF,
    SAMPLE_DIS_GME_TYPE_BUTT
} dis_gme_type;

ot_vi_pipe g_vi_pipe = 0;
ot_vi_pipe g_send_vi_pipe = 3;

ot_vi_chn  g_vi_chn = 0;

ot_vpss_grp g_vpss_grp = 0;
ot_vpss_chn g_vpss_chn = 0;

ot_vo_chn g_vo_chn = 0;
ot_venc_chn g_venc_chn = 0;
static td_u32 g_dis_sample_signal_flag = 0;
td_bool g_dis_save_stream = TD_TRUE;
td_bool g_dis_send_data = TD_FALSE;

static sample_comm_venc_chn_param g_venc_chn_param = {
    .frame_rate           = 30,
    .stats_time           = 2,
    .gop                  = 60,
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

td_void sample_dis_set_send_data_statue(td_bool send_data)
{
    g_dis_send_data = send_data;
}

td_bool sample_dis_get_send_data_statue(td_void)
{
    return g_dis_send_data;
}

td_bool sample_dis_get_save_stream(void)
{
    return g_dis_save_stream;
}

td_void sample_dis_set_save_stream(td_bool save_stream)
{
    g_dis_save_stream = save_stream;
}

td_void sample_dis_set_venc_chn_size(ot_size img_size)
{
    g_venc_chn_param.venc_size = img_size;
}

/* function : show usage */
static td_void sample_dis_usage(td_char *argv_name)
{
    printf("Usage : %s <index> <intf> \n", argv_name);
    printf("index:\n");
    printf("\t 0)DIS-4DOF_GME.VI-VPSS-VO VENC.\n");
    printf("\t 1)DIS-6DOF_GME.VI-VPSS-VO VENC.\n");
#ifdef OT_GYRODIS_SUPPORT
    printf("\t 2)DIS_GYRO(IPC).VI-VPSS-VO VENC.\n");
    printf("\t 3)DIS_GYRO(DV).VI-VPSS-VO VENC.\n");
    printf("\t 4)DIS_GYRO and DIS_LDC SWITCH. VI-VPSS-VO VENC. DIS_LDC+DIS_GYRO -> DIS_LDC -> DIS_LDC+DIS_GYRO.\n");
    printf("\t 5)DIS_GYRO and LDCV2 SWITCH. VI-VPSS-VO VENC. DIS_LDC+DIS_GYRO -> LDCV2 -> DIS_LDC+DIS_GYRO.\n");
    printf("\t 6)Unsupported\n");
    printf("\t 7)DIS_GYRO(TWO PIPE). ONE WITH DIS, ONE WITHOUT DIS.\n");
    printf("\t 8)DIS_GYRO(rotation compensation).\n");
    printf("\t 9)DIS_GYRO dis send data.Usage : %s <index> <width> <height>\n", argv_name);
    printf("\t 10)DIS_QUERY_POINT(DV).VI-VPSS-VO VENC.\n");
#endif
    printf("intf:\n");
    printf("\t 0) vo BT1120 output, default.\n");
    return;
}

/* function : Get param by different sensor */
static td_s32 sample_dis_get_param_by_sensor(sample_sns_type sns_type, ot_dis_cfg *dis_cfg, ot_dis_attr *dis_attr)
{
    td_s32 ret = TD_SUCCESS;

    if (dis_cfg == NULL  || dis_attr == NULL) {
        return TD_FAILURE;
    }

    dis_cfg->frame_rate = 30; /* 30 fps default frame_rate */
    dis_attr->timelag  = 1000; /* 1000 default timelag */

    return ret;
}

/* function : to process abnormal case */
static void sample_dis_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_dis_sample_signal_flag = 1;
    }
}

static td_s32 sample_dis_get_dis_cfg_and_attr(dis_gme_type gme_type, ot_dis_cfg *dis_cfg, ot_dis_attr *dis_attr)
{
    td_s32 ret = TD_SUCCESS;

    if (dis_cfg == NULL || dis_attr == NULL) {
        return TD_FAILURE;
    }

    dis_cfg->motion_level = OT_DIS_MOTION_LEVEL_NORM;
    dis_cfg->crop_ratio = 80; /* 80 sample crop ratio */
    dis_cfg->buf_num = 10;    /* 10 sample buf num */
    dis_cfg->frame_rate = 30; /* 30 sample frame rate */
    dis_cfg->camera_steady = TD_FALSE;

    if (gme_type == SAMPLE_DIS_GME_TYPE_4DOF) {
        dis_cfg->scale = TD_FALSE;
        dis_cfg->pdt_type = OT_DIS_PDT_TYPE_DV;
        dis_cfg->mode = OT_DIS_MODE_4_DOF_GME;
    } else {
        dis_cfg->scale = TD_TRUE;
        dis_cfg->pdt_type = OT_DIS_PDT_TYPE_RECORDER;
        dis_cfg->mode = OT_DIS_MODE_6_DOF_GME;
    }

    dis_attr->enable = TD_TRUE;
    dis_attr->moving_subject_level = 0;
    dis_attr->rolling_shutter_coef = 0;
    dis_attr->timelag = 0;
    dis_attr->still_crop = TD_FALSE;
    dis_attr->hor_limit = 512; /* 512 sample hor_limit */
    dis_attr->ver_limit = 512; /* 512 sample ver_limit */
    dis_attr->gdc_bypass = TD_FALSE;
    dis_attr->strength = 1024; /* 1024 sample strength */
    return ret;
}

static td_s32 sample_dis_set_dis_attr(const ot_dis_attr *dis_attr)
{
    td_s32 ret;
    ot_vpss_chn_attr vpss_chn_attr = { 0 };
    ot_size dis_out_size = { 0 };

    ret = ss_mpi_vi_set_chn_dis_attr(g_vi_pipe, g_vi_chn, dis_attr);
    if (ret != TD_SUCCESS) {
        sample_print("set dis attr failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    ret = ss_mpi_vpss_get_chn_attr(g_vpss_grp, g_vpss_chn, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("get vpss chn attr failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    ret = ss_mpi_vi_get_chn_dis_out_size(g_vi_pipe, g_vi_chn, &dis_out_size);
    if (ret != TD_SUCCESS) {
        sample_print("get dis out size failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    vpss_chn_attr.width = dis_out_size.width;
    vpss_chn_attr.height = dis_out_size.height;
    ret = ss_mpi_vpss_set_chn_attr(g_vpss_grp, g_vpss_chn, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("set vpss chn attr failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_dis_gme_enable(sample_sns_type sns_type, dis_gme_type gme_type)
{
    td_s32 ret;
    ot_dis_cfg dis_cfg = { 0 };
    ot_dis_attr dis_attr = { 0 };

    ret = sample_dis_get_dis_cfg_and_attr(gme_type, &dis_cfg, &dis_attr);
    if (ret != TD_SUCCESS) {
        sample_print("sample dis get dis_cfg and dis_attr failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    ret = sample_dis_get_param_by_sensor(sns_type, &dis_cfg, &dis_attr);
    if (ret != TD_SUCCESS) {
        sample_print("sample_dis_get_param_by_sensor failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    ret = ss_mpi_vi_set_chn_dis_cfg(g_vi_pipe, g_vi_chn, &dis_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("set dis config failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    ret = sample_dis_set_dis_attr(&dis_attr);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_void sample_dis_pause(td_char *s)
{
    if (g_dis_sample_signal_flag == 1) {
        return;
    }

    printf("please hit the Enter key to %s!\n", s);
    (td_void)getchar();
}

static td_s32 sample_dis_gme_change()
{
    td_s32 ret;
    ot_dis_attr dis_attr = {0};

    sample_dis_pause("Disable DIS");

    ret = ss_mpi_vi_get_chn_dis_attr(g_vi_pipe, g_vi_chn, &dis_attr);
    if (ret != TD_SUCCESS) {
        sample_print("get dis attr failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }

    dis_attr.enable = TD_FALSE;
    ret = sample_dis_set_dis_attr(&dis_attr);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_dis_pause("enable DIS");

    dis_attr.enable = TD_TRUE;
    ret = sample_dis_set_dis_attr(&dis_attr);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    sample_dis_pause("exit");
    return TD_SUCCESS;
}

static td_s32 sample_dis_init_sys_vb(const ot_size *img_size)
{
    ot_vb_cfg vb_cfg = {0};
    ot_pic_buf_attr buf_attr = {0};
    td_u32 blk_size;
    td_s32 ret;
    td_bool save_stream = sample_dis_get_save_stream();

    buf_attr.width = img_size->width;
    buf_attr.height = img_size->height;
    buf_attr.align = 0;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_SEG;

    blk_size = ot_common_get_pic_buf_size(&buf_attr);
    vb_cfg.max_pool_cnt = 128; /* 128 max pool cnt */
    vb_cfg.common_pool[0].blk_size = blk_size;
    vb_cfg.common_pool[0].blk_cnt = 20; /* 20 normal blk cnt */

    vb_cfg.common_pool[1].blk_size = blk_size;
    vb_cfg.common_pool[1].blk_cnt = 4; /* 4 bayer 16bpp blk cnt */

    if (save_stream == TD_FALSE) {
        vb_cfg.common_pool[0].blk_cnt = 20; /* 20 normal blk cnt */
        vb_cfg.common_pool[1].blk_cnt = 12; /* 12 bayer 16bpp blk cnt */
    }

    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg,
        OT_VB_SUPPLEMENT_BNR_MOT_MASK | OT_VB_SUPPLEMENT_MOTION_DATA_MASK);
    if (ret != TD_SUCCESS) {
        sample_print("init sys fail.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void sample_dis_reset_vpss_cfg(ot_vpss_grp vpss_grp)
{
    ot_vpss_grp_cfg grp_cfg;
    td_s32 ret;

    ret = ss_mpi_vpss_get_grp_cfg(vpss_grp, &grp_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("get vpss grp cfg failed. ret: 0x%x !\n", ret);
        return;
    }
    grp_cfg.is_dis_gyro_support = TD_FALSE;
    ret = ss_mpi_vpss_set_grp_cfg(vpss_grp, &grp_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("set vpss grp cfg failed. ret: 0x%x !\n", ret);
    }
}

static td_void sample_dis_vpss_stop(ot_vpss_grp grp, const td_bool *chn_enable, td_u32 chn_array_size)
{
    sample_common_vpss_stop(grp, chn_enable, chn_array_size);
    sample_dis_reset_vpss_cfg(grp);
}

static td_s32 sample_dis_start_vpss(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, const ot_size *img_size)
{
    ot_vpss_grp_attr vpss_grp_attr = {0};
    sample_vpss_chn_attr vpss_chn_attr = {0};
    ot_vpss_grp_cfg grp_cfg;
    td_s32 ret;

    ss_mpi_vpss_get_grp_cfg(vpss_grp, &grp_cfg);
    grp_cfg.is_dis_gyro_support = TD_TRUE;
    ss_mpi_vpss_set_grp_cfg(vpss_grp, &grp_cfg);

    vpss_grp_attr.max_width = img_size->width;
    vpss_grp_attr.max_height = img_size->height;
    vpss_grp_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vpss_grp_attr.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    vpss_grp_attr.frame_rate.src_frame_rate = -1;
    vpss_grp_attr.frame_rate.dst_frame_rate = -1;

    vpss_chn_attr.chn_enable[0] = TD_TRUE;
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    vpss_chn_attr.chn_attr[0].width = img_size->width;
    vpss_chn_attr.chn_attr[0].height = img_size->height;
    vpss_chn_attr.chn_attr[0].chn_mode = OT_VPSS_CHN_MODE_USER;
    vpss_chn_attr.chn_attr[0].compress_mode = OT_COMPRESS_MODE_NONE;
    vpss_chn_attr.chn_attr[0].dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    vpss_chn_attr.chn_attr[0].pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vpss_chn_attr.chn_attr[0].video_format = OT_VIDEO_FORMAT_LINEAR;
    vpss_chn_attr.chn_attr[0].frame_rate.src_frame_rate = -1;
    vpss_chn_attr.chn_attr[0].frame_rate.dst_frame_rate = -1;
    vpss_chn_attr.chn_attr[0].depth = 1;
    vpss_chn_attr.chn_attr[0].mirror_en = TD_FALSE;
    vpss_chn_attr.chn_attr[0].flip_en = TD_FALSE;
    vpss_chn_attr.chn_attr[0].aspect_ratio.mode = OT_ASPECT_RATIO_NONE;

    ret = sample_common_vpss_start(vpss_grp, &vpss_grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed. ret: 0x%x !\n", ret);
        return TD_FAILURE;
    }

    ot_gdc_param gdc_param = {0};
    gdc_param.in_size.width  = img_size->width;
    gdc_param.in_size.height = img_size->height;
    gdc_param.cell_size = OT_LUT_CELL_SIZE_16;
    if (ss_mpi_vpss_set_grp_gdc_param(vpss_grp, &gdc_param) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_dis_start_vi(const sample_vi_cfg *vi_cfg)
{
    ot_vi_vpss_mode_type mode_type = OT_VI_OFFLINE_VPSS_OFFLINE;
    ot_isp_nr_attr isp_nr_attr = {0};
    td_s32 ret;

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, OT_VI_AIISP_MODE_DEFAULT);
    if (ret != TD_SUCCESS) {
        sample_print("set vi failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }

    ret = sample_comm_vi_start_vi(vi_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vi failed.ret:0x%x !\n", ret);
        return TD_FAILURE;
    }
    if (g_dis_send_data == TD_FALSE) {
        ret = ss_mpi_isp_get_nr_attr(g_vi_pipe, &isp_nr_attr);
        if (ret != TD_SUCCESS) {
            sample_print("get nr attr failed.ret:0x%x !\n", ret);
            return TD_FAILURE;
        }

        isp_nr_attr.enable = TD_FALSE;
        ret = ss_mpi_isp_set_nr_attr(g_vi_pipe, &isp_nr_attr);
        if (ret != TD_SUCCESS) {
            sample_print("set nr attr failed.ret:0x%x !\n", ret);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_s32 sample_dis_start_venc(td_void)
{
    td_s32 ret;

    ret = sample_comm_venc_start(g_venc_chn, &g_venc_chn_param);
    if (ret != TD_SUCCESS) {
        sample_print("start venc failed. ret: 0x%x !\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_dis_bind(const sample_vo_cfg *vo_cfg)
{
    td_s32 ret;
    td_bool save_stream = sample_dis_get_save_stream();

    ret = sample_comm_vpss_bind_vo(g_vpss_grp, g_vpss_chn, vo_cfg->vo_dev, g_vo_chn);
    if (ret != TD_SUCCESS) {
        sample_print("vo bind vpss failed. ret: 0x%x !\n", ret);
        return TD_FAILURE;
    }
    if (g_dis_send_data == TD_TRUE) {
        ret = sample_comm_vi_bind_vpss(g_send_vi_pipe, g_vi_chn, g_vpss_grp, g_vpss_chn);
    } else {
        ret = sample_comm_vi_bind_vpss(g_vi_pipe, g_vi_chn, g_vpss_grp, g_vpss_chn);
    }
    if (ret != TD_SUCCESS) {
        sample_print("vi bind vpss failed. ret: 0x%x !\n", ret);
        return TD_FAILURE;
    }

    if (save_stream == TD_TRUE) {
        ret = sample_comm_vpss_bind_venc(g_vpss_grp, g_vpss_chn, g_venc_chn);
        if (ret != TD_SUCCESS) {
            sample_print("vpss bind venc failed. ret: 0x%x !\n", ret);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_void sample_dis_unbind(const sample_vo_cfg *vo_cfg)
{
    td_bool save_stream = sample_dis_get_save_stream();
    if (save_stream == TD_TRUE) {
        sample_comm_vpss_un_bind_venc(g_vpss_grp, g_vpss_chn, g_venc_chn);
    }
    if (g_dis_send_data == TD_TRUE) {
        sample_comm_vi_un_bind_vpss(g_send_vi_pipe, g_vi_chn, g_vpss_grp, g_vpss_chn);
    } else {
        sample_comm_vi_un_bind_vpss(g_vi_pipe, g_vi_chn, g_vpss_grp, g_vpss_chn);
    }
    sample_comm_vpss_un_bind_vo(g_vpss_grp, g_vpss_chn, vo_cfg->vo_dev, g_vo_chn);
}

/* define SAMPLE_MEM_SHARE_ENABLE, when use tools to dump YUV/RAW. */
#ifdef SAMPLE_MEM_SHARE_ENABLE
td_void sample_dis_init_mem_share(td_void)
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

td_s32 sample_dis_start_sample(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg, ot_size *img_size)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {TD_TRUE};
    td_bool save_stream = sample_dis_get_save_stream();

    /* step 1: init SYS and common VB */
    if (sample_dis_init_sys_vb(img_size) != TD_SUCCESS) {
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_dis_init_mem_share();
#endif

    /* step 2: set vpss pos */
    if (ss_mpi_sys_set_3dnr_pos(OT_3DNR_POS_VPSS) != TD_SUCCESS) {
        goto sys_exit;
    }

    /* step 3: start VI */
    if (sample_dis_start_vi(vi_cfg) != TD_SUCCESS) {
        goto sys_exit;
    }

    /* step 4:  start VPSS */
    if (sample_dis_start_vpss(g_vpss_grp, g_vpss_chn, img_size) != TD_SUCCESS) {
        goto vi_stop;
    }

    /* step 5:  start VO */
    if (sample_comm_vo_start_vo(vo_cfg) != TD_SUCCESS) {
        goto vpss_stop;
    }

    if (save_stream == TD_TRUE) {
        /* step 6:  start VENC */
        if (sample_dis_start_venc() != TD_SUCCESS) {
            goto vo_stop;
        }
    }

    /* step 7:  start bind */
    if (sample_dis_bind(vo_cfg) != TD_SUCCESS) {
        goto venc_stop;
    }

    if (save_stream == TD_TRUE) {
        /* step 8: stream VENC process -- get stream, then save it to file. */
        if (sample_comm_venc_start_get_stream(&g_venc_chn, 1) != TD_SUCCESS) {
            sample_print("venc start get stream failed!\n");
            goto unbind;
        }
    }
    return TD_SUCCESS;

unbind:
    sample_dis_unbind(vo_cfg);
venc_stop:
    if (save_stream == TD_TRUE) {
        sample_comm_venc_stop(g_venc_chn);
    }
vo_stop:
    sample_comm_vo_stop_vo(vo_cfg);
vpss_stop:
    sample_dis_vpss_stop(g_vpss_grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
vi_stop:
    sample_comm_vi_stop_vi(vi_cfg);
sys_exit:
    sample_comm_sys_exit();
    return TD_FAILURE;
}

td_void sample_dis_stop_sample_without_sys_exit(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg)
{
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {0};
    td_bool save_stream = sample_dis_get_save_stream();

    chn_enable[0] = TD_TRUE;

    if (save_stream == TD_TRUE) {
        sample_comm_venc_stop_get_stream(1);
    }
    sample_dis_unbind(vo_cfg);
    if (save_stream == TD_TRUE) {
        sample_comm_venc_stop(g_venc_chn);
    }
    sample_comm_vo_stop_vo(vo_cfg);
    sample_dis_vpss_stop(g_vpss_grp, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
    sample_comm_vi_stop_vi(vi_cfg);
}

td_void sample_dis_stop_sample(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg)
{
    sample_dis_stop_sample_without_sys_exit(vi_cfg, vo_cfg);
    sample_comm_sys_exit();
}

static td_s32 sample_dis_gme(ot_vo_intf_type vo_intf_type, dis_gme_type gme_type)
{
    td_s32 ret;
    ot_size img_size;
    sample_vi_cfg vi_cfg;
    sample_vo_cfg vo_cfg = {0};
    sample_dis_set_save_stream(TD_TRUE);

    if (gme_type != SAMPLE_DIS_GME_TYPE_4DOF && gme_type != SAMPLE_DIS_GME_TYPE_6DOF) {
        sample_print("wrong gme_type %d!\n", gme_type);
        return TD_FAILURE;
    }

    /* step 1:  get sensors information and vo config */
    sample_comm_vi_get_default_vi_cfg(SENSOR0_TYPE, &vi_cfg);
    sample_comm_vo_get_def_config(&vo_cfg);
    vo_cfg.vo_intf_type = vo_intf_type;

    /* step 2:  get input size */
    sample_comm_vi_get_size_by_sns_type(vi_cfg.sns_info.sns_type, &img_size);
    if (gme_type == SAMPLE_DIS_GME_TYPE_6DOF) {
        sample_dis_set_venc_chn_size(img_size);
    }

    /* step 3: start VI-VPSS-VO-VENC */
    vi_cfg.pipe_info[g_vi_pipe].nr_attr.enable = TD_FALSE;
    vi_cfg.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_attr.video_format = OT_VIDEO_FORMAT_TILE_32x4;
    vi_cfg.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_attr.compress_mode = OT_COMPRESS_MODE_TILE;
    ret = sample_dis_start_sample(&vi_cfg, &vo_cfg, &img_size);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    /* step 4: set DIS config & attribute */
    if (sample_dis_gme_enable(vi_cfg.sns_info.sns_type, gme_type) != TD_SUCCESS) {
        goto exit;
    }

    /* step 5: dis enable disable */
    if (sample_dis_gme_change() != TD_SUCCESS) {
        goto exit;
    }

exit:
    /* exit process */
    sample_dis_stop_sample(&vi_cfg, &vo_cfg);
    return TD_SUCCESS;
}

static td_s32 sample_dis_proc(td_char *argv_name, td_u32 case_index, ot_vo_intf_type vo_intf_type,
    ot_size* input_size)
{
    td_s32 ret = TD_FAILURE;
    switch (case_index) {
        case 0: /* 0 index */
            ret = sample_dis_gme(vo_intf_type, SAMPLE_DIS_GME_TYPE_4DOF);
            break;
        case 1: /* 1 index */
            ret = sample_dis_gme(vo_intf_type, SAMPLE_DIS_GME_TYPE_6DOF);
            break;

#ifdef OT_GYRODIS_SUPPORT
        case 2: /* 2 index */
            ret = sample_dis_ipc_gyro(vo_intf_type);
            break;
        case 3: /* 3 index */
            ret = sample_dis_dv_gyro(vo_intf_type);
            break;
        case 4: /* 4 index */
            ret = sample_dis_gyro_ldc_switch(vo_intf_type);
            break;
        case 5: /* 5 index */
            ret = sample_dis_gyro_ldcv2_switch(vo_intf_type);
            break;
        case 6: /* 6 index */
            break;
        case 7: /* 7 index */
            ret = sample_dis_gyro_demo(vo_intf_type);
            break;
        case 8: /* 8 index */
            ret = sample_dis_gyro_rotation_compensation(vo_intf_type);
            break;
        case 9: /* 9 index */
            ret = sample_dis_send(vo_intf_type, input_size);
            break;
        case 10: /* 10 index */
            ret = sample_dis_query_center_point(vo_intf_type);
            break;
#endif
        default:
            sample_print("the index is invalid!\n");
            sample_dis_usage(argv_name);
            return TD_FAILURE;
    }
    return ret;
}

#ifdef __LITEOS__
int app_main(td_s32 argc, td_char *argv[])
#else
int main(td_s32 argc, td_char *argv[])
#endif
{
    td_s32 ret;
    ot_size input_size = {3840, 2160};
    ot_vo_intf_type vo_intf_type = OT_VO_INTF_BT1120;
    td_u32 index;
    td_char *end_ptr = TD_NULL;
    if ((argc < 2) || (argc > 4) || (strlen(argv[1]) > 2) || /* 2 4 argv num */
        (strlen(argv[1]) == 2 && (!check_digit(argv[1][1]) || argv[1][0] == '0'))) { /* 2:arg len */
        sample_dis_usage(argv[0]);
        return TD_FAILURE;
    }
    g_dis_sample_signal_flag = 0;
#ifndef __LITEOS__
    sample_sys_signal(&sample_dis_handle_sig);
#endif
    index = (td_u32)strtol(argv[1], &end_ptr, 10); /* arg index, 10 : decimal */
    if (argc == 4) { /* 4 : argv num */
        input_size.width = (td_u32)strtol(argv[2], &end_ptr, 10); /* 2 : arg index, 10 : decimal */
        input_size.height = (td_u32)strtol(argv[3], &end_ptr, 10); /* 3 : arg index, 10 : decimal */
    }
    if (argc == 3) { /* 3 argv num */
        if ((strlen(argv[2]) != 1)) { /* 2 intf */
            sample_dis_usage(argv[0]);
            return TD_FAILURE;
        }
        switch (*argv[2]) { /* 2 intf */
            case '0':
                break;
            case '1':
                vo_intf_type = OT_VO_INTF_BT1120;
                break;
            default:
                sample_print("the index is invalid!\n");
                sample_dis_usage(argv[0]);
                return TD_FAILURE;
        }
    }
    ret = sample_dis_proc(argv[0], index, vo_intf_type, &input_size);
    if (g_dis_sample_signal_flag == 1) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        exit(-1);
    }
    if (ret == TD_SUCCESS) {
        sample_print("program exit normally!\n");
    } else {
        sample_print("program exit abnormally!\n");
    }
    return ret;
}
