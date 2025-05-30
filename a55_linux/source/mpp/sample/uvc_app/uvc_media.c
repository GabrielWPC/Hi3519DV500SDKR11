/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "uvc_media.h"
#include <unistd.h>
#include "log.h"
#include "ot_common_vb.h"
#include "ot_buffer.h"
#include "sample_comm.h"
#include "sample_venc.h"
#include "ot_camera.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_awb.h"
#include "uvc.h"
#include "ot_common_uvc.h"
#include "ss_mpi_uvc.h"

static uvc_media_cfg g_media_cfg = {0};
static td_bool g_is_media_start = TD_FALSE;
static encoder_property g_encoder_property = {0};
static ot_uvc_data_input_mode g_data_input_mode = OT_UVC_MPP_BIND_UVC;

static td_s32 sample_comm_vpss_bind_uvc(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_uvc_chn uvc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_UVC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = uvc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VPSS-UVC)");

    return TD_SUCCESS;
}

static td_s32 sample_comm_vpss_un_bind_uvc(ot_vpss_grp vpss_grp, ot_vpss_chn vpss_chn, ot_uvc_chn uvc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VPSS;
    src_chn.dev_id = vpss_grp;
    src_chn.chn_id = vpss_chn;

    dest_chn.mod_id = OT_ID_UVC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = uvc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VPSS-UVC)");

    return TD_SUCCESS;
}

static td_s32 sample_comm_venc_bind_uvc(ot_venc_chn venc_chn, ot_uvc_chn uvc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VENC;
    src_chn.dev_id = 0;
    src_chn.chn_id = venc_chn;

    dest_chn.mod_id = OT_ID_UVC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = uvc_chn;

    check_return(ss_mpi_sys_bind(&src_chn, &dest_chn), "ss_mpi_sys_bind(VENC-UVC)");

    return TD_SUCCESS;
}

static td_s32 sample_comm_venc_un_bind_uvc(ot_venc_chn venc_chn, ot_uvc_chn uvc_chn)
{
    ot_mpp_chn src_chn;
    ot_mpp_chn dest_chn;

    src_chn.mod_id = OT_ID_VENC;
    src_chn.dev_id = 0;
    src_chn.chn_id = venc_chn;

    dest_chn.mod_id = OT_ID_UVC;
    dest_chn.dev_id = 0;
    dest_chn.chn_id = uvc_chn;

    check_return(ss_mpi_sys_unbind(&src_chn, &dest_chn), "ss_mpi_sys_unbind(VENC-UVC)");

    return TD_SUCCESS;
}

static td_void sample_uvc_get_default_pic_buf_attr(ot_size *pic_size, ot_pic_buf_attr *buf_attr)
{
    buf_attr->width         = pic_size->width;
    buf_attr->height        = pic_size->height;
    buf_attr->bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr->pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr->compress_mode = OT_COMPRESS_MODE_SEG;
    buf_attr->align         = OT_DEFAULT_ALIGN;
    buf_attr->video_format  = OT_VIDEO_FORMAT_LINEAR;
}

static td_void sample_uvc_get_sensor_size(sample_sns_type sns_type, ot_size *sensor_size)
{
    switch (sns_type) {
        case OV_OS08A20_MIPI_8M_30FPS_12BIT:
        case OV_OS08A20_MIPI_8M_30FPS_10BIT_WDR2TO1:
        case SONY_IMX515_MIPI_8M_30FPS_12BIT:
            sensor_size->width = 3840;      /* 3840: width */
            sensor_size->height = 2160;     /* 2160: height */
            break;
        case OV_OS04A10_MIPI_4M_30FPS_12BIT:
        case OV_OS04A10_SLAVE_MIPI_4M_30FPS_12BIT:
        case OV_OS04A10_MIPI_4M_30FPS_12BIT_WDR2TO1:
            sensor_size->width  = WIDTH_2688;
            sensor_size->height = HEIGHT_1520;
            break;
        case SONY_IMX347_SLAVE_MIPI_4M_30FPS_12BIT:
            sensor_size->width  = WIDTH_2592;
            sensor_size->height = HEIGHT_1520;
            break;
        default:
            sensor_size->width = 3840;      /* 3840: width */
            sensor_size->height = 2160;     /* 2160: height */
            break;
    }
}

static td_bool sample_uvc_is_need_venc(unsigned int format)
{
    if ((format != VIDEO_IMG_FORMAT_YUYV) && (format != VIDEO_IMG_FORMAT_YUV420) &&
        (format != VIDEO_IMG_FORMAT_NV21) && (format != VIDEO_IMG_FORMAT_NV12)) {
        return TD_TRUE;
    } else {
        return TD_FALSE;
    }
}

