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
#include <sys/prctl.h>

#include "sample_comm.h"

#define MM16_BMP "./res/mm16.bmp"
#define MM_BMP "./res/mm.bmp"
#define STREAM_PATH "res"
#define STREAM_H265_1080P "1080P.h265"
#define STREAM_H265_4K "3840x2160_8bit.h265"

#define VB_NUM_PRE_CHN 8
#define RGN_VI_CHN   0
#define RGN_VI_PIPE  0
#define RGN_VPSS_GRP 0
#define RGN_VPSS_CHN 0
#define RGN_VENC_CHN 0

#define RGN_HANDLE_NUM_1 1
#define RGN_HANDLE_NUM_4 4
#define RGN_HANDLE_NUM_8 8

#define RGN_VO_CHN 0
#define RGN_VDEC_CHN 0
#define RGN_VI_CHN_NUM 1
#define RGN_VDEC_CHN_NUM 1

#define REGION_STOP_GET_VENC_STREAM (0x01L << 0)
#define REGION_UNBIND_VPSS_VENC     (0x01L << 1)
#define REGION_STOP_VENC            (0x01L << 2)
#define REGION_UNBIND_VPSS_VO       (0x01L << 3)
#define REGION_STOP_VO              (0x01L << 4)
#define REGION_UNBIND_VDEC_VPSS     (0x01L << 5)
#define REGION_STOP_SEND_STREAM     (0x01L << 6)
#define REGION_STOP_VPSS            (0x01L << 7)
#define REGION_STOP_VDEC            (0x01L << 8)
#define REGION_UNBIND_VI_VPSS       (0x01L << 9)
#define REGION_STOP_VI              (0x01L << 10)
typedef td_u32 region_stop_flag;


static td_char *g_path_bmp;
static pthread_t g_vdec_thread;
static ot_vo_intf_sync g_rgn_intf_sync = OT_VO_OUT_1080P60;
static int g_rgn_sample_exit = 0;

static sample_vdec_attr g_rgn_vdec_cfg = {
    .type = OT_PT_H265,
    .mode = OT_VDEC_SEND_MODE_FRAME,
    .width = _4K_WIDTH,
    .height = _4K_HEIGHT,
    .sample_vdec_video.dec_mode = OT_VIDEO_DEC_MODE_IP,
    .sample_vdec_video.bit_width = OT_DATA_BIT_WIDTH_8,
    .sample_vdec_video.ref_frame_num = 2, /* 2:ref_frame_num */
    .display_frame_num = 2,               /* 2:display_frame_num */
    .frame_buf_cnt = 5,                   /* 5:2+2+1 */
};

static sampe_sys_cfg g_vio_sys_cfg = {
    .route_num = 1,
    .mode_type = OT_VI_OFFLINE_VPSS_OFFLINE,
    .nr_pos = OT_3DNR_POS_VI,
    .vi_fmu = {0},
    .vpss_fmu = {0},
};

static sample_vi_cfg g_vi_cfg = { 0 };

static vdec_thread_param g_rgn_vdec_thread_param = {
    .chn_id = 0,
    .type = OT_PT_H265,
    .stream_mode = OT_VDEC_SEND_MODE_FRAME,
    .interval_time = 1000, /* 1000000:interval_time */
    .pts_init = 0,
    .pts_increase = 0,
    .e_thread_ctrl = THREAD_CTRL_START,
    .circle_send = TD_TRUE,
    .milli_sec = 0,
    .min_buf_size = (_4K_WIDTH * _4K_HEIGHT * 3) >> 1, /* 3:buff_size */
    .c_file_path = "res",
    .c_file_name = STREAM_H265_4K,
    .fps = 60, /* 60:default fps */
};

static sample_vo_cfg g_rgn_vo_cfg = {
    .vo_dev = SAMPLE_VO_DEV_DHD0,
    .vo_layer = SAMPLE_VO_LAYER_VHD0,
    .vo_intf_type = OT_VO_INTF_BT1120,
    .intf_sync = OT_VO_OUT_1080P60,
    .bg_color = COLOR_RGB_BLUE,
    .vo_mode = VO_MODE_1MUX,
    .vo_part_mode = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len = 3, /* 3:buf len */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .compress_mode = OT_COMPRESS_MODE_NONE,
};

