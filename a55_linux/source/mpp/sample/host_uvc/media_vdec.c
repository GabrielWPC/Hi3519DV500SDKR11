/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "media_vdec.h"
#include <unistd.h>

ot_vb_src g_vdec_vb_source = OT_VB_SRC_MOD;

#define REF_NUM 2
#define DISPLAY_NUM 2

static vdec_display_cfg g_vdec_display_cfg = {
    .pic_size = PIC_1080P,
    .intf_sync = OT_VO_OUT_1080P60,
    .intf_type = OT_VO_INTF_BT1120,
};

static td_u32 g_input_width;
static td_u32 g_input_height;
static ot_size g_disp_size;
sample_vo_cfg g_vo_config;
static td_bool g_is_need_vdec = TD_TRUE;

static td_s32 sample_uvc_init_module_vb(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, ot_payload_type type,
    td_u32 len)
{
    td_u32 i;
    td_s32 ret;
    for (i = 0; (i < vdec_chn_num) && (i < len); i++) {
        sample_vdec[i].type                           = type;
        sample_vdec[i].width                         = g_input_width;
        sample_vdec[i].height                        = g_input_height;
        sample_vdec[i].mode                           = sample_comm_vdec_get_lowdelay_en() ? OT_VDEC_SEND_MODE_COMPAT :
                                                        OT_VDEC_SEND_MODE_FRAME;
        sample_vdec[i].sample_vdec_video.dec_mode      = OT_VIDEO_DEC_MODE_IP;
        sample_vdec[i].sample_vdec_video.bit_width     = OT_DATA_BIT_WIDTH_8;
        if (type == OT_PT_JPEG) {
            sample_vdec[i].sample_vdec_video.ref_frame_num = 0;
        } else {
            sample_vdec[i].sample_vdec_video.ref_frame_num = REF_NUM;
        }
        sample_vdec[i].display_frame_num               = DISPLAY_NUM;
        sample_vdec[i].frame_buf_cnt = (type == OT_PT_JPEG) ? (sample_vdec[i].display_frame_num + 1) :
            (sample_vdec[i].sample_vdec_video.ref_frame_num + sample_vdec[i].display_frame_num + 1);
        if (type == OT_PT_JPEG) {
            sample_vdec[i].sample_vdec_picture.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            sample_vdec[i].sample_vdec_picture.alpha      = 255; /* 255:pic alpha value */
        }
    }
    ret = sample_comm_vdec_init_vb_pool(vdec_chn_num, &sample_vdec[0], len);
    if (ret != TD_SUCCESS) {
        sample_print("init mod common vb fail for %#x!\n", ret);
        return ret;
    }
    return ret;
}

static td_s32 sample_uvc_init_sys_and_vb(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, ot_payload_type type,
    td_u32 len)
{
    ot_vb_cfg vb_cfg;
    ot_pic_buf_attr buf_attr = {0};
    td_s32 ret;

    ret = sample_comm_sys_get_pic_size(g_vdec_display_cfg.pic_size, &g_disp_size);
    if (ret != TD_SUCCESS) {
        sample_print("sys get pic size fail for %#x!\n", ret);
        return ret;
    }
    buf_attr.align = 0;
    buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    buf_attr.compress_mode = OT_COMPRESS_MODE_SEG;
    buf_attr.height = 2160; /* 2160:height */
    buf_attr.width = 3840;  /* 3840:width */
    buf_attr.pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    (td_void)memset_s(&vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg.max_pool_cnt             = 1;
    vb_cfg.common_pool[0].blk_cnt  = 10 * vdec_chn_num; /* 10:common vb cnt */
    vb_cfg.common_pool[0].blk_size = ot_common_get_pic_buf_size(&buf_attr);
    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, OT_VB_SUPPLEMENT_BNR_MOT_MASK);
    if (ret != TD_SUCCESS) {
        sample_print("init sys fail for %#x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }

    if (g_is_need_vdec == TD_TRUE) {
        ret = sample_uvc_init_module_vb(&sample_vdec[0], vdec_chn_num, type, len);
        if (ret != TD_SUCCESS) {
            sample_print("init mod vb fail for %#x!\n", ret);
            sample_comm_vdec_exit_vb_pool();
            sample_comm_sys_exit();
            return ret;
        }
    }

    return ret;
}

static td_s32 sample_uvc_vdec_bind_vpss(td_u32 vpss_grp_num)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vdec_bind_vpss(i, i);
        if (ret != TD_SUCCESS) {
            sample_print("vdec bind vpss fail for %#x!\n", ret);
            return ret;
        }
    }
    return ret;
}