static td_s32 sample_uvc_sys_init(sample_sns_type sns_type, td_u32 supplement_cfg)
{
    ot_size              sensor_size;
    ot_vb_cfg            vb_cfg = {0};
    td_u32               blk_size;
    ot_pic_buf_attr      buf_attr;
    ot_vb_supplement_cfg vb_supplement_cfg;
    td_s32               ret;

    sample_uvc_get_sensor_size(sns_type, &sensor_size);
    sample_uvc_get_default_pic_buf_attr(&sensor_size, &buf_attr);

    vb_cfg.max_pool_cnt = 1;
    blk_size = ot_common_get_pic_buf_size(&buf_attr);
    vb_cfg.common_pool[0].blk_size = blk_size;
    vb_cfg.common_pool[0].blk_cnt = 12;  /* 12: blk cnt */
    vb_cfg.common_pool[0].remap_mode = OT_VB_REMAP_MODE_CACHED;

    vb_supplement_cfg.supplement_cfg = supplement_cfg;
    ret = ss_mpi_vb_set_supplement_cfg(&vb_supplement_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("set vb cfg failed!\n");
        return TD_FAILURE;
    }

    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_cfg);

    return ret;
}

static td_void sample_uvc_get_vi_vpss_mode(ot_vi_vpss_mode_type vi_vpss_mode_type, ot_vi_vpss_mode *vi_vpss_mode)
{
    td_u32 i;

    for (i = 0; i < OT_VI_MAX_PIPE_NUM; i++) {
        vi_vpss_mode->mode[i] = OT_VI_OFFLINE_VPSS_OFFLINE;
    }

    if (vi_vpss_mode_type == OT_VI_ONLINE_VPSS_ONLINE) {
        vi_vpss_mode->mode[0] = OT_VI_ONLINE_VPSS_ONLINE;
    }
}

static td_s32 sample_uvc_vi_init(uvc_media_cfg *media_cfg)
{
    td_s32 ret;
    ot_vi_vpss_mode vi_vpss_mode;

    sample_comm_vi_get_default_vi_cfg(media_cfg->sns_type, &media_cfg->vi_cfg);
    sample_uvc_get_vi_vpss_mode(media_cfg->vi_vpss_mode_type, &vi_vpss_mode);

    ret = ss_mpi_sys_set_vi_vpss_mode(&vi_vpss_mode);
    if (ret != TD_SUCCESS) {
        sample_print("set vi vpss mode failed!\n");
        return TD_FAILURE;
    }

    ret = ss_mpi_sys_set_vi_aiisp_mode(0, media_cfg->aiisp_mode);
    if (ret != TD_SUCCESS) {
        sample_print("set vi video mode failed!\n");
        return TD_FAILURE;
    }

    for (td_s32 i = 0; i < OT_VI_MAX_PHYS_PIPE_NUM; i++) {
        if (media_cfg->vi_cfg.pipe_info[i].pipe_need_start == TD_TRUE) {
            media_cfg->vi_cfg.pipe_info[i].nr_attr.enable = TD_FALSE;  /* disable 3dnr */
        }
    }
    ret = sample_comm_vi_start_vi(&media_cfg->vi_cfg);

    return ret;
}

static td_void sample_uvc_get_uvc_chn_attr(ot_uvc_chn uvc_chn, ot_uvc_chn_attr* uvc_chn_attr)
{
    if (g_encoder_property.format == VIDEO_IMG_FORMAT_MJPEG) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_MJPEG;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_H264) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_H264;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_H265) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_H265;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_NV12) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_NV12;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_NV21) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_NV21;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_YUYV;
    } else {
        uvc_chn_attr->uvc_format = OT_UVC_FORMAT_BUTT;
    }
    uvc_chn_attr->data_mode = g_data_input_mode;
    if (g_data_input_mode == OT_UVC_SEND_USER_STREAM) {
        uvc_chn_attr->buffer_size = 1536; /* 1536 ring buffer size for copy user stream to uvc module */
    } else {
        uvc_chn_attr->buffer_size = 0;
    }
}