static sample_vb_param g_vb_param = {
    .vb_size = {3840, 2160},
    .pixel_format = {OT_PIXEL_FORMAT_RGB_BAYER_12BPP, OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420},
    .compress_mode = {OT_COMPRESS_MODE_LINE, OT_COMPRESS_MODE_SEG},
    .video_format = {OT_VIDEO_FORMAT_LINEAR, OT_VIDEO_FORMAT_LINEAR},
    .blk_num = {20, 20}
};

/*
 * function: to process abnormal case
 */
static td_void sample_region_handle_sig(td_s32 signo)
{
    if ((signo == SIGINT) || (signo == SIGTERM)) {
        g_rgn_sample_exit = 1;
    }
}

static td_void rgn_sample_pause(td_void)
{
    if (g_rgn_sample_exit == 1) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        sample_comm_sys_exit();
        exit(-1);
    }

    sample_pause();

    if (g_rgn_sample_exit == 1) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        sample_comm_sys_exit();
        exit(-1);
    }
}

static td_s32 sample_region_start_venc(td_void)
{
    td_s32 ret;
    ot_venc_gop_mode gop_mode;
    ot_venc_gop_attr gop_attr;
    sample_comm_venc_chn_param venc_create_param;
    ot_venc_start_param start_param;

    gop_mode = OT_VENC_GOP_MODE_NORMAL_P;

    (td_void)memset_s(&venc_create_param, sizeof(sample_comm_venc_chn_param), 0, sizeof(sample_comm_venc_chn_param));
    ret = sample_comm_venc_get_gop_attr(gop_mode, &gop_attr);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_venc_get_gop_attr failed!\n");
        return ret;
    }
    venc_create_param.type = OT_PT_H265;
    venc_create_param.gop = 60; /* 60:default gop val */
    venc_create_param.frame_rate = 30; /* 30:is a number */
    venc_create_param.stats_time = 2; /* 2:is a number */
    venc_create_param.rc_mode = SAMPLE_RC_CBR;
    venc_create_param.venc_size.height = _4K_HEIGHT;
    venc_create_param.venc_size.width = _4K_WIDTH;
    venc_create_param.size = PIC_3840X2160;

    (td_void)memcpy_s(&venc_create_param.gop_attr, sizeof(ot_venc_gop_attr), &gop_attr, sizeof(ot_venc_gop_attr));

    /* step 1:  creat encode channel */
    ret = sample_comm_venc_create(RGN_VENC_CHN, &venc_create_param);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_venc_create failed with%#x! \n", ret);
        return TD_FAILURE;
    }
    /* step 2:  start recv venc pictures */
    start_param.recv_pic_num = -1;
    ret = ss_mpi_venc_start_chn(RGN_VENC_CHN, &start_param);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_venc_start_recv_pic failed with%#x! \n", ret);
        ret = ss_mpi_venc_destroy_chn(RGN_VENC_CHN);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_venc_destroy_chn vechn[%d] failed with %#x!\n", RGN_VENC_CHN, ret);
        }
    }
    return ret;
}

static td_s32 sample_region_stop_venc(td_void)
{
    return sample_comm_venc_stop(RGN_VENC_CHN);
}

static td_s32 sample_region_start_get_venc_stream(td_void)
{
    td_s32 venc_chn[2] = {0, 1}; /* 2:VENC chn max num */

    return sample_comm_venc_start_get_stream(venc_chn, 1);
}

static td_void sample_region_stop_get_venc_stream(td_void)
{
    sample_comm_venc_stop_get_stream(1);
}