static td_void sample_uvc_stop_vpss(ot_vpss_grp vpss_grp, td_bool *vpss_chn_enable, td_u32 chn_array_size)
{
    td_s32 i;
    for (i = vpss_grp; i >= 0; i--) {
        vpss_grp = i;
        sample_common_vpss_stop(vpss_grp, &vpss_chn_enable[0], chn_array_size);
    }
}

static td_s32 sample_uvc_vdec_unbind_vpss(td_u32 vpss_grp_num)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vdec_un_bind_vpss(i, i);
        if (ret != TD_SUCCESS) {
            sample_print("vdec unbind vpss fail for %#x!\n", ret);
        }
    }
    return ret;
}

static td_void sample_uvc_config_vpss_grp_attr(ot_vpss_grp_attr *vpss_grp_attr)
{
    vpss_grp_attr->max_width = g_input_width;
    vpss_grp_attr->max_height = g_input_height;
    vpss_grp_attr->frame_rate.src_frame_rate = -1;
    vpss_grp_attr->frame_rate.dst_frame_rate = -1;
    vpss_grp_attr->pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vpss_grp_attr->ie_en   = TD_FALSE;
    vpss_grp_attr->dci_en   = TD_FALSE;
    vpss_grp_attr->dei_mode = OT_VPSS_DEI_MODE_OFF;
    vpss_grp_attr->buf_share_en   = TD_FALSE;
}

static td_void sample_uvc_stop_vdec(td_u32 vdec_chn_num)
{
    if (g_is_need_vdec == TD_FALSE) {
        return;
    }

    sample_comm_vdec_stop(vdec_chn_num);
    sample_comm_vdec_exit_vb_pool();

    return;
}

static td_s32 sample_uvc_start_vdec(sample_vdec_attr *sample_vdec, td_u32 vdec_chn_num, td_u32 len)
{
    td_s32 ret;

    if (g_is_need_vdec == TD_FALSE) {
        return TD_SUCCESS;
    }

    ret = sample_comm_vdec_start(vdec_chn_num, &sample_vdec[0], len);
    if (ret != TD_SUCCESS) {
        sample_print("start VDEC fail for %#x!\n", ret);
        sample_uvc_stop_vdec(vdec_chn_num);
    }

    return ret;
}

static td_s32 sample_uvc_config_vpss_ldy_attr(td_u32 vpss_grp_num)
{
    td_u32 i;
    td_s32 ret;
    ot_low_delay_info vpss_ldy_info;

    for (i = 0; i < vpss_grp_num; i++) {
        ret = ss_mpi_vpss_get_chn_low_delay(i, 0, &vpss_ldy_info);
        if (ret != TD_SUCCESS) {
            sample_print("vpss get low delay attr fail for %#x!\n", ret);
            return ret;
        }
        vpss_ldy_info.enable = TD_TRUE;
        vpss_ldy_info.line_cnt = g_disp_size.height / 4 * 1; /* 1/4:lowdelay line num */
        ret = ss_mpi_vpss_set_chn_low_delay(i, 0, &vpss_ldy_info);
        if (ret != TD_SUCCESS) {
            sample_print("vpss set low delay attr fail for %#x!\n", ret);
            return ret;
        }
    }
    return TD_SUCCESS;
}