static td_s32 sample_uvc_chn_init(ot_uvc_chn uvc_chn)
{
    td_s32 ret;
    ot_uvc_chn_attr uvc_chn_attr = {0};

    sample_uvc_get_uvc_chn_attr(uvc_chn, &uvc_chn_attr);
    ret = ss_mpi_uvc_create_chn(uvc_chn, &uvc_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_uvc_create_chn(chn:%d) failed with %#x!\n", uvc_chn, ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_uvc_start_chn(uvc_chn);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_uvc_start_chn failed with %#x\n", ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 sample_uvc_chn_deinit(ot_uvc_chn uvc_chn)
{
    td_s32 ret;

    ret = ss_mpi_uvc_stop_chn(uvc_chn);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_uvc_stop_chn failed with %#x\n", ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_uvc_destroy_chn(uvc_chn);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_uvc_destroy_chn(chn:%d) failed with %#x!\n", uvc_chn, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_uvc_disable_vpss_3dnr(ot_vpss_grp grp)
{
    ot_3dnr_attr nr_attr = {0};
    ot_3dnr_pos_type pos = OT_3DNR_POS_VI;
    td_s32 ret;

    ret = ss_mpi_sys_get_3dnr_pos(&pos);
    if (ret != TD_SUCCESS) {
        sample_print("sys_get_3dnr_pos failed with %#x!\n", ret);
        return;
    }
    if (pos == OT_3DNR_POS_VI) {
        return;
    }
    nr_attr.enable = TD_FALSE;
    nr_attr.nr_type = OT_NR_TYPE_VIDEO_NORM;
    nr_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    nr_attr.nr_motion_mode = OT_NR_MOTION_MODE_NORM;
    ret = ss_mpi_vpss_set_grp_3dnr_attr(grp, &nr_attr);
    if (ret != TD_SUCCESS) {
        sample_print("vpss_set_grp_3dnr_attr failed with %#x!\n", ret);
    }
}

static td_void sample_uvc_get_default_vpss_ext_chn_attr(ot_vpss_ext_chn_attr *vpss_ext_chn_attr)
{
    vpss_ext_chn_attr->bind_chn = OT_VPSS_CHN0;
    vpss_ext_chn_attr->src_type = OT_EXT_CHN_SRC_TYPE_TAIL;
    vpss_ext_chn_attr->width = 1920;     /* 1920: default width */
    vpss_ext_chn_attr->height = 1080;    /* 1080: default height */
    vpss_ext_chn_attr->depth = 0;
    vpss_ext_chn_attr->video_format = OT_VIDEO_FORMAT_LINEAR;
    vpss_ext_chn_attr->dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    vpss_ext_chn_attr->pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    vpss_ext_chn_attr->compress_mode = OT_COMPRESS_MODE_NONE;
    vpss_ext_chn_attr->frame_rate.src_frame_rate = -1;
    vpss_ext_chn_attr->frame_rate.dst_frame_rate = -1;
    if (g_data_input_mode == OT_UVC_SEND_YUV_FRAME && g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
        vpss_ext_chn_attr->depth = 1;
    }
}

static td_s32 sample_uvc_vpss_init(uvc_media_cfg *media_cfg)
{
    td_s32 ret;
    ot_low_delay_info low_delay_info;
    ot_vpss_chn vpss_ext_chn = OT_VPSS_MAX_PHYS_CHN_NUM;
    ot_vpss_ext_chn_attr vpss_ext_chn_attr;
    td_bool is_need_venc = sample_uvc_is_need_venc(g_encoder_property.format);
    sample_vpss_chn_attr vpss_chn_attr = {0};

    memcpy_s(&vpss_chn_attr.chn_attr[0], sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM,
        media_cfg->vpss_chn_attr, sizeof(ot_vpss_chn_attr) * OT_VPSS_MAX_PHYS_CHN_NUM);
    memcpy_s(vpss_chn_attr.chn_enable, sizeof(vpss_chn_attr.chn_enable),
        media_cfg->vpss_chn_enable, sizeof(vpss_chn_attr.chn_enable));
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_PHYS_CHN_NUM;
    ret = sample_common_vpss_start(media_cfg->vpss_grp, &media_cfg->vpss_grp_attr, &vpss_chn_attr);
    if (ret != TD_SUCCESS) {
        sample_print("start vpss failed!\n");
        return TD_FAILURE;
    }
    sample_uvc_disable_vpss_3dnr(media_cfg->vpss_grp);

    if (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
        sample_uvc_get_default_vpss_ext_chn_attr(&vpss_ext_chn_attr);
        vpss_ext_chn_attr.width = media_cfg->vpss_chn_attr[0].width;
        vpss_ext_chn_attr.height = media_cfg->vpss_chn_attr[0].height;
        vpss_ext_chn_attr.pixel_format = OT_PIXEL_FORMAT_VY1UY0_PACKAGE_422;
        ret = ss_mpi_vpss_set_ext_chn_attr(media_cfg->vpss_grp, vpss_ext_chn, &vpss_ext_chn_attr);
        if (ret != TD_SUCCESS) {
            sample_print("set vpss ext_chn%d attr failed!\n", vpss_ext_chn);
            goto vpss_fail;
        }

        ret = ss_mpi_vpss_enable_chn(media_cfg->vpss_grp, vpss_ext_chn);
        if (ret != TD_SUCCESS) {
            sample_print("enable vpss ext chn%d failed!\n", vpss_ext_chn);
            goto vpss_fail;
        }
    }

    if (is_need_venc == TD_TRUE) {
        low_delay_info.enable = TD_TRUE;
        low_delay_info.line_cnt = 16;       /* 16: low delay line */
        low_delay_info.one_buf_en = TD_FALSE;
        ret = ss_mpi_vpss_set_chn_low_delay(media_cfg->vpss_grp, media_cfg->vpss_chn, &low_delay_info);
        if (ret != TD_SUCCESS) {
            sample_print("set vpss chn low delay failed!\n");
            goto vpss_fail;
        }
    }

    return TD_SUCCESS;

vpss_fail:
   sample_common_vpss_stop(media_cfg->vpss_grp, media_cfg->vpss_chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
   return TD_FAILURE;
}

static td_void sample_uvc_vpss_de_init(uvc_media_cfg *media_cfg)
{
    if (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
        (td_void)ss_mpi_vpss_disable_chn(media_cfg->vpss_grp, OT_VPSS_MAX_PHYS_CHN_NUM);
    }
    (td_void)sample_common_vpss_stop(media_cfg->vpss_grp, media_cfg->vpss_chn_enable, OT_VPSS_MAX_PHYS_CHN_NUM);
}

static td_void sample_venc_set_one_stream_buf(td_void)
{
    td_s32 ret;
    ot_venc_mod_param mod_param = {0};
    mod_param.mod_type = OT_VENC_MOD_H265;
    ret = ss_mpi_venc_get_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_get_mod_param 265 ret %#x\n", ret);
    }
    mod_param.h265_mod_param.one_stream_buf = 1;
    ret = ss_mpi_venc_set_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_set_mod_param 265 ret %#x\n", ret);
    }

    (td_void)memset_s(&mod_param, sizeof(ot_venc_mod_param), 0, sizeof(ot_venc_mod_param));
    mod_param.mod_type = OT_VENC_MOD_H264;
    ret = ss_mpi_venc_get_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_get_mod_param 264 ret %#x\n", ret);
    }
    mod_param.h264_mod_param.one_stream_buf = 1;
    ret = ss_mpi_venc_set_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_set_mod_param 264 ret %#x\n", ret);
    }

    (td_void)memset_s(&mod_param, sizeof(ot_venc_mod_param), 0, sizeof(ot_venc_mod_param));
    mod_param.mod_type = OT_VENC_MOD_JPEG;
    ret = ss_mpi_venc_get_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_get_mod_param jpeg ret %#x\n", ret);
    }
    mod_param.jpeg_mod_param.one_stream_buf = 1;
    ret = ss_mpi_venc_set_mod_param(&mod_param);
    if (ret != TD_SUCCESS) {
        sample_print("venc_set_mod_param jpeg ret %#x\n", ret);
    }
}