static td_s32 sample_region_start_vdec(td_u32 chn_num, sample_vdec_attr *vdec_attr)
{
    td_s32 ret;
    ot_pic_buf_attr buf_attr = { 0 };
    ot_vb_cfg vb_cfg;
    ot_size disp_size;
    ot_pic_size disp_pic_size = PIC_3840X2160;

    ret = sample_comm_sys_get_pic_size(disp_pic_size, &disp_size);
    if (ret != TD_SUCCESS) {
        sample_print("sys get pic size fail for %#x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }
    buf_attr.align = 0;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.height = disp_size.height;
    buf_attr.width = disp_size.width;
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    (td_void)memset_s(&vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg.max_pool_cnt = 1;                               /* 1:vb pool cnt */
    vb_cfg.common_pool[0].blk_cnt = 10 * RGN_VDEC_CHN_NUM; /* 10:vb blk cnt */
    vb_cfg.common_pool[0].blk_size = ot_common_get_pic_buf_size(&buf_attr);
    ret = sample_comm_sys_vb_init(&vb_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("init sys fail for %#x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }
    ret = sample_comm_vdec_init_vb_pool(chn_num, vdec_attr, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("vdec init vb_pool fail\n");
        sample_comm_sys_exit();
        return ret;
    }
    ret = sample_comm_vdec_start(chn_num, vdec_attr, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("vdec start fail\n");
        sample_comm_vdec_exit_vb_pool();
        sample_comm_sys_exit();
    }

    return ret;
}

static td_s32 sample_region_start_vi(td_u32 chn_num)
{
    td_s32 i;
    td_s32 ret = TD_SUCCESS;
    ot_vb_cfg vb_cfg;
    sample_sns_type sns_type = SENSOR0_TYPE;
    td_u32 supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK;

    sample_comm_sys_get_default_vb_cfg(&g_vb_param, &vb_cfg);
    if (sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config) != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (sample_comm_vi_set_vi_vpss_mode(g_vio_sys_cfg.mode_type, OT_VI_AIISP_MODE_DEFAULT) != TD_SUCCESS) {
        sample_comm_sys_exit();
        return ret;
    }

    if (ss_mpi_sys_set_3dnr_pos(g_vio_sys_cfg.nr_pos) != TD_SUCCESS) {
        sample_comm_sys_exit();
        return ret;
    }

    sample_comm_vi_get_vi_cfg_by_fmu_mode(sns_type, g_vio_sys_cfg.vi_fmu[0], &g_vi_cfg);

    for (i = 0; i < g_vio_sys_cfg.route_num; i++) {
        ret = sample_comm_vi_start_vi(&g_vi_cfg);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    return ret;
}


static td_s32 sample_region_stop_vdec(td_u32 chn_num)
{
    td_s32 ret;
    ret = sample_comm_vdec_stop(chn_num);
    if (ret != TD_SUCCESS) {
        printf("vdec stop fail\n");
        return TD_FAILURE;
    }
    sample_comm_vdec_exit_vb_pool();
    return TD_SUCCESS;
}

static td_s32 sample_region_common_start_vpss(td_s32 vpss_grp, td_s32 vpss_chn, const ot_size *size)
{
    td_s32 ret;
    ot_vpss_grp_attr grp_attr;
    sample_vpss_chn_attr vpss_chn_attr = {0};

    sample_comm_vpss_get_default_grp_attr(&grp_attr);
    grp_attr.max_width = size->width;
    grp_attr.max_height = size->height;
    sample_comm_vpss_get_default_chn_attr(&vpss_chn_attr.chn_attr[vpss_chn]);
    vpss_chn_attr.chn_attr[vpss_chn].width = size->width;
    vpss_chn_attr.chn_attr[vpss_chn].height = size->height;
    vpss_chn_attr.chn_attr[vpss_chn].compress_mode = OT_COMPRESS_MODE_NONE;
    vpss_chn_attr.chn_enable[vpss_chn] = TD_TRUE;
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    ret = sample_common_vpss_start(vpss_grp, &grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("failed with %#x!\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_region_stop_vpss()
{
    td_s32 ret;
    td_bool chn_enable[OT_VPSS_MAX_PHYS_CHN_NUM] = {0};

    ret = sample_common_vpss_stop(RGN_VPSS_GRP, chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("failed with %#x!\n", ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_region_start_vpss(td_void)
{
    td_s32 ret;
    ot_size size;

    size.width = _4K_WIDTH;
    size.height = _4K_HEIGHT;
    ret = sample_region_common_start_vpss(RGN_VPSS_GRP, RGN_VPSS_CHN, &size);
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed with 0x%x!\n", ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_region_start_send_stream(td_void)
{
    sample_comm_vdec_start_send_stream(RGN_VDEC_CHN_NUM, &g_rgn_vdec_thread_param, &g_vdec_thread, OT_VDEC_MAX_CHN_NUM,
        2 * OT_VDEC_MAX_CHN_NUM); /* 2:thread num */
}

static td_void sample_region_stop_send_stream(td_void)
{
    sample_comm_vdec_stop_send_stream(RGN_VDEC_CHN_NUM, &g_rgn_vdec_thread_param, &g_vdec_thread, OT_VDEC_MAX_CHN_NUM,
        2 * OT_VDEC_MAX_CHN_NUM); /* 2:thread num */
    sleep(1);
}

static td_void sample_region_do_stop_vi(region_stop_flag flag)
{
    if (flag & REGION_STOP_VI) {
        sample_comm_vi_stop_vi(&g_vi_cfg);
    }

    if (flag & REGION_UNBIND_VI_VPSS) {
        sample_comm_vi_un_bind_vpss(RGN_VI_PIPE, RGN_VI_CHN, RGN_VPSS_GRP, RGN_VPSS_CHN);
    }
}

static td_s32 sample_region_do_stop(region_stop_flag flag)
{
    td_s32 ret = TD_SUCCESS;

    if (flag & REGION_STOP_GET_VENC_STREAM) {
        sample_region_stop_get_venc_stream();
    }

    if (flag & REGION_UNBIND_VPSS_VENC) {
        ret = sample_comm_vpss_un_bind_venc(RGN_VPSS_GRP, RGN_VPSS_CHN, RGN_VENC_CHN);
        if (ret != TD_SUCCESS) {
            sample_print("sample_comm_vpss_un_bind_venc failed!\n");
        }
    }

    if (flag & REGION_STOP_VENC) {
        ret = sample_region_stop_venc();
        if (ret != TD_SUCCESS) {
            sample_print("stop venc failed!\n");
        }
    }

    if (flag & REGION_UNBIND_VPSS_VO) {
        ret = sample_comm_vpss_un_bind_vo(RGN_VPSS_GRP, RGN_VPSS_CHN, SAMPLE_VO_LAYER_VHD0, RGN_VO_CHN);
        if (ret != TD_SUCCESS) {
            sample_print("sample_comm_vpss_un_bind_vo failed with 0x%x!\n", ret);
        }
    }

    if (flag & REGION_STOP_VO) {
        sample_comm_vo_stop_vo(&g_rgn_vo_cfg);
    }

    sample_region_do_stop_vi(flag);

    if (flag & REGION_UNBIND_VDEC_VPSS) {
        sample_comm_vdec_un_bind_vpss(RGN_VDEC_CHN, RGN_VPSS_GRP);
    }

    if (flag & REGION_STOP_SEND_STREAM) {
        sample_region_stop_send_stream();
    }

    if (flag & REGION_STOP_VPSS) {
        ret = sample_region_stop_vpss();
        if (ret != TD_SUCCESS) {
            sample_print("stop vpss failed!\n");
        }
    }

    if (flag & REGION_STOP_VDEC) {
        ret = sample_region_stop_vdec(RGN_VDEC_CHN_NUM);
        if (ret != TD_SUCCESS) {
            sample_print("stop vdec failed!\n");
        }
    }

    return ret;
}

static td_s32 sample_region_mpp_vdec_vpss_venc_start(td_void)
{
    td_s32 ret;

    ret = sample_region_start_vdec(RGN_VDEC_CHN_NUM, &g_rgn_vdec_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vdec failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_start_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VDEC);
    }
    ret = sample_comm_vdec_bind_vpss(RGN_VDEC_CHN, RGN_VPSS_GRP);
    if (ret != TD_SUCCESS) {
        sample_print("vdec_bind_multi_vpss 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VDEC | REGION_STOP_VPSS);
    }
    sample_region_start_send_stream();
    ret = sample_region_start_venc();
    if (ret != TD_SUCCESS) {
        sample_print("start venc failed!\n");
        return sample_region_do_stop(REGION_STOP_VDEC | REGION_STOP_VPSS | REGION_UNBIND_VDEC_VPSS);
    }
    ret = sample_comm_vpss_bind_venc(RGN_VPSS_GRP, RGN_VPSS_CHN, RGN_VENC_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("bind vpss venc failed!\n");
        return sample_region_do_stop(REGION_STOP_VDEC | REGION_STOP_VPSS | REGION_UNBIND_VDEC_VPSS | REGION_STOP_VENC);
    }
    ret = sample_region_start_get_venc_stream();
    if (ret != TD_SUCCESS) {
        sample_print("start get venc stream failed!\n");
        g_rgn_sample_exit = 1;
        return sample_region_do_stop(REGION_STOP_VDEC | REGION_STOP_VPSS | REGION_UNBIND_VDEC_VPSS
            | REGION_STOP_VENC | REGION_UNBIND_VPSS_VENC);
    }
    return ret;
}

static td_s32 sample_region_mpp_vdec_vpss_venc_end(td_void)
{
    td_s32 ret;

    sample_region_stop_get_venc_stream();
    ret = sample_comm_vpss_un_bind_venc(RGN_VPSS_GRP, RGN_VPSS_CHN, RGN_VENC_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vpss_un_bind_venc failed!\n");
        return ret;
    }
    ret = sample_region_stop_venc();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_venc failed!\n");
        return ret;
    }
    sample_region_stop_send_stream();
    ret = sample_region_stop_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_vpss failed!\n");
        return ret;
    }
    ret = sample_region_stop_vdec(RGN_VDEC_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_vdec failed!\n");
        return ret;
    }
    sample_comm_sys_exit();

    return TD_SUCCESS;
}

static td_s32 sample_region_do_destroy(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn, region_op_flag flag)
{
    td_s32 ret = TD_SUCCESS;

    if (flag & REGION_OP_CHN) {
        ret = sample_comm_region_detach(handle_num, type, chn, flag);
        if (ret != TD_SUCCESS) {
            sample_print("sample_comm_region_detach failed!\n");
        }
    } else if (flag & REGION_OP_DEV) {
        ret = sample_comm_region_detach(handle_num, type, chn, flag);
        if (ret != TD_SUCCESS) {
            sample_print("sample_comm_region_detach failed!\n");
        }
    }

    if (flag & REGION_DESTROY) {
        ret = sample_comm_region_destroy(handle_num, type);
        if (ret != TD_SUCCESS) {
            sample_print("sample_comm_region_destroy failed!\n");
        }
    }

    return ret;
}

static td_s32 sample_region_do_destroy_venc(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn,
    region_op_flag flag)
{
    td_s32 ret;

    (td_void)sample_region_do_destroy(handle_num, type, chn, flag);
    ret = sample_region_mpp_vdec_vpss_venc_end();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_mpp_vdec_vpss_venc_end failed!\n");
    }

    return ret;
}

static td_s32 sample_region_vdec_vpss_venc(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn)
{
    td_s32 i;
    td_s32 ret;
    td_s32 min_handle;

    rgn_check_handle_num_return(handle_num);

    ret = sample_region_mpp_vdec_vpss_venc_start();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    ret = sample_comm_region_create(handle_num, type);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_region_create failed!\n");
        return sample_region_do_destroy_venc(handle_num, type, chn, REGION_DESTROY);
    }
    ret = sample_comm_region_attach(handle_num, type, chn, REGION_OP_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_region_attach failed!\n");
        return sample_region_do_destroy_venc(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
    }
    min_handle = sample_comm_region_get_min_handle(type);
    if (sample_comm_check_min(min_handle) != TD_SUCCESS) {
        sample_print("min_handle(%d) should be in [0, %d).\n", min_handle, OT_RGN_HANDLE_MAX);
        return sample_region_do_destroy_venc(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
    }
    if (type == OT_RGN_OVERLAY || type == OT_RGN_OVERLAYEX) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_set_bit_map(i, g_path_bmp);
            if (ret != TD_SUCCESS) {
                sample_print("sample_comm_region_set_bit_map failed!\n");
                sample_region_do_destroy_venc(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
            }
        }
    }
    rgn_sample_pause();

    return sample_region_do_destroy_venc(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
}

static td_s32 sample_region_vi_vpss_vo_start(td_void)
{
    td_s32 ret;

    ret = sample_region_start_vi(RGN_VDEC_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("start vi failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_start_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VI);
    }
    ret = sample_comm_vi_bind_vpss(RGN_VI_PIPE, RGN_VI_CHN, RGN_VPSS_GRP, RGN_VPSS_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("vi_bind_multi_vpss 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VPSS | REGION_STOP_VI);
    }
    sample_comm_vo_get_def_config(&g_rgn_vo_cfg);
    g_rgn_vo_cfg.intf_sync = g_rgn_intf_sync;
    ret = sample_comm_vo_start_vo(&g_rgn_vo_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vo failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_UNBIND_VI_VPSS | REGION_STOP_VPSS | REGION_STOP_VI);
    }
    ret = sample_comm_vpss_bind_vo(RGN_VPSS_GRP, RGN_VPSS_CHN, SAMPLE_VO_LAYER_VHD0, RGN_VO_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("vpss bind vo failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VO | REGION_UNBIND_VI_VPSS | REGION_STOP_VPSS | REGION_STOP_VI);
    }

    return ret;
}

static td_s32 sample_region_vdec_vpss_vo_start(td_void)
{
    td_s32 ret;

    ret = sample_region_start_vdec(RGN_VDEC_CHN_NUM, &g_rgn_vdec_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vdec failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_start_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VDEC);
    }
    ret = sample_comm_vdec_bind_vpss(RGN_VDEC_CHN, RGN_VPSS_GRP);
    if (ret != TD_SUCCESS) {
        sample_print("vi_bind_multi_vpss 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VPSS | REGION_STOP_VDEC);
    }
    sample_comm_vo_get_def_config(&g_rgn_vo_cfg);
    g_rgn_vo_cfg.intf_sync = g_rgn_intf_sync;
    ret = sample_comm_vo_start_vo(&g_rgn_vo_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vo failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_UNBIND_VDEC_VPSS | REGION_STOP_VPSS | REGION_STOP_VDEC);
    }
    ret = sample_comm_vpss_bind_vo(RGN_VPSS_GRP, RGN_VPSS_CHN, SAMPLE_VO_LAYER_VHD0, RGN_VO_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("vpss bind vo failed with 0x%x!\n", ret);
        return sample_region_do_stop(REGION_STOP_VO | REGION_UNBIND_VDEC_VPSS | REGION_STOP_VPSS | REGION_STOP_VDEC);
    }
    sample_region_start_send_stream();

    return ret;
}

static td_s32 sample_region_vi_vpss_vo_end(td_void)
{
    td_s32 ret;

    ret = sample_comm_vpss_un_bind_vo(RGN_VPSS_GRP, RGN_VPSS_CHN, SAMPLE_VO_LAYER_VHD0, RGN_VO_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vpss_un_bind_vo failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_comm_vo_stop_vo(&g_rgn_vo_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vo_stop_vo failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_comm_vi_un_bind_vpss(RGN_VI_PIPE, RGN_VI_CHN, RGN_VPSS_GRP, RGN_VPSS_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vi_un_bind_vpss failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_stop_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_vpss failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_do_stop(REGION_STOP_VI);
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_do_stop vi failed with 0x%x!\n", ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 sample_region_vdec_vpss_vo_end(td_void)
{
    td_s32 ret;

    ret = sample_comm_vpss_un_bind_vo(RGN_VPSS_GRP, RGN_VPSS_CHN, SAMPLE_VO_LAYER_VHD0, RGN_VO_CHN);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vpss_un_bind_vo failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_comm_vo_stop_vo(&g_rgn_vo_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vo_stop_vo failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_comm_vdec_un_bind_vpss(RGN_VDEC_CHN, RGN_VPSS_GRP);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vdec_un_bind_vpss failed with 0x%x!\n", ret);
        return ret;
    }
    sample_region_stop_send_stream();
    ret = sample_region_stop_vpss();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_vpss failed with 0x%x!\n", ret);
        return ret;
    }
    ret = sample_region_stop_vdec(RGN_VDEC_CHN_NUM);
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_stop_vdec failed with 0x%x!\n", ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 sample_region_do_destroy_vo(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn,
    region_op_flag flag)
{
    td_s32 ret;

    (td_void)sample_region_do_destroy(handle_num, type, chn, flag);
    ret = sample_region_vdec_vpss_vo_end();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_mpp_vdec_vpss_venc_end failed!\n");
    }

    return ret;
}

static td_s32 sample_region_do_destroy_vi_vpss_vo(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn,
    region_op_flag flag)
{
    td_s32 ret;

    (td_void)sample_region_do_destroy(handle_num, type, chn, flag);
    ret = sample_region_vi_vpss_vo_end();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_mpp_vi_vpss_venc_end failed!\n");
    }

    return ret;
}

static td_s32 sample_region_vi_vpss_vo(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn,
    region_op_flag op_flag)
{
    td_s32 i, ret, min_handle;

    rgn_check_handle_num_return(handle_num);

    ret = sample_region_vi_vpss_vo_start();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_mpp_vi_vpss_vo_start failed!\n");
        return ret;
    }
    ret = sample_comm_region_create(handle_num, type);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_region_create failed!\n");
        return sample_region_do_destroy_vo(handle_num, type, chn, REGION_DESTROY);
    }

    ret = sample_comm_region_attach(handle_num, type, chn, op_flag);
    if (ret != TD_SUCCESS) {
        if (op_flag & REGION_OP_CHN) {
            sample_print("sample_comm_region_attach failed!\n");
            return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
        } else if (op_flag & REGION_OP_DEV) {
            sample_print("sample_comm_region_attach_to_dev failed!\n");
            return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_DEV | REGION_DESTROY);
        }
    }

    min_handle = sample_comm_region_get_min_handle(type);
    if (sample_comm_check_min(min_handle) != TD_SUCCESS) {
        sample_print("min_handle(%d) should be in [0, %d).\n", min_handle, OT_RGN_HANDLE_MAX);
        return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
    }
    if (type == OT_RGN_OVERLAY) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_set_bit_map(i, g_path_bmp);
            if (ret != TD_SUCCESS) {
                sample_print("sample_comm_region_set_bit_map failed!\n");
                return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
            }
        }
    } else if (type == OT_RGN_OVERLAYEX) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_get_up_canvas(i, g_path_bmp);
            if (ret != TD_SUCCESS) {
                sample_print("sample_comm_region_get_up_canvas failed!\n");
                return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
            }
        }
    }
    rgn_sample_pause();

    return sample_region_do_destroy_vi_vpss_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
}

static td_s32 sample_region_vdec_vpss_vo(td_s32 handle_num, ot_rgn_type type, ot_mpp_chn *chn,
    region_op_flag op_flag)
{
    td_s32 i, ret, min_handle;

    rgn_check_handle_num_return(handle_num);

    ret = sample_region_vdec_vpss_vo_start();
    if (ret != TD_SUCCESS) {
        sample_print("sample_region_mpp_vi_vpss_vo_start failed!\n");
        return ret;
    }
    ret = sample_comm_region_create(handle_num, type);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_region_create failed!\n");
        return sample_region_do_destroy_vo(handle_num, type, chn, REGION_DESTROY);
    }

    ret = sample_comm_region_attach(handle_num, type, chn, op_flag);
    if (ret != TD_SUCCESS) {
        if (op_flag & REGION_OP_CHN) {
            sample_print("sample_comm_region_attach failed!\n");
            return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
        } else if (op_flag & REGION_OP_DEV) {
            sample_print("sample_comm_region_attach_to_dev failed!\n");
            return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_DEV | REGION_DESTROY);
        }
    }

    min_handle = sample_comm_region_get_min_handle(type);
    if (sample_comm_check_min(min_handle) != TD_SUCCESS) {
        sample_print("min_handle(%d) should be in [0, %d).\n", min_handle, OT_RGN_HANDLE_MAX);
        return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
    }
    if (type == OT_RGN_OVERLAY) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_set_bit_map(i, g_path_bmp);
            if (ret != TD_SUCCESS) {
                sample_print("sample_comm_region_set_bit_map failed!\n");
                return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
            }
        }
    } else if (type == OT_RGN_OVERLAYEX) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_get_up_canvas(i, g_path_bmp);
            if (ret != TD_SUCCESS) {
                sample_print("sample_comm_region_get_up_canvas failed!\n");
                return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
            }
        }
    }
    rgn_sample_pause();

    return sample_region_do_destroy_vo(handle_num, type, chn, REGION_OP_CHN | REGION_DESTROY);
}