static td_s32 sample_uvc_start_vpss(ot_vpss_grp *vpss_grp, td_u32 vpss_grp_num,
    td_bool *vpss_chn_enable, td_u32 arr_len)
{
    td_u32 i;
    td_s32 ret;
    sample_vpss_chn_attr vpss_chn_attr = {0};
    ot_vpss_grp_attr vpss_grp_attr = {0};
    sample_uvc_config_vpss_grp_attr(&vpss_grp_attr);
    (td_void)memset_s(vpss_chn_enable, arr_len * sizeof(td_bool), 0, arr_len * sizeof(td_bool));
    if (arr_len > 1) {
        vpss_chn_enable[0] = TD_TRUE;
        vpss_chn_attr.chn_attr[0].width                     = g_disp_size.width;
        vpss_chn_attr.chn_attr[0].height                    = g_disp_size.height;
        vpss_chn_attr.chn_attr[0].chn_mode                  = OT_VPSS_CHN_MODE_USER;
        vpss_chn_attr.chn_attr[0].compress_mode             = OT_COMPRESS_MODE_NONE;
        vpss_chn_attr.chn_attr[0].pixel_format              = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
        vpss_chn_attr.chn_attr[0].frame_rate.src_frame_rate = -1;
        vpss_chn_attr.chn_attr[0].frame_rate.dst_frame_rate = -1;
        vpss_chn_attr.chn_attr[0].depth                     = 0;
        vpss_chn_attr.chn_attr[0].mirror_en                 = TD_FALSE;
        vpss_chn_attr.chn_attr[0].flip_en                   = TD_FALSE;
        vpss_chn_attr.chn_attr[0].border_en                 = TD_FALSE;
        vpss_chn_attr.chn_attr[0].aspect_ratio.mode         = OT_ASPECT_RATIO_NONE;
    }

    memcpy_s(vpss_chn_attr.chn_enable, sizeof(vpss_chn_attr.chn_enable),
        vpss_chn_enable, sizeof(vpss_chn_attr.chn_enable));
    vpss_chn_attr.chn_array_size = OT_VPSS_MAX_CHN_NUM;
    for (i = 0; i < vpss_grp_num; i++) {
        *vpss_grp = i;
        ret = sample_common_vpss_start(*vpss_grp, &vpss_grp_attr, &vpss_chn_attr);
        if (ret != TD_SUCCESS) {
            sample_print("start VPSS fail for %#x!\n", ret);
            sample_uvc_stop_vpss(*vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
            return ret;
        }
    }

    ret = sample_uvc_config_vpss_ldy_attr(vpss_grp_num);
    if (ret != TD_SUCCESS) {
        sample_uvc_stop_vpss(*vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
        return ret;
    }

    ret = sample_uvc_vdec_bind_vpss(vpss_grp_num);
    if (ret != TD_SUCCESS) {
        sample_uvc_vdec_unbind_vpss(vpss_grp_num);
        sample_uvc_stop_vpss(*vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
    }
    return ret;
}

static td_s32 sample_uvc_vpss_bind_vo(const sample_vo_cfg *vo_config, td_u32 vpss_grp_num)
{
    td_u32 i;
    ot_vo_layer vo_layer;
    td_s32 ret = TD_SUCCESS;
    vo_layer = vo_config->vo_dev;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vpss_bind_vo(i, 0, vo_layer, i);
        if (ret != TD_SUCCESS) {
            sample_print("vpss bind vo fail for %#x!\n", ret);
            return ret;
        }
    }
    return ret;
}

static td_s32 sample_uvc_vpss_unbind_vo(td_u32 vpss_grp_num, const sample_vo_cfg *vo_config)
{
    td_u32 i;
    ot_vo_layer vo_layer = vo_config->vo_dev;
    td_s32 ret = TD_SUCCESS;
    for (i = 0; i < vpss_grp_num; i++) {
        ret = sample_comm_vpss_un_bind_vo(i, 0, vo_layer, i);
        if (ret != TD_SUCCESS) {
            sample_print("vpss unbind vo fail for %#x!\n", ret);
        }
    }
    return ret;
}

static td_void sample_uvc_get_default_vo_cfg(sample_vo_cfg *vo_config)
{
    vo_config->vo_dev            = SAMPLE_VO_DEV_UHD;
    vo_config->vo_intf_type      = g_vdec_display_cfg.intf_type;
    vo_config->intf_sync         = g_vdec_display_cfg.intf_sync;
    vo_config->pic_size          = g_vdec_display_cfg.pic_size;
    vo_config->bg_color          = COLOR_RGB_BLUE;
    vo_config->dis_buf_len       = 3; /* 3:buf length */
    vo_config->dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    vo_config->vo_mode           = VO_MODE_1MUX;
    vo_config->pix_format        = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    vo_config->disp_rect.x       = 0;
    vo_config->disp_rect.y       = 0;
    vo_config->disp_rect.width   = g_disp_size.width;
    vo_config->disp_rect.height  = g_disp_size.height;
    vo_config->image_size.width  = g_disp_size.width;
    vo_config->image_size.height = g_disp_size.height;
    vo_config->vo_part_mode      = OT_VO_PARTITION_MODE_SINGLE;
    vo_config->compress_mode     = OT_COMPRESS_MODE_NONE;
}

static td_s32 sample_uvc_start_vo(sample_vo_cfg *vo_config, td_u32 vpss_grp_num)
{
    td_s32 ret;

    sample_uvc_get_default_vo_cfg(vo_config);

    ret = sample_comm_vo_start_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("start VO fail for %#x!\n", ret);
        sample_comm_vo_stop_vo(vo_config);
        return ret;
    }

    ret = sample_uvc_vpss_bind_vo(vo_config, vpss_grp_num);
    if (ret != TD_SUCCESS) {
        sample_uvc_vpss_unbind_vo(vpss_grp_num, vo_config);
        sample_comm_vo_stop_vo(vo_config);
    }

    return ret;
}

static ot_payload_type sample_uvc_get_payload_type(const td_char *type_name)
{
    if (strcmp(type_name, "H264") == 0) {
        return OT_PT_H264;
    } else if (strcmp(type_name, "H265") == 0) {
        return OT_PT_H265;
    } else if (strcmp(type_name, "MJPEG") == 0) {
        return OT_PT_JPEG;
    } else {
        sample_print("type name error!\n");
        return OT_PT_BUTT;
    }
}

static td_void sample_uvc_update_vdec_flag(const td_char *type_name)
{
    if (type_name == TD_NULL) {
        g_is_need_vdec = TD_FALSE;
        return;
    }
    if ((strcmp(type_name, "H264") == 0) ||
        (strcmp(type_name, "H265") == 0) ||
        (strcmp(type_name, "MJPEG") == 0)) {
        g_is_need_vdec = TD_TRUE;
    } else {
        g_is_need_vdec = TD_FALSE;
    }
}

td_s32 sample_uvc_media_init(const td_char *type_name, td_u32 width, td_u32 height)
{
    td_s32 ret;
    td_u32 vdec_chn_num, vpss_grp_num;
    sample_vdec_attr sample_vdec[OT_VDEC_MAX_CHN_NUM];
    td_bool vpss_chn_enable[OT_VPSS_MAX_CHN_NUM];
    ot_vpss_grp vpss_grp;
    ot_payload_type payload_type = OT_PT_H264;

    vdec_chn_num = 1;
    vpss_grp_num = vdec_chn_num;

    g_input_width = width;
    g_input_height = height;

    sample_uvc_update_vdec_flag(type_name);

    if (g_is_need_vdec == TD_TRUE) {
        payload_type = sample_uvc_get_payload_type(type_name);
        if (payload_type == OT_PT_BUTT) {
            return TD_FAILURE;
        }
    }

    ret = sample_uvc_init_sys_and_vb(&sample_vdec[0], vdec_chn_num, payload_type, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = sample_uvc_start_vdec(&sample_vdec[0], vdec_chn_num, OT_VDEC_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto stop_sys;
    }

    ret = sample_uvc_start_vpss(&vpss_grp, vpss_grp_num, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
    if (ret != TD_SUCCESS) {
        goto stop_vdec;
    }

    ret = sample_uvc_start_vo(&g_vo_config, vpss_grp_num);
    if (ret != TD_SUCCESS) {
        goto stop_vpss;
    }

    return TD_SUCCESS;

stop_vpss:
    ret = sample_uvc_vdec_unbind_vpss(vpss_grp_num);
    sample_uvc_stop_vpss(vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
stop_vdec:
    sample_uvc_stop_vdec(vdec_chn_num);
stop_sys:
    sample_comm_sys_exit();

    return TD_FAILURE;
}

td_s32 sample_uvc_media_exit(td_void)
{
    td_u32 vdec_chn_num = 1;
    td_u32 vpss_grp_num = 1;
    ot_vpss_grp vpss_grp = 0;
    td_bool vpss_chn_enable[OT_VPSS_MAX_CHN_NUM] = {0};
    vpss_chn_enable[0] = TD_TRUE;

    (td_void)sample_uvc_vpss_unbind_vo(vpss_grp_num, &g_vo_config);
    sample_comm_vo_stop_vo(&g_vo_config);
    (td_void)sample_uvc_vdec_unbind_vpss(vpss_grp_num);
    sample_uvc_stop_vpss(vpss_grp, &vpss_chn_enable[0], OT_VPSS_MAX_CHN_NUM);
    sample_uvc_stop_vdec(vdec_chn_num);
    sample_comm_sys_exit();

    return TD_SUCCESS;
}

static td_void sample_uvc_cut_stream_for_mjpeg(td_void *data, td_u32 size, td_s32 chn_id,
    td_s32 *read_num, td_u32 *start)
{
    td_s32 i;
    td_u32 len;
    td_s32 read_len = size;
    td_u8 *buf = data;
    td_bool find_start = TD_FALSE;

    for (i = 0; i < read_len - 1; i++) {
        if (buf[i] == 0xFF && buf[i + 1] == 0xD8) {
            *start = i;
            find_start = TD_TRUE;
            i = i + 2; /* 2 mjpeg data structure index */
            break;
        }
    }

    for (; i < read_len - 3; i++) { /* 2 3 mjpeg data structure limit */
        if ((buf[i] == 0xFF) && (buf[i + 1] & 0xF0) == 0xE0) {
            len = (buf[i + 2] << 8) + buf[i + 3]; /* 2 3 mjpeg data structure index, 8 high byte in integer */
            i += 1 + len;
        } else {
            break;
        }
    }

    for (; i < read_len - 1; i++) {
        if (buf[i] == 0xFF && buf[i + 1] == 0xD9) {
            break;
        }
    }
    read_len = i + 2; /* 2 mjpeg data structure index */

    if (find_start == TD_FALSE) {
        sample_print("chn %d can not find JPEG start code! read_len %d!\n", chn_id, read_len);
    }

    *read_num = read_len;
}

static td_void sample_uvc_cut_stream_for_h264(td_void *data, td_u32 size, td_s32 chn_id, td_s32 *read_num)
{
    td_bool find_start = TD_FALSE;
    td_bool find_end = TD_FALSE;
    td_s32 i;
    td_u8 *buf = data;
    td_s32 read_len = size;
    const int min_block_len = 8;

    for (i = 0; i < read_len - min_block_len; i++) {
        int tmp = buf[i + 3] & 0x1F;  /* byte 3 contains NALU type */
        if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1 && /* index 0 1 2 for start code */
            (((tmp == 0x5 || tmp == 0x1) && ((buf[i + 4] & 0x80) == 0x80)) || /* 5 i-frame, index 4 for stream info */
            (tmp == 20 && (buf[i + 7] & 0x80) == 0x80))) { /* 20 nalue type, index 7 for type info */
            find_start = TD_TRUE;
            i += min_block_len;
            break;
        }
    }

    for (; i < read_len - min_block_len; i++) {
        int tmp = buf[i + 3] & 0x1F;  /* byte 3 contains NALU type */
        if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1 &&  /* index 0 1 2 for start code */
            (tmp == 15 || tmp == 7 || tmp == 8 || tmp == 6 || /* 6 SEI, 7 SPS, 8 PPS, 15 reserve NALU type */
            ((tmp == 5 || tmp == 1) && ((buf[i + 4] & 0x80) == 0x80)) || /* 5 i-frame, index 4 for stream info */
            (tmp == 20 && (buf[i + 7] & 0x80) == 0x80))) { /* 20 nalue type, index 7 for type info */
            find_end = TD_TRUE;
            break;
        }
    }

    if (i > 0) {
        read_len = i;
    }

    if (find_start == TD_FALSE) {
        sample_print("chn %d can not find H264 start code! read_len %d!\n", chn_id, read_len);
    }

    if (find_end == TD_FALSE) {
        read_len = i + min_block_len;
    }

    *read_num = read_len;
}

static td_void sample_uvc_cut_stream_for_h265(td_void *data, td_u32 size, td_s32 chn_id, td_s32 *read_num)
{
    td_bool find_start = TD_FALSE;
    td_bool find_end = TD_FALSE;
    td_bool new_pic = TD_FALSE;
    td_s32 i;
    td_u8 *buf = data;
    td_s32 read_len = size;

    for (i = 0; i < read_len - 6; i++) { /* 6:h265 frame start code length */
        td_u32 tmp = (buf[i + 3] & 0x7E) >> 1; /* 0x7E:frame start marker 3:index */
        new_pic = (buf[i + 0] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1 && /* 1 2:index */
            (tmp <= 21) && ((buf[i + 5] & 0x80) == 0x80)); /* 5:index 21 0x80:frame start marker */

        if (new_pic) {
            find_start = TD_TRUE;
            i += 6; /* 6:h265 frame start code length */
            break;
        }
    }

    for (; i < read_len - 6; i++) { /* 6:h265 frame start code length */
        td_u32 tmp = (buf[i + 3] & 0x7E) >> 1; /* 0x7E:frame start marker 3:index */
        new_pic = (buf[i + 0] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1 && /* 1 2:index */
            (tmp == 32 || tmp == 33 || tmp == 34 || tmp == 39 || tmp == 40 || /* 32 33 34 39 40:frame start marker */
            ((tmp <= 21) && (buf[i + 5] & 0x80) == 0x80))); /* 5:index 21 0x80:frame start marker */

        if (new_pic) {
            find_end = TD_TRUE;
            break;
        }
    }

    if (i > 0) {
        read_len = i;
    }

    if (find_start == TD_FALSE) {
        sample_print("chn %d can not find H265 start code! read_len %d!\n", chn_id, read_len);
    }

    if (find_end == TD_FALSE) {
        read_len = i + 6; /* 6:h265 frame start code length */
    }

    *read_num = read_len;
}

static td_s32 sample_uvc_prepare_frame_info(ot_vb_blk vb_blk, const ot_pic_buf_attr *buf_attr,
    const ot_vb_calc_cfg *calc_cfg, ot_video_frame_info *video_frame)
{
    video_frame->video_frame.header_phys_addr[0] = ss_mpi_vb_handle_to_phys_addr(vb_blk);
    if (video_frame->video_frame.header_phys_addr[0] == TD_NULL) {
        sample_print("ss_mpi_vb_handle_to_phys_addr fail\n");
        return TD_FAILURE;
    }

    video_frame->video_frame.header_virt_addr[0] =
        (td_u8*)ss_mpi_sys_mmap(video_frame->video_frame.header_phys_addr[0], calc_cfg->vb_size);
    if (video_frame->video_frame.header_virt_addr[0] == TD_NULL) {
        sample_print("ss_mpi_sys_mmap fail\n");
        return TD_FAILURE;
    }

    video_frame->mod_id = OT_ID_VGS;
    video_frame->pool_id = ss_mpi_vb_handle_to_pool_id(vb_blk);

    video_frame->video_frame.header_phys_addr[1] =
        video_frame->video_frame.header_phys_addr[0] + calc_cfg->head_y_size;
    video_frame->video_frame.header_virt_addr[1] =
        video_frame->video_frame.header_virt_addr[0] + calc_cfg->head_y_size;
    video_frame->video_frame.phys_addr[0] =
        video_frame->video_frame.header_phys_addr[0] + calc_cfg->head_size;
    video_frame->video_frame.phys_addr[1] =
        video_frame->video_frame.phys_addr[0] + calc_cfg->main_y_size;
    video_frame->video_frame.virt_addr[0] =
        video_frame->video_frame.header_virt_addr[0] + calc_cfg->head_size;
    video_frame->video_frame.virt_addr[1] =
        video_frame->video_frame.virt_addr[0] + calc_cfg->main_y_size;
    video_frame->video_frame.header_stride[0] = calc_cfg->head_stride;
    video_frame->video_frame.header_stride[1] = calc_cfg->head_stride;
    video_frame->video_frame.stride[0] = calc_cfg->main_stride;
    video_frame->video_frame.stride[1] = calc_cfg->main_stride;

    video_frame->video_frame.width = buf_attr->width;
    video_frame->video_frame.height = buf_attr->height;
    video_frame->video_frame.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    video_frame->video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    video_frame->video_frame.video_format = OT_VIDEO_FORMAT_LINEAR;
    video_frame->video_frame.field = OT_VIDEO_FIELD_FRAME;
    video_frame->video_frame.pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    return TD_SUCCESS;
}

static td_void sample_uvc_buf_attr_init(const ot_size *pic_size, ot_pic_buf_attr *buf_attr)
{
    buf_attr->width = pic_size->width;
    buf_attr->height = pic_size->height;
    buf_attr->pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    buf_attr->compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr->align = 0;
    buf_attr->bit_width = OT_DATA_BIT_WIDTH_8;
}

static td_void sample_uvc_yuyv_to_nv12(td_char *image_in, td_u32 width, td_u32 height, td_u32 size, td_char *image_out)
{
    td_u32 pixel_num = width * height;
    td_u32 cycle_num = size / pixel_num / 2; /* every pixel have 2 bytes in YUYV */

    td_char *y = image_out;
    td_char *uv = image_out + pixel_num;

    td_char *start = image_in;
    td_u32 i = 0, j = 0, k = 0;

    /* Y */
    for (i = 0; i < cycle_num; i++) {
        int index = 0;
        for (j = 0; j < pixel_num * 2; j = j + 2) { /* 2 bytes have 1 Y. (Y U Y V ...) */
            *(y + index) = *(start + j);
            index++;
        }
        start = image_in + pixel_num * 2 * i;   /* every pixel have 2 bytes in YUYV */
        y = y + pixel_num * 3 / 2;              /* 3 / 2 : every pixel 1.5 bytes  for NV12 */
    }

    /* UV */
    start = image_in;
    for (i = 0; i < cycle_num; i++) {
        int uv_index = 0;
        for (j = 0; j < height; j = j + 2) {  /* 2 : even lines have UV for YUYV */
            for (k = j * width * 2 + 1; k < width * 2 * (j + 1); k = k + 4) { /* 4 bytes has a UV, 2 for step */
                *(uv + uv_index) = *(start + k);
                *(uv + uv_index + 1) = *(start + k + 2); /* 2 is step length from U to V */
                uv_index += 2;  /* 2 bytes UV for NV12(U V U V...) */
            }
        }
        start = image_in + pixel_num * 2 * i;  /* every pixel have 2 bytes in YUYV */
        uv = uv + pixel_num * 3 / 2;  /* 3 / 2 : every pixel 1.5 bytes  for NV12 */
    }
}

static td_void sample_uvc_update_vb_cfg(ot_pixel_format pixel_format, td_u32 stride, const ot_size *pic_size,
    ot_vb_calc_cfg *calc_cfg)
{
    if (pixel_format != OT_PIXEL_FORMAT_VY1UY0_PACKAGE_422) {
        return;
    }

    calc_cfg->main_stride = stride >> 1;
    calc_cfg->main_y_size = calc_cfg->main_stride * pic_size->height;
}

static td_s32 sample_uvc_send_frame_to_vpss(td_void *data, td_u32 size, td_u32 stride,
    const ot_size *pic_size, ot_pixel_format pixel_format)
{
    td_s32 ret;
    ot_vpss_grp grp = 0;
    ot_video_frame_info frame_info = {0};
    td_s32 milli_sec = -1;
    ot_vb_blk vb_blk;
    ot_pic_buf_attr buf_attr = {0};
    ot_vb_calc_cfg calc_cfg = {0};

    sample_uvc_buf_attr_init(pic_size, &buf_attr);
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);
    sample_uvc_update_vb_cfg(pixel_format, stride, pic_size, &calc_cfg);

    vb_blk = ss_mpi_vb_get_blk(OT_VB_INVALID_POOL_ID, calc_cfg.vb_size, TD_NULL);
    if (vb_blk == OT_VB_INVALID_HANDLE) {
        sample_print("get vb blk(size:%u) failed!\n", calc_cfg.vb_size);
        return TD_FAILURE;
    }

    ret = sample_uvc_prepare_frame_info(vb_blk, &buf_attr, &calc_cfg, &frame_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (pixel_format == OT_PIXEL_FORMAT_VY1UY0_PACKAGE_422) {
        sample_uvc_yuyv_to_nv12(data, pic_size->width, pic_size->height, size, frame_info.video_frame.virt_addr[0]);
    } else {
        frame_info.video_frame.pixel_format = pixel_format;
        ret = memcpy_s(frame_info.video_frame.virt_addr[0], size, data, size);
        if (ret != EOK) {
            sample_print("memcpy_s video frame data fail %x\n", ret);
        }
    }

    ret = ss_mpi_vpss_send_frame(grp, &frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        sample_print("send frame to vpss failed!\n");
    }

    ss_mpi_sys_munmap(frame_info.video_frame.virt_addr[0], calc_cfg.vb_size);
    ret = ss_mpi_vb_release_blk(vb_blk);
    if (ret != TD_SUCCESS) {
        sample_print("release vb failed!\n");
        return TD_FAILURE;
    }

    return ret;
}

static td_s32 sample_uvc_get_pixel_format(const td_char *type_name, ot_pixel_format *pixel_format)
{
    if (strcmp(type_name, "YUYV") == 0) {
        *pixel_format = OT_PIXEL_FORMAT_VY1UY0_PACKAGE_422;
    } else if (strcmp(type_name, "NV12") == 0) {
        *pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    } else if (strcmp(type_name, "NV21") == 0) {
        *pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    } else {
        sample_print("pixel format error!\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 sample_uvc_media_send_data(td_void *data, td_u32 size, td_u32 stride,
    const ot_size *pic_size, const td_char *type_name)
{
    td_bool end_of_stream = TD_FALSE;
    td_s32 read_len = size;
    td_u8 *buf = data;
    ot_vdec_stream vdec_stream;
    td_u32 start = 0;
    td_s32 ret;
    td_s32 chn_id = 0;
    ot_pixel_format pixel_format;
    td_bool send_again = TD_TRUE;

    if (data == NULL || pic_size == NULL || type_name == NULL) {
        sample_print("data, pic_size or type_name is NULL\n");
        return TD_FAILURE;
    }

    if (strcmp(type_name, "MJPEG") == 0) {
        sample_uvc_cut_stream_for_mjpeg(data, size, chn_id, &read_len, &start);
    } else if (strcmp(type_name, "H264") == 0) {
        sample_uvc_cut_stream_for_h264(data, size, chn_id, &read_len);
    } else if (strcmp(type_name, "H265") == 0) {
        sample_uvc_cut_stream_for_h265(data, size, chn_id, &read_len);
    } else {
        ret = sample_uvc_get_pixel_format(type_name, &pixel_format);
        if (ret != TD_SUCCESS) {
            return ret;
        }
        return sample_uvc_send_frame_to_vpss(data, size, stride, pic_size, pixel_format);
    }

    vdec_stream.addr      = buf + start;
    vdec_stream.len       = read_len;
    vdec_stream.end_of_frame  = TD_TRUE;
    vdec_stream.end_of_stream = end_of_stream;
    vdec_stream.need_display     = 1;

    while (send_again == TD_TRUE) {
        ss_mpi_sys_get_cur_pts(&vdec_stream.pts);
        ret = ss_mpi_vdec_send_stream(chn_id, &vdec_stream, 0);
        if (ret != TD_SUCCESS) {
            usleep(1000);   /* 1000:1000us */
            sample_print("send stream to vdec failed!\n");
            continue;
        } else {
            end_of_stream = TD_FALSE;
        }
        send_again = TD_FALSE;
    }
    usleep(1000);       /* 1000:1000us */

    return TD_SUCCESS;
}

td_s32 sample_uvc_media_stop_receive_data(td_void)
{
    ot_vdec_stream vdec_stream = {0};
    td_s32 vdec_chn_num = 1;
    td_s32 chn_id = 0;

    if (g_is_need_vdec == TD_FALSE) {
        return TD_SUCCESS;
    }

    vdec_stream.end_of_stream = TD_TRUE;
    ss_mpi_vdec_send_stream(chn_id, &vdec_stream, -1);
    ss_mpi_vdec_stop_recv_stream(vdec_chn_num);

    return TD_SUCCESS;
}