static td_s32 sample_uvc_venc_init(uvc_media_cfg *media_cfg)
{
    td_s32 ret;
    ot_venc_chn venc_chn = media_cfg->venc_chn;

    if (media_cfg->venc_chn_param.type == OT_PT_H264) {
        media_cfg->venc_chn_param.rc_mode = SAMPLE_RC_AVBR;
    }

    sample_venc_set_one_stream_buf();

    ret = sample_comm_venc_start(venc_chn, &media_cfg->venc_chn_param);
    if (ret != TD_SUCCESS) {
        sample_print("start venc failed!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void sample_uvc_get_default_venc_chn_param(sample_comm_venc_chn_param *venc_chn_param)
{
    venc_chn_param->frame_rate = 30;            /* 30: default frame rate */
    venc_chn_param->stats_time = 2;             /* 2: default stats_time */
    venc_chn_param->gop = 60;                   /* 60: default gop */
    venc_chn_param->venc_size.width = 1920;     /* 1920: default width */
    venc_chn_param->venc_size.height = 1080;    /* 1080: default height */
    venc_chn_param->profile = 0;
    venc_chn_param->is_rcn_ref_share_buf = TD_FALSE;
    venc_chn_param->gop_attr.gop_mode = OT_VENC_GOP_MODE_NORMAL_P;
    venc_chn_param->gop_attr.normal_p.ip_qp_delta = 2;  /* 2: default ip_qp_delta */
    venc_chn_param->type = OT_PT_H264;
    venc_chn_param->rc_mode = SAMPLE_RC_CBR;
}

static td_void sample_uvc_media_cfg(uvc_media_cfg *media_cfg)
{
    td_u32 i;
    ot_size sensor_size;

    media_cfg->sns_type = SENSOR0_TYPE;
    media_cfg->aiisp_mode = OT_VI_AIISP_MODE_DEFAULT;
    media_cfg->vi_vpss_mode_type = OT_VI_ONLINE_VPSS_ONLINE;

    sample_uvc_get_sensor_size(media_cfg->sns_type, &sensor_size);

    sample_comm_vpss_get_default_grp_attr(&media_cfg->vpss_grp_attr);
    media_cfg->vpss_grp_attr.max_width = sensor_size.width;
    media_cfg->vpss_grp_attr.max_height = sensor_size.height;

    for (i = 0; i < OT_VPSS_MAX_PHYS_CHN_NUM; i++) {
        sample_comm_vpss_get_default_chn_attr(&media_cfg->vpss_chn_attr[i]);
        media_cfg->vpss_chn_attr[i].width = sensor_size.width;
        media_cfg->vpss_chn_attr[i].height = sensor_size.height;
        media_cfg->vpss_chn_enable[i] = TD_FALSE;
        media_cfg->vpss_chn_attr[i].compress_mode = OT_COMPRESS_MODE_NONE;
    }
    media_cfg->vpss_chn = OT_VPSS_CHN0;
    media_cfg->vpss_chn_enable[OT_VPSS_CHN0] = TD_TRUE;

    sample_uvc_get_default_venc_chn_param(&media_cfg->venc_chn_param);

    media_cfg->supplement_cfg = OT_VB_SUPPLEMENT_BNR_MOT_MASK;
}

static td_s32 sample_uvc_init_and_bind_vi_vpss(uvc_media_cfg *media_cfg)
{
    td_s32 ret = sample_uvc_vi_init(media_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("vi init failed!\n");
        return TD_FAILURE;
    }

    ret = sample_uvc_vpss_init(media_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("vpss init failed!\n");
        goto vi_exit;
    }

    ret = sample_comm_vi_bind_vpss(media_cfg->vi_pipe, media_cfg->vi_chn, media_cfg->vpss_grp, media_cfg->vpss_chn);
    if (ret != TD_SUCCESS) {
        sample_print("vi bind vpss failed!\n");
        goto vpss_exit;
    }
    return TD_SUCCESS;

vpss_exit:
    sample_uvc_vpss_de_init(media_cfg);
vi_exit:
    sample_comm_vi_stop_vi(&media_cfg->vi_cfg);
    return TD_FAILURE;
}

static td_s32 sampe_uvc_start_vi_vpss_venc(uvc_media_cfg *media_cfg)
{
    ot_uvc_chn    uvc_chn = 0;

    td_s32 ret = sample_uvc_init_and_bind_vi_vpss(media_cfg);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_uvc_chn_init(uvc_chn);
    if (ret != TD_SUCCESS) {
        sample_print("init uvc err for %#x!\n", ret);
        goto vi_unbind_vpss;
    }

    if (sample_uvc_is_need_venc(g_encoder_property.format) == TD_TRUE) {
        ret = sample_uvc_venc_init(media_cfg);
        if (ret != TD_SUCCESS) {
            sample_print("venc init failed!\n");
            goto uvc_exit;
        }

        ret = sample_comm_vpss_bind_venc(media_cfg->vpss_grp, media_cfg->vpss_chn, media_cfg->venc_chn);
        if (ret != TD_SUCCESS) {
            goto venc_exit;
        }
        if (g_data_input_mode == OT_UVC_MPP_BIND_UVC) {
            ret = sample_comm_venc_bind_uvc(media_cfg->venc_chn, uvc_chn);
            if (ret != TD_SUCCESS) {
                goto vpss_unbind_venc;
            }
        }
    } else {
        if (g_data_input_mode != OT_UVC_SEND_YUV_FRAME) {
            ot_vpss_chn vpss_chn =
                g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV ? OT_VPSS_MAX_PHYS_CHN_NUM : media_cfg->vpss_chn;
            ret = sample_comm_vpss_bind_uvc(media_cfg->vpss_grp, vpss_chn, uvc_chn);
            if (ret != TD_SUCCESS) {
                goto uvc_exit;
            }
        }
    }

    return TD_SUCCESS;

vpss_unbind_venc:
    sample_comm_vpss_un_bind_venc(media_cfg->vpss_grp, media_cfg->vpss_chn, media_cfg->venc_chn);
venc_exit:
    sample_comm_venc_stop(media_cfg->venc_chn);
uvc_exit:
    sample_uvc_chn_deinit(uvc_chn);
vi_unbind_vpss:
    sample_comm_vi_un_bind_vpss(media_cfg->vi_pipe, media_cfg->vi_chn, media_cfg->vpss_grp, media_cfg->vpss_chn);
    sample_uvc_vpss_de_init(media_cfg);
    sample_comm_vi_stop_vi(&media_cfg->vi_cfg);
    return TD_FAILURE;
}

static td_void sample_uvc_stop_vi_vpss_venc(uvc_media_cfg *media_cfg)
{
    ot_vi_pipe    vi_pipe      = media_cfg->vi_pipe;
    ot_vi_chn     vi_chn       = media_cfg->vi_chn;
    sample_vi_cfg *vi_cfg      = &media_cfg->vi_cfg;
    ot_vpss_grp   vpss_grp     = media_cfg->vpss_grp;
    ot_vpss_chn   vpss_chn     = media_cfg->vpss_chn;
    ot_vpss_chn   vpss_ext_chn = OT_VPSS_MAX_PHYS_CHN_NUM;
    ot_venc_chn   venc_chn     = media_cfg->venc_chn;
    ot_uvc_chn    uvc_chn      = 0;
    td_bool       is_need_venc = sample_uvc_is_need_venc(g_encoder_property.format);
    if (is_need_venc == TD_TRUE) {
        (td_void)ss_mpi_venc_stop_chn(venc_chn); // need stop venc chn firstly when venc unbind uvc.
        if (g_data_input_mode == OT_UVC_MPP_BIND_UVC) {
            sample_comm_venc_un_bind_uvc(venc_chn, uvc_chn);
        }
        sample_comm_vpss_un_bind_venc(vpss_grp, vpss_chn, venc_chn);
        sample_uvc_chn_deinit(uvc_chn);
        sample_comm_venc_stop(venc_chn);
    } else {
        if (g_data_input_mode != OT_UVC_SEND_YUV_FRAME) {
            if (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
                sample_comm_vpss_un_bind_uvc(vpss_grp, vpss_ext_chn, uvc_chn);
            } else {
                sample_comm_vpss_un_bind_uvc(vpss_grp, vpss_chn, uvc_chn);
            }
        }
        sample_uvc_chn_deinit(uvc_chn);
    }

    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp, vpss_chn);
    sample_uvc_vpss_de_init(media_cfg);
    sample_comm_vi_stop_vi(vi_cfg);
}

static td_s32 sample_uvc_media_route(td_void)
{
    td_s32 ret;
    ot_pic_size pic_size = PIC_1080P;
    ot_size stream_size;

    if (g_data_input_mode == OT_UVC_SEND_YUV_FRAME && sample_uvc_is_need_venc(g_encoder_property.format)) {
        g_data_input_mode = OT_UVC_SEND_VENC_STREAM;
    } else if (g_data_input_mode == OT_UVC_SEND_VENC_STREAM && !sample_uvc_is_need_venc(g_encoder_property.format)) {
        g_data_input_mode = OT_UVC_SEND_YUV_FRAME;
    }
    sample_uvc_media_cfg(&g_media_cfg);

    /* edit media cfg here */
    set_user_config_format(&g_media_cfg.venc_chn_param.type, &pic_size, &g_media_cfg.venc_chn_num);
    ret = sample_comm_sys_get_pic_size(pic_size, &stream_size);
    if (ret != TD_SUCCESS) {
        sample_print("get stream_size failed!\n");
        return ret;
    }

    g_media_cfg.venc_chn_param.venc_size.width = stream_size.width;
    g_media_cfg.venc_chn_param.venc_size.height = stream_size.height;
    g_media_cfg.venc_chn_param.size = pic_size;

    g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].width = stream_size.width;
    g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].height = stream_size.height;
    g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].frame_rate.src_frame_rate = FRAME_INTERVAL_MAX;
    g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].frame_rate.dst_frame_rate = g_encoder_property.fps;
    if (g_data_input_mode == OT_UVC_SEND_YUV_FRAME && g_encoder_property.format != VIDEO_IMG_FORMAT_YUYV) {
        g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].depth = 1; /* for user space get YUV(NV12 or NV21) */
    } else {
        g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].depth = 0;
    }
 
    if (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) {
        g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_NV12) {
        g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    } else if (g_encoder_property.format == VIDEO_IMG_FORMAT_NV21) {
        g_media_cfg.vpss_chn_attr[g_media_cfg.vpss_chn].pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }

    ret = sampe_uvc_start_vi_vpss_venc(&g_media_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vi-vpss-venc failed!\n");
        goto sys_exit;
    }

    return TD_SUCCESS;