static td_s32 sample_region_vpss_overlayex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_OVERLAYEX;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = MM_BMP;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_cover(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_COVER;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_DEV);
}

static td_s32 sample_region_vpss_coverex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_COVEREX;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_lineex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_4;
    type = OT_RGN_LINEEX;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_mosaic(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_4;
    type = OT_RGN_MOSAIC;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_DEV);
}

static td_s32 sample_region_vpss_chn_mosaic(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_4;
    type = OT_RGN_MOSAIC;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_mosaicex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_4;
    type = OT_RGN_MOSAICEX;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_corner_rect(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_CORNER_RECT;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vpss_corner_rectex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_CORNER_RECTEX;
    chn.mod_id = OT_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_venc_overlay(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;
    handle_num = RGN_HANDLE_NUM_8;
    type = OT_RGN_OVERLAY;
    chn.mod_id = OT_ID_VENC;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = MM_BMP;

    return sample_region_vdec_vpss_venc(handle_num, type, &chn);
}

static td_s32 sample_region_vo_corner_rectex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_1;
    type = OT_RGN_CORNER_RECTEX;
    chn.mod_id = OT_ID_VO;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vdec_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

static td_s32 sample_region_vi_coverex(td_void)
{
    td_s32 handle_num;
    ot_rgn_type type;
    ot_mpp_chn chn;

    handle_num = RGN_HANDLE_NUM_1;
    type = OT_RGN_COVEREX;
    chn.mod_id = OT_ID_VI;
    chn.dev_id = 0;
    chn.chn_id = 0;

    return sample_region_vi_vpss_vo(handle_num, type, &chn, REGION_OP_CHN);
}

/*
 * function : show usage
 */
static td_void sample_region_usage(const char *s_prg_nm)
{
    printf("usage : %s <index> \n", s_prg_nm);
    printf("index:\n");
    printf("\t  0)VPSS OVERLAYEX.\n");
    printf("\t  1)VPSS COVER.\n");
    printf("\t  2)VPSS COVEREX.\n");
    printf("\t  3)vpss lineex.\n");
    printf("\t  4)VPSS MOSAIC.\n");
    printf("\t  5)VPSS CHN MOSAIC.\n");
    printf("\t  6)VPSS MOSAICEX.\n");
    printf("\t  7)VPSS CORNER_RECT.\n");
    printf("\t  8)VPSS CORNER_RECTEX.\n");
    printf("\t  9)VENC OVERLAY.\n");
    printf("\t 10)VO CORNER_RECTEX.\n");
    printf("\t 11)VI COVEREX.\n");
}

static td_s32 rgn_get_index_start(td_s32 index)
{
    td_s32 ret;

    switch (index) {
        case 0: /* 0 vpss overlayex */
            ret = sample_region_vpss_overlayex();
            break;
        case 1: /* 1 vpss cover */
            ret = sample_region_vpss_cover();
            break;
        case 2: /* 2 vpss coverex */
            ret = sample_region_vpss_coverex();
            break;
        case 3: /* 3 vpss lineex */
            ret = sample_region_vpss_lineex();
            break;
        case 4: /* 4 vpss mosaic */
            ret = sample_region_vpss_mosaic();
            break;
        case 5: /* 5 vpss mosaic chn */
            ret = sample_region_vpss_chn_mosaic();
            break;
        case 6: /* 6 vpss mosaicex */
            ret = sample_region_vpss_mosaicex();
            break;
        case 7: /* 7 vpss corner rect */
            ret = sample_region_vpss_corner_rect();
            break;
        case 8: /* 8 vpss corner rectex */
            ret = sample_region_vpss_corner_rectex();
            break;
        case 9: /* 9 venc overlay */
            ret = sample_region_venc_overlay();
            break;
        case 10: /* 10 vo corner rectex */
            ret = sample_region_vo_corner_rectex();
            break;
        case 11: /* 11 vi coverex */
            ret = sample_region_vi_coverex();
            break;
        default:
            ret = TD_FAILURE;
            break;
    }

    return ret;
}

#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    td_s32 ret = TD_FAILURE;
    td_slong index;
#ifndef __LITEOS__
    struct sigaction sa;
#endif

    if (argc != 2) { /* 2: arg num */
        sample_region_usage(argv[0]);
        return TD_FAILURE;
    }
    if (!strncmp(argv[1], "-h", 2)) { /* 2: arg num */
        sample_region_usage(argv[0]);
        return TD_SUCCESS;
    }
    if (strlen(argv[1]) > 2 || strlen(argv[1]) == 0 || !check_digit(argv[1][0]) || /* 2:arg len */
        (strlen(argv[1]) == 2 && (!check_digit(argv[1][1]) || argv[1][0] == '0'))) { /* 2: num width */
        sample_print("the index is invalid!\n");
        sample_region_usage(argv[0]);
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sample_region_handle_sig;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif

    index = strtol(argv[1], TD_NULL, 10); /* 10: base */
    if (!index && strncmp(argv[1], "0", 1)) {
        index = -1;
    }
    if (index > 11 || index < 0) { /* 11:max index */
        sample_print("the index is invalid!\n");
        sample_region_usage(argv[0]);
    } else {
        ret = rgn_get_index_start(index);
    }
    if (ret == TD_SUCCESS) {
        sample_print("program exit normally!\n");
    } else {
        sample_print("program exit abnormally!\n");
    }

    return (ret);
}