sys_exit:
    sample_comm_sys_exit();
    return ret;
}

uint16_t venc_brightness_get(td_void)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);

    return (uint16_t)cscf_attr.luma;
}

uint16_t venc_contrast_get(td_void)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);

    return (uint16_t)cscf_attr.contr;
}

uint16_t venc_hue_get(td_void)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);

    return (uint16_t)cscf_attr.hue;
}

uint8_t venc_power_line_frequency_get(td_void)
{
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);

    if ((exp_attr.auto_attr.antiflicker.frequency != 50) &&   /* 50: frequency */
        (exp_attr.auto_attr.antiflicker.frequency != 60)) {   /* 60: frequency */
        return 0;
    }

    return (exp_attr.auto_attr.antiflicker.frequency == 50) ? 0x1 : 0x2;    /* 50: frequency */
}

uint16_t venc_saturation_get(td_void)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);

    return (uint16_t)cscf_attr.satu;
}

uint8_t venc_white_balance_temperature_auto_get(td_void)
{
    ot_isp_wb_attr wb_attr;

    ss_mpi_isp_get_wb_attr(0, &wb_attr);

    return (wb_attr.op_type == OT_OP_MODE_AUTO) ? 0x1 : 0x0;
}

uint16_t venc_white_balance_temperature_get(td_void)
{
    ot_isp_wb_info wb_info;

    ss_mpi_isp_query_wb_info(0, &wb_info);

    return (uint16_t)wb_info.color_temp;
}

td_void venc_brightness_set(uint16_t v)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);
    cscf_attr.luma = (td_u8)v;
    ss_mpi_isp_set_csc_attr(0, &cscf_attr);
}

td_void venc_contrast_set(uint16_t v)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);
    cscf_attr.contr = (td_u8)v;
    ss_mpi_isp_set_csc_attr(0, &cscf_attr);
}

td_void venc_hue_set(uint16_t v)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);
    cscf_attr.hue = (td_u8)v;
    ss_mpi_isp_set_csc_attr(0, &cscf_attr);
}

td_void venc_power_line_frequency_set(uint8_t v)
{
    td_s32 ret;
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);
    if (v == 0) {
        exp_attr.auto_attr.antiflicker.enable = TD_FALSE;
    } else if (v == 1) {
        exp_attr.auto_attr.antiflicker.enable = TD_TRUE;
        exp_attr.auto_attr.antiflicker.frequency = 50;  /* 50: frequency */
    } else if (v == 2) {    /* 2: value */
        exp_attr.auto_attr.antiflicker.enable = TD_TRUE;
        exp_attr.auto_attr.antiflicker.frequency = 60;  /* 60: frequency */
    }

    ret = ss_mpi_isp_set_exposure_attr(0, &exp_attr);
    if (ret != TD_SUCCESS) {
        rlog("ss_mpi_isp_set_exposure_attr err 0x%x\n", ret);
    }
}

td_void venc_saturation_set(uint16_t v)
{
    ot_isp_csc_attr cscf_attr;

    ss_mpi_isp_get_csc_attr(0, &cscf_attr);
    cscf_attr.satu = (td_u8)v;
    ss_mpi_isp_set_csc_attr(0, &cscf_attr);
}

td_void venc_white_balance_temperature_auto_set(uint8_t v)
{
    ot_isp_wb_attr wb_attr;

    ss_mpi_isp_get_wb_attr(0, &wb_attr);
    wb_attr.op_type = (v == 1) ? OT_OP_MODE_AUTO : OT_OP_MODE_MANUAL;
    ss_mpi_isp_set_wb_attr(0, &wb_attr);
}

td_void venc_white_balance_temperature_set(uint16_t v)
{
    ot_isp_wb_info wb_info;
    ot_isp_wb_attr wb_attr;
    td_u16 color_temp;
    td_u16 awb_gain[4];
    errno_t ret;

    ss_mpi_isp_query_wb_info(0, &wb_info);
    ss_mpi_isp_get_wb_attr(0, &wb_attr);

    color_temp = v;
    ss_mpi_isp_cal_gain_by_temp(0, &wb_attr, color_temp, 0, awb_gain);

    wb_attr.op_type = OT_OP_MODE_MANUAL;
    ret = memcpy_s(&wb_attr.manual_attr, sizeof(wb_attr.manual_attr), awb_gain, sizeof(wb_attr.manual_attr));
    if (ret != EOK) {
        err("memcpy_s awb gain fail %x\n", ret);
    }

    ss_mpi_isp_set_wb_attr(0, &wb_attr);
}

uint8_t venc_exposure_auto_mode_get(td_void)
{
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);

    return (exp_attr.op_type == OT_OP_MODE_AUTO) ? 0x02 : 0x04;
}

uint32_t venc_exposure_ansolute_time_get(td_void)
{
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);

    return exp_attr.manual_attr.exp_time;
}

td_void venc_exposure_auto_mode_set(uint8_t v)
{
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);
    exp_attr.op_type = (v == 4) ? OT_OP_MODE_MANUAL : OT_OP_MODE_AUTO;  /* 4: value */
    ss_mpi_isp_set_exposure_attr(0, &exp_attr);
}

td_void venc_exposure_ansolute_time_set(uint32_t v)
{
    ot_isp_exposure_attr exp_attr;

    ss_mpi_isp_get_exposure_attr(0, &exp_attr);
    exp_attr.manual_attr.exp_time = v * 100;    /* 100: ratio */
    exp_attr.manual_attr.exp_time_op_type = OT_OP_MODE_MANUAL;
    ss_mpi_isp_set_exposure_attr(0, &exp_attr);
}

td_s32 sample_venc_set_idr(td_void)
{
    return ss_mpi_venc_request_idr(0, TD_TRUE);
}

td_s32 sample_venc_init(td_void)
{
    td_s32 ret;
    sample_sns_type sns_type = SENSOR0_TYPE;

    ret = sample_uvc_sys_init(sns_type, g_media_cfg.supplement_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("sys init failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 sample_venc_deinit(td_void)
{
    sample_comm_sys_exit();
    return TD_SUCCESS;
}

td_s32 sample_venc_startup(td_void)
{
    td_s32 ret;

    g_is_media_start = TD_TRUE;
    ret = sample_uvc_media_route();
    if (g_encoder_property.format == VIDEO_IMG_FORMAT_H264 || g_encoder_property.format == VIDEO_IMG_FORMAT_H265) {
        (td_void)ss_mpi_venc_request_idr(0, TD_TRUE);
    }
    return ret;
}

td_s32 sample_venc_shutdown(td_void)
{
    if (g_is_media_start == TD_FALSE) {
        return 0;
    }

    g_is_media_start = TD_FALSE;
    sample_uvc_stop_vi_vpss_venc(&g_media_cfg);

    return 0;
}

td_s32 sample_venc_set_property(encoder_property *p)
{
    if (p == NULL) {
        err("p is NULL\n");
        return TD_FAILURE;
    }
    (td_void)memcpy_s(&g_encoder_property, sizeof(encoder_property), p, sizeof(encoder_property));
    return 0;
}

td_void sample_uvc_get_encoder_property(encoder_property *p)
{
    if (p == NULL) {
        err("p is NULL\n");
        return;
    }
    (td_void)memcpy_s(p, sizeof(encoder_property), &g_encoder_property, sizeof(encoder_property));
}

td_void sample_uvc_send_user_stream(ot_venc_stream *venc_stream)
{
    td_s32 ret;
    ot_uvc_user_stream user_stream;

    for (td_u32 i = 0; i < venc_stream->pack_cnt; i++) {
        user_stream.pts = venc_stream->pack[i].pts;
        user_stream.addr = venc_stream->pack[i].addr + venc_stream->pack[i].offset;
        user_stream.len = venc_stream->pack[i].len - venc_stream->pack[i].offset;
        user_stream.is_frame_end = venc_stream->pack[i].is_frame_end;
        ret = ss_mpi_uvc_send_user_stream(0, &user_stream, 0); /* replace stream addr with your own buffer */
        if (ret != 0) {
            sample_print("ss_mpi_uvc_send_user_stream failed %#x\n", ret);
        }
    }
}

td_s32 sample_send_yuv_frame(td_void)
{
    td_s32 ret = TD_SUCCESS;
    ot_video_frame_info frame_info;
    ot_vpss_chn vpss_chn =
        (g_encoder_property.format == VIDEO_IMG_FORMAT_YUYV) ? OT_VPSS_MAX_PHYS_CHN_NUM : g_media_cfg.vpss_chn;

    if ((ret = ss_mpi_vpss_get_chn_frame(0, vpss_chn, &frame_info, 100)) != TD_SUCCESS) { /* 100 timeout ms */
        printf("get frame from VPSS %d fail(0x%x)!\n", vpss_chn, ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_uvc_send_frame(0, &frame_info, 0);
    if (ret != TD_SUCCESS && ret != OT_ERR_UVC_UNEXIST) {
        sample_print("uvc_send_frame failed %#x\n", ret);
    }

    /* release frame after using */
    ret = ss_mpi_vpss_release_chn_frame(0, vpss_chn, &frame_info);
    if (ret != TD_SUCCESS) {
        sample_print("mpi_vpss_release_chn_frame failed %#x\n", ret);
    }
    return ret;
}

td_s32 sample_venc_get_uvc_send(td_void)
{
    td_s32 ret;
    ot_venc_stream stream = {0};
    ot_venc_chn_status stat;

    if (g_is_media_start == TD_TRUE && g_data_input_mode == OT_UVC_SEND_YUV_FRAME
        && !sample_uvc_is_need_venc(g_encoder_property.format)) {
        return sample_send_yuv_frame();
    }

    if (g_is_media_start == TD_FALSE || g_data_input_mode == OT_UVC_MPP_BIND_UVC ||
        (!sample_uvc_is_need_venc(g_encoder_property.format))) {
        return TD_SUCCESS;
    }

    ret = ss_mpi_venc_query_status(g_media_cfg.venc_chn, &stat);
    if (ret == OT_ERR_VENC_UNEXIST) {
        return TD_SUCCESS;
    } else if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_venc_query_status chn[%d] failed with %#x!\n", g_media_cfg.venc_chn, ret);
        return TD_FAILURE;
    }

    if (stat.cur_packs == 0) {
        return TD_SUCCESS;
    }

    stream.pack = (ot_venc_pack *)malloc(sizeof(ot_venc_pack) * stat.cur_packs);
    if (stream.pack == TD_NULL) {
        sample_print("malloc stream pack failed!\n");
        return TD_FAILURE;
    }

    stream.pack_cnt = stat.cur_packs;
    ret = ss_mpi_venc_get_stream(g_media_cfg.venc_chn, &stream, 100); /* get stream timeout 100 ms */
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_venc_get_stream failed with %#x!\n", ret);
        goto end;
    }

    if (g_data_input_mode == OT_UVC_SEND_VENC_STREAM) {
        ret = ss_mpi_uvc_send_stream(0, g_media_cfg.venc_chn, &stream, 0);
        if (ret != TD_SUCCESS) {
            sample_print("ss_mpi_uvc_send_stream failed with %#x!\n", ret);
            goto end;
        }
    } else { /* OT_UVC_SEND_USER_STREAM */
        sample_uvc_send_user_stream(&stream);
    }

end:
    ret = ss_mpi_venc_release_stream(g_media_cfg.venc_chn, &stream);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_venc_release_stream failed %x\n", ret);
    }

    free(stream.pack);

    return ret;
}

